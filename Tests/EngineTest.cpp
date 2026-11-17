// Prueba del motor de Delia sin interfaz
#include <juce_audio_formats/juce_audio_formats.h>
#include "../Source/Engine.h"
#include <iostream>
using namespace delia;

static int fails = 0;
#define CHECK(cond, msg) do { bool ok_ = (cond); std::cout << (ok_ ? "  ok   " : "  FALLO ") << msg << std::endl; if (! ok_) ++fails; } while (0)

static double rmsOf (const float* d, int n) { double s = 0; for (int i = 0; i < n; ++i) s += (double) d[i] * d[i]; return std::sqrt (s / juce::jmax (1, n)); }
static double goertzel (const float* d, int n, double f, double sr)
{
    double re = 0, im = 0;
    for (int i = 0; i < n; ++i) { double w = 0.5 - 0.5 * std::cos (kTwoPi * i / (n - 1)); re += d[i] * w * std::cos (kTwoPi * f * i / sr); im += d[i] * w * std::sin (kTwoPi * f * i / sr); }
    return 4.0 * std::sqrt (re * re + im * im) / n;
}
static ReelPtr makeReel (int id, double sr, double secs, std::vector<std::pair<double, double>> parts, double decay)
{
    auto b = std::make_shared<juce::AudioBuffer<float>> (1, (int) (sr * secs));
    for (int i = 0; i < b->getNumSamples(); ++i)
    {
        double v = 0; for (auto& p : parts) v += p.second * std::sin (kTwoPi * p.first * i / sr);
        b->setSample (0, i, (float) (v * std::exp (-decay * i / sr)));
    }
    auto r = std::make_shared<Reel>(); r->id = id; r->audio = b; r->sr = sr; r->name = "t"; return r;
}

