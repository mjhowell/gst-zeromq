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

/**
 * SECTION:element-zmqsink
 * @see_also: #zmqsrc
 *
 * <refsect2>
 * <title>Example launch line</title>
 * |[
 * # server:
 * gst-launch-1.0 fdsrc fd=0 ! zmqsink
 * # client:
 * gst-launch-1.0 zmqsrc ! fdsink fd=1
 * ]| everything you type in the server is shown on the client
 * </refsect2>
 */

#include <string.h> // for memcpy

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstzmq.h"
#include "gstzmqsink.h"

GST_DEBUG_CATEGORY_STATIC (zmqsink_debug);
#define GST_CAT_DEFAULT (zmqsink_debug)

static GstStaticPadTemplate sinktemplate = GST_STATIC_PAD_TEMPLATE ("sink",
    GST_PAD_SINK,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

enum
{
  PROP_0,
  PROP_ENDPOINT,
  PROP_BIND
};

static void gst_zmq_sink_finalize (GObject * gobject);

static void gst_zmq_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_zmq_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static gboolean gst_zmq_sink_start (GstBaseSink * sink);
static gboolean gst_zmq_sink_stop (GstBaseSink * sink);
static GstFlowReturn gst_zmq_sink_render (GstBaseSink * sink,
    GstBuffer * buffer);

#define gst_zmq_sink_parent_class parent_class
G_DEFINE_TYPE (GstZmqSink, gst_zmq_sink,  GST_TYPE_BASE_SINK);
    
static void
gst_zmq_sink_class_init (GstZmqSinkClass * klass)
{
  GST_DEBUG("zmqsink class init");
  
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSinkClass *gstbasesink_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesink_class = (GstBaseSinkClass *) klass;

  gobject_class->set_property = gst_zmq_sink_set_property;
  gobject_class->get_property = gst_zmq_sink_get_property;
  gobject_class->finalize = gst_zmq_sink_finalize;


  g_object_class_install_property (gobject_class, PROP_ENDPOINT,
      g_param_spec_string ("endpoint", "Endpoint",
          "ZeroMQ endpoint through which to send buffers", ZMQ_DEFAULT_ENDPOINT_SERVER,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
          
  g_object_class_install_property (gobject_class, PROP_BIND,
          g_param_spec_boolean ("bind", "Bind", "If true, bind to the endpoint (be the \"server\")",
          ZMQ_DEFAULT_BIND_SINK,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&sinktemplate));

  gst_element_class_set_static_metadata (gstelement_class,
      "ZeroMQ sink", "Sink/Network",
      "Send data on ZeroMQ PUB socket",
      "Mark J. Howell <m0ppy at hypgnosys dot org>");

  gstbasesink_class->start = GST_DEBUG_FUNCPTR (gst_zmq_sink_start);
  gstbasesink_class->stop = GST_DEBUG_FUNCPTR (gst_zmq_sink_stop);
  gstbasesink_class->render = GST_DEBUG_FUNCPTR (gst_zmq_sink_render);

  GST_DEBUG_CATEGORY_INIT (zmqsink_debug, "zmqsink", 0, "ZeroMQ Sink");
}

static void
gst_zmq_sink_init (GstZmqSink * this)
{
  this->endpoint = ZMQ_DEFAULT_ENDPOINT_SERVER;
  this->bind = ZMQ_DEFAULT_BIND_SINK;
  this->context = zmq_ctx_new();
}

static void
gst_zmq_sink_finalize (GObject * gobject)
{
  GstZmqSink *this = GST_ZMQ_SINK (gobject);
  zmq_ctx_destroy(this->context);
  G_OBJECT_CLASS (parent_class)->finalize (gobject);
}

static void
gst_zmq_sink_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstZmqSink *sink;

  sink = GST_ZMQ_SINK (object);

  switch (prop_id) {
    case PROP_ENDPOINT:
      if (!g_value_get_string (value)) {
        g_warning ("endpoint property cannot be NULL");
        break;
      }
      g_free (sink->endpoint);
      sink->endpoint = g_strdup (g_value_get_string (value));
      break;
    case PROP_BIND:
      sink->bind = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_zmq_sink_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstZmqSink *sink;

  sink = GST_ZMQ_SINK (object);

  switch (prop_id) {
    case PROP_ENDPOINT:
      g_value_set_string (value, sink->endpoint);
      break;
    case PROP_BIND:
      g_value_set_boolean (value, sink->bind);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static GstFlowReturn
gst_zmq_sink_render (GstBaseSink * basesink, GstBuffer * buffer)
{

  GstZmqSink *sink;
  GstMapInfo map;
  
  sink = GST_ZMQ_SINK (basesink);
  
  gst_buffer_map (buffer, &map, GST_MAP_READ);
  
  GST_DEBUG_OBJECT(sink, "publishing %" G_GSIZE_FORMAT " bytes", map.size);
  
  if (map.size > 0 && map.data != NULL) {
    zmq_msg_t msg;
    int rc = zmq_msg_init_size(&msg, map.size);
    g_assert(rc==0);
    memcpy(zmq_msg_data(&msg), map.data, map.size);
    rc = zmq_msg_send(&msg, sink->socket, 0);
    g_assert(rc==map.size);
  }
  
  gst_buffer_unmap (buffer, &map);

  return GST_FLOW_OK;

}

static gboolean
gst_zmq_sink_start (GstBaseSink * basesink)
{
  GstZmqSink *sink;
  //Gerror *err = NULL;
  
  sink = GST_ZMQ_SINK (basesink);
  
  int rc;
  
  GST_DEBUG_OBJECT (sink, "starting");  
  
  sink->socket = zmq_socket(sink->context, ZMQ_PUB);
  g_assert(sink->socket);
  
  if (sink->bind) {
    GST_DEBUG("binding to endpoint %s", sink->endpoint);
    rc = zmq_bind(sink->socket, sink->endpoint);
  } else {
    GST_DEBUG("connecting to endpoint %s", sink->endpoint);
    rc = zmq_connect(sink->socket, sink->endpoint);
  }
  g_assert (rc==0);
  
  return TRUE;
  
}

static gboolean
gst_zmq_sink_stop (GstBaseSink * basesink)
{
  GstZmqSink *sink;
  //Gerror *err = NULL;

  sink = GST_ZMQ_SINK (basesink);
  
  GST_DEBUG_OBJECT (sink, "stopping");
  
  zmq_close(sink->socket);

  return TRUE;
}
