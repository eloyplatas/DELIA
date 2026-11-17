// Delia - editor: cuatro platinas, taller de empalme, laboratorio y cajon de doce osciladores
#include "PluginEditor.h"

namespace delia
{
namespace
{
using APVTS = juce::AudioProcessorValueTreeState;
const float kPiF = juce::MathConstants<float>::pi;

juce::String num (double v, int dec) { return juce::String (v, dec).replaceCharacter ('.', ','); }
double parseNum (const juce::String& s) { return s.replaceCharacter (',', '.').retainCharacters ("0123456789.-").getDoubleValue(); }

void setupField (juce::Label& l, juce::Component& parent, std::function<void (double)> onValue)
{
    l.setEditable (true); l.getProperties().set ("window", true); l.setJustificationType (juce::Justification::centred);
    l.onTextChange = [&l, onValue] { onValue (parseNum (l.getText())); };
    parent.addAndMakeVisible (l);
}

void drawTitle (juce::Graphics& g, juce::Rectangle<float> r, const juce::String& t)
{
    g.setGradientFill (juce::ColourGradient (col::brass.brighter (0.25f), r.getX(), r.getY(), col::brass.darker (0.25f), r.getX(), r.getBottom(), false));
    g.fillRoundedRectangle (r, 2.5f);
    g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawRoundedRectangle (r, 2.5f, 0.8f);
    g.setFont (uiFont (10.0f, true)); g.setColour (col::bakelite);
    g.drawText (t, r, juce::Justification::centred, false);
}

juce::Point<float> tangent (juce::Point<float> c, float r, juce::Point<float> p, float sign)
{
    const float d = juce::jmax (r + 0.01f, c.getDistanceFrom (p));
    const float a = std::atan2 (p.y - c.y, p.x - c.x) + sign * std::acos (r / d);
    return { c.x + std::cos (a) * r, c.y + std::sin (a) * r };
}

void drawReel (juce::Graphics& g, juce::Point<float> c, float R, float angle, float packR)
{
    g.setColour (juce::Colours::black.withAlpha (0.30f)); g.fillEllipse (c.x - R + 2, c.y - R + 4, R * 2, R * 2);
    // cinta bobinada
    g.setGradientFill (juce::ColourGradient (col::tapeLt, c.x - packR * 0.4f, c.y - packR * 0.5f, col::tape.darker (0.5f), c.x + packR, c.y + packR, true));
    g.fillEllipse (c.x - packR, c.y - packR, packR * 2, packR * 2);
    g.setColour (juce::Colours::black.withAlpha (0.12f));
    for (float rr = R * 0.36f; rr < packR; rr += 3.0f) g.drawEllipse (c.x - rr, c.y - rr, rr * 2, rr * 2, 0.5f);
    // brida de aluminio con tres ventanas
    juce::Path fl; fl.setUsingNonZeroWinding (false);
    fl.addEllipse (-R, -R, R * 2, R * 2);
    for (int i = 0; i < 3; ++i)
    {
        const float a = (float) i * kPiF * 2.0f / 3.0f;
        juce::Path w; w.addPieSegment (-R * 0.86f, -R * 0.86f, R * 1.72f, R * 1.72f, a + 0.32f, a + kPiF * 2.0f / 3.0f - 0.32f, 0.44f);
        fl.addPath (w.createPathWithRoundedCorners (5.0f));
    }
    auto tr = juce::AffineTransform::rotation (angle).translated (c.x, c.y);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xf2eef0ee), c.x - R * 0.7f, c.y - R * 0.8f, juce::Colour (0xf29a9e9b), c.x + R * 0.8f, c.y + R, false));
    g.fillPath (fl, tr);
    g.setColour (juce::Colours::black.withAlpha (0.35f)); g.strokePath (fl, juce::PathStrokeType (0.7f), tr);
    g.setColour (juce::Colours::white.withAlpha (0.5f)); g.drawEllipse (c.x - R + 1.5f, c.y - R + 1.5f, R * 2 - 3, R * 2 - 3, 0.8f);
    // cubo
    const float h = R * 0.30f;
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a4744), c.x - h, c.y - h, juce::Colour (0xff151413), c.x + h, c.y + h, false));
    g.fillEllipse (c.x - h, c.y - h, h * 2, h * 2);
    g.setColour (col::chrome);
    for (int i = 0; i < 3; ++i)
    {
        const float a = angle + (float) i * kPiF * 2.0f / 3.0f + 0.5f;
        g.fillEllipse (c.x + std::sin (a) * h * 0.58f - 2.0f, c.y - std::cos (a) * h * 0.58f - 2.0f, 4.0f, 4.0f);
    }
    g.setGradientFill (juce::ColourGradient (juce::Colours::white, c.x - 2, c.y - 3, juce::Colour (0xff7a7d7a), c.x + 3, c.y + 4, true));
    g.fillEllipse (c.x - 3.5f, c.y - 3.5f, 7.0f, 7.0f);
}

juce::Colour pieceColour (const Piece& p)
{
    if (p.reelId < 0) return col::leader;
    return col::tape.withRotatedHue (0.012f * (float) (p.reelId % 7)).withMultipliedBrightness (juce::jlimit (0.7f, 1.7f, 1.05f + 0.022f * (float) p.degree));
}

// Dibuja una tira empalmada. Devuelve nada; x0 = posicion del cm 0.
void drawStrip (juce::Graphics& g, juce::Rectangle<float> area, const Strip& strip, double pxPerCm, float x0, int selected, float playFrac, bool labels)
{
    g.saveState(); g.reduceClipRegion (area.toNearestInt());
    float x = x0; const float y = area.getY(), h = area.getHeight();
    for (int i = 0; i < (int) strip.pieces.size(); ++i)
    {
        const auto& p = strip.pieces[(size_t) i];
        const float w = (float) (p.lengthCm * pxPerCm);
        juce::Rectangle<float> r (x, y, w, h);
        if (r.getRight() > area.getX() && r.getX() < area.getRight())
        {
            auto c = pieceColour (p);
            g.setGradientFill (juce::ColourGradient (c.brighter (0.18f), x, y, c.darker (0.22f), x, y + h, false));
            g.fillRect (r);
            g.setColour (juce::Colours::white.withAlpha (0.10f)); g.fillRect (r.withHeight (h * 0.22f).translated (0, h * 0.12f));
            if (p.reverse) { g.setColour (juce::Colours::black.withAlpha (0.18f)); for (float xx = x + 4; xx < x + w - 2; xx += 7) g.drawLine (xx + 3, y + 3, xx, y + h - 3, 1.0f); }
            if (labels && w > 20)
            {
                g.setColour (p.reelId < 0 ? col::ink.withAlpha (0.6f) : col::ivory.withAlpha (0.95f)); g.setFont (uiFont (juce::jmin (11.0f, h * 0.36f), true));
                juce::String t = p.reelId < 0 ? juce::String ("virgen") : (p.degree > 0 ? "+" : "") + juce::String (p.degree);
                g.drawText (t, r.reduced (5, 1), juce::Justification::centredLeft, false);
            }
            if (i == selected) { g.setColour (col::brass.brighter (0.4f)); g.drawRect (r.reduced (0.5f), 2.0f); }
        }
        // cinta de empalme sobre la union
        const float sw = juce::jmax (5.0f, juce::jmin (9.0f, w * 0.4f));
        g.setColour (juce::Colours::white.withAlpha (0.55f)); g.fillRect (x - sw * 0.5f, y + 1.0f, sw, h - 2.0f);
        g.setColour (col::ink.withAlpha (0.8f));
        const float lean = p.cut == 0 ? 0.0f : (p.cut == 1 ? sw * 0.45f : sw * 0.25f);
        g.drawLine (x - lean, y + h - 1, x + lean, y + 1, 1.0f);
        x += w;
    }
    if (playFrac >= 0.0f && x > x0)
    {
        const float px = x0 + (x - x0) * playFrac;
        g.setColour (col::lampOn); g.drawLine (px, y - 2, px, y + h + 2, 1.6f);
    }
    g.restoreState();
}
} // namespace

// =================================================================== cabecera
class HeaderBar : public juce::Component
{
public:
    HeaderBar (DeliaEditor& e) : ed (e), proc (e.proc),
        tape (proc.apvts, "tapeSpeed", "CINTA cm/s", "chicken", col::ivory), wear (proc.apvts, "quality", "DESGASTE", {}, col::ivory),
        drift (proc.apvts, "drift", "DERIVA", {}, col::ivory), inertia (proc.apvts, "inertia", "INERCIA", {}, col::ivory),
        master (proc.apvts, "master", "SALIDA", {}, col::ivory)
    {
        for (auto* k : { &tape, &wear, &drift, &inertia, &master }) addAndMakeVisible (k);
        for (auto* t : { &corridorSw, &drawerSw })
        {
            t->setColour (juce::ToggleButton::textColourId, col::ivory); addAndMakeVisible (t);
        }
        corridorSw.setButtonText ("PASILLO"); drawerSw.setButtonText ("12 OSCILADORES");
        corridorSw.setTooltip (U8 ("Extiende las cuatro tiras a escala para ver qu\xc3\xa9 trozo desajusta"));
        drawerSw.setTooltip (U8 ("Despliega el banco de doce osciladores y ocho puertas"));
        corridorSw.onClick = [this] { proc.corridor = corridorSw.getToggleState(); ed.refreshAll(); };
        drawerSw.onClick = [this] { proc.drawerOpen = drawerSw.getToggleState(); ed.layoutForDrawer(); };
        status.setColour (juce::Label::textColourId, col::ivory.withAlpha (0.85f)); status.setJustificationType (juce::Justification::centredLeft);
        addAndMakeVisible (status);
    }
    void refresh()
    {
        corridorSw.setToggleState (proc.corridor, juce::dontSendNotification); drawerSw.setToggleState (proc.drawerOpen, juce::dontSendNotification);
        status.setText (proc.statusText, juce::dontSendNotification);
    }
    void tick()
    {
        for (int c = 0; c < 2; ++c) vu[c] += 0.35f * (vuMap (proc.engine->masterPeak[c].load()) - vu[c]);
        repaint (vuArea.toNearestInt());
    }
    void resized() override
    {
        int x = 250; const int kw = 46;
        for (auto* k : { &tape, &wear, &drift, &inertia, &master }) { k->setBounds (x, 5, kw, 55); x += kw + 6; }
        vuArea = juce::Rectangle<float> ((float) x + 6, 10.0f, 150.0f, 44.0f);
        corridorSw.setBounds (x + 170, 8, 92, 22); drawerSw.setBounds (x + 170, 34, 140, 22);
        status.setBounds (x + 320, 20, getWidth() - x - 330, 24);
    }
    void paint (juce::Graphics& g) override
    {
        // placa de laton con el nombre
        juce::Rectangle<float> p (12.0f, 9.0f, 222.0f, 46.0f);
        g.setColour (juce::Colours::black.withAlpha (0.4f)); g.fillRoundedRectangle (p.translated (0, 2), 4.0f);
        g.setGradientFill (juce::ColourGradient (col::brass.brighter (0.35f), p.getX(), p.getY(), col::brass.darker (0.3f), p.getRight(), p.getBottom(), false));
        g.fillRoundedRectangle (p, 4.0f);
        g.setColour (col::bakelite.withAlpha (0.7f)); g.drawRoundedRectangle (p.reduced (3.0f), 2.5f, 0.8f);
        drawScrew (g, p.getX() + 9, p.getCentreY(), 2.6f, 0.7f); drawScrew (g, p.getRight() - 9, p.getCentreY(), 2.6f, 2.0f);
        g.setColour (juce::Colours::white.withAlpha (0.35f)); g.setFont (serifFont (31.0f, juce::Font::bold | juce::Font::italic));
        g.drawText ("Delia", p.withTrimmedBottom (9).translated (0, 1), juce::Justification::centred, false);
        g.setColour (col::bakelite); g.drawText ("Delia", p.withTrimmedBottom (9), juce::Justification::centred, false);
        g.setFont (uiFont (7.5f, true)); g.drawText ("TALLER  DE  CINTA", p.withTrimmedTop (33), juce::Justification::centred, false);
        drawVU (g, vuArea.withWidth (72), vu[0], "IZQ"); drawVU (g, vuArea.withTrimmedLeft (78), vu[1], "DCHA");
    }
private:
    DeliaEditor& ed; DeliaProcessor& proc;
    Knob tape, wear, drift, inertia, master;
    juce::ToggleButton corridorSw, drawerSw; juce::Label status;
    juce::Rectangle<float> vuArea; float vu[2] { 0, 0 };
};

