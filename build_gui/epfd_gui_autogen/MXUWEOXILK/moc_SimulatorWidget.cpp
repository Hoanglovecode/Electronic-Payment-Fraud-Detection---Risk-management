/****************************************************************************
** Meta object code from reading C++ file 'SimulatorWidget.hpp'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.2)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/gui/SimulatorWidget.hpp"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SimulatorWidget.hpp' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.2. It"
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
struct qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t {};
} // unnamed namespace

template <> constexpr inline auto epfd::gui::SimulatorWidget::qt_create_metaobjectdata<qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "epfd::gui::SimulatorWidget",
        "triggerScenarioBatch",
        "",
        "SimulationScenario",
        "scenario",
        "size_t",
        "count",
        "startContinuousStream",
        "interval_ms",
        "pauseContinuousStream",
        "onContinuousToggled"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'triggerScenarioBatch'
        QtMocHelpers::SignalData<void(SimulationScenario, size_t)>(1, 2, QMC::AccessPublic, QMetaType::Void, {{
            { 0x80000000 | 3, 4 }, { 0x80000000 | 5, 6 },
        }}),
        // Signal 'startContinuousStream'
        QtMocHelpers::SignalData<void(int)>(7, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 8 },
        }}),
        // Signal 'pauseContinuousStream'
        QtMocHelpers::SignalData<void()>(9, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onContinuousToggled'
        QtMocHelpers::SlotData<void()>(10, 2, QMC::AccessPrivate, QMetaType::Void),
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SimulatorWidget, qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject epfd::gui::SimulatorWidget::staticMetaObject = { {
    QMetaObject::SuperData::link<QWidget::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>.metaTypes,
    nullptr
} };

void epfd::gui::SimulatorWidget::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SimulatorWidget *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->triggerScenarioBatch((*reinterpret_cast<std::add_pointer_t<SimulationScenario>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<size_t>>(_a[2]))); break;
        case 1: _t->startContinuousStream((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        case 2: _t->pauseContinuousStream(); break;
        case 3: _t->onContinuousToggled(); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SimulatorWidget::*)(SimulationScenario , size_t )>(_a, &SimulatorWidget::triggerScenarioBatch, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SimulatorWidget::*)(int )>(_a, &SimulatorWidget::startContinuousStream, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SimulatorWidget::*)()>(_a, &SimulatorWidget::pauseContinuousStream, 2))
            return;
    }
}

const QMetaObject *epfd::gui::SimulatorWidget::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *epfd::gui::SimulatorWidget::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN4epfd3gui15SimulatorWidgetE_t>.strings))
        return static_cast<void*>(this);
    return QWidget::qt_metacast(_clname);
}

int epfd::gui::SimulatorWidget::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QWidget::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 4)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 4;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 4)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 4;
    }
    return _id;
}

// SIGNAL 0
void epfd::gui::SimulatorWidget::triggerScenarioBatch(SimulationScenario _t1, size_t _t2)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 0, nullptr, _t1, _t2);
}

// SIGNAL 1
void epfd::gui::SimulatorWidget::startContinuousStream(int _t1)
{
    QMetaObject::activate<void>(this, &staticMetaObject, 1, nullptr, _t1);
}

// SIGNAL 2
void epfd::gui::SimulatorWidget::pauseContinuousStream()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}
QT_WARNING_POP
