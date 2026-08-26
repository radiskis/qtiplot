/****************************************************************************
** Meta object code from reading C++ file 'RandomPlot.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../RandomPlot.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'RandomPlot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_RandomPlot_t {
    QByteArrayData data[11];
    char stringdata0[75];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_RandomPlot_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_RandomPlot_t qt_meta_stringdata_RandomPlot = {
    {
QT_MOC_LITERAL(0, 0, 10), // "RandomPlot"
QT_MOC_LITERAL(1, 11, 7), // "running"
QT_MOC_LITERAL(2, 19, 0), // ""
QT_MOC_LITERAL(3, 20, 7), // "elapsed"
QT_MOC_LITERAL(4, 28, 2), // "ms"
QT_MOC_LITERAL(5, 31, 5), // "clear"
QT_MOC_LITERAL(6, 37, 4), // "stop"
QT_MOC_LITERAL(7, 42, 6), // "append"
QT_MOC_LITERAL(8, 49, 7), // "timeout"
QT_MOC_LITERAL(9, 57, 5), // "count"
QT_MOC_LITERAL(10, 63, 11) // "appendPoint"

    },
    "RandomPlot\0running\0\0elapsed\0ms\0clear\0"
    "stop\0append\0timeout\0count\0appendPoint"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_RandomPlot[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       2,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   44,    2, 0x06 /* Public */,
       3,    1,   47,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    0,   50,    2, 0x0a /* Public */,
       6,    0,   51,    2, 0x0a /* Public */,
       7,    2,   52,    2, 0x0a /* Public */,
      10,    0,   57,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, QMetaType::Bool,    2,
    QMetaType::Void, QMetaType::Int,    4,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int, QMetaType::Int,    8,    9,
    QMetaType::Void,

       0        // eod
};

void RandomPlot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<RandomPlot *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->running((*reinterpret_cast< bool(*)>(_a[1]))); break;
        case 1: _t->elapsed((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 2: _t->clear(); break;
        case 3: _t->stop(); break;
        case 4: _t->append((*reinterpret_cast< int(*)>(_a[1])),(*reinterpret_cast< int(*)>(_a[2]))); break;
        case 5: _t->appendPoint(); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (RandomPlot::*)(bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RandomPlot::running)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (RandomPlot::*)(int );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&RandomPlot::elapsed)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject RandomPlot::staticMetaObject = { {
    QMetaObject::SuperData::link<IncrementalPlot::staticMetaObject>(),
    qt_meta_stringdata_RandomPlot.data,
    qt_meta_data_RandomPlot,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *RandomPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *RandomPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_RandomPlot.stringdata0))
        return static_cast<void*>(this);
    return IncrementalPlot::qt_metacast(_clname);
}

int RandomPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = IncrementalPlot::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void RandomPlot::running(bool _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void RandomPlot::elapsed(int _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
