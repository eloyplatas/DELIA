// Captura de la interfaz de Delia sin anfitrion: carga un carrete de prueba, monta tiras y guarda PNG
#include "../Source/PluginProcessor.h"
#include "../Source/PluginEditor.h"
using namespace delia;

static juce::File makePluck (const juce::File& dir, const juce::String& name, double f0, bool bell)
{
    const double sr = 48000.0; juce::AudioBuffer<float> b (1, (int) (sr * 1.6));
    for (int i = 0; i < b.getNumSamples(); ++i)
    {
        double t = i / sr, v = 0;
        if (bell) for (auto p : { std::pair<double, double> { 1.0, 0.5 }, { 2.76, 0.35 }, { 5.4, 0.25 }, { 8.93, 0.15 } }) v += p.second * std::sin (kTwoPi * f0 * p.first * t) * std::exp (-t * (1.5 + p.first));
        else for (int h = 1; h <= 8; ++h) v += (0.6 / h) * std::sin (kTwoPi * f0 * h * t) * std::exp (-t * (1.2 + 0.6 * h));
        b.setSample (0, i, (float) (v * (1.0 - std::exp (-t * 900.0))));
    }
    auto f = dir.getChildFile (name + ".wav"); f.deleteFile();
    juce::WavAudioFormat wav; std::unique_ptr<juce::OutputStream> os = std::make_unique<juce::FileOutputStream> (f);
    if (auto w = wav.createWriterFor (os, juce::AudioFormatWriterOptions().withSampleRate (sr).withNumChannels (1).withBitsPerSample (24))) w->writeFromAudioSampleBuffer (b, 0, b.getNumSamples());
    return f;
}

struct Runner
{
    std::unique_ptr<DeliaProcessor> proc; std::unique_ptr<juce::AudioProcessorEditor> ed; juce::File outDir;
    juce::AudioBuffer<float> buf { 2, 512 }; juce::MidiBuffer midi; int r1 = 0, r2 = 0, stage = 0;