// =================================================================== platina
class DeckView : public juce::Component
{
public:
    DeckView (DeliaProcessor& p, int m) : proc (p), index (m)
    {
        const juce::String pre = "m" + juce::String (m + 1) + "_";
        auto addKnob = [&] (const char* id, const juce::String& label, const juce::String& style = {}, int ticks = -1)
        {
            auto* k = knobs.add (new Knob (proc.apvts, pre + id, label, style));
            if (ticks > 0) k->slider.getProperties().set ("ticks", ticks);
            addAndMakeVisible (k); return k;
        };
        speed = addKnob ("speed", "VARIADOR", "big", 8);
        addKnob ("level", "NIVEL"); addKnob ("pan", "PAN"); addKnob ("hp", "GRAVES", {}, 7); addKnob ("lp", "AGUDOS", {}, 8);
        addKnob ("echo", "ECO"); addKnob ("plate", "PLACA"); addKnob ("sync", "SINCRO", "chicken"); addKnob ("ring", "ANILLO", "chicken");
        auto addKey = [&] (const char* id, const juce::String& text, const juce::String& tip, bool red = false)
        {
            auto* t = keys.add (new ParamToggle (proc.apvts, pre + id, text, "key", red)); t->setTooltip (tip); addAndMakeVisible (t);
        };
        addKey ("rev", U8 ("\xe2\x97\x80"), U8 ("Marcha atr\xc3\xa1s"));
        addKey ("run", U8 ("\xe2\x96\xb6"), "Marcha / paro", true);
        addKey ("loop", U8 ("\xe2\x88\x9e"), "Bucle (apagado: una sola pasada)");
        slope = std::make_unique<ParamToggle> (proc.apvts, pre + "slope", "12 dB/oct"); addAndMakeVisible (*slope);
        speed->slider.setTooltip (U8 ("Velocidad de la m\xc3\xa1quina: 0,25x a 4x. Cambia altura y tempo a la vez"));
        setInterceptsMouseClicks (true, true);
    }

    void tick()
    {
        auto& mc = proc.engine->machines[index];
        const float sp = mc.uiSpeed.load(), pos = mc.uiPos.load();
        const bool has = proc.mountedLoop (index) && proc.mountedLoop (index)->audio.getNumSamples() > 1;
        packL = has ? std::sqrt (1.0f - pos * 0.85f) : 0.95f; packR = has ? std::sqrt (0.10f + pos * 0.85f) : 0.30f;
        angL += sp * 0.20f / juce::jmax (0.3f, packL); angR += sp * 0.20f / juce::jmax (0.3f, packR);
        vu += 0.4f * (vuMap (mc.uiPeak.load()) - vu);
        running = std::abs (sp) > 0.01f; cm = has ? pos * (float) proc.strips[index].totalCm() : 0.0f;
        repaint (0, 0, getWidth(), topH); repaint (vuArea.toNearestInt());
    }

    void resized() override
    {
        const int y1 = topH + 8;
        speed->setBounds (8, y1, 66, 77);
        for (int i = 0; i < 3; ++i) keys[i]->setBounds (84 + i * 43, y1 + 4, 41, 44);
        vuArea = juce::Rectangle<float> (218.0f, (float) y1 + 2.0f, (float) getWidth() - 228.0f, 44.0f);
        slope->setBounds (218, y1 + 50, getWidth() - 224, 20);
        const int y2 = y1 + 86; const float step = (getWidth() - 12) / 8.0f;
        for (int i = 1; i < knobs.size(); ++i) knobs[i]->setBounds ((int) (6 + (i - 1) * step), y2, (int) step - 1, 52);
    }

    void mouseDown (const juce::MouseEvent&) override { proc.activeMachine = index; proc.sendChangeMessage(); }

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const bool active = proc.activeMachine == index;
        // chapa del transporte
        juce::Rectangle<float> top (0, 0, w, (float) topH);
        drawPlate (g, top.reduced (1.0f), juce::Colour (0xffc7cbb9));
        const juce::Point<float> cL (64.0f, 86.0f), cR (w - 64.0f, 86.0f); const float R = 55.0f;
        const float pl = R * (0.36f + 0.56f * packL), pr = R * (0.36f + 0.56f * packR);
        const juce::Point<float> gL (26.0f, 150.0f), gR (w - 26.0f, 150.0f);
        const float ty = 158.0f;
        // recorrido de la cinta
        juce::Path tp;
        auto a = tangent (cL, pl, gL, 1.0f); tp.startNewSubPath (a); tp.lineTo (gL.x - 4.5f, gL.y); tp.lineTo (gL.x, ty);
        tp.lineTo (gR.x, ty); tp.lineTo (gR.x + 4.5f, gR.y); tp.lineTo (tangent (cR, pr, gR, -1.0f));
        g.setColour (juce::Colours::black.withAlpha (0.25f)); g.strokePath (tp, juce::PathStrokeType (2.6f), juce::AffineTransform::translation (1.0f, 2.0f));
        g.setColour (col::tape.darker (0.3f)); g.strokePath (tp, juce::PathStrokeType (2.4f, juce::PathStrokeType::curved));
        drawReel (g, cL, R, angL, pl); drawReel (g, cR, R, angR, pr);
        // guias, cabestrante y rodillo
        for (auto p : { gL, gR })
        {
            g.setGradientFill (juce::ColourGradient (juce::Colours::white, p.x - 2, p.y - 3, juce::Colour (0xff6e716e), p.x + 4, p.y + 5, true));
            g.fillEllipse (p.x - 5, p.y - 5, 10, 10); g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawEllipse (p.x - 5, p.y - 5, 10, 10, 0.7f);
        }
        const float hx = w * 0.5f;
        g.setColour (juce::Colour (0xff1c1a18)); g.fillEllipse (hx + 58, ty - 20, 18, 18);
        g.setColour (juce::Colours::white.withAlpha (0.2f)); g.drawEllipse (hx + 58, ty - 20, 18, 18, 0.8f);
        g.setColour (col::chrome); g.fillEllipse (hx + 63.5f, ty + 1.5f, 6, 6);
        // bloque de cabezas
        juce::Rectangle<float> hb (hx - 50, ty - 22, 100, 30);
        g.setColour (juce::Colours::black.withAlpha (0.35f)); g.fillRoundedRectangle (hb.translated (0, 2), 5.0f);
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4f5753), hb.getX(), hb.getY(), juce::Colour (0xff2a302d), hb.getX(), hb.getBottom(), false));
        g.fillRoundedRectangle (hb, 5.0f);
        g.setColour (juce::Colours::white.withAlpha (0.25f)); g.drawRoundedRectangle (hb.reduced (0.6f), 5.0f, 0.8f);
        const char* hn[3] = { "B", "G", "R" };
        for (int i = 0; i < 3; ++i)
        {
            juce::Rectangle<float> hd (hb.getX() + 12 + i * 29.0f, hb.getBottom() - 14, 18, 12);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffeceeea), hd.getX(), hd.getY(), juce::Colour (0xff8a8d8a), hd.getRight(), hd.getBottom(), false));
            g.fillRoundedRectangle (hd, 2.0f); g.setColour (col::ink); g.drawLine (hd.getCentreX(), hd.getY() + 2, hd.getCentreX(), hd.getBottom() - 2, 0.8f);
            g.setFont (uiFont (7.0f, true)); g.setColour (col::ivory.withAlpha (0.7f)); g.drawText (hn[i], hd.translated (0, -11), juce::Justification::centred, false);
        }
        // contador mecanico en cm
        juce::Rectangle<float> cn (hx - 34, 10, 68, 17);
        g.setColour (juce::Colour (0xff0d0c0b)); g.fillRoundedRectangle (cn.expanded (2), 2.5f);
        const juce::String digits = juce::String ((int) cm).paddedLeft ('0', 5);
        for (int i = 0; i < 5; ++i)
        {
            juce::Rectangle<float> d (cn.getX() + i * 13.6f, cn.getY(), 12.6f, cn.getHeight());
            g.setGradientFill (juce::ColourGradient (i == 4 ? juce::Colour (0xff8c2a22) : juce::Colour (0xff2a2826), d.getX(), d.getY(), juce::Colour (0xff0a0a0a), d.getX(), d.getBottom() + 6, false));
            g.fillRect (d); g.setColour (col::ivory); g.setFont (uiFont (12.0f, true)); g.drawText (digits.substring (i, i + 1), d, juce::Justification::centred, false);
        }
        g.setFont (uiFont (7.0f, true)); g.setColour (col::ink.withAlpha (0.8f)); g.drawText ("cm", cn.translated (0, 17), juce::Justification::centred, false);
        // placa de identificacion y piloto
        juce::Rectangle<float> np (hx - 29, 44, 58, 14);
        g.setGradientFill (juce::ColourGradient ((active ? col::brass.brighter (0.5f) : col::brass.darker (0.15f)), np.getX(), np.getY(), col::brass.darker (active ? 0.1f : 0.5f), np.getX(), np.getBottom(), false));
        g.fillRoundedRectangle (np, 2.0f); g.setColour (col::bakelite); g.drawRoundedRectangle (np, 2.0f, 0.7f);
        g.setFont (uiFont (8.0f, true)); g.drawText (U8 ("M\xc3\x81QUINA ") + juce::String (index + 1), np, juce::Justification::centred, false);
        drawLamp (g, hx, 78.0f, 5.0f, running);
        // panel de mandos
        juce::Rectangle<float> ctl (0, (float) topH + 2, w, (float) getHeight() - topH - 2);
        drawPlate (g, ctl.reduced (1.0f), col::cream);
        drawVU (g, vuArea, vu);
        if (active) { g.setColour (col::brass.brighter (0.3f)); g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.8f), 5.0f, 1.6f); }
    }

private:
    DeliaProcessor& proc; int index; static constexpr int topH = 172;
    juce::OwnedArray<Knob> knobs; juce::OwnedArray<ParamToggle> keys; std::unique_ptr<ParamToggle> slope; Knob* speed = nullptr;
    juce::Rectangle<float> vuArea; float angL = 0, angR = 1, packL = 0.9f, packR = 0.3f, vu = 0, cm = 0; bool running = false;
};

