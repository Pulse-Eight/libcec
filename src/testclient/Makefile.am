bin_PROGRAMS = cec-client
cec_client_SOURCES = main.cpp

if USE_CURSES_API
cec_client_SOURCES += Curses/CursesControl.cpp
endif

cec_client_CPPFLAGS = -I@abs_top_srcdir@/include
cec_client_LDFLAGS = @LIBS@
