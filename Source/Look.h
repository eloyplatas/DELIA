// Delia - aspecto: magnetofono de los anos 50 y 60 dibujado por codigo (sin imagenes)
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "Dsp.h"

namespace delia
{
namespace col
{
const juce::Colour hammer   { 0xff5e6b66 };   // esmalte martele gris verdoso
const juce::Colour hammerDk { 0xff3f4946 };
const juce::Colour cream    { 0xffe6dfc9 };   // paneles esmaltados crema
const juce::Colour creamDk  { 0xffcfc6ab };
const juce::Colour ink      { 0xff2b2622 };   // serigrafia
const juce::Colour bakelite { 0xff1d1917 };
const juce::Colour brass    { 0xffb9975a };
const juce::Colour chrome   { 0xffd9dcdc };
const juce::Colour oxblood  { 0xff9a2f25 };
const juce::Colour tape     { 0xff5d3622 };
const juce::Colour tapeLt   { 0xff7d4b30 };
const juce::Colour leader   { 0xffe9e4d2 };
const juce::Colour ivory    { 0xfff1ead6 };
const juce::Colour lampOn   { 0xffff5a3c };
}

inline juce::Font uiFont (float h, bool bold = false)
{
    return juce::Font (juce::FontOptions (h, bold ? juce::Font::bold : juce::Font::plain));
}
inline juce::Font serifFont (float h, int style = juce::Font::plain)
{
    return juce::Font (juce::FontOptions (juce::Font::getDefaultSerifFontName(), h, style));
}

// ---------------------------------------------------------------- texturas
inline juce::Image makeHammertone (int w, int h, juce::Colour base, uint32_t seed)
{
    juce::Image img (juce::Image::RGB, w, h, false);
    juce::Graphics g (img);
    g.fillAll (base);
    Rng r (seed);
    const int n = w * h / 55;
    for (int i = 0; i < n; ++i)
    {
        float x = r.uni() * w, y = r.uni() * h, s = 3.0f + r.uni() * 9.0f;
        bool light = r.uni() > 0.5f;
        g.setColour ((light ? juce::Colours::white : juce::Colours::black).withAlpha (0.035f + 0.05f * r.uni()));
        g.fillEllipse (x - s * 0.5f, y - s * 0.4f, s, s * (0.6f + 0.5f * r.uni()));
    }
    for (int i = 0; i < n / 3; ++i)
    {
        float x = r.uni() * w, y = r.uni() * h, s = 1.5f + r.uni() * 2.5f;
        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.fillEllipse (x, y, s, s * 0.7f);
    }
    return img;
}

inline void drawScrew (juce::Graphics& g, float cx, float cy, float r, float angle)
{
    g.setColour (juce::Colours::black.withAlpha (0.35f)); g.fillEllipse (cx - r + 0.6f, cy - r + 1.0f, r * 2, r * 2);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff2f2ee), cx - r * 0.5f, cy - r * 0.6f, juce::Colour (0xff8d908c), cx + r, cy + r, true));
    g.fillEllipse (cx - r, cy - r, r * 2, r * 2);
    g.setColour (juce::Colour (0xff3a3a38));
    juce::Line<float> slot (cx - std::cos (angle) * r * 0.8f, cy - std::sin (angle) * r * 0.8f, cx + std::cos (angle) * r * 0.8f, cy + std::sin (angle) * r * 0.8f);
    g.drawLine (slot, juce::jmax (1.0f, r * 0.32f));
}

