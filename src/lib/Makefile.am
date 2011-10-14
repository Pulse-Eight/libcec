lib_LTLIBRARIES = libcec.la
library_includedir=$(includedir)/libcec
library_include_HEADERS = ../../include/cec.h \
                          ../../include/cecc.h \
                          ../../include/cectypes.h \
                          ../../include/cecloader.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcec.pc

libcec_la_SOURCES = AdapterCommunication.cpp \
                    AdapterCommunication.h \
                    AdapterDetection.cpp \
                    AdapterDetection.h \
                    CECProcessor.cpp \
                    CECProcessor.h \
                    LibCEC.cpp \
                    LibCEC.h \
                    LibCECC.cpp \
                    util/StdString.h \
                    platform/timeutils.h \
                    platform/baudrate.h \
                    platform/os-dependent.h \
                    platform/linux/os_posix.h \
                    platform/linux/serialport.cpp \
                    platform/serialport.h \
                    platform/threads.cpp \
                    platform/threads.h

libcec_la_LDFLAGS = @LIBS@ -version-info @VERSION@
libcec_la_CPPFLAGS = -I@abs_top_srcdir@/include
