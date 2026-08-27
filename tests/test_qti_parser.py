import os
import sys
sys.path.insert(0, os.path.abspath(os.path.join(os.path.dirname(__file__), "..")))

import pytest
import numpy as np

import qtiparser


def test_parse_simple_2dplot():
    qti_path = "manual/html/tutorial/simple-2dplot.qti"
    assert os.path.exists(qti_path)

    project = qtiparser.read_qti(qti_path)

    # 1. Project Metadata
    assert project.version == "0.8.9"
    assert project.scripting_lang == "muParser"
    assert project.window_count == 3

    # 2. Tables
    assert "table1" in project.tables
    t1 = project.tables["table1"]
    assert t1.num_rows == 7
    assert t1.num_cols == 4
    assert t1.window_label == "data 1"
    assert len(t1.columns) == 4
    assert t1.columns[0].name == "1"
    assert t1.columns[0].plot_type == "X"
    assert t1.columns[1].name == "2"
    assert t1.columns[1].plot_type == "Y"
    assert t1.columns[2].name == "3"
    assert t1.columns[2].plot_type == "xEr"
    assert t1.columns[3].name == "4"
    assert t1.columns[3].plot_type == "yEr"

    # Verify column data
    assert t1.columns[0].data == ["1", "2.2", "3", "4.5", "5.2", "7.1", "8.2"]
    assert t1.columns[1].data[0] == "4.2"
    assert t1.columns[1].data[4] == "-3.2"

    # NumPy Array conversion
    np_arr = t1.to_numpy()
    assert np_arr.shape == (7, 4)
    assert np_arr[0, 0] == 1.0
    assert np_arr[0, 1] == 4.2

    # Pandas DataFrame conversion (if pandas available)
    try:
        import pandas as pd
        df = t1.to_dataframe()
        assert isinstance(df, pd.DataFrame)
        assert df.shape == (7, 4)
        assert list(df.columns) == ["1", "2", "3", "4"]
        assert df["1"].iloc[1] == 2.2
    except ImportError:
        pass

    # Table 2
    assert "table2" in project.tables
    t2 = project.tables["table2"]
    assert t2.num_rows == 4
    assert t2.num_cols == 2
    assert t2.columns[0].data == ["1", "2", "5", "6"]
    assert t2.columns[1].data == ["5", "2", "3", "5"]

    # 3. 2D Graph
    assert "graph2" in project.graphs
    g = project.graphs["graph2"]
    assert len(g.layers) == 1
    layer = g.layers[0]
    assert len(layer.curves) == 1
    c = layer.curves[0]
    assert c.x_source == "table1_1"
    assert c.y_source == "table1_2"

    # Error bars
    assert len(layer.error_bars) == 2
    err1 = layer.error_bars[0]
    assert err1.err_source == "table1_3"
    assert err1.color == "#ff0000"
    err2 = layer.error_bars[1]
    assert err2.err_source == "table1_4"
    assert err2.color == "#5500ff"

    # Scales
    assert 0 in layer.scales
    assert layer.scales[0].min_val == -4.0
    assert layer.scales[0].max_val == 5.0
    assert layer.scales[2].min_val == 0.0
    assert layer.scales[2].max_val == 9.0

    # Legend
    assert len(layer.legends) >= 1
    assert "table1_2" in layer.legends[0].text


def test_parse_polar_graph():
    qti_path = "qtiplot/bndpolar.qti"
    if os.path.exists(qti_path):
        project = qtiparser.read_qti(qti_path)
        assert project.version == "0.9.8"
        assert len(project.tables) >= 1
        assert len(project.polar_graphs) >= 1


