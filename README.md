# gst-zeromq

## Introduction

gst-zeromq provides [GStreamer](http://gstreamer.freedesktop.org) elements for moving data with [ZeroMQ](http://zeromq.org).

gst-zeromq is written in C for GStreamer 1.x. 

At present it has been tested with:

* Ubuntu Trusty Tahr (14.04)
* GStreamer 1.2.4 packages available on Trusty (but any 1.x should work)
* ZeroMQ 4.1.1 (but any version back to 2.2.0 should work)

Git repo at http://github.com/mjhowell/gst-zeromq

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
* move zmq context into zeromq class (one context for all instances)
* add search for zeromq package to configure.ac
* destroy zmq context on release
* nice gst error handling instead of asserts
