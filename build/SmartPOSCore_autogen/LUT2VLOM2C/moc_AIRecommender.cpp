/****************************************************************************
** Meta object code from reading C++ file 'AIRecommender.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.9.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/ai/AIRecommender.h"
#include <QtCore/qmetatype.h>
#include <QtCore/QList>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'AIRecommender.h' doesn't include <QObject>."
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
struct qt_meta_tag_ZN13AIRecommenderE_t {};
} // unnamed namespace

template <> constexpr inline auto AIRecommender::qt_create_metaobjectdata<qt_meta_tag_ZN13AIRecommenderE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "AIRecommender",
        "modelTrained",
        "",
        "success",
        "modelInfo",
        "recommendationsUpdated",
        "QList<int>",
        "productIds",
        "recommendationError",
        "errorMessage",
        "onAutoModelUpdate",
        "RecommendationStrategy",
        "CollaborativeFiltering",
        "ContentBasedFiltering",
        "HybridFiltering",
        "PopularityBased"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'modelTrained'
        QtMocHelpers::SignalData<void(bool, const QString &)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Bool, 3 }, { QMetaType::QString, 4 },
        }}),
        // Signal 'recommendationsUpdated'
        QtMocHelpers::SignalData<void(const QList<int> &)>(5, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 6, 7 },
        }}),
        // Signal 'recommendationError'
        QtMocHelpers::SignalData<void(const QString &)>(8, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 9 },
        }}),
        // Slot 'onAutoModelUpdate'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
        // enum 'RecommendationStrategy'
        QtMocHelpers::EnumData<RecommendationStrategy>(11, 11, QMC::EnumFlags{}).add({
            {   12, RecommendationStrategy::CollaborativeFiltering },
            {   13, RecommendationStrategy::ContentBasedFiltering },
            {   14, RecommendationStrategy::HybridFiltering },
            {   15, RecommendationStrategy::PopularityBased },
        }),
    };
    return QtMocHelpers::metaObjectData<AIRecommender, qt_meta_tag_ZN13AIRecommenderE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject AIRecommender::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AIRecommenderE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AIRecommenderE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN13AIRecommenderE_t>.metaTypes,
    nullptr
} };

void AIRecommender::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<AIRecommender *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->modelTrained((*reinterpret_cast< std::add_pointer_t<bool>>(_a[1])),(*reinterpret_cast< std::add_pointer_t<QString>>(_a[2]))); break;
        case 1: _t->recommendationsUpdated((*reinterpret_cast< std::add_pointer_t<QList<int>>>(_a[1]))); break;
        case 2: _t->recommendationError((*reinterpret_cast< std::add_pointer_t<QString>>(_a[1]))); break;
        case 3: _t->onAutoModelUpdate(); break;
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
                *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType::fromType< QList<int> >(); break;
            }
            break;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (AIRecommender::*)(bool , const QString & )>(_a, &AIRecommender::modelTrained, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (AIRecommender::*)(const QList<int> & )>(_a, &AIRecommender::recommendationsUpdated, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (AIRecommender::*)(const QString & )>(_a, &AIRecommender::recommendationError, 2))
            return;
    }
}

const QMetaObject *AIRecommender::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *AIRecommender::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN13AIRecommenderE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int AIRecommender::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
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
void AIRecommender::modelTrained(bool _t1, const QString & _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void AIRecommender::recommendationsUpdated(const QList<int> & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void AIRecommender::recommendationError(const QString & _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 2, nullptr, _t1);
}
QT_WARNING_POP