// =================================================================== carretes y makeup
class ReelPanel : public juce::Component
{
public:
    ReelPanel (DeliaEditor& e) : ed (e), proc (e.proc), list (*this), wave (*this)
    {
        addAndMakeVisible (list); addAndMakeVisible (wave);
        for (auto* b : { &loadB, &removeB, &folderB, &detectB, &sclB }) addAndMakeVisible (b);
        loadB.setButtonText ("CARGAR..."); removeB.setButtonText ("QUITAR"); folderB.setButtonText ("CARPETA..."); detectB.setButtonText ("DETECTAR"); sclB.setButtonText (".SCL...");
        stereoSw.setButtonText (U8 ("EST\xc3\x89REO")); stereoSw.setColour (juce::ToggleButton::textColourId, col::ink); addAndMakeVisible (stereoSw);
        stereoSw.setTooltip (U8 ("Los carretes nuevos (cargados o volcados) conservan los dos canales. Apagado: mono"));
        stereoSw.onClick = [this] { proc.stereoReels = stereoSw.getToggleState(); };
        folderB.setTooltip (U8 ("Carpeta donde se escriben los carretes volcados y capturados"));
        loadB.onClick = [this]
        {
            chooser = std::make_unique<juce::FileChooser> ("Cargar audio como carrete", juce::File(), "*.wav;*.aif;*.aiff;*.flac;*.ogg;*.mp3");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::canSelectMultipleItems,
                                  [this] (const juce::FileChooser& fc) { for (auto& f : fc.getResults()) proc.addReelFromFile (f, proc.stereoReels); });
        };
        removeB.onClick = [this] { if (proc.activeReelId >= 0) proc.removeReel (proc.activeReelId); };
        folderB.onClick = [this]
        {
            chooser = std::make_unique<juce::FileChooser> ("Carpeta de carretes", proc.reelsFolder);
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectDirectories,
                                  [this] (const juce::FileChooser& fc) { if (fc.getResult() != juce::File()) { proc.reelsFolder = fc.getResult(); proc.statusText = "Carpeta de carretes: " + proc.reelsFolder.getFullPathName(); proc.sendChangeMessage(); } });
        };
        detectB.onClick = [this] { if (auto* r = proc.activeReel()) { double hz = detectPitch (*r); if (hz > 20.0) { r->rootHz = hz; proc.reelEdited (r->id); } else { proc.statusText = U8 ("No se detecta una altura clara: escribe la ra\xc3\xadz a mano"); proc.sendChangeMessage(); } } };
        sclB.onClick = [this]
        {
            chooser = std::make_unique<juce::FileChooser> ("Cargar escala Scala", juce::File(), "*.scl");
            chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& fc)
            {
                auto* r = proc.activeReel(); if (r == nullptr || ! fc.getResult().existsAsFile()) return;
                Scale s; if (Scale::parse (fc.getResult().loadFileAsString(), s)) { r->custom = s; r->scaleType = scaleCustom; proc.reelEdited (r->id); }
            });
        };
        scaleBox.addItem ("Temperada (12)", 1); scaleBox.addItem (U8 ("Pitag\xc3\xb3rica"), 2); scaleBox.addItem (U8 ("Mesot\xc3\xb3nica 1/4 coma"), 3); scaleBox.addItem ("Libre", 4);
        scaleBox.onChange = [this] { if (auto* r = proc.activeReel()) { r->scaleType = scaleBox.getSelectedId() - 1; proc.reelEdited (r->id); } };
        addAndMakeVisible (scaleBox);
        auto edit = [this] (std::function<void (Reel&, double)> fn) { return [this, fn] (double v) { if (auto* r = proc.activeReel()) { fn (*r, v); proc.reelEdited (r->id); } }; };
        setupField (rootF, *this, edit ([] (Reel& r, double v) { r.rootHz = juce::jlimit (10.0, 20000.0, v); }));
        setupField (fadeInF, *this, edit ([] (Reel& r, double v) { r.fadeIn = juce::jlimit (0.0, 20000.0, v) * 0.001; }));
        setupField (fadeOutF, *this, edit ([] (Reel& r, double v) { r.fadeOut = juce::jlimit (0.0, 20000.0, v) * 0.001; }));
        setupField (lowF, *this, edit ([] (Reel& r, double v) { r.lowDeg = juce::jlimit (-48, 0, (int) v); }));
        setupField (highF, *this, edit ([] (Reel& r, double v) { r.highDeg = juce::jlimit (0, 48, (int) v); }));
        ratiosF.setEditable (true); ratiosF.getProperties().set ("window", true); ratiosF.setJustificationType (juce::Justification::centredLeft);
        ratiosF.setTooltip (U8 ("Escala libre: razones separadas por espacios, por ejemplo 1 9/8 5/4 3/2 5/3"));
        ratiosF.onTextChange = [this] { if (auto* r = proc.activeReel()) { Scale s; if (Scale::parse (ratiosF.getText(), s)) { r->custom = s; r->scaleType = scaleCustom; proc.reelEdited (r->id); } } };
        addAndMakeVisible (ratiosF);
        auto secs = [] (Reel& r) { return juce::jmax (1e-6, r.numSamples() / r.sr); };
        setupField (startF, *this, edit ([secs] (Reel& r, double v) { r.regionStart = juce::jlimit (0.0, r.regionEnd - 0.002, v / secs (r)); }));
        setupField (endF, *this, edit ([secs] (Reel& r, double v) { r.regionEnd = juce::jlimit (r.regionStart + 0.002, 1.0, v / secs (r)); }));
        startF.setTooltip (U8 ("Punto de inicio de lectura del carrete, en segundos. Las notas empiezan a leer aqu\xc3\xad"));
        endF.setTooltip (U8 ("Punto final de lectura, en segundos"));
        previewB.setButtonText (U8 ("\xe2\x96\xb6 ESCUCHAR")); previewB.getProperties().set ("red", true);
        previewB.setTooltip (U8 ("Preescucha la regi\xc3\xb3n desde el punto de inicio, a velocidad original y con los fundidos (as\xc3\xad suena la nota 0)"));
        previewB.onClick = [this] { if (proc.isPreviewing()) proc.stopPreview(); else if (proc.activeReel() != nullptr) proc.startPreview (0.0, true); };
        addAndMakeVisible (previewB);
    }

    void tick()
    {
        const bool pv = proc.isPreviewing();
        if (pv != wasPreview) { wasPreview = pv; previewB.setButtonText (pv ? U8 ("\xe2\x96\xa0 PARAR") : U8 ("\xe2\x96\xb6 ESCUCHAR")); previewB.setToggleState (pv, juce::dontSendNotification); wave.repaint(); }
        if (pv) wave.repaint();
    }

    void refresh()
    {
        stereoSw.setToggleState (proc.stereoReels, juce::dontSendNotification);
        auto* r = proc.activeReel();
        const bool on = r != nullptr;
        for (juce::Component* c : std::initializer_list<juce::Component*> { &rootF, &fadeInF, &fadeOutF, &lowF, &highF, &scaleBox, &detectB, &sclB, &ratiosF, &startF, &endF, &previewB }) c->setEnabled (on);
        if (on)
        {
            rootF.setText (num (r->rootHz, 2), juce::dontSendNotification);
            const double secs = r->numSamples() / juce::jmax (1.0, r->sr);
            startF.setText (num (r->regionStart * secs, 3), juce::dontSendNotification); endF.setText (num (r->regionEnd * secs, 3), juce::dontSendNotification);
            fadeInF.setText (num (r->fadeIn * 1000.0, 1), juce::dontSendNotification); fadeOutF.setText (num (r->fadeOut * 1000.0, 1), juce::dontSendNotification);
            lowF.setText (juce::String (r->lowDeg), juce::dontSendNotification); highF.setText (juce::String (r->highDeg), juce::dontSendNotification);
            scaleBox.setSelectedId (r->scaleType + 1, juce::dontSendNotification);
            juce::StringArray rs; for (auto v : r->custom.ratios) rs.add (juce::String (v, 4));
            ratiosF.setText (rs.joinIntoString (" "), juce::dontSendNotification);
        }
        ratiosF.setVisible (on && r->scaleType == scaleCustom);
        wave.rebuild(); list.repaint(); repaint();
    }

    void resized() override
    {
        list.setBounds (10, 26, getWidth() - 20, 92);
        int x = 10; const int by = 122;
        loadB.setBounds (x, by, 74, 22); x += 78; removeB.setBounds (x, by, 58, 22); x += 62; folderB.setBounds (x, by, 78, 22); x += 84; stereoSw.setBounds (x, by, 86, 22);
        previewB.setBounds (getWidth() - 116, 149, 106, 21);
        wave.setBounds (10, 174, getWidth() - 20, 84);
        const int ay = 274;
        startF.setBounds (10, ay, 70, 20); endF.setBounds (88, ay, 70, 20); fadeInF.setBounds (172, ay, 64, 20); fadeOutF.setBounds (246, ay, 64, 20);
        const int fy = 314;
        rootF.setBounds (10, fy, 70, 20); detectB.setBounds (84, fy, 70, 20); sclB.setBounds (164, fy, 52, 20); lowF.setBounds (228, fy, 40, 20); highF.setBounds (274, fy, 40, 20);
        const int sy = 354;
        scaleBox.setBounds (10, sy, 150, 21); ratiosF.setBounds (166, sy, getWidth() - 176, 21);
    }

    void paint (juce::Graphics& g) override
    {
        drawPlate (g, getLocalBounds().toFloat().reduced (1.0f), col::cream);
        drawTitle (g, { 20.0f, 6.0f, 96.0f, 15.0f }, "CARRETES");
        drawTitle (g, { 20.0f, 152.0f, 96.0f, 15.0f }, "MAKEUP");
        auto cap = [&] (const juce::String& t, juce::Component& c) { drawEngraved (g, t, juce::Rectangle<float> ((float) c.getX() - 8, (float) c.getY() - 12, (float) c.getWidth() + 16, 11.0f), 8.5f); };
        cap (U8 ("RA\xc3\x8dZ Hz"), rootF); cap ("FUND. ENT. ms", fadeInF); cap ("FUND. SAL. ms", fadeOutF); cap ("ESCALA", scaleBox); cap ("DESDE", lowF); cap ("HASTA", highF);
        cap ("INICIO s", startF); cap ("FIN s", endF);
        if (auto* r = proc.activeReel())
        {
            const double secs = r->regionSeconds();
            const juce::String dot = U8 ("  \xc2\xb7  ");
            juce::String info = "nota 0 = " + num (r->rootHz, 1) + " Hz" + dot + num (secs, 2) + " s" + dot + num (secs * proc.tapeSpeedCm(), 1) + " cm" + dot + "gen. " + juce::String (r->generation);
            drawEngraved (g, info, { 10.0f, 384.0f, (float) getWidth() - 20.0f, 14.0f }, 10.0f, juce::Justification::centredLeft, col::ink, false);
        }
        else drawEngraved (g, U8 ("Carga un fichero de audio o arr\xc3\xa1stralo a la ventana"), { 10.0f, 384.0f, (float) getWidth() - 20.0f, 14.0f }, 10.0f, juce::Justification::centredLeft, col::ink, false);
    }

