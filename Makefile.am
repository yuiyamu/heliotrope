bin_PROGRAMS = heliotrope
heliotrope_SOURCES = \
    heliotrope.c \
    heliotrope.h \
    cli.c

# should work with down to zlib 0.99, since that's when deflateInit2 was added
# PKG_CHECK_MODULES([ZLIB], [zlib >= 0.99])
# i hate autotools so ill set this up properly later~

AM_CFLAGS = -Wextra
heliotrope_LDFLAGS = -lz
