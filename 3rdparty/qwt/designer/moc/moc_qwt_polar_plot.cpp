/****************************************************************************
** Meta object code from reading C++ file 'qwt_polar_plot.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/qwt_polar_plot.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#include <QtCore/QList>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_polar_plot.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QwtPolarPlot_t {
    QByteArrayData data[16];
    char stringdata0[185];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QwtPolarPlot_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QwtPolarPlot_t qt_meta_stringdata_QwtPolarPlot = {
    {
QT_MOC_LITERAL(0, 0, 12), // "QwtPolarPlot"
QT_MOC_LITERAL(1, 13, 12), // "itemAttached"
QT_MOC_LITERAL(2, 26, 0), // ""
QT_MOC_LITERAL(3, 27, 13), // "QwtPolarItem*"
QT_MOC_LITERAL(4, 41, 8), // "plotItem"
QT_MOC_LITERAL(5, 50, 2), // "on"
QT_MOC_LITERAL(6, 53, 17), // "legendDataChanged"
QT_MOC_LITERAL(7, 71, 8), // "itemInfo"
QT_MOC_LITERAL(8, 80, 20), // "QList<QwtLegendData>"
QT_MOC_LITERAL(9, 101, 4), // "data"
QT_MOC_LITERAL(10, 106, 13), // "layoutChanged"
QT_MOC_LITERAL(11, 120, 6), // "replot"
QT_MOC_LITERAL(12, 127, 11), // "autoRefresh"
QT_MOC_LITERAL(13, 139, 16), // "setAzimuthOrigin"
QT_MOC_LITERAL(14, 156, 14), // "plotBackground"
QT_MOC_LITERAL(15, 171, 13) // "azimuthOrigin"

    },
    "QwtPolarPlot\0itemAttached\0\0QwtPolarItem*\0"
    "plotItem\0on\0legendDataChanged\0itemInfo\0"
    "QList<QwtLegendData>\0data\0layoutChanged\0"
    "replot\0autoRefresh\0setAzimuthOrigin\0"
    "plotBackground\0azimuthOrigin"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QwtPolarPlot[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       6,   14, // methods
       2,   60, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       3,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    2,   44,    2, 0x06 /* Public */,
       6,    2,   49,    2, 0x06 /* Public */,
      10,    0,   54,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
      11,    0,   55,    2, 0x0a /* Public */,
      12,    0,   56,    2, 0x0a /* Public */,
      13,    1,   57,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Bool,    4,    5,
    QMetaType::Void, QMetaType::QVariant, 0x80000000 | 8,    7,    9,
    QMetaType::Void,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    2,

 // properties: name, type, flags
      14, QMetaType::QBrush, 0x00095103,
      15, QMetaType::Double, 0x00095103,

       0        // eod
};

void QwtPolarPlot::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QwtPolarPlot *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->itemAttached((*reinterpret_cast< QwtPolarItem*(*)>(_a[1])),(*reinterpret_cast< bool(*)>(_a[2]))); break;
        case 1: _t->legendDataChanged((*reinterpret_cast< const QVariant(*)>(_a[1])),(*reinterpret_cast< const QList<QwtLegendData>(*)>(_a[2]))); break;
        case 2: _t->layoutChanged(); break;
        case 3: _t->replot(); break;
        case 4: _t->autoRefresh(); break;
        case 5: _t->setAzimuthOrigin((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<int*>(_a[0]) = -1; break;
        case 0:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<int*>(_a[0]) = -1; break;
            case 0:
                *reinterpret_cast<int*>(_a[0]) = qRegisterMetaType< QwtPolarItem* >(); break;
            }
            break;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QwtPolarPlot::*)(QwtPolarItem * , bool );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtPolarPlot::itemAttached)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (QwtPolarPlot::*)(const QVariant & , const QList<QwtLegendData> & );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtPolarPlot::legendDataChanged)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (QwtPolarPlot::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtPolarPlot::layoutChanged)) {
                *result = 2;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<QwtPolarPlot *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QBrush*>(_v) = _t->plotBackground(); break;
        case 1: *reinterpret_cast< double*>(_v) = _t->azimuthOrigin(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<QwtPolarPlot *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setPlotBackground(*reinterpret_cast< QBrush*>(_v)); break;
        case 1: _t->setAzimuthOrigin(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QwtPolarPlot::staticMetaObject = { {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    qt_meta_stringdata_QwtPolarPlot.data,
    qt_meta_data_QwtPolarPlot,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QwtPolarPlot::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QwtPolarPlot::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPolarPlot.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QwtPolarItemDict"))
        return static_cast< QwtPolarItemDict*>(this);
    return QFrame::qt_metacast(_clname);
}

int QwtPolarPlot::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QFrame::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 6)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 2;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 2;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QwtPolarPlot::itemAttached(QwtPolarItem * _t1, bool _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtPolarPlot::legendDataChanged(const QVariant & _t1, const QList<QwtLegendData> & _t2)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}

// SIGNAL 2
void QwtPolarPlot::layoutChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
