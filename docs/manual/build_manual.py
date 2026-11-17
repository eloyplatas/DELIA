# Genera los manuales de Delia en PDF (castellano e ingles).
# Uso: python3 build_manual.py  ->  docs/Delia_Manual_ES.pdf y docs/Delia_Manual_EN.pdf
import os, sys
from reportlab.lib.pagesizes import A4
from reportlab.lib.units import mm
from reportlab.lib import colors
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.enums import TA_LEFT, TA_CENTER, TA_JUSTIFY
from reportlab.pdfbase import pdfmetrics
from reportlab.pdfbase.ttfonts import TTFont
from reportlab.lib.fonts import addMapping
from reportlab.platypus import (BaseDocTemplate, PageTemplate, Frame, Paragraph, Spacer, Image, Table, TableStyle,
                                PageBreak, KeepTogether, NextPageTemplate, CondPageBreak)
from reportlab.platypus.tableofcontents import TableOfContents

HERE = os.path.dirname(os.path.abspath(__file__))
DOCS = os.path.dirname(HERE)
IMG = os.path.join(DOCS, "img")
sys.path.insert(0, HERE)

FD = "/usr/share/fonts/truetype/dejavu/"
for name, f in [("Sans", "DejaVuSans.ttf"), ("Sans-B", "DejaVuSans-Bold.ttf"), ("Sans-I", "DejaVuSans-Oblique.ttf"),
                ("Sans-BI", "DejaVuSans-BoldOblique.ttf"), ("Serif", "DejaVuSerif.ttf"), ("Serif-B", "DejaVuSerif-Bold.ttf"),
                ("Serif-I", "DejaVuSerif-Italic.ttf"), ("Serif-BI", "DejaVuSerif-BoldItalic.ttf"), ("Mono", "DejaVuSansMono.ttf")]:
    pdfmetrics.registerFont(TTFont(name, FD + f))
for fam in ("Sans", "Serif"):
    addMapping(fam, 0, 0, fam); addMapping(fam, 1, 0, fam + "-B"); addMapping(fam, 0, 1, fam + "-I"); addMapping(fam, 1, 1, fam + "-BI")

INK = colors.HexColor("#2b2622"); BRASS = colors.HexColor("#b9975a"); OX = colors.HexColor("#9a2f25")
CREAM = colors.HexColor("#efe9d6"); HAMMER = colors.HexColor("#5e6b66"); TAPE = colors.HexColor("#5d3622")

ST = {
    "body": ParagraphStyle("body", fontName="Serif", fontSize=9.6, leading=13.6, textColor=INK, alignment=TA_JUSTIFY, spaceAfter=5),
    "h1": ParagraphStyle("h1", fontName="Sans-B", fontSize=17, leading=21, textColor=TAPE, spaceBefore=4, spaceAfter=8, keepWithNext=1),
    "h2": ParagraphStyle("h2", fontName="Sans-B", fontSize=12.2, leading=15, textColor=OX, spaceBefore=9, spaceAfter=4, keepWithNext=1),
    "h3": ParagraphStyle("h3", fontName="Sans-B", fontSize=10, leading=13, textColor=INK, spaceBefore=6, spaceAfter=2, keepWithNext=1),
    "bullet": ParagraphStyle("bullet", fontName="Serif", fontSize=9.6, leading=13.4, textColor=INK, leftIndent=12, bulletIndent=2, spaceAfter=2),
    "cap": ParagraphStyle("cap", fontName="Sans-I", fontSize=8, leading=10, textColor=colors.HexColor("#6b625a"), alignment=TA_CENTER, spaceAfter=8),
    "cell": ParagraphStyle("cell", fontName="Sans", fontSize=7.8, leading=9.8, textColor=INK),
    "cellb": ParagraphStyle("cellb", fontName="Sans-B", fontSize=7.8, leading=9.8, textColor=colors.white),
    "note": ParagraphStyle("note", fontName="Sans", fontSize=8.6, leading=11.6, textColor=INK, backColor=CREAM, borderColor=BRASS,
                           borderWidth=0.6, borderPadding=6, spaceBefore=4, spaceAfter=10),
    "src": ParagraphStyle("src", fontName="Sans", fontSize=7.6, leading=10, textColor=INK, leftIndent=10, bulletIndent=0, spaceAfter=1.5),
    "toc1": ParagraphStyle("toc1", fontName="Sans-B", fontSize=10, leading=15, textColor=INK, leftIndent=0),
    "toc2": ParagraphStyle("toc2", fontName="Sans", fontSize=8.8, leading=12, textColor=INK, leftIndent=14),
}


