/****************************************************************************
** Meta object code from reading C++ file 'DkDeckLinkPlugin.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.1)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../src/DkDeckLinkPlugin.h"
#include <QtGui/qtextcursor.h>
#include <QtCore/qmetatype.h>
#include <QtCore/qplugin.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'DkDeckLinkPlugin.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.1. It"
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
struct qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t {};
} // unnamed namespace

template <> constexpr inline auto nmc::DkDeckLinkPlugin::qt_create_metaobjectdata<qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "nmc::DkDeckLinkPlugin"
    };

    QtMocHelpers::UintData qt_methods {
    };
    QtMocHelpers::UintData qt_properties {
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<DkDeckLinkPlugin, qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject nmc::DkDeckLinkPlugin::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>.metaTypes,
    nullptr
} };

void nmc::DkDeckLinkPlugin::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<DkDeckLinkPlugin *>(_o);
    (void)_t;
    (void)_c;
    (void)_id;
    (void)_a;
}

const QMetaObject *nmc::DkDeckLinkPlugin::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *nmc::DkDeckLinkPlugin::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN3nmc16DkDeckLinkPluginE_t>.strings))
        return static_cast<void*>(this);
    if (!strcmp(_clname, "DkViewPortInterface"))
        return static_cast< DkViewPortInterface*>(this);
    if (!strcmp(_clname, "com.nomacs.ImageLounge.DkViewPortInterface/3.8"))
        return static_cast< nmc::DkViewPortInterface*>(this);
    return QObject::qt_metacast(_clname);
}

int nmc::DkDeckLinkPlugin::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    return _id;
}
using namespace nmc;

#ifdef QT_MOC_EXPORT_PLUGIN_V2
static constexpr unsigned char qt_pluginMetaDataV2_DkDeckLinkPlugin[] = {
    0xbf, 
    // "IID"
    0x02,  0x78,  0x2e,  'c',  'o',  'm',  '.',  'n', 
    'o',  'm',  'a',  'c',  's',  '.',  'I',  'm', 
    'a',  'g',  'e',  'L',  'o',  'u',  'n',  'g', 
    'e',  '.',  'D',  'k',  'V',  'i',  'e',  'w', 
    'P',  'o',  'r',  't',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e',  '/',  '3',  '.', 
    '8', 
    // "className"
    0x03,  0x70,  'D',  'k',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  'P',  'l',  'u',  'g', 
    'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x6a,  'A',  'u',  't',  'h',  'o', 
    'r',  'N',  'a',  'm',  'e',  0x70,  'T',  'h', 
    'a',  't',  'c',  'h',  'e',  'r',  ' ',  'F', 
    'r',  'e',  'e',  'm',  'a',  'n',  0x67,  'C', 
    'o',  'm',  'p',  'a',  'n',  'y',  0x60,  0x6b, 
    'D',  'a',  't',  'e',  'C',  'r',  'e',  'a', 
    't',  'e',  'd',  0x6a,  '2',  '0',  '2',  '6', 
    '-',  '0',  '6',  '-',  '0',  '6',  0x6c,  'D', 
    'a',  't',  'e',  'M',  'o',  'd',  'i',  'f', 
    'i',  'e',  'd',  0x6a,  '2',  '0',  '2',  '6', 
    '-',  '0',  '6',  '-',  '0',  '6',  0x6b,  'D', 
    'e',  's',  'c',  'r',  'i',  'p',  't',  'i', 
    'o',  'n',  0x78,  0xf1,  'O',  'u',  't',  'p', 
    'u',  't',  's',  ' ',  't',  'h',  'e',  ' ', 
    'c',  'u',  'r',  'r',  'e',  'n',  't',  ' ', 
    'i',  'm',  'a',  'g',  'e',  ' ',  'a',  's', 
    ' ',  'a',  ' ',  'c',  'l',  'e',  'a',  'n', 
    '-',  'f',  'e',  'e',  'd',  ' ',  's',  't', 
    'i',  'l',  'l',  ' ',  'f',  'r',  'a',  'm', 
    'e',  ' ',  't',  'o',  ' ',  'a',  ' ',  'c', 
    'o',  'n',  'n',  'e',  'c',  't',  'e',  'd', 
    ' ',  'B',  'l',  'a',  'c',  'k',  'm',  'a', 
    'g',  'i',  'c',  ' ',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'd',  'e',  'v', 
    'i',  'c',  'e',  '.',  ' ',  'S',  'u',  'p', 
    'p',  'o',  'r',  't',  's',  ' ',  'a',  'n', 
    'y',  ' ',  'm',  'o',  'd',  'e',  ' ',  't', 
    'h',  'e',  ' ',  'd',  'e',  'v',  'i',  'c', 
    'e',  ' ',  'r',  'e',  'p',  'o',  'r',  't', 
    's',  ' ',  'a',  's',  ' ',  's',  'u',  'p', 
    'p',  'o',  'r',  't',  'e',  'd',  ' ',  '(', 
    'S',  'D',  ',',  ' ',  'H',  'D',  ',',  ' ', 
    '2',  'K',  ',',  ' ',  '4',  'K',  ')',  '.', 
    ' ',  'T',  'h',  'e',  ' ',  'p',  'i',  'x', 
    'e',  'l',  ' ',  'v',  'a',  'l',  'u',  'e', 
    's',  ' ',  's',  'e',  'n',  't',  ' ',  'a', 
    'r',  'e',  ' ',  't',  'h',  'e',  ' ',  'd', 
    'i',  's',  'p',  'l',  'a',  'y',  '-',  'd', 
    'e',  'c',  'o',  'd',  'e',  'd',  ' ',  'c', 
    'o',  'd',  'e',  ' ',  'v',  'a',  'l',  'u', 
    'e',  's',  ' ',  'a',  's',  ' ',  's',  'e', 
    'e',  'n',  ' ',  'i',  'n',  ' ',  'n',  'o', 
    'm',  'a',  'c',  's',  '.',  0x68,  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'd',  0x78,  0x20, 
    '6',  'b',  '3',  'f',  '4',  'a',  '2',  'e', 
    '8',  'c',  '1',  'd',  '0',  'e',  '5',  'f', 
    '9',  'a',  '7',  'b',  '2',  'c',  '4',  'd', 
    '6',  'e',  '8',  'f',  '0',  'a',  '1',  'b', 
    0x6a,  'P',  'l',  'u',  'g',  'i',  'n',  'N', 
    'a',  'm',  'e',  0x6f,  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'O',  'u',  't', 
    'p',  'u',  't',  0x67,  'T',  'a',  'g',  'l', 
    'i',  'n',  'e',  0x78,  0x3d,  'S',  'e',  'n', 
    'd',  ' ',  't',  'h',  'e',  ' ',  'c',  'u', 
    'r',  'r',  'e',  'n',  't',  ' ',  'i',  'm', 
    'a',  'g',  'e',  ' ',  't',  'o',  ' ',  'a', 
    ' ',  'B',  'l',  'a',  'c',  'k',  'm',  'a', 
    'g',  'i',  'c',  ' ',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'v',  'i',  'd', 
    'e',  'o',  ' ',  'o',  'u',  't',  'p',  'u', 
    't',  '.',  0x67,  'V',  'e',  'r',  's',  'i', 
    'o',  'n',  0x65,  '0',  '.',  '1',  '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN_V2(nmc::DkDeckLinkPlugin, DkDeckLinkPlugin, qt_pluginMetaDataV2_DkDeckLinkPlugin)
#else
QT_PLUGIN_METADATA_SECTION
Q_CONSTINIT static constexpr unsigned char qt_pluginMetaData_DkDeckLinkPlugin[] = {
    'Q', 'T', 'M', 'E', 'T', 'A', 'D', 'A', 'T', 'A', ' ', '!',
    // metadata version, Qt version, architectural requirements
    0, QT_VERSION_MAJOR, QT_VERSION_MINOR, qPluginArchRequirements(),
    0xbf, 
    // "IID"
    0x02,  0x78,  0x2e,  'c',  'o',  'm',  '.',  'n', 
    'o',  'm',  'a',  'c',  's',  '.',  'I',  'm', 
    'a',  'g',  'e',  'L',  'o',  'u',  'n',  'g', 
    'e',  '.',  'D',  'k',  'V',  'i',  'e',  'w', 
    'P',  'o',  'r',  't',  'I',  'n',  't',  'e', 
    'r',  'f',  'a',  'c',  'e',  '/',  '3',  '.', 
    '8', 
    // "className"
    0x03,  0x70,  'D',  'k',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  'P',  'l',  'u',  'g', 
    'i',  'n', 
    // "MetaData"
    0x04,  0xa9,  0x6a,  'A',  'u',  't',  'h',  'o', 
    'r',  'N',  'a',  'm',  'e',  0x70,  'T',  'h', 
    'a',  't',  'c',  'h',  'e',  'r',  ' ',  'F', 
    'r',  'e',  'e',  'm',  'a',  'n',  0x67,  'C', 
    'o',  'm',  'p',  'a',  'n',  'y',  0x60,  0x6b, 
    'D',  'a',  't',  'e',  'C',  'r',  'e',  'a', 
    't',  'e',  'd',  0x6a,  '2',  '0',  '2',  '6', 
    '-',  '0',  '6',  '-',  '0',  '6',  0x6c,  'D', 
    'a',  't',  'e',  'M',  'o',  'd',  'i',  'f', 
    'i',  'e',  'd',  0x6a,  '2',  '0',  '2',  '6', 
    '-',  '0',  '6',  '-',  '0',  '6',  0x6b,  'D', 
    'e',  's',  'c',  'r',  'i',  'p',  't',  'i', 
    'o',  'n',  0x78,  0xf1,  'O',  'u',  't',  'p', 
    'u',  't',  's',  ' ',  't',  'h',  'e',  ' ', 
    'c',  'u',  'r',  'r',  'e',  'n',  't',  ' ', 
    'i',  'm',  'a',  'g',  'e',  ' ',  'a',  's', 
    ' ',  'a',  ' ',  'c',  'l',  'e',  'a',  'n', 
    '-',  'f',  'e',  'e',  'd',  ' ',  's',  't', 
    'i',  'l',  'l',  ' ',  'f',  'r',  'a',  'm', 
    'e',  ' ',  't',  'o',  ' ',  'a',  ' ',  'c', 
    'o',  'n',  'n',  'e',  'c',  't',  'e',  'd', 
    ' ',  'B',  'l',  'a',  'c',  'k',  'm',  'a', 
    'g',  'i',  'c',  ' ',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'd',  'e',  'v', 
    'i',  'c',  'e',  '.',  ' ',  'S',  'u',  'p', 
    'p',  'o',  'r',  't',  's',  ' ',  'a',  'n', 
    'y',  ' ',  'm',  'o',  'd',  'e',  ' ',  't', 
    'h',  'e',  ' ',  'd',  'e',  'v',  'i',  'c', 
    'e',  ' ',  'r',  'e',  'p',  'o',  'r',  't', 
    's',  ' ',  'a',  's',  ' ',  's',  'u',  'p', 
    'p',  'o',  'r',  't',  'e',  'd',  ' ',  '(', 
    'S',  'D',  ',',  ' ',  'H',  'D',  ',',  ' ', 
    '2',  'K',  ',',  ' ',  '4',  'K',  ')',  '.', 
    ' ',  'T',  'h',  'e',  ' ',  'p',  'i',  'x', 
    'e',  'l',  ' ',  'v',  'a',  'l',  'u',  'e', 
    's',  ' ',  's',  'e',  'n',  't',  ' ',  'a', 
    'r',  'e',  ' ',  't',  'h',  'e',  ' ',  'd', 
    'i',  's',  'p',  'l',  'a',  'y',  '-',  'd', 
    'e',  'c',  'o',  'd',  'e',  'd',  ' ',  'c', 
    'o',  'd',  'e',  ' ',  'v',  'a',  'l',  'u', 
    'e',  's',  ' ',  'a',  's',  ' ',  's',  'e', 
    'e',  'n',  ' ',  'i',  'n',  ' ',  'n',  'o', 
    'm',  'a',  'c',  's',  '.',  0x68,  'P',  'l', 
    'u',  'g',  'i',  'n',  'I',  'd',  0x78,  0x20, 
    '6',  'b',  '3',  'f',  '4',  'a',  '2',  'e', 
    '8',  'c',  '1',  'd',  '0',  'e',  '5',  'f', 
    '9',  'a',  '7',  'b',  '2',  'c',  '4',  'd', 
    '6',  'e',  '8',  'f',  '0',  'a',  '1',  'b', 
    0x6a,  'P',  'l',  'u',  'g',  'i',  'n',  'N', 
    'a',  'm',  'e',  0x6f,  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'O',  'u',  't', 
    'p',  'u',  't',  0x67,  'T',  'a',  'g',  'l', 
    'i',  'n',  'e',  0x78,  0x3d,  'S',  'e',  'n', 
    'd',  ' ',  't',  'h',  'e',  ' ',  'c',  'u', 
    'r',  'r',  'e',  'n',  't',  ' ',  'i',  'm', 
    'a',  'g',  'e',  ' ',  't',  'o',  ' ',  'a', 
    ' ',  'B',  'l',  'a',  'c',  'k',  'm',  'a', 
    'g',  'i',  'c',  ' ',  'D',  'e',  'c',  'k', 
    'L',  'i',  'n',  'k',  ' ',  'v',  'i',  'd', 
    'e',  'o',  ' ',  'o',  'u',  't',  'p',  'u', 
    't',  '.',  0x67,  'V',  'e',  'r',  's',  'i', 
    'o',  'n',  0x65,  '0',  '.',  '1',  '.',  '0', 
    0xff, 
};
QT_MOC_EXPORT_PLUGIN(nmc::DkDeckLinkPlugin, DkDeckLinkPlugin)
#endif  // QT_MOC_EXPORT_PLUGIN_V2

QT_WARNING_POP