private:
    // ---- lista de carretes
    struct List : juce::Component
    {
        ReelPanel& o; int scroll = 0; explicit List (ReelPanel& p) : o (p) {}
        static constexpr int rowH = 22;
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colour (0xff171412)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 3.0f);
            auto& reels = o.proc.reels;
            scroll = juce::jlimit (0, juce::jmax (0, (int) reels.size() - getHeight() / rowH), scroll);
            for (int i = scroll; i < (int) reels.size(); ++i)
            {
                auto& r = *reels[(size_t) i];
                juce::Rectangle<float> row (2.0f, 2.0f + (float) ((i - scroll) * rowH), (float) getWidth() - 4.0f, (float) rowH - 1.0f);
                if (row.getBottom() > getHeight()) break;
                const bool act = r.id == o.proc.activeReelId;
                g.setColour (act ? col::brass.withAlpha (0.9f) : juce::Colour (0xff2a2623)); g.fillRoundedRectangle (row, 2.0f);
                // carrete en miniatura
                const float cx = row.getX() + 11, cy = row.getCentreY();
                g.setColour (col::chrome); g.fillEllipse (cx - 8, cy - 8, 16, 16); g.setColour (col::tape); g.fillEllipse (cx - 6, cy - 6, 12, 12);
                g.setColour (col::chrome); g.fillEllipse (cx - 2.5f, cy - 2.5f, 5, 5);
                g.setFont (uiFont (11.5f, true)); g.setColour (act ? col::bakelite : col::ivory);
                g.drawText (r.name, row.withTrimmedLeft (24).withTrimmedRight (112), juce::Justification::centredLeft, true);
                g.setFont (uiFont (9.5f, true));
                juce::String info = r.missing ? juce::String ("FALTA EL FICHERO") : "G" + juce::String (r.generation) + (r.isStereo() ? "  ST  " : "  M  ") + num (r.numSamples() / r.sr, 2) + " s";
                g.setColour (r.missing ? col::lampOn : (act ? col::bakelite : col::ivory.withAlpha (0.7f)));
                g.drawText (info, row.removeFromRight (110).reduced (4, 0), juce::Justification::centredRight, false);
            }
            if (reels.empty()) { g.setColour (col::ivory.withAlpha (0.45f)); g.setFont (uiFont (11.0f)); g.drawText (U8 ("estanter\xc3\xad" "a vac\xc3\xad" "a"), getLocalBounds(), juce::Justification::centred, false); }
        }
        int rowAt (int y) const { int i = scroll + (y - 2) / rowH; return i >= 0 && i < (int) o.proc.reels.size() ? i : -1; }
        void mouseDown (const juce::MouseEvent& e) override
        {
            const int i = rowAt (e.y); if (i < 0) return;
            auto& r = *o.proc.reels[(size_t) i];
            o.proc.activeReelId = r.id; o.proc.reelEdited (r.id);
            if (r.missing)
            {
                const int id = r.id;
                o.chooser = std::make_unique<juce::FileChooser> ("Reubicar " + r.name, r.file.getParentDirectory(), "*.wav;*.aif;*.aiff;*.flac;*.ogg");
                o.chooser->launchAsync (juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
                                        [this, id] (const juce::FileChooser& fc) { if (fc.getResult().existsAsFile()) o.proc.relocateReel (id, fc.getResult()); });
            }
        }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (e.getDistanceFromDragStart() < 12 || dragging) return;
            const int i = rowAt (e.getMouseDownY()); if (i < 0) return;
            auto f = o.proc.reels[(size_t) i]->file;
            if (f.existsAsFile()) { dragging = true; juce::DragAndDropContainer::performExternalDragDropOfFiles ({ f.getFullPathName() }, false, this, [this] { dragging = false; }); }
        }
        void mouseUp (const juce::MouseEvent&) override { dragging = false; }
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override { scroll += w.deltaY > 0 ? -1 : 1; repaint(); }
        bool dragging = false;
    };

    // ---- forma de onda con region y fundidos
    struct Wave : juce::Component
    {
        ReelPanel& o; std::vector<float> mn, mx; int drag = 0; float lastScrubX = -100; explicit Wave (ReelPanel& p) : o (p) {}
        static constexpr int laneH = 14;
        void rebuild()
        {
            mn.clear(); mx.clear();
            auto* r = o.proc.activeReel(); const int w = getWidth() - 4;
            if (r != nullptr && r->audio && w > 0)
            {
                const int n = r->numSamples(); mn.assign ((size_t) w, 0.0f); mx.assign ((size_t) w, 0.0f);
                const float* d = r->audio->getReadPointer (0);
                for (int x = 0; x < w; ++x)
                {
                    const int a = (int) ((juce::int64) x * n / w), b = juce::jmax (a + 1, (int) ((juce::int64) (x + 1) * n / w));
                    const int st = juce::jmax (1, (b - a) / 256); float lo = 0, hi = 0;
                    for (int i = a; i < b && i < n; i += st) { lo = juce::jmin (lo, d[i]); hi = juce::jmax (hi, d[i]); }
                    mn[(size_t) x] = lo; mx[(size_t) x] = hi;
                }
            }
            repaint();
        }
        void resized() override { rebuild(); }
        float xOf (double frac) const { return 2.0f + (float) frac * ((float) getWidth() - 4.0f); }
        double fracOf (float x) const { return juce::jlimit (0.0, 1.0, (x - 2.0) / juce::jmax (1.0, (double) getWidth() - 4.0)); }
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            g.setColour (juce::Colour (0xff171412)); g.fillRoundedRectangle (b, 3.0f);
            auto* r = o.proc.activeReel(); if (r == nullptr || mn.empty()) return;
            const float wave = b.getHeight() - laneH;
            const float w = b.getWidth() - 4, cy = wave * 0.5f, hh = wave * 0.44f;
            const float xa = xOf (r->regionStart), xb = xOf (r->regionEnd);
            g.setColour (col::tape.withAlpha (0.35f)); g.fillRect (xa, 2.0f, xb - xa, wave - 2.0f);
            for (int x = 0; x < (int) mn.size(); ++x)
            {
                const bool in = x + 2 >= xa && x + 2 <= xb;
                g.setColour (in ? col::ivory : col::ivory.withAlpha (0.3f));
                g.drawVerticalLine (x + 2, cy - mx[(size_t) x] * hh - 0.5f, cy - mn[(size_t) x] * hh + 0.5f);
            }
            const double secs = r->numSamples() / r->sr;
            const float fi = (float) (r->fadeIn / secs) * w, fo = (float) (r->fadeOut / secs) * w;
            g.setColour (col::brass.brighter (0.3f));
            g.drawLine (xa, wave - 2, juce::jmin (xb, xa + fi), 3.0f, 1.4f); g.drawLine (juce::jmax (xa, xb - fo), 3.0f, xb, wave - 2, 1.4f);
            g.fillRect (xa - 1, 2.0f, 2.0f, wave - 2); g.fillRect (xb - 1, 2.0f, 2.0f, wave - 2);
            g.fillEllipse (juce::jmin (xb, xa + fi) - 4, 1.0f, 8, 8); g.fillEllipse (juce::jmax (xa, xb - fo) - 4, 1.0f, 8, 8);
            // regla de lectura: segundos, marcas de INICIO y FIN, clic para escuchar desde ese punto
            juce::Rectangle<float> lane (2.0f, wave, w, (float) laneH - 2.0f);
            g.setColour (juce::Colour (0xff2b2622)); g.fillRect (lane);
            double tick = 0.1; for (double t : { 0.1, 0.25, 0.5, 1.0, 2.0, 5.0, 10.0, 30.0, 60.0 }) { tick = t; if (t / secs * w >= 26.0) break; }
            g.setFont (uiFont (8.0f)); 
            for (double t = 0; t <= secs; t += tick)
            {
                const float x = xOf (t / secs);
                g.setColour (col::ivory.withAlpha (0.35f)); g.drawVerticalLine ((int) x, lane.getY(), lane.getY() + 4);
                if (t > 0 && x < b.getRight() - 24) { g.setColour (col::ivory.withAlpha (0.5f)); g.drawText (num (t, tick < 1.0 ? 2 : 0), juce::Rectangle<float> (x + 2, lane.getY() + 1, 30, 10), juce::Justification::centredLeft, false); }
            }
            juce::Path flagA; flagA.addTriangle (xa - 5, lane.getBottom(), xa + 5, lane.getBottom(), xa, lane.getY());
            juce::Path flagB; flagB.addTriangle (xb - 5, lane.getBottom(), xb + 5, lane.getBottom(), xb, lane.getY());
            g.setColour (col::brass.brighter (0.3f)); g.fillPath (flagA); g.fillPath (flagB);
            const float pf = o.proc.previewFrac();
            if (pf >= 0.0f)
            {
                const float px = xOf (pf);
                g.setColour (col::lampOn); g.drawLine (px, 2.0f, px, b.getBottom() - 2.0f, 1.6f);
                juce::Path head; head.addTriangle (px - 5, 2.0f, px + 5, 2.0f, px, 9.0f); g.fillPath (head);
            }
            else if (isMouseOver (true))
            {
                g.setColour (col::ivory.withAlpha (0.55f)); g.setFont (uiFont (8.5f));
                g.drawText (U8 ("clic aqu\xc3\xad: escuchar desde ese punto"), lane.withTrimmedLeft (8), juce::Justification::centredRight, false);
            }
        }
        bool inLane (const juce::MouseEvent& e) const { return e.y >= getHeight() - laneH; }
        void mouseEnter (const juce::MouseEvent&) override { repaint(); }
        void mouseExit (const juce::MouseEvent&) override { repaint(); }
        void mouseDown (const juce::MouseEvent& e) override
        {
            auto* r = o.proc.activeReel(); if (r == nullptr) return;
            if (inLane (e)) { drag = 5; lastScrubX = (float) e.x; o.proc.startPreview (fracOf ((float) e.x), false); repaint(); return; }
            const float w = (float) getWidth() - 4; const double secs = r->numSamples() / r->sr;
            const float xa = xOf (r->regionStart), xb = xOf (r->regionEnd);
            const float fi = xa + (float) (r->fadeIn / secs) * w, fo = xb - (float) (r->fadeOut / secs) * w;
            if (e.y < 14 && std::abs (e.x - fi) < 9) drag = 3; else if (e.y < 14 && std::abs (e.x - fo) < 9) drag = 4;
            else drag = std::abs (e.x - xa) <= std::abs (e.x - xb) ? 1 : 2;
            mouseDrag (e);
        }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            auto* r = o.proc.activeReel(); if (r == nullptr || drag == 0) return;
            if (drag == 5) { if (std::abs (e.x - lastScrubX) >= 3.0f) { lastScrubX = (float) e.x; o.proc.startPreview (fracOf ((float) e.x), false); } return; }
            const double f = fracOf ((float) e.x); const double secs = r->numSamples() / r->sr;
            if (drag == 1) r->regionStart = juce::jmin (f, r->regionEnd - 0.002);
            else if (drag == 2) r->regionEnd = juce::jmax (f, r->regionStart + 0.002);
            else if (drag == 3) r->fadeIn = juce::jlimit (0.0, (r->regionEnd - r->regionStart) * secs, (f - r->regionStart) * secs);
            else r->fadeOut = juce::jlimit (0.0, (r->regionEnd - r->regionStart) * secs, (r->regionEnd - f) * secs);
            repaint();
        }
        void mouseUp (const juce::MouseEvent&) override
        {
            if (drag >= 1 && drag <= 4) if (auto* r = o.proc.activeReel()) o.proc.reelEdited (r->id);
            drag = 0;
        }
        void mouseDoubleClick (const juce::MouseEvent& e) override
        {
            // doble clic en la forma de onda: coloca el punto de inicio ahi y lo preescucha
            auto* r = o.proc.activeReel(); if (r == nullptr || inLane (e)) return;
            r->regionStart = juce::jmin (fracOf ((float) e.x), r->regionEnd - 0.002);
            o.proc.reelEdited (r->id);
            juce::Timer::callAfterDelay (60, [p = &o.proc] { p->startPreview (0.0, true); });
        }
    };

    DeliaEditor& ed; DeliaProcessor& proc; List list; Wave wave;
    juce::TextButton loadB, removeB, folderB, detectB, sclB; juce::ToggleButton stereoSw; juce::ComboBox scaleBox;
    juce::Label rootF, fadeInF, fadeOutF, lowF, highF, ratiosF, startF, endF; juce::TextButton previewB; bool wasPreview = false;
    std::unique_ptr<juce::FileChooser> chooser;
};

