#include "TableSeriesData.h"
#include <Table.h>
#include <Graph.h>
#include <QDateTime>
#include <QTime>
#include <cmath>

TableSeriesData::TableSeriesData()
    : QwtArraySeriesData<QPointF>(),
      d_table(nullptr),
      d_x_table(nullptr),
      d_y_col(-1),
      d_x_col(-1),
      d_start_row(0),
      d_end_row(-1),
      d_curve_type(0),
      d_x_offset(0.0),
      d_y_offset(0.0)
{
}

TableSeriesData::TableSeriesData(Table *t, int yCol, Table *xt, int xCol,
                                 int startRow, int endRow,
                                 int curveType,
                                 double xOffset, double yOffset)
    : QwtArraySeriesData<QPointF>(),
      d_table(t),
      d_x_table(xt ? xt : t),
      d_y_col(yCol),
      d_x_col(xCol),
      d_start_row(startRow),
      d_end_row(endRow),
      d_curve_type(curveType),
      d_x_offset(xOffset),
      d_y_offset(yOffset)
{
}

bool TableSeriesData::load(Table *t, int ycol, Table *xt, int xcol,
                           int startRow, int endRow, int curveType,
                           double xOffset, double yOffset,
                           const QLocale &locale,
                           QStringList *xLabels,
                           QStringList *yLabels)
{
    d_table = t;
    d_x_table = xt ? xt : t;
    d_y_col = ycol;
    d_x_col = xcol;
    d_start_row = startRow;
    d_end_row = endRow;
    d_curve_type = curveType;
    d_x_offset = xOffset;
    d_y_offset = yOffset;

    m_samples.clear();
    d_table_rows.clear();
    d_data_ranges.clear();
    cachedBoundingRect = QRectF(0.0, 0.0, -1.0, -1.0);

    if (!d_table || !d_x_table || d_x_col < 0 || d_y_col < 0)
        return false;

    int rows = d_table->numRows();
    if (d_end_row < 0 || d_end_row >= rows)
        d_end_row = rows - 1;

    int xColType = d_x_table->columnType(d_x_col);
    int yColType = d_table->columnType(d_y_col);
    int r = std::abs(d_end_row - d_start_row) + 1;

    m_samples.reserve(r);
    d_table_rows.reserve(r);

    QString date_time_fmt = d_x_table->columnFormat(d_x_col);
    int size = 0, from = 0;

    for (int i = d_start_row; i <= d_end_row; i++) {
        QString xval = d_x_table->text(i, d_x_col);
        QString yval = d_table->text(i, d_y_col);
        if (!xval.isEmpty() && !yval.isEmpty()) {
            bool valid_data = true;
            QPointF p;
            if (xColType == Table::Text) {
                if (xLabels) *xLabels << xval;
                p.setX((double)(size + 1));
            } else if (xColType == Table::Time) {
                p.setX(Table::fromTime(QTime::fromString(xval.trimmed(), date_time_fmt)));
            } else if (xColType == Table::Date) {
                p.setX(Table::fromDateTime(QDateTime::fromString(xval.trimmed(), date_time_fmt)));
            } else {
                p.setX(locale.toDouble(xval, &valid_data));
            }

            if (yColType == Table::Text) {
                if (yLabels) *yLabels << yval;
                p.setY((double)(size + 1));
            } else {
                p.setY(locale.toDouble(yval, &valid_data));
            }

            if (valid_data) {
                m_samples.append(p);
                d_table_rows.append(i);
                size++;
            }
        } else if (from < size) {
            DataRange range;
            range.from = from;
            range.to = size - 1;
            d_data_ranges.push_back(range);
            from = size;
        }
    }

    if (!d_data_ranges.empty() && from < size) {
        DataRange range;
        range.from = from;
        range.to = size - 1;
        d_data_ranges.push_back(range);
    }

    if (size == 0)
        return false;

    if (d_curve_type == Graph::HorizontalBars) {
        for (int i = 0; i < size; i++) {
            QPointF &p = m_samples[i];
            p = QPointF(p.y(), p.x());
        }
    }

    if (d_x_offset != 0.0 || d_y_offset != 0.0) {
        for (int i = 0; i < size; i++) {
            QPointF &p = m_samples[i];
            p.rx() += d_x_offset;
            p.ry() += d_y_offset;
        }
    }

    return true;
}
