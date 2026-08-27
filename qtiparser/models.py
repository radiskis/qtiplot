"""
Comprehensive Data Models representing all QtiPlot project structures.
"""

from dataclasses import dataclass, field
from typing import List, Dict, Optional, Any
import numpy as np


@dataclass
class Column:
    """Represents a column in a QtiPlot data table."""
    name: str
    plot_type: str = ""          # "X", "Y", "Z", "xEr", "yEr", "L", or ""
    width: int = 100
    col_type: int = 0            # 0=Numeric, 1=Text, 2=Date, 3=Time, etc.
    format_spec: str = "0/6"
    comment: str = ""
    command: str = ""            # Formula calculation
    is_read_only: bool = False
    is_hidden: bool = False
    data: List[Any] = field(default_factory=list)


@dataclass
class QtiTable:
    """Represents a QtiPlot spreadsheet table."""
    name: str
    num_rows: int = 0
    num_cols: int = 0
    birth_date: str = ""
    window_label: str = ""
    caption_policy: int = 0
    geometry: Dict[str, Any] = field(default_factory=dict)
    columns: List[Column] = field(default_factory=list)

    def column(self, name_or_idx) -> Optional[Column]:
        if isinstance(name_or_idx, int):
            if 0 <= name_or_idx < len(self.columns):
                return self.columns[name_or_idx]
            return None
        for col in self.columns:
            if col.name == name_or_idx:
                return col
        return None

    def to_dataframe(self):
        """Convert table data to a Pandas DataFrame."""
        try:
            import pandas as pd
            data_dict = {}
            for col in self.columns:
                col_data = col.data
                if col.col_type == 0:
                    converted = []
                    for val in col_data:
                        try:
                            converted.append(float(val) if val != "" and val is not None else np.nan)
                        except (ValueError, TypeError):
                            converted.append(val)
                    data_dict[col.name] = converted
                else:
                    data_dict[col.name] = col_data
            return pd.DataFrame(data_dict)
        except ImportError:
            raise RuntimeError("pandas is required for to_dataframe(). Install via 'pip install pandas'.")

    def to_numpy(self) -> np.ndarray:
        """Convert numeric columns to a 2D NumPy array."""
        cols = []
        for col in self.columns:
            col_floats = []
            for val in col.data:
                try:
                    col_floats.append(float(val) if val != "" and val is not None else np.nan)
                except (ValueError, TypeError):
                    col_floats.append(np.nan)
            cols.append(col_floats)
        return np.column_stack(cols) if cols else np.empty((0, 0))


@dataclass
class QtiTableStatistics(QtiTable):
    """Represents a statistics table generated from a base table."""
    base_table_name: str = ""
    stat_type: str = "col"       # "row" or "col"
    targets: List[int] = field(default_factory=list)
    start_idx: int = 0
    end_idx: int = 0


@dataclass
class QtiColorMap:
    """Represents a discrete or continuous color palette/colormap."""
    mode: int = 0                # 0=ColorMap, 1=Indexed
    levels: List[float] = field(default_factory=list)
    colors: List[str] = field(default_factory=list)


@dataclass
class QtiMatrix:
    """Represents a QtiPlot 2D matrix grid."""
    name: str
    num_rows: int = 0
    num_cols: int = 0
    birth_date: str = ""
    window_label: str = ""
    caption_policy: int = 0
    formula: str = ""
    x_start: float = 0.0
    x_end: float = 1.0
    y_start: float = 0.0
    y_end: float = 1.0
    x_label: str = ""
    y_label: str = ""
    z_label: str = ""
    x_unit: str = ""
    y_unit: str = ""
    z_unit: str = ""
    x_comment: str = ""
    y_comment: str = ""
    z_comment: str = ""
    view_type: int = 0           # 0=Table, 1=Image
    header_view_type: int = 0    # 0=RowCol, 1=XYCoordinates
    color_policy: int = 0
    color_map: Optional[QtiColorMap] = None
    geometry: Dict[str, Any] = field(default_factory=dict)
    data: Optional[np.ndarray] = None


@dataclass
class QtiCurve:
    """Represents a plotted curve series in a 2D graph layer."""
    x_source: str
    y_source: str
    curve_type: int = 0          # 0=Line, 1=Scatter, 2=LineSymbol, 3=VerticalBars, etc.
    pen_color: str = "#000000"
    pen_width: float = 1.0
    pen_style: int = 0
    symbol_style: int = 0
    symbol_size: int = 8
    symbol_color: str = "#000000"
    symbol_fill: str = "#ffffff"
    is_visible: bool = True
    raw_params: List[str] = field(default_factory=list)


