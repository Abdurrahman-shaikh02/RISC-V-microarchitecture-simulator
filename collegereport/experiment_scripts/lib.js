// Helpers that reproduce the COEP template styling with docx-js.
const fs = require("fs");
const d = require("docx");
const {
  Paragraph, TextRun, ImageRun, Table, TableRow, TableCell, WidthType, AlignmentType,
  BorderStyle, ShadingType, HeadingLevel, PageBreak, LevelFormat,
} = d;

const FONT = "Times New Roman";
const MONO = "Consolas";
const CONTENT_W = 9792; // 12240 - 2*1224 DXA
let figNo = {}, tabNo = {}, curChapter = "0";

function setChapter(c) { curChapter = String(c); }
function nextFig() { figNo[curChapter] = (figNo[curChapter] || 0) + 1; return `${curChapter}.${figNo[curChapter]}`; }
function nextTab() { tabNo[curChapter] = (tabNo[curChapter] || 0) + 1; return `${curChapter}.${tabNo[curChapter]}`; }

// inline markup: **bold**, *italic*, `code`, [[placeholder]]
function runs(text, base = {}) {
  const out = [];
  const re = /(\*\*[^*]+\*\*|`[^`]+`|\[\[[^\]]+\]\]|\*[^*]+\*)/g;
  let last = 0, m;
  while ((m = re.exec(text))) {
    if (m.index > last) out.push(new TextRun({ text: text.slice(last, m.index), ...base }));
    const t = m[0];
    if (t.startsWith("**")) out.push(new TextRun({ text: t.slice(2, -2), bold: true, ...base }));
    else if (t.startsWith("`")) out.push(new TextRun({ text: t.slice(1, -1), font: MONO, size: 20, ...base }));
    else if (t.startsWith("[[")) out.push(new TextRun({ text: "[" + t.slice(2, -2) + "]", highlight: "yellow", ...base }));
    else out.push(new TextRun({ text: t.slice(1, -1), italics: true, ...base }));
    last = m.index + t.length;
  }
  if (last < text.length) out.push(new TextRun({ text: text.slice(last), ...base }));
  return out;
}

const P = (text, opts = {}) => new Paragraph({ children: runs(text), alignment: AlignmentType.JUSTIFIED, spacing: { after: 120, line: 288 }, ...opts });
const H1 = (text, opts = {}) => new Paragraph({ heading: HeadingLevel.HEADING_1, children: [new TextRun(text)], pageBreakBefore: opts.newPage !== false, spacing: { before: 240, after: 160 } });
const H2 = (text) => new Paragraph({ heading: HeadingLevel.HEADING_2, children: [new TextRun(text)], spacing: { before: 240, after: 100 } });
const H3 = (text) => new Paragraph({ heading: HeadingLevel.HEADING_3, children: [new TextRun(text)], spacing: { before: 180, after: 80 } });
const B = (text, level = 0) => new Paragraph({ numbering: { reference: "bullets", level }, children: runs(text), spacing: { after: 60, line: 276 } });
let numInst = 0;
function NL(items) { // numbered list, restarting each call
  numInst++;
  return items.map((t) => new Paragraph({ numbering: { reference: "numbers", level: 0, instance: numInst }, children: runs(t), spacing: { after: 60, line: 276 } }));
}

function caption(label, text) {
  return new Paragraph({
    alignment: AlignmentType.CENTER, spacing: { before: 60, after: 200 }, keepLines: true,
    children: [new TextRun({ text: label + ": ", bold: true, size: 20 }), ...runs(text, { size: 20 })],
  });
}

function pngSize(path) { const b = fs.readFileSync(path); return [b.readUInt32BE(16), b.readUInt32BE(20)]; }

