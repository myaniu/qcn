/****************************************************************************
** Meta object code from reading C++ file 'myframe.h'
**
** Created: Sat Jan 5 14:22:59 2013
**      by: The Qt Meta Object Compiler version 67 (Qt 5.0.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../myframe.h"
#include <QtCore/qbytearray.h>
#include <QtCore/qmetatype.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'myframe.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 67
#error "This file was generated using the moc from 5.0.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
struct qt_meta_stringdata_MyFrame_t {
    QByteArrayData data[13];
    char stringdata[183];
};
#define QT_MOC_LITERAL(idx, ofs, len) \
    Q_STATIC_BYTE_ARRAY_DATA_HEADER_INITIALIZER_WITH_OFFSET(len, \
    offsetof(qt_meta_stringdata_MyFrame_t, stringdata) + ofs \
        - idx * sizeof(QByteArrayData) \
    )
static const qt_meta_stringdata_MyFrame_t qt_meta_stringdata_MyFrame = {
    {
QT_MOC_LITERAL(0, 0, 7),
QT_MOC_LITERAL(1, 8, 18),
QT_MOC_LITERAL(2, 27, 0),
QT_MOC_LITERAL(3, 28, 4),
QT_MOC_LITERAL(4, 33, 18),
QT_MOC_LITERAL(5, 52, 13),
QT_MOC_LITERAL(6, 66, 10),
QT_MOC_LITERAL(7, 77, 17),
QT_MOC_LITERAL(8, 95, 18),
QT_MOC_LITERAL(9, 114, 22),
QT_MOC_LITERAL(10, 137, 16),
QT_MOC_LITERAL(11, 154, 10),
QT_MOC_LITERAL(12, 165, 16)
    },
    "MyFrame\0signalTimePosition\0\0iPos\0"
    "fileDialogSettings\0fileMakeQuake\0"
    "actionView\0actionOptionEarth\0"
    "actionOptionSensor\0actionOptionScreenshot\0"
    "actionOptionLogo\0actionHelp\0"
    "slotTimePosition\0"
};
#undef QT_MOC_LITERAL

static const uint qt_meta_data_MyFrame[] = {

 // content:
       7,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: name, argc, parameters, tag, flags
       1,    1,   64,    2, 0x05,

 // slots: name, argc, parameters, tag, flags
       4,    0,   67,    2, 0x08,
       5,    0,   68,    2, 0x08,
       6,    0,   69,    2, 0x08,
       7,    0,   70,    2, 0x08,
       8,    0,   71,    2, 0x08,
       9,    0,   72,    2, 0x08,
      10,    0,   73,    2, 0x08,
      11,    0,   74,    2, 0x08,
      12,    1,   75,    2, 0x08,

 // signals: parameters
    QMetaType::Void, QMetaType::Int,    3,

 // slots: parameters
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void,
    QMetaType::Void, QMetaType::Int,    3,

       0        // eod
};

void MyFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        MyFrame *_t = static_cast<MyFrame *>(_o);
        switch (_id) {
        case 0: _t->signalTimePosition((*reinterpret_cast< int(*)>(_a[1]))); break;
        case 1: _t->fileDialogSettings(); break;
        case 2: _t->fileMakeQuake(); break;
        case 3: _t->actionView(); break;
        case 4: _t->actionOptionEarth(); break;
        case 5: _t->actionOptionSensor(); break;
        case 6: _t->actionOptionScreenshot(); break;
        case 7: _t->actionOptionLogo(); break;
        case 8: _t->actionHelp(); break;
        case 9: _t->slotTimePosition((*reinterpret_cast< int(*)>(_a[1]))); break;
        default: ;
        }
    } else if (_c == QMetaObject::IndexOfMethod) {
        int *result = reinterpret_cast<int *>(_a[0]);
        void **func = reinterpret_cast<void **>(_a[1]);
        {
            typedef void (MyFrame::*_t)(int );
            if (*reinterpret_cast<_t *>(func) == static_cast<_t>(&MyFrame::signalTimePosition)) {
                *result = 0;
            }
        }
    }
}

const QMetaObject MyFrame::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MyFrame.data,
      qt_meta_data_MyFrame,  qt_static_metacall, 0, 0}
};


const QMetaObject *MyFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *MyFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyFrame.stringdata))
        return static_cast<void*>(const_cast< MyFrame*>(this));
    return QMainWindow::qt_metacast(_clname);
}

int MyFrame::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QMainWindow::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 10)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 10;
    } else if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 10)
            *reinterpret_cast<int*>(_a[0]) = -1;
        _id -= 10;
    }
    return _id;
}

// SIGNAL 0
void MyFrame::signalTimePosition(int _t1)
{
    void *_a[] = { 0, const_cast<void*>(reinterpret_cast<const void*>(&_t1)) };
    QMetaObject::activate(this, &staticMetaObject, 0, _a);
}
QT_END_MOC_NAMESPACE