// Panel esmaltado con bisel y tornillos
inline void drawPlate (juce::Graphics& g, juce::Rectangle<float> b, juce::Colour base, bool screws = true, float radius = 5.0f)
{
    g.setColour (juce::Colours::black.withAlpha (0.35f)); g.fillRoundedRectangle (b.translated (0.0f, 2.0f), radius);
    g.setGradientFill (juce::ColourGradient (base.brighter (0.08f), b.getX(), b.getY(), base.darker (0.10f), b.getX(), b.getBottom(), false));
    g.fillRoundedRectangle (b, radius);
    g.setColour (juce::Colours::white.withAlpha (0.45f)); g.drawRoundedRectangle (b.reduced (0.8f), radius, 1.0f);
    g.setColour (juce::Colours::black.withAlpha (0.28f)); g.drawRoundedRectangle (b, radius, 1.0f);
    if (screws)
    {
        const float m = 9.0f;
        drawScrew (g, b.getX() + m, b.getY() + m, 3.2f, 0.6f); drawScrew (g, b.getRight() - m, b.getY() + m, 3.2f, 2.1f);
        drawScrew (g, b.getX() + m, b.getBottom() - m, 3.2f, 1.3f); drawScrew (g, b.getRight() - m, b.getBottom() - m, 3.2f, 0.2f);
    }
}

inline void drawEngraved (juce::Graphics& g, const juce::String& t, juce::Rectangle<float> b, float h, juce::Justification j = juce::Justification::centred,
                          juce::Colour c = col::ink, bool bold = true)
{
    g.setFont (uiFont (h, bold));
    g.setColour (juce::Colours::white.withAlpha (0.35f)); g.drawText (t, b.translated (0.0f, 0.8f), j, false);
    g.setColour (c); g.drawText (t, b, j, false);
}

inline void drawLamp (juce::Graphics& g, float cx, float cy, float r, bool on, juce::Colour c = col::lampOn)
{
    g.setColour (col::chrome.darker (0.3f)); g.fillEllipse (cx - r - 2, cy - r - 2, (r + 2) * 2, (r + 2) * 2);
    g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawEllipse (cx - r - 2, cy - r - 2, (r + 2) * 2, (r + 2) * 2, 1.0f);
    if (on) { g.setColour (c.withAlpha (0.25f)); g.fillEllipse (cx - r * 2.4f, cy - r * 2.4f, r * 4.8f, r * 4.8f); }
    g.setGradientFill (juce::ColourGradient (on ? c.brighter (0.7f) : c.darker (1.6f).brighter (0.15f), cx - r * 0.3f, cy - r * 0.4f, on ? c : c.darker (2.2f), cx + r, cy + r, true));
    g.fillEllipse (cx - r, cy - r, r * 2, r * 2);
    g.setColour (juce::Colours::white.withAlpha (on ? 0.7f : 0.25f)); g.fillEllipse (cx - r * 0.55f, cy - r * 0.6f, r * 0.5f, r * 0.35f);
}

// Medidor de aguja
inline void drawVU (juce::Graphics& g, juce::Rectangle<float> b, float level01, const juce::String& legend = "VU")
{
    g.setColour (juce::Colours::black); g.fillRoundedRectangle (b, 3.0f);
    auto f = b.reduced (2.5f);
    g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff6edcf), f.getCentreX(), f.getY(), juce::Colour (0xffd9cba0), f.getCentreX(), f.getBottom(), false));
    g.fillRoundedRectangle (f, 2.0f);
    const float cx = f.getCentreX(), cy = f.getBottom() + f.getHeight() * 0.35f, rad = f.getHeight() * 1.15f;
    const float a0 = -0.82f, a1 = 0.82f;
    juce::Path arc; arc.addCentredArc (cx, cy, rad, rad, 0.0f, a0, a0 + (a1 - a0) * 0.72f, true);
    g.setColour (col::ink); g.strokePath (arc, juce::PathStrokeType (1.2f));
    juce::Path red; red.addCentredArc (cx, cy, rad, rad, 0.0f, a0 + (a1 - a0) * 0.72f, a1, true);
    g.setColour (col::oxblood); g.strokePath (red, juce::PathStrokeType (2.6f));
    for (int i = 0; i <= 8; ++i)
    {
        const float a = a0 + (a1 - a0) * (float) i / 8.0f;
        g.setColour (i >= 6 ? col::oxblood : col::ink);
        g.drawLine (cx + std::sin (a) * rad, cy - std::cos (a) * rad, cx + std::sin (a) * (rad + 3.5f), cy - std::cos (a) * (rad + 3.5f), 1.0f);
    }
    g.setFont (uiFont (7.5f, true)); g.setColour (col::ink.withAlpha (0.8f));
    g.drawText (legend, f.withTrimmedTop (f.getHeight() * 0.55f), juce::Justification::centred, false);
    const float a = a0 + (a1 - a0) * juce::jlimit (0.0f, 1.0f, level01);
    g.saveState(); g.reduceClipRegion (f.toNearestInt());
    g.setColour (juce::Colours::black.withAlpha (0.25f));
    g.drawLine (cx + 1.5f, cy + 1.5f, cx + 1.5f + std::sin (a) * (rad + 2), cy + 1.5f - std::cos (a) * (rad + 2), 1.4f);
    g.setColour (juce::Colour (0xff1a1a1a));
    g.drawLine (cx, cy, cx + std::sin (a) * (rad + 2), cy - std::cos (a) * (rad + 2), 1.3f);
    g.restoreState();
    g.setColour (juce::Colours::white.withAlpha (0.18f)); g.fillRoundedRectangle (f.withHeight (f.getHeight() * 0.4f), 2.0f);
}

