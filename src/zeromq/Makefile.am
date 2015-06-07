plugin_LTLIBRARIES = libgstzmq.la

libgstzmq_la_SOURCES = \
	gstzmqplugin.c \
	gstzmqsrc.c \
	gstzmqsink.c

libgstzmq_la_CFLAGS = $(GST_CFLAGS)
libgstzmq_la_LDFLAGS = $(GST_PLUGIN_LDFLAGS) -lzmq
libgstzmq_la_LIBADD = $(GST_BASE_LIBS) $(GST_NET_LIBS) $(GST_LIBS) $(GIO_LIBS)
libgstzmq_la_LIBTOOLFLAGS = $(GST_PLUGIN_LIBTOOLFLAGS)

noinst_HEADERS = \
  gstzmqsrc.h \
  gstzmqsink.h \
  gstzmq.h

CLEANFILES = $(BUILT_SOURCES)
