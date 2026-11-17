// Delia - motor de audio en tiempo real: maquinas, laboratorio, eco, placa, doce osciladores
#pragma once
#include <atomic>
#include "Model.h"
#include "Tape.h"

namespace delia
{
constexpr int kMaxBlock = 512;
constexpr int kNumOsc = 12, kNumGates = 8;

const float kHpFreqs[] = { 0.0f, 40.0f, 80.0f, 160.0f, 320.0f, 640.0f, 1250.0f, 2500.0f };
const float kLpFreqs[] = { 0.0f, 12000.0f, 8000.0f, 5600.0f, 4000.0f, 2800.0f, 2000.0f, 1400.0f, 1000.0f };
enum SyncMode { syncFree = 0, syncStart, syncLocked };
enum RingMode { ringOff = 0, ringOsc, ringNeighbour };

struct MachineParams
{
    float speed = 1, level = 1, pan = 0, sendEcho = 0, sendPlate = 0;
    bool run = false, reverse = false, loop = true, slope12 = false;
    int hp = 0, lp = 0, sync = syncFree, ring = ringOff;
};

struct EngineParams
{
    MachineParams m[kNumMachines];
    float drift = 0, quality = 0.5f, master = 1, inertia = 0.35f;
    int tapeSpeed = 2;
    float echoSpeed = 19, echoFb = 0.4f, echoSpacing = 5, echoReturn = 1; bool echoHead[3] { true, false, false };
    float plateDecay = 2.5f, plateDamp = 0.4f, platePre = 0.01f, plateReturn = 1;
    float oscFreq = 440, oscLevel = 0; int oscWave = 0;
    float noiseLevel = 0;
    float wobCenter = 200, wobWidth = 100, wobRate = 5, wobLevel = 0; int wobShape = 0;
    int labHp = 0, labLp = 0; float labEcho = 0, labPlate = 0;
    float bankFreq[kNumOsc] {}, bankLevel[kNumOsc] {}; int bankWave[kNumOsc] {}, bankGate[kNumOsc] {};
    float gateA[kNumGates] {}, gateD[kNumGates] {};
    float bankTransp = 1, bankMaster = 0.5f, bankDrift = 0; bool bankOn = false;
};

struct Transport { bool playing = false; double timeSamples = 0.0; };

struct PassiveFilter
{
    OnePoleHP hp[2]; OnePoleLP lp[2]; int hpI = -1, lpI = -1;
    void set (int h, int l, double sr)
    {
        if (h != hpI) { hpI = h; for (auto& f : hp) f.set (kHpFreqs[h] > 0 ? kHpFreqs[h] : 1.0, sr); }
        if (l != lpI) { lpI = l; for (auto& f : lp) f.set (kLpFreqs[l] > 0 ? kLpFreqs[l] : sr * 0.49, sr); }
    }
    inline float tick (float x, bool slope12)
    {
        if (hpI > 0) { x = hp[0].tick (x); if (slope12) x = hp[1].tick (x); }
        if (lpI > 0) { x = lp[0].tick (x); if (slope12) x = lp[1].tick (x); }
        return x;
    }
};

inline float polyBlep (double t, double dt)
{
    if (t < dt) { t /= dt; return (float) (t + t - t * t - 1.0); }
    if (t > 1.0 - dt) { t = (t - 1.0) / dt; return (float) (t * t + t + t + 1.0); }
    return 0.0f;
}
inline float oscWave (double ph, double inc, int wave)
{
    if (wave == 0) return (float) std::sin (kTwoPi * ph);
    float v = ph < 0.5 ? 1.0f : -1.0f;
    v += polyBlep (ph, inc);
    double p2 = ph + 0.5; if (p2 >= 1.0) p2 -= 1.0;
    v -= polyBlep (p2, inc);
    return v * 0.7f;
}

// ---------------------------------------------------------------- maquina de cinta
class Machine
{
public:
    void prepare (double sampleRate, uint32_t seed)
    {
        sr = sampleRate; Rng r (seed);
        for (int i = 0; i < 3; ++i) { dPh[i] = r.uni(); dRate[i] = (0.05 + 0.25 * r.uni()) / sr; }
        inertia = 0; speedSm = 1; gL = gR = 0;
    }

    void setLoop (LoopPtr l) { loop = std::move (l); int n = length(); if (n > 0 && pos >= n) pos = std::fmod (pos, (double) n); if (n == 0) pos = 0; }
    int length() const { return loop ? loop->audio.getNumSamples() : 0; }
    void rewind() { pos = 0; ended = false; }

