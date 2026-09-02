#include "Tracked.h"
#include <QVector>
#include <QPair>
#include <algorithm>

AllocLedger& AllocLedger::instance()
{
    static AllocLedger s_instance;
    return s_instance;
}

void AllocLedger::add(const char *type)
{
    if (type)
        d_counts[QString::fromUtf8(type)]++;
}

void AllocLedger::remove(const char *type)
{
    if (type) {
        QString key = QString::fromUtf8(type);
        d_counts[key]--;
        if (d_counts[key] == 0)
            d_counts.erase(key);
    }
}

bool AllocLedger::clean() const
{
    for (const auto &pair : d_counts) {
        if (pair.second != 0)
            return false;
    }
    return true;
}

QString AllocLedger::report() const
{
    QVector<QPair<long, QString>> items;
    for (const auto &pair : d_counts) {
        if (pair.second != 0)
            items.append(qMakePair(pair.second, pair.first));
    }
    std::sort(items.begin(), items.end(), [](const QPair<long, QString> &a, const QPair<long, QString> &b) {
        return a.first > b.first;
    });

    QString s;
    for (const auto &item : items) {
        s += QString("%1 x %2\n").arg(item.first).arg(item.second);
    }
    return s;
}

void AllocLedger::reset()
{
    d_counts.clear();
}
