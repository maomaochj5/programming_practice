/****************************************************************************
** Meta object code from reading C++ file 'Sale.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/models/Sale.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'Sale.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN4SaleE_t {};
} // unnamed namespace

template <> constexpr inline auto Sale::qt_create_metaobjectdata<qt_meta_tag_ZN4SaleE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "Sale",
        "saleChanged",
        "",
        "itemAdded",
        "SaleItem*",
        "item",
        "itemRemoved",
        "index",
        "totalChanged",
        "newTotal",
        "PaymentMethod",
        "Cash",
        "Card",
        "MobilePay",
        "GiftCard",
        "LoyaltyPoints",
        "Mixed",
        "TransactionStatus",
        "InProgress",
        "Completed",
        "Cancelled",
        "Refunded"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'saleChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'itemAdded'
        QtMocHelpers::SignalData<void(SaleItem *)>(3, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 4, 5 },
        }}),
        // Signal 'itemRemoved'
        QtMocHelpers::SignalData<void(int)>(6, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 7 },
        }}),
        // Signal 'totalChanged'
        QtMocHelpers::SignalData<void(double)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Double, 9 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'PaymentMethod'
        QtMocHelpers::EnumData<PaymentMethod>(10, 10, QMC::EnumFlags{}).add({
            {   11, PaymentMethod::Cash },
            {   12, PaymentMethod::Card },
            {   13, PaymentMethod::MobilePay },
            {   14, PaymentMethod::GiftCard },
            {   15, PaymentMethod::LoyaltyPoints },
            {   16, PaymentMethod::Mixed },
        }),
        // enum 'TransactionStatus'
        QtMocHelpers::EnumData<TransactionStatus>(17, 17, QMC::EnumFlags{}).add({
            {   18, TransactionStatus::InProgress },
            {   19, TransactionStatus::Completed },
            {   20, TransactionStatus::Cancelled },
            {   21, TransactionStatus::Refunded },
        }),
    };
    return QtMocHelpers::metaObjectData<Sale, qt_meta_tag_ZN4SaleE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject Sale::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4SaleE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4SaleE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN4SaleE_t>.metaTypes,
    nullptr
} };

void Sale::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<Sale *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->saleChanged(); break;
        case 1: _t->itemAdded((*reinterpret_cast< std::add_pointer_t<SaleItem*>>(_a[1]))); break;
        case 2: _t->itemRemoved((*reinterpret_cast< std::add_pointer_t<int>>(_a[1]))); break;
        case 3: _t->totalChanged((*reinterpret_cast< std::add_pointer_t<double>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        switch (_id) {
        default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
        case 1:
            switch (*reinterpret_cast<int*>(_a[1])) {
            default: *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType(); break;
            case 0:
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< SaleItem* >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (Sale::*)()>(_a, &Sale::saleChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (Sale::*)(SaleItem * )>(_a, &Sale::itemAdded, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (Sale::*)(int )>(_a, &Sale::itemRemoved, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (Sale::*)(double )>(_a, &Sale::totalChanged, 3))
            return;
    }
}

const QMetaObject *Sale::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *Sale::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4SaleE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int Sale::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void Sale::saleChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void Sale::itemAdded(SaleItem * _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void Sale::itemRemoved(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}

// SIGNAL 3
void Sale::totalChanged(double _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 3, nullptr, _t1);
}
QT_WARNING_POP