    // Genera la salida cruda (sin filtros ni mezcla) en raw[0..1]
    void renderRaw (const MachineParams& p, const EngineParams& g, const Transport& t, bool playEdge, int n, float* rawL, float* rawR)
    {
        const int len = length();
        const bool running = p.run && len > 1 && (p.sync != syncLocked || t.playing);
        if (p.run && ! wasRun) { if (! p.loop) pos = p.reverse ? len - 1 : 0; ended = false; }
        wasRun = p.run;
        if (playEdge && p.sync == syncStart) { pos = p.reverse ? juce::jmax (0, len - 1) : 0; ended = false; }
        if (p.sync == syncLocked && t.playing && len > 1)
        {
            pos = std::fmod (t.timeSamples * p.speed, (double) len);
            if (p.reverse) pos = len - 1 - pos;
            inertia = running ? 1.0f : 0.0f;
        }
        const float target = (running && ! ended) ? 1.0f : 0.0f;
        const float ramp = 1.0f / (float) juce::jmax (1.0, g.inertia * sr);
        const float dir = p.reverse ? -1.0f : 1.0f;
        const bool stereo = loop && loop->audio.getNumChannels() > 1;
        const float* sL = len > 1 ? loop->audio.getReadPointer (0) : nullptr;
        const float* sR = stereo ? loop->audio.getReadPointer (1) : sL;
        const auto& bank = SincBank::get();
        const int cut = bank.cutFor (p.speed * 1.01);
        const float driftAmt = p.sync == syncLocked ? 0.0f : g.drift;
        float peak = 0.0f;
        for (int i = 0; i < n; ++i)
        {
            if (inertia < target) inertia = juce::jmin (target, inertia + ramp);
            else if (inertia > target) inertia = juce::jmax (target, inertia - ramp);
            speedSm += 0.002f * (p.speed - speedSm);
            float d = 0.0f;
            for (int k = 0; k < 3; ++k) { dPh[k] += dRate[k]; if (dPh[k] >= 1.0) dPh[k] -= 1.0; d += (float) std::sin (kTwoPi * dPh[k]); }
            const double v = (double) speedSm * (1.0 + driftAmt * d * (1.0f / 3.0f)) * inertia;
            float l = 0.0f, r = 0.0f;
            if (sL != nullptr && inertia > 0.0f)
            {
                l = bank.read (sL, len, pos, cut, p.loop);
                r = stereo ? bank.read (sR, len, pos, cut, p.loop) : l;
                pos += v * dir;
                if (p.loop) { if (pos >= len) pos -= len; else if (pos < 0) pos += len; }
                else if (pos >= len || pos < 0) { ended = true; pos = juce::jlimit (0.0, (double) len - 1, pos); inertia = 0; }
                // el motor al arrancar o parar: la amplitud de lectura sigue a la velocidad
                const float amp = juce::jmin (1.0f, inertia * 4.0f);
                l *= amp; r *= amp;
            }
            rawL[i] = l; rawR[i] = r;
            peak = juce::jmax (peak, std::abs (l), std::abs (r));
        }
        rawPeak = peak;
        uiPos.store (len > 0 ? (float) (pos / len) : 0.0f);
        uiSpeed.store ((float) (speedSm * inertia * dir));
        uiEnded.store (ended);
    }

