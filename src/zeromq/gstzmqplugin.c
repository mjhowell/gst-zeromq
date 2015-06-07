/* GStreamer
 * Copyright (C) <1999> Erik Walthinsen <omega@cse.ogi.edu>
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "gstzmqsrc.h"
#include "gstzmqsink.h"

GST_DEBUG_CATEGORY (zmq_debug);

static gboolean
plugin_init (GstPlugin * plugin)
{
  if (!gst_element_register (plugin, "zmqsrc", GST_RANK_NONE,
          GST_TYPE_ZMQ_SRC))
    return FALSE;

  if (!gst_element_register (plugin, "zmqsink", GST_RANK_NONE,
          GST_TYPE_ZMQ_SINK))
    return FALSE;

  GST_DEBUG_CATEGORY_INIT (zmq_debug, "zmq", 0, "ZeroMQ calls");

  return TRUE;
}

GST_PLUGIN_DEFINE (GST_VERSION_MAJOR,
    GST_VERSION_MINOR,
    zmq,
    "Transfer data via ZeroMQ",
    plugin_init, VERSION, GST_LICENSE, "gst-zeromq", "http://github.com/mjhowell/gst-zeromq")
