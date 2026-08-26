/****************************************************************************
** Meta object code from reading C++ file 'ScrollBar.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../ScrollBar.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'ScrollBar.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_ScrollBar_t {
    QByteArrayData data[12];
    char stringdata0[120];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_ScrollBar_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_ScrollBar_t qt_meta_stringdata_ScrollBar = {
    {
QT_MOC_LITERAL(0, 0, 9), // "ScrollBar"
QT_MOC_LITERAL(1, 10, 11), // "sliderMoved"
QT_MOC_LITERAL(2, 22, 0), // ""
QT_MOC_LITERAL(3, 23, 15), // "Qt::Orientation"
QT_MOC_LITERAL(4, 39, 12), // "valueChanged"
QT_MOC_LITERAL(5, 52, 7), // "setBase"
QT_MOC_LITERAL(6, 60, 3), // "min"
QT_MOC_LITERAL(7, 64, 3), // "max"
QT_MOC_LITERAL(8, 68, 10), // "moveSlider"
QT_MOC_LITERAL(9, 79, 17), // "catchValueChanged"
QT_MOC_LITERAL(10, 97, 5), // "value"
QT_MOC_LITERAL(11, 103, 16) // "catchSliderMoved"

    },
    "ScrollBar\0sliderMoved\0\0Qt::Orientation\0"
    "valueChanged\0setBase\0min\0max\0moveSlider\0"
    "catchValueChanged\0value\0catchSliderMoved"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_ScrollBar[] = {

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
       1,    3,   44,    2, 0x06 /* Public */,
       4,    3,   51,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       5,    2,   58,    2, 0x0a /* Public */,
       8,    2,   63,    2, 0x0a /* Public */,
       9,    1,   68,    2, 0x08 /* Private */,
      11,    1,   71,    2, 0x08 /* Private */,

 // signals: parameters
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    2,    2,    2,
    QMetaType::Void, 0x80000000 | 3, QMetaType::Double, QMetaType::Double,    2,    2,    2,

 // slots: parameters
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    6,    7,
    QMetaType::Void, QMetaType::Double, QMetaType::Double,    6,    7,
    QMetaType::Void, QMetaType::Int,   10,
    QMetaType::Void, QMetaType::Int,   10,

       0        // eod
};

void ScrollBar::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<ScrollBar *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->sliderMoved((*reinterpret_cast< Qt::Orientation(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 1: _t->valueChanged((*reinterpret_cast< Qt::Orientation(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2])),(*reinterpret_cast< double(*)>(_a[3]))); break;
        case 2: _t->setBase((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 3: _t->moveSlider((*reinterpret_cast< double(*)>(_a[1])),(*reinterpret_cast< double(*)>(_a[2]))); break;
        case 4: _t->catchValueChanged((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 5: _t->catchSliderMoved((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (ScrollBar::*)(Qt::Orientation , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScrollBar::sliderMoved)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (ScrollBar::*)(Qt::Orientation , double , double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&ScrollBar::valueChanged)) {
                *result = 1;
                return;
            }
        }
    }
}

QT_INIT_METAOBJECT const QMetaObject ScrollBar::staticMetaObject = { {
    QMetaObject::SuperData::link<QScrollBar::staticMetaObject>(),
    qt_meta_stringdata_ScrollBar.data,
    qt_meta_data_ScrollBar,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *ScrollBar::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *ScrollBar::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_ScrollBar.stringdata0))
        return static_cast<void*>(this);
    return QScrollBar::qt_metacast(_clname);
}

int ScrollBar::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QScrollBar::qt_metacall(_c, _id, _a);
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
void ScrollBar::sliderMoved(Qt::Orientation _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void ScrollBar::valueChanged(Qt::Orientation _t1, double _t2, double _t3)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t2))), const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t3))) };
    QMetaObject::activate(this, &staticMetaObject, 1, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