@dataclass
class QtiFunctionCurve:
    """Represents an analytical mathematical function curve in a 2D graph layer."""
    title: str = ""
    function_type: int = 0       # 0=Normal/Cartesian, 1=Parametric, 2=Polar
    formulas: List[str] = field(default_factory=list)
    variable: str = "x"
    from_val: float = 0.0
    to_val: float = 1.0
    points: int = 100
    is_log10: bool = False
    constants: Dict[str, float] = field(default_factory=dict)
    pen_color: str = "#000000"
    pen_width: float = 1.0


@dataclass
class QtiSpectrogram:
    """Represents a 2D matrix spectrogram / heatmap / contour plot."""
    matrix_name: str
    use_matrix_formula: bool = False
    x_axis: int = 0
    y_axis: int = 1
    image_mode: bool = True
    contour_lines: bool = False
    contour_levels: List[float] = field(default_factory=list)
    color_policy: int = 0
    color_map: Optional[QtiColorMap] = None


@dataclass
class QtiErrorBar:
    """Represents an error bar item attached to a curve."""
    curve_index: int
    x_source: str
    y_source: str
    err_source: str
    direction: int = 1           # 0=Horizontal, 1=Vertical, 2=Both
    width: float = 1.0
    color: str = "#000000"
    cap_width: int = 8
    through: bool = True
    plus: bool = True
    minus: bool = True


@dataclass
class QtiAxisScale:
    """Represents the range and scale type of an axis."""
    axis_id: int                 # 0=Bottom, 1=Left, 2=Top, 3=Right
    min_val: float = 0.0
    max_val: float = 1.0
    step: float = 0.1
    major_ticks: int = 5
    minor_ticks: int = 5
    scale_type: int = 0          # 0=Linear, 1=Log10, 2=Ln, 3=Log2, 4=Reciprocal, etc.
    inverted: bool = False


@dataclass
class QtiLineMarker:
    """Represents an arrow or line annotation marker."""
    start_x: float = 0.0
    start_y: float = 0.0
    end_x: float = 0.0
    end_y: float = 0.0
    width: float = 1.0
    color: str = "#000000"
    style: int = 0
    has_end_arrow: bool = True
    has_start_arrow: bool = False
    head_length: int = 10
    head_angle: int = 30
    filled_arrow_head: bool = True


@dataclass
class QtiLegend:
    """Represents a legend or text box inside a graph layer."""
    x: float = 0.0
    y: float = 0.0
    text: str = ""
    font_family: str = "Segoe UI"
    font_size: int = 10
    color: str = "#000000"
    bg_color: str = "#ffffff"
    frame_type: int = 0


@dataclass
class QtiTexMarker:
    """Represents a LaTeX rendered math formula annotation."""
    x: float = 0.0
    y: float = 0.0
    formula: str = ""
    frame_type: int = 0


@dataclass
class QtiImageMarker:
    """Represents an embedded raster image annotation."""
    x: float = 0.0
    y: float = 0.0
    file_path: str = ""


@dataclass
class QtiShapeMarker:
    """Represents a geometric rectangle or ellipse annotation."""
    shape_type: str              # "Rectangle" or "Ellipse"
    x: float = 0.0
    y: float = 0.0
    right: float = 0.0
    bottom: float = 0.0
    color: str = "#000000"
    bg_color: str = "#ffffff"
    alpha: int = 255


@dataclass
class QtiLayer:
    """Represents an individual 2D plot layer inside a MultiLayer graph."""
    title: str = ""
    geometry: Dict[str, Any] = field(default_factory=dict)
    background_color: str = "#ffffff"
    axes_titles: Dict[int, str] = field(default_factory=dict)
    scales: Dict[int, QtiAxisScale] = field(default_factory=dict)
    curves: List[QtiCurve] = field(default_factory=list)
    function_curves: List[QtiFunctionCurve] = field(default_factory=list)
    spectrograms: List[QtiSpectrogram] = field(default_factory=list)
    error_bars: List[QtiErrorBar] = field(default_factory=list)
    legends: List[QtiLegend] = field(default_factory=list)
    line_markers: List[QtiLineMarker] = field(default_factory=list)
    tex_markers: List[QtiTexMarker] = field(default_factory=list)
    image_markers: List[QtiImageMarker] = field(default_factory=list)
    shape_markers: List[QtiShapeMarker] = field(default_factory=list)
    grid_enabled: bool = False
    antialiasing: bool = True


