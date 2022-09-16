#!/bin/sh
set -e
gcc $(gimptool-2.0 --cflags) -g -Wall -Werror -Wno-error=deprecated-declarations -o ktx_plugin plugin.c save.c load.c -lktx $(gimptool-2.0 --libs)
gimptool-2.0 --install-bin ktx_plugin
