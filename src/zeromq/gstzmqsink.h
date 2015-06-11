/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2004> Thomas Vander Stichele <thomas at apestaart dot org>
 * Copyright (C) <2015> Mark J. Howell <m0ppy at hypgnosys dot org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin St, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */


#ifndef __GST_ZMQ_SINK_H__
#define __GST_ZMQ_SINK_H__


#include <gst/gst.h>
#include <gst/base/gstbasesink.h>

G_BEGIN_DECLS

#define GST_TYPE_ZMQ_SINK \
  (gst_zmq_sink_get_type())
#define GST_ZMQ_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_CAST((obj),GST_TYPE_ZMQ_SINK,GstZmqSink))
#define GST_ZMQ_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_CAST((klass),GST_TYPE_ZMQ_SINK,GstZmqSinkClass))
#define GST_IS_ZMQ_SINK(obj) \
  (G_TYPE_CHECK_INSTANCE_TYPE((obj),GST_TYPE_ZMQ_SINK))
#define GST_IS_ZMQ_SINK_CLASS(klass) \
  (G_TYPE_CHECK_CLASS_TYPE((klass),GST_TYPE_ZMQ_SINK))

typedef struct _GstZmqSink GstZmqSink;
typedef struct _GstZmqSinkClass GstZmqSinkClass;

typedef enum {
  GST_ZMQ_SINK_OPEN             = (GST_ELEMENT_FLAG_LAST << 0),

  GST_ZMQ_SINK_FLAG_LAST        = (GST_ELEMENT_FLAG_LAST << 2)
} GstZmqSinkFlags;

struct _GstZmqSink {
  GstBaseSink parent;

  // properties
  gchar *endpoint;
  gboolean bind;
  
  // zmq stuff
  void *context;
  void *socket;
};

struct _GstZmqSinkClass {
  GstBaseSinkClass parent_class;
};

GType gst_zmq_sink_get_type (void);

G_END_DECLS

#endif /* __GST_ZMQ_SINK_H__ */
