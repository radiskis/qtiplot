# QtiPlot Project File Format (.qti / .qti.gz) Specification

## 1. Overview & Architecture

A **QtiPlot project file** (`.qti`) is a plain-text, line-oriented, tag-delimited document encoded in UTF-8 that stores the complete workspace state of a QtiPlot session. Compressed projects (`.qti.gz`) are standard Gzip-compressed `.qti` files.

The format serializes:
- Project metadata (QtiPlot version, active scripting language, window counts).
- Hierarchical project folders, modification timestamps, and analysis log history.
- Data tables (column types, labels, comments, formulas, formats, and tab-separated cell data).
- Statistical summary tables (`<TableStatistics>`).
- 2D mathematical matrices (dimensions, coordinates, formula strings, color palettes, and grid data).
- 2D graphs and multi-layer plots (layers, curves, analytical functions, spectrograms, error bars, axes, scales, fonts, legends, and graphic markers).
- Polar coordinate graphs (`<PolarGraph>`).
- Multi-tab script notes (embedded Python or muParser code).
- 3D OpenGL surface plots (rotations, scales, color maps, lighting, and data bindings).

---

## 2. File Header & Project Metadata

Every `.qti` file begins with three mandatory header lines:

```
QtiPlot 0.9.9 project file
<scripting-lang>	Python
<windows>	3
```

### Field Definitions:
1. **Line 1 (`Signature & Version`)**:
   `QtiPlot <major>.<minor>.<patch> project file`
   - Numeric version is computed as: $100 \times \text{major} + 10 \times \text{minor} + \text{patch}$ (e.g., $0.8.9 \to 89$, $0.9.9 \to 99$).
2. **Line 2 (`Scripting Language`)**:
   `<scripting-lang>\t<language_name>`
   - Supported values: `Python`, `muParser`.
3. **Line 3 (`Window Count`)**:
   `<windows>\t<integer>`
   - Total number of top-level MDI windows contained in the project file.

---

## 3. Folder Hierarchy & Log History

Projects can contain a nested folder tree.

### Grammar & Structure:
```
<folder>	FolderName	27.08.2026 08:30:00	27.08.2026 08:35:00	current
<open>1</open>
<!-- Windows belonging to this folder -->
<log>
Analysis output text...
</log>
</folder>
```

- **`<folder>\t<name>\t<created>\t<modified>[\tcurrent]`**: Starts a folder. The optional 5th token `current` designates the active workspace directory upon opening.
- **`<open>0|1</open>`**: GUI tree expansion state ($1 = \text{expanded}$, $0 = \text{collapsed}$).
- **`<log>...</log>`**: Plain-text history of mathematical fits and log console outputs attached to this folder.
- **`</folder>`**: Closes the current folder scope and returns to the parent folder.

---

## 4. Data Tables & Statistics

### 4.1 Data Tables (`<table>`)
```
<table>
OscillationData	60	3	27.08.2026 08:30:00
geometry	20	20	440	320	
header	Time[X]	Signal[Y]	Decay[yEr]
ColWidth	100	100	100
<com>
<col nr="0">
i * 0.1
</col>
</com>
ColType	0;0/6	0;0/6	0;0/6
Comments	Seconds	Volts	Error
ReadOnlyColumn	0	0	0
HiddenColumn	0	0	0
WindowLabel	Harmonic Data	2
<data>
0	0.0	0.0	1.0
1	0.169	0.142	0.95
2	0.338	0.254	0.90
</data>
</table>
```

### Table Column Roles:
- `[X]`: Independent variable / abscissa.
- `[Y]`: Dependent variable / ordinate.
- `[Z]`: 3D coordinate.
- `[xEr]`: Horizontal X-error bar.
- `[yEr]`: Vertical Y-error bar.
- `[L]`: Label / Annotation column.

### 4.2 Table Statistics (`<TableStatistics>`)
Stores statistical column or row analysis windows generated from a parent data table:
```
<TableStatistics>
Table1_Stats	Table1	col	27.08.2026 08:31:00
Targets	0	1	2
Range	0	59
ColStatType	0	1	2	3
geometry	50	50	400	200	
header	Col[L]	Mean[Y]	StdDev[Y]	Min[Y]	Max[Y]
ColWidth	80	80	80	80	80
...
</TableStatistics>
```

