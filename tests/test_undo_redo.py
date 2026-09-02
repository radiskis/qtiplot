"""
End-to-End GUI Undo / Redo automated test suite for QtiPlot.
Tests that operations on Tables, Matrices, Notes, and GUI components
correctly push undo commands to QUndoStack and properly restore/reapply
states when triggering undo() and redo().
"""

import pytest
import numpy as np
import qti


def test_table_column_name_undo_redo():
    """Test undo and redo of column renaming in a Table."""
    app = qti.app
    t = app.newTable("TestUndoColName", 5, 2)
    assert t is not None

    # Initial column name is "1"
    initial_name = t.colName(1)
    assert initial_name == "1"

    # Rename column to "ModifiedX"
    t.setColName(1, "ModifiedX")
    assert t.colName(1) == "ModifiedX"

    # Undo renaming
    t.undo()
    assert t.colName(1) == initial_name

    # Redo renaming
    t.redo()
    assert t.colName(1) == "ModifiedX"


def test_table_cell_edit_undo_redo():
    """Test undo and redo of manual numerical cell entry."""
    app = qti.app
    t = app.newTable("TestUndoCellEdit", 5, 2)
    assert t is not None

    t.setCell(1, 1, 10.5)
    assert t.cell(1, 1) == 10.5

    # Edit cell value
    t.setCell(1, 1, 42.0)
    assert t.cell(1, 1) == 42.0

    # Undo
    t.undo()
    assert t.cell(1, 1) == 10.5

    # Redo
    t.redo()
    assert t.cell(1, 1) == 42.0


def test_table_two_cells_sequential_undo_redo():
    """Test entering 2 cells in a column: undoing only undoes the last cell, not both."""
    app = qti.app
    t = app.newTable("TestUndoTwoCells", 5, 2)
    assert t is not None

    # Enter cell 1 (row 1, col 1)
    t.setText(1, 1, "FirstVal")
    assert t.text(1, 1) == "FirstVal"

    # Enter cell 2 (row 2, col 1)
    t.setText(1, 2, "SecondVal")
    assert t.text(1, 2) == "SecondVal"

    # Undo ONCE: only the 2nd cell should be undone; 1st cell must remain "FirstVal"
    t.undo()
    assert t.text(1, 2) == ""
    assert t.text(1, 1) == "FirstVal"

    # Undo SECOND time: 1st cell is now undone
    t.undo()
    assert t.text(1, 1) == ""

    # Redo ONCE: 1st cell restored
    t.redo()
    assert t.text(1, 1) == "FirstVal"
    assert t.text(1, 2) == ""

    # Redo SECOND time: 2nd cell restored
    t.redo()
    assert t.text(1, 1) == "FirstVal"
    assert t.text(1, 2) == "SecondVal"


def test_table_delete_two_rows_undo_redo():
    """Test deleting 2 rows in a table and undoing restores BOTH rows with their data."""
    app = qti.app
    t = app.newTable("TestUndoTwoRows", 5, 2)
    assert t is not None

    t.setText(1, 1, "R1C1")
    t.setText(2, 1, "R1C2")
    t.setText(1, 2, "R2C1")
    t.setText(2, 2, "R2C2")

    # Delete 2 rows (row 1 and row 2)
    t.deleteRows(1, 2)
    assert t.numRows() == 3

    # Undo row deletion
    t.undo()
    assert t.numRows() == 5
    assert t.text(1, 1) == "R1C1"
    assert t.text(2, 1) == "R1C2"
    assert t.text(1, 2) == "R2C1"
    assert t.text(2, 2) == "R2C2"

    # Redo row deletion
    t.redo()
    assert t.numRows() == 3


def test_table_cell_text_entry_undo_redo():
    """Test undo and redo of entering arbitrary text string in table cell."""
    app = qti.app
    t = app.newTable("TestUndoCellText", 5, 2)
    assert t is not None

    t.setText(1, 1, "hello")
    assert t.text(1, 1) == "hello"

    t.setText(1, 1, "dsdf")
    assert t.text(1, 1) == "dsdf"

    # Undo
    t.undo()
    assert t.text(1, 1) == "hello"

    # Redo
    t.redo()
    assert t.text(1, 1) == "dsdf"


def test_table_clear_cell_undo_redo():
    """Test undo and redo of clearing a cell."""
    app = qti.app
    t = app.newTable("TestUndoClearCell", 5, 2)
    assert t is not None

    t.setText(1, 1, "value1")
    assert t.text(1, 1) == "value1"

    t.clearCell(0, 0)
    assert t.text(1, 1) == ""

    # Undo clearing
    t.undo()
    assert t.text(1, 1) == "value1"

    # Redo clearing
    t.redo()
    assert t.text(1, 1) == ""