function figure(path, text, widthIn = 6.5) {
  const [w, h] = pngSize(path);
  const wp = Math.round(widthIn * 96), hp = Math.round(wp * h / w);
  const n = nextFig();
  return {
    no: n,
    items: [
      new Paragraph({ alignment: AlignmentType.CENTER, keepNext: true, spacing: { before: 120, after: 0 },
        children: [new ImageRun({ type: "png", data: fs.readFileSync(path), transformation: { width: wp, height: hp },
          altText: { title: `Figure ${n}`, description: text.replace(/[`*]/g, ""), name: `fig${n}` } })] }),
      caption(`Figure ${n}`, text),
    ],
  };
}

const border = { style: BorderStyle.SINGLE, size: 4, color: "8A8984" };
const borders = { top: border, bottom: border, left: border, right: border };

function table(headers, rows, widths, text, opts = {}) {
  const total = widths.reduce((a, b) => a + b, 0);
  const scale = CONTENT_W / total;
  const ws = widths.map((w) => Math.floor(w * scale));
  ws[ws.length - 1] += CONTENT_W - ws.reduce((a, b) => a + b, 0);
  const fs_ = opts.size || 20;
  const cell = (t, i, head) => new TableCell({
    borders, width: { size: ws[i], type: WidthType.DXA },
    shading: head ? { fill: "DBE5F1", type: ShadingType.CLEAR, color: "auto" } : undefined,
    margins: { top: 50, bottom: 50, left: 90, right: 90 },
    children: String(t).split("\n").map((line) => new Paragraph({
      spacing: { after: 0, line: 252 },
      alignment: (opts.align && opts.align[i]) || AlignmentType.LEFT,
      children: runs(line, { size: fs_, bold: head || undefined }),
    })),
  });
  const n = nextTab();
  return {
    no: n,
    items: [
      new Paragraph({ alignment: AlignmentType.CENTER, keepNext: true, spacing: { before: 160, after: 80 },
        children: [new TextRun({ text: `Table ${n}: `, bold: true, size: 20 }), ...runs(text, { size: 20 })] }),
      new Table({
        width: { size: CONTENT_W, type: WidthType.DXA }, columnWidths: ws,
        rows: [
          new TableRow({ tableHeader: true, children: headers.map((h, i) => cell(h, i, true)) }),
          ...rows.map((r) => new TableRow({ cantSplit: true, children: r.map((c, i) => cell(c, i, false)) })),
        ],
      }),
      new Paragraph({ spacing: { after: 120 }, children: [] }),
    ],
  };
}

function code(src, title) {
  const lines = src.replace(/\t/g, "    ").split("\n");
  const out = [];
  if (title) out.push(new Paragraph({ keepNext: true, spacing: { before: 120, after: 40 }, children: [new TextRun({ text: title, italics: true, size: 19, color: "52514E" })] }));
  lines.forEach((l, i) => out.push(new Paragraph({
    keepNext: i < lines.length - 1 && lines.length < 30,
    shading: { fill: "F4F4F2", type: ShadingType.CLEAR, color: "auto" },
    border: i === 0 ? { top: { style: BorderStyle.SINGLE, size: 4, color: "C9C8C3", space: 2 } }
      : i === lines.length - 1 ? { bottom: { style: BorderStyle.SINGLE, size: 4, color: "C9C8C3", space: 2 } } : undefined,
    spacing: { after: 0, line: 240 }, indent: { left: 120, right: 120 },
    children: [new TextRun({ text: l.length ? l : " ", font: MONO, size: 17 })],
  })));
  out.push(new Paragraph({ spacing: { after: 100 }, children: [] }));
  return out;
}

const numbering = {
  config: [
    { reference: "bullets", levels: [
      { level: 0, format: LevelFormat.BULLET, text: "•", alignment: AlignmentType.LEFT, style: { paragraph: { indent: { left: 540, hanging: 270 } } } },
      { level: 1, format: LevelFormat.BULLET, text: "–", alignment: AlignmentType.LEFT, style: { paragraph: { indent: { left: 1080, hanging: 270 } } } }] },
    { reference: "numbers", levels: [
      { level: 0, format: LevelFormat.DECIMAL, text: "%1.", alignment: AlignmentType.LEFT, style: { paragraph: { indent: { left: 540, hanging: 320 } } } }] },
  ],
};

const styles = {
  default: { document: { run: { font: FONT, size: 24 } } },
  paragraphStyles: [
    { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
      run: { size: 32, bold: true, font: FONT, color: "365F91" }, paragraph: { spacing: { before: 240, after: 160 }, outlineLevel: 0, keepNext: true } },
    { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
      run: { size: 26, bold: true, font: FONT, color: "4F81BD" }, paragraph: { spacing: { before: 240, after: 100 }, outlineLevel: 1, keepNext: true } },
    { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
      run: { size: 24, bold: true, font: FONT, color: "4F81BD" }, paragraph: { spacing: { before: 180, after: 80 }, outlineLevel: 2, keepNext: true } },
  ],
};

module.exports = { d, runs, P, H1, H2, H3, B, NL, figure, table, code, caption, numbering, styles, setChapter, FONT, CONTENT_W };
