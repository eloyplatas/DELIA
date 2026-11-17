// Delia - utilidades DSP basicas
#pragma once
#include <cmath>
#include <vector>
#include <array>
#include <memory>
#include <cstdint>
#include <algorithm>

namespace delia
{
constexpr double kPi = 3.14159265358979323846;
constexpr double kTwoPi = 2.0 * kPi;

inline float dbToGain (float db) { return db <= -90.0f ? 0.0f : std::pow (10.0f, db * 0.05f); }

// Generador pseudoaleatorio con semilla (xorshift32): todo el azar del plugin es reproducible
struct Rng
{
    uint32_t s = 0x9E3779B9u;
    explicit Rng (uint32_t seed = 1) { s = seed ? seed : 1u; next(); next(); }
    uint32_t next() { s ^= s << 13; s ^= s >> 17; s ^= s << 5; return s; }
    float uni() { return (float) (next() >> 8) * (1.0f / 16777216.0f); }
    float bip() { return uni() * 2.0f - 1.0f; }
};

struct OnePoleLP
{
    float z = 0.0f, a = 1.0f;
    void set (double fc, double sr) { a = (float) (1.0 - std::exp (-kTwoPi * std::min (fc, sr * 0.49) / sr)); }
    float tick (float x) { z += a * (x - z); return z; }
    void reset() { z = 0.0f; }
};

struct OnePoleHP
{
    OnePoleLP lp;
    void set (double fc, double sr) { lp.set (fc, sr); }
    float tick (float x) { return x - lp.tick (x); }
    void reset() { lp.reset(); }
};

// Biquad RBJ (forma directa transpuesta II)
struct Biquad
{
    float b0 = 1, b1 = 0, b2 = 0, a1 = 0, a2 = 0, z1 = 0, z2 = 0;
    void reset() { z1 = z2 = 0; }
    float tick (float x)
    {
        float y = b0 * x + z1;
        z1 = b1 * x - a1 * y + z2;
        z2 = b2 * x - a2 * y;
        return y;
    }
    void setPeak (double f, double q, double gainDb, double sr)
    {
        double A = std::pow (10.0, gainDb / 40.0), w = kTwoPi * f / sr, al = std::sin (w) / (2 * q), c = std::cos (w);
        double a0 = 1 + al / A;
        b0 = (float) ((1 + al * A) / a0); b1 = (float) (-2 * c / a0); b2 = (float) ((1 - al * A) / a0);
        a1 = (float) (-2 * c / a0); a2 = (float) ((1 - al / A) / a0);
    }
    void setLowpass (double f, double q, double sr)
    {
        double w = kTwoPi * std::min (f, sr * 0.49) / sr, al = std::sin (w) / (2 * q), c = std::cos (w), a0 = 1 + al;
        b0 = (float) ((1 - c) * 0.5 / a0); b1 = (float) ((1 - c) / a0); b2 = b0;
        a1 = (float) (-2 * c / a0); a2 = (float) ((1 - al) / a0);
    }
    void setHighpass (double f, double q, double sr)
    {
        double w = kTwoPi * std::min (f, sr * 0.49) / sr, al = std::sin (w) / (2 * q), c = std::cos (w), a0 = 1 + al;
        b0 = (float) ((1 + c) * 0.5 / a0); b1 = (float) (-(1 + c) / a0); b2 = b0;
        a1 = (float) (-2 * c / a0); a2 = (float) ((1 - al) / a0);
    }
};

// Banco de tablas sinc enventanadas para lectura a velocidad variable.
// Varias frecuencias de corte: al leer mas rapido que 1x se limita la banda para evitar aliasing,
// y el nucleo se alarga en proporcion para conservar la pendiente del filtro.
class SincBank
{
public:
    static constexpr int phases = 256, numCut = 7;

    static const SincBank& get() { static SincBank b; return b; }

    int cutFor (double speed) const
    {
        double need = speed <= 1.0 ? 1.0 : 1.0 / speed;
        for (int i = 0; i < numCut; ++i)
            if (cuts[(size_t) i] <= need * 1.0001) return i;
        return numCut - 1;
    }

    // Lee una muestra interpolada en pos (en muestras). loop: envuelve; si no, ceros fuera.
    inline float read (const float* d, int len, double pos, int cut, bool loop) const
    {
        if (len <= 0) return 0.0f;
        const int taps = ntaps[(size_t) cut];
        double fl = std::floor (pos);
        int i0 = (int) fl;
        float fr = (float) (pos - fl) * (float) phases;
        int ph = (int) fr;
        float mu = fr - (float) ph;
        if (ph >= phases) { ph = phases - 1; mu = 1.0f; }
        const float* ha = table[(size_t) cut].data() + (size_t) ph * (size_t) taps;
        const float* hb = ha + taps;
        int start = i0 - taps / 2 + 1;
        float acc = 0.0f;
        if (start >= 0 && start + taps <= len)
        {
            const float* p = d + start;
            for (int k = 0; k < taps; ++k)
                acc += p[k] * (ha[k] + mu * (hb[k] - ha[k]));
        }
        else if (loop)
        {
            int idx = start % len; if (idx < 0) idx += len;
            for (int k = 0; k < taps; ++k)
            {
                acc += d[idx] * (ha[k] + mu * (hb[k] - ha[k]));
                if (++idx >= len) idx = 0;
            }
        }
        else
        {
            for (int k = 0; k < taps; ++k)
            {
                int idx = start + k;
                if (idx >= 0 && idx < len) acc += d[idx] * (ha[k] + mu * (hb[k] - ha[k]));
            }
        }
        return acc;
    }

private:
    std::array<double, numCut> cuts { 1.0, 0.8, 0.63, 0.5, 0.4, 0.315, 0.25 };
    std::array<int, numCut> ntaps {};
    std::array<std::vector<float>, numCut> table;

