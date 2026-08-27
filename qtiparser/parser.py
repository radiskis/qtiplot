"""
Comprehensive, complete parser engine for QtiPlot (.qti / .qti.gz) project files.
"""

import os
import gzip
import re
from typing import List, Dict, Tuple, Optional, Any
import numpy as np

from .models import (
    QtiProject, QtiFolder, QtiTable, QtiTableStatistics, QtiMatrix,
    QtiGraph, QtiLayer, QtiCurve, QtiFunctionCurve, QtiSpectrogram,
    QtiErrorBar, QtiAxisScale, QtiLegend, QtiLineMarker, QtiTexMarker,
    QtiImageMarker, QtiShapeMarker, QtiPolarGraph, QtiPolarCurve,
    QtiNote, QtiTab, QtiSurfacePlot, QtiColorMap, Column
)


def read_qti(filepath_or_buffer) -> QtiProject:
    """
    Parse a QtiPlot project file (.qti or .qti.gz) into a QtiProject model.
    """
    parser = QtiParser()
    return parser.parse(filepath_or_buffer)


class QtiParser:
    def __init__(self):
        self.project = QtiProject()
        self.current_folder: Optional[QtiFolder] = None
        self.folder_stack: List[QtiFolder] = []

    def parse(self, filepath_or_buffer) -> QtiProject:
        lines = self._read_lines(filepath_or_buffer)
        if not lines:
            return self.project

        # Initialize root folder
        base_name = "Project"
        if isinstance(filepath_or_buffer, str):
            base_name = os.path.splitext(os.path.basename(filepath_or_buffer))[0]
            if base_name.endswith(".qti"):
                base_name = os.path.splitext(base_name)[0]

        self.project.root_folder = QtiFolder(name=base_name)
        self.current_folder = self.project.root_folder
        self.folder_stack = [self.current_folder]

        line_idx = 0
        total_lines = len(lines)

        # 1. Parse Header
        line_idx = self._parse_header(lines, line_idx)

        # 2. Main Block Loop
        while line_idx < total_lines:
            line = lines[line_idx].strip()

            if not line:
                line_idx += 1
                continue

            if line.startswith("<folder>"):
                line_idx = self._parse_folder_start(lines, line_idx)
            elif line == "</folder>":
                self._parse_folder_end()
                line_idx += 1
            elif line.startswith("<open>"):
                val = line.replace("<open>", "").replace("</open>", "").strip()
                if self.current_folder and val.isdigit():
                    self.current_folder.is_open = (val == "1")
                line_idx += 1
            elif line == "<table>":
                line_idx = self._parse_table(lines, line_idx)
            elif line == "<TableStatistics>" or line.startswith("<TableStatistics>"):
                line_idx = self._parse_table_statistics(lines, line_idx)
            elif line == "<matrix>":
                line_idx = self._parse_matrix(lines, line_idx)
            elif line == "<multiLayer>":
                line_idx = self._parse_multilayer(lines, line_idx)
            elif line == "<PolarGraph>":
                line_idx = self._parse_polar_graph(lines, line_idx)
            elif line == "<note>":
                line_idx = self._parse_note(lines, line_idx)
            elif line == "<SurfacePlot>":
                line_idx = self._parse_surface_plot(lines, line_idx)
            elif line.startswith("<log>"):
                line_idx = self._parse_log(lines, line_idx)
            else:
                line_idx += 1

        return self.project

    def _read_lines(self, source) -> List[str]:
        if isinstance(source, str):
            is_gz = source.endswith(".gz")
            if not is_gz and os.path.exists(source):
                with open(source, "rb") as f:
                    magic = f.read(2)
                    is_gz = (magic == b'\x1f\x8b')

            if is_gz:
                with gzip.open(source, "rt", encoding="utf-8", errors="replace") as f:
                    return f.read().splitlines()
            else:
                with open(source, "r", encoding="utf-8", errors="replace") as f:
                    return f.read().splitlines()
        elif hasattr(source, "read"):
            content = source.read()
            if isinstance(content, bytes):
                content = content.decode("utf-8", errors="replace")
            return content.splitlines()
        return []

    def _parse_header(self, lines: List[str], idx: int) -> int:
        while idx < len(lines) and idx < 5:
            line = lines[idx].strip()
            if line.startswith("QtiPlot"):
                parts = line.split()
                if len(parts) >= 2:
                    self.project.version = parts[1]
            elif line.startswith("<scripting-lang>"):
                parts = line.split("\t")
                if len(parts) >= 2:
                    self.project.scripting_lang = parts[1].strip()
            elif line.startswith("<windows>"):
                parts = line.split("\t")
                if len(parts) >= 2 and parts[1].strip().isdigit():
                    self.project.window_count = int(parts[1].strip())
            else:
                break
            idx += 1
        return idx

    def _parse_folder_start(self, lines: List[str], idx: int) -> int:
        line = lines[idx]
        parts = line.split("\t")
        name = parts[1] if len(parts) > 1 else "Folder"
        birth = parts[2] if len(parts) > 2 else ""
        mod = parts[3] if len(parts) > 3 else ""
        is_active = (len(parts) > 4 and parts[4].strip() == "current")

        new_folder = QtiFolder(name=name, birth_date=birth, modification_date=mod, is_active=is_active)
        if self.current_folder:
            self.current_folder.subfolders.append(new_folder)
        self.folder_stack.append(new_folder)
        self.current_folder = new_folder
        return idx + 1

    def _parse_folder_end(self):
        if len(self.folder_stack) > 1:
            self.folder_stack.pop()
            self.current_folder = self.folder_stack[-1]

    def _parse_log(self, lines: List[str], idx: int) -> int:
        log_lines = []
        idx += 1
        while idx < len(lines):
            line = lines[idx]
            if "</log>" in line:
                break
            log_lines.append(line)
            idx += 1
        if self.current_folder:
            self.current_folder.log += "\n".join(log_lines)
        return idx + 1

    def _parse_table(self, lines: List[str], idx: int) -> int:
        idx += 1
        if idx >= len(lines):
            return idx

        meta_line = lines[idx]
        parts = meta_line.split("\t")
        name = parts[0] if parts else "Table"
        num_rows = int(parts[1]) if len(parts) > 1 and parts[1].isdigit() else 0
        num_cols = int(parts[2]) if len(parts) > 2 and parts[2].isdigit() else 0
        birth = parts[3] if len(parts) > 3 else ""

        table = QtiTable(name=name, num_rows=num_rows, num_cols=num_cols, birth_date=birth)
        idx += 1

        in_data = False
        in_com = False
        current_col_nr = None
        commands: Dict[int, str] = {}
        data_rows: List[List[Any]] = []

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</table>":
                idx += 1
                break

            if trimmed == "<data>":
                in_data = True
                idx += 1
                continue
            elif trimmed == "</data>":
                in_data = False
                idx += 1
                continue

            if trimmed == "<com>":
                in_com = True
                idx += 1
                continue
            elif trimmed == "</com>":
                in_com = False
                idx += 1
                continue

            if in_com:
                if trimmed.startswith('<col nr="'):
                    match = re.search(r'nr="(\d+)"', trimmed)
                    if match:
                        current_col_nr = int(match.group(1))
                elif trimmed == "</col>":
                    current_col_nr = None
                else:
                    if current_col_nr is not None:
                        commands[current_col_nr] = commands.get(current_col_nr, "") + line + "\n"
                idx += 1
                continue

            if in_data:
                row_parts = line.split("\t")
                if len(row_parts) > 1:
                    data_rows.append(row_parts[1:])
                idx += 1
                continue

            if line.startswith("geometry"):
                gparts = line.split("\t")
                if len(gparts) >= 5:
                    table.geometry = {
                        "x": int(gparts[1]), "y": int(gparts[2]),
                        "width": int(gparts[3]), "height": int(gparts[4]),
                        "status": gparts[5] if len(gparts) > 5 else ""
                    }
            elif line.startswith("header"):
                hparts = line.split("\t")[1:]
                for h in hparts:
                    if not h:
                        continue
                    match = re.match(r"^(.*?)(?:\[(.*?)\])?$", h)
                    col_name = match.group(1) if match else h
                    plot_type = match.group(2) if match and match.group(2) else ""
                    table.columns.append(Column(name=col_name, plot_type=plot_type))
            elif line.startswith("ColWidth"):
                wparts = line.split("\t")[1:]
                for c_idx, w in enumerate(wparts):
                    if c_idx < len(table.columns) and w.isdigit():
                        table.columns[c_idx].width = int(w)
            elif line.startswith("ColType"):
                tparts = line.split("\t")[1:]
                for c_idx, t in enumerate(tparts):
                    if c_idx < len(table.columns) and t:
                        sub = t.split(";")
                        table.columns[c_idx].col_type = int(sub[0]) if sub[0].isdigit() else 0
                        table.columns[c_idx].format_spec = sub[1] if len(sub) > 1 else "0/6"
            elif line.startswith("Comments"):
                cparts = line.split("\t")[1:]
                for c_idx, c in enumerate(cparts):
                    if c_idx < len(table.columns):
                        table.columns[c_idx].comment = c
            elif line.startswith("ReadOnlyColumn"):
                rparts = line.split("\t")[1:]
                for c_idx, r in enumerate(rparts):
                    if c_idx < len(table.columns):
                        table.columns[c_idx].is_read_only = (r == "1")
            elif line.startswith("HiddenColumn"):
                hparts = line.split("\t")[1:]
                for c_idx, h in enumerate(hparts):
                    if c_idx < len(table.columns):
                        table.columns[c_idx].is_hidden = (h == "1")
            elif line.startswith("WindowLabel"):
                lparts = line.split("\t")
                if len(lparts) > 1:
                    table.window_label = lparts[1]
                if len(lparts) > 2 and lparts[2].isdigit():
                    table.caption_policy = int(lparts[2])

            idx += 1

        for c_idx, cmd in commands.items():
            if c_idx < len(table.columns):
                table.columns[c_idx].command = cmd.strip()

        for c_idx, col in enumerate(table.columns):
            col_cells = []
            for row in data_rows:
                val = row[c_idx] if c_idx < len(row) else ""
                col_cells.append(val)
            col.data = col_cells

        if self.current_folder:
            self.current_folder.tables.append(table)
        self.project.tables[table.name] = table
        return idx

    def _parse_table_statistics(self, lines: List[str], idx: int) -> int:
        idx += 1
        if idx >= len(lines):
            return idx

        meta_line = lines[idx]
        parts = meta_line.split("\t")
        name = parts[0] if parts else "TableStatistics"
        base_name = parts[1] if len(parts) > 1 else ""
        stat_type = parts[2] if len(parts) > 2 else "col"
        birth = parts[3] if len(parts) > 3 else ""

        stats = QtiTableStatistics(
            name=name, base_table_name=base_name, stat_type=stat_type, birth_date=birth
        )
        idx += 1

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</TableStatistics>":
                idx += 1
                break

            if line.startswith("Targets"):
                tparts = line.split("\t")[1:]
                stats.targets = [int(p) for p in tparts if p.isdigit()]
            elif line.startswith("Range"):
                rparts = line.split("\t")[1:]
                if len(rparts) >= 2:
                    stats.start_idx = int(rparts[0]) if rparts[0].isdigit() else 0
                    stats.end_idx = int(rparts[1]) if rparts[1].isdigit() else 0
            elif line.startswith("header"):
                hparts = line.split("\t")[1:]
                for h in hparts:
                    if not h:
                        continue
                    match = re.match(r"^(.*?)(?:\[(.*?)\])?$", h)
                    col_name = match.group(1) if match else h
                    plot_type = match.group(2) if match and match.group(2) else ""
                    stats.columns.append(Column(name=col_name, plot_type=plot_type))

            idx += 1

        if self.current_folder:
            self.current_folder.tables.append(stats)
        self.project.tables[stats.name] = stats
        return idx

    def _parse_matrix(self, lines: List[str], idx: int) -> int:
        idx += 1
        if idx >= len(lines):
            return idx

        meta_line = lines[idx]
        parts = meta_line.split("\t")
        name = parts[0] if parts else "Matrix"
        num_rows = int(parts[1]) if len(parts) > 1 and parts[1].isdigit() else 0
        num_cols = int(parts[2]) if len(parts) > 2 and parts[2].isdigit() else 0
        birth = parts[3] if len(parts) > 3 else ""

        matrix = QtiMatrix(name=name, num_rows=num_rows, num_cols=num_cols, birth_date=birth)
        idx += 1

        in_data = False
        in_formula = False
        formula_lines = []
        data_rows: List[List[float]] = []

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</matrix>":
                idx += 1
                break

            if trimmed == "<data>":
                in_data = True
                idx += 1
                continue
            elif trimmed == "</data>":
                in_data = False
                idx += 1
                continue

            if trimmed == "<formula>":
                in_formula = True
                idx += 1
                continue
            elif trimmed == "</formula>":
                in_formula = False
                matrix.formula = "\n".join(formula_lines)
                idx += 1
                continue

            if in_formula:
                formula_lines.append(line)
                idx += 1
                continue

            if in_data:
                row_parts = line.split("\t")
                if len(row_parts) > 1:
                    row_floats = []
                    for v in row_parts[1:]:
                        try:
                            row_floats.append(float(v) if v else np.nan)
                        except ValueError:
                            row_floats.append(np.nan)
                    data_rows.append(row_floats)
                idx += 1
                continue

            if line.startswith("Coordinates"):
                cparts = line.split("\t")[1:]
                if len(cparts) >= 4:
                    matrix.x_start = float(cparts[0])
                    matrix.x_end = float(cparts[1])
                    matrix.y_start = float(cparts[2])
                    matrix.y_end = float(cparts[3])
            elif line.startswith("ViewType"):
                parts = line.split("\t")
                if len(parts) > 1 and parts[1].isdigit():
                    matrix.view_type = int(parts[1])
            elif line.startswith("HeaderViewType"):
                parts = line.split("\t")
                if len(parts) > 1 and parts[1].isdigit():
                    matrix.header_view_type = int(parts[1])
            elif line.startswith("ColorPolicy"):
                parts = line.split("\t")
                if len(parts) > 1 and parts[1].isdigit():
                    matrix.color_policy = int(parts[1])
            elif line.startswith("<xLabel>"):
                matrix.x_label = line.replace("<xLabel>", "").replace("</xLabel>", "").strip()
            elif line.startswith("<yLabel>"):
                matrix.y_label = line.replace("<yLabel>", "").replace("</yLabel>", "").strip()
            elif line.startswith("<zLabel>"):
                matrix.z_label = line.replace("<zLabel>", "").replace("</zLabel>", "").strip()
            elif line.startswith("<xUnit>"):
                matrix.x_unit = line.replace("<xUnit>", "").replace("</xUnit>", "").strip()
            elif line.startswith("<yUnit>"):
                matrix.y_unit = line.replace("<yUnit>", "").replace("</yUnit>", "").strip()
            elif line.startswith("<zUnit>"):
                matrix.z_unit = line.replace("<zUnit>", "").replace("</zUnit>", "").strip()
            elif line.startswith("<xComment>"):
                matrix.x_comment = line.replace("<xComment>", "").replace("</xComment>", "").strip()
            elif line.startswith("<yComment>"):
                matrix.y_comment = line.replace("<yComment>", "").replace("</yComment>", "").strip()
            elif line.startswith("<zComment>"):
                matrix.z_comment = line.replace("<zComment>", "").replace("</zComment>", "").strip()

            idx += 1

        if data_rows:
            matrix.data = np.array(data_rows, dtype=float)
        else:
            matrix.data = np.empty((matrix.num_rows, matrix.num_cols))

        if self.current_folder:
            self.current_folder.matrices.append(matrix)
        self.project.matrices[matrix.name] = matrix
        return idx

    def _parse_multilayer(self, lines: List[str], idx: int) -> int:
        idx += 1
        if idx >= len(lines):
            return idx

        meta_line = lines[idx]
        parts = meta_line.split("\t")
        name = parts[0] if parts else "Graph"
        cols = int(parts[1]) if len(parts) > 1 and parts[1].isdigit() else 1
        rows = int(parts[2]) if len(parts) > 2 and parts[2].isdigit() else 1
        birth = parts[3] if len(parts) > 3 else ""

        graph = QtiGraph(name=name, cols=cols, rows=rows, birth_date=birth)
        idx += 1

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</multiLayer>":
                idx += 1
                break

            if trimmed == "<waterfall>1</waterfall>":
                graph.is_waterfall = True
            elif trimmed == "<graph>":
                layer, idx = self._parse_graph_layer(lines, idx)
                graph.layers.append(layer)
                continue
            elif line.startswith("geometry"):
                gparts = line.split("\t")
                if len(gparts) >= 5:
                    graph.geometry = {
                        "x": int(gparts[1]), "y": int(gparts[2]),
                        "width": int(gparts[3]), "height": int(gparts[4])
                    }

            idx += 1

        if self.current_folder:
            self.current_folder.graphs.append(graph)
        self.project.graphs[graph.name] = graph
        return idx

    def _parse_graph_layer(self, lines: List[str], idx: int) -> Tuple[QtiLayer, int]:
        layer = QtiLayer()
        idx += 1

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</graph>":
                idx += 1
                break

            if line.startswith("PlotTitle"):
                parts = line.split("\t")
                layer.title = parts[1] if len(parts) > 1 else ""
            elif line.startswith("Background"):
                parts = line.split("\t")
                if len(parts) > 1:
                    layer.background_color = parts[1]
            elif line.startswith("AxesTitles"):
                parts = line.split("\t")[1:]
                for axis_id, title in enumerate(parts):
                    layer.axes_titles[axis_id] = title
            elif line.startswith("scale"):
                parts = line.split("\t")
                if len(parts) >= 4:
                    axis_id = int(parts[1])
                    min_val = float(parts[2])
                    max_val = float(parts[3])
                    step = float(parts[4]) if len(parts) > 4 else 0.1
                    scale_type = int(parts[7]) if len(parts) > 7 and parts[7].isdigit() else 0
                    layer.scales[axis_id] = QtiAxisScale(
                        axis_id=axis_id, min_val=min_val, max_val=max_val,
                        step=step, scale_type=scale_type
                    )
            elif line.startswith("curve"):
                parts = line.split("\t")
                if len(parts) >= 3:
                    x_src = parts[1]
                    y_src = parts[2]
                    c_type = int(parts[3]) if len(parts) > 3 and parts[3].isdigit() else 0
                    p_color = "#000000"
                    p_width = 1.0
                    
                    for token in parts[4:]:
                        if token.startswith("#"):
                            p_color = token.split(",")[0]
                        else:
                            try:
                                val = float(token)
                                if 0.1 <= val <= 50.0 and p_width == 1.0:
                                    p_width = val
                            except ValueError:
                                pass

                    layer.curves.append(QtiCurve(
                        x_source=x_src, y_source=y_src, curve_type=c_type,
                        pen_color=p_color, pen_width=p_width, raw_params=parts
                    ))
            elif line.startswith("ErrorBars"):
                parts = line.split("\t")
                if len(parts) >= 5:
                    c_idx = int(parts[1]) if parts[1].isdigit() else 0
                    x_src = parts[2]
                    y_src = parts[3]
                    err_src = parts[4]
                    direction = int(parts[5]) if len(parts) > 5 and parts[5].isdigit() else 1
                    color = parts[7] if len(parts) > 7 else "#000000"
                    layer.error_bars.append(QtiErrorBar(
                        curve_index=c_idx, x_source=x_src, y_source=y_src,
                        err_source=err_src, direction=direction, color=color
                    ))
            elif trimmed == "<Function>":
                func_curve, idx = self._parse_function_curve(lines, idx)
                layer.function_curves.append(func_curve)
                continue
            elif trimmed == "<spectrogram>":
                spectro, idx = self._parse_spectrogram(lines, idx)
                layer.spectrograms.append(spectro)
                continue
            elif trimmed.startswith("<line>") and trimmed.endswith("</line>"):
                raw = trimmed.replace("<line>", "").replace("</line>", "").strip()
                parts = [p for p in raw.split("\t") if p != ""]
                if len(parts) >= 5:
                    layer.line_markers.append(QtiLineMarker(
                        start_x=float(parts[0]),
                        start_y=float(parts[1]) if len(parts) > 1 else 0.0,
                        end_x=float(parts[2]) if len(parts) > 2 else 0.0,
                        end_y=float(parts[3]) if len(parts) > 3 else 0.0,
                        width=float(parts[4]) if len(parts) > 4 else 1.0,
                        color=parts[5] if len(parts) > 5 else "#000000"
                    ))
            elif line.startswith("Legend") or trimmed == "<Legend>":
                if trimmed == "<Legend>":
                    leg, idx = self._parse_legend_block(lines, idx)
                    layer.legends.append(leg)
                    continue
                else:
                    parts = line.split("\t")
                    if len(parts) >= 3:
                        x = float(parts[1])
                        y = float(parts[2])
                        text = parts[-1] if len(parts) > 3 else ""
                        font = parts[3] if len(parts) > 3 else "Segoe UI"
                        size = int(parts[4]) if len(parts) > 4 and parts[4].isdigit() else 10
                        layer.legends.append(QtiLegend(x=x, y=y, text=text, font_family=font, font_size=size))
            elif trimmed == "<TexFormula>":
                tex, idx = self._parse_tex_marker(lines, idx)
                layer.tex_markers.append(tex)
                continue
            elif trimmed == "<Image>":
                img, idx = self._parse_image_marker(lines, idx)
                layer.image_markers.append(img)
                continue
            elif trimmed == "<Rectangle>" or trimmed == "<Ellipse>":
                shape, idx = self._parse_shape_marker(lines, idx, "Rectangle" if trimmed == "<Rectangle>" else "Ellipse")
                layer.shape_markers.append(shape)
                continue

            idx += 1

        return layer, idx

    def _parse_function_curve(self, lines: List[str], idx: int) -> Tuple[QtiFunctionCurve, int]:
        fc = QtiFunctionCurve()
        idx += 1
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</Function>":
                idx += 1
                break
            if line.startswith("<Type>"):
                fc.function_type = int(line.replace("<Type>", "").replace("</Type>", ""))
            elif line.startswith("<Title>"):
                fc.title = line.replace("<Title>", "").replace("</Title>", "")
            elif line.startswith("<Expression>"):
                fc.formulas = line.replace("<Expression>", "").replace("</Expression>", "").split("\t")
            elif line.startswith("<Variable>"):
                fc.variable = line.replace("<Variable>", "").replace("</Variable>", "")
            elif line.startswith("<Range>"):
                rparts = line.replace("<Range>", "").replace("</Range>", "").split("\t")
                if len(rparts) >= 2:
                    fc.from_val = float(rparts[0])
                    fc.to_val = float(rparts[1])
            elif line.startswith("<Points>"):
                fc.points = int(line.replace("<Points>", "").replace("</Points>", ""))
            elif line.startswith("<Constant>"):
                cparts = line.replace("<Constant>", "").replace("</Constant>", "").split("\t")
                if len(cparts) >= 2:
                    fc.constants[cparts[0]] = float(cparts[1])
            idx += 1
        return fc, idx

    def _parse_spectrogram(self, lines: List[str], idx: int) -> Tuple[QtiSpectrogram, int]:
        spec = QtiSpectrogram(matrix_name="")
        idx += 1
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</spectrogram>":
                idx += 1
                break
            if line.startswith("<matrix>"):
                spec.matrix_name = line.replace("<matrix>", "").replace("</matrix>", "")
            elif line.startswith("<xAxis>"):
                spec.x_axis = int(line.replace("<xAxis>", "").replace("</xAxis>", ""))
            elif line.startswith("<yAxis>"):
                spec.y_axis = int(line.replace("<yAxis>", "").replace("</yAxis>", ""))
            elif line.startswith("<Image>"):
                spec.image_mode = (line.replace("<Image>", "").replace("</Image>", "") == "1")
            elif line.startswith("<ContourLines>"):
                spec.contour_lines = (line.replace("<ContourLines>", "").replace("</ContourLines>", "") == "1")
            elif line.startswith("<z>"):
                spec.contour_levels.append(float(line.replace("<z>", "").replace("</z>", "")))
            elif line.startswith("<ColorPolicy>"):
                spec.color_policy = int(line.replace("<ColorPolicy>", "").replace("</ColorPolicy>", ""))
            idx += 1
        return spec, idx

    def _parse_legend_block(self, lines: List[str], idx: int) -> Tuple[QtiLegend, int]:
        leg = QtiLegend()
        idx += 1
        text_lines = []
        in_text = False
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</Legend>":
                idx += 1
                break
            if line == "<Text>":
                in_text = True
                idx += 1
                continue
            elif line == "</Text>":
                in_text = False
                leg.text = "\n".join(text_lines)
                idx += 1
                continue
            if in_text:
                text_lines.append(lines[idx])
                idx += 1
                continue
            if line.startswith("<x>"):
                leg.x = float(line.replace("<x>", "").replace("</x>", ""))
            elif line.startswith("<y>"):
                leg.y = float(line.replace("<y>", "").replace("</y>", ""))
            elif line.startswith("<Color>"):
                leg.color = line.replace("<Color>", "").replace("</Color>", "")
            elif line.startswith("<Background>"):
                leg.bg_color = line.replace("<Background>", "").replace("</Background>", "")
            idx += 1
        return leg, idx

    def _parse_tex_marker(self, lines: List[str], idx: int) -> Tuple[QtiTexMarker, int]:
        tex = QtiTexMarker()
        idx += 1
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</TexFormula>":
                idx += 1
                break
            if line.startswith("<tex>"):
                tex.formula = line.replace("<tex>", "").replace("</tex>", "")
            elif line.startswith("<x>"):
                tex.x = float(line.replace("<x>", "").replace("</x>", ""))
            elif line.startswith("<y>"):
                tex.y = float(line.replace("<y>", "").replace("</y>", ""))
            idx += 1
        return tex, idx

    def _parse_image_marker(self, lines: List[str], idx: int) -> Tuple[QtiImageMarker, int]:
        img = QtiImageMarker()
        idx += 1
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</Image>":
                idx += 1
                break
            if line.startswith("<path>"):
                img.file_path = line.replace("<path>", "").replace("</path>", "")
            elif line.startswith("<x>"):
                img.x = float(line.replace("<x>", "").replace("</x>", ""))
            elif line.startswith("<y>"):
                img.y = float(line.replace("<y>", "").replace("</y>", ""))
            idx += 1
        return img, idx

    def _parse_shape_marker(self, lines: List[str], idx: int, shape_type: str) -> Tuple[QtiShapeMarker, int]:
        shape = QtiShapeMarker(shape_type=shape_type)
        idx += 1
        end_tag = f"</{shape_type}>"
        while idx < len(lines):
            line = lines[idx].strip()
            if line == end_tag:
                idx += 1
                break
            if line.startswith("<x>"):
                shape.x = float(line.replace("<x>", "").replace("</x>", ""))
            elif line.startswith("<y>"):
                shape.y = float(line.replace("<y>", "").replace("</y>", ""))
            elif line.startswith("<right>"):
                shape.right = float(line.replace("<right>", "").replace("</right>", ""))
            elif line.startswith("<bottom>"):
                shape.bottom = float(line.replace("<bottom>", "").replace("</bottom>", ""))
            elif line.startswith("<Color>"):
                shape.color = line.replace("<Color>", "").replace("</Color>", "")
            elif line.startswith("<Background>"):
                shape.bg_color = line.replace("<Background>", "").replace("</Background>", "")
            elif line.startswith("<Alpha>"):
                shape.alpha = int(line.replace("<Alpha>", "").replace("</Alpha>", ""))
            idx += 1
        return shape, idx

    def _parse_polar_graph(self, lines: List[str], idx: int) -> int:
        idx += 1
        name = "PolarGraph"
        polar = QtiPolarGraph(name=name)

        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</PolarGraph>":
                idx += 1
                break

            if line.startswith("<Name>"):
                polar.name = line.replace("<Name>", "").replace("</Name>", "")
            elif line.startswith("geometry"):
                gparts = line.split("\t")
                if len(gparts) >= 5:
                    polar.geometry = {
                        "x": int(gparts[1]), "y": int(gparts[2]),
                        "width": int(gparts[3]), "height": int(gparts[4])
                    }
            elif line == "<Curve>":
                curve, idx = self._parse_polar_curve(lines, idx)
                polar.curves.append(curve)
                continue

            idx += 1

        if self.current_folder:
            self.current_folder.polar_graphs.append(polar)
        self.project.polar_graphs[polar.name] = polar
        return idx

    def _parse_polar_curve(self, lines: List[str], idx: int) -> Tuple[QtiPolarCurve, int]:
        c = QtiPolarCurve(table_name="", r_col_name="", theta_col_name="")
        idx += 1
        while idx < len(lines):
            line = lines[idx].strip()
            if line == "</Curve>":
                idx += 1
                break
            if line.startswith("<Table>"):
                c.table_name = line.replace("<Table>", "").replace("</Table>", "")
            elif line.startswith("<RCol>"):
                c.r_col_name = line.replace("<RCol>", "").replace("</RCol>", "")
            elif line.startswith("<ThetaCol>"):
                c.theta_col_name = line.replace("<ThetaCol>", "").replace("</ThetaCol>", "")
            elif line.startswith("<LineColor>"):
                c.color = line.replace("<LineColor>", "").replace("</LineColor>", "")
            elif line.startswith("<LineWidth>"):
                c.width = float(line.replace("<LineWidth>", "").replace("</LineWidth>", ""))
            idx += 1
        return c, idx

    def _parse_note(self, lines: List[str], idx: int) -> int:
        idx += 1
        if idx >= len(lines):
            return idx

        meta_line = lines[idx]
        parts = meta_line.split("\t")
        name = parts[0] if parts else "Note"
        birth = parts[1] if len(parts) > 1 else ""

        note = QtiNote(name=name, birth_date=birth)
        idx += 1

        in_tab = False
        in_content = False
        tab_active = False
        tab_title = "Tab"
        content_lines = []

        while idx < len(lines):
            line = lines[idx]
            trimmed = line.strip()

            if trimmed == "</note>":
                idx += 1
                break

            if trimmed == "<tab>":
                in_tab = True
                tab_active = False
                tab_title = "Tab"
                content_lines = []
                idx += 1
                continue
            elif trimmed == "</tab>":
                in_tab = False
                note.tabs.append(QtiTab(
                    title=tab_title,
                    content="\n".join(content_lines),
                    is_active=tab_active
                ))
                idx += 1
                continue

            if in_tab:
                if trimmed == "<active>1</active>":
                    tab_active = True
                elif trimmed.startswith("<title>"):
                    tab_title = trimmed.replace("<title>", "").replace("</title>", "")
                elif trimmed == "<content>":
                    in_content = True
                elif trimmed == "</content>":
                    in_content = False
                elif in_content:
                    content_lines.append(line)
                idx += 1
                continue

            if line.startswith("AutoExec"):
                parts = line.split("\t")
                if len(parts) > 1:
                    note.auto_exec = (parts[1] == "1")

            idx += 1

        if self.current_folder:
            self.current_folder.notes.append(note)
        self.project.notes[note.name] = note
        return idx

    def _parse_surface_plot(self, lines: List[str], idx: int) -> int:
        idx += 1
        name = "SurfacePlot"
        surface = QtiSurfacePlot(name=name)
        while idx < len(lines):
            line = lines[idx]
            if line.strip() == "</SurfacePlot>":
                idx += 1
                break
            idx += 1
        if self.current_folder:
            self.current_folder.surfaces.append(surface)
        self.project.surfaces[name] = surface
        return idx
