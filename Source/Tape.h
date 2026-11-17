// Delia - modelo de cinta
//
// La histeresis magnetica (clase Hysteresis) es una adaptacion escalar del modelo de
// ChowTape / AnalogTapeModel de Jatin Chowdhury (https://github.com/jatinchowdhury18/AnalogTapeModel),
// publicado bajo licencia GPLv3. Referencia: J. Chowdhury, "Real-time Physical Modelling for
// Analog Tape Machines", DAFx 2019. Por incluir este codigo, Delia se distribuye bajo GPLv3.
#pragma once
#include "Dsp.h"

namespace delia
{
// ---------------------------------------------------------------- histeresis (Jiles-Atherton, RK4)
class Hysteresis
{
public:
    void setSampleRate (double sr) { T = 1.0 / sr; }
    void reset() { M_n1 = H_n1 = H_d_n1 = 0.0; }

    void cook (double drive, double width, double sat)
    {
        M_s = 0.5 + 1.5 * (1.0 - sat);
        a = M_s / (0.01 + 6.0 * drive);
        c = std::sqrt (1.0 - width) - 0.01;
        k = 0.47875;
        nc = 1.0 - c;
        M_s_oa = M_s / a;
        M_s_oa_talpha = alpha * M_s_oa;
        M_s_oa_tc = c * M_s_oa;
        M_s_oa_tc_talpha = alpha * M_s_oa_tc;
        makeup = (1.0 + 0.6 * width) / (0.5 + 1.5 * (1.0 - sat));
    }

    double makeupGain() const { return makeup; }

    inline double process (double H) noexcept
    {
        H = std::clamp (H, -10.0, 10.0);
        double H_d = deriv (H, H_n1, H_d_n1);
        const double H_12 = (H + H_n1) * 0.5, H_d_12 = (H_d + H_d_n1) * 0.5;
        const double k1 = f (M_n1, H_n1, H_d_n1) * T;
        const double k2 = f (M_n1 + k1 * 0.5, H_12, H_d_12) * T;
        const double k3 = f (M_n1 + k2 * 0.5, H_12, H_d_12) * T;
        const double k4 = f (M_n1 + k3, H, H_d) * T;
        double M = M_n1 + k1 / 6.0 + k2 / 3.0 + k3 / 3.0 + k4 / 6.0;
        if (std::isnan (M) || std::abs (M) > 20.0) { M = 0.0; H_d = 0.0; }
        M_n1 = M; H_n1 = H; H_d_n1 = H_d;
        return M;
    }

private:
    static constexpr double alpha = 1.6e-3;
    double M_s = 1.0, a = 0.25, k = 0.47875, c = 0.17, nc = 0.83;
    double M_s_oa = 4, M_s_oa_talpha = 0, M_s_oa_tc = 0, M_s_oa_tc_talpha = 0, makeup = 1.0;
    double T = 1.0 / 96000.0;
    double M_n1 = 0, H_n1 = 0, H_d_n1 = 0;

    inline double deriv (double x_n, double x_n1, double x_d_n1) const noexcept
    {
        const double dAlpha = 0.75;
        return (((1.0 + dAlpha) / T) * (x_n - x_n1)) - dAlpha * x_d_n1;
    }

    inline double f (double M, double H, double H_d) const noexcept
    {
        const double Q = (H + M * alpha) / a;
        const bool nearZero = Q < 0.001 && Q > -0.001;
        double L, Lp;
        if (nearZero) { L = Q / 3.0; Lp = 1.0 / 3.0; }
        else
        {
            const double coth = 1.0 / std::tanh (Q), oneOverQ = 1.0 / Q;
            L = coth - oneOverQ;
            Lp = oneOverQ * oneOverQ - coth * coth + 1.0;
        }
        const double M_diff = L * M_s - M;
        const double delta = H_d >= 0.0 ? 1.0 : -1.0;
        const double sgnM = (M_diff > 0.0) - (M_diff < 0.0);
        const double kap1 = (delta == sgnM) ? nc : 0.0;
        const double f1Denom = nc * delta * k - alpha * M_diff;
        const double f1 = kap1 * M_diff / f1Denom;
        const double f2 = Lp * M_s_oa_tc;
        const double f3 = 1.0 - Lp * M_s_oa_tc_talpha;
        return H_d * (f1 + f2) / f3;
    }
};

// ---------------------------------------------------------------- una pasada por cinta (un canal)
// Saturacion con histeresis (2x), respuesta en frecuencia segun velocidad, wow/flutter, soplido, caidas.
class TapePass
{
public:
    void prepare (double sampleRate, uint32_t seed)
    {
        sr = sampleRate;
        hyst.setSampleRate (sr * 2.0);
        hyst.reset(); os.reset();
        wowDelay.resize ((int) (sr * 0.05));
        rng = Rng (seed);
        lp1.reset(); lp2.reset(); bump.reset(); dc.reset(); hissLp.reset(); hissHp.reset();
        wowPh = rng.uni(); flutPh = rng.uni(); dropGain = 1.0f; dropTarget = 1.0f; dropCount = (int) (sr * (1.0 + 5.0 * rng.uni()));
        dc.set (12.0, sr);
        hissLp.set (9000.0, sr); hissHp.set (500.0, sr);
        set (quality, speedCm, level);
    }