// =================================================================== empalme
class SplicePanel : public juce::Component
{
public:
    SplicePanel (DeliaEditor& e) : ed (e), proc (e.proc), strip (*this), palette (*this)
    {
        addAndMakeVisible (strip); addAndMakeVisible (palette);
        for (int m = 0; m < kNumMachines; ++m)
        {
            auto* b = machineB.add (new juce::TextButton (juce::String (m + 1))); b->setClickingTogglesState (false);
            b->onClick = [this, m] { proc.activeMachine = m; sel = -1; proc.sendChangeMessage(); }; addAndMakeVisible (b);
        }
        const char* figs[6] = { "1", "1/2", "1/4", "1/8", "1/16", "1/32" };
        for (int i = 0; i < 6; ++i)
        {
            auto* b = figB.add (new juce::TextButton (figs[i])); b->onClick = [this, i] { proc.figureIndex = i; refresh(); }; addAndMakeVisible (b);
        }
        dotB.setButtonText ("."); tripB.setButtonText ("3");
        dotB.setTooltip ("Puntillo"); tripB.setTooltip ("Tresillo");
        dotB.onClick = [this] { proc.figureDotted = ! proc.figureDotted; refresh(); }; tripB.onClick = [this] { proc.figureTriplet = ! proc.figureTriplet; refresh(); };
        auto btn = [this] (juce::TextButton& b, const juce::String& t, std::function<void()> fn, const juce::String& tip = {}) { b.setButtonText (t); b.onClick = std::move (fn); b.setTooltip (tip); addAndMakeVisible (b); };
        addAndMakeVisible (dotB); addAndMakeVisible (tripB);
        btn (blankB, "+ VIRGEN", [this] { Piece p; p.reelId = -1; p.lengthCm = proc.figureCm(); p.cut = cutBox.getSelectedId() > 0 ? cutBox.getSelectedId() - 1 : 1; insert (p); }, U8 ("A\xc3\xb1" "ade un trozo de cinta virgen (silencio) con la longitud de la figura"));
        btn (leftB, U8 ("\xe2\x97\x80"), [this] { move (-1); }, "Mueve el trozo hacia el principio");
        btn (rightB, U8 ("\xe2\x96\xb6"), [this] { move (1); }, "Mueve el trozo hacia el final");
        btn (dupB, "DUPLICAR", [this] { if (auto* p = piece()) { Piece c = *p; insert (c); } });
        btn (delB, "CORTAR", [this] { auto& s = cur(); if (sel >= 0 && sel < (int) s.pieces.size()) { s.pieces.erase (s.pieces.begin() + sel); sel = juce::jmin (sel, (int) s.pieces.size() - 1); changed(); } }, "Quita el trozo seleccionado");
        btn (clearB, "VACIAR", [this] { cur().pieces.clear(); sel = -1; changed(); }, "Quita todos los trozos de la tira");
        btn (revB, "INVERTIR", [this] { if (auto* p = piece()) { p->reverse = ! p->reverse; changed(); } }, U8 ("Pega el trozo al rev\xc3\xa9s"));
        btn (exportB, "EXPORTAR TIRA", [this] { proc.exportStrip (proc.activeMachine); }, U8 ("Escribe la tira montada como WAV en la carpeta de carretes"));
        cutBox.addItem ("Corte recto", 1); cutBox.addItem (U8 ("Corte a 45\xc2\xb0"), 2); cutBox.addItem (U8 ("Corte a 60\xc2\xb0"), 3); cutBox.setSelectedId (2, juce::dontSendNotification);
        cutBox.onChange = [this] { if (auto* p = piece()) { p->cut = cutBox.getSelectedId() - 1; changed(); } };
        addAndMakeVisible (cutBox);
        setupField (lenF, *this, [this] (double v) { if (auto* p = piece()) { p->lengthCm = juce::jlimit (0.1, 2000.0, v); changed(); } });
        setupField (offF, *this, [this] (double v) { if (auto* p = piece()) { p->offsetCm = juce::jlimit (0.0, 2000.0, v); changed(); } });
        setupField (degF, *this, [this] (double v) { if (auto* p = piece()) { p->degree = juce::jlimit (-60, 60, (int) v); changed(); } });
        setupField (bpmF, *this, [this] (double v) { proc.manualBpm = juce::jlimit (20.0, 400.0, v); proc.useHostTempo = false; refresh(); });
        setupField (errF, *this, [this] (double v) { proc.cutErrorMm = juce::jlimit (0.0, 20.0, v); proc.remountAll(); });
        hostSw.setButtonText (U8 ("DEL ANFITRI\xc3\x93N")); hostSw.setTooltip (U8 ("Usa el tempo del proyecto. Apagado: el tempo escrito a la izquierda")); stepSw.setButtonText ("PASOS POR MIDI");
        stepSw.setTooltip (U8 ("Cada nota MIDI del canal 1 a\xc3\xb1" "ade un trozo con la figura activa (do central = nota 0)"));
        for (auto* t : { &hostSw, &stepSw }) { t->setColour (juce::ToggleButton::textColourId, col::ink); addAndMakeVisible (t); }
        hostSw.onClick = [this] { proc.useHostTempo = hostSw.getToggleState(); refresh(); };
        stepSw.onClick = [this] { proc.stepInput = stepSw.getToggleState(); };
    }

    Strip& cur() { return proc.strips[proc.activeMachine]; }
    Piece* piece() { auto& s = cur(); return sel >= 0 && sel < (int) s.pieces.size() ? &s.pieces[(size_t) sel] : nullptr; }
    void changed() { proc.stripEdited (proc.activeMachine); }
    void insert (Piece p)
    {
        auto& s = cur(); const int at = sel >= 0 && sel < (int) s.pieces.size() ? sel + 1 : (int) s.pieces.size();
        s.pieces.insert (s.pieces.begin() + at, p); sel = at; changed();
    }
    void move (int d)
    {
        auto& s = cur(); const int j = sel + d;
        if (sel < 0 || j < 0 || j >= (int) s.pieces.size()) return;
        std::swap (s.pieces[(size_t) sel], s.pieces[(size_t) j]); sel = j; changed();
    }
    void addNote (int degree)
    {
        if (proc.activeReel() == nullptr) { proc.statusText = "Carga antes un carrete"; proc.sendChangeMessage(); return; }
        Piece p; p.reelId = proc.activeReelId; p.degree = degree; p.lengthCm = proc.figureCm(); p.cut = juce::jmax (0, cutBox.getSelectedId() - 1); insert (p);
    }

    void refresh()
    {
        for (int m = 0; m < kNumMachines; ++m) machineB[m]->setToggleState (m == proc.activeMachine, juce::dontSendNotification);
        for (int i = 0; i < 6; ++i) figB[i]->setToggleState (i == proc.figureIndex, juce::dontSendNotification);
        dotB.setToggleState (proc.figureDotted, juce::dontSendNotification); tripB.setToggleState (proc.figureTriplet, juce::dontSendNotification);
        hostSw.setToggleState (proc.useHostTempo, juce::dontSendNotification); stepSw.setToggleState (proc.stepInput, juce::dontSendNotification);
        bpmF.setText (num (proc.bpm(), 1), juce::dontSendNotification); errF.setText (num (proc.cutErrorMm, 1), juce::dontSendNotification);
        auto* p = piece();
        for (juce::Component* c : std::initializer_list<juce::Component*> { &lenF, &offF, &degF, &leftB, &rightB, &dupB, &delB, &revB }) c->setEnabled (p != nullptr);
        if (p != nullptr)
        {
            lenF.setText (num (p->lengthCm, 2), juce::dontSendNotification); offF.setText (num (p->offsetCm, 2), juce::dontSendNotification);
            degF.setText (juce::String (p->degree), juce::dontSendNotification); cutBox.setSelectedId (p->cut + 1, juce::dontSendNotification);
            revB.setToggleState (p->reverse, juce::dontSendNotification);
        }
        else { lenF.setText ("", juce::dontSendNotification); offF.setText ("", juce::dontSendNotification); degF.setText ("", juce::dontSendNotification); revB.setToggleState (false, juce::dontSendNotification); }
        strip.repaint(); palette.repaint(); repaint();
    }
    void tick() { strip.repaint(); }

    void resized() override
    {
        const int w = getWidth();
        for (int m = 0; m < kNumMachines; ++m) machineB[m]->setBounds (w - 124 + m * 28, 5, 26, 18);
        strip.setBounds (10, 28, w - 20, 118);
        palette.setBounds (10, 166, w - 20, 62);
        int x = 10; const int fy = 250;
        for (int i = 0; i < 6; ++i) { figB[i]->setBounds (x, fy, 38, 22); x += 40; }
        dotB.setBounds (x + 4, fy, 24, 22); tripB.setBounds (x + 30, fy, 24, 22);
        bpmF.setBounds (w - 196, fy, 56, 22); hostSw.setBounds (w - 134, fy, 128, 22);
        const int iy = 300; x = 10;
        degF.setBounds (x, iy, 44, 21); x += 48; lenF.setBounds (x, iy, 58, 21); x += 62; offF.setBounds (x, iy, 58, 21); x += 62;
        cutBox.setBounds (x, iy, 98, 21); x += 102; revB.setBounds (x, iy, 60, 21); x += 64; leftB.setBounds (x, iy, 24, 21); x += 26; rightB.setBounds (x, iy, 24, 21); x += 28;
        dupB.setBounds (x, iy, 62, 21); x += 66; delB.setBounds (x, iy, 56, 21);
        const int oy = 352;
        blankB.setBounds (10, oy, 76, 22); clearB.setBounds (90, oy, 60, 22); stepSw.setBounds (160, oy, 124, 22); errF.setBounds (292, oy, 50, 22);
        exportB.setBounds (w - 118, oy, 108, 22);
    }