    PassiveFilter filt[2];
    float gL = 0, gR = 0, gE = 0, gP = 0, rawPeak = 0;
    std::atomic<float> uiPos { 0 }, uiSpeed { 0 }, uiPeak { 0 };
    std::atomic<bool> uiEnded { false };

private:
    LoopPtr loop; double sr = 48000, pos = 0; float inertia = 0, speedSm = 1;
    double dPh[3] {}, dRate[3] {}; bool wasRun = false, ended = false;
};

// ---------------------------------------------------------------- eco de cinta con tres cabezales
class TapeEcho
{
public:
    void prepare (double sampleRate, uint32_t seed)
    {
        sr = sampleRate; dl.resize ((int) (sr * 3.6)); tape.prepare (sr, seed); speedSm = 19; lp.set (9000, sr); hp.set (60, sr);
    }
    inline float tick (float in, const EngineParams& p)
    {
        speedSm += 0.00025f * (p.echoSpeed - speedSm);
        const double unit = (double) p.echoSpacing / speedSm * sr;
        float out = 0.0f, last = 0.0f; int heads = 0;
        for (int h = 0; h < 3; ++h)
            if (p.echoHead[h]) { last = dl.read (unit * (h + 1)); out += last; ++heads; }
        if (heads == 0) last = dl.read (unit);
        float w = in + p.echoFb * last;
        w = tape.tick (std::tanh (w * 0.8f) * 1.25f);
        w = hp.tick (lp.tick (w));
        dl.push (w);
        return out;
    }
    void setTape (float q, float speed) { tape.set (q * 0.7f, speed, 1.0f); }
private:
    double sr = 48000; DelayLine dl; TapePass tape; float speedSm = 19; OnePoleLP lp; OnePoleHP hp;
};

// ---------------------------------------------------------------- reverberacion de placa (topologia de Dattorro)
class Plate
{
public:
    void prepare (double sampleRate)
    {
        sr = sampleRate; k = sr / 29761.0;
        auto L = [this] (int n) { return juce::jmax (1, (int) std::round (n * k)); };
        pre.resize ((int) (sr * 0.25));
        in1.set (L (142), 0.75f); in2.set (L (107), 0.75f); in3.set (L (379), 0.625f); in4.set (L (277), 0.625f);
        apA.set (L (672) + 40, -0.7f); apB.set (L (908) + 40, -0.7f); lenA = L (672); lenB = L (908);
        dA1.resize (L (4453) + 8); apA2.set (L (1800), 0.5f); dA2.resize (L (3720) + 8);
        dB1.resize (L (4217) + 8); apB2.set (L (2656), 0.5f); dB2.resize (L (3163) + 8);
        nA1 = L (4453); nA2 = L (3720); nB1 = L (4217); nB2 = L (3163);
        bw.set (9000, sr); inHp.set (120, sr); fbA = fbB = 0; lfo = 0;
    }
    inline void tick (float in, const EngineParams& p, float& outL, float& outR)
    {
        auto T = [this] (int n) { return (int) (n * k); };
        const float decay = (float) std::pow (10.0, -3.0 * 0.725 / (4.0 * juce::jlimit (0.3f, 8.0f, p.plateDecay)));
        dampA.a = dampB.a = 1.0f - juce::jlimit (0.0f, 0.95f, p.plateDamp);
        pre.push (in);
        float x = pre.read (juce::jmax (1.0, (double) p.platePre * sr));
        x = inHp.tick (bw.tick (x));
        x = in4.tick (in3.tick (in2.tick (in1.tick (x))));
        lfo += 0.7 / sr; if (lfo >= 1.0) lfo -= 1.0;
        const double mod = 8.0 * k * std::sin (kTwoPi * lfo);
        float a = apA.tickMod (x + decay * fbB, lenA + mod);
        dA1.push (a); a = dampA.tick (dA1.at (nA1 - 1)) * decay;
        a = apA2.tick (a); dA2.push (a); fbA = dA2.at (nA2 - 1);
        float b = apB.tickMod (x + decay * fbA, lenB - mod);
        dB1.push (b); b = dampB.tick (dB1.at (nB1 - 1)) * decay;
        b = apB2.tick (b); dB2.push (b); fbB = dB2.at (nB2 - 1);
        outL = 0.6f * (dA1.at (T (266)) + dA1.at (T (2974)) - apA2.tap (T (1913)) + dA2.at (T (1996)) - dB1.at (T (1990)) - apB2.tap (T (187)) - dB2.at (T (1066)));
        outR = 0.6f * (dB1.at (T (353)) + dB1.at (T (3627)) - apB2.tap (T (1228)) + dB2.at (T (2673)) - dA1.at (T (2111)) - apA2.tap (T (335)) - dA2.at (T (121)));
    }
private:
    double sr = 48000, k = 1, lfo = 0; int lenA = 1, lenB = 1, nA1 = 1, nA2 = 1, nB1 = 1, nB2 = 1;
    DelayLine pre, dA1, dA2, dB1, dB2; Allpass in1, in2, in3, in4, apA, apB, apA2, apB2;
    OnePoleLP bw, dampA, dampB; OnePoleHP inHp; float fbA = 0, fbB = 0;
};

// ---------------------------------------------------------------- motor
class Engine
{
public:
    Machine machines[kNumMachines];
    std::atomic<bool> gateHeld[kNumGates];
    std::atomic<float> masterPeak[2], gateLevel[kNumGates];
    // buses del ultimo bloque, para captura
    float labBus[kMaxBlock] {}, bankBus[kMaxBlock] {}, mixL[kMaxBlock] {}, mixR[kMaxBlock] {};

