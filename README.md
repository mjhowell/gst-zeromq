# gst-zeromq

## Introduction

gst-zeromq provides [GStreamer](http://gstreamer.freedesktop.org) elements for moving data with [ZeroMQ](http://zeromq.org).

gst-zeromq is written in C for GStreamer 1.x.

At present it has been tested with:

* Ubuntu Trusty Tahr (14.04)
* GStreamer 1.2.4 packages available on Trusty (but any 1.x should work)
* ZeroMQ 4.1.1 (but any version back to 2.2.0 should work)

Git repo at http://github.com/mjhowell/gst-zeromq

## Build It

On Ubuntu, you'll need at least build-essential and libgstreamer1.0-dev installed. Then, to build:

    $ ./autogen.sh

    $ make

The libs will be built in src/zeromq/.libs. To test them in place without installing, run the gst-zeromq-vars script:

    $ . gst-zeromq-vars.sh

Now see if GStreamer can find the libs and catalog the elements:

    $ gst-inspect-1.0 zmqsrc

You should see something like this:

```
Factory Details:
Rank                     none (0)
Long-name                ZeroMQ source
Klass                    Source/Network
Description              Receive data on ZeroMQ SUB socket

...

do-timestamp        : Apply current stream time to buffers
                      flags: readable, writable
                      Boolean. Default: false
endpoint            : ZeroMQ endpoint from which to receive buffers
                      flags: readable, writable
                      String. Default: "tcp://localhost:5556"
bind                : If true, bind to the endpoint (be the "server")
                      flags: readable, writable
                      Boolean. Default: false

```

## Try It

In one terminal, start a "server":

    $ gst-launch-1.0 fdsrc fd=0 ! zmqsink

In another terminal, start a "client":

    $ gst-launch-1.0 zmqsrc ! fdsink fd=1

When you type in a line of text at the server, it will show at the client.

Sure, you can do the same thing with, say, the tcpserversink and tcpclientsrc elements in gst-plugins-base. We're just getting started.

## License

This project uses the GNU LGPL. See COPYING and COPYING.LIB.

## TODO
* move zmq context into class (one context for all instances)?
* destroy zmq context on release
* fix src getting stuck in create() after stopping pipeline.
* zero copy possible, rather than memcpy() between mapped GstBuffer and zmq_msg_t?
