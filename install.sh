set -e
gcc $(gimptool-2.0 --cflags) -Werror -o ktx_plugin plugin.c -lktx $(gimptool-2.0 --libs)
gimptool-2.0 --install-bin ktx_plugin