def test_table_clear_selection_undo_redo():
    """Test undo and redo of clearing cell selection."""
    app = qti.app
    t = app.newTable("TestUndoClearSel", 5, 2)
    assert t is not None

    t.setText(1, 1, "A1")
    t.setText(1, 2, "A2")

    t.clearSelection()

    # Undo
    t.undo()
    # Values should be restored
    assert t.text(1, 1) == "A1" or t.text(1, 2) == "A2"


def test_table_column_comment_undo_redo():
    """Test undo and redo of column comments."""
    app = qti.app
    t = app.newTable("TestUndoComment", 5, 2)
    assert t is not None

    # Initial comment is empty
    initial_comment = t.comment(1)

    # Modify comment
    t.setComment(1, "UpdatedComment")
    assert t.comment(1) == "UpdatedComment"

    # Undo comment
    t.undo()
    assert t.comment(1) == initial_comment

    # Redo comment
    t.redo()
    assert t.comment(1) == "UpdatedComment"


def test_table_plot_designation_undo_redo():
    """Test undo and redo of column plot designation (X, Y, Z, Error)."""
    app = qti.app
    t = app.newTable("TestUndoPlotDes", 5, 2)
    assert t is not None

    initial_des = t.columnRole(1)
    assert initial_des == 1  # Table.PlotDesignation.X

    # Change role to Table.PlotDesignation.Y (2)
    t.setColumnRole(1, qti.Table.PlotDesignation.Y)
    assert t.columnRole(1) == 2

    # Undo
    t.undo()
    assert t.columnRole(1) == initial_des

    # Redo
    t.redo()
    assert t.columnRole(1) == 2


def test_table_add_column_undo_redo():
    """Test undo and redo of adding columns."""
    app = qti.app
    t = app.newTable("TestUndoAddCol", 5, 2)
    assert t is not None
    initial_cols = t.numCols()

    # Add a column
    t.addCol()
    assert t.numCols() == initial_cols + 1

    # Undo addition
    t.undo()
    assert t.numCols() == initial_cols

    # Redo addition
    t.redo()
    assert t.numCols() == initial_cols + 1


def test_table_column_sorting_undo_redo():
    """Test undo and redo of column sorting."""
    app = qti.app
    t = app.newTable("TestUndoSort", 5, 1)
    assert t is not None

    values = [50.0, 10.0, 40.0, 20.0, 30.0]
    for i, v in enumerate(values):
        t.setCell(1, i + 1, v)

    # Verify initial unsorted order
    assert t.cell(1, 1) == 50.0
    assert t.cell(1, 2) == 10.0

    # Sort ascending (0)
    t.sortColumn(1, 0)
    assert t.cell(1, 1) == 10.0
    assert t.cell(1, 2) == 20.0
    assert t.cell(1, 5) == 50.0

    # Undo sorting
    t.undo()
    assert t.cell(1, 1) == 50.0
    assert t.cell(1, 2) == 10.0
    assert t.cell(1, 5) == 30.0

    # Redo sorting
    t.redo()
    assert t.cell(1, 1) == 10.0
    assert t.cell(1, 2) == 20.0
    assert t.cell(1, 5) == 50.0


def test_matrix_coordinates_undo_redo():
    """Test undo and redo of Matrix coordinate bounds."""
    app = qti.app
    m = app.newMatrix("TestUndoCoords", 10, 10)
    assert m is not None

    # Set initial coordinates
    m.setCoordinates(0.0, 10.0, 0.0, 20.0)
    assert m.xStart() == 0.0
    assert m.xEnd() == 10.0
    assert m.yStart() == 0.0
    assert m.yEnd() == 20.0

    # Modify coordinates
    m.setCoordinates(-5.0, 5.0, -15.0, 15.0)
    assert m.xStart() == -5.0
    assert m.xEnd() == 5.0
    assert m.yStart() == -15.0
    assert m.yEnd() == 15.0

    # Undo
    m.undo()
    assert m.xStart() == 0.0
    assert m.xEnd() == 10.0
    assert m.yStart() == 0.0
    assert m.yEnd() == 20.0

    # Redo
    m.redo()
    assert m.xStart() == -5.0
    assert m.xEnd() == 5.0
    assert m.yStart() == -15.0
    assert m.yEnd() == 15.0


def test_matrix_transpose_undo_redo():
    """Test undo and redo of Matrix transpose operation."""
    app = qti.app
    m = app.newMatrix("TestUndoTranspose", 4, 4)
    assert m is not None

    m.setCell(1, 2, 42.5)
    assert m.cell(1, 2) == 42.5

    # Transpose
    m.transpose()
    assert m.cell(2, 1) == 42.5

    # Undo transpose
    m.undo()
    assert m.cell(1, 2) == 42.5

    # Redo transpose
    m.redo()
    assert m.cell(2, 1) == 42.5


