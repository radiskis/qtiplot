/****************************************************************************
** Meta object code from reading C++ file 'qwt_abstract_slider.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/qwt_abstract_slider.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_abstract_slider.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QwtAbstractSlider_t {
    QByteArrayData data[16];
    char stringdata0[179];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QwtAbstractSlider_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QwtAbstractSlider_t qt_meta_stringdata_QwtAbstractSlider = {
    {
QT_MOC_LITERAL(0, 0, 17), // "QwtAbstractSlider"
QT_MOC_LITERAL(1, 18, 12), // "valueChanged"
QT_MOC_LITERAL(2, 31, 0), // ""
QT_MOC_LITERAL(3, 32, 5), // "value"
QT_MOC_LITERAL(4, 38, 13), // "sliderPressed"
QT_MOC_LITERAL(5, 52, 14), // "sliderReleased"
QT_MOC_LITERAL(6, 67, 11), // "sliderMoved"
QT_MOC_LITERAL(7, 79, 8), // "setValue"
QT_MOC_LITERAL(8, 88, 10), // "totalSteps"
QT_MOC_LITERAL(9, 99, 11), // "singleSteps"
QT_MOC_LITERAL(10, 111, 9), // "pageSteps"
QT_MOC_LITERAL(11, 121, 13), // "stepAlignment"
QT_MOC_LITERAL(12, 135, 8), // "readOnly"
QT_MOC_LITERAL(13, 144, 8), // "tracking"
QT_MOC_LITERAL(14, 153, 8), // "wrapping"
QT_MOC_LITERAL(15, 162, 16) // "invertedControls"

    },
    "QwtAbstractSlider\0valueChanged\0\0value\0"
    "sliderPressed\0sliderReleased\0sliderMoved\0"
    "setValue\0totalSteps\0singleSteps\0"
    "pageSteps\0stepAlignment\0readOnly\0"
    "tracking\0wrapping\0invertedControls"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QwtAbstractSlider[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       5,   14, // methods
       9,   50, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       4,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   39,    2, 0x06 /* Public */,
       4,    0,   42,    2, 0x06 /* Public */,
       5,    0,   43,    2, 0x06 /* Public */,
       6,    1,   44,    2, 0x06 /* Public */,

 // slots: name, argc, parameters, tag, flags
       7,    1,   47,    2, 0x0a /* Public */,

 // signals: parameters
    QMetaType::Void, QMetaType::Double,    3,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Double,    3,

 // slots: parameters
    QMetaType::Void, QMetaType::Double,    3,

 // properties: name, type, flags
       3, QMetaType::Double, 0x00595103,
       8, QMetaType::UInt, 0x00095103,
       9, QMetaType::UInt, 0x00095103,
      10, QMetaType::UInt, 0x00095103,
      11, QMetaType::Bool, 0x00095103,
      12, QMetaType::Bool, 0x00095103,
      13, QMetaType::Bool, 0x00095103,
      14, QMetaType::Bool, 0x00095103,
      15, QMetaType::Bool, 0x00095103,

 // properties: notify_signal_id
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,
       0,

       0        // eod
};

void QwtAbstractSlider::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QwtAbstractSlider *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->valueChanged((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 1: _t->sliderPressed(); break;
        case 2: _t->sliderReleased(); break;
        case 3: _t->sliderMoved((*reinterpret_cast< double(*)>(_a[1]))); break;
        case 4: _t->setValue((*reinterpret_cast< double(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        {
            using _t = void (QwtAbstractSlider::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtAbstractSlider::valueChanged)) {
                *result = 0;
                return;
            }
        }
        {
            using _t = void (QwtAbstractSlider::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtAbstractSlider::sliderPressed)) {
                *result = 1;
                return;
            }
        }
        {
            using _t = void (QwtAbstractSlider::*)();
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtAbstractSlider::sliderReleased)) {
                *result = 2;
                return;
            }
        }
        {
            using _t = void (QwtAbstractSlider::*)(double );
            if (*reinterpret_cast<_t *>(_a[1]) == static_cast<_t>(&QwtAbstractSlider::sliderMoved)) {
                *result = 3;
                return;
            }
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<QwtAbstractSlider *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< double*>(_v) = _t->value(); break;
        case 1: *reinterpret_cast< uint*>(_v) = _t->totalSteps(); break;
        case 2: *reinterpret_cast< uint*>(_v) = _t->singleSteps(); break;
        case 3: *reinterpret_cast< uint*>(_v) = _t->pageSteps(); break;
        case 4: *reinterpret_cast< bool*>(_v) = _t->stepAlignment(); break;
        case 5: *reinterpret_cast< bool*>(_v) = _t->isReadOnly(); break;
        case 6: *reinterpret_cast< bool*>(_v) = _t->isTracking(); break;
        case 7: *reinterpret_cast< bool*>(_v) = _t->wrapping(); break;
        case 8: *reinterpret_cast< bool*>(_v) = _t->invertedControls(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<QwtAbstractSlider *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setValue(*reinterpret_cast< double*>(_v)); break;
        case 1: _t->setTotalSteps(*reinterpret_cast< uint*>(_v)); break;
        case 2: _t->setSingleSteps(*reinterpret_cast< uint*>(_v)); break;
        case 3: _t->setPageSteps(*reinterpret_cast< uint*>(_v)); break;
        case 4: _t->setStepAlignment(*reinterpret_cast< bool*>(_v)); break;
        case 5: _t->setReadOnly(*reinterpret_cast< bool*>(_v)); break;
        case 6: _t->setTracking(*reinterpret_cast< bool*>(_v)); break;
        case 7: _t->setWrapping(*reinterpret_cast< bool*>(_v)); break;
        case 8: _t->setInvertedControls(*reinterpret_cast< bool*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

QT_INIT_METAOBJECT const QMetaObject QwtAbstractSlider::staticMetaObject = { {
    QMetaObject::SuperData::link<QwtAbstractScale::staticMetaObject>(),
    qt_meta_stringdata_QwtAbstractSlider.data,
    qt_meta_data_QwtAbstractSlider,
    qt_static_metacall,
    nullptr,
    nullptr
} };


const QMetaObject *QwtAbstractSlider::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QwtAbstractSlider::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QwtAbstractSlider.stringdata0))
        return static_cast<void*>(this);
    return QwtAbstractScale::qt_metacast(_clname);
}

int QwtAbstractSlider::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QwtAbstractScale::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 5)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 5;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 5)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 5;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 9;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 9;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}

// SIGNAL 0
void QwtAbstractSlider::valueChanged(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}

// SIGNAL 1
void QwtAbstractSlider::sliderPressed()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void QwtAbstractSlider::sliderReleased()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void QwtAbstractSlider::sliderMoved(double _t1)
{
    void *_a[] = { nullptr, const_cast<void*>(reinterpret_cast<const void*>(std::addressof(_t1))) };
    QMetaObject::activate(this, &staticMetaObject, 3, _a);
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
