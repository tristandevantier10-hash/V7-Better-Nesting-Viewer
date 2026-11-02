# DXF nesting test pack

All drawings are ASCII DXF in **millimetres**. Each file contains separate source
profiles, with inserts drawn beside the host profiles. Internal contours import
as holes, not separate rows.

## How to test

1. Start a fresh Nesting job, or remove every existing row before importing.
   Imports append to an existing job, so mixing samples changes the test.
2. Import one DXF and select Custom shapes if necessary.
3. Enter the sheet dimensions, gap, margin and quantities from the table below.
   **Quantities are not set by DXF import.** The QTY layer labels in newer
   samples are reminders; enter the quantity manually for each imported row.
4. Select the specified rotation setting and calculate.
5. Check the sheet count, piece count, visible gaps, empty cutouts and sheet edges.

The results below were verified with the current importer and nesting engine,
including independent checks of quantities, identities, rotations and spacing.

## Main tests

Quantities follow the imported row order. A gap or margin of **0** is intentional.
"Fixed" means **Keep orientation**; "90°" means **90° rotation steps**.

| File | Tests | Sheet (mm) | Gap | Margin | Quantities | Rotation | Expected |
| --- | --- | --- | --- | --- | --- | --- | --- |
| [01 — Rectangular frame](01-rectangular-frame-with-two-cutouts.dxf) | Two cutouts and separate inserts | 300 × 200 | 0 | 0 | 1, 1, 1 | 90° | 3 rows, 3 pieces, 1 sheet |
| [02 — Circular frame](02-circular-frame-with-disk.dxf) | Circular hole and disk insert | 300 × 300 | 0 | 0 | 1, 1 | 90° | 2 rows, 2 pieces, 1 sheet |
| [03 — Concave L and insert](03-concave-L-and-insert-zero-gap.dxf) | Repeated mixed-source interlocking | 300 × 200 | 0 | 0 | 6, 6 | Fixed | 2 rows, 12 pieces, 1 sheet |
| [04 — Concave L with spacing](04-concave-L-and-insert-with-gap.dxf) | Grouping with actual gap and margin | 308 × 206 | 2 | 2 | 6, 6 | Fixed | 2 rows, 12 pieces, 1 sheet |
| [05 — Irregular profiles](05-irregular-complementary-profiles.dxf) | Complementary pentagon/quadrilateral; no preset rule | 300 × 200 | 0 | 0 | 6, 6 | Fixed | 2 rows, 12 pieces, 1 sheet |
| [06 — Perforated frame](06-frame-and-cutout-insert-with-gap.dxf) | Cutout nesting, spacing and CAD origin conversion | 310 × 106 | 2 | 3 | 3, 3 | Fixed | 2 rows, 6 pieces, 1 sheet |
| [07 — Circles and triangles](07-mixed-circles-and-triangles.dxf) | The 40-piece performance and layout regression | 2440 × 1220 | 6 | 20 | 30, 10 | 90° | 2 rows, 40 pieces, 1 sheet |
| [08 — Rotation required](08-rotation-required-L-profile.dxf) | Respecting allowed rotation | 60 × 100 | 0 | 0 | 1 | 90° | 1 row, 1 piece, 1 sheet |

### What the shapes should do

- **01:** The 300 × 200 frame has two 100 × 140 holes. Each 90 × 130 insert
  should occupy a different hole. Expected utilisation is approximately 92.33%.
- **02:** The 300 mm outer circle has a 200 mm hole; the 160 mm disk should fit
  inside it. Unused space around the disk must remain visible. Expected
  utilisation is approximately 66.11%, including circle polygon approximations.
- **03:** Each 100 × 100 L has 20 mm arms. Its 80 × 80 insert completes the
  square. The six pairs should fill the sheet with approximately 100% utilisation.
- **04:** Inserts are reduced to 76 × 76 so a 2 mm gap fits inside the 80 × 80
  recess. The 2 mm sheet border must also be respected.
- **05:** Six pairs of irregular profiles should fit without overlaps. This
  exercises geometry-based grouping rather than circle/triangle recognition.
- **06:** Each 100 × 100 frame contains a 60 × 60 hole and a 56 × 56 insert.
  Each inner wall needs 2 mm clearance. The frame starts at negative CAD X and
  nonzero Y, testing that import normalises the outer and inner rings together.
- **07:** Look for three staggered circle rows and five pairs of triangles,
  or another independently valid one-sheet arrangement. A Debug build takes
  longer than an optimised build; it should still finish rather than hang.
- **08:** The imported L is 100 × 60 and must turn to fit the 60 × 100 sheet.

## Extra checks using the same files

| File | Change from main test | Expected result |
| --- | --- | --- |
| 03 | Set both quantities to **12**, retaining the 300 × 200 sheet and zero spacing | **24 pieces on 2 sheets**; no missing or duplicated parts |
| 04 | Reduce insert quantity to **5**, keep L quantity at 6 | **11 pieces on 1 sheet**; the unmatched L remains present |
| 05 | Set sheet to **330 × 230**, gap **2**, margin **2**, and allow 90° steps | **12 pieces on 1 sheet**, with visible separation |
| 08 | Switch to **Keep orientation** | A clear **does not fit** error; the engine must not silently rotate the part |

You can then vary dimensions, quantities, gaps and margins. Recalculate after each
change. Results from modified inputs may legitimately need more sheets.

## Invalid import tests

The [Invalid](Invalid) subfolder contains drawings that **must be rejected**:

| File | Expected reason |
| --- | --- |
| [Open polyline](Invalid/01-open-polyline-reject.dxf) | The outline is not closed |
| [Touching cutout](Invalid/02-touching-cutout-reject.dxf) | The hole touches the outer boundary |
| [Crossing cutouts](Invalid/03-crossing-cutouts-reject.dxf) | Two hole boundaries cross |

Import these into a job that already has valid rows. You should get an error,
and the existing rows should remain intact. These are rejection tests, not
drawings to calculate.

These files test geometry and layout; they are not machine-ready toolpaths.
