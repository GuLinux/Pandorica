#ifndef VERSIONS_COMPAT
#define VERSIONS_COMPAT

#ifdef HAVE_QT
  #include <QtCore>
  
  #if QT_VERSION >= QT_VERSION_CHECK(5, 4, 0)
    #define HAVE_QT_QSTORAGEINFO 1
  #endif

#endif

#include <Wt/WConfig.h>
#if WT_SERIES == 3 && WT_MAJOR == 3 && WT_MINOR <= 3
  #warning Old Wt Auth API Version detected, disabling some auth features...
#else
  #define WT_AUTH_NEWAPI 1
#endif

#endif