    void open() { ed.reset (proc->createEditor()); ed->setVisible (true); for (int i = 0; i < 20; ++i) proc->processBlock (buf, midi); }
    void save (const juce::String& name)
    {
        auto img = ed->createComponentSnapshot (ed->getLocalBounds(), true, 1.0f);
        auto f = outDir.getChildFile (name + ".png"); f.deleteFile();
        juce::FileOutputStream os (f); juce::PNGImageFormat().writeImageToStream (img, os);
        std::cout << "captura: " << f.getFullPathName() << " " << img.getWidth() << "x" << img.getHeight() << std::endl;
        ed.reset();
    }
    void next() { juce::Timer::callAfterDelay (500, [this] { step(); }); }
    void step()
    {
        switch (stage++)
        {
            case 0:
            {
                proc->activeMachine = 1; proc->activeReelId = r2; proc->reelEdited (r2);
                if (auto* rr = proc->activeReel()) { rr->regionStart = 0.05; rr->fadeIn = 0.03; proc->reelEdited (r2); }
                for (int b = 0; b < 4; ++b) proc->processBlock (buf, midi);
                // silencia las maquinas para medir solo la preescucha
                for (int m = 1; m <= 4; ++m) proc->setParam ("m" + juce::String (m) + "_run", 0);
                for (int b = 0; b < 200; ++b) proc->processBlock (buf, midi);
                proc->startPreview (0.30, false); double e1 = 0; float f0 = -2, f1 = -2;
                for (int b = 0; b < 30; ++b) { proc->processBlock (buf, midi); e1 += buf.getRMSLevel (0, 0, 512); if (b == 0) f0 = proc->previewFrac(); f1 = proc->previewFrac(); }
                std::cout << "preescucha desde 30 %: rms " << e1 / 30 << ", posicion " << f0 << " -> " << f1 << std::endl;
                proc->stopPreview(); for (int b = 0; b < 4; ++b) proc->processBlock (buf, midi);
                std::cout << "tras parar: previsualizando=" << proc->isPreviewing() << std::endl;
                proc->startPreview (0.0, true); proc->processBlock (buf, midi);
                std::cout << "preescucha de region: empieza en " << proc->previewFrac() << " (inicio 0,05)" << std::endl;
                for (int b = 0; b < 40; ++b) proc->processBlock (buf, midi);
                for (int m = 1; m <= 3; ++m) proc->setParam ("m" + juce::String (m) + "_run", 1);
                open(); next(); break;
            }
            case 1: save ("delia-principal"); proc->drawerOpen = true; proc->setParam ("bankOn", 1); proc->analyseToBank(); proc->setGate (0, true); proc->setGate (3, true); open(); next(); break;
            case 2: save ("delia-osciladores"); proc->drawerOpen = false; proc->corridor = true; open(); next(); break;
            case 3: save ("delia-pasillo"); proc->corridor = false; finish(); juce::MessageManager::getInstance()->stopDispatchLoop(); break;
            default: break;
        }
    }
    void start (const juce::String& args)
    {
        outDir = juce::File (args.isNotEmpty() ? args.unquoted() : "/tmp/delia-shots"); outDir.createDirectory();
        proc = std::make_unique<DeliaProcessor>();
        proc->reelsFolder = outDir.getChildFile ("carretes");
        proc->setPlayConfigDetails (0, 2, 48000.0, 512); proc->prepareToPlay (48000.0, 512);
        proc->addReelFromFile (makePluck (outDir, "Cuerda pulsada", 110.0, false), false);
        r1 = proc->activeReelId;
        proc->addReelFromFile (makePluck (outDir, "Pantalla verde", 523.0, true), false);
        r2 = proc->activeReelId;
        const int bass[] = { 0, 0, 12, 0, 3, 0, 12, 10 };
        for (int d : bass) { Piece p; p.reelId = r1; p.degree = d; p.lengthCm = 9.5; proc->strips[0].pieces.push_back (p); }
        const int mel[] = { 7, 12, 10, 7, 3, 5, 7 }; const double ml[] = { 19, 9.5, 9.5, 19, 9.5, 9.5, 28.5 };
        for (int i = 0; i < 7; ++i) { Piece p; p.reelId = r2; p.degree = mel[i] - 12; p.lengthCm = ml[i]; p.cut = i % 3; p.reverse = i == 4; proc->strips[1].pieces.push_back (p); }
        { Piece p; p.reelId = -1; p.lengthCm = 9.5; proc->strips[1].pieces.push_back (p); }
        for (int i = 0; i < 11; ++i) { Piece p; p.reelId = r2; p.degree = (i * 5) % 9 - 16; p.lengthCm = 6.9; proc->strips[2].pieces.push_back (p); }
        proc->remountAll();
        proc->setParam ("m1_run", 1); proc->setParam ("m2_run", 1); proc->setParam ("m3_run", 1); proc->setParam ("m2_echo", 0.4f); proc->setParam ("m3_plate", 0.5f);
        proc->setParam ("m3_speed", 0.5f); proc->setParam ("m2_pan", -0.4f); proc->setParam ("m3_pan", 0.5f);
        double rms = 0;
        for (int b = 0; b < 170; ++b) { proc->processBlock (buf, midi); rms += buf.getRMSLevel (0, 0, 512); }
        std::cout << "rms medio de salida: " << rms / 170 << std::endl;
        next();
    }
    void finish()
    {
        // volcado fuera de linea y estado
        proc->offlineBounce (152.0);
        std::cout << "carretes tras volcar: " << proc->reels.size() << ", generacion del ultimo: " << proc->reels.back()->generation << ", fichero: " << proc->reels.back()->file.getFullPathName() << std::endl;
        juce::MemoryBlock mb; proc->getStateInformation (mb);
        auto p2 = std::make_unique<DeliaProcessor>(); p2->setPlayConfigDetails (0, 2, 48000.0, 512); p2->prepareToPlay (48000.0, 512);
        p2->setStateInformation (mb.getData(), (int) mb.getSize());
        std::cout << "estado restaurado: " << p2->reels.size() << " carretes, " << p2->strips[0].pieces.size() << "/" << p2->strips[1].pieces.size() << "/" << p2->strips[2].pieces.size() << " trozos, m1_run=" << p2->param ("m1_run") << std::endl;
        juce::AudioBuffer<float> a (2, 512), c (2, 512); double diff = 0, lev = 0;
        auto p3 = std::make_unique<DeliaProcessor>(); p3->setPlayConfigDetails (0, 2, 48000.0, 512); p3->prepareToPlay (48000.0, 512); p3->setStateInformation (mb.getData(), (int) mb.getSize());
        for (int b = 0; b < 200; ++b) { p2->processBlock (a, midi); p3->processBlock (c, midi); for (int i = 0; i < 512; ++i) { diff = juce::jmax (diff, (double) std::abs (a.getSample (0, i) - c.getSample (0, i))); lev = juce::jmax (lev, (double) std::abs (a.getSample (0, i))); } }
        std::cout << "dos cargas del mismo estado: pico " << lev << ", diferencia maxima " << diff << std::endl;
    }
};

int main (int argc, char** argv)
{
    juce::ScopedJuceInitialiser_GUI init;
    Runner r; r.start (argc > 1 ? juce::String (argv[1]) : juce::String());
    juce::MessageManager::getInstance()->runDispatchLoop();
    return 0;
}
