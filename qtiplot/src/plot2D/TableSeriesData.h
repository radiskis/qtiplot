#ifndef TABLESERIESDATA_H
#define TABLESERIESDATA_H

#include <qwt_series_data.h>
#include <QVector>
#include <QPointF>
#include <QStringList>
#include <QLocale>
#include <vector>

class Table;

/*!
  \brief QwtSeriesData implementation bridging Table data to QwtPlotCurve.
  Adheres to modern Qwt 6 conventions for series data, caching sample coordinates,
  table row mappings for O(1) tableRow lookup, and missing-data ranges.
*/
class TableSeriesData : public QwtArraySeriesData<QPointF>
{
public:
    struct DataRange {
        unsigned int from;
        unsigned int to;
    };

    TableSeriesData();
    TableSeriesData(Table *t, int yCol, Table *xt, int xCol,
                    int startRow, int endRow,
                    int curveType = 0,
                    double xOffset = 0.0, double yOffset = 0.0);

    Table* table() const { return d_table; }
    Table* xTable() const { return d_x_table; }
    int yColumn() const { return d_y_col; }
    int xColumn() const { return d_x_col; }
    int startRow() const { return d_start_row; }
    int endRow() const { return d_end_row; }

    int tableRow(int point) const {
        if (point >= 0 && point < d_table_rows.size())
            return d_table_rows[point];
        return -1;
    }

    const QVector<int>& tableRows() const { return d_table_rows; }
    const std::vector<DataRange>& dataRanges() const { return d_data_ranges; }

    void setOffsets(double xOffset, double yOffset) {
        d_x_offset = xOffset;
        d_y_offset = yOffset;
    }

    bool load(Table *t, int ycol, Table *xt, int xcol,
              int startRow, int endRow, int curveType,
              double xOffset, double yOffset,
              const QLocale &locale,
              QStringList *xLabels = nullptr,
              QStringList *yLabels = nullptr);

private:
    Table *d_table;
    Table *d_x_table;
    int d_y_col;
    int d_x_col;
    int d_start_row;
    int d_end_row;
    int d_curve_type;
    double d_x_offset;
    double d_y_offset;
    QVector<int> d_table_rows;
    std::vector<DataRange> d_data_ranges;
};

#endif // TABLESERIESDATA_H