def test_matrix_flip_and_rotate_undo_redo():
    """Test undo and redo of Matrix flip and rotate operations."""
    app = qti.app
    m = app.newMatrix("TestUndoFlipRotate", 4, 4)
    assert m is not None

    m.setCell(1, 1, 99.0)
    assert m.cell(1, 1) == 99.0

    # Flip horizontally
    m.flipHorizontally()
    assert m.cell(1, 4) == 99.0

    # Undo flip
    m.undo()
    assert m.cell(1, 1) == 99.0

    # Redo flip
    m.redo()
    assert m.cell(1, 4) == 99.0


def test_note_editor_undo_redo():
    """Test undo and redo inside a Note script editor."""
    app = qti.app
    n = app.newNote("TestUndoNote")
    assert n is not None

    editor = n.currentEditor()
    assert editor is not None

    editor.setText("Initial Script Text")
    assert "Initial Script Text" in editor.toPlainText()

    # Insert text into editor
    editor.insertPlainText("\n# Additional Line")
    assert "Additional Line" in editor.toPlainText()

    # Undo via editor
    editor.undo()
    assert "Additional Line" not in editor.toPlainText()

    # Redo via editor
    editor.redo()
    assert "Additional Line" in editor.toPlainText()


def test_table_delete_rows_undo_redo():
    """Test undo and redo of deleting rows from a Table."""
    app = qti.app
    t = app.newTable("TestUndoDeleteRows", 5, 2)
    assert t is not None

    t.setCell(1, 1, 10.0)
    t.setCell(1, 2, 20.0)
    t.setCell(1, 3, 30.0)
    assert t.numRows() == 5

    # Delete rows 2 to 4 (in 1-based indexing)
    t.deleteRows(2, 4)
    assert t.numRows() == 2

    # Undo row deletion
    t.undo()
    assert t.numRows() == 5
    assert t.cell(1, 2) == 20.0
    assert t.cell(1, 3) == 30.0

    # Redo row deletion
    t.redo()
    assert t.numRows() == 2


def test_table_remove_col_undo_redo():
    """Test undo and redo of removing a column."""
    app = qti.app
    t = app.newTable("TestUndoRemoveCol", 5, 3)
    assert t is not None
    assert t.numCols() == 3

    t.setCell(2, 1, 120.0)
    assert t.cell(2, 1) == 120.0

    # Remove column 2
    t.removeCol(2)
    assert t.numCols() == 2

    # Undo column removal
    t.undo()
    assert t.numCols() == 3
    assert t.cell(2, 1) == 120.0

    # Redo column removal
    t.redo()
    assert t.numCols() == 2


def test_matrix_dimensions_undo_redo():
    """Test undo and redo of matrix dimension changes."""
    app = qti.app
    m = app.newMatrix("TestUndoDimensions", 4, 4)
    assert m is not None
    assert m.numRows() == 4
    assert m.numCols() == 4

    m.setDimensions(8, 6)
    assert m.numRows() == 8
    assert m.numCols() == 6

    # Undo dimension change
    m.undo()
    assert m.numRows() == 4
    assert m.numCols() == 4

    # Redo dimension change
    m.redo()
    assert m.numRows() == 8
    assert m.numCols() == 6


def test_plot_scale_undo_redo():
    """Test undo and redo of plot axis scale ranges."""
    app = qti.app
    t = app.newTable("TestPlotScaleTable", 10, 2)
    for i in range(10):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float((i + 1) * 2))

    g = app.plot(t, "TestPlotScaleTable_2", 1)
    assert g is not None
    layer = g.activeLayer()
    assert layer is not None

    # Set scale on Bottom axis (2)
    layer.undoSetScale(2, 0.0, 100.0)
    # Undo scale
    layer.undo()
    # Redo scale
    layer.redo()


def test_plot_axis_title_undo_redo():
    """Test undo and redo of plot axis title."""
    app = qti.app
    t = app.newTable("TestPlotAxisTitleTable", 5, 2)
    for i in range(5):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float(i + 2))

    g = app.plot(t, "TestPlotAxisTitleTable_2", 1)
    layer = g.activeLayer()
    assert layer is not None

    layer.undoSetAxisTitle(2, "Initial Custom Title")
    assert layer.axisTitleString(2) == "Initial Custom Title"

    layer.undoSetAxisTitle(2, "Updated Custom Title")
    assert layer.axisTitleString(2) == "Updated Custom Title"

    # Undo
    layer.undo()
    assert layer.axisTitleString(2) == "Initial Custom Title"

    # Redo
    layer.redo()
    assert layer.axisTitleString(2) == "Updated Custom Title"