class Doc(BaseDocTemplate):
    def __init__(self, path, L):
        super().__init__(path, pagesize=A4, leftMargin=20 * mm, rightMargin=20 * mm, topMargin=20 * mm, bottomMargin=18 * mm,
                         title=L["meta_title"], author="Eloy Platas", subject=L["meta_subject"], creator="Delia build_manual.py")
        self.L = L
        fr = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id="f")
        self.addPageTemplates([PageTemplate(id="cover", frames=[fr], onPage=self.cover),
                               PageTemplate(id="body", frames=[fr], onPage=self.deco)])

    def afterFlowable(self, f):
        if isinstance(f, Paragraph) and f.style.name in ("h1", "h2"):
            lvl = 0 if f.style.name == "h1" else 1
            key = "h%d" % id(f)
            self.canv.bookmarkPage(key)
            self.canv.addOutlineEntry(f.getPlainText(), key, level=lvl, closed=lvl > 0)
            self.notify("TOCEntry", (lvl, f.getPlainText(), self.page, key))

    def cover(self, c, d):
        W, H = A4
        c.saveState()
        c.setFillColor(HAMMER); c.rect(0, 0, W, H, stroke=0, fill=1)
        import random
        rnd = random.Random(1958)
        for _ in range(2600):
            x, y, s = rnd.random() * W, rnd.random() * H, 1 + rnd.random() * 5
            c.setFillColor(colors.Color(1, 1, 1, alpha=0.035) if rnd.random() > 0.5 else colors.Color(0, 0, 0, alpha=0.05))
            c.ellipse(x, y, x + s, y + s * 0.7, stroke=0, fill=1)
        # placa de laton
        px, py, pw, ph = 45 * mm, H - 112 * mm, W - 90 * mm, 52 * mm
        c.setFillColor(colors.Color(0, 0, 0, alpha=0.35)); c.roundRect(px + 1.5, py - 3, pw, ph, 8, stroke=0, fill=1)
        c.setFillColor(BRASS); c.roundRect(px, py, pw, ph, 8, stroke=0, fill=1)
        c.setStrokeColor(colors.HexColor("#3a2c18")); c.setLineWidth(1); c.roundRect(px + 6, py + 6, pw - 12, ph - 12, 5, stroke=1, fill=0)
        for sx in (px + 14, px + pw - 14):
            c.setFillColor(colors.HexColor("#d9dcdc")); c.circle(sx, py + ph / 2, 4.5, stroke=0, fill=1)
            c.setStrokeColor(colors.HexColor("#444")); c.line(sx - 3, py + ph / 2 - 1, sx + 3, py + ph / 2 + 1)
        c.setFillColor(colors.HexColor("#1d1917")); c.setFont("Serif-BI", 58); c.drawCentredString(W / 2, py + 23 * mm, "Delia")
        c.setFont("Sans-B", 10.5); c.drawCentredString(W / 2, py + 12 * mm, self.L["cover_sub"])
        c.setFont("Sans", 9); c.drawCentredString(W / 2, py + 7 * mm, self.L["cover_ver"])
        img = os.path.join(IMG, "delia-principal.png")
        iw = W - 40 * mm; ih = iw * 820 / 1240
        c.setFillColor(colors.Color(0, 0, 0, alpha=0.4)); c.rect(20 * mm + 2, 60 * mm - 4, iw, ih, stroke=0, fill=1)
        c.drawImage(img, 20 * mm, 60 * mm, iw, ih)
        c.setFillColor(colors.HexColor("#efe9d6")); c.setFont("Sans", 9.5)
        c.drawCentredString(W / 2, 44 * mm, self.L["cover_line1"])
        c.drawCentredString(W / 2, 38 * mm, self.L["cover_line2"])
        c.setFont("Sans-B", 10); c.drawCentredString(W / 2, 26 * mm, "Eloy Platas")
        c.setFont("Sans", 8); c.drawCentredString(W / 2, 21 * mm, "github.com/eloyplatas/DELIA  ·  GPLv3")
        c.restoreState()

    def deco(self, c, d):
        W, H = A4
        c.saveState()
        c.setStrokeColor(BRASS); c.setLineWidth(0.8); c.line(20 * mm, H - 13 * mm, W - 20 * mm, H - 13 * mm)
        c.setFont("Serif-BI", 10); c.setFillColor(TAPE); c.drawString(20 * mm, H - 11.5 * mm, "Delia")
        c.setFont("Sans", 7.5); c.setFillColor(INK); c.drawRightString(W - 20 * mm, H - 11.5 * mm, self.L["header"])
        c.line(20 * mm, 12 * mm, W - 20 * mm, 12 * mm)
        c.drawCentredString(W / 2, 8 * mm, str(d.page))
        c.restoreState()


