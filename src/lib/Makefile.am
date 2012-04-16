lib_LTLIBRARIES = libcec.la
library_includedir=$(includedir)/libcec
library_include_HEADERS = ../../include/cec.h \
                          ../../include/cecc.h \
                          ../../include/cectypes.h \
                          ../../include/cecloader.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libcec.pc

libcec_la_SOURCES = CECProcessor.cpp \
                    LibCEC.cpp \
                    LibCECC.cpp \
                    adapter/USBCECAdapterCommands.cpp \
                    adapter/USBCECAdapterCommunication.cpp \
                    adapter/USBCECAdapterDetection.cpp \
                    adapter/USBCECAdapterMessage.cpp \
                    adapter/USBCECAdapterMessageQueue.cpp \
                    devices/CECAudioSystem.cpp \
                    devices/CECBusDevice.cpp \
                    devices/CECPlaybackDevice.cpp \
                    devices/CECRecordingDevice.cpp \
                    devices/CECTuner.cpp \
                    devices/CECTV.cpp \
                    implementations/ANCommandHandler.cpp \
                    implementations/CECCommandHandler.cpp \
                    implementations/SLCommandHandler.cpp \
                    implementations/VLCommandHandler.cpp \
                    implementations/RLCommandHandler.cpp \
                    platform/posix/serialport.cpp

libcec_la_LDFLAGS = @LIBS@ -version-info @VERSION@
libcec_la_CPPFLAGS = -I@abs_top_srcdir@/include
