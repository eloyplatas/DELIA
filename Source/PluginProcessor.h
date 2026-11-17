// Delia - procesador
#pragma once
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_formats/juce_audio_formats.h>
#include <juce_cryptography/juce_cryptography.h>
#include "Engine.h"

#define U8(s) juce::String::fromUTF8 (s)

namespace delia
{
enum CaptureSource { capMix = 0, capLab, capBank };

class DeliaProcessor : public juce::AudioProcessor, public juce::ChangeBroadcaster, private juce::Timer
{
public:
    DeliaProcessor();
    ~DeliaProcessor() override;

    // ---- AudioProcessor
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported (const BusesLayout&) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }
    const juce::String getName() const override { return "Delia"; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    double getTailLengthSeconds() const override { return 6.0; }
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    // ---- modelo (hilo de mensajes)
    juce::AudioProcessorValueTreeState apvts;
    std::vector<ReelPtr> reels;
    Strip strips[kNumMachines];
    int activeReelId = -1, activeMachine = 0;
    double manualBpm = 120.0; bool useHostTempo = true;
    double cutErrorMm = 0.0; uint32_t seed = 1963;
    bool stepInput = false, stereoReels = false, drawerOpen = false, corridor = false;
    int figureIndex = 2; bool figureDotted = false, figureTriplet = false;
    juce::File reelsFolder;
    juce::String statusText;

    Reel* reel (int id);
    Reel* activeReel() { return reel (activeReelId); }
    ReelPtr addReelFromFile (const juce::File&, bool stereo);
    void removeReel (int id);
    bool relocateReel (int id, const juce::File&);
    void reelEdited (int id);                 // makeup cambiado: remonta las tiras que lo usan
    void stripEdited (int machine);
    void remountAll();
    double tapeSpeedCm() const;
    double bpm() const { return useHostTempo && hostBpm.load() > 1.0 ? hostBpm.load() : manualBpm; }
    double figureCm() const;
    LoopPtr mountedLoop (int m) const { return mounted[m]; }

    // ---- captura y volcado
    void startCapture (CaptureSource src);
    void stopCapture();
    bool isCapturing() const { return recState.load() == 1; }
    int captureSource() const { return recSrc; }
    double captureSeconds() const { return recPos.load() / juce::jmax (1.0, sr); }
    void offlineBounce (double lengthCm);
    juce::File exportStrip (int machine);
    void analyseToBank();
    void setGate (int g, bool held) { engine->gateHeld[g].store (held); }

    // ---- preescucha del carrete activo a su velocidad original
    // regionOnly: de INICIO a FIN con los fundidos de Makeup (como suena la nota 0);
    // si no, desde fromFrac hasta el final del fichero, sin fundidos.
    void startPreview (double fromFrac, bool regionOnly);
    void stopPreview() { previewCmd.store (2); }
    bool isPreviewing() const { return uiPreview.load() >= 0.0f; }
    float previewFrac() const { return uiPreview.load(); }

    // ---- para la interfaz
    std::unique_ptr<Engine> engine;
    std::atomic<double> hostBpm { 0.0 };
    std::atomic<bool> hostPlaying { false };
    bool popStepNote (int& degree);
    void auditionNote (int degree, bool on);
    float param (const juce::String& id) const { return apvts.getRawParameterValue (id)->load(); }
    void setParam (const juce::String& id, float value);

private:
    double sr = 48000.0;
    juce::AudioFormatManager formats;
    int nextReelId = 1;

    // cambio de bucles hacia el hilo de audio
    juce::SpinLock loopLock;
    LoopPtr pending[kNumMachines]; bool pendingFlag[kNumMachines] {};
    LoopPtr mounted[kNumMachines];
    std::vector<std::pair<LoopPtr, int>> graveyard;

    // audicion por MIDI
    struct Voice { bool on = false; int note = 0; double t = 0, step = 1; float gain = 0, target = 0; };
    Voice voices[8];
    juce::SpinLock audLock;
    std::shared_ptr<const Reel> audition, auditionPending; bool auditionFlag = false;
    std::vector<std::shared_ptr<const Reel>> audGrave;
    std::atomic<int> uiNoteOn { -1000 }, uiNoteOff { -1000 };
    std::atomic<int> previewCmd { 0 }; std::atomic<double> previewFrom { 0.0 }; std::atomic<bool> previewRegion { true };
    std::atomic<float> uiPreview { -1.0f };
    bool pvOn = false, pvRegion = true; double pvPos = 0, pvEnd = 0; float pvGain = 0, pvTarget = 0;
    void renderPreview (float* l, float* r, int n);
    void publishAudition();
    void renderVoices (float* l, float* r, int n);

    // colas hacia el hilo de mensajes
    std::array<std::atomic<int>, 64> stepFifo; std::atomic<int> stepW { 0 }, stepR { 0 };
    std::atomic<int> runToggle[kNumMachines];

    // captura
    std::atomic<int> recState { 0 }, recPos { 0 };
    int recSrc = capMix, recCool = 0; juce::AudioBuffer<float> recBuf;
    bool lastLoopFlag[kNumMachines] { true, true, true, true }; int lastTapeSpeed = 2;
    void finishCapture();
    ReelPtr makeReelFromBuffer (juce::AudioBuffer<float>& buf, int numSamples, const juce::String& baseName, int generation, bool tapePass);

    // parametros
    struct MP { std::atomic<float>* speed, *run, *rev, *loop, *level, *pan, *hp, *lp, *slope, *echo, *plate, *sync, *ring; } mp[kNumMachines];
    std::atomic<float>* gp[64] {}; std::atomic<float>* bFreq[kNumOsc], *bLevel[kNumOsc], *bWave[kNumOsc], *bGate[kNumOsc], *gAtt[kNumGates], *gDec[kNumGates];
    void fillParams (EngineParams&) const;
    static juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

    AudioPtr loadAudio (const juce::File&, bool stereo, double& fileSr);
    void mount (int m);
    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DeliaProcessor)
};
} // namespace delia
