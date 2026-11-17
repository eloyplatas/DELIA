// Delia - modelo de datos: carretes, tiras empalmadas y su render
#pragma once
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include "Dsp.h"
#include "Tuning.h"

namespace delia
{
constexpr double kTapeSpeeds[3] = { 9.5, 19.0, 38.0 };
constexpr double kTapeWidthCm = 0.635;
constexpr int kNumMachines = 4;

using AudioPtr = std::shared_ptr<const juce::AudioBuffer<float>>;

struct Reel
{
    int id = 0;
    juce::String name;
    juce::File file;
    AudioPtr audio;
    double sr = 48000.0;
    int generation = 0;
    bool missing = false;
    juce::String md5;

    // makeup
    double regionStart = 0.0, regionEnd = 1.0;     // fraccion de la longitud
    double fadeIn = 0.002, fadeOut = 0.01;         // segundos
    double rootHz = 220.0;
    int scaleType = scaleEqual;
    Scale custom = Scale::equal();
    int lowDeg = -12, highDeg = 12;

    int numSamples() const { return audio ? audio->getNumSamples() : 0; }
    int numChannels() const { return audio ? audio->getNumChannels() : 0; }
    bool isStereo() const { return numChannels() > 1; }
    Scale scale() const { return makeScale (scaleType, custom); }
    double regionSeconds() const { return (regionEnd - regionStart) * numSamples() / sr; }
    double ratioFor (int degree) const { return scale().ratioForDegree (degree); }
    double lengthCm() const { return 0; }
};
using ReelPtr = std::shared_ptr<Reel>;

// Lee una nota del carrete (region con fundidos, a la razon de velocidad 'ratio') y la SUMA en dst.
// gainFn(n) permite aplicar fundidos de empalme. Devuelve nada; fuera de la region es silencio.
template <typename GainFn>
inline void renderNote (const Reel& r, double ratio, double offsetSec, bool reverse, double hostSr,
                        juce::AudioBuffer<float>& dst, int dstStart, int num, bool wrap, GainFn&& gainFn)
{
    if (! r.audio || r.numSamples() < 2) return;
    const auto& src = *r.audio;
    const int len = src.getNumSamples();
    const double regA = r.regionStart * len, regB = r.regionEnd * len;
    if (regB - regA < 2.0) return;
    const double step = ratio * r.sr / hostSr;
    const double fi = juce::jmax (1.0, r.fadeIn * r.sr), fo = juce::jmax (1.0, r.fadeOut * r.sr);
    const auto& bank = SincBank::get();
    const int cut = bank.cutFor (step);
    const int dstLen = dst.getNumSamples();
    for (int ch = 0; ch < dst.getNumChannels(); ++ch)
    {
        const float* s = src.getReadPointer (juce::jmin (ch, src.getNumChannels() - 1));
        float* d = dst.getWritePointer (ch);
        double t = offsetSec * hostSr * step;      // avance en muestras de origen
        for (int n = 0; n < num; ++n, t += step)
        {
            const double pos = reverse ? regB - 1.0 - t : regA + t;
            if (pos < regA || pos > regB - 1.0) { if (t > regB - regA) break; continue; }
            float g = (float) juce::jmin (1.0, (pos - regA) / fi, (regB - 1.0 - pos) / fo);
            g *= gainFn (n);
            int di = dstStart + n;
            if (di >= dstLen) { if (! wrap) break; di %= dstLen; }
            d[di] += g * bank.read (s, len, pos, cut, false);
        }
    }
}

struct Piece
{
    int reelId = -1;          // -1 = cinta virgen
    int degree = 0;
    double offsetCm = 0.0;
    double lengthCm = 19.0;
    int cut = 1;              // 0 recto, 1 = 45 grados, 2 = 60 grados
    bool reverse = false;
};

struct Strip
{
    std::vector<Piece> pieces;
    double totalCm() const { double t = 0; for (auto& p : pieces) t += p.lengthCm; return t; }
};

struct LoopData
{
    juce::AudioBuffer<float> audio;
    std::vector<int> bounds;      // muestra de inicio de cada trozo, mas el final
    double sr = 48000.0;
    int maxGeneration = 0;
};
using LoopPtr = std::shared_ptr<const LoopData>;

inline double cutLengthCm (int cut)
{
    switch (cut) { case 1: return kTapeWidthCm; case 2: return kTapeWidthCm / std::tan (kPi / 3.0); default: return 0.0; }
}

inline const Reel* findReel (const std::vector<ReelPtr>& reels, int id)
{
    for (auto& r : reels) if (r->id == id) return r.get();
    return nullptr;
}

// Monta una tira: convierte la lista de trozos en audio a la frecuencia del anfitrion
inline LoopPtr renderStrip (const Strip& strip, const std::vector<ReelPtr>& reels, double tapeSpeedCm,
                            double hostSr, double cutErrorMm, uint32_t seed, bool loop)
{
    auto out = std::make_shared<LoopData>();
    out->sr = hostSr;
    const int n = (int) strip.pieces.size();
    if (n == 0) return out;
    std::vector<int> len ((size_t) n);
    bool stereo = false; int total = 0;
    for (int i = 0; i < n; ++i)
    {
        Rng rng (seed + (uint32_t) i * 7919u);
        double cm = strip.pieces[(size_t) i].lengthCm + rng.bip() * cutErrorMm * 0.1;
        len[(size_t) i] = juce::jmax (1, (int) std::round (juce::jmax (0.05, cm) / tapeSpeedCm * hostSr));
        total += len[(size_t) i];
        if (auto* r = findReel (reels, strip.pieces[(size_t) i].reelId))
        {
            stereo = stereo || r->isStereo();
            out->maxGeneration = juce::jmax (out->maxGeneration, r->generation);
        }
    }
    out->audio.setSize (stereo ? 2 : 1, total);
    out->audio.clear();
    // fundido de cada union (la que precede al trozo i)
    std::vector<int> xf ((size_t) n);
    for (int i = 0; i < n; ++i)
    {
        int prev = (i + n - 1) % n;
        int d = (int) std::round (cutLengthCm (strip.pieces[(size_t) i].cut) / tapeSpeedCm * hostSr);
        d = juce::jmin (d, len[(size_t) i] / 2, len[(size_t) prev] / 2);
        if (i == 0 && ! loop) d = 0;
        xf[(size_t) i] = juce::jmax (0, d);
    }
    int start = 0;
    for (int i = 0; i < n; ++i)
    {
        out->bounds.push_back (start);
        const auto& p = strip.pieces[(size_t) i];
        const int L = len[(size_t) i];
        const int din = xf[(size_t) i];
        int dout = xf[(size_t) ((i + 1) % n)];
        if (i == n - 1 && ! loop) dout = 0;
        if (auto* r = findReel (reels, p.reelId))
        {
            const double ratio = r->ratioFor (p.degree);
            renderNote (*r, ratio, p.offsetCm / tapeSpeedCm, p.reverse, hostSr, out->audio, start, L + dout, loop,
                        [=] (int k) -> float
                        {
                            float g = 1.0f;
                            if (k < din) g *= (float) (k + 1) / (float) (din + 1);
                            if (k >= L) g *= 1.0f - (float) (k - L + 1) / (float) (dout + 1);
                            return g;
                        });
        }
        start += L;
    }
    out->bounds.push_back (total);
    return out;
}

// ---------------------------------------------------------------- analisis
// Deteccion de altura (diferencia normalizada acumulada, tipo YIN) sobre el centro de la region
inline double detectPitch (const Reel& r)
{
    if (! r.audio) return 0.0;
    const int len = r.numSamples();
    int a = (int) (r.regionStart * len), b = (int) (r.regionEnd * len);
    const int W = 4096;
    if (b - a < W * 2) return 0.0;
    int start = a + (b - a) / 3;
    if (start + W * 2 > b) start = b - W * 2;
    std::vector<float> x ((size_t) W * 2);
    for (int i = 0; i < W * 2; ++i)
    {
        float v = 0; for (int ch = 0; ch < r.numChannels(); ++ch) v += r.audio->getSample (ch, start + i);
        x[(size_t) i] = v;
    }
    const int maxLag = W, minLag = (int) (r.sr / 4000.0);
    std::vector<double> dfn ((size_t) maxLag, 1.0);
    double run = 0.0;
    for (int lag = 1; lag < maxLag; ++lag)
    {
        double s = 0;
        for (int i = 0; i < W; ++i) { double df = x[(size_t) i] - x[(size_t) (i + lag)]; s += df * df; }
        run += s;
        dfn[(size_t) lag] = run > 0 ? s * lag / run : 1.0;
    }
    for (int lag = juce::jmax (2, minLag); lag < maxLag - 1; ++lag)
    {
        if (dfn[(size_t) lag] < 0.12)
        {
            while (lag + 1 < maxLag - 1 && dfn[(size_t) (lag + 1)] < dfn[(size_t) lag]) ++lag;
            double y0 = dfn[(size_t) (lag - 1)], y1 = dfn[(size_t) lag], y2 = dfn[(size_t) (lag + 1)];
            double den = y0 - 2 * y1 + y2;
            double off = std::abs (den) > 1e-12 ? 0.5 * (y0 - y2) / den : 0.0;
            return r.sr / (lag + off);
        }
    }
    return 0.0;
}

struct Partial { double freq = 0.0, amp = 0.0; };

// Los parciales mas fuertes de la region: FFT promediada, picos con interpolacion parabolica
inline std::vector<Partial> analysePartials (const Reel& r, int maxPartials, double minSepHz)
{
    std::vector<Partial> res;
    if (! r.audio) return res;
    const int len = r.numSamples();
    const int a = (int) (r.regionStart * len), b = (int) (r.regionEnd * len);
    if (b - a < 256) return res;
    const int order = (b - a) >= 32768 ? 15 : 14, N = 1 << order;
    juce::dsp::FFT fft (order);
    std::vector<float> acc ((size_t) N / 2, 0.0f), buf ((size_t) N * 2);
    int frames = 0;
    for (int s = a; s == a || s + N <= b; s += N / 2)
    {
        std::fill (buf.begin(), buf.end(), 0.0f);
        for (int i = 0; i < N && s + i < b; ++i)
        {
            float v = 0; for (int ch = 0; ch < r.numChannels(); ++ch) v += r.audio->getSample (ch, s + i);
            buf[(size_t) i] = v * (float) (0.5 - 0.5 * std::cos (kTwoPi * i / (N - 1)));
        }
        fft.performFrequencyOnlyForwardTransform (buf.data());
        for (int i = 0; i < N / 2; ++i) acc[(size_t) i] += buf[(size_t) i];
        if (++frames > 64) break;
    }
    std::vector<Partial> peaks;
    const double binHz = r.sr / N;
    for (int i = 2; i < N / 2 - 1; ++i)
    {
        float m = acc[(size_t) i];
        if (m > acc[(size_t) (i - 1)] && m >= acc[(size_t) (i + 1)] && i * binHz > 20.0)
        {
            double y0 = std::log (acc[(size_t) (i - 1)] + 1e-12), y1 = std::log (m + 1e-12), y2 = std::log (acc[(size_t) (i + 1)] + 1e-12);
            double den = y0 - 2 * y1 + y2, off = std::abs (den) > 1e-12 ? 0.5 * (y0 - y2) / den : 0.0;
            peaks.push_back ({ (i + off) * binHz, std::exp (y1 - 0.25 * (y0 - y2) * off) });
        }
    }
    std::sort (peaks.begin(), peaks.end(), [] (const Partial& x, const Partial& y) { return x.amp > y.amp; });
    for (auto& p : peaks)
    {
        bool ok = true;
        for (auto& q : res) if (std::abs (q.freq - p.freq) < minSepHz) { ok = false; break; }
        if (ok) res.push_back (p);
        if ((int) res.size() >= maxPartials) break;
    }
    if (! res.empty()) { double top = res.front().amp; for (auto& p : res) p.amp /= top; }
    std::sort (res.begin(), res.end(), [] (const Partial& x, const Partial& y) { return x.freq < y.freq; });
    return res;
}
} // namespace delia
