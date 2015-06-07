#ifndef __GST_ZMQ_H_
#define __GST_ZMQ_H_

#include <gst/gst.h>
#include <zmq.h>

#define ZMQ_DEFAULT_BIND_SRC FALSE
#define ZMQ_DEFAULT_BIND_SINK TRUE

#define ZMQ_DEFAULT_ENDPOINT_SERVER "tcp://*:5556"
#define ZMQ_DEFAULT_ENDPOINT_CLIENT "tcp://localhost:5556"

#endif // __GST_ZMQ_H_
