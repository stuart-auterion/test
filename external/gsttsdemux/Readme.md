# About tsdemux

This is a custom version of gstreamer the mpegtsbase and associated Gstreamer plugins (tsdemux & tsparse). These files are pulled from gst-plugins-bad and compiled into our project with modifications, replacing the default gstreamer plugins.

tsdemux is changed to that both asynchronous and synchronous KLV (stream_type=0x06 and stream_type=0x15, respectively) will be parsed. Base gstreamer will only handle asynchronous KLV. This is fixed in [a Gstreamer patch here](https://gitlab.freedesktop.org/gstreamer/gstreamer/-/merge_requests/1312), but has yet to make it into an official release, and regardless will likely not be backported to Ubuntu 20.24 (or 22.04, for that matter).