def test_plot_canvas_background_undo_redo():
    """Test undo and redo of plot canvas background color."""
    from PyQt6.QtGui import QColor

    app = qti.app
    t = app.newTable("TestPlotCanvasTable", 5, 2)
    for i in range(5):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float(i + 2))

    g = app.plot(t, "TestPlotCanvasTable_2", 1)
    layer = g.activeLayer()
    assert layer is not None

    orig_color = layer.canvasBackground().color()
    new_color = QColor(240, 230, 220)

    layer.undoSetCanvasBackground(new_color)
    assert layer.canvasBackground().color() == new_color

    # Undo
    layer.undo()
    assert layer.canvasBackground().color() == orig_color

    # Redo
    layer.redo()
    assert layer.canvasBackground().color() == new_color


def test_plot_grid_undo_redo():
    """Test undo and redo of plot grid visibility."""
    app = qti.app
    t = app.newTable("TestPlotGridTable", 5, 2)
    for i in range(5):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float(i + 2))

    g = app.plot(t, "TestPlotGridTable_2", 1)
    layer = g.activeLayer()
    assert layer is not None

    orig_state = layer.isGridEnabled(2)

    layer.undoShowGrid(2, not orig_state, False)
    assert layer.isGridEnabled(2) == (not orig_state)

    # Undo
    layer.undo()
    assert layer.isGridEnabled(2) == orig_state

    # Redo
    layer.redo()
    assert layer.isGridEnabled(2) == (not orig_state)


def test_plot_curve_pen_undo_redo():
    """Test undo and redo of curve pen styling."""
    from PyQt6.QtGui import QColor, QPen

    app = qti.app
    t = app.newTable("TestPlotCurvePenTable", 5, 2)
    for i in range(5):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float(i + 2))

    g = app.plot(t, "TestPlotCurvePenTable_2", 1)
    layer = g.activeLayer()
    assert layer is not None

    new_pen = QPen(QColor(255, 0, 0), 3)
    layer.undoSetCurvePen(0, new_pen)

    # Undo
    layer.undo()

    # Redo
    layer.redo()


def test_table_swap_columns_undo_redo():
    """Test undo and redo of column swapping in a Table."""
    app = qti.app
    t = app.newTable("TestSwapColsTable", 3, 2)
    t.setText(1, 1, "A1")
    t.setText(2, 1, "B1")

    # Swap column 1 and 2
    t.swapColumns(1, 2)
    assert t.text(1, 1) == "B1"
    assert t.text(2, 1) == "A1"

    # Undo
    t.undo()
    assert t.text(1, 1) == "A1"
    assert t.text(2, 1) == "B1"

    # Redo
    t.redo()
    assert t.text(1, 1) == "B1"
    assert t.text(2, 1) == "A1"


def test_table_set_column_width_undo_redo():
    """Test undo and redo of column width adjustments."""
    app = qti.app
    t = app.newTable("TestColWidthTable", 3, 2)
    orig_width = t.columnWidth(1)

    t.setColumnWidth(1, 145)
    assert t.columnWidth(1) == 145

    # Undo
    t.undo()
    assert t.columnWidth(1) == orig_width

    # Redo
    t.redo()
    assert t.columnWidth(1) == 145


def test_table_set_read_only_undo_redo():
    """Test undo and redo of toggling column read-only mode."""
    app = qti.app
    t = app.newTable("TestReadOnlyTable", 3, 2)
    assert not t.isReadOnlyColumn(1)

    t.setReadOnlyColumn(1, True)
    assert t.isReadOnlyColumn(1)

    # Undo
    t.undo()
    assert not t.isReadOnlyColumn(1)

    # Redo
    t.redo()
    assert t.isReadOnlyColumn(1)


def test_plot_title_undo_redo():
    """Test undo and redo of plot title changes."""
    app = qti.app
    t = app.newTable("TestPlotTitleTable", 5, 2)
    for i in range(5):
        t.setCell(1, i + 1, float(i + 1))
        t.setCell(2, i + 1, float(i + 2))

    g = app.plot(t, "TestPlotTitleTable_2", 1)
    layer = g.activeLayer()
    assert layer is not None

    orig_title = layer.plotTitle()
    layer.undoSetTitle("New Meaningful Title")
    assert layer.plotTitle() == "New Meaningful Title"

    # Undo
    layer.undo()
    assert layer.plotTitle() == orig_title

    # Redo
    layer.redo()
    assert layer.plotTitle() == "New Meaningful Title"



