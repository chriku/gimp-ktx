#include <vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <libgimp/gimp.h>
#include <string.h>

#define LOAD_PROC "file-ktx2-load"
#define SAVE_PROC "file-ktx2-save"
#define PLUG_IN_BINARY "file-ktx2"

static void query();
static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);
static void load(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
  gchar* filename = param[1].data.d_string;

  GimpParam* ret_values = g_new(GimpParam, 2);
  *nreturn_vals = 2;
  *return_vals = ret_values;
  {
    ret_values[0].type = GIMP_PDB_STATUS;
    ret_values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
    ret_values[1].type = GIMP_PDB_STRING;
    ret_values[1].data.d_string = "ErroY loading file";
  }

  ktxTexture* texture;
  KTX_error_code result = ktxTexture_CreateFromNamedFile(gimp_filename_to_utf8(filename), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
  if (result != KTX_SUCCESS) {
    ret_values[1].data.d_string = (char*)ktxErrorString(result);
    return;
  }
  if (texture->baseDepth != 1) {
    ret_values[1].data.d_string = "Invalid base level depth";
    ktxTexture_Destroy(texture);
    return;
  }

  if (ktxTexture_NeedsTranscoding(texture) && (texture->classId == ktxTexture2_c)) {
    result = ktxTexture2_TranscodeBasis((ktxTexture2*)texture, KTX_TTF_RGBA32, 0);
    if (result != KTX_SUCCESS) {
      ret_values[1].data.d_string = (char*)ktxErrorString(result);
      ktxTexture_Destroy(texture);
      return;
    }
  }

  if (texture->isCompressed) {
    ret_values[1].data.d_string = "Cannot handle compressed texture";
    ktxTexture_Destroy(texture);
    return;
  }

  const Babl* format;
  GimpImageBaseType base_type;
  GimpImageType image_type;
  if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_UNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_USCALED) ||
      (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_UINT)) {
    format = babl_format("Y u8");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_SNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_SINT)) {
    format = babl_format("Y s8");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8_SRGB) {
    format = babl_format("Y' u8");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_UNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_UINT)) {
    format = babl_format("YA u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_SNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_SINT)) {
    format = babl_format("YA s8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8_SRGB) {
    format = babl_format("Y'A u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_UNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_UINT)) {
    format = babl_format("RGB u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_SNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_SINT)) {
    format = babl_format("RGB s8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8_SRGB) {
    format = babl_format("R'G'B'A u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_UNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_UINT)) {
    format = babl_format("RGBA u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_SNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_SINT)) {
    format = babl_format("RGBA s8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R8G8B8A8_SRGB) {
    format = babl_format("R'G'B'A u8");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_UNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_UINT)) {
    format = babl_format("Y u16");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_SNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_SINT)) {
    format = babl_format("Y s16");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16_SFLOAT) {
    format = babl_format("Y' half");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_UNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_UINT)) {
    format = babl_format("YA u16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_SNORM) || (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_SINT)) {
    format = babl_format("YA s16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16_SFLOAT) {
    format = babl_format("Y'A half");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_UNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_UINT)) {
    format = babl_format("RGB u16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_SNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_SINT)) {
    format = babl_format("RGB s16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16_SFLOAT) {
    format = babl_format("R'G'B' half");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_UNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_USCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_UINT)) {
    format = babl_format("RGBA u16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_SNORM) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_SSCALED) ||
             (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_SINT)) {
    format = babl_format("RGBA s16");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R16G16B16A16_SFLOAT) {
    format = babl_format("R'G'B'A half");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32_UINT)) {
    format = babl_format("Y u32");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32_SINT)) {
    format = babl_format("Y s32");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32_SFLOAT) {
    format = babl_format("Y' half");
    base_type = GIMP_GRAY;
    image_type = GIMP_GRAY_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32_UINT)) {
    format = babl_format("YA u32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32_SINT)) {
    format = babl_format("YA s32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32_SFLOAT) {
    format = babl_format("Y'A float");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32_UINT)) {
    format = babl_format("RGB u32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32_SINT)) {
    format = babl_format("RGB s32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32_SFLOAT) {
    format = babl_format("R'G'B' float");
    base_type = GIMP_RGB;
    image_type = GIMP_RGB_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32A32_UINT)) {
    format = babl_format("RGBA u32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if ((ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32A32_SINT)) {
    format = babl_format("RGBA s32");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else if (ktxTexture_GetVkFormat(texture) == VK_FORMAT_R32G32B32A32_SFLOAT) {
    format = babl_format("R'G'B'A float");
    base_type = GIMP_RGB;
    image_type = GIMP_RGBA_IMAGE;
  } else {
    char* message = malloc(128); // Memory leak, as I am stupid
    snprintf(message, 128, "Unknown format %llu", (unsigned long long)ktxTexture_GetVkFormat(texture));
    ret_values[1].data.d_string = message;
    ktxTexture_Destroy(texture);
    return;
  }

  gint32 image_ID = gimp_image_new_with_precision(texture->baseWidth, texture->baseHeight, base_type, GIMP_PRECISION_FLOAT_LINEAR);
  gimp_image_set_filename(image_ID, filename);
  gint32 layer_ID = gimp_layer_new(image_ID, "Image", texture->baseWidth, texture->baseHeight, image_type, 100.0, GIMP_NORMAL_MODE);
  GeglBuffer* drawable = gimp_drawable_get_buffer(layer_ID);

  ktx_size_t offset;
  ktxTexture_GetImageOffset(texture, 0, 0, 0, &offset);
  if (result != KTX_SUCCESS) {
    ret_values[1].data.d_string = (char*)ktxErrorString(result);
    ktxTexture_Destroy(texture);
    g_object_unref(drawable);
    return;
  }
  GeglRectangle rect = {.x = 0, .y = 0, .width = texture->baseWidth, .height = texture->baseHeight};
  gegl_buffer_set(drawable, &rect, 0, format, ktxTexture_GetData(texture) + offset, ktxTexture_GetRowPitch(texture, 0));

  gegl_buffer_flush(drawable);
  g_object_unref(drawable);

  gimp_drawable_update(layer_ID, 0, 0, texture->baseWidth, texture->baseHeight);
  gimp_image_insert_layer(image_ID, layer_ID, 0, 0);

  ret_values[0].type = GIMP_PDB_STATUS;
  ret_values[0].data.d_status = GIMP_PDB_SUCCESS;
  ret_values[1].type = GIMP_PDB_IMAGE;
  ret_values[1].data.d_image = image_ID;

  return;
}

static void save(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
  GimpParam* ret_values = g_new(GimpParam, 2);
  *nreturn_vals = 2;
  *return_vals = ret_values;
  {
    ret_values[0].type = GIMP_PDB_STATUS;
    ret_values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
    ret_values[1].type = GIMP_PDB_STRING;
    ret_values[1].data.d_string = "Not yet implemented";
    return;
  }
}

const GimpPlugInInfo PLUG_IN_INFO = {
    NULL,
    NULL,
    (GimpQueryProc)query,
    (GimpRunProc)run,
};

G_BEGIN_DECLS

MAIN()

static void query() {
  static const GimpParamDef load_args[] = {{GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
      {GIMP_PDB_STRING, "filename", "The name of the file to load"},
      {GIMP_PDB_STRING, "raw-filename", "The name entered"}};

  static const GimpParamDef load_return_vals[] = {{GIMP_PDB_IMAGE, "image", "Output image"}};

  static const GimpParamDef save_args[] = {
      {GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
      {GIMP_PDB_IMAGE, "image", "Input image"},
      {GIMP_PDB_DRAWABLE, "drawable", "Drawable to save"},
      {GIMP_PDB_STRING, "filename", "The name of the file to save the image in"},
      {GIMP_PDB_STRING, "raw-filename", "The name entered"},
      {GIMP_PDB_INT32, "quality", "Quality of saved image (0 <= quality <= 100, 100 = lossless)"},
      {GIMP_PDB_INT32, "alpha-quality", "Quality of alpha channel (0 <= quality <= 100, 100 = lossless)"},
      {GIMP_PDB_INT32, "overlap", "Overlap level (0 = auto, 1 = none, 2 = one level, 3 = two level)"},
      {GIMP_PDB_INT32, "subsampling", "Chroma subsampling (0 = Y-only, 1 = 4:2:0, 2 = 4:2:2, 3 = 4:4:4)"},
      {GIMP_PDB_INT32, "tiling", "Tiling (0 = none, 1 = 256 x 256, 2 = 512 x 512, 3 = 1024 x 1024)"},
  };

  gimp_install_procedure(LOAD_PROC,
      "Loads KTX/KTX2 images",
      "Loads KTX/KTX2 image files.",
      "Christian Kurz",
      "Christian Kurz",
      "2022",
      "KTX/KTX2 image",
      NULL,
      GIMP_PLUGIN,
      G_N_ELEMENTS(load_args),
      G_N_ELEMENTS(load_return_vals),
      load_args,
      load_return_vals);

  gimp_register_file_handler_mime(LOAD_PROC, "image/ktx,image/ktx2");
  gimp_register_magic_load_handler(LOAD_PROC,
      "ktx,ktx2",
      "",
      "0,string,\xAB\x4B\x54\x58\x20\x31\x31\xBB\x0D\x0A\x1A\x0A,0,string,\xAB\x4B\x54\x58\x20\x32\x30\xBB\x0D\x0A\x1A\x0A");

  gimp_install_procedure(SAVE_PROC,
      "Saves KTX2 images",
      "Saves KTX2 image files.",
      "Christian Kurz",
      "Christian Kurz",
      "2022",
      "KTX2 image",
      "RGB*, GRAY",
      GIMP_PLUGIN,
      G_N_ELEMENTS(save_args),
      0,
      save_args,
      0);

  gimp_register_save_handler(SAVE_PROC, "ktx2", "");
  gimp_register_file_handler_mime(SAVE_PROC, "image/ktx2");
}

static void run(const gchar* name, gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
  babl_init();
  if (strcmp(name, LOAD_PROC) == 0)
    load(nparams, param, nreturn_vals, return_vals);
  else if (strcmp(name, SAVE_PROC) == 0)
    save(nparams, param, nreturn_vals, return_vals);
}

G_END_DECLS