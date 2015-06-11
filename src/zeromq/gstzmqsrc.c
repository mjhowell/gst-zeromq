/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
 * Copyright (C) <2004> Thomas Vander Stichele <thomas at apestaart dot org>
 * Copyright (C) <2011> Collabora Ltd.
 *     Author: Sebastian Dröge <sebastian.droege@collabora.co.uk>
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
 * SECTION:element-zmqsrc
 * @see_also: #zmqsink
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

#include <errno.h>
#include <string.h> // for memcpy

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstzmq.h"
#include "gstzmqsrc.h"

GST_DEBUG_CATEGORY_STATIC (zmqsrc_debug);
#define GST_CAT_DEFAULT zmqsrc_debug

static GstStaticPadTemplate srctemplate = GST_STATIC_PAD_TEMPLATE ("src",
    GST_PAD_SRC,
    GST_PAD_ALWAYS,
    GST_STATIC_CAPS_ANY);

enum
{
  PROP_0,
  PROP_ENDPOINT,
  PROP_BIND
};

#define gst_zmq_src_parent_class parent_class
G_DEFINE_TYPE (GstZmqSrc, gst_zmq_src, GST_TYPE_PUSH_SRC);

static void gst_zmq_src_finalize (GObject * gobject);

static GstCaps *gst_zmq_src_getcaps (GstBaseSrc * psrc,
    GstCaps * filter);

static GstFlowReturn gst_zmq_src_create (GstPushSrc * psrc,
    GstBuffer ** outbuf);
static gboolean gst_zmq_src_stop (GstBaseSrc * bsrc);
static gboolean gst_zmq_src_start (GstBaseSrc * bsrc);
static GstStateChangeReturn gst_zmq_src_change_state (GstElement * element, GstStateChange transition);

static void gst_zmq_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec);
static void gst_zmq_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec);

static void
gst_zmq_src_class_init (GstZmqSrcClass * klass)
{
  GObjectClass *gobject_class;
  GstElementClass *gstelement_class;
  GstBaseSrcClass *gstbasesrc_class;
  GstPushSrcClass *gstpush_src_class;

  gobject_class = (GObjectClass *) klass;
  gstelement_class = (GstElementClass *) klass;
  gstbasesrc_class = (GstBaseSrcClass *) klass;
  gstpush_src_class = (GstPushSrcClass *) klass;

  gobject_class->set_property = GST_DEBUG_FUNCPTR(gst_zmq_src_set_property);
  gobject_class->get_property = GST_DEBUG_FUNCPTR(gst_zmq_src_get_property);
  gobject_class->finalize = GST_DEBUG_FUNCPTR(gst_zmq_src_finalize);

  g_object_class_install_property (gobject_class, PROP_ENDPOINT,
      g_param_spec_string ("endpoint", "Endpoint",
          "ZeroMQ endpoint from which to receive buffers", ZMQ_DEFAULT_ENDPOINT_CLIENT,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));
  g_object_class_install_property (gobject_class, PROP_BIND,
          g_param_spec_boolean ("bind", "Bind", "If true, bind to the endpoint (be the \"server\")",
          ZMQ_DEFAULT_BIND_SRC,
          G_PARAM_READWRITE | G_PARAM_STATIC_STRINGS));

  gst_element_class_add_pad_template (gstelement_class,
      gst_static_pad_template_get (&srctemplate));

  gst_element_class_set_static_metadata (gstelement_class,
      "ZeroMQ source", "Source/Network",
      "Receive data on ZeroMQ SUB socket",
      "Mark J. Howell <m0ppy at hypgnosys dot org>");

  gstelement_class->change_state = GST_DEBUG_FUNCPTR(gst_zmq_src_change_state);

  gstbasesrc_class->get_caps = GST_DEBUG_FUNCPTR(gst_zmq_src_getcaps);
  gstbasesrc_class->start = GST_DEBUG_FUNCPTR(gst_zmq_src_start);
  gstbasesrc_class->stop = GST_DEBUG_FUNCPTR(gst_zmq_src_stop);

  gstpush_src_class->create = GST_DEBUG_FUNCPTR(gst_zmq_src_create);

  GST_DEBUG_CATEGORY_INIT (zmqsrc_debug, "zmqsrc", 0,
      "ZeroMQ Source");
}

