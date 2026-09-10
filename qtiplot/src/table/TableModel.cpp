#include "TableModel.h"

TableModel::TableModel(int rows, int cols, QObject *parent)
    : QAbstractTableModel(parent),
      m_rowCount(std::max(0, rows))
{
    int colCount = std::max(0, cols);
    m_columns.reserve(colCount);
    for (int i = 0; i < colCount; ++i) {
        auto *col = new ColumnData();
        col->resize(m_rowCount);
        m_columns.append(col);
    }
}

TableModel::~TableModel()
{
    qDeleteAll(m_columns);
    m_columns.clear();
}

QVariant TableModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return {};

    int r = index.row();
    int c = index.column();
    if (r < 0 || r >= m_rowCount || c < 0 || c >= m_columns.size())
        return {};

    const ColumnData *col = m_columns[c];
    if (role == Qt::DisplayRole || role == Qt::EditRole)
        return col->text[r];
    if (role == Qt::TextAlignmentRole)
        return static_cast<int>(col->hasRaw[r] ? (Qt::AlignRight | Qt::AlignVCenter)
                                               : (Qt::AlignLeft | Qt::AlignVCenter));

    return {};
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || role != Qt::EditRole)
        return false;

    int r = index.row();
    int c = index.column();
    if (r < 0 || r >= m_rowCount || c < 0 || c >= m_columns.size())
        return false;

    ColumnData *col = m_columns[c];
    m_lastEdited = {r, c, col->text[r], col->rawValues[r], col->hasRaw[r]};

    col->text[r] = value.toString();
    col->hasRaw[r] = false;
    col->rawValues[r] = 0.0;

    emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole, Qt::TextAlignmentRole});
    return true;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;

    Qt::ItemFlags f = Qt::ItemIsEnabled | Qt::ItemIsSelectable;
    int c = index.column();
    if (c >= 0 && c < m_columns.size() && !m_columns[c]->readOnly)
        f |= Qt::ItemIsEditable;
    return f;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section >= 0 && section < m_columns.size()) {
            const QString &hdr = m_columns[section]->headerText;
            return hdr.isEmpty() ? QString::number(section + 1) : hdr;
        }
        return QString::number(section + 1);
    }
    if (role == Qt::TextAlignmentRole)
        return static_cast<int>(Qt::AlignCenter);
    return {};
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal && section >= 0 && section < m_columns.size() &&
        (role == Qt::DisplayRole || role == Qt::EditRole)) {
        m_columns[section]->headerText = value.toString();
        emit headerDataChanged(Qt::Horizontal, section, section);
        return true;
    }
    return false;
}

bool TableModel::insertRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || row > m_rowCount)
        return false;

    beginInsertRows(parent, row, row + count - 1);
    m_rowCount += count;
    for (ColumnData *col : m_columns) {
        col->text.insert(col->text.begin() + row, count, QString());
        col->rawValues.insert(col->rawValues.begin() + row, count, 0.0);
        col->hasRaw.insert(col->hasRaw.begin() + row, count, false);
    }
    endInsertRows();
    return true;
}

bool TableModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count <= 0 || row < 0 || row + count > m_rowCount)
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    m_rowCount -= count;
    for (ColumnData *col : m_columns) {
        int end = row + count;
        col->text.erase(col->text.begin() + row, col->text.begin() + end);
        col->rawValues.erase(col->rawValues.begin() + row, col->rawValues.begin() + end);
        col->hasRaw.erase(col->hasRaw.begin() + row, col->hasRaw.begin() + end);
    }
    endRemoveRows();
    return true;
}

bool TableModel::insertColumns(int column, int count, const QModelIndex &parent)
{
    if (count <= 0 || column < 0 || column > m_columns.size())
        return false;

    beginInsertColumns(parent, column, column + count - 1);
    for (int i = 0; i < count; ++i) {
        auto *col = new ColumnData();
        col->resize(m_rowCount);
        m_columns.insert(column + i, col);
    }
    endInsertColumns();
    return true;
}

bool TableModel::removeColumns(int column, int count, const QModelIndex &parent)
{
    if (count <= 0 || column < 0 || column + count > m_columns.size())
        return false;

    beginRemoveColumns(parent, column, column + count - 1);
    for (int i = 0; i < count; ++i)
        delete m_columns.takeAt(column);
    endRemoveColumns();
    return true;
}

