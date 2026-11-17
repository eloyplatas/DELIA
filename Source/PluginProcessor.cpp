// Delia - procesador
#include "PluginProcessor.h"
#include "PluginEditor.h"

namespace delia
{
namespace
{
enum G { gDrift, gQuality, gMaster, gInertia, gTapeSpeed, gEchoSpeed, gEchoFb, gEchoSpacing, gEchoRet, gEchoH1, gEchoH2, gEchoH3,
         gPlateDecay, gPlateDamp, gPlatePre, gPlateRet, gOscFreq, gOscLevel, gOscWave, gNoiseLevel, gWobCenter, gWobWidth, gWobRate,
         gWobLevel, gWobShape, gLabHp, gLabLp, gLabEcho, gLabPlate, gBankTransp, gBankMaster, gBankDrift, gBankOn, gCount };
const char* const gIds[gCount] = { "drift", "quality", "master", "inertia", "tapeSpeed", "echoSpeed", "echoFb", "echoSpacing", "echoRet",
         "echoH1", "echoH2", "echoH3", "plateDecay", "plateDamp", "platePre", "plateRet", "oscFreq", "oscLevel", "oscWave", "noiseLevel",
         "wobCenter", "wobWidth", "wobRate", "wobLevel", "wobShape", "labHp", "labLp", "labEcho", "labPlate", "bankTransp", "bankMaster",
         "bankDrift", "bankOn" };

juce::NormalisableRange<float> logRange (float lo, float hi)
{
    return { lo, hi, [] (float a, float b, float t) { return a * std::pow (b / a, t); },
                     [] (float a, float b, float v) { return std::log (v / a) / std::log (b / a); } };
}
juce::StringArray hpNames() { juce::StringArray a; a.add ("--"); for (int i = 1; i < 8; ++i) a.add (juce::String ((int) kHpFreqs[i]) + " Hz"); return a; }
juce::StringArray lpNames() { juce::StringArray a; a.add ("--"); for (int i = 1; i < 9; ++i) a.add (juce::String (kLpFreqs[i] / 1000.0f, 1) + " kHz"); return a; }
}

juce::AudioProcessorValueTreeState::ParameterLayout DeliaProcessor::createLayout()
{
    using F = juce::AudioParameterFloat; using B = juce::AudioParameterBool; using C = juce::AudioParameterChoice;
    juce::AudioProcessorValueTreeState::ParameterLayout l;
    auto pid = [] (const juce::String& s) { return juce::ParameterID { s, 1 }; };
    for (int m = 1; m <= kNumMachines; ++m)
    {
        juce::String p = "m" + juce::String (m) + "_", n = U8 ("M\xc3\xa1quina ") + juce::String (m) + " ";
        l.add (std::make_unique<F> (pid (p + "speed"), n + "velocidad", logRange (0.25f, 4.0f), 1.0f));
        l.add (std::make_unique<B> (pid (p + "run"), n + "marcha", false));
        l.add (std::make_unique<B> (pid (p + "rev"), n + U8 ("atr\xc3\xa1s"), false));
        l.add (std::make_unique<B> (pid (p + "loop"), n + "bucle", true));
        l.add (std::make_unique<F> (pid (p + "level"), n + "nivel", juce::NormalisableRange<float> (-60.0f, 6.0f, 0.0f, 2.0f), -3.0f));
        l.add (std::make_unique<F> (pid (p + "pan"), n + "panorama", juce::NormalisableRange<float> (-1.0f, 1.0f), 0.0f));
        l.add (std::make_unique<C> (pid (p + "hp"), n + "paso alto", hpNames(), 0));
        l.add (std::make_unique<C> (pid (p + "lp"), n + "paso bajo", lpNames(), 0));
        l.add (std::make_unique<B> (pid (p + "slope"), n + "12 dB/oct", false));
        l.add (std::make_unique<F> (pid (p + "echo"), n + U8 ("env\xc3\xado eco"), juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        l.add (std::make_unique<F> (pid (p + "plate"), n + U8 ("env\xc3\xado placa"), juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
        l.add (std::make_unique<C> (pid (p + "sync"), n + U8 ("sincron\xc3\xad" "a"), juce::StringArray { "Libre", "Arranque", "Enganchado" }, 0));
        l.add (std::make_unique<C> (pid (p + "ring"), n + "anillo", juce::StringArray { "No", "Oscilador", U8 ("M\xc3\xa1quina vecina") }, 0));
    }
    l.add (std::make_unique<F> (pid ("drift"), "Deriva (%)", juce::NormalisableRange<float> (0.0f, 0.5f), 0.05f));
    l.add (std::make_unique<F> (pid ("quality"), "Desgaste de cinta (%)", juce::NormalisableRange<float> (0.0f, 100.0f), 35.0f));
    l.add (std::make_unique<F> (pid ("master"), "Salida", juce::NormalisableRange<float> (-60.0f, 6.0f, 0.0f, 2.0f), -3.0f));
    l.add (std::make_unique<F> (pid ("inertia"), "Inercia (s)", juce::NormalisableRange<float> (0.0f, 2.0f), 0.35f));
    l.add (std::make_unique<C> (pid ("tapeSpeed"), "Velocidad de cinta", juce::StringArray { "9,5 cm/s", "19 cm/s", "38 cm/s" }, 2));
    l.add (std::make_unique<F> (pid ("echoSpeed"), "Eco velocidad (cm/s)", logRange (9.5f, 38.0f), 19.0f));
    l.add (std::make_unique<F> (pid ("echoFb"), U8 ("Eco realimentaci\xc3\xb3n"), juce::NormalisableRange<float> (0.0f, 1.1f), 0.35f));
    l.add (std::make_unique<F> (pid ("echoSpacing"), U8 ("Eco separaci\xc3\xb3n (cm)"), juce::NormalisableRange<float> (2.0f, 10.0f), 5.0f));
    l.add (std::make_unique<F> (pid ("echoRet"), "Eco retorno", juce::NormalisableRange<float> (0.0f, 1.0f), 0.8f));
    l.add (std::make_unique<B> (pid ("echoH1"), "Eco cabezal 1", true));
    l.add (std::make_unique<B> (pid ("echoH2"), "Eco cabezal 2", false));
    l.add (std::make_unique<B> (pid ("echoH3"), "Eco cabezal 3", false));
    l.add (std::make_unique<F> (pid ("plateDecay"), U8 ("Placa ca\xc3\xad" "da (s)"), juce::NormalisableRange<float> (1.0f, 5.0f), 2.5f));
    l.add (std::make_unique<F> (pid ("plateDamp"), U8 ("Placa amortiguaci\xc3\xb3n"), juce::NormalisableRange<float> (0.0f, 1.0f), 0.45f));
    l.add (std::make_unique<F> (pid ("platePre"), "Placa predelay (s)", juce::NormalisableRange<float> (0.0f, 0.2f), 0.01f));
    l.add (std::make_unique<F> (pid ("plateRet"), "Placa retorno", juce::NormalisableRange<float> (0.0f, 1.0f), 0.8f));
    l.add (std::make_unique<F> (pid ("oscFreq"), "Oscilador frecuencia", logRange (20.0f, 20000.0f), 440.0f));
    l.add (std::make_unique<F> (pid ("oscLevel"), "Oscilador nivel", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    l.add (std::make_unique<C> (pid ("oscWave"), "Oscilador onda", juce::StringArray { "Senoidal", "Cuadrada" }, 0));
    l.add (std::make_unique<F> (pid ("noiseLevel"), "Ruido nivel", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    l.add (std::make_unique<F> (pid ("wobCenter"), "Wobbulator centro", logRange (20.0f, 10000.0f), 220.0f));
    l.add (std::make_unique<F> (pid ("wobWidth"), "Wobbulator anchura (Hz)", juce::NormalisableRange<float> (0.0f, 5000.0f, 0.0f, 0.4f), 120.0f));
    l.add (std::make_unique<F> (pid ("wobRate"), "Wobbulator velocidad", logRange (0.1f, 50.0f), 4.0f));
    l.add (std::make_unique<F> (pid ("wobLevel"), "Wobbulator nivel", juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    l.add (std::make_unique<C> (pid ("wobShape"), "Wobbulator forma", juce::StringArray { "Seno", U8 ("Tri\xc3\xa1ngulo"), "Sierra" }, 0));
    l.add (std::make_unique<C> (pid ("labHp"), "Laboratorio paso alto", hpNames(), 0));
    l.add (std::make_unique<C> (pid ("labLp"), "Laboratorio paso bajo", lpNames(), 0));
    l.add (std::make_unique<F> (pid ("labEcho"), U8 ("Laboratorio env\xc3\xado eco"), juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    l.add (std::make_unique<F> (pid ("labPlate"), U8 ("Laboratorio env\xc3\xado placa"), juce::NormalisableRange<float> (0.0f, 1.0f), 0.0f));
    l.add (std::make_unique<F> (pid ("bankTransp"), U8 ("Banco transposici\xc3\xb3n"), logRange (0.25f, 4.0f), 1.0f));
    l.add (std::make_unique<F> (pid ("bankMaster"), "Banco nivel", juce::NormalisableRange<float> (0.0f, 1.0f), 0.6f));
    l.add (std::make_unique<F> (pid ("bankDrift"), U8 ("Banco deriva de v\xc3\xa1lvula (cents)"), juce::NormalisableRange<float> (0.0f, 5.0f), 1.0f));
    l.add (std::make_unique<B> (pid ("bankOn"), "Banco encendido", false));
    juce::StringArray gates { "Abierto" }; for (int g = 1; g <= kNumGates; ++g) gates.add ("Puerta " + juce::String (g));
    for (int o = 1; o <= kNumOsc; ++o)
    {
        juce::String s (o);
        l.add (std::make_unique<F> (pid ("bFreq" + s), "Banco " + s + " frecuencia", juce::NormalisableRange<float> (0.0f, 20000.0f, 0.0f, 0.3f), 110.0f * (float) o));
        l.add (std::make_unique<F> (pid ("bLevel" + s), "Banco " + s + " nivel", juce::NormalisableRange<float> (0.0f, 1.0f), 1.0f / (float) o));
        l.add (std::make_unique<C> (pid ("bWave" + s), "Banco " + s + " onda", juce::StringArray { "Senoidal", "Cuadrada" }, 0));
        l.add (std::make_unique<C> (pid ("bGate" + s), "Banco " + s + " puerta", gates, 1 + (o - 1) % kNumGates));
    }
    for (int g = 1; g <= kNumGates; ++g)
    {
        juce::String s (g);
        l.add (std::make_unique<F> (pid ("gAtt" + s), "Puerta " + s + " ataque", juce::NormalisableRange<float> (0.001f, 2.0f, 0.0f, 0.35f), 0.01f));
        l.add (std::make_unique<F> (pid ("gDec" + s), "Puerta " + s + U8 (" ca\xc3\xad" "da"), juce::NormalisableRange<float> (0.01f, 10.0f, 0.0f, 0.35f), 1.2f));
    }
    return l;
}

DeliaProcessor::DeliaProcessor()
    : AudioProcessor (BusesProperties().withOutput ("Salida", juce::AudioChannelSet::stereo(), true)
                                        .withOutput (U8 ("M\xc3\xa1quina 1"), juce::AudioChannelSet::mono(), false)
                                        .withOutput (U8 ("M\xc3\xa1quina 2"), juce::AudioChannelSet::mono(), false)
                                        .withOutput (U8 ("M\xc3\xa1quina 3"), juce::AudioChannelSet::mono(), false)
                                        .withOutput (U8 ("M\xc3\xa1quina 4"), juce::AudioChannelSet::mono(), false)),
      apvts (*this, nullptr, "PARAMS", createLayout())
{
    formats.registerBasicFormats();
    engine = std::make_unique<Engine>();
    for (int m = 0; m < kNumMachines; ++m)
    {
        juce::String p = "m" + juce::String (m + 1) + "_";
        auto r = [&] (const char* s) { return apvts.getRawParameterValue (p + s); };
        mp[m] = { r ("speed"), r ("run"), r ("rev"), r ("loop"), r ("level"), r ("pan"), r ("hp"), r ("lp"), r ("slope"), r ("echo"), r ("plate"), r ("sync"), r ("ring") };
        runToggle[m] = 0;
    }
    for (int i = 0; i < gCount; ++i) gp[i] = apvts.getRawParameterValue (gIds[i]);
    for (int o = 0; o < kNumOsc; ++o)
    {
        juce::String s (o + 1);
        bFreq[o] = apvts.getRawParameterValue ("bFreq" + s); bLevel[o] = apvts.getRawParameterValue ("bLevel" + s);
        bWave[o] = apvts.getRawParameterValue ("bWave" + s); bGate[o] = apvts.getRawParameterValue ("bGate" + s);
    }
    for (int g = 0; g < kNumGates; ++g)
    {
        gAtt[g] = apvts.getRawParameterValue ("gAtt" + juce::String (g + 1)); gDec[g] = apvts.getRawParameterValue ("gDec" + juce::String (g + 1));
    }
    for (auto& s : stepFifo) s = 0;
    reelsFolder = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory).getChildFile ("Delia").getChildFile ("Carretes");
    engine->prepare (sr, seed);
    startTimerHz (30);
}

DeliaProcessor::~DeliaProcessor() { stopTimer(); }

void DeliaProcessor::fillParams (EngineParams& e) const
{
    for (int m = 0; m < kNumMachines; ++m)
    {
        auto& d = e.m[m]; const auto& s = mp[m];
        d.speed = s.speed->load(); d.run = s.run->load() > 0.5f; d.reverse = s.rev->load() > 0.5f; d.loop = s.loop->load() > 0.5f;
        d.level = dbToGain (s.level->load() <= -59.5f ? -100.0f : s.level->load()); d.pan = s.pan->load();
        d.hp = (int) s.hp->load(); d.lp = (int) s.lp->load(); d.slope12 = s.slope->load() > 0.5f;
        d.sendEcho = s.echo->load(); d.sendPlate = s.plate->load(); d.sync = (int) s.sync->load(); d.ring = (int) s.ring->load();
    }
    auto g = [this] (int i) { return gp[i]->load(); };
    e.drift = g (gDrift) * 0.01f; e.quality = g (gQuality) * 0.01f; e.master = dbToGain (g (gMaster) <= -59.5f ? -100.0f : g (gMaster));
    e.inertia = g (gInertia); e.tapeSpeed = (int) g (gTapeSpeed);
    e.echoSpeed = g (gEchoSpeed); e.echoFb = g (gEchoFb); e.echoSpacing = g (gEchoSpacing); e.echoReturn = g (gEchoRet);
    e.echoHead[0] = g (gEchoH1) > 0.5f; e.echoHead[1] = g (gEchoH2) > 0.5f; e.echoHead[2] = g (gEchoH3) > 0.5f;
    e.plateDecay = g (gPlateDecay); e.plateDamp = g (gPlateDamp); e.platePre = g (gPlatePre); e.plateReturn = g (gPlateRet);
    e.oscFreq = g (gOscFreq); e.oscLevel = g (gOscLevel); e.oscWave = (int) g (gOscWave); e.noiseLevel = g (gNoiseLevel);
    e.wobCenter = g (gWobCenter); e.wobWidth = g (gWobWidth); e.wobRate = g (gWobRate); e.wobLevel = g (gWobLevel); e.wobShape = (int) g (gWobShape);
    e.labHp = (int) g (gLabHp); e.labLp = (int) g (gLabLp); e.labEcho = g (gLabEcho); e.labPlate = g (gLabPlate);
    e.bankTransp = g (gBankTransp); e.bankMaster = g (gBankMaster); e.bankDrift = g (gBankDrift); e.bankOn = g (gBankOn) > 0.5f;
    for (int o = 0; o < kNumOsc; ++o)
    {
        e.bankFreq[o] = bFreq[o]->load(); e.bankLevel[o] = bLevel[o]->load(); e.bankWave[o] = (int) bWave[o]->load(); e.bankGate[o] = (int) bGate[o]->load();
    }
    for (int k = 0; k < kNumGates; ++k) { e.gateA[k] = gAtt[k]->load(); e.gateD[k] = gDec[k]->load(); }
}

void DeliaProcessor::setParam (const juce::String& id, float value)
{
    if (auto* p = apvts.getParameter (id))
    {
        p->beginChangeGesture();
        p->setValueNotifyingHost (p->convertTo0to1 (value));
        p->endChangeGesture();
    }
}

double DeliaProcessor::tapeSpeedCm() const { return kTapeSpeeds[juce::jlimit (0, 2, (int) gp[gTapeSpeed]->load())]; }

double DeliaProcessor::figureCm() const
{
    static const double beats[] = { 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
    double b = beats[juce::jlimit (0, 5, figureIndex)];
    if (figureDotted) b *= 1.5;
    if (figureTriplet) b *= 2.0 / 3.0;
    return b * 60.0 / juce::jmax (1.0, bpm()) * tapeSpeedCm();
}

// ---------------------------------------------------------------- audio
bool DeliaProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    if (l.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()) return false;
    for (int b = 1; b < l.outputBuses.size(); ++b)
        if (! l.outputBuses[b].isDisabled() && l.outputBuses[b] != juce::AudioChannelSet::mono()) return false;
    return true;
}

void DeliaProcessor::prepareToPlay (double sampleRate, int)
{
    const bool changed = std::abs (sampleRate - sr) > 0.5;
    sr = sampleRate;
    engine->prepare (sr, seed);
    {
        juce::SpinLock::ScopedLockType sl (loopLock);
        for (int m = 0; m < kNumMachines; ++m) { pending[m] = mounted[m]; pendingFlag[m] = true; }
    }
    if (changed)
    {
        if (juce::MessageManager::getInstance()->isThisTheMessageThread()) remountAll();
        else juce::MessageManager::callAsync ([this] { remountAll(); });
    }
}

void DeliaProcessor::renderVoices (float* l, float* r, int n)
{
    const Reel* a = audition.get();
    if (a == nullptr || ! a->audio) { for (auto& v : voices) v.on = false; return; }
    const auto& src = *a->audio; const int len = src.getNumSamples();
    const double regA = a->regionStart * len, regB = a->regionEnd * len;
    const double fi = juce::jmax (1.0, a->fadeIn * a->sr), fo = juce::jmax (1.0, a->fadeOut * a->sr);
    const auto& bank = SincBank::get();
    const float* s0 = src.getReadPointer (0); const float* s1 = src.getReadPointer (src.getNumChannels() > 1 ? 1 : 0);
    for (auto& v : voices)
    {
        if (! v.on) continue;
        const int cut = bank.cutFor (v.step);
        for (int i = 0; i < n; ++i)
        {
            const double pos = regA + v.t;
            if (pos > regB - 1.0) { v.on = false; break; }
            v.gain += 0.003f * (v.target - v.gain);
            if (v.target == 0.0f && v.gain < 1e-3f) { v.on = false; break; }
            const float g = v.gain * 0.8f * (float) juce::jmin (1.0, (pos - regA) / fi + 0.0005, (regB - 1.0 - pos) / fo);
            l[i] += g * bank.read (s0, len, pos, cut, false);
            r[i] += g * bank.read (s1, len, pos, cut, false);
            v.t += v.step;
        }
    }
}

void DeliaProcessor::startPreview (double fromFrac, bool regionOnly)
{
    previewFrom.store (fromFrac); previewRegion.store (regionOnly);
    if (auto* r = activeReel()) uiPreview.store ((float) (regionOnly ? r->regionStart : fromFrac));
    previewCmd.store (1);
}

void DeliaProcessor::renderPreview (float* l, float* r, int n)
{
    const Reel* a = audition.get();
    if (! pvOn || a == nullptr || ! a->audio) { if (! pvOn) uiPreview.store (-1.0f); return; }
    const auto& src = *a->audio; const int len = src.getNumSamples();
    const double step = a->sr / sr, regA = a->regionStart * len, regB = a->regionEnd * len;
    const double fi = juce::jmax (1.0, a->fadeIn * a->sr), fo = juce::jmax (1.0, a->fadeOut * a->sr);
    const auto& bank = SincBank::get(); const int cut = bank.cutFor (step);
    const float* s0 = src.getReadPointer (0); const float* s1 = src.getReadPointer (src.getNumChannels() > 1 ? 1 : 0);
    for (int i = 0; i < n; ++i)
    {
        if (pvPos >= pvEnd - 1.0) { pvOn = false; break; }
        pvGain += 0.004f * (pvTarget - pvGain);
        if (pvTarget == 0.0f && pvGain < 1e-3f) { pvOn = false; break; }
        float g = pvGain * 0.8f;
        if (pvRegion) g *= (float) juce::jlimit (0.0, 1.0, juce::jmin ((pvPos - regA) / fi + 0.0005, (regB - 1.0 - pvPos) / fo));
        l[i] += g * bank.read (s0, len, pvPos, cut, false);
        r[i] += g * bank.read (s1, len, pvPos, cut, false);
        pvPos += step;
    }
    uiPreview.store (pvOn ? (float) (pvPos / len) : -1.0f);
}

void DeliaProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals nd;
    const int total = buffer.getNumSamples();
    buffer.clear();

    {   // bucles y fuente de audicion nuevos
        juce::SpinLock::ScopedTryLockType tl (loopLock);
        if (tl.isLocked())
            for (int m = 0; m < kNumMachines; ++m)
                if (pendingFlag[m]) { engine->machines[m].setLoop (pending[m]); pendingFlag[m] = false; }
    }
    {
        juce::SpinLock::ScopedTryLockType tl (audLock);
        if (tl.isLocked() && auditionFlag) { audition = auditionPending; auditionFlag = false; for (auto& v : voices) v.on = false; pvOn = false; uiPreview.store (-1.0f); }
    }
    if (const int cmd = previewCmd.exchange (0); cmd == 1)
    {
        const Reel* a = audition.get();
        if (a != nullptr && a->audio && a->numSamples() > 1)
        {
            const double len = a->numSamples();
            pvRegion = previewRegion.load();
            pvPos = (pvRegion ? a->regionStart : juce::jlimit (0.0, 1.0, previewFrom.load())) * len;
            pvEnd = pvRegion ? a->regionEnd * len : len;
            pvGain = pvRegion ? 1.0f : 0.0f; pvTarget = 1.0f; pvOn = true;
        }
        else uiPreview.store (-1.0f);
    }
    else if (cmd == 2) pvTarget = 0.0f;

    Transport tr;
    if (auto* ph = getPlayHead())
        if (auto pos = ph->getPosition())
        {
            tr.playing = pos->getIsPlaying();
            if (auto t = pos->getTimeInSamples()) tr.timeSamples = (double) *t;
            if (auto b = pos->getBpm()) hostBpm.store (*b);
        }
    hostPlaying.store (tr.playing);

    auto noteOn = [this] (int note)
    {
        const Reel* a = audition.get(); if (a == nullptr) return;
        Voice* v = &voices[0];
        for (auto& c : voices) if (! c.on) { v = &c; break; }
        v->on = true; v->note = note; v->t = 0; v->gain = 0; v->target = 1;
        v->step = a->ratioFor (note - 60) * a->sr / sr;
    };
    auto noteOff = [this] (int note) { for (auto& v : voices) if (v.on && v.note == note) v.target = 0; };

    for (const auto meta : midi)
    {
        const auto msg = meta.getMessage();
        const int ch = msg.getChannel(), note = msg.getNoteNumber();
        if (msg.isNoteOn())
        {
            if (ch == 2) { if (note >= 60 && note < 60 + kNumGates) engine->gateHeld[note - 60].store (true); }
            else if (ch == 3) { if (note >= 60 && note < 60 + kNumMachines) runToggle[note - 60].fetch_add (1); }
            else
            {
                noteOn (note);
                int w = stepW.load(); stepFifo[(size_t) (w & 63)].store (note - 60); stepW.store (w + 1);
            }
        }
        else if (msg.isNoteOff())
        {
            if (ch == 2) { if (note >= 60 && note < 60 + kNumGates) engine->gateHeld[note - 60].store (false); }
            else if (ch != 3) noteOff (note);
        }
    }
    midi.clear();
    { int on = uiNoteOn.exchange (-1000); if (on > -1000) noteOn (on + 60); int off = uiNoteOff.exchange (-1000); if (off > -1000) noteOff (off + 60); }

    EngineParams ep; fillParams (ep);
    float* outL = buffer.getWritePointer (0); float* outR = buffer.getNumChannels() > 1 ? buffer.getWritePointer (1) : outL;
    float* aux[kNumMachines] {};
    for (int m = 0; m < kNumMachines; ++m)
        if (auto* bus = getBus (false, m + 1))
            if (bus->isEnabled() && bus->getNumberOfChannels() > 0)
                aux[m] = buffer.getWritePointer (bus->getChannelIndexInProcessBlockBuffer (0));

    int done = 0;
    while (done < total)
    {
        const int n = juce::jmin (kMaxBlock, total - done);
        float* ax[kNumMachines]; for (int m = 0; m < kNumMachines; ++m) ax[m] = aux[m] != nullptr ? aux[m] + done : nullptr;
        engine->render (ep, tr, n, outL + done, outR + done, ax);
        if (recState.load() == 1)
        {
            int p = recPos.load(); const int room = recBuf.getNumSamples() - p; const int k = juce::jmin (n, room);
            if (k > 0)
            {
                const float* a = recSrc == capLab ? engine->labBus : (recSrc == capBank ? engine->bankBus : outL + done);
                const float* b = recSrc == capLab ? engine->labBus : (recSrc == capBank ? engine->bankBus : outR + done);
                recBuf.copyFrom (0, p, a, k); recBuf.copyFrom (1, p, b, k);
                recPos.store (p + k);
            }
            if (k < n) recState.store (2);
        }
        renderVoices (outL + done, outR + done, n);
        renderPreview (outL + done, outR + done, n);
        tr.timeSamples += n;
        done += n;
    }
}

// ---------------------------------------------------------------- modelo
Reel* DeliaProcessor::reel (int id) { for (auto& r : reels) if (r->id == id) return r.get(); return nullptr; }

AudioPtr DeliaProcessor::loadAudio (const juce::File& f, bool stereo, double& fileSr)
{
    std::unique_ptr<juce::AudioFormatReader> rd (formats.createReaderFor (f));
    if (rd == nullptr || rd->lengthInSamples < 2) return {};
    const int len = (int) std::min<juce::int64> (rd->lengthInSamples, (juce::int64) (rd->sampleRate * 1200.0));
    juce::AudioBuffer<float> tmp ((int) juce::jmax (1u, rd->numChannels), len);
    rd->read (&tmp, 0, len, 0, true, true);
    fileSr = rd->sampleRate;
    auto out = std::make_shared<juce::AudioBuffer<float>>();
    if (stereo && tmp.getNumChannels() >= 2) { out->setSize (2, len); out->copyFrom (0, 0, tmp, 0, 0, len); out->copyFrom (1, 0, tmp, 1, 0, len); }
    else
    {
        out->setSize (1, len); out->clear();
        const float g = 1.0f / (float) tmp.getNumChannels();
        for (int c = 0; c < tmp.getNumChannels(); ++c) out->addFrom (0, 0, tmp, c, 0, len, g);
    }
    return out;
}

ReelPtr DeliaProcessor::addReelFromFile (const juce::File& f, bool stereo)
{
    double fsr = 48000.0;
    auto a = loadAudio (f, stereo, fsr);
    if (! a) { statusText = "No se pudo leer " + f.getFileName(); sendChangeMessage(); return {}; }
    auto r = std::make_shared<Reel>();
    r->id = nextReelId++; r->name = f.getFileNameWithoutExtension(); r->file = f; r->audio = a; r->sr = fsr;
    double hz = detectPitch (*r); if (hz > 20.0) r->rootHz = hz;
    reels.push_back (r);
    activeReelId = r->id;
    publishAudition();
    statusText = "Carrete cargado: " + r->name;
    sendChangeMessage();
    return r;
}

bool DeliaProcessor::relocateReel (int id, const juce::File& f)
{
    auto* r = reel (id); if (r == nullptr) return false;
    double fsr = 48000.0;
    auto a = loadAudio (f, r->isStereo() || stereoReels, fsr);
    if (! a) return false;
    r->audio = a; r->sr = fsr; r->file = f; r->missing = false;
    reelEdited (id);
    return true;
}

void DeliaProcessor::removeReel (int id)
{
    reels.erase (std::remove_if (reels.begin(), reels.end(), [id] (const ReelPtr& r) { return r->id == id; }), reels.end());
    if (activeReelId == id) activeReelId = reels.empty() ? -1 : reels.back()->id;
    publishAudition(); remountAll(); sendChangeMessage();
}

void DeliaProcessor::publishAudition()
{
    std::shared_ptr<const Reel> snap;
    if (auto* r = activeReel()) snap = std::make_shared<const Reel> (*r);
    juce::SpinLock::ScopedLockType sl (audLock);
    if (auditionPending) audGrave.push_back (auditionPending);
    auditionPending = snap; auditionFlag = true;
}

void DeliaProcessor::mount (int m)
{
    const bool loop = mp[m].loop->load() > 0.5f;
    auto l = renderStrip (strips[m], reels, tapeSpeedCm(), sr, cutErrorMm, seed + (uint32_t) m * 1000u, loop);
    juce::SpinLock::ScopedLockType sl (loopLock);
    if (mounted[m]) graveyard.push_back ({ mounted[m], 90 });
    mounted[m] = l; pending[m] = l; pendingFlag[m] = true;
}

void DeliaProcessor::reelEdited (int id)
{
    for (int m = 0; m < kNumMachines; ++m)
        for (auto& p : strips[m].pieces) if (p.reelId == id) { mount (m); break; }
    if (id == activeReelId) publishAudition();
    sendChangeMessage();
}
void DeliaProcessor::stripEdited (int m) { mount (m); sendChangeMessage(); }
void DeliaProcessor::remountAll() { for (int m = 0; m < kNumMachines; ++m) mount (m); sendChangeMessage(); }

bool DeliaProcessor::popStepNote (int& degree)
{
    int r = stepR.load(); if (r == stepW.load()) return false;
    degree = stepFifo[(size_t) (r & 63)].load(); stepR.store (r + 1); return true;
}

void DeliaProcessor::auditionNote (int degree, bool on) { (on ? uiNoteOn : uiNoteOff).store (degree); }

void DeliaProcessor::timerCallback()
{
    {
        juce::SpinLock::ScopedLockType sl (loopLock);
        for (auto& g : graveyard) --g.second;
        graveyard.erase (std::remove_if (graveyard.begin(), graveyard.end(), [] (const auto& g) { return g.second <= 0; }), graveyard.end());
    }
    { juce::SpinLock::ScopedLockType sl (audLock); if (! auditionFlag && audGrave.size() > 2) audGrave.erase (audGrave.begin(), audGrave.end() - 2); }
    for (int m = 0; m < kNumMachines; ++m)
        if (runToggle[m].exchange (0) % 2 == 1)
            setParam ("m" + juce::String (m + 1) + "_run", mp[m].run->load() > 0.5f ? 0.0f : 1.0f);
    if (recState.load() == 2) { if (++recCool >= 2) { recCool = 0; finishCapture(); } }
    // el modo bucle forma parte del montaje (union entre el ultimo trozo y el primero)
    for (int m = 0; m < kNumMachines; ++m)
    {
        const bool lp = mp[m].loop->load() > 0.5f;
        if (lp != lastLoopFlag[m]) { lastLoopFlag[m] = lp; mount (m); }
    }
    const int ts = (int) gp[gTapeSpeed]->load();
    if (ts != lastTapeSpeed) { lastTapeSpeed = ts; remountAll(); }
}

// ---------------------------------------------------------------- captura y volcado
void DeliaProcessor::startCapture (CaptureSource src)
{
    if (recState.load() != 0) return;
    recSrc = src;
    recBuf.setSize (2, (int) (sr * 180.0), false, true, false);
    recPos.store (0); recState.store (1);
    sendChangeMessage();
}
void DeliaProcessor::stopCapture() { if (recState.load() == 1) recState.store (2); }

ReelPtr DeliaProcessor::makeReelFromBuffer (juce::AudioBuffer<float>& buf, int num, const juce::String& baseName, int generation, bool tapePass)
{
    if (num < 16) return {};
    auto out = std::make_shared<juce::AudioBuffer<float>>();
    const bool stereo = stereoReels && buf.getNumChannels() > 1;
    out->setSize (stereo ? 2 : 1, num);
    if (stereo) { out->copyFrom (0, 0, buf, 0, 0, num); out->copyFrom (1, 0, buf, 1, 0, num); }
    else { out->copyFrom (0, 0, buf, 0, 0, num); if (buf.getNumChannels() > 1) { out->addFrom (0, 0, buf, 1, 0, num); out->applyGain (0.5f); } }
    const int id = nextReelId++;
    if (tapePass)
    {
        const float q = gp[gQuality]->load() * 0.01f;
        for (int c = 0; c < out->getNumChannels(); ++c)
        {
            TapePass tp; tp.prepare (sr, seed + (uint32_t) id * 31u + (uint32_t) c);
            tp.set (q, (float) tapeSpeedCm(), 1.0f);
            float* d = out->getWritePointer (c);
            for (int i = 0; i < num; ++i) d[i] = tp.tick (d[i]);
        }
    }
    auto r = std::make_shared<Reel>();
    r->id = id; r->generation = generation; r->sr = sr; r->audio = out;
    r->name = baseName + " " + juce::String (id);
    reelsFolder.createDirectory();
    auto f = reelsFolder.getNonexistentChildFile (baseName.replaceCharacter (' ', '_') + "_" + juce::String (id), ".wav");
    {
        juce::WavAudioFormat wav;
        std::unique_ptr<juce::OutputStream> os = std::make_unique<juce::FileOutputStream> (f);
        if (static_cast<juce::FileOutputStream*> (os.get())->openedOk())
        {
            auto opts = juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (out->getNumChannels()).withBitsPerSample (24);
            if (auto w = wav.createWriterFor (os, opts)) w->writeFromAudioSampleBuffer (*out, 0, num);
        }
    }
    if (f.existsAsFile()) { r->file = f; r->md5 = juce::MD5 (f).toHexString(); statusText = "Nuevo carrete: " + r->name + U8 (" (generaci\xc3\xb3n ") + juce::String (generation) + ")"; }
    else statusText = "Aviso: no se pudo escribir " + f.getFullPathName() + U8 ("; el carrete solo vive en esta sesi\xc3\xb3n");
    if (double hz = detectPitch (*r); hz > 20.0) r->rootHz = hz;
    reels.push_back (r);
    activeReelId = id;
    publishAudition();
    sendChangeMessage();
    return r;
}

void DeliaProcessor::finishCapture()
{
    const int num = recPos.load();
    int gen = 0;
    if (recSrc == capMix)
    {
        for (int m = 0; m < kNumMachines; ++m)
            if (mounted[m] && mounted[m]->audio.getNumSamples() > 0 && mp[m].run->load() > 0.5f) gen = juce::jmax (gen, mounted[m]->maxGeneration);
        gen += 1;
    }
    makeReelFromBuffer (recBuf, num, recSrc == capMix ? "Volcado" : (recSrc == capLab ? "Laboratorio" : "Osciladores"), gen, recSrc == capMix);
    recBuf.setSize (2, 16);
    recState.store (0);
    sendChangeMessage();
}

void DeliaProcessor::offlineBounce (double lengthCm)
{
    const int num = (int) juce::jlimit (16.0, sr * 600.0, lengthCm / tapeSpeedCm() * sr);
    auto eng = std::make_unique<Engine>();
    eng->prepare (sr, seed);
    EngineParams ep; fillParams (ep);
    int gen = 0; bool any = false;
    for (int m = 0; m < kNumMachines; ++m)
    {
        eng->machines[m].setLoop (mounted[m]);
        ep.m[m].sync = syncFree;
        if (ep.m[m].run && mounted[m] && mounted[m]->audio.getNumSamples() > 0) { gen = juce::jmax (gen, mounted[m]->maxGeneration); any = true; }
    }
    for (int g = 0; g < kNumGates; ++g) eng->gateHeld[g].store (engine->gateHeld[g].load());
    if (! any && ep.oscLevel <= 0 && ep.noiseLevel <= 0 && ep.wobLevel <= 0 && ! ep.bankOn)
    { statusText = U8 ("Nada que volcar: pon en marcha alguna m\xc3\xa1quina o fuente"); sendChangeMessage(); return; }
    ep.inertia = 0.0f;
    juce::AudioBuffer<float> buf (2, num);
    Transport tr; int done = 0;
    while (done < num)
    {
        const int n = juce::jmin (kMaxBlock, num - done);
        eng->render (ep, tr, n, buf.getWritePointer (0) + done, buf.getWritePointer (1) + done, nullptr);
        done += n;
    }
    makeReelFromBuffer (buf, num, "Volcado", gen + 1, true);
}

juce::File DeliaProcessor::exportStrip (int m)
{
    auto l = renderStrip (strips[m], reels, tapeSpeedCm(), sr, cutErrorMm, seed + (uint32_t) m * 1000u, false);
    if (l->audio.getNumSamples() < 2) return {};
    reelsFolder.createDirectory();
    auto f = reelsFolder.getNonexistentChildFile ("Tira_M" + juce::String (m + 1), ".wav");
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> os = std::make_unique<juce::FileOutputStream> (f);
    auto opts = juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (l->audio.getNumChannels()).withBitsPerSample (24);
    if (auto w = wav.createWriterFor (os, opts)) w->writeFromAudioSampleBuffer (l->audio, 0, l->audio.getNumSamples());
    statusText = "Tira exportada: " + f.getFullPathName(); sendChangeMessage();
    return f;
}

void DeliaProcessor::analyseToBank()
{
    auto* r = activeReel(); if (r == nullptr || ! r->audio) return;
    auto parts = analysePartials (*r, kNumOsc, juce::jmax (8.0, r->rootHz * 0.2));
    for (int o = 0; o < kNumOsc; ++o)
    {
        juce::String s (o + 1);
        const bool has = o < (int) parts.size() && parts[(size_t) o].amp > 0.002;
        setParam ("bFreq" + s, has ? (float) parts[(size_t) o].freq : 0.0f);
        setParam ("bLevel" + s, has ? (float) std::sqrt (parts[(size_t) o].amp) : 0.0f);
    }
    statusText = U8 ("An\xc3\xa1lisis: ") + juce::String ((int) parts.size()) + " parciales de " + r->name; sendChangeMessage();
}

// ---------------------------------------------------------------- estado
void DeliaProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    auto state = apvts.copyState();
    state.removeChild (state.getChildWithName ("DELIA"), nullptr);
    juce::ValueTree d ("DELIA");
    d.setProperty ("activeReel", activeReelId, nullptr); d.setProperty ("activeMachine", activeMachine, nullptr);
    d.setProperty ("bpm", manualBpm, nullptr); d.setProperty ("hostTempo", useHostTempo, nullptr);
    d.setProperty ("cutError", cutErrorMm, nullptr); d.setProperty ("seed", (int) seed, nullptr);
    d.setProperty ("stepInput", stepInput, nullptr); d.setProperty ("stereoReels", stereoReels, nullptr);
    d.setProperty ("drawer", drawerOpen, nullptr); d.setProperty ("corridor", corridor, nullptr);
    d.setProperty ("figure", figureIndex, nullptr); d.setProperty ("dotted", figureDotted, nullptr); d.setProperty ("triplet", figureTriplet, nullptr);
    d.setProperty ("folder", reelsFolder.getFullPathName(), nullptr); d.setProperty ("nextId", nextReelId, nullptr);
    for (auto& r : reels)
    {
        juce::ValueTree t ("REEL");
        t.setProperty ("id", r->id, nullptr); t.setProperty ("name", r->name, nullptr); t.setProperty ("path", r->file.getFullPathName(), nullptr);
        t.setProperty ("gen", r->generation, nullptr); t.setProperty ("md5", r->md5, nullptr); t.setProperty ("stereo", r->isStereo(), nullptr);
        t.setProperty ("rs", r->regionStart, nullptr); t.setProperty ("re", r->regionEnd, nullptr);
        t.setProperty ("fi", r->fadeIn, nullptr); t.setProperty ("fo", r->fadeOut, nullptr); t.setProperty ("root", r->rootHz, nullptr);
        t.setProperty ("scale", r->scaleType, nullptr); t.setProperty ("custom", r->custom.toText(), nullptr);
        t.setProperty ("lo", r->lowDeg, nullptr); t.setProperty ("hi", r->highDeg, nullptr);
        d.addChild (t, -1, nullptr);
    }
    for (int m = 0; m < kNumMachines; ++m)
    {
        juce::ValueTree s ("STRIP"); s.setProperty ("m", m, nullptr);
        for (auto& p : strips[m].pieces)
        {
            juce::ValueTree t ("PIECE");
            t.setProperty ("reel", p.reelId, nullptr); t.setProperty ("deg", p.degree, nullptr); t.setProperty ("off", p.offsetCm, nullptr);
            t.setProperty ("len", p.lengthCm, nullptr); t.setProperty ("cut", p.cut, nullptr); t.setProperty ("rev", p.reverse, nullptr);
            s.addChild (t, -1, nullptr);
        }
        d.addChild (s, -1, nullptr);
    }
    state.addChild (d, -1, nullptr);
    if (auto xml = state.createXml()) copyXmlToBinary (*xml, dest);
}

void DeliaProcessor::setStateInformation (const void* data, int size)
{
    auto xml = getXmlFromBinary (data, size);
    if (xml == nullptr) return;
    auto state = juce::ValueTree::fromXml (*xml);
    if (! state.isValid()) return;
    auto d = state.getChildWithName ("DELIA");
    state.removeChild (d, nullptr);
    apvts.replaceState (state);
    reels.clear(); for (auto& s : strips) s.pieces.clear();
    if (d.isValid())
    {
        activeReelId = d.getProperty ("activeReel", -1); activeMachine = juce::jlimit (0, kNumMachines - 1, (int) d.getProperty ("activeMachine", 0));
        manualBpm = d.getProperty ("bpm", 120.0); useHostTempo = d.getProperty ("hostTempo", true);
        cutErrorMm = d.getProperty ("cutError", 0.0); seed = (uint32_t) (int) d.getProperty ("seed", 1963);
        stepInput = d.getProperty ("stepInput", false); stereoReels = d.getProperty ("stereoReels", false);
        drawerOpen = d.getProperty ("drawer", false); corridor = d.getProperty ("corridor", false);
        figureIndex = d.getProperty ("figure", 2); figureDotted = d.getProperty ("dotted", false); figureTriplet = d.getProperty ("triplet", false);
        if (d.hasProperty ("folder")) reelsFolder = juce::File (d.getProperty ("folder").toString());
        nextReelId = d.getProperty ("nextId", 1);
        int missing = 0;
        for (auto t : d)
        {
            if (t.hasType ("REEL"))
            {
                auto r = std::make_shared<Reel>();
                r->id = t.getProperty ("id"); r->name = t.getProperty ("name").toString(); r->file = juce::File (t.getProperty ("path").toString());
                r->generation = t.getProperty ("gen", 0); r->md5 = t.getProperty ("md5").toString();
                r->regionStart = t.getProperty ("rs", 0.0); r->regionEnd = t.getProperty ("re", 1.0);
                r->fadeIn = t.getProperty ("fi", 0.002); r->fadeOut = t.getProperty ("fo", 0.01); r->rootHz = t.getProperty ("root", 220.0);
                r->scaleType = t.getProperty ("scale", 0); r->custom = Scale::fromText (t.getProperty ("custom").toString());
                r->lowDeg = t.getProperty ("lo", -12); r->highDeg = t.getProperty ("hi", 12);
                double fsr = 48000.0;
                r->audio = r->file.existsAsFile() ? loadAudio (r->file, (bool) t.getProperty ("stereo", false), fsr) : AudioPtr();
                r->sr = fsr; r->missing = ! r->audio; if (r->missing) ++missing;
                nextReelId = juce::jmax (nextReelId, r->id + 1);
                reels.push_back (r);
            }
            else if (t.hasType ("STRIP"))
            {
                const int m = juce::jlimit (0, kNumMachines - 1, (int) t.getProperty ("m", 0));
                for (auto pt : t)
                {
                    Piece p; p.reelId = pt.getProperty ("reel", -1); p.degree = pt.getProperty ("deg", 0); p.offsetCm = pt.getProperty ("off", 0.0);
                    p.lengthCm = pt.getProperty ("len", 19.0); p.cut = pt.getProperty ("cut", 1); p.reverse = pt.getProperty ("rev", false);
                    strips[m].pieces.push_back (p);
                }
            }
        }
        statusText = missing > 0 ? juce::String (missing) + " carrete(s) sin fichero: pulsa sobre ellos para reubicarlos" : juce::String();
    }
    for (int m = 0; m < kNumMachines; ++m) lastLoopFlag[m] = mp[m].loop->load() > 0.5f;
    lastTapeSpeed = (int) gp[gTapeSpeed]->load();
    publishAudition();
    remountAll();
}

juce::AudioProcessorEditor* DeliaProcessor::createEditor() { return new DeliaEditor (*this); }
} // namespace delia

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new delia::DeliaProcessor(); }
