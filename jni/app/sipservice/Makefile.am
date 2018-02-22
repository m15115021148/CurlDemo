AUTOMAKE_OPTIONS = foreign
AM_CPPFLAGS = -g -Wall -Werror \
              -D_STLP_USE_NEWALLOC -pthread -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -D__DragonFly__ \
              -I$(top_srcdir) \
              -I$(top_srcdir)/framework \
              -I$(top_srcdir)/util \
              -I$(top_srcdir)/libs/rapidjson/include

AM_CFLAGS = -g -Wall -Werror \
            -D_STLP_USE_NEWALLOC -pthread -DPJ_IS_BIG_ENDIAN=0 -DPJ_IS_LITTLE_ENDIAN=1 -D__DragonFly__ \
            -I$(top_srcdir) \
            -I$(top_srcdir)/framework \
            -I$(top_srcdir)/util \
            -I$(top_srcdir)/libs/rapidjson/include

AM_LDFLAGS = -pthread -lrt -rdynamic \
            -lpj \
            -lpjlib-util \
            -lpjsua \
            -lpjsip \
            -lpjsip-simple \
            -lpjsip-ua \
            -lpjmedia \
            -lpjmedia-audiodev \
            -lpjmedia-codec \
            -lresample \
            -lspeex \
            -lsrtp \
            -lixml \
            -lthreadutil \
            -lupnp



bin_PROGRAMS = cubic-sipd
cubic_sipd_SOURCES = SipService.cc

