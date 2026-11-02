# DXF nesting samples

Both files use ASCII DXF with drawing units set to millimetres. Import one file
at a time into a fresh Nesting job. Keep every imported row at quantity **1**.
The inserts are drawn beside the frames so they import as separate parts.

## 01 — Rectangular frame with two cutouts

- Frame: **300 × 200 mm**.
- Two cutouts: **100 × 140 mm** each.
- Two separate inserts: **90 × 130 mm** each.
- Test sheet: **300 × 200 mm**.
- Expected import: **3 rows** (one frame with two holes, two inserts).
- Expected layout: **one sheet**, with one insert inside each cutout.
- Expected utilisation: approximately **92.33%**.

## 02 — Circular frame with disk

- Outer frame diameter: **300 mm**.
- Central cutout diameter: **200 mm**.
- Separate disk insert diameter: **160 mm**.
- Test sheet: **300 × 300 mm**.
- Expected import: **2 rows** (one frame with one hole, one disk).
- Expected layout: **one sheet**, with the disk inside the circular cutout.
- Expected utilisation: approximately **66.11%**, including conservative polygon
  approximations of circles.

Check that the empty space around the inserts remains visible, then try resizing
the parts or changing rotation settings and calculating again. These samples are
for import and nesting checks, not machine-ready cutting toolpaths.