    Engine() { for (auto& g : gateHeld) g = false; for (auto& g : gateLevel) g = 0; masterPeak[0] = masterPeak[1] = 0; }

    void prepare (double sampleRate, uint32_t seed)
    {
        sr = sampleRate;
        for (int m = 0; m < kNumMachines; ++m) machines[m].prepare (sr, seed + 101u * (uint32_t) (m + 1));
        echo.prepare (sr, seed + 7u); plate.prepare (sr);
        Rng r (seed + 33u);
        for (int i = 0; i < kNumOsc; ++i) { bPh[i] = r.uni(); bdPh[i] = r.uni(); bdRate[i] = (0.03 + 0.2 * r.uni()) / sr; }
        for (auto& e : env) e = 0;
        noise = Rng (seed + 55u);
        oscPh = wobPh = wobLfo = 0; oscFreqSm = 440; wasPlaying = false;
        for (auto& f : labFilt.hp) f.reset(); for (auto& f : labFilt.lp) f.reset();
    }

    // n <= kMaxBlock. aux[m] puede ser nullptr.
    void render (const EngineParams& p, const Transport& t, int n, float* outL, float* outR, float* const* aux)
    {
        const bool playEdge = t.playing && ! wasPlaying; wasPlaying = t.playing;
        // --- laboratorio
        labFilt.set (p.labHp, p.labLp, sr);
        const float oscG = p.oscLevel, noiG = p.noiseLevel, wobG = p.wobLevel;
        for (int i = 0; i < n; ++i)
        {
            oscFreqSm += 0.0006f * (p.oscFreq - oscFreqSm);
            const double inc = juce::jlimit (1.0, sr * 0.45, (double) oscFreqSm) / sr;
            oscPh += inc; if (oscPh >= 1.0) oscPh -= 1.0;
            const float o = oscWave (oscPh, inc, p.oscWave);
            carrier[i] = o;
            wobLfo += p.wobRate / sr; if (wobLfo >= 1.0) wobLfo -= 1.0;
            double sh = p.wobShape == 0 ? std::sin (kTwoPi * wobLfo) : (p.wobShape == 1 ? 4.0 * std::abs (wobLfo - 0.5) - 1.0 : 2.0 * wobLfo - 1.0);
            const double wf = juce::jlimit (2.0, sr * 0.45, (double) p.wobCenter + 0.5 * p.wobWidth * sh);
            wobPh += wf / sr; if (wobPh >= 1.0) wobPh -= 1.0;
            float lab = o * oscG + (float) std::sin (kTwoPi * wobPh) * wobG;
            if (noiG > 0.0f) lab += noise.bip() * noiG; else noise.next();
            labBus[i] = labFilt.tick (lab, true);
        }
        // --- doce osciladores
        renderBank (p, n);
        // --- maquinas
        for (int m = 0; m < kNumMachines; ++m)
            machines[m].renderRaw (p.m[m], p, t, playEdge, n, raw[m][0], raw[m][1]);
        for (int i = 0; i < n; ++i) { mixL[i] = labBus[i] + bankBus[i]; mixR[i] = mixL[i]; sendE[i] = labBus[i] * p.labEcho; sendP[i] = labBus[i] * p.labPlate; }
        for (int m = 0; m < kNumMachines; ++m)
        {
            auto& mc = machines[m]; const auto& mp = p.m[m];
            mc.filt[0].set (mp.hp, mp.lp, sr); mc.filt[1].set (mp.hp, mp.lp, sr);
            const float ang = (mp.pan * 0.5f + 0.5f) * (float) (kPi * 0.5);
            const float tl = mp.level * std::cos (ang) * 1.41421f, tr = mp.level * std::sin (ang) * 1.41421f;
            const float* car = mp.ring == ringOsc ? carrier : (mp.ring == ringNeighbour ? raw[(m + 1) % kNumMachines][0] : nullptr);
            float* ax = aux != nullptr ? aux[m] : nullptr;
            float peak = 0.0f;
            for (int i = 0; i < n; ++i)
            {
                float l = raw[m][0][i], r = raw[m][1][i];
                if (car != nullptr) { l *= car[i] * 1.5f; r *= car[i] * 1.5f; }
                l = mc.filt[0].tick (l, mp.slope12); r = mc.filt[1].tick (r, mp.slope12);
                mc.gL += 0.002f * (tl - mc.gL); mc.gR += 0.002f * (tr - mc.gR);
                mc.gE += 0.002f * (mp.sendEcho * mp.level - mc.gE); mc.gP += 0.002f * (mp.sendPlate * mp.level - mc.gP);
                const float mono = 0.5f * (l + r);
                mixL[i] += l * mc.gL; mixR[i] += r * mc.gR;
                sendE[i] += mono * mc.gE; sendP[i] += mono * mc.gP;
                if (ax != nullptr) ax[i] = mono * mp.level;
                peak = juce::jmax (peak, std::abs (mono) * mp.level);
            }
            mc.uiPeak.store (juce::jmax (peak, mc.uiPeak.load() * 0.9f));
        }
        // --- tratamientos
        echo.setTape (p.quality, (float) kTapeSpeeds[juce::jlimit (0, 2, p.tapeSpeed)]);
        float pk[2] = { 0, 0 };
        for (int i = 0; i < n; ++i)
        {
            const float e = echo.tick (sendE[i], p) * p.echoReturn;
            float pl = 0, pr = 0;
            plate.tick (sendP[i] + e * 0.0f, p, pl, pr);
            mixL[i] += e + pl * p.plateReturn; mixR[i] += e + pr * p.plateReturn;
            outL[i] = mixL[i] * p.master; outR[i] = mixR[i] * p.master;
            pk[0] = juce::jmax (pk[0], std::abs (outL[i])); pk[1] = juce::jmax (pk[1], std::abs (outR[i]));
        }
        for (int c = 0; c < 2; ++c) masterPeak[c].store (juce::jmax (pk[c], masterPeak[c].load() * 0.92f));
    }

private:
    double sr = 48000; bool wasPlaying = false;
    TapeEcho echo; Plate plate; PassiveFilter labFilt; Rng noise { 5 };
    double oscPh = 0, wobPh = 0, wobLfo = 0; float oscFreqSm = 440;
    double bPh[kNumOsc] {}, bdPh[kNumOsc] {}, bdRate[kNumOsc] {}; float env[kNumGates] {}, bLvl[kNumOsc] {};
    float raw[kNumMachines][2][kMaxBlock] {}, carrier[kMaxBlock] {}, sendE[kMaxBlock] {}, sendP[kMaxBlock] {};

