# gimp-ktx

Import [KTX](https://www.khronos.org/registry/KTX/specs/1.0/ktxspec_v1.html) and [KTX2](https://github.khronos.org/KTX-Specification/) files and export [KTX2](https://github.khronos.org/KTX-Specification/) seamlessly into and from gimp

__WARNING: currently only lossy compression on export. Repeatedly loading and saving__ (with compression slider >0) __will degrade your image irrevertably!__ Only use compression for the final export

## Building

Currently a useful build system is missing.
If vulkan (for `VkFormat`) and libktx are installed globally then `install.sh` can be used for compilation.

A PR with a build system would be very welcome.

## TODO

- [x] Export (WARNING: currently only lossy. Repeatedly loading and saving will degrade an image considerably)
- [ ] More export compression control
- [X] Generate MipMaps
- [x] CubeMap
- [ ] Export CubeMap
- [ ] Multiple layers/channel/...
- [ ] Build system
