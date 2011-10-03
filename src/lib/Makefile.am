lib_LTLIBRARIES = libcec.la
library_includedir=$(includedir)/libcec
library_include_HEADERS = ../../include/CECExports.h \
                          ../../include/CECExportsCpp.h \
                          ../../include/CECExportsC.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcec.pc


libcec_la_SOURCES = CECParser.cpp \
                    CECParser.h \
                    CECParserC.cpp \
                    CECDetect.cpp \
                    CECDetect.h \
                    AdapterCommunication.cpp \
                    AdapterCommunication.h \
                    ../../include/CECExports.h \
                    ../../include/CECExportsCpp.h \
                    ../../include/CECExportsC.h \
                    util/misc.h \
                    util/misc.cpp \
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
