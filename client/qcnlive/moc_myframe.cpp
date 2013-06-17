/****************************************************************************
** Meta object code from reading C++ file 'myframe.h'
**
** Created: Mon Jun 17 13:55:59 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "myframe.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'myframe.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyFrame[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
      10,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       1,       // signalCount

 // signals: signature, parameters, type, tag, flags
      14,    9,    8,    8, 0x05,

 // slots: signature, parameters, type, tag, flags
      38,    8,    8,    8, 0x08,
      59,    8,    8,    8, 0x08,
      75,    8,    8,    8, 0x08,
      88,    8,    8,    8, 0x08,
     108,    8,    8,    8, 0x08,
     129,    8,    8,    8, 0x08,
     154,    8,    8,    8, 0x08,
     173,    8,    8,    8, 0x08,
     186,    9,    8,    8, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MyFrame[] = {
    "MyFrame\0\0iPos\0signalTimePosition(int)\0"
    "fileDialogSettings()\0fileMakeQuake()\0"
    "actionView()\0actionOptionEarth()\0"
    "actionOptionSensor()\0actionOptionScreenshot()\0"
    "actionOptionLogo()\0actionHelp()\0"
    "slotTimePosition(int)\0"
};

void MyFrame::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
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
    }
}

const QMetaObjectExtraData MyFrame::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyFrame::staticMetaObject = {
    { &QMainWindow::staticMetaObject, qt_meta_stringdata_MyFrame,
      qt_meta_data_MyFrame, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyFrame::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyFrame::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyFrame::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyFrame))
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
