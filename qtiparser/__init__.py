"""
QtiParser: Standalone Pure-Python Parser and Data Extraction Library for QtiPlot Projects (.qti / .qti.gz)
"""

from .parser import read_qti, QtiParser
from .models import (
    QtiProject,
    QtiFolder,
    QtiTable,
    QtiTableStatistics,
    QtiMatrix,
    QtiGraph,
    QtiLayer,
    QtiCurve,
    QtiFunctionCurve,
    QtiSpectrogram,
    QtiErrorBar,
    QtiAxisScale,
    QtiLegend,
    QtiLineMarker,
    QtiTexMarker,
    QtiImageMarker,
    QtiShapeMarker,
    QtiPolarGraph,
    QtiPolarCurve,
    QtiNote,
    QtiTab,
    QtiSurfacePlot,
    QtiColorMap,
    Column,
)

__all__ = [
    "read_qti",
    "QtiParser",
    "QtiProject",
    "QtiFolder",
    "QtiTable",
    "QtiTableStatistics",
    "QtiMatrix",
    "QtiGraph",
    "QtiLayer",
    "QtiCurve",
    "QtiFunctionCurve",
    "QtiSpectrogram",
    "QtiErrorBar",
    "QtiAxisScale",
    "QtiLegend",
    "QtiLineMarker",
    "QtiTexMarker",
    "QtiImageMarker",
    "QtiShapeMarker",
    "QtiPolarGraph",
    "QtiPolarCurve",
    "QtiNote",
    "QtiTab",
    "QtiSurfacePlot",
    "QtiColorMap",
    "Column",
]

__version__ = "1.0.0"