void TableModel::setRowCount(int r)
{
    if (r < 0 || r == m_rowCount)
        return;
    if (r > m_rowCount)
        insertRows(m_rowCount, r - m_rowCount);
    else
        removeRows(r, m_rowCount - r);
}

void TableModel::setColumnCount(int c)
{
    if (c < 0 || c == m_columns.size())
        return;
    if (c > m_columns.size())
        insertColumns(m_columns.size(), c - m_columns.size());
    else
        removeColumns(c, m_columns.size() - c);
}

void TableModel::swapColumns(int col1, int col2)
{
    if (col1 == col2 || col1 < 0 || col2 < 0 || col1 >= m_columns.size() || col2 >= m_columns.size())
        return;
    beginResetModel();
    std::swap(m_columns[col1], m_columns[col2]);
    endResetModel();
}

void TableModel::swapRows(int row1, int row2)
{
    if (row1 == row2 || row1 < 0 || row2 < 0 || row1 >= m_rowCount || row2 >= m_rowCount)
        return;

    for (ColumnData *col : m_columns) {
        std::swap(col->text[row1], col->text[row2]);
        std::swap(col->rawValues[row1], col->rawValues[row2]);
        bool hr = col->hasRaw[row1];
        col->hasRaw[row1] = col->hasRaw[row2];
        col->hasRaw[row2] = hr;
    }
    emit dataChanged(index(std::min(row1, row2), 0),
                     index(std::max(row1, row2), m_columns.size() - 1),
                     {Qt::DisplayRole, Qt::EditRole, Qt::TextAlignmentRole});
}

QString TableModel::text(int r, int c) const
{
    return (r >= 0 && r < m_rowCount && c >= 0 && c < m_columns.size()) ? m_columns[c]->text[r] : QString();
}

void TableModel::setText(int r, int c, const QString &t)
{
    if (c < 0 || c >= m_columns.size())
        return;
    if (r >= m_rowCount)
        setRowCount(r + 1);
    if (r < 0)
        return;

    m_columns[c]->text[r] = t;
    emit dataChanged(index(r, c), index(r, c), {Qt::DisplayRole, Qt::EditRole, Qt::TextAlignmentRole});
}

double TableModel::rawValue(int r, int c) const
{
    return (r >= 0 && r < m_rowCount && c >= 0 && c < m_columns.size()) ? m_columns[c]->rawValues[r] : 0.0;
}

void TableModel::setRawValue(int r, int c, double val)
{
    if (c < 0 || c >= m_columns.size())
        return;
    if (r >= m_rowCount)
        setRowCount(r + 1);
    if (r < 0)
        return;

    m_columns[c]->rawValues[r] = val;
    m_columns[c]->hasRaw[r] = true;
    emit dataChanged(index(r, c), index(r, c), {Qt::TextAlignmentRole});
}

bool TableModel::hasRawValue(int r, int c) const
{
    return (r >= 0 && r < m_rowCount && c >= 0 && c < m_columns.size()) ? m_columns[c]->hasRaw[r] : false;
}

void TableModel::clearRawValue(int r, int c)
{
    if (r < 0 || r >= m_rowCount || c < 0 || c >= m_columns.size())
        return;
    m_columns[c]->rawValues[r] = 0.0;
    m_columns[c]->hasRaw[r] = false;
    emit dataChanged(index(r, c), index(r, c), {Qt::TextAlignmentRole});
}

bool TableModel::isColumnReadOnly(int col) const
{
    return (col >= 0 && col < m_columns.size()) ? m_columns[col]->readOnly : false;
}

void TableModel::setColumnReadOnly(int col, bool ro)
{
    if (col < 0 || col >= m_columns.size())
        return;
    m_columns[col]->readOnly = ro;
    if (m_rowCount > 0)
        emit dataChanged(index(0, col), index(m_rowCount - 1, col));
}

void TableModel::setHeaderText(int col, const QString &text)
{
    setHeaderData(col, Qt::Horizontal, text, Qt::DisplayRole);
}

QString TableModel::headerText(int col) const
{
    return (col >= 0 && col < m_columns.size()) ? m_columns[col]->headerText : QString();
}

CellState TableModel::previousCellState(int r, int c) const
{
    if (m_lastEdited.row == r && m_lastEdited.col == c)
        return m_lastEdited;
    return {r, c, text(r, c), rawValue(r, c), hasRawValue(r, c)};
}

void TableModel::setPreviousCellState(int r, int c, const QString &text, double val, bool hasVal)
{
    m_lastEdited = {r, c, text, val, hasVal};
}
