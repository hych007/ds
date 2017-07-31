#ifndef DS_GLOBAL_H
#define DS_GLOBAL_H

#include <QtCore/qglobal.h>
#include <QApplication>
#include <QDesktopWidget>

#include <QWidget>
#include <QLabel>
#include <QPushButton>

#include <QBoxLayout>

#include <QMouseEvent>
#include <QKeyEvent>

#if defined(DS_LIBRARY)
#  define DSSHARED_EXPORT Q_DECL_EXPORT
#else
#  define DSSHARED_EXPORT Q_DECL_IMPORT
#endif

#include <string>
#include <deque>
#include <map>
#include <vector>

#include <ook/trace>
#include <ook/fourcc.h>

#include <ook/apps/transcoder/service/service.h>
#include <ook/apps/transcoder/service/director/director_service.h>

#include <ook/xmlparser>

#include <ook/separator>
#include <ook/macro.h>
#include <ook/tools/chkpath.h>

#include <ook/sock/http_client.h>
#include <ook/cycbuf2>

extern "C" {
    #include <ook/codecs/avdef.h>
}

#include "hdsctx.h"
#include "hrcloader.h"

#endif // DS_GLOBAL_H
