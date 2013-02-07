/****************************************************************************
** Meta object code from reading C++ file 'qcnqt.h'
**
** Created: Thu Feb 7 07:36:08 2013
**      by: The Qt Meta Object Compiler version 63 (Qt 4.8.4)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "qcnqt.h"
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'qcnqt.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 63
#error "This file was generated using the moc from 4.8.4. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

QT_BEGIN_MOC_NAMESPACE
static const uint qt_meta_data_MyApp[] = {

 // content:
       6,       // revision
       0,       // classname
       0,    0, // classinfo
       3,   14, // methods
       0,    0, // properties
       0,    0, // enums/sets
       0,    0, // constructors
       0,       // flags
       0,       // signalCount

 // slots: signature, parameters, type, tag, flags
       7,    6,    6,    6, 0x08,
      32,    6,    6,    6, 0x08,
      52,   48,    6,    6, 0x08,

       0        // eod
};

static const char qt_meta_stringdata_MyApp[] = {
    "MyApp\0\0slotGetLatestQuakeList()\0"
    "slotMakeQuake()\0qpr\0slotPrintPreview(QPrinter*)\0"
};

void MyApp::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    if (_c == QMetaObject::InvokeMetaMethod) {
        Q_ASSERT(staticMetaObject.cast(_o));
        MyApp *_t = static_cast<MyApp *>(_o);
        switch (_id) {
        case 0: _t->slotGetLatestQuakeList(); break;
        case 1: _t->slotMakeQuake(); break;
        case 2: _t->slotPrintPreview((*reinterpret_cast< QPrinter*(*)>(_a[1]))); break;
        default: ;
        }
    }
}

const QMetaObjectExtraData MyApp::staticMetaObjectExtraData = {
    0,  qt_static_metacall 
};

const QMetaObject MyApp::staticMetaObject = {
    { &QApplication::staticMetaObject, qt_meta_stringdata_MyApp,
      qt_meta_data_MyApp, &staticMetaObjectExtraData }
};

#ifdef Q_NO_DATA_RELOCATION
const QMetaObject &MyApp::getStaticMetaObject() { return staticMetaObject; }
#endif //Q_NO_DATA_RELOCATION

const QMetaObject *MyApp::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->metaObject : &staticMetaObject;
}

void *MyApp::qt_metacast(const char *_clname)
{
    if (!_clname) return 0;
    if (!strcmp(_clname, qt_meta_stringdata_MyApp))
        return static_cast<void*>(const_cast< MyApp*>(this));
    return QApplication::qt_metacast(_clname);
}

int MyApp::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QApplication::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 3)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 3;
    }
    return _id;
}
QT_END_MOC_NAMESPACE
