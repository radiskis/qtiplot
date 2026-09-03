#pragma once
#include <QString>
#include <map>
#include <typeinfo>

class AllocLedger {
public:
    static AllocLedger& instance();
    void add(const char *type);
    void remove(const char *type);
    //! Types with a non-zero count, formatted "N x TypeName", most numerous first.
    QString report() const;
    bool clean() const;
    void reset();
private:
    std::map<QString, long> d_counts;
};

#ifdef QTIPLOT_LEDGER
template <class Derived>
class Tracked {
protected:
    Tracked()  { AllocLedger::instance().add(typeid(Derived).name()); }
    ~Tracked() { AllocLedger::instance().remove(typeid(Derived).name()); }
    Tracked(const Tracked&) : Tracked() {}
    Tracked& operator=(const Tracked&) { return *this; }
};
#else
template <class Derived>
class Tracked {
protected:
    Tracked() = default;
    ~Tracked() = default;
    Tracked(const Tracked&) = default;
    Tracked& operator=(const Tracked&) = default;
};
#endif
