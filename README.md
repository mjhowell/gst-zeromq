# gst-zeromq

## Introduction

gst-zeromq provides [GStreamer](http://gstreamer.freedesktop.org) elements for moving data with [ZeroMQ](http://zeromq.org).

Specifically, it supports ZeroMQ PUB/SUB sockets via a sink (zmqsink) which provides a PUB endpoint, and a source (zmqsrc) that uses a SUB socket to connect to a PUB.

Other ZeroMQ topologies may be implemented in the future.

gst-zeromq is written in C for GStreamer 1.x, using the usual GStreamer GLib C idiom.

It has been developed and tested with:

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

### With stdin/stdout

In one terminal, start a "server":

    $ gst-launch-1.0 fdsrc fd=0 ! zmqsink

In another terminal, start a "client":

    $ gst-launch-1.0 zmqsrc ! fdsink fd=1

When you type in a line of text at the server and hit Enter, the text will show at the client.

Stop each side with Ctrl-C.

### With Video

In one terminal, start a "server":

    $  gst-launch-1.0 videotestsrc ! video/x-raw, format=I420, width=640, height=480, framerate=30/1 ! zmqsink

In another terminal, start a "client":

    $ gst-launch-1.0 zmqsrc ! video/x-raw, format=I420, width=640, height=480, framerate=30/1 ! autovideosink

Note caps are specified here because this is two separate pipelines with no cap negotiation happening between them.

### ZeroMQ PUB/SUB in action

With ZeroMQ PUB/SUB, multiple SUBs can connect to one PUB. PUBs and SUBs can come and go at will, and reconnect automatically.

Let's start a server (which provides a PUB endpoint):

    $  gst-launch-1.0 videotestsrc ! video/x-raw, format=I420, width=640, height=480, framerate=30/1 ! zmqsink

Then in two separate terminals, start two clients (which SUB to the server's PUB):

    $ gst-launch-1.0 zmqsrc ! video/x-raw, format=I420, width=640, height=480, framerate=30/1 ! autovideosink

and

    $ gst-launch-1.0 zmqsrc ! video/x-raw, format=I420, width=640, height=480, framerate=30/1 ! autovideosink

Stop and restart a client. It will start to play again.

Stop and restart the server. Both clients will stop playing, then reconnect and start playing again.

You can start and stop the server and clients in any order, and they will connect and play. You can connect any number of clients (SUBs) to the server's PUB, within the limitations of your system.

Servers and clients can be on different systems as long as the PUB endpoint is reachable by clients over the network, just change the endpoint from the default. Multiple streams can be served on the same system by changing the endpoint's port number or protocol type. See the [ZeroMQ](http://zeromq.org) docs for more information about endpoints and protocols.

## License

This project uses the GNU LGPL. See COPYING and COPYING.LIB.

## TODO
* move zmq context into class (one context for all instances)?
* destroy zmq context on release
* fix src getting stuck in create() after stopping pipeline.
* zero copy possible, rather than memcpy() between mapped GstBuffer and zmq_msg_t?
* correct reset behavior... works OK in gst-launch cmdline but not in apps.