    void renderBank (const EngineParams& p, int n)
    {
        std::fill (bankBus, bankBus + n, 0.0f);
        if (! p.bankOn) { for (auto& e : env) e = 0; for (auto& g : gateLevel) g.store (0); return; }
        float envBuf[kNumGates][kMaxBlock];
        for (int g = 0; g < kNumGates; ++g)
        {
            const bool held = gateHeld[g].load();
            const float up = 1.0f / (float) juce::jmax (1.0, p.gateA[g] * sr), down = (float) std::exp (-1.0 / juce::jmax (1.0, p.gateD[g] * sr * 0.3));
            float e = env[g];
            for (int i = 0; i < n; ++i) { if (held) e = juce::jmin (1.0f, e + up); else e *= down; envBuf[g][i] = e; }
            env[g] = e < 1e-5f ? 0.0f : e; gateLevel[g].store (e);
        }
        for (int o = 0; o < kNumOsc; ++o)
        {
            if (p.bankFreq[o] <= 0.0f) continue;
            const int gate = p.bankGate[o];
            const float tgt = p.bankLevel[o];
            for (int i = 0; i < n; ++i)
            {
                bdPh[o] += bdRate[o]; if (bdPh[o] >= 1.0) bdPh[o] -= 1.0;
                const double cents = p.bankDrift * std::sin (kTwoPi * bdPh[o]);
                const double f = juce::jlimit (1.0, sr * 0.45, (double) p.bankFreq[o] * p.bankTransp * (1.0 + cents * 0.00057779));
                const double inc = f / sr;
                bPh[o] += inc; if (bPh[o] >= 1.0) bPh[o] -= 1.0;
                bLvl[o] += 0.002f * (tgt - bLvl[o]);
                const float e = gate > 0 ? envBuf[gate - 1][i] : 1.0f;
                bankBus[i] += oscWave (bPh[o], inc, p.bankWave[o]) * bLvl[o] * e;
            }
        }
        for (int i = 0; i < n; ++i) bankBus[i] *= p.bankMaster * 0.35f;
    }
};
} // namespace delia
