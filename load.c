#include <vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <libgimp/gimp.h>

#include <libgimp/gimpui.h>
#include <string.h>

#define LOAD_PROC "file-ktx2-load"
#define SAVE_PROC "file-ktx2-save"
#define PLUG_IN_BINARY "file-ktx2"

void load_ktx(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
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
  KTX_error_code result = ktxTexture_CreateFromNamedFile(filename, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
  if (result != KTX_SUCCESS) {
    ret_values[1].data.d_string = (char*)ktxErrorString(result);
    return;
  }
  if (texture->baseDepth != 1) {
    ret_values[1].data.d_string = "Unsupported base level depth";
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

  for (size_t face_i = 0; face_i < texture->numFaces; face_i++) {
    char* layer_name = malloc(128);
    if (texture->isCubemap) {
      snprintf(layer_name, 128, "Face (%s %s)", (face_i % 2) ? "negative" : "positive", face_i < 2 ? "x" : face_i < 4 ? "y" : "z");
    } else {
      snprintf(layer_name, 128, "Layer %llu", (unsigned long long)face_i);
    }
    gint32 layer_ID = gimp_layer_new(image_ID, layer_name, texture->baseWidth, texture->baseHeight, image_type, 100.0, GIMP_NORMAL_MODE);
    free(layer_name);
    GeglBuffer* drawable = gimp_drawable_get_buffer(layer_ID);

    ktx_size_t offset;
    ktxTexture_GetImageOffset(texture, 0, 0, face_i, &offset);
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
    gimp_image_insert_layer(image_ID, layer_ID, 0, face_i);
  }

  ret_values[0].type = GIMP_PDB_STATUS;
  ret_values[0].data.d_status = GIMP_PDB_SUCCESS;
  ret_values[1].type = GIMP_PDB_IMAGE;
  ret_values[1].data.d_image = image_ID;

  return;
}