def esc(s):
    return s


def build(L, out):
    doc = Doc(out, L)
    story = [NextPageTemplate("body"), PageBreak()]
    toc = TableOfContents(); toc.levelStyles = [ST["toc1"], ST["toc2"]]; toc.dotsMinLevel = 0
    story += [Paragraph(L["toc"], ParagraphStyle("toct", parent=ST["h1"])), toc, PageBreak()]
    W = A4[0] - 40 * mm
    for blk in L["content"]:
        k = blk[0]
        if k == "h1":
            story += [CondPageBreak(60 * mm), Paragraph(blk[1], ST["h1"])]
        elif k == "h2":
            story += [CondPageBreak(30 * mm), Paragraph(blk[1], ST["h2"])]
        elif k == "h3":
            story.append(Paragraph(blk[1], ST["h3"]))
        elif k == "p":
            story.append(Paragraph(blk[1], ST["body"]))
        elif k == "note":
            story.append(Paragraph(blk[1], ST["note"]))
        elif k == "ul":
            for it in blk[1]:
                story.append(Paragraph(it, ST["bullet"], bulletText="•"))
            story.append(Spacer(1, 4))
        elif k == "ol":
            for i, it in enumerate(blk[1], 1):
                story.append(Paragraph(it, ST["bullet"], bulletText="%d." % i))
            story.append(Spacer(1, 4))
        elif k == "img":
            _, name, frac, cap = blk
            from reportlab.lib.utils import ImageReader
            p = os.path.join(IMG, name); iw, ih = ImageReader(p).getSize()
            w = W * frac; h = w * ih / iw
            story.append(KeepTogether([Image(p, w, h), Spacer(1, 3), Paragraph(cap, ST["cap"])]))
        elif k == "table":
            _, rows, widths = blk[:3]
            data = [[Paragraph(str(c), ST["cellb"] if r == 0 else ST["cell"]) for c in row] for r, row in enumerate(rows)]
            t = Table(data, colWidths=[W * x for x in widths], repeatRows=1)
            t.setStyle(TableStyle([("BACKGROUND", (0, 0), (-1, 0), TAPE), ("VALIGN", (0, 0), (-1, -1), "TOP"),
                                   ("ROWBACKGROUNDS", (0, 1), (-1, -1), [colors.white, CREAM]),
                                   ("GRID", (0, 0), (-1, -1), 0.3, colors.HexColor("#c9bfa5")),
                                   ("LEFTPADDING", (0, 0), (-1, -1), 4), ("RIGHTPADDING", (0, 0), (-1, -1), 4),
                                   ("TOPPADDING", (0, 0), (-1, -1), 2.5), ("BOTTOMPADDING", (0, 0), (-1, -1), 2.5)]))
            story += [t, Spacer(1, 8)]
        elif k == "src":
            for it in blk[1]:
                story.append(Paragraph(it, ST["src"], bulletText="–"))
        elif k == "break":
            story.append(PageBreak())
    doc.multiBuild(story)


if __name__ == "__main__":
    import content_es, content_en
    build(content_es.L, os.path.join(DOCS, "Delia_Manual_ES.pdf"))
    build(content_en.L, os.path.join(DOCS, "Delia_Manual_EN.pdf"))
    print("ok")