inline float vuMap (float peak) { return juce::jlimit (0.0f, 1.0f, (juce::Decibels::gainToDecibels (peak + 1e-6f) + 36.0f) / 40.0f); }

// ---------------------------------------------------------------- LookAndFeel
class VintageLook : public juce::LookAndFeel_V4
{
public:
    VintageLook()
    {
        setColour (juce::Label::textColourId, col::ink);
        setColour (juce::Label::textWhenEditingColourId, col::ink);
        setColour (juce::Label::backgroundWhenEditingColourId, col::ivory);
        setColour (juce::TextEditor::textColourId, col::ink);
        setColour (juce::TextEditor::backgroundColourId, col::ivory);
        setColour (juce::TextEditor::highlightColourId, col::brass.withAlpha (0.5f));
        setColour (juce::TextEditor::highlightedTextColourId, col::ink);
        setColour (juce::CaretComponent::caretColourId, col::ink);
        setColour (juce::PopupMenu::backgroundColourId, col::bakelite.brighter (0.08f));
        setColour (juce::PopupMenu::textColourId, col::ivory);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, col::brass);
        setColour (juce::PopupMenu::highlightedTextColourId, col::bakelite);
        setColour (juce::ComboBox::textColourId, col::ivory);
        setColour (juce::BubbleComponent::backgroundColourId, col::ivory);
        setColour (juce::TooltipWindow::backgroundColourId, col::ivory);
        setColour (juce::TooltipWindow::textColourId, col::ink);
        setColour (juce::ScrollBar::thumbColourId, col::bakelite.withAlpha (0.6f));
    }

    juce::Font getLabelFont (juce::Label& l) override { return uiFont (juce::jmin (13.0f, (float) l.getHeight() * 0.8f), true); }
    juce::Font getComboBoxFont (juce::ComboBox&) override { return uiFont (11.5f, true); }
    juce::Font getPopupMenuFont() override { return uiFont (13.0f); }
    juce::Font getTextButtonFont (juce::TextButton&, int h) override { return uiFont (juce::jmin (11.0f, h * 0.55f), true); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float startA, float endA, juce::Slider& s) override
    {
        const auto style = s.getProperties()["style"].toString();
        const bool chicken = style == "chicken", big = style == "big";
        auto b = juce::Rectangle<float> ((float) x, (float) y, (float) w, (float) h);
        const float size = juce::jmin (b.getWidth(), b.getHeight());
        const float cx = b.getCentreX(), cy = b.getCentreY();
        const float rTick = size * 0.5f - 1.0f, rKnob = size * (chicken ? 0.30f : 0.36f);
        const float ang = startA + pos * (endA - startA);
        // escala serigrafiada
        const int steps = (int) s.getProperties().getWithDefault ("ticks", chicken ? 0 : 10);
        const int nTicks = chicken ? juce::jmax (2, (int) std::round ((s.getMaximum() - s.getMinimum()) / juce::jmax (1.0, s.getInterval())) + 1) : steps + 1;
        g.setColour (col::ink.withAlpha (0.85f));
        for (int i = 0; i < nTicks; ++i)
        {
            const float a = startA + (endA - startA) * (float) i / (float) (nTicks - 1);
            const float r0 = rTick - (chicken || i % 5 == 0 ? size * 0.10f : size * 0.06f);
            g.drawLine (cx + std::sin (a) * r0, cy - std::cos (a) * r0, cx + std::sin (a) * rTick, cy - std::cos (a) * rTick, chicken ? 1.4f : 1.0f);
        }
        // sombra
        g.setColour (juce::Colours::black.withAlpha (0.35f));
        g.fillEllipse (cx - rKnob - 1, cy - rKnob + 2.5f, rKnob * 2 + 3, rKnob * 2 + 3);
        if (chicken)
        {
            // mando de pico ("chicken head")
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff3a3431), cx - rKnob * 0.5f, cy - rKnob * 0.6f, col::bakelite, cx + rKnob, cy + rKnob, true));
            g.fillEllipse (cx - rKnob, cy - rKnob, rKnob * 2, rKnob * 2);
            juce::Path p; const float len = size * 0.44f, wd = rKnob * 0.62f;
            p.startNewSubPath (-wd, rKnob * 0.75f); p.lineTo (-wd * 0.55f, -len * 0.55f); p.lineTo (0, -len); p.lineTo (wd * 0.55f, -len * 0.55f); p.lineTo (wd, rKnob * 0.75f);
            p.closeSubPath();
            auto pr = p.createPathWithRoundedCorners (2.5f);
            pr.applyTransform (juce::AffineTransform::rotation (ang).translated (cx, cy));
            g.setColour (juce::Colours::black.withAlpha (0.3f)); g.fillPath (pr, juce::AffineTransform::translation (1.0f, 2.0f));
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4a423d), cx - 4, cy - 6, col::bakelite, cx + 6, cy + 8, false));
            g.fillPath (pr);
            g.setColour (juce::Colours::white.withAlpha (0.18f)); g.strokePath (pr, juce::PathStrokeType (0.8f));
            g.setColour (col::ivory);
            g.drawLine (cx + std::sin (ang) * len * 0.35f, cy - std::cos (ang) * len * 0.35f, cx + std::sin (ang) * len * 0.9f, cy - std::cos (ang) * len * 0.9f, 1.6f);
        }
        else
        {
            // mando estriado de baquelita con faldon
            if (big)
            {
                const float rs = rKnob * 1.22f;
                g.setGradientFill (juce::ColourGradient (juce::Colour (0xffe9e9e4), cx - rs, cy - rs, juce::Colour (0xff8a8d89), cx + rs, cy + rs, false));
                g.fillEllipse (cx - rs, cy - rs, rs * 2, rs * 2);
                g.setColour (juce::Colours::black.withAlpha (0.4f)); g.drawEllipse (cx - rs, cy - rs, rs * 2, rs * 2, 0.8f);
            }
            juce::Path body; const int flutes = big ? 28 : 18;
            for (int i = 0; i < flutes * 2; ++i)
            {
                const float a = ang + (float) i * juce::MathConstants<float>::pi / (float) flutes;
                const float r = (i % 2 == 0) ? rKnob : rKnob * 0.93f;
                const float px = cx + std::sin (a) * r, py = cy - std::cos (a) * r;
                if (i == 0) body.startNewSubPath (px, py); else body.lineTo (px, py);
            }
            body.closeSubPath();
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4b433e), cx - rKnob * 0.6f, cy - rKnob * 0.7f, juce::Colour (0xff0f0d0c), cx + rKnob * 0.8f, cy + rKnob, true));
            g.fillPath (body);
            const float rc = rKnob * 0.72f;
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff35302c), cx, cy - rc, juce::Colour (0xff181513), cx, cy + rc, false));
            g.fillEllipse (cx - rc, cy - rc, rc * 2, rc * 2);
            g.setColour (juce::Colours::white.withAlpha (0.10f)); g.drawEllipse (cx - rc, cy - rc, rc * 2, rc * 2, 0.8f);
            g.setColour (col::ivory);
            g.drawLine (cx + std::sin (ang) * rKnob * 0.30f, cy - std::cos (ang) * rKnob * 0.30f, cx + std::sin (ang) * rKnob * 0.95f, cy - std::cos (ang) * rKnob * 0.95f, big ? 2.2f : 1.7f);
        }
    }

    void drawLinearSlider (juce::Graphics& g, int x, int y, int w, int h, float pos, float, float, juce::Slider::SliderStyle st, juce::Slider& s) override
    {
        if (st == juce::Slider::LinearVertical)
        {
            const float cx = x + w * 0.5f;
            g.setColour (col::bakelite); g.fillRoundedRectangle (cx - 2.5f, (float) y, 5.0f, (float) h, 2.0f);
            g.setColour (col::ink.withAlpha (0.6f));
            for (int i = 0; i <= 10; ++i) { const float yy = y + h * i / 10.0f; g.drawLine (cx - (i % 5 == 0 ? 11.0f : 8.0f), yy, cx - 5.0f, yy, 1.0f); }
            juce::Rectangle<float> cap (cx - 10.0f, pos - 7.0f, 20.0f, 14.0f);
            g.setColour (juce::Colours::black.withAlpha (0.4f)); g.fillRoundedRectangle (cap.translated (0, 2), 2.5f);
            g.setGradientFill (juce::ColourGradient (juce::Colour (0xff4b433e), cap.getX(), cap.getY(), col::bakelite, cap.getX(), cap.getBottom(), false));
            g.fillRoundedRectangle (cap, 2.5f);
            g.setColour (col::ivory); g.fillRect (cap.getX() + 2.0f, cap.getCentreY() - 0.8f, cap.getWidth() - 4.0f, 1.6f);
        }
        else
        {
            const float cy = y + h * 0.5f;
            g.setColour (col::bakelite); g.fillRoundedRectangle ((float) x, cy - 2.0f, (float) w, 4.0f, 2.0f);
            juce::Rectangle<float> cap (pos - 5.0f, cy - 7.0f, 10.0f, 14.0f);
            g.setColour (juce::Colours::black.withAlpha (0.4f)); g.fillRoundedRectangle (cap.translated (0, 1.5f), 2.0f);
            g.setColour (s.isEnabled() ? col::ivory : col::creamDk); g.fillRoundedRectangle (cap, 2.0f);
            g.setColour (col::ink); g.drawRoundedRectangle (cap, 2.0f, 0.8f);
        }
    }

    void drawToggleButton (juce::Graphics& g, juce::ToggleButton& b, bool over, bool down) override
    {
        const auto style = b.getProperties()["style"].toString();
        auto r = b.getLocalBounds().toFloat();
        const bool on = b.getToggleState();
        if (style == "key")
        {
            // tecla de piano de magnetofono
            const bool red = (bool) b.getProperties().getWithDefault ("red", false);
            auto k = r.reduced (1.5f, 1.0f);
            const float sink = (on || down) ? 3.0f : 0.0f;
            g.setColour (juce::Colours::black.withAlpha (0.55f)); g.fillRoundedRectangle (k.withTrimmedTop (2.0f), 3.0f);
            auto top = k.withTrimmedBottom (5.0f - sink * 0.6f).translated (0.0f, sink);
            juce::Colour base = red ? (on ? juce::Colour (0xffd0402f) : juce::Colour (0xffa7352a)) : (on ? juce::Colour (0xffd9d0b6) : col::ivory);
            g.setGradientFill (juce::ColourGradient (base.brighter (0.12f), top.getX(), top.getY(), base.darker (0.14f), top.getX(), top.getBottom(), false));
            g.fillRoundedRectangle (top, 3.0f);
            g.setColour (juce::Colours::white.withAlpha (0.5f)); g.drawLine (top.getX() + 3, top.getY() + 1.2f, top.getRight() - 3, top.getY() + 1.2f, 1.0f);
            g.setColour (juce::Colours::black.withAlpha (0.45f)); g.drawRoundedRectangle (top, 3.0f, 0.9f);
            if (over) { g.setColour (juce::Colours::white.withAlpha (0.08f)); g.fillRoundedRectangle (top, 3.0f); }
            g.setColour (red ? col::ivory : col::ink); g.setFont (uiFont (juce::jmin (11.0f, top.getHeight() * 0.42f), true));
            g.drawFittedText (b.getButtonText(), top.toNearestInt().reduced (1), juce::Justification::centred, 2, 0.8f);
            return;
        }
        // interruptor de palanca
        const float sw = juce::jmin (22.0f, r.getHeight());
        const float cx = r.getX() + sw * 0.5f, cy = r.getCentreY();
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfff0f0ec), cx - 6, cy - 6, juce::Colour (0xff7d807c), cx + 7, cy + 7, true));
        g.fillEllipse (cx - sw * 0.36f, cy - sw * 0.36f, sw * 0.72f, sw * 0.72f);
        g.setColour (juce::Colours::black.withAlpha (0.5f)); g.drawEllipse (cx - sw * 0.36f, cy - sw * 0.36f, sw * 0.72f, sw * 0.72f, 0.8f);
        g.setColour (juce::Colour (0xff2a2a28)); g.fillEllipse (cx - sw * 0.17f, cy - sw * 0.17f, sw * 0.34f, sw * 0.34f);
        const float ty = cy + (on ? -sw * 0.36f : sw * 0.36f);
        juce::Path lever; lever.startNewSubPath (cx - 2.0f, cy); lever.lineTo (cx - 3.2f, ty); lever.lineTo (cx + 3.2f, ty); lever.lineTo (cx + 2.0f, cy); lever.closeSubPath();
        g.setColour (juce::Colours::black.withAlpha (0.3f)); g.fillPath (lever, juce::AffineTransform::translation (1.5f, 1.5f));
        g.setGradientFill (juce::ColourGradient (juce::Colour (0xfffafaf6), cx - 3, cy, juce::Colour (0xff8f928e), cx + 3, cy, false));
        g.fillPath (lever);
        g.fillEllipse (cx - 3.6f, ty - 3.6f, 7.2f, 7.2f);
        g.setColour (juce::Colours::black.withAlpha (0.45f)); g.drawEllipse (cx - 3.6f, ty - 3.6f, 7.2f, 7.2f, 0.7f);
        const juce::Colour tc = b.findColour (juce::ToggleButton::textColourId);
        g.setFont (uiFont (juce::jmin (10.5f, r.getHeight() * 0.6f), true)); g.setColour (tc.withAlpha (on ? 1.0f : 0.75f));
        g.drawFittedText (b.getButtonText(), r.withTrimmedLeft (sw + 2.0f).toNearestInt(), juce::Justification::centredLeft, 2, 0.8f);
    }

    void drawButtonBackground (juce::Graphics& g, juce::Button& b, const juce::Colour&, bool over, bool down) override
    {
        auto r = b.getLocalBounds().toFloat().reduced (1.0f);
        const bool red = (bool) b.getProperties().getWithDefault ("red", false);
        g.setColour (juce::Colours::black.withAlpha (0.4f)); g.fillRoundedRectangle (r.translated (0, 1.5f), 3.5f);
        auto t = r.translated (0, down ? 1.2f : 0.0f);
        juce::Colour base = red ? col::oxblood : juce::Colour (0xff2c2724);
        if (b.getToggleState()) base = red ? juce::Colour (0xffd0402f) : col::brass.darker (0.2f);
        g.setGradientFill (juce::ColourGradient (base.brighter (over ? 0.25f : 0.15f), t.getX(), t.getY(), base.darker (0.25f), t.getX(), t.getBottom(), false));
        g.fillRoundedRectangle (t, 3.5f);
        g.setColour (juce::Colours::white.withAlpha (0.18f)); g.drawLine (t.getX() + 3, t.getY() + 1, t.getRight() - 3, t.getY() + 1, 1.0f);
        g.setColour (juce::Colours::black.withAlpha (0.6f)); g.drawRoundedRectangle (t, 3.5f, 0.8f);
    }
    void drawButtonText (juce::Graphics& g, juce::TextButton& b, bool, bool down) override
    {
        g.setFont (getTextButtonFont (b, b.getHeight())); g.setColour (b.isEnabled() ? col::ivory : col::ivory.withAlpha (0.4f));
        g.drawFittedText (b.getButtonText(), b.getLocalBounds().reduced (3, 1).translated (0, down ? 1 : 0), juce::Justification::centred, 2, 0.8f);
    }

    void drawComboBox (juce::Graphics& g, int w, int h, bool, int, int, int, int, juce::ComboBox&) override
    {
        juce::Rectangle<float> r (0.5f, 0.5f, (float) w - 1.0f, (float) h - 1.0f);
        g.setColour (col::bakelite); g.fillRoundedRectangle (r, 3.0f);
        g.setColour (juce::Colours::white.withAlpha (0.15f)); g.drawRoundedRectangle (r, 3.0f, 0.8f);
        juce::Path p; p.addTriangle ((float) w - 13.0f, h * 0.4f, (float) w - 5.0f, h * 0.4f, (float) w - 9.0f, h * 0.68f);
        g.setColour (col::brass); g.fillPath (p);
    }
    void positionComboBoxText (juce::ComboBox& c, juce::Label& l) override { l.setBounds (5, 0, c.getWidth() - 20, c.getHeight()); l.setFont (getComboBoxFont (c)); }

    void drawLabel (juce::Graphics& g, juce::Label& l) override
    {
        const bool window = (bool) l.getProperties().getWithDefault ("window", false);
        auto r = l.getLocalBounds().toFloat();
        if (window)
        {
            g.setColour (juce::Colour (0xff141210)); g.fillRoundedRectangle (r, 2.5f);
            g.setColour (juce::Colours::white.withAlpha (0.2f)); g.drawRoundedRectangle (r.reduced (0.5f), 2.5f, 0.8f);
        }
        if (! l.isBeingEdited())
        {
            g.setFont (getLabelFont (l));
            g.setColour (window ? col::ivory : l.findColour (juce::Label::textColourId));
            g.drawFittedText (l.getText(), l.getLocalBounds().reduced (window ? 4 : 1, 0), l.getJustificationType(), 1, 0.75f);
        }
    }
};

