# libgsft: Common functions.

AUTOMAKE_OPTIONS = foreign subdir-objects

if GENS_OS_WIN32
SUBDIRS = w32u
endif

noinst_LTLIBRARIES = libgsft.la

libgsft_la_CFLAGS	= $(AM_CFLAGS) -O3 -funroll-loops
libgsft_la_CXXFLAGS	= $(AM_CXXFLAGS) -O3 -funroll-loops
libgsft_la_LDFLAGS	= $(AM_LDFLAGS)
libgsft_la_LIBTOOLFLAGS	= $(AM_LIBTOOLFLAGS) --tag=disable-shared

if GENS_OS_LINUX
libgsft_la_CFLAGS	+= -fPIC -DPIC
libgsft_la_CXXFLAGS	+= -fPIC -DPIC
libgsft_la_LDFLAGS	+= -fPIC -DPIC
endif

libgsft_la_SOURCES = \
		gsft_byteswap.c \
		gsft_file.c \
		gsft_space_elim.c

noinst_HEADERS = \
		gsft_fncall.h \
		gsft_hashtable.hpp \
		gsft_byteswap.h \
		gsft_bool.h \
		gsft_fastcall.h \
		gsft_malloc_align.h \
		gsft_unused.h \
		gsft_file.h \
		gsft_szprintf.h \
		gsft_strlcpy.h \
		gsft_space_elim.h \
		gsft_pure.h

if !HAVE_STRDUP
libgsft_la_SOURCES	+= gsft_strdup.c
noinst_HEADERS		+= gsft_strdup.h
endif

if !HAVE_STRSEP
libgsft_la_SOURCES	+= gsft_strsep.c
noinst_HEADERS		+= gsft_strsep.h
endif

if !HAVE_STRLCPY
libgsft_la_SOURCES	+= strlcpy.c
endif

if !HAVE_STRLCAT
libgsft_la_SOURCES	+= strlcat.c
endif

# PNG-specific code.
if GENS_PNG

noinst_LTLIBRARIES += libgsft_png.la

libgsft_png_la_CFLAGS		= $(libgsft_la_CFLAGS) @PNG_CFLAGS@
libgsft_png_la_CXXFLAGS		= $(libgsft_la_CXXFLAGS) @PNG_CFLAGS@
libgsft_png_la_LDFLAGS		= $(libgsft_la_LDFLAGS)
libgsft_png_la_LIBADD		= @PNG_LIBS@
libgsft_png_la_LIBTOOLFLAGS	= $(libgsft_la_LIBTOOLFLAGS)

INCLUDES =

if GENS_ZLIB_INTERNAL
INCLUDES += -I@top_srcdir@/src/extlib/zlib/
endif

if GENS_PNG_INTERNAL
INCLUDES += -I@top_srcdir@/src/extlib/libpng/
endif

libgsft_png_la_SOURCES	= gsft_png.c
noinst_HEADERS		+= gsft_png.h gsft_png_dll.h

if GENS_PNG_DLOPEN
libgsft_png_la_SOURCES	+= gsft_png_dll.c
endif

endif

# iconv-specific code.
if HAVE_ICONV

noinst_LTLIBRARIES += libgsft_iconv.la

libgsft_iconv_la_CFLAGS		= $(libgsft_la_CFLAGS)
libgsft_iconv_la_CXXFLAGS	= $(libgsft_la_CXXFLAGS)
libgsft_iconv_la_LDFLAGS	= $(libgsft_la_LDFLAGS)
libgsft_iconv_la_LIBADD		= @ICONV_LIBS@
libgsft_iconv_la_LIBTOOLFLAGS	= $(libgsft_la_LIBTOOLFLAGS)

libgsft_iconv_la_SOURCES	= gsft_iconv.c
noinst_HEADERS			+= gsft_iconv.h

endif # HAVE_ICONV

# Win32-specific code.
if GENS_OS_WIN32

noinst_LTLIBRARIES += libgsft_win32.la libgsft_win32_gdi.la

# General Win32 stuff.
libgsft_win32_la_CFLAGS		= $(libgsft_la_CFLAGS)
libgsft_win32_la_CXXFLAGS	= $(libgsft_la_CXXFLAGS)
libgsft_win32_la_LDFLAGS	= $(libgsft_la_LDFLAGS)
libgsft_win32_la_LIBTOOLFLAGS	= $(libgsft_la_LIBTOOLFLAGS)

libgsft_win32_la_SOURCES = \
		gsft_win32.c
noinst_HEADERS += \
		gsft_win32.h

# Win32 GDI stuff.
libgsft_win32_gdi_la_CFLAGS		= $(libgsft_win32_la_CFLAGS)
libgsft_win32_gdi_la_CXXFLAGS		= $(libgsft_win32_la_CXXFLAGS)
libgsft_win32_gdi_la_LDFLAGS		= $(libgsft_win32_la_LDFLAGS)
libgsft_win32_gdi_la_LIBADD		= -lgdi32
libgsft_win32_gdi_la_LIBTOOLFLAGS	= $(libgsft_la_LIBTOOLFLAGS)

libgsft_win32_gdi_la_SOURCES = \
		gsft_win32_gdi.c
noinst_HEADERS += \
		gsft_win32_gdi.h

endif