    // quality 0..1 (0 = copia exacta), speed en cm/s, recLevel: ganancia de grabacion
    void set (float q, float speed, float recLevel)
    {
        quality = std::clamp (q, 0.0f, 1.0f); speedCm = speed; level = recLevel;
        const double drive = 0.18 + 0.30 * quality;
        hyst.cook (drive, 0.5, 0.5);
        outTrim = trimFor (drive);
        // respuesta: mas agudos a mas velocidad; la perdida crece con 'quality'
        double top = speedCm >= 30.0f ? 19000.0 : (speedCm >= 15.0f ? 13000.0 : 7500.0);
        double fc = top * std::pow (2.0, -(double) quality * 0.9);
        lp1.setLowpass (fc, 0.62, sr);
        lp2.setLowpass (fc * 1.6, 0.55, sr);
        double bumpF = speedCm >= 30.0f ? 70.0 : (speedCm >= 15.0f ? 55.0 : 45.0);
        bump.setPeak (bumpF, 0.9, 1.2 * quality, sr);
        // wow y flutter: mas profundos a baja velocidad
        double depthScale = (38.0 / std::max (9.5f, speedCm));
        wowDepth = (float) (0.00035 * quality * depthScale * sr * 0.5);      // en muestras
        flutDepth = (float) (0.00006 * quality * depthScale * sr * 0.12);
        wowInc = 0.9 / sr; flutInc = 8.7 / sr;
        hissGain = quality <= 0.0f ? 0.0f : dbToGain (-66.0f + 10.0f * quality - (speedCm >= 30.0f ? 3.0f : 0.0f));
    }

    inline float tick (float x)
    {
        if (quality <= 0.0f) return x;
        // saturacion
        const float wet = std::min (1.0f, quality * 2.5f);
        const float in = x * level;
        float y = os.process (in, [this] (float v) { return (float) (hyst.process ((double) v) * hyst.makeupGain() * outTrim); });
        // 'os' introduce retardo de 15 muestras; se alinea la senal seca
        dryDelay[(size_t) dryIdx] = in; dryIdx = (dryIdx + 1) % kDry;
        float dry = dryDelay[(size_t) dryIdx];
        y = dry + wet * (y - dry);
        y = dc.tick (y) / std::max (0.05f, level);
        // respuesta
        y = bump.tick (lp2.tick (lp1.tick (y)));
        // wow y flutter
        wowPh += wowInc; if (wowPh >= 1.0) wowPh -= 1.0;
        flutPh += flutInc; if (flutPh >= 1.0) flutPh -= 1.0;
        double d = 8.0 + wowDepth * (1.0 + std::sin (kTwoPi * wowPh)) + flutDepth * (1.0 + std::sin (kTwoPi * flutPh));
        wowDelay.push (y);
        y = wowDelay.read (d);
        // caidas de senal: breves y escasas
        if (--dropCount <= 0)
        {
            if (dropTarget < 1.0f) { dropTarget = 1.0f; dropCount = (int) (sr * (2.0 + 9.0 * rng.uni()) / std::max (0.15f, quality)); }
            else { dropTarget = 1.0f - quality * (0.15f + 0.45f * rng.uni()); dropCount = (int) (sr * (0.004 + 0.02 * rng.uni())); }
        }
        dropGain += 0.004f * (dropTarget - dropGain);
        y *= dropGain;
        // soplido
        y += hissGain * hissHp.tick (hissLp.tick (rng.bip())) * 3.0f;
        return y;
    }

private:
    // Ganancia de pequena senal de la histeresis segun 'drive', medida una vez, para conservar el nivel
    static float trimFor (double drive)
    {
        static const std::array<float, 9> table = []
        {
            std::array<float, 9> t {};
            for (int i = 0; i < 9; ++i)
            {
                Hysteresis h; h.setSampleRate (96000.0); h.cook (0.15 + 0.045 * i, 0.5, 0.5);
                double sx = 0, sy = 0;
                for (int n = 0; n < 9600; ++n)
                {
                    double x = 0.2 * std::sin (kTwoPi * 500.0 * n / 96000.0), y = h.process (x) * h.makeupGain();
                    if (n >= 4800) { sx += x * x; sy += y * y; }
                }
                t[(size_t) i] = (float) std::sqrt (sx / std::max (1e-12, sy));
            }
            return t;
        }();
        double pos = std::clamp ((drive - 0.15) / 0.045, 0.0, 7.999);
        int i = (int) pos; float f = (float) (pos - i);
        return table[(size_t) i] + f * (table[(size_t) i + 1] - table[(size_t) i]);
    }
    static constexpr int kDry = 16;
    double sr = 48000.0;
    float quality = 0.5f, speedCm = 38.0f, level = 1.0f;
    Hysteresis hyst; Halfband os;
    std::array<float, kDry> dryDelay {}; int dryIdx = 0;
    Biquad lp1, lp2, bump; OnePoleHP dc, hissHp; OnePoleLP hissLp;
    DelayLine wowDelay; double wowPh = 0, flutPh = 0, wowInc = 0, flutInc = 0;
    float wowDepth = 0, flutDepth = 0, hissGain = 0, outTrim = 1.0f;
    float dropGain = 1, dropTarget = 1; int dropCount = 0;
    Rng rng { 1 };
};
} // namespace delia
