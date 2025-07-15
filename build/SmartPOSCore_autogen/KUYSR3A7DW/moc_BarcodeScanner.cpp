/****************************************************************************
** Meta object code from reading C++ file 'BarcodeScanner.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/barcode/BarcodeScanner.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'BarcodeScanner.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.9.1. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN14BarcodeScannerE_t {};
} // unnamed namespace

template <> constexpr inline auto BarcodeScanner::qt_create_metaobjectdata<qt_meta_tag_ZN14BarcodeScannerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "BarcodeScanner",
        "barcodeDetected",
        "",
        "barcode",
        "statusChanged",
        "ScannerStatus",
        "status",
        "scannerError",
        "errorMessage",
        "frameUpdated",
        "onCameraStatusChanged",
        "onCameraError",
        "error",
        "onSimulationTimer",
        "Stopped",
        "Starting",
        "Running",
        "Error"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'barcodeDetected'
        QtMocHelpers::SignalData<void(const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 3 },
        }}),
        // Signal 'statusChanged'
        QtMocHelpers::SignalData<void(ScannerStatus)>(4, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 5, 6 },
        }}),
        // Signal 'scannerError'
        QtMocHelpers::SignalData<void(const QString &)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 8 },
        }}),
        // Signal 'frameUpdated'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onCameraStatusChanged'
        QtMocHelpers::SlotData<void(int)>(10, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 6 },
        }}),
        // Slot 'onCameraError'
        QtMocHelpers::SlotData<void(int)>(11, 2, QMC::AccessPrivate, QMetaType::Void, {{
            { QMetaType::Int, 12 },
        }}),
        // Slot 'onSimulationTimer'
        QtMocHelpers::SlotData<void()>(13, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'ScannerStatus'
        QtMocHelpers::EnumData<ScannerStatus>(5, 5, QMC::EnumFlags{}).add({
            {   14, ScannerStatus::Stopped },
            {   15, ScannerStatus::Starting },
            {   16, ScannerStatus::Running },
            {   17, ScannerStatus::Error },
        }),
    };
    return QtMocHelpers::metaObjectData<BarcodeScanner, qt_meta_tag_ZN14BarcodeScannerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject BarcodeScanner::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BarcodeScannerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BarcodeScannerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN14BarcodeScannerE_t>.metaTypes,
    nullptr
} };

void BarcodeScanner::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<BarcodeScanner *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->barcodeDetected((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 1: _t->statusChanged((*reinterpret_cast< std::add_pointer_t<ScannerStatus>>(_a[1]))); break;
        case 2: _t->scannerError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->frameUpdated(); break;
        case 4: _t->onCameraStatusChanged((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 5: _t->onCameraError((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 6: _t->onSimulationTimer(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (BarcodeScanner::*)(const QString & )>(_a, &BarcodeScanner::barcodeDetected, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (BarcodeScanner::*)(ScannerStatus )>(_a, &BarcodeScanner::statusChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (BarcodeScanner::*)(const QString & )>(_a, &BarcodeScanner::scannerError, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (BarcodeScanner::*)()>(_a, &BarcodeScanner::frameUpdated, 3))
            return;
    }
}

const QMetaObject *BarcodeScanner::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *BarcodeScanner::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN14BarcodeScannerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int BarcodeScanner::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 7)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 7;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 7)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 7;
    }
    return _id;
}

// SIGNAL 0
void BarcodeScanner::barcodeDetected(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1);
}

// SIGNAL 1
void BarcodeScanner::statusChanged(ScannerStatus _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void BarcodeScanner::scannerError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void BarcodeScanner::frameUpdated()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}
QT_WARNING_POP
