// Delia - escalas y afinaciones
#pragma once
#include <juce_core/juce_core.h>
#include <vector>
#include <cmath>

namespace delia
{
enum ScaleType { scaleEqual = 0, scalePythagorean, scaleMeantone, scaleCustom, numScaleTypes };

struct Scale
{
    std::vector<double> ratios { 1.0 };   // dentro del periodo, ascendente, el primero = 1
    double period = 2.0;

    int size() const { return (int) ratios.size(); }

    double ratioForDegree (int degree) const
    {
        const int n = size();
        int oct = (int) std::floor ((double) degree / n);
        int idx = degree - oct * n;
        return ratios[(size_t) idx] * std::pow (period, oct);
    }

    static Scale equal()
    {
        Scale s; s.ratios.clear();
        for (int i = 0; i < 12; ++i) s.ratios.push_back (std::pow (2.0, i / 12.0));
        return s;
    }
    static Scale pythagorean()
    {
        Scale s;
        s.ratios = { 1.0, 256.0 / 243.0, 9.0 / 8.0, 32.0 / 27.0, 81.0 / 64.0, 4.0 / 3.0, 729.0 / 512.0,
                     3.0 / 2.0, 128.0 / 81.0, 27.0 / 16.0, 16.0 / 9.0, 243.0 / 128.0 };
        return s;
    }
    static Scale meantone()
    {
        // mesotonica de cuarto de coma, cadena de quintas Mib..Sol#
        const int fifths[12] = { 0, 7, 2, -3, 4, -1, 6, 1, 8, 3, -2, 5 };
        const double fifth = std::pow (5.0, 0.25);
        Scale s; s.ratios.clear();
        for (int i = 0; i < 12; ++i)
        {
            double r = std::pow (fifth, fifths[i]);
            while (r >= 2.0) r *= 0.5;
            while (r < 1.0) r *= 2.0;
            s.ratios.push_back (r);
        }
        return s;
    }

    // Admite una lista "1, 9/8, 5/4, 1.5" (razones) o el contenido de un fichero Scala (.scl)
    static bool parse (const juce::String& text, Scale& out)
    {
        juce::StringArray lines; lines.addLines (text);
        bool scl = false;
        for (auto& l : lines) if (l.trimStart().startsWith ("!")) { scl = true; break; }
        std::vector<double> r;
        if (scl || lines.size() > 2)
        {
            int stage = 0, count = 0;
            for (auto l : lines)
            {
                l = l.trim();
                if (l.startsWith ("!")) continue;
                if (stage == 0) { stage = 1; continue; }                       // descripcion
                if (stage == 1) { count = l.getIntValue(); stage = 2; continue; }
                if (l.isEmpty()) continue;
                auto tok = l.upToFirstOccurrenceOf (" ", false, false).upToFirstOccurrenceOf ("\t", false, false);
                double v = 0;
                if (tok.contains (".")) v = std::pow (2.0, tok.getDoubleValue() / 1200.0);       // cents
                else if (tok.contains ("/")) v = tok.upToFirstOccurrenceOf ("/", false, false).getDoubleValue()
                                                 / juce::jmax (1.0, tok.fromFirstOccurrenceOf ("/", false, false).getDoubleValue());
                else v = tok.getDoubleValue();
                if (v > 0) r.push_back (v);
                if ((int) r.size() >= count) break;
            }
            if (r.empty()) return false;
            out.period = r.back(); r.pop_back();
            r.insert (r.begin(), 1.0);
        }
        else
        {
            juce::StringArray toks; toks.addTokens (text, ",; \n\t", "");
            for (auto t : toks)
            {
                t = t.trim(); if (t.isEmpty()) continue;
                double v = t.contains ("/") ? t.upToFirstOccurrenceOf ("/", false, false).getDoubleValue()
                                                  / juce::jmax (1e-9, t.fromFirstOccurrenceOf ("/", false, false).getDoubleValue())
                                            : t.getDoubleValue();
                if (v > 0) r.push_back (v);
            }
            if (r.empty()) return false;
            std::sort (r.begin(), r.end());
            double base = r.front();
            for (auto& v : r) v /= base;
            out.period = 2.0;
            if (r.size() > 1 && std::abs (r.back() - 2.0) < 1e-6) r.pop_back();
            for (auto v : r) if (v >= 2.0) { out.period = std::ceil (r.back() + 1e-9); break; }
        }
        std::sort (r.begin(), r.end());
        out.ratios = r;
        return out.period > 1.0 && ! out.ratios.empty();
    }

    juce::String toText() const
    {
        juce::StringArray a;
        for (auto v : ratios) a.add (juce::String (v, 8));
        a.add (juce::String (period, 8));
        return a.joinIntoString (" ");
    }
    static Scale fromText (const juce::String& t)
    {
        Scale s; juce::StringArray a; a.addTokens (t, " ", "");
        a.removeEmptyStrings();
        if (a.size() < 2) return equal();
        s.ratios.clear();
        for (int i = 0; i < a.size() - 1; ++i) s.ratios.push_back (a[i].getDoubleValue());
        s.period = a[a.size() - 1].getDoubleValue();
        if (s.period <= 1.0 || s.ratios.empty()) return equal();
        return s;
    }
};

inline Scale makeScale (int type, const Scale& custom)
{
    switch (type)
    {
        case scalePythagorean: return Scale::pythagorean();
        case scaleMeantone:    return Scale::meantone();
        case scaleCustom:      return custom;
        default:               return Scale::equal();
    }
}
} // namespace delia
