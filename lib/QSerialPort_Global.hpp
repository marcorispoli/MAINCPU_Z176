#ifndef QSERIALPORT_GLOBAL_HPP
#define QSERIALPORT_GLOBAL_HPP

// This header allows for qmake library support
// this code gets built into a DLL or SO file depending on platform

#include <QtCore/qglobal.h>

#if defined(QSERIALPORT_LIBRARY)
#  define QSERIALPORTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define QSERIALPORTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // QSERIALPORT_GLOBAL_HPP