def test_parse_advanced_features_synthetic():
    """Test parsing of FunctionCurves, Spectrograms, Markers, Shapes, and Statistics."""
    synthetic_qti = """QtiPlot 0.9.9 project file
<scripting-lang>	Python
<windows>	4
<folder>	SubAnalysis	27.08.2026 08:00:00	27.08.2026 08:10:00	current
<open>1</open>
<TableStatistics>
Table1_Stats	Table1	col	27.08.2026 08:05:00
Targets	0	1
Range	0	10
geometry	10	10	300	200
header	Col[L]	Mean[Y]
ColWidth	100	100
</TableStatistics>
<matrix>
MatrixGrid	10	10	27.08.2026 08:06:00
geometry	10	220	300	200
ColWidth	60
<formula>
x * y
</formula>
Coordinates	-2	2	-2	2
ViewType	1
HeaderViewType	1
<xLabel>X-Coords</xLabel>
<yLabel>Y-Coords</yLabel>
<data>
0	1.0	2.0	3.0
1	4.0	5.0	6.0
</data>
</matrix>
<multiLayer>
AdvPlot	1	1	27.08.2026 08:07:00
geometry	320	10	600	450
<graph>
ggeometry	5	5	580	400
PlotTitle	Advanced Features Layer	#000000	4
<Function>
<Type>0</Type>
<Title>SinWave</Title>
<Expression>sin(2*pi*x)</Expression>
<Variable>x</Variable>
<Range>0.0	5.0</Range>
<Points>200</Points>
</Function>
<spectrogram>
<matrix>MatrixGrid</matrix>
<xAxis>0</xAxis>
<yAxis>1</yAxis>
<Image>1</Image>
<ContourLines>1</ContourLines>
<z>0.5</z>
<z>1.5</z>
<ColorPolicy>0</ColorPolicy>
</spectrogram>
    <line>	0.5	0.5	2.0	3.5	2.0	#0000ff	0	1	0	10	30	1	0</line>
    <TexFormula>
    <tex>\\int e^{-x} dx</tex>
<x>1.5</x>
<y>2.5</y>
</TexFormula>
<Image>
<path>docs/images/logo.png</path>
<x>0.5</x>
<y>1.0</y>
</Image>
<Rectangle>
<x>0.2</x>
<y>0.8</y>
<right>1.2</right>
<bottom>0.3</bottom>
<Background>#ff0000</Background>
<Alpha>100</Alpha>
</Rectangle>
<Ellipse>
<x>2.0</x>
<y>3.0</y>
<right>3.5</right>
<bottom>2.0</bottom>
<Background>#00ff00</Background>
<Alpha>150</Alpha>
</Ellipse>
</graph>
</multiLayer>
</folder>
"""
    import io
    project = qtiparser.read_qti(io.StringIO(synthetic_qti))

    # 1. Folder
    assert project.root_folder is not None
    assert len(project.root_folder.subfolders) == 1
    sub = project.root_folder.subfolders[0]
    assert sub.name == "SubAnalysis"
    assert sub.is_active is True

    # 2. Table Statistics
    assert "Table1_Stats" in project.tables
    stat = project.tables["Table1_Stats"]
    assert isinstance(stat, qtiparser.QtiTableStatistics)
    assert stat.base_table_name == "Table1"
    assert stat.stat_type == "col"
    assert stat.targets == [0, 1]

    # 3. Matrix
    assert "MatrixGrid" in project.matrices
    mat = project.matrices["MatrixGrid"]
    assert mat.x_start == -2.0
    assert mat.x_end == 2.0
    assert mat.view_type == 1
    assert mat.x_label == "X-Coords"
    assert mat.formula == "x * y"

    # 4. MultiLayer Graph & Advanced Items
    assert "AdvPlot" in project.graphs
    g = project.graphs["AdvPlot"]
    assert len(g.layers) == 1
    layer = g.layers[0]
    assert layer.title == "Advanced Features Layer"

    # Function curve
    assert len(layer.function_curves) == 1
    fc = layer.function_curves[0]
    assert fc.title == "SinWave"
    assert fc.formulas == ["sin(2*pi*x)"]
    assert fc.points == 200

    # Spectrogram
    assert len(layer.spectrograms) == 1
    sp = layer.spectrograms[0]
    assert sp.matrix_name == "MatrixGrid"
    assert sp.contour_lines is True
    assert sp.contour_levels == [0.5, 1.5]

    # Line Marker
    assert len(layer.line_markers) == 1
    lm = layer.line_markers[0]
    assert lm.start_x == 0.5
    assert lm.end_x == 2.0
    assert lm.color == "#0000ff"

    # Tex Marker
    assert len(layer.tex_markers) == 1
    assert layer.tex_markers[0].formula == r"\int e^{-x} dx"

    # Image Marker
    assert len(layer.image_markers) == 1
    assert layer.image_markers[0].file_path == "docs/images/logo.png"

    # Shape Markers
    assert len(layer.shape_markers) == 2
    assert layer.shape_markers[0].shape_type == "Rectangle"
    assert layer.shape_markers[0].bg_color == "#ff0000"
    assert layer.shape_markers[1].shape_type == "Ellipse"
    assert layer.shape_markers[1].bg_color == "#00ff00"


def test_roundtrip_with_qtiplot():
    """Test generating a project from QtiPlot and parsing it with QtiParser."""
    target_qti = os.path.abspath("build/roundtrip_test.qti")
    os.makedirs("build", exist_ok=True)
    if os.path.exists(target_qti):
        os.remove(target_qti)

    try:
        import qti
        app = qti.app
        t = app.newTable("RoundTripTable", 10, 2)
        t.setColName(1, "XVal")
        t.setColName(2, "YVal")
        for i in range(10):
            t.setCell(1, i + 1, float(i * 0.5))
            t.setCell(2, i + 1, float(np.sin(i * 0.5)))

        n = app.newNote("TestNote")
        n.currentEditor().setText("print('Hello QtiParser')")

        app.saveFolder(app.projectFolder(), target_qti, False)
    except ImportError:
        import subprocess
        script = f"""
import qti
import numpy as np

app = qti.app
t = app.newTable("RoundTripTable", 10, 2)
t.setColName(1, "XVal")
t.setColName(2, "YVal")
for i in range(10):
    t.setCell(1, i + 1, float(i * 0.5))
    t.setCell(2, i + 1, float(np.sin(i * 0.5)))

n = app.newNote("TestNote")
n.currentEditor().setText("print('Hello QtiParser')")

app.saveFolder(app.projectFolder(), r"{target_qti}", False)
"""
        script_path = "build/test_script_roundtrip.py"
        with open(script_path, "w", encoding="utf-8") as f:
            f.write(script)

        qtiplot_exe = "build/qtiplot/qtiplot.exe"
        if os.path.exists(qtiplot_exe):
            res = subprocess.run([qtiplot_exe, "-X", script_path], capture_output=True, text=True)
            assert res.returncode == 0

    if os.path.exists(target_qti):
        project = qtiparser.read_qti(target_qti)
        assert "RoundTripTable" in project.tables
        tbl = project.tables["RoundTripTable"]
        assert tbl.num_rows == 10
        assert tbl.num_cols == 2
        assert float(tbl.columns[0].data[0]) == 0.0
        assert float(tbl.columns[0].data[2]) == 1.0

        assert "TestNote" in project.notes
        note = project.notes["TestNote"]
        assert "Hello QtiParser" in note.content