---

## 5. Mathematical Matrices (`<matrix>`)

2D grids used for image data, 3D surface plotting, and contour maps.

### Structure Example:
```
<matrix>
WaveMatrix	40	40	27.08.2026 08:31:00
geometry	20	360	440	455	
ColWidth	60
<formula>
sin(sqrt(x^2 + y^2))
</formula>
TextFormat	f	6
WindowLabel	3D Surface Matrix	2
Coordinates	-4	4	-4	4
ViewType	0
HeaderViewType	1
<xLabel>X Axis</xLabel>
<yLabel>Y Axis</yLabel>
<zLabel>Intensity</zLabel>
<data>
0	0.123	0.456	0.789	...
1	0.234	0.567	0.890	...
</data>
</matrix>
```

---

## 6. MultiLayer 2D Graphs & Plot Layers

A `<multiLayer>` window contains one or more 2D plot layers (`<graph>`).

### 6.1 MultiLayer Header & Layout:
```
<multiLayer>
Graph1	1	1	27.08.2026 08:32:00
geometry	480	20	780	410	
WindowLabel	Oscillation Plot	2
Margins	5	5	5	5
Spacing	5	5
LayerCanvasSize	700	340
Alignement	0	0
<AlignPolicy>0</AlignPolicy>
<CommonAxes>0</CommonAxes>
<graph>
...
</graph>
</multiLayer>
```

### 6.2 Data Series Curves & Functions:
1. **Discrete Data Series (`curve`)**:
   `curve\t<x_col>\t<y_col>\t<curve_type>\t<pen_color>\t<pen_width>\t<pen_style>\t<symbol_style>\t<symbol_size>\t<symbol_color>\t<symbol_fill>...`
   - Curve types: `0 = Line`, `1 = Scatter`, `2 = Line + Symbol`, `3 = Vertical Bars`, `4 = Area`, `5 = Pie`, `6 = Histogram`, `7 = Spline`, `8 = BoxPlot`, `9 = Step`.
2. **Error Bars (`ErrorBars`)**:
   `ErrorBars\t<curve_idx>\t<x_col>\t<y_col>\t<err_col>\t<direction>\t<width>\t<color>\t<cap_width>\t<through>\t<plus>\t<minus>`
3. **Analytical Mathematical Functions (`<Function>`)**:
   ```
   <Function>
   <Type>0</Type>
   <Title>f(x) = exp(-0.5*x)*sin(2*pi*x)</Title>
   <Expression>exp(-0.5*x)*sin(2*pi*x)</Expression>
   <Variable>x</Variable>
   <Range>0.0	10.0</Range>
   <Points>500</Points>
   </Function>
   ```
4. **Spectrograms & Contours (`<spectrogram>`)**:
   ```
   <spectrogram>
   <matrix>WaveMatrix</matrix>
   <xAxis>0</xAxis>
   <yAxis>1</yAxis>
   <Image>1</Image>
   <ContourLines>1</ContourLines>
   <Levels>3</Levels>
   <z>-0.5</z>
   <z>0.0</z>
   <z>0.5</z>
   <ColorPolicy>0</ColorPolicy>
   </spectrogram>
   ```

### 6.3 Graphical Annotations & Markers:
- **Lines & Arrows (`<line>`)**:
  `<line>\t<x1>\t<y1>\t<x2>\t<y2>\t<width>\t<color>\t<style>\t<endArrow>\t<startArrow>\t<headLength>\t<headAngle>\t<filled>\t<attachPolicy></line>`
- **Legends & Text Boxes (`<Legend>` / `Legend`)**:
  `Legend\t<x>\t<y>\t<font_family>\t<font_size>\t<weight>\t<italic>\t<under>\t<strike>\t<color>\t<frame>\t<align>\t<bg_color>\t<text>`
