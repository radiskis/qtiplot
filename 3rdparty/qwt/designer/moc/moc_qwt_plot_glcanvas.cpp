/****************************************************************************
** Meta object code from reading C++ file 'qwt_plot_glcanvas.h'
**
** Created by: The Qt Meta Object Compiler version 67 (Qt 5.15.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include <memory>
#include "../../src/qwt_plot_glcanvas.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qwt_plot_glcanvas.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.15.2. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
struct qt_meta_stringdata_QwtPlotGLCanvas_t {
    QByteArrayData data[15];
    char stringdata0[180];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    qptrdiff(offsetof(qt_meta_stringdata_QwtPlotGLCanvas_t, stringdata0) + ofs \
        - idx * sizeof(QByteArrayData)) \
    )
static const qt_meta_stringdata_QwtPlotGLCanvas_t qt_meta_stringdata_QwtPlotGLCanvas = {
    {
QT_MOC_LITERAL(0, 0, 15), // "QwtPlotGLCanvas"
QT_MOC_LITERAL(1, 16, 6), // "replot"
QT_MOC_LITERAL(2, 23, 0), // ""
QT_MOC_LITERAL(3, 24, 22), // "invalidateBackingStore"
QT_MOC_LITERAL(4, 47, 10), // "borderPath"
QT_MOC_LITERAL(5, 58, 12), // "QPainterPath"
QT_MOC_LITERAL(6, 71, 11), // "frameShadow"
QT_MOC_LITERAL(7, 83, 14), // "QFrame::Shadow"
QT_MOC_LITERAL(8, 98, 10), // "frameShape"
QT_MOC_LITERAL(9, 109, 13), // "QFrame::Shape"
QT_MOC_LITERAL(10, 123, 9), // "lineWidth"
QT_MOC_LITERAL(11, 133, 12), // "midLineWidth"
QT_MOC_LITERAL(12, 146, 10), // "frameWidth"
QT_MOC_LITERAL(13, 157, 9), // "frameRect"
QT_MOC_LITERAL(14, 167, 12) // "borderRadius"

    },
    "QwtPlotGLCanvas\0replot\0\0invalidateBackingStore\0"
    "borderPath\0QPainterPath\0frameShadow\0"
    "QFrame::Shadow\0frameShape\0QFrame::Shape\0"
    "lineWidth\0midLineWidth\0frameWidth\0"
    "frameRect\0borderRadius"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_QwtPlotGLCanvas[] = {

 // content:
       8,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       7,   34, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: name, argc, parameters, tag, flags
       1,    0,   29,    2, 0x0a /* Public */,

 // methods: name, argc, parameters, tag, flags
       3,    0,   30,    2, 0x02 /* Public */,
       4,    1,   31,    2, 0x02 /* Public */,

 // slots: parameters
    QMetaType::Void,

 // methods: parameters
    QMetaType::Void,
    0x80000000 | 5, QMetaType::QRect,    2,

 // properties: name, type, flags
       6, 0x80000000 | 7, 0x0009510b,
       8, 0x80000000 | 9, 0x0009510b,
      10, QMetaType::Int, 0x00095103,
      11, QMetaType::Int, 0x00095103,
      12, QMetaType::Int, 0x00095001,
      13, QMetaType::QRect, 0x00094001,
      14, QMetaType::Double, 0x00095103,

       0        // eod
};

void QwtPlotGLCanvas::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        auto *_t = static_cast<QwtPlotGLCanvas *>(_o);
        Q_UNUSED(_t)
        switch (_id) {
        case 0: _t->replot(); break;
        case 1: _t->invalidateBackingStore(); break;
        case 2: { QPainterPath _r = _t->borderPath((*reinterpret_cast< const QRect(*)>(_a[1])));
            if (_a[0]) *reinterpret_cast< QPainterPath*>(_a[0]) = std::move(_r); }  break;
        default: ;
        }
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty) {
        auto *_t = static_cast<QwtPlotGLCanvas *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast< QFrame::Shadow*>(_v) = _t->frameShadow(); break;
        case 1: *reinterpret_cast< QFrame::Shape*>(_v) = _t->frameShape(); break;
        case 2: *reinterpret_cast< int*>(_v) = _t->lineWidth(); break;
        case 3: *reinterpret_cast< int*>(_v) = _t->midLineWidth(); break;
        case 4: *reinterpret_cast< int*>(_v) = _t->frameWidth(); break;
        case 5: *reinterpret_cast< QRect*>(_v) = _t->frameRect(); break;
        case 6: *reinterpret_cast< double*>(_v) = _t->borderRadius(); break;
        default: break;
        }
    } else if (_c == QMetaObject::WriteProperty) {
        auto *_t = static_cast<QwtPlotGLCanvas *>(_o);
        Q_UNUSED(_t)
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setFrameShadow(*reinterpret_cast< QFrame::Shadow*>(_v)); break;
        case 1: _t->setFrameShape(*reinterpret_cast< QFrame::Shape*>(_v)); break;
        case 2: _t->setLineWidth(*reinterpret_cast< int*>(_v)); break;
        case 3: _t->setMidLineWidth(*reinterpret_cast< int*>(_v)); break;
        case 6: _t->setBorderRadius(*reinterpret_cast< double*>(_v)); break;
        default: break;
        }
    } else if (_c == QMetaObject::ResetProperty) {
    }
#endif // QT_NO_PROPERTIES
}

static const QMetaObject::SuperData qt_meta_extradata_QwtPlotGLCanvas[] = {
    QMetaObject::SuperData::link<QFrame::staticMetaObject>(),
    nullptr
};

QT_INIT_METAOBJECT const QMetaObject QwtPlotGLCanvas::staticMetaObject = { {
    QMetaObject::SuperData::link<QGLWidget::staticMetaObject>(),
    qt_meta_stringdata_QwtPlotGLCanvas.data,
    qt_meta_data_QwtPlotGLCanvas,
    qt_static_metacall,
    qt_meta_extradata_QwtPlotGLCanvas,
    nullptr
} };


const QMetaObject *QwtPlotGLCanvas::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *QwtPlotGLCanvas::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_meta_stringdata_QwtPlotGLCanvas.stringdata0))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "QwtPlotAbstractGLCanvas"))
        return static_cast< QwtPlotAbstractGLCanvas*>(this);
    return QGLWidget::qt_metacast(_clname);
}

int QwtPlotGLCanvas::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QGLWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 3)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 3;
    }
#ifndef QT_NO_PROPERTIES
    else if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyDesignable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyScriptable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyStored) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyEditable) {
        _id -= 7;
    } else if (_c == QMetaObject::QueryPropertyUser) {
        _id -= 7;
    }
#endif // QT_NO_PROPERTIES
    return _id;
}
QT_WARNING_POP
QT_END_MOC_NAMESPACE