    void paint (juce::Graphics& g) override
    {
        drawPlate (g, getLocalBounds().toFloat().reduced (1.0f), col::cream);
        drawTitle (g, { 20.0f, 6.0f, 190.0f, 15.0f }, U8 ("EMPALME \xc2\xb7 M\xc3\x81QUINA ") + juce::String (proc.activeMachine + 1));
        const double total = cur().totalCm(), sp = proc.tapeSpeedCm();
        drawEngraved (g, num (total, 1) + " cm  =  " + num (total / sp, 2) + " s a " + num (sp, 1) + " cm/s   " + juce::String ((int) cur().pieces.size()) + " trozos",
                      { 216.0f, 6.0f, (float) getWidth() - 350.0f, 15.0f }, 10.5f, juce::Justification::centredLeft, col::ink, false);
        drawEngraved (g, U8 ("NOTAS DEL CARRETE ACTIVO  \xc2\xb7  clic: a\xc3\xb1" "adir trozo   clic derecho: solo escuchar"), { 10.0f, 150.0f, (float) getWidth() - 20.0f, 13.0f }, 8.5f, juce::Justification::centredLeft);
        drawEngraved (g, "FIGURA  =  " + num (proc.figureCm(), 2) + " cm", { 10.0f, 235.0f, 300.0f, 12.0f }, 9.0f, juce::Justification::centredLeft);
        auto cap = [&] (const juce::String& t, juce::Component& c) { drawEngraved (g, t, juce::Rectangle<float> ((float) c.getX() - 10, (float) c.getY() - 12, (float) c.getWidth() + 20, 11.0f), 8.5f); };
        cap ("TEMPO", bpmF); cap ("NOTA", degF); cap ("LARGO cm", lenF); cap ("INICIO cm", offF); cap ("ERROR DE CORTE mm", errF);
        drawEngraved (g, U8 ("Arrastra el borde derecho de un trozo para alargarlo o acortarlo (May\xc3\xbas: sin rejilla). Rueda: desplazar."),
                      { 10.0f, 384.0f, (float) getWidth() - 20.0f, 13.0f }, 9.5f, juce::Justification::centredLeft, col::ink.withAlpha (0.8f), false);
    }

private:
    // ---- la tira sobre el bloque de empalme
    struct StripView : juce::Component
    {
        SplicePanel& o; float scrollCm = 0; int dragPiece = -1; double dragStartLen = 0; explicit StripView (SplicePanel& p) : o (p) {}
        double pxPerCm() const { const double t = o.cur().totalCm(); return juce::jmax (2.2, t > 0 ? (getWidth() - 24.0) / t : 6.0); }
        juce::Rectangle<float> tapeArea() const { return { 0.0f, 44.0f, (float) getWidth(), 34.0f }; }
        void paint (juce::Graphics& g) override
        {
            auto b = getLocalBounds().toFloat();
            // bloque de aluminio
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xffd4d7d3), 0, 0, juce::Colour (0xff9da19d), 0, b.getHeight(), false)); g.fillRoundedRectangle (b, 4.0f);
            g.setColour (juce::Colours::white.withAlpha (0.10f)); for (float y = 3; y < b.getHeight(); y += 3) g.drawHorizontalLine ((int) y, 2.0f, b.getRight() - 2);
            g.setColour (juce::Colours::black.withAlpha (0.4f)); g.drawRoundedRectangle (b, 4.0f, 1.0f);
            auto ta = tapeArea();
            g.setColour (juce::Colour (0xff5b5f5c)); g.fillRect (ta.expanded (0, 3)); g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawRect (ta.expanded (0, 3), 1.0f);
            // ranuras de corte del bloque
            g.setColour (juce::Colour (0xff2a2c2a));
            g.drawLine (b.getCentreX() - 60, ta.getY() - 8, b.getCentreX() - 60, ta.getBottom() + 8, 1.2f);
            g.drawLine (b.getCentreX() - 14, ta.getBottom() + 8, b.getCentreX() + 20, ta.getY() - 8, 1.2f);
            g.drawLine (b.getCentreX() + 62, ta.getBottom() + 8, b.getCentreX() + 82, ta.getY() - 8, 1.2f);
            const double pp = pxPerCm(); const float x0 = 12.0f - (float) (scrollCm * pp);
            // regla en cm
            const double total = o.cur().totalCm();
            double stepCm = 1.0; for (double s : { 1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0 }) { stepCm = s; if (s * pp >= 34.0) break; }
            g.setFont (uiFont (9.0f, true));
            for (double c = 0; c <= juce::jmax (total, (getWidth() / pp) + scrollCm) + 0.01; c += stepCm)
            {
                const float x = x0 + (float) (c * pp); if (x < 2 || x > b.getRight() - 2) continue;
                g.setColour (col::ink); g.drawLine (x, 22, x, 34, 1.0f); g.drawText (juce::String ((int) c), juce::Rectangle<float> (x + 2, 10, 40, 11), juce::Justification::centredLeft, false);
            }
            // rejilla de figura
            const double fig = o.proc.figureCm();
            if (fig * pp >= 5.0) { g.setColour (col::oxblood.withAlpha (0.55f)); for (double c = 0; c <= total + 0.01; c += fig) { const float x = x0 + (float) (c * pp); if (x > 2 && x < b.getRight() - 2) g.drawLine (x, 34, x, 40, 1.0f); } }
            auto& mc = o.proc.engine->machines[o.proc.activeMachine];
            drawStrip (g, ta, o.cur(), pp, x0, o.sel, total > 0 ? mc.uiPos.load() : -1.0f, true);
            if (o.cur().pieces.empty())
            { g.setColour (col::ink.withAlpha (0.7f)); g.setFont (uiFont (11.5f)); g.drawText (U8 ("Tira vac\xc3\xad" "a: pulsa una nota de abajo para pegar el primer trozo"), ta.translated (0, 34), juce::Justification::centred, false); }
            else if (auto* p = o.piece())
            {
                juce::String t;
                if (auto* r = o.proc.reel (p->reelId)) t = r->name + "   " + num (r->rootHz * r->ratioFor (p->degree), 2) + " Hz   x" + num (r->ratioFor (p->degree), 4) + "   " + num (p->lengthCm, 2) + " cm";
                else t = "cinta virgen   " + num (p->lengthCm, 2) + " cm";
                g.setColour (col::ink); g.setFont (uiFont (10.5f, true)); g.drawText (t, juce::Rectangle<float> (10, ta.getBottom() + 12, b.getWidth() - 20, 14), juce::Justification::centredLeft, false);
            }
        }
        int pieceAt (float x, bool& edge) const
        {
            const double pp = pxPerCm(); float px = 12.0f - (float) (scrollCm * pp); edge = false;
            auto& s = o.cur();
            for (int i = 0; i < (int) s.pieces.size(); ++i)
            {
                const float w = (float) (s.pieces[(size_t) i].lengthCm * pp);
                if (x >= px && x < px + w) { edge = x > px + w - juce::jmin (7.0f, w * 0.4f); return i; }
                px += w;
            }
            return -1;
        }
        void mouseMove (const juce::MouseEvent& e) override { bool edge; pieceAt ((float) e.x, edge); setMouseCursor (edge && tapeArea().expanded (0, 6).contains (e.position) ? juce::MouseCursor::LeftRightResizeCursor : juce::MouseCursor::NormalCursor); }
        void mouseDown (const juce::MouseEvent& e) override
        {
            bool edge; const int i = pieceAt ((float) e.x, edge); o.sel = i; dragPiece = (edge && i >= 0) ? i : -1;
            if (dragPiece >= 0) dragStartLen = o.cur().pieces[(size_t) i].lengthCm;
            o.refresh();
        }
        void mouseDrag (const juce::MouseEvent& e) override
        {
            if (dragPiece < 0) return;
            double len = dragStartLen + e.getDistanceFromDragStartX() / pxPerCm();
            const double fig = o.proc.figureCm() * 0.25;
            if (! e.mods.isShiftDown() && fig > 0.05) len = std::round (len / fig) * fig;
            o.cur().pieces[(size_t) dragPiece].lengthCm = juce::jlimit (0.2, 2000.0, len); o.refresh();
        }
        void mouseUp (const juce::MouseEvent&) override { if (dragPiece >= 0) { dragPiece = -1; o.changed(); } }
        void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override
        {
            const double vis = (getWidth() - 24.0) / pxPerCm(); const double maxS = juce::jmax (0.0, o.cur().totalCm() - vis);
            scrollCm = (float) juce::jlimit (0.0, maxS, scrollCm - (w.deltaY + w.deltaX) * vis * 0.3); repaint();
        }
    };

    // ---- teclado de notas del carrete activo
    struct Palette : juce::Component, juce::SettableTooltipClient
    {
        SplicePanel& o; int held = -1000; explicit Palette (SplicePanel& p) : o (p) {}
        void range (int& lo, int& hi) const { auto* r = o.proc.activeReel(); lo = r ? r->lowDeg : -12; hi = r ? r->highDeg : 12; if (hi <= lo) hi = lo + 1; }
        int degAt (int x) const { int lo, hi; range (lo, hi); return juce::jlimit (lo, hi, lo + (int) (x * (hi - lo + 1) / juce::jmax (1, getWidth()))); }
        void paint (juce::Graphics& g) override
        {
            g.setColour (juce::Colour (0xff171412)); g.fillRoundedRectangle (getLocalBounds().toFloat(), 3.0f);
            auto* r = o.proc.activeReel(); int lo, hi; range (lo, hi); const int n = hi - lo + 1; const float kw = (getWidth() - 4.0f) / n;
            const int period = r ? r->scale().size() : 12;
            for (int i = 0; i < n; ++i)
            {
                const int d = lo + i; juce::Rectangle<float> k (2 + i * kw, 2.0f, kw - 1.0f, (float) getHeight() - 4.0f);
                const bool root = ((d % period) + period) % period == 0;
                juce::Colour c = d == held ? col::brass.brighter (0.4f) : (d == 0 ? col::brass : (root ? col::ivory : col::creamDk.darker (0.08f)));
                if (r == nullptr) c = c.withAlpha (0.35f);
                g.setGradientFill (juce::ColourGradient (c.brighter (0.1f), k.getX(), k.getY(), c.darker (0.2f), k.getX(), k.getBottom(), false)); g.fillRoundedRectangle (k, 2.0f);
                if (kw > 13) { g.setColour (col::ink); g.setFont (uiFont (juce::jmin (10.0f, kw * 0.55f), true)); g.drawText ((d > 0 ? "+" : "") + juce::String (d), k.withTrimmedTop (k.getHeight() - 15), juce::Justification::centred, false); }
                if (r != nullptr && kw > 30) { g.setColour (col::ink.withAlpha (0.7f)); g.setFont (uiFont (8.5f)); g.drawText (num (r->rootHz * r->ratioFor (d), 0), k.withHeight (14), juce::Justification::centred, false); }
            }
        }
        void mouseMove (const juce::MouseEvent& e) override
        {
            if (auto* r = o.proc.activeReel()) { const int d = degAt (e.x); setTooltip ("nota " + juce::String (d) + ":  " + num (r->rootHz * r->ratioFor (d), 2) + " Hz,  " + num (r->regionSeconds() / r->ratioFor (d) * o.proc.tapeSpeedCm(), 1) + " cm de sonido"); }
        }
        void mouseDown (const juce::MouseEvent& e) override
        {
            if (o.proc.activeReel() == nullptr) return;
            held = degAt (e.x); o.proc.auditionNote (held, true);
            if (! e.mods.isPopupMenu()) o.addNote (held);
            repaint();
        }
        void mouseUp (const juce::MouseEvent&) override { if (held > -1000) o.proc.auditionNote (held, false); held = -1000; repaint(); }
    };

    DeliaEditor& ed; DeliaProcessor& proc; StripView strip; Palette palette; int sel = -1;
    juce::OwnedArray<juce::TextButton> machineB, figB; juce::TextButton dotB, tripB, blankB, leftB, rightB, dupB, delB, clearB, revB, exportB;
    juce::ComboBox cutBox; juce::Label lenF, offF, degF, bpmF, errF; juce::ToggleButton hostSw, stepSw;
    friend class DeliaEditor;
};

