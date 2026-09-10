#ifndef TABLE_MODEL_H
#define TABLE_MODEL_H

#include <QAbstractTableModel>
#include <QString>
#include <QVector>
#include <vector>

struct ColumnData
{
    std::vector<QString> text;
    std::vector<double> rawValues;
    std::vector<bool> hasRaw;
    bool readOnly = false;
    QString headerText;

    void resize(int rows) {
        text.resize(rows);
        rawValues.resize(rows, 0.0);
        hasRaw.resize(rows, false);
    }
};

struct CellState
{
    int row = -1;
    int col = -1;
    QString text;
    double raw = 0.0;
    bool hasRaw = false;
};

class TableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TableModel(int rows = 0, int cols = 0, QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : m_rowCount; }
    int columnCount(const QModelIndex &parent = QModelIndex()) const override { return parent.isValid() ? 0 : m_columns.size(); }
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    bool setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int role = Qt::EditRole) override;

    bool insertRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex()) override;
    bool insertColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;
    bool removeColumns(int column, int count, const QModelIndex &parent = QModelIndex()) override;

    void setRowCount(int r);
    void setColumnCount(int c);
    void swapColumns(int c1, int c2);
    void swapRows(int r1, int r2);

    QString text(int r, int c) const;
    void setText(int r, int c, const QString &t);
    double rawValue(int r, int c) const;
    void setRawValue(int r, int c, double val);
    bool hasRawValue(int r, int c) const;
    void clearRawValue(int r, int c);

    bool isColumnReadOnly(int col) const;
    void setColumnReadOnly(int col, bool ro);

    void setHeaderText(int col, const QString &text);
    QString headerText(int col) const;

    CellState previousCellState(int r, int c) const;
    void setPreviousCellState(int r, int c, const QString &text, double val, bool hasVal);

private:
    int m_rowCount = 0;
    QVector<ColumnData> m_columns;   // value semantics — no heap per column
    CellState m_lastEdited;
};

#endif // TABLE_MODEL_H