@dataclass
class QtiGraph:
    """Represents a MultiLayer 2D graph window."""
    name: str
    cols: int = 1
    rows: int = 1
    birth_date: str = ""
    window_label: str = ""
    geometry: Dict[str, Any] = field(default_factory=dict)
    layers: List[QtiLayer] = field(default_factory=list)
    is_waterfall: bool = False


@dataclass
class QtiPolarCurve:
    """Represents a curve plotted in a polar coordinate graph."""
    table_name: str
    r_col_name: str
    theta_col_name: str
    color: str = "#000000"
    width: float = 1.0


@dataclass
class QtiPolarGraph:
    """Represents a Polar Coordinate Plot window."""
    name: str
    birth_date: str = ""
    geometry: Dict[str, Any] = field(default_factory=dict)
    radius_scale: Dict[str, Any] = field(default_factory=dict)
    azimuth_scale: Dict[str, Any] = field(default_factory=dict)
    curves: List[QtiPolarCurve] = field(default_factory=list)


@dataclass
class QtiTab:
    """Represents a tab inside a multi-tab script Note window."""
    title: str
    content: str
    is_active: bool = False


@dataclass
class QtiNote:
    """Represents a script note window."""
    name: str
    birth_date: str = ""
    window_label: str = ""
    auto_exec: bool = False
    line_numbers: bool = True
    tabs: List[QtiTab] = field(default_factory=list)

    @property
    def content(self) -> str:
        """Returns the content of the active tab (or first tab)."""
        for tab in self.tabs:
            if tab.is_active:
                return tab.content
        return self.tabs[0].content if self.tabs else ""


@dataclass
class QtiSurfacePlot:
    """Represents a 3D OpenGL surface plot."""
    name: str
    source_name: str = ""        # Name of Table or Matrix source
    birth_date: str = ""
    window_label: str = ""
    geometry: Dict[str, Any] = field(default_factory=dict)
    params: Dict[str, Any] = field(default_factory=dict)


@dataclass
class QtiFolder:
    """Represents a folder within the QtiPlot workspace hierarchy."""
    name: str
    birth_date: str = ""
    modification_date: str = ""
    is_active: bool = False
    is_open: bool = True
    log: str = ""
    subfolders: List['QtiFolder'] = field(default_factory=list)
    tables: List[QtiTable] = field(default_factory=list)
    matrices: List[QtiMatrix] = field(default_factory=list)
    graphs: List[QtiGraph] = field(default_factory=list)
    polar_graphs: List[QtiPolarGraph] = field(default_factory=list)
    notes: List[QtiNote] = field(default_factory=list)
    surfaces: List[QtiSurfacePlot] = field(default_factory=list)


@dataclass
class QtiProject:
    """Root container representing an entire QtiPlot project file."""
    version: str = "0.9.9"
    scripting_lang: str = "Python"
    window_count: int = 0
    root_folder: Optional[QtiFolder] = None
    
    # Flat lookup dictionaries across all folders
    tables: Dict[str, QtiTable] = field(default_factory=dict)
    matrices: Dict[str, QtiMatrix] = field(default_factory=dict)
    graphs: Dict[str, QtiGraph] = field(default_factory=dict)
    polar_graphs: Dict[str, QtiPolarGraph] = field(default_factory=dict)
    notes: Dict[str, QtiNote] = field(default_factory=dict)
    surfaces: Dict[str, QtiSurfacePlot] = field(default_factory=dict)

    def summary(self) -> str:
        """Return a formatted string summary of the project workspace."""
        lines = [
            f"QtiPlot Project (v{self.version})",
            f"Scripting Engine: {self.scripting_lang}",
            f"Tables ({len(self.tables)}): {list(self.tables.keys())}",
            f"Matrices ({len(self.matrices)}): {list(self.matrices.keys())}",
            f"2D Graphs ({len(self.graphs)}): {list(self.graphs.keys())}",
            f"Polar Graphs ({len(self.polar_graphs)}): {list(self.polar_graphs.keys())}",
            f"Notes ({len(self.notes)}): {list(self.notes.keys())}",
            f"3D Surfaces ({len(self.surfaces)}): {list(self.surfaces.keys())}"
        ]
        return "\n".join(lines)