// =================================================================== laboratorio, tratamientos y volcado
class LabPanel : public juce::Component
{
public:
    LabPanel (DeliaEditor& e) : ed (e), proc (e.proc)
    {
        auto K = [this] (juce::OwnedArray<Knob>& arr, const char* id, const juce::String& label, const juce::String& style = {}, int ticks = -1)
        { auto* k = arr.add (new Knob (proc.apvts, id, label, style)); if (ticks > 0) k->slider.getProperties().set ("ticks", ticks); addAndMakeVisible (k); };
        K (row1, "oscFreq", "OSC Hz"); K (row1, "oscLevel", "OSC NIV."); K (row1, "noiseLevel", "RUIDO"); K (row1, "labHp", "GRAVES", {}, 7); K (row1, "labLp", "AGUDOS", {}, 8);
        K (row1, "labEcho", "A ECO"); K (row1, "labPlate", "A PLACA");
        K (row2, "wobCenter", "WOB Hz"); K (row2, "wobWidth", "ANCHO"); K (row2, "wobRate", "VELOC."); K (row2, "wobLevel", "WOB NIV."); K (row2, "wobShape", "FORMA", "chicken");
        K (row3, "echoSpeed", "ECO cm/s"); K (row3, "echoFb", "REALIM."); K (row3, "echoSpacing", "SEPAR."); K (row3, "echoRet", "RET. ECO");
        K (row3, "plateDecay", U8 ("CA\xc3\x8d" "DA")); K (row3, "plateDamp", "AMORT."); K (row3, "platePre", "PREDEL."); K (row3, "plateRet", "RET. PL.");
        square = std::make_unique<ParamToggle> (proc.apvts, "oscWave", "CUADRADA"); addAndMakeVisible (*square);
        for (int h = 0; h < 3; ++h) { auto* t = heads.add (new ParamToggle (proc.apvts, "echoH" + juce::String (h + 1), "CABEZAL " + juce::String (h + 1))); addAndMakeVisible (t); }
        auto key = [this] (juce::ToggleButton& b, const juce::String& t, CaptureSource src, const juce::String& tip)
        {
            b.setButtonText (t); b.getProperties().set ("style", "key"); b.getProperties().set ("red", true); b.setTooltip (tip); addAndMakeVisible (b);
            b.onClick = [this, &b, src] { if (b.getToggleState()) proc.startCapture (src); else proc.stopCapture(); };
        };
        key (bounceKey, "VOLCAR", capMix, U8 ("Graba la mezcla a un carrete nuevo: una generaci\xc3\xb3n m\xc3\xa1s, con p\xc3\xa9rdida de cinta. Pulsa otra vez para parar"));
        key (labKey, "CAPTURAR LAB.", capLab, "Graba el laboratorio (oscilador, ruido, wobbulator) a un carrete");
        offlineB.setButtonText (U8 ("VOLCAR FUERA DE L\xc3\x8dNEA")); offlineB.getProperties().set ("red", true); addAndMakeVisible (offlineB);
        offlineB.setTooltip (U8 ("Renderiza al instante la longitud indicada con las m\xc3\xa1quinas en marcha desde el principio"));
        offlineB.onClick = [this] { proc.offlineBounce (cmValue()); };
        loopLenB.setButtonText ("= BUCLE"); loopLenB.setTooltip (U8 ("Pone la longitud del bucle m\xc3\xa1s largo")); addAndMakeVisible (loopLenB);
        loopLenB.onClick = [this] { double m = 0; for (auto& s : proc.strips) m = juce::jmax (m, s.totalCm()); if (m > 0) cmF.setText (num (m, 1), juce::dontSendNotification); };
        cmF.setEditable (true); cmF.getProperties().set ("window", true); cmF.setJustificationType (juce::Justification::centred); cmF.setText ("152,0", juce::dontSendNotification); addAndMakeVisible (cmF);
    }
    double cmValue() const { return juce::jlimit (1.0, 100000.0, parseNum (cmF.getText())); }
    void refresh()
    {
        const bool cap = proc.isCapturing();
        bounceKey.setToggleState (cap && proc.captureSource() == capMix, juce::dontSendNotification);
        labKey.setToggleState (cap && proc.captureSource() == capLab, juce::dontSendNotification);
        repaint();
    }
    void tick() { if (proc.isCapturing() || wasCap) { wasCap = proc.isCapturing(); refresh(); } }
    void resized() override
    {
        const int w = getWidth(); const float s7 = (w - 16) / 7.0f, s8 = (w - 12) / 8.0f;
        for (int i = 0; i < row1.size(); ++i) row1[i]->setBounds ((int) (8 + i * s7), 26, (int) s7 - 1, 52);
        for (int i = 0; i < row2.size(); ++i) row2[i]->setBounds ((int) (8 + i * s7), 84, (int) s7 - 1, 52);
        square->setBounds ((int) (8 + 5 * s7), 88, (int) (2 * s7), 20); labKey.setBounds ((int) (8 + 5 * s7) + 2, 110, (int) (2 * s7) - 6, 28);
        for (int i = 0; i < row3.size(); ++i) row3[i]->setBounds ((int) (6 + i * s8), 172, (int) s8 - 1, 52);
        for (int h = 0; h < 3; ++h) heads[h]->setBounds (10 + h * 104, 230, 100, 20);
        bounceKey.setBounds (10, 288, 110, 50); offlineB.setBounds (130, 316, w - 140, 22);
        cmF.setBounds (130, 290, 70, 21); loopLenB.setBounds (236, 290, w - 246, 21);
    }
    void paint (juce::Graphics& g) override
    {
        drawPlate (g, getLocalBounds().toFloat().reduced (1.0f), col::cream);
        drawTitle (g, { 20.0f, 6.0f, 110.0f, 15.0f }, "LABORATORIO"); drawTitle (g, { 20.0f, 150.0f, 110.0f, 15.0f }, "TRATAMIENTOS"); drawTitle (g, { 20.0f, 264.0f, 110.0f, 15.0f }, "VOLCADO");
        drawEngraved (g, "cm", { 202.0f, 290.0f, 30.0f, 21.0f }, 10.0f, juce::Justification::centredLeft);
        int maxGen = 0; for (auto& r : proc.reels) maxGen = juce::jmax (maxGen, r->generation);
        juce::String t = proc.isCapturing() ? "GRABANDO  " + num (proc.captureSeconds(), 1) + " s   (" + num (proc.captureSeconds() * proc.tapeSpeedCm(), 0) + " cm)"
                                            : U8 ("Generaci\xc3\xb3n m\xc3\xa1s alta en la estanter\xc3\xad" "a: ") + juce::String (maxGen);
        drawLamp (g, 18.0f, 356.0f, 5.0f, proc.isCapturing());
        drawEngraved (g, t, { 32.0f, 348.0f, (float) getWidth() - 42.0f, 16.0f }, 10.5f, juce::Justification::centredLeft, col::ink, true);
        g.setFont (uiFont (9.5f)); g.setColour (col::ink.withAlpha (0.8f));
        g.drawFittedText (U8 ("El volcado graba la mezcla en un carrete nuevo y descarta las capas. La p\xc3\xa9rdida por generaci\xc3\xb3n la fija el mando DESGASTE."),
                          juce::Rectangle<int> (10, 370, getWidth() - 20, 30), juce::Justification::topLeft, 2, 0.9f);
    }
private:
    DeliaEditor& ed; DeliaProcessor& proc; juce::OwnedArray<Knob> row1, row2, row3; std::unique_ptr<ParamToggle> square; juce::OwnedArray<ParamToggle> heads;
    juce::ToggleButton bounceKey, labKey; juce::TextButton offlineB, loopLenB; juce::Label cmF; bool wasCap = false;
};

// =================================================================== cajon de los doce osciladores
class BankDrawer : public juce::Component
{
public:
    BankDrawer (DeliaEditor& e) : ed (e), proc (e.proc),
        transp (proc.apvts, "bankTransp", U8 ("TRANSPOSICI\xc3\x93N"), "big", col::ivory), drift (proc.apvts, "bankDrift", U8 ("DERIVA V\xc3\x81LV."), {}, col::ivory),
        level (proc.apvts, "bankMaster", "NIVEL", {}, col::ivory), power (proc.apvts, "bankOn", "ENCENDIDO")
    {
        power.setColour (juce::ToggleButton::textColourId, col::ivory);
        for (juce::Component* c : std::initializer_list<juce::Component*> { &transp, &drift, &level, &power }) addAndMakeVisible (c);
        for (int o = 0; o < kNumOsc; ++o)
        {
            const juce::String s (o + 1);
            auto* f = freqF.add (new juce::Label());
            setupField (*f, *this, [this, s] (double v) { proc.setParam ("bFreq" + s, (float) juce::jlimit (0.0, 20000.0, v)); });
            f->setTooltip ("Frecuencia en Hz (0 = apagado)");
            auto* sl = levels.add (new juce::Slider (juce::Slider::LinearVertical, juce::Slider::NoTextBox)); addAndMakeVisible (sl);
            levelAtt.add (new APVTS::SliderAttachment (proc.apvts, "bLevel" + s, *sl));
            auto* w = waves.add (new ParamToggle (proc.apvts, "bWave" + s, "CUAD.")); w->setColour (juce::ToggleButton::textColourId, col::ivory); w->setTooltip ("Onda cuadrada"); addAndMakeVisible (w);
            auto* cb = gates.add (new juce::ComboBox()); cb->addItem (U8 ("\xe2\x80\x94"), 1); for (int g = 1; g <= kNumGates; ++g) cb->addItem ("P" + juce::String (g), g + 1);
            cb->setTooltip (U8 ("Puerta que abre este oscilador (\xe2\x80\x94 = siempre abierto)")); addAndMakeVisible (cb);
            gateAtt.add (new APVTS::ComboBoxAttachment (proc.apvts, "bGate" + s, *cb));
        }
        for (int g = 0; g < kNumGates; ++g)
        {
            const juce::String s (g + 1);
            auto* b = gateB.add (new juce::TextButton ("P" + s)); b->setTooltip (U8 ("Mant\xc3\xa9n pulsado para abrir la puerta. May\xc3\xbas + clic: dejarla abierta. MIDI canal 2, notas 60 a 67"));
            b->onStateChange = [this, b, g] { if (! latched[g]) proc.setGate (g, b->isDown()); };
            b->onClick = [this, b, g] { if (juce::ModifierKeys::currentModifiers.isShiftDown()) { latched[g] = ! latched[g]; proc.setGate (g, latched[g]); b->setToggleState (latched[g], juce::dontSendNotification); } };
            addAndMakeVisible (b);
            for (int k = 0; k < 2; ++k)
            {
                auto* sl = gateSl.add (new juce::Slider (juce::Slider::LinearHorizontal, juce::Slider::NoTextBox)); sl->setPopupDisplayEnabled (true, true, nullptr); addAndMakeVisible (sl);
                gateSlAtt.add (new APVTS::SliderAttachment (proc.apvts, juce::String (k == 0 ? "gAtt" : "gDec") + s, *sl));
            }
        }
        analyseB.setButtonText ("ANALIZAR CARRETE ACTIVO"); analyseB.setTooltip (U8 ("Busca los doce parciales m\xc3\xa1s fuertes de la regi\xc3\xb3n de Makeup y los reparte en los osciladores"));
        analyseB.onClick = [this] { proc.analyseToBank(); refresh(); }; addAndMakeVisible (analyseB);
        capKey.setButtonText ("CAPTURAR"); capKey.getProperties().set ("style", "key"); capKey.getProperties().set ("red", true); addAndMakeVisible (capKey);
        capKey.setTooltip ("Graba el banco a un carrete nuevo. Pulsa otra vez para parar");
        capKey.onClick = [this] { if (capKey.getToggleState()) proc.startCapture (capBank); else proc.stopCapture(); };
    }
    void refresh()
    {
        for (int o = 0; o < kNumOsc; ++o) if (! freqF[o]->isBeingEdited()) freqF[o]->setText (num (proc.param ("bFreq" + juce::String (o + 1)), 1), juce::dontSendNotification);
        capKey.setToggleState (proc.isCapturing() && proc.captureSource() == capBank, juce::dontSendNotification);
    }
    void tick() { if (isVisible()) { refresh(); repaint (gateArea); } }
    void resized() override
    {
        const int colW = 58, x0 = 16;
        for (int o = 0; o < kNumOsc; ++o)
        {
            const int x = x0 + o * colW;
            freqF[o]->setBounds (x, 36, colW - 5, 19); levels[o]->setBounds (x + 10, 60, 32, 108);
            waves[o]->setBounds (x, 176, colW - 4, 20); gates[o]->setBounds (x, 200, colW - 5, 20);
        }
        const int gx = x0 + kNumOsc * colW + 14;
        gateArea = { gx, 30, 224, 196 };
        for (int g = 0; g < kNumGates; ++g)
        {
            const int y = 36 + g * 23;
            gateB[g]->setBounds (gx + 16, y, 40, 20); gateSl[g * 2]->setBounds (gx + 62, y, 72, 20); gateSl[g * 2 + 1]->setBounds (gx + 142, y, 72, 20);
        }
        const int kx = gx + 236;
        power.setBounds (kx, 34, 110, 22); transp.setBounds (kx, 62, 70, 82); drift.setBounds (kx + 78, 70, 54, 62); level.setBounds (kx + 136, 70, 54, 62);
        analyseB.setBounds (kx, 152, 196, 24); capKey.setBounds (kx, 182, 110, 42);
    }
    void paint (juce::Graphics& g) override
    {
        drawPlate (g, getLocalBounds().toFloat().reduced (1.0f), juce::Colour (0xff2f3633));
        drawTitle (g, { 20.0f, 8.0f, 210.0f, 16.0f }, U8 ("DOCE OSCILADORES  \xc2\xb7  OCHO PUERTAS"));
        g.setFont (uiFont (9.0f, true)); g.setColour (col::ivory.withAlpha (0.8f));
        for (int o = 0; o < kNumOsc; ++o)
        {
            g.drawText (juce::String (o + 1), juce::Rectangle<int> (16 + o * 58, 24, 53, 11), juce::Justification::centred, false);
            g.setColour (col::ivory.withAlpha (0.12f)); g.fillRoundedRectangle (juce::Rectangle<float> (17.0f + o * 58, 58.0f, 50.0f, 114.0f), 3.0f); g.setColour (col::ivory.withAlpha (0.8f));
        }
        g.drawText ("ATAQUE", juce::Rectangle<int> (gateArea.getX() + 62, 24, 72, 11), juce::Justification::centred, false);
        g.drawText (U8 ("CA\xc3\x8d" "DA"), juce::Rectangle<int> (gateArea.getX() + 142, 24, 72, 11), juce::Justification::centred, false);
        for (int k = 0; k < kNumGates; ++k)
            drawLamp (g, (float) gateArea.getX() + 6.0f, 46.0f + k * 23.0f, 3.6f, proc.engine->gateLevel[k].load() > 0.05f, juce::Colour (0xffffb43c));
    }
private:
    DeliaEditor& ed; DeliaProcessor& proc; Knob transp, drift, level; ParamToggle power;
    juce::OwnedArray<juce::Label> freqF; juce::OwnedArray<juce::Slider> levels, gateSl; juce::OwnedArray<ParamToggle> waves; juce::OwnedArray<juce::ComboBox> gates;
    juce::OwnedArray<APVTS::SliderAttachment> levelAtt, gateSlAtt; juce::OwnedArray<APVTS::ComboBoxAttachment> gateAtt;
    juce::OwnedArray<juce::TextButton> gateB; juce::TextButton analyseB; juce::ToggleButton capKey; bool latched[kNumGates] {}; juce::Rectangle<int> gateArea;
};