- **LaTeX Math Formulas (`<TexFormula>`)**:
  ```
  <TexFormula>
  <tex>\int_{0}^{\infty} e^{-x^2} dx = \frac{\sqrt{\pi}}{2}</tex>
  <x>2.5</x>
  <y>0.8</y>
  </TexFormula>
  ```
- **Embedded Images (`<Image>`)**:
  ```
  <Image>
  <path>docs/images/logo.png</path>
  <x>1.0</x>
  <y>4.0</y>
  </Image>
  ```
- **Geometric Shapes (`<Rectangle>`, `<Ellipse>`)**:
  ```
  <Rectangle>
  <x>1.0</x>
  <y>2.0</y>
  <right>4.0</right>
  <bottom>1.0</bottom>
  <Background>#ffff00</Background>
  <Alpha>120</Alpha>
  </Rectangle>
  ```

---

## 7. Polar Coordinate Graphs (`<PolarGraph>`)

Specialized single-layer polar plots:
```
<PolarGraph>
geometry	100	100	500	500
<Name>PolarPlot1</Name>
<RadiusScale>
	<Start>0</Start>
	<End>10</End>
	<Autoscale>1</Autoscale>
</RadiusScale>
<AzimuthScale>
	<Start>0</Start>
	<End>360</End>
	<Autoscale>0</Autoscale>
</AzimuthScale>
<Curve>
	<Table>AntennaData</Table>
	<RCol>Gain</RCol>
	<ThetaCol>Angle</ThetaCol>
	<LineColor>#ff0000</LineColor>
	<LineWidth>2</LineWidth>
</Curve>
</PolarGraph>
```

---

## 8. Script Notes (`<note>`)

Notes store multi-tab Python or analytical text scripts.

```
<note>
AnalysisScript	27.08.2026 08:33:00
geometry	100	100	600	400	
WindowLabel	Automation Script	2
AutoExec	0
<LineNumbers>1</LineNumbers>
<tab>
<active>1</active>
<title>main.py</title>
<content>
import numpy as np
t = app.table("OscillationData")
print("Row count:", t.numRows())
</content>
</tab>
</note>
```

---

## 9. 3D OpenGL Surface Plots (`<SurfacePlot>`)

```
<SurfacePlot>
SurfacePlot1	27.08.2026 08:34:00
geometry	100	100	600	500
SurfaceFunction	WaveMatrix
Style	frame	nofloor
...
</SurfacePlot>
```

---

## 10. Complete XML-like Tag Summary Table

| Block Element | Parent Scope | Feature Description |
| :--- | :--- | :--- |
| `<folder> ... </folder>` | Root / Folder | Nested directory container |
| `<table> ... </table>` | Folder | Data spreadsheet table |
| `<TableStatistics> ... </TableStatistics>` | Folder | Column/row statistical summary table |
| `<matrix> ... </matrix>` | Folder | 2D array / continuous matrix grid |
| `<multiLayer> ... </multiLayer>` | Folder | MultiLayer 2D graph container |
| `<graph> ... </graph>` | MultiLayer | Individual 2D plot layer |
| `<PolarGraph> ... </PolarGraph>` | Folder | Polar coordinate plot |
| `<note> ... </note>` | Folder | Multi-tab script editor |
| `<tab> ... </tab>` | Note | Single tab within a script note |
| `<SurfacePlot> ... </SurfacePlot>` | Folder | 3D OpenGL surface plot |
| `<Function> ... </Function>` | Graph Layer | Analytical math function curve |
| `<spectrogram> ... </spectrogram>` | Graph Layer | Matrix heatmap / contour plot |
| `<TexFormula> ... </TexFormula>` | Graph Layer | LaTeX formula annotation |
| `<Image> ... </Image>` | Graph Layer | Raster image annotation |
| `<Rectangle> / <Ellipse>` | Graph Layer | Geometric shape annotations |
| `<line> ... </line>` | Graph Layer | Arrow / line annotations |
| `<data> ... </data>` | Table / Matrix | Tab-delimited data payload |
| `<com> ... </com>` | Table | Column calculation formulas |
| `<log> ... </log>` | Folder | Analytical text log history |