// ---------------------------------------------------------------- controles ligados a parametros
class Knob : public juce::Component
{
public:
    juce::Slider slider;
    Knob (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& labelText, const juce::String& style = {}, juce::Colour textCol = col::ink)
        : label (labelText), colour (textCol)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f, juce::MathConstants<float>::pi * 2.75f, true);
        if (style.isNotEmpty()) slider.getProperties().set ("style", style);
        slider.setPopupDisplayEnabled (true, true, nullptr);
        addAndMakeVisible (slider);
        att = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (s, id, slider);
        if (auto* p = s.getParameter (id)) slider.setDoubleClickReturnValue (true, p->convertFrom0to1 (p->getDefaultValue()));
        if (style == "chicken") slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.70f, juce::MathConstants<float>::pi * 2.30f, true);
    }
    void resized() override { slider.setBounds (getLocalBounds().withTrimmedBottom (label.isEmpty() ? 0 : 11)); }
    void paint (juce::Graphics& g) override
    {
        if (label.isNotEmpty()) drawEngraved (g, label, getLocalBounds().removeFromBottom (11).toFloat().expanded (6.0f, 0.0f), 8.5f, juce::Justification::centred, colour);
    }
private:
    juce::String label; juce::Colour colour;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> att;
};

class ParamToggle : public juce::ToggleButton
{
public:
    ParamToggle (juce::AudioProcessorValueTreeState& s, const juce::String& id, const juce::String& text, const juce::String& style = {}, bool red = false)
    {
        setButtonText (text); setColour (juce::ToggleButton::textColourId, col::ink);
        if (style.isNotEmpty()) getProperties().set ("style", style);
        if (red) getProperties().set ("red", true);
        att = std::make_unique<juce::AudioProcessorValueTreeState::ButtonAttachment> (s, id, *this);
    }
private:
    std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> att;
};
} // namespace delia