// =================================================================== pasillo
class CorridorView : public juce::Component
{
public:
    CorridorView (DeliaEditor& e) : proc (e.proc) {}
    void paint (juce::Graphics& g) override
    {
        auto b = getLocalBounds().toFloat();
        // suelo de linoleo del pasillo
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xff6b5f4c), 0, 0, juce::Colour (0xff4a4235), 0, b.getHeight(), false)); g.fillRoundedRectangle (b.reduced (1), 5.0f);
        g.setColour (juce::Colours::black.withAlpha (0.12f));
        for (float x = 0; x < b.getWidth(); x += 62) g.drawVerticalLine ((int) x, 2.0f, b.getBottom() - 2);
        for (float y = 0; y < b.getHeight(); y += 62) g.drawHorizontalLine ((int) y, 2.0f, b.getRight() - 2);
        drawTitle (g, { 20.0f, 8.0f, 120.0f, 16.0f }, "PASILLO");
        double maxCm = 1.0; for (auto& s : proc.strips) maxCm = juce::jmax (maxCm, s.totalCm());
        const float left = 96.0f; const double pp = (b.getWidth() - left - 24.0) / maxCm;
        g.setFont (uiFont (10.0f, true)); g.setColour (col::ivory.withAlpha (0.85f));
        g.drawText (U8 ("Las cuatro tiras extendidas a la misma escala (") + num (maxCm, 1) + " cm). Un empalme demasiado largo se ve a ojo.", juce::Rectangle<float> (150, 8, b.getWidth() - 170, 16), juce::Justification::centredLeft, false);
        double stepCm = 1.0; for (double s : { 1.0, 2.0, 5.0, 10.0, 20.0, 50.0, 100.0, 200.0, 500.0, 1000.0 }) { stepCm = s; if (s * pp >= 40.0) break; }
        for (double c = 0; c <= maxCm + 0.01; c += stepCm)
        {
            const float x = left + (float) (c * pp);
            g.setColour (col::ivory.withAlpha (0.25f)); g.drawLine (x, 36, x, b.getBottom() - 12, 1.0f);
            g.setColour (col::ivory.withAlpha (0.8f)); g.drawText (juce::String ((int) c), juce::Rectangle<float> (x + 3, 30, 50, 11), juce::Justification::centredLeft, false);
        }
        const float rowH = (b.getHeight() - 70.0f) / kNumMachines;
        for (int m = 0; m < kNumMachines; ++m)
        {
            juce::Rectangle<float> row (left, 52.0f + m * rowH + rowH * 0.2f, b.getWidth() - left - 24, rowH * 0.5f);
            g.setColour (col::ivory); g.setFont (uiFont (11.0f, true));
            g.drawText (U8 ("M\xc3\x81QUINA ") + juce::String (m + 1), juce::Rectangle<float> (12, row.getY(), 80, row.getHeight()), juce::Justification::centredLeft, false);
            g.setColour (juce::Colours::black.withAlpha (0.3f)); g.fillRect (row.withWidth ((float) (proc.strips[m].totalCm() * pp)).translated (2, 3));
            const float pf = proc.strips[m].pieces.empty() ? -1.0f : proc.engine->machines[m].uiPos.load();
            drawStrip (g, row, proc.strips[m], pp, left, -1, pf, true);
            g.setColour (col::ivory.withAlpha (0.7f)); g.setFont (uiFont (9.5f));
            g.drawText (num (proc.strips[m].totalCm(), 2) + " cm", juce::Rectangle<float> (left, row.getBottom() + 2, 200, 12), juce::Justification::centredLeft, false);
        }
    }
    void mouseDown (const juce::MouseEvent& e) override
    {
        const float rowH = (getHeight() - 70.0f) / kNumMachines; const int m = (int) ((e.y - 52) / rowH);
        if (m >= 0 && m < kNumMachines) { proc.activeMachine = m; proc.sendChangeMessage(); }
    }
private:
    DeliaProcessor& proc;
};

// =================================================================== editor
DeliaEditor::DeliaEditor (DeliaProcessor& p) : AudioProcessorEditor (&p), proc (p)
{
    setLookAndFeel (&look);
    hammer = makeHammertone (256, 256, col::hammer, 1958);
    header = std::make_unique<HeaderBar> (*this); addAndMakeVisible (*header);
    for (int m = 0; m < kNumMachines; ++m) { decks[m] = std::make_unique<DeckView> (proc, m); addAndMakeVisible (*decks[m]); }
    reelPanel = std::make_unique<ReelPanel> (*this); splicePanel = std::make_unique<SplicePanel> (*this); labPanel = std::make_unique<LabPanel> (*this);
    drawer = std::make_unique<BankDrawer> (*this); corridor = std::make_unique<CorridorView> (*this);
    addAndMakeVisible (*reelPanel); addAndMakeVisible (*splicePanel); addAndMakeVisible (*labPanel); addChildComponent (*drawer); addChildComponent (*corridor);
    proc.addChangeListener (this);
    setSize (baseW, baseH + (proc.drawerOpen ? drawerH : 0));
    refreshAll();
    startTimerHz (30);
}

DeliaEditor::~DeliaEditor() { stopTimer(); proc.removeChangeListener (this); setLookAndFeel (nullptr); }

void DeliaEditor::layoutForDrawer() { setSize (baseW, baseH + (proc.drawerOpen ? drawerH : 0)); refreshAll(); }

void DeliaEditor::paint (juce::Graphics& g)
{
    g.setTiledImageFill (hammer, 0, 0, 1.0f); g.fillAll();
    g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.07f), 0, 0, juce::Colours::black.withAlpha (0.22f), 0, (float) getHeight(), false)); g.fillAll();
    g.setColour (juce::Colours::black.withAlpha (0.35f)); g.fillRect (0, 62, getWidth(), 2);
    g.setColour (juce::Colours::white.withAlpha (0.12f)); g.fillRect (0, 64, getWidth(), 1);
}

void DeliaEditor::resized()
{
    header->setBounds (0, 0, baseW, 64);
    for (int m = 0; m < kNumMachines; ++m) decks[m]->setBounds (10 + m * 306, 68, 300, 326);
    const int by = 400, bh = baseH - by - 8;
    reelPanel->setBounds (10, by, 330, bh); splicePanel->setBounds (346, by, 548, bh); labPanel->setBounds (900, by, 330, bh);
    corridor->setBounds (10, by, 884, bh);
    drawer->setBounds (10, baseH - 2, baseW - 20, drawerH - 6);
}

void DeliaEditor::refreshAll()
{
    drawer->setVisible (proc.drawerOpen);
    corridor->setVisible (proc.corridor); reelPanel->setVisible (! proc.corridor); splicePanel->setVisible (! proc.corridor);
    header->refresh(); reelPanel->refresh(); splicePanel->refresh(); labPanel->refresh(); drawer->refresh();
    for (auto& d : decks) d->repaint();
    corridor->repaint();
}

void DeliaEditor::timerCallback()
{
    for (auto& d : decks) d->tick();
    header->tick(); reelPanel->tick(); labPanel->tick(); drawer->tick();
    if (proc.corridor) corridor->repaint(); else splicePanel->tick();
    int deg = 0; bool any = false;
    while (proc.popStepNote (deg)) if (proc.stepInput && ! proc.corridor) { splicePanel->addNote (deg); any = true; }
    juce::ignoreUnused (any);
}

void DeliaEditor::filesDropped (const juce::StringArray& files, int, int)
{
    for (auto& f : files) { juce::File file (f); if (file.existsAsFile()) proc.addReelFromFile (file, proc.stereoReels); }
}
} // namespace delia