static void
gst_zmq_src_init (GstZmqSrc * this)
{
  this->endpoint = g_strdup(ZMQ_DEFAULT_ENDPOINT_CLIENT);
  this->bind = ZMQ_DEFAULT_BIND_SRC;
  this->context = zmq_ctx_new();
}

static void
gst_zmq_src_finalize (GObject * gobject)
{
  GstZmqSrc *this = GST_ZMQ_SRC (gobject);
  zmq_ctx_destroy(this->context);
  G_OBJECT_CLASS (parent_class)->finalize (gobject);
}

static GstCaps *
gst_zmq_src_getcaps (GstBaseSrc * bsrc, GstCaps * filter)
{
  GstZmqSrc *src;
  GstCaps *caps = NULL;

  src = GST_ZMQ_SRC (bsrc);

  caps = (filter ? gst_caps_ref (filter) : gst_caps_new_any ());

  GST_DEBUG_OBJECT (src, "returning caps %" GST_PTR_FORMAT, caps);
  g_assert (GST_IS_CAPS (caps));
  return caps;
}

static GstFlowReturn
gst_zmq_src_create (GstPushSrc * psrc, GstBuffer ** outbuf)
{
  GstZmqSrc *src;
  GstFlowReturn retval = GST_FLOW_OK;
  GstMapInfo map;
  
  src = GST_ZMQ_SRC (psrc);

  GST_LOG_OBJECT (src, "was asked for a buffer");
  
  zmq_msg_t msg;
  int rc = zmq_msg_init(&msg);
  if (rc) {
    GST_ELEMENT_ERROR(src, RESOURCE, FAILED, ("zmq_msg_init() failed with error code %d [%s]", errno, zmq_strerror(errno)), NULL);
    retval = GST_FLOW_ERROR;
    goto done;
  }
  
  while(1) {
    rc = zmq_msg_recv(&msg, src->socket, 0);
    if ((rc < 0) && (EAGAIN == errno)) {
      GST_LOG_OBJECT (src, "No message available on socket");
      continue;
    } else {
      break;
    }
  }
  
  if (rc < 0) {
    if (ENOTSOCK == errno) {
      GST_DEBUG_OBJECT (src, "Connection closed");
      retval = GST_FLOW_EOS;
/*    } else if (EAGAIN == errno) {
      GST_LOG_OBJECT (src, "No message available on socket");
      g_print("w");*/
    } else {
      GST_ELEMENT_ERROR(src, RESOURCE, READ, ("zmq_msg_recv() failed with error code %d [%s]", errno, zmq_strerror(errno)), NULL);
      retval = GST_FLOW_ERROR;
    }
    *outbuf = NULL;
    zmq_msg_close(&msg);
    goto done;
  }
  size_t msg_size = zmq_msg_size(&msg);
  
  *outbuf = gst_buffer_new_and_alloc(msg_size);
  gst_buffer_map(*outbuf, &map, GST_MAP_READWRITE);
  
  memcpy(map.data, zmq_msg_data(&msg), msg_size);
  
  gst_buffer_unmap(*outbuf, &map);
  
  zmq_msg_close(&msg);

  GST_LOG_OBJECT (src, "delivered a buffer of size %" G_GSIZE_FORMAT " bytes", msg_size);  
done:
  return retval;
}

