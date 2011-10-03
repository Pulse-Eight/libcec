lib_LTLIBRARIES = libcec.la
library_includedir=$(includedir)/libcec
library_include_HEADERS = ../../include/CECExports.h \
                          ../../include/CECExportsCpp.h \
                          ../../include/CECExportsC.h

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
                    ../../include/CECExports.h \
                    ../../include/CECExportsCpp.h \
                    ../../include/CECExportsC.h \
                    util/misc.cpp \
                    util/misc.h \
                    util/StdString.h \
                    util/threads.cpp \
                    util/threads.h \
                    util/timeutils.h \
                    libPlatform/baudrate.h \
                    libPlatform/os-dependent.h \
                    libPlatform/linux/os_posix.h \
                    libPlatform/linux/serialport.cpp \
                    libPlatform/serialport.h

libcec_la_LDFLAGS = -lrt -lpthread -ludev -version-info @VERSION@