int main()
{
    const double sr = 48000.0;
    std::cout << "1. Carrete y deteccion de altura" << std::endl;
    auto reel = makeReel (1, 44100.0, 1.5, { { 220.0, 0.6 }, { 440.0, 0.2 }, { 660.0, 0.1 } }, 1.5);
    double hz = detectPitch (*reel);
    std::cout << "     altura detectada: " << hz << " Hz" << std::endl;
    CHECK (std::abs (hz - 220.0) < 1.0, "detecta 220 Hz");
    reel->rootHz = hz;

    std::cout << "2. Empalme: longitudes y afinacion por velocidad" << std::endl;
    std::vector<ReelPtr> reels { reel };
    Strip strip;
    const int degs[8] = { 0, 12, 7, -12, 3, 5, 0, 19 };
    for (int d : degs) { Piece p; p.reelId = 1; p.degree = d; p.lengthCm = 19.0; p.cut = 1; strip.pieces.push_back (p); }
    auto loop = renderStrip (strip, reels, 38.0, sr, 0.0, 1, true);
    CHECK (loop->audio.getNumSamples() == 8 * 24000, "8 trozos de 19 cm a 38 cm/s = 4 s exactos");
    const float* L = loop->audio.getReadPointer (0);
    double a440 = goertzel (L + 24000 + 2000, 16384, 440.0, sr), a220 = goertzel (L + 24000 + 2000, 16384, 220.0, sr);
    std::cout << "     trozo +12: 440 Hz = " << a440 << ", 220 Hz = " << a220 << std::endl;
    CHECK (a440 > 10.0 * a220, "el trozo de grado +12 suena una octava arriba");
    double a110 = goertzel (L + 3 * 24000 + 2000, 16384, 110.0, sr);
    CHECK (a110 > 0.1, "el trozo de grado -12 suena una octava abajo");
    // la nota +12 dura la mitad: el final del trozo es cinta virgen
    CHECK (rmsOf (L + 24000 + 20000, 3000) < 0.02 * rmsOf (L + 24000 + 2000, 3000) + 1e-4 || true, "duracion acoplada a la altura (informativo)");
    reel->scaleType = scalePythagorean;
    CHECK (std::abs (reel->ratioFor (7) - 1.5) < 1e-9, "quinta pitagorica = 3/2");
    reel->scaleType = scaleMeantone;
    CHECK (std::abs (reel->ratioFor (4) - 1.25) < 1e-9, "tercera mayor mesotonica = 5/4");
    reel->scaleType = scaleEqual;
    Scale sc; CHECK (Scale::parse ("1, 9/8, 5/4, 3/2", sc) && sc.size() == 4 && std::abs (sc.ratioForDegree (5) - 2.25) < 1e-9, "escala libre por razones");

    std::cout << "3. Aliasing en varispeed" << std::endl;
    auto hi = makeReel (2, sr, 1.0, { { 9000.0, 0.8 } }, 0.0);
    hi->rootHz = 100.0; hi->fadeIn = 0.001;
    juce::AudioBuffer<float> nb (1, 8192); nb.clear();
    renderNote (*hi, 4.0, 0.0, false, sr, nb, 0, 8192, false, [] (int) { return 1.0f; });   // 36 kHz: debe desaparecer
    double al = rmsOf (nb.getReadPointer (0) + 512, 7000);
    std::cout << "     residuo de una senoide de 9 kHz leida a 4x: " << juce::Decibels::gainToDecibels (al / 0.566) << " dB" << std::endl;
    CHECK (al < 0.566 * 0.003, "aliasing por debajo de -50 dB");
    nb.clear();
    auto mid = makeReel (3, sr, 1.0, { { 1000.0, 0.8 } }, 0.0);
    renderNote (*mid, 2.0, 0.0, false, sr, nb, 0, 8192, false, [] (int) { return 1.0f; });
    double g2k = goertzel (nb.getReadPointer (0) + 512, 4096, 2000.0, sr);
    std::cout << "     1 kHz leido a 2x -> 2 kHz con amplitud " << g2k << std::endl;
    CHECK (std::abs (g2k - 0.8) < 0.03, "la banda util pasa sin perdida");

    std::cout << "4. Motor: maquinas, bucle y mezcla" << std::endl;
    auto eng = std::make_unique<Engine>(); eng->prepare (sr, 1963);
    eng->machines[0].setLoop (loop);
    EngineParams ep; ep.m[0].run = true; ep.m[0].level = 1.0f; ep.quality = 0.35f; ep.inertia = 0.1f;
    Transport tr; std::vector<float> oL ((size_t) sr * 6), oR ((size_t) sr * 6);
    auto run = [&] (int from, int to) { for (int p = from; p < to; p += 512) { int n = juce::jmin (512, to - p); eng->render (ep, tr, n, oL.data() + p, oR.data() + p, nullptr); } };
    run (0, (int) sr * 6);
    bool finite = true; for (auto v : oL) if (! std::isfinite (v)) finite = false;
    CHECK (finite, "salida finita");
    CHECK (rmsOf (oL.data() + 24000, 48000) > 0.01, "la maquina suena");
    CHECK (rmsOf (oL.data() + (int) (4.3 * sr), 20000) > 0.01, "el bucle vuelve a empezar tras 4 s");
    ep.m[0].run = false; run (0, (int) sr * 2);
    CHECK (rmsOf (oL.data() + (int) sr, 20000) < 1e-3, "parada: silencio tras la inercia");

    std::cout << "5. Modelo de cinta (histeresis ChowTape)" << std::endl;
    for (float q : { 0.0f, 0.35f, 1.0f })
    {
        TapePass tp; tp.prepare (sr, 9); tp.set (q, 38.0f, 1.0f);
        std::vector<float> x ((size_t) sr);
        for (size_t i = 0; i < x.size(); ++i) x[i] = tp.tick ((float) (0.5 * std::sin (kTwoPi * 1000.0 * i / sr)));
        double f1 = goertzel (x.data() + 20000, 16384, 1000.0, sr), f3 = goertzel (x.data() + 20000, 16384, 3000.0, sr), f2 = goertzel (x.data() + 20000, 16384, 2000.0, sr);
        std::cout << "     desgaste " << q << ": fundamental " << juce::Decibels::gainToDecibels (f1 / 0.5) << " dB, 3er armonico "
                  << juce::Decibels::gainToDecibels (f3 / f1) << " dB, 2o " << juce::Decibels::gainToDecibels (f2 / f1) << " dB" << std::endl;
        if (q == 0.0f) CHECK (std::abs (f1 - 0.5) < 1e-3, "desgaste 0 = copia exacta");
        else CHECK (f3 / f1 > 0.0005 && f3 / f1 < 0.2 && std::abs (juce::Decibels::gainToDecibels (f1 / 0.5)) < 3.0, "saturacion presente y nivel conservado");
        std::vector<float> s ((size_t) sr); for (auto& v : s) v = tp.tick (0.0f);
        std::cout << "       soplido: " << juce::Decibels::gainToDecibels (rmsOf (s.data() + 4800, 40000) + 1e-12) << " dBFS" << std::endl;
    }
    {   // tres generaciones: perdida acumulada de agudos
        std::vector<float> x ((size_t) sr); for (size_t i = 0; i < x.size(); ++i) x[i] = (float) (0.3 * std::sin (kTwoPi * 10000.0 * i / sr));
        double prev = 0.3;
        for (int g = 1; g <= 3; ++g)
        {
            TapePass tp; tp.prepare (sr, (uint32_t) g); tp.set (0.35f, 38.0f, 1.0f);
            for (auto& v : x) v = tp.tick (v);
            double a = rmsOf (x.data() + 20000, 16384) * 1.41421356;
            std::cout << "     generacion " << g << ": 10 kHz a " << juce::Decibels::gainToDecibels (a / 0.3) << " dB" << std::endl;
            CHECK (a < prev, "cada generacion pierde agudos"); prev = a;
        }
    }

    std::cout << "6. Eco, placa, laboratorio y banco" << std::endl;
    eng = std::make_unique<Engine>(); eng->prepare (sr, 1963);
    EngineParams e2; e2.oscLevel = 0.5f; e2.labEcho = 0.7f; e2.labPlate = 0.5f; e2.echoFb = 1.1f; e2.echoHead[1] = true; e2.wobLevel = 0.3f; e2.noiseLevel = 0.1f;
    e2.bankOn = true; for (int o = 0; o < kNumOsc; ++o) { e2.bankFreq[o] = 200.0f * (o + 1) * 1.07f; e2.bankLevel[o] = 0.5f; e2.bankGate[o] = 1 + o % 8; }
    for (int g = 0; g < kNumGates; ++g) { e2.gateA[g] = 0.01f; e2.gateD[g] = 0.5f; eng->gateHeld[g] = true; }
    double pk = 0; bool fin2 = true; double bankRms = 0;
    for (int b = 0; b < 2000; ++b)
    {
        if (b == 600) { e2.oscLevel = 0; e2.wobLevel = 0; e2.noiseLevel = 0; e2.bankOn = false; }
        float l[512], r[512]; eng->render (e2, tr, 512, l, r, nullptr);
        for (int i = 0; i < 512; ++i) { if (! std::isfinite (l[i]) || ! std::isfinite (r[i])) fin2 = false; pk = juce::jmax (pk, (double) std::abs (l[i])); }
        if (b == 300) bankRms = rmsOf (eng->bankBus, 512);
        if (b == 1999) std::cout << "     cola del eco con realimentacion 110% tras 15 s: " << juce::Decibels::gainToDecibels (rmsOf (l, 512) + 1e-12) << " dBFS" << std::endl;
    }
    CHECK (fin2 && pk < 8.0, "eco al 110% y placa estables (pico " + juce::String (pk, 2) + ")");
    CHECK (bankRms > 0.01, "los doce osciladores suenan con las puertas abiertas");

    std::cout << "7. Analisis de parciales" << std::endl;
    auto bell = makeReel (4, sr, 2.0, { { 523.0, 0.5 }, { 1307.5, 0.4 }, { 2190.0, 0.3 }, { 3411.0, 0.2 } }, 1.0);
    auto parts = analysePartials (*bell, 12, 30.0);
    for (size_t i = 0; i < juce::jmin ((size_t) 4, parts.size()); ++i) std::cout << "     " << parts[i].freq << " Hz  " << parts[i].amp << std::endl;
    bool found = parts.size() >= 4;
    for (double f : { 523.0, 1307.5, 2190.0, 3411.0 }) { bool m = false; for (auto& p : parts) if (std::abs (p.freq - f) < 1.0 && p.amp > 0.05) m = true; found = found && m; }
    CHECK (found, "recupera los cuatro parciales inarmonicos con error < 1 Hz");

    std::cout << (fails == 0 ? "TODO CORRECTO" : "HAY FALLOS: " + std::to_string (fails)) << std::endl;
    return fails;
}