    SincBank()
    {
        for (int c = 0; c < numCut; ++c)
        {
            const int taps = 2 * (int) std::ceil (16.0 / cuts[(size_t) c]);
            ntaps[(size_t) c] = taps;
            auto& t = table[(size_t) c];
            t.assign ((size_t) (phases + 1) * (size_t) taps, 0.0f);
            // la banda de transicion (aprox. 8/taps de la frecuencia de muestreo) queda por debajo del nuevo Nyquist
            const double fc = cuts[(size_t) c] * (c == 0 ? 0.94 : 0.78);
            std::vector<double> h ((size_t) taps);
            for (int p = 0; p <= phases; ++p)
            {
                double frac = (double) p / phases, sum = 0.0;
                for (int k = 0; k < taps; ++k)
                {
                    double x = (double) (k - taps / 2 + 1) - frac;
                    double s = std::abs (x) < 1e-9 ? fc : std::sin (kPi * fc * x) / (kPi * x);
                    double wn = std::clamp ((x + taps / 2.0) / (double) taps, 0.0, 1.0);
                    double w = 0.35875 - 0.48829 * std::cos (kTwoPi * wn) + 0.14128 * std::cos (2 * kTwoPi * wn) - 0.01168 * std::cos (3 * kTwoPi * wn);
                    h[(size_t) k] = s * w; sum += h[(size_t) k];
                }
                for (int k = 0; k < taps; ++k)
                    t[(size_t) p * (size_t) taps + (size_t) k] = (float) (h[(size_t) k] / sum);
            }
        }
    }
};

// Sobremuestreo 2x con FIR de media banda (copiable, sin dependencias)
struct Halfband
{
    static constexpr int N = 31;
    std::array<float, N> h {};
    std::array<float, N> zu {}, zd {};
    int iu = 0, id = 0;
    Halfband()
    {
        for (int k = 0; k < N; ++k)
        {
            double x = k - (N - 1) / 2.0;
            double s = std::abs (x) < 1e-9 ? 0.5 : std::sin (kPi * 0.5 * x) / (kPi * x);
            double w = 0.42 - 0.5 * std::cos (kTwoPi * k / (N - 1)) + 0.08 * std::cos (2 * kTwoPi * k / (N - 1));
            h[(size_t) k] = (float) (s * w);
        }
        float sum = 0; for (auto v : h) sum += v;
        for (auto& v : h) v /= sum;
    }
    void reset() { zu.fill (0); zd.fill (0); }
    float fir (std::array<float, N>& z, int& i, float x)
    {
        z[(size_t) i] = x;
        float acc = 0; int j = i;
        for (int k = 0; k < N; ++k) { acc += h[(size_t) k] * z[(size_t) j]; if (--j < 0) j = N - 1; }
        if (++i >= N) i = 0;
        return acc;
    }
    // f: funcion no lineal aplicada a frecuencia doble
    template <typename F> float process (float x, F&& f)
    {
        float a = f (2.0f * fir (zu, iu, x));
        float ya = fir (zd, id, a);
        float b = f (2.0f * fir (zu, iu, 0.0f));
        fir (zd, id, b);
        return ya;
    }
};

// Linea de retardo con lectura interpolada (Hermite)
struct DelayLine
{
    std::vector<float> buf; int w = 0;
    void resize (int n) { buf.assign ((size_t) std::max (8, n), 0.0f); w = 0; }
    void clear() { std::fill (buf.begin(), buf.end(), 0.0f); }
    void push (float x) { buf[(size_t) w] = x; if (++w >= (int) buf.size()) w = 0; }
    float at (int d) const { int n = (int) buf.size(); int i = w - 1 - d; while (i < 0) i += n; return buf[(size_t) (i % n)]; }
    float read (double d) const
    {
        int n = (int) buf.size();
        d = std::clamp (d, 1.0, (double) n - 4.0);
        int di = (int) d; float f = (float) (d - di);
        float ym = at (di - 1), y0 = at (di), y1 = at (di + 1), y2 = at (di + 2);
        float c1 = 0.5f * (y1 - ym), c2 = ym - 2.5f * y0 + 2.0f * y1 - 0.5f * y2, c3 = 0.5f * (y2 - ym) + 1.5f * (y0 - y1);
        return ((c3 * f + c2) * f + c1) * f + y0;
    }
};

struct Allpass
{
    DelayLine dl; int len = 1; float g = 0.5f;
    void set (int n, float gain) { len = std::max (1, n); g = gain; dl.resize (len + 64); }
    float tick (float x) { float d = dl.at (len - 1); float v = x + g * d; dl.push (v); return d - g * v; }
    float tickMod (float x, double l) { float d = dl.read (l); float v = x + g * d; dl.push (v); return d - g * v; }
    float tap (int n) const { return dl.at (n); }
};
} // namespace delia