static void
gst_zmq_src_set_property (GObject * object, guint prop_id,
    const GValue * value, GParamSpec * pspec)
{
  GstZmqSrc *zmqsrc = GST_ZMQ_SRC (object);

  switch (prop_id) {
    case PROP_ENDPOINT:
      if (!g_value_get_string (value)) {
        g_warning ("endpoint property cannot be NULL");
        break;
      }
      g_free (zmqsrc->endpoint);
      zmqsrc->endpoint = g_strdup (g_value_get_string (value));
      break;
    case PROP_BIND:
      zmqsrc->bind = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static void
gst_zmq_src_get_property (GObject * object, guint prop_id,
    GValue * value, GParamSpec * pspec)
{
  GstZmqSrc *zmqsrc = GST_ZMQ_SRC (object);

  switch (prop_id) {
    case PROP_ENDPOINT:
      g_value_set_string (value, zmqsrc->endpoint);
      break;
    case PROP_BIND:
      g_value_set_boolean (value, zmqsrc->bind);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, prop_id, pspec);
      break;
  }
}

static gboolean
gst_zmq_src_start (GstBaseSrc * bsrc)
{
  GstZmqSrc *src = GST_ZMQ_SRC (bsrc);
  
  GST_DEBUG_OBJECT (src, "starting");  

  return TRUE;
}

static gboolean
gst_zmq_src_stop (GstBaseSrc * bsrc)
{
  GstZmqSrc *src;

  src = GST_ZMQ_SRC (bsrc);
  
  GST_DEBUG_OBJECT (src, "stopping");

  return TRUE;
}

static gboolean
gst_zmq_src_open (GstZmqSrc * src) {

  gboolean retval = TRUE;

  int rc;

  src->socket = zmq_socket(src->context, ZMQ_SUB);
  if (!src->socket) {
    GST_ELEMENT_ERROR(src, RESOURCE, OPEN_READ_WRITE, ("zmq_socket() failed with error code %d [%s]", errno, zmq_strerror(errno)), NULL);
    retval = FALSE;
  }

  if (retval) {
      if (src->bind) {
        GST_DEBUG("binding to endpoint %s", src->endpoint);
        rc = zmq_bind(src->socket, src->endpoint);
        if (rc) {
          GST_ELEMENT_ERROR(src, RESOURCE, OPEN_READ_WRITE, ("zmq_bind() to endpoint \"%s\" failed with error code %d [%s]", src->endpoint, errno, zmq_strerror(errno)), NULL);
          retval = FALSE;
        }
      } else {
        GST_DEBUG("connecting to endpoint %s", src->endpoint);
        rc = zmq_connect(src->socket, src->endpoint);
        if (rc) {
          GST_ELEMENT_ERROR(src, RESOURCE, OPEN_READ_WRITE, ("zmq_connect() to endpoint \"%s\" failed with error code %d [%s]", src->endpoint, errno, zmq_strerror(errno)), NULL);
          retval = FALSE;
        }
      }
    }

  if (retval) {
      rc = zmq_setsockopt(src->socket, ZMQ_SUBSCRIBE, "", 0);
      if (rc) {
        GST_ELEMENT_ERROR(src, RESOURCE, OPEN_READ_WRITE, ("zmq_setsockopt() failed with error code %d [%s]", errno, zmq_strerror(errno)), NULL);
        retval = FALSE;
      }
  }

  if (retval) {
      int timeout_ms = 1000;
      rc = zmq_setsockopt(src->socket, ZMQ_RCVTIMEO, &timeout_ms, sizeof(timeout_ms));
      if (rc) {
        GST_ELEMENT_ERROR(src, RESOURCE, OPEN_READ_WRITE, ("zmq_setsockopt() failed with error code %d [%s]", errno, zmq_strerror(errno)), NULL);
        retval = FALSE;
      }
  }

  return retval;
}

static gboolean
gst_zmq_src_close (GstZmqSrc * src) {

  gboolean retval = TRUE;

  int rc = zmq_close(src->socket);

  if (rc) {
    GST_ELEMENT_WARNING(src, RESOURCE, CLOSE, ("zmq_close() failed with error code %d [%s]", errno, strerror(errno)), NULL);
    retval = FALSE;
  }

  return retval;
}

static GstStateChangeReturn
gst_zmq_src_change_state (GstElement * element, GstStateChange transition)
{
  GstZmqSrc *src;
  GstStateChangeReturn result;

  src = GST_ZMQ_SRC (element);

  switch (transition) {
    case GST_STATE_CHANGE_NULL_TO_READY:
      if (!gst_zmq_src_open (src))
        goto open_failed;
      break;
    default:
      break;
  }
  if ((result =
          GST_ELEMENT_CLASS (parent_class)->change_state (element,
              transition)) == GST_STATE_CHANGE_FAILURE)
    goto failure;

  switch (transition) {
    case GST_STATE_CHANGE_READY_TO_NULL:
      gst_zmq_src_close (src);
      break;
    default:
      break;
  }
  return result;
  /* ERRORS */
open_failed:
  {
    GST_DEBUG_OBJECT (src, "failed to open socket");
    return GST_STATE_CHANGE_FAILURE;
  }
failure:
  {
    GST_DEBUG_OBJECT (src, "parent failed state change");
    return result;
  }
}

