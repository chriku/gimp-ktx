#include <vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <libgimp/gimp.h>

#include <libgimp/gimpui.h>
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
  KTX_error_code result = ktxTexture_CreateFromNamedFile(filename, KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &texture);
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

typedef struct {
  gint super_compression;
} SaveOptions;

static const SaveOptions DEFAULT_SAVE_OPTIONS = {0, 0};
static gboolean show_options(SaveOptions* save_options) {

  GtkWidget* dialog = gimp_export_dialog_new("KTX2", PLUG_IN_BINARY, NULL);

  g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  gtk_window_set_resizable(GTK_WINDOW(dialog), FALSE);

  GtkWidget* vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 12);
  gtk_container_set_border_width(GTK_CONTAINER(vbox), 12);
  gtk_box_pack_start(GTK_BOX(gimp_export_dialog_get_content_area(dialog)), vbox, TRUE, TRUE, 0);
  gtk_widget_show(vbox);

  GtkWidget* quality_table = gtk_table_new(2, 3, FALSE);
  gtk_table_set_col_spacings(GTK_TABLE(quality_table), 6);
  gtk_table_set_row_spacings(GTK_TABLE(quality_table), 6);
  gtk_box_pack_start(GTK_BOX(vbox), quality_table, FALSE, FALSE, 0);
  gtk_widget_show(quality_table);

  GtkObject* super_compression_combo_box = gimp_scale_entry_new(GTK_TABLE(quality_table),
      0,
      0,
      "super_compression:",
      125,
      0,
      save_options->super_compression,
      0.0,
      255.0,
      1.0,
      10.0,
      0,
      TRUE,
      0.0,
      0.0,
      "Super Compression: 0 Uncompressed",
      "?");

  gtk_widget_show(dialog);

  gboolean dialog_result = gimp_dialog_run(GIMP_DIALOG(dialog)) == GTK_RESPONSE_OK;

  save_options->super_compression = (gint)gtk_adjustment_get_value(GTK_ADJUSTMENT(super_compression_combo_box));

  gtk_widget_destroy(dialog);

  return dialog_result;
}

static void save(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
  GimpParam* ret_values = g_new(GimpParam, 2);
  *nreturn_vals = 2;
  *return_vals = ret_values;
  {
    ret_values[0].type = GIMP_PDB_STATUS;
    ret_values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;
    ret_values[1].type = GIMP_PDB_STRING;
    ret_values[1].data.d_string = "Error while saveing";
  }
  GimpRunMode run_mode = (GimpRunMode)param[0].data.d_int32;
  gint32 image_ID = param[1].data.d_int32;
  gint32 drawable_ID = param[2].data.d_int32;
  gchar* filename = param[3].data.d_string;
  gimp_ui_init(PLUG_IN_BINARY, FALSE);

  GimpExportCapabilities capabilities = GIMP_EXPORT_CAN_HANDLE_RGB | GIMP_EXPORT_CAN_HANDLE_GRAY | GIMP_EXPORT_CAN_HANDLE_ALPHA;
  GimpExportReturn export_return = gimp_export_image(&image_ID, &drawable_ID, "KTX2", capabilities);
  if (export_return == GIMP_EXPORT_CANCEL) {
    ret_values[0].data.d_status = GIMP_PDB_CANCEL;
    return;
  }
  GimpImageType image_type = gimp_drawable_type(drawable_ID);
  GimpPrecision precision = gimp_image_get_precision(image_ID);

  ktxTextureCreateInfo create_info;
  const Babl* format = gimp_drawable_get_format(drawable_ID);
  switch (image_type) {
  case GIMP_GRAY_IMAGE:
    switch (precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8_UNORM;
      break;
    case GIMP_PRECISION_U16_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_GRAYA_IMAGE:
    switch (precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8_UNORM;
      break;
    case GIMP_PRECISION_U16_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32G32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32G32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_RGB_IMAGE:
    switch (precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8B8_UNORM;
      break;
    case GIMP_PRECISION_U16_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16B16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32G32B32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_RGBA_IMAGE:
    switch (precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    case GIMP_PRECISION_U16_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32G32B32A32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
      create_info.vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
      create_info.vkFormat = VK_FORMAT_R32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  default:
    ret_values[1].data.d_string = "Unhandled image format";
    return;
  }
  SaveOptions save_options = DEFAULT_SAVE_OPTIONS;
  switch (run_mode) {
  case GIMP_RUN_INTERACTIVE:
    gimp_get_data(SAVE_PROC, &save_options);
    if (show_options(&save_options)) {
      gimp_set_data(SAVE_PROC, &save_options, sizeof(SaveOptions));
    } else {
      ret_values[0].data.d_status = GIMP_PDB_CANCEL;
      return;
    }
    break;

  case GIMP_RUN_NONINTERACTIVE:
    if (nparams == 6) {
      save_options.super_compression = param[5].data.d_int32;

      if ((save_options.super_compression < 0) || (save_options.super_compression > 255)) {
        ret_values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
        return;
      }
    } else {
      ret_values[0].data.d_status = GIMP_PDB_CALLING_ERROR;
      return;
    }
    break;

  case GIMP_RUN_WITH_LAST_VALS:
    gimp_get_data(SAVE_PROC, &save_options);
    break;
  }

  GeglBuffer* drawable = gimp_drawable_get_buffer(drawable_ID);
  create_info.baseWidth = gegl_buffer_get_width(drawable);
  create_info.baseHeight = gegl_buffer_get_height(drawable);
  create_info.baseDepth = 1;
  create_info.numDimensions = 2;
  create_info.numLevels = 1;
  create_info.numLayers = 1;
  create_info.numFaces = 1;
  create_info.isArray = KTX_FALSE;
  create_info.generateMipmaps = KTX_FALSE;
  ktxTexture2* texture;
  KTX_error_code result = ktxTexture2_Create(&create_info, KTX_TEXTURE_CREATE_ALLOC_STORAGE, &texture);
  if (result != KTX_SUCCESS) {
    ret_values[1].data.d_string = (char*)ktxErrorString(result);
    g_object_unref(drawable);
    return;
  }
  uint8_t* dest = malloc(create_info.baseWidth * create_info.baseHeight * babl_format_get_bytes_per_pixel(format));
  gegl_buffer_get(
      drawable, gegl_buffer_get_extent(drawable), 1, format, dest, ktxTexture_GetRowPitch(ktxTexture(texture), 0), GEGL_ABYSS_NONE);
  result = ktxTexture_SetImageFromMemory(
      ktxTexture(texture), 0, 0, 0, dest, create_info.baseWidth * create_info.baseHeight * babl_format_get_bytes_per_pixel(format));
  free(dest);
  if (result != KTX_SUCCESS) {
    ret_values[1].data.d_string = (char*)ktxErrorString(result);
    ktxTexture_Destroy(ktxTexture(texture));
    g_object_unref(drawable);
    return;
  }
  g_object_unref(drawable);
  if (save_options.super_compression) {
    result = ktxTexture2_CompressBasis(texture, save_options.super_compression);
    if (result != KTX_SUCCESS) {
      ret_values[1].data.d_string = (char*)ktxErrorString(result);
      ktxTexture_Destroy(ktxTexture(texture));
      return;
    }
  }
  ktxTexture_WriteToNamedFile(ktxTexture(texture), filename);
  ktxTexture_Destroy(ktxTexture(texture));
  *nreturn_vals = 1;
  ret_values[0].data.d_status = GIMP_PDB_SUCCESS;
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

  static const GimpParamDef save_args[] = {{GIMP_PDB_INT32, "run-mode", "Interactive, non-interactive"},
      {GIMP_PDB_IMAGE, "image", "Input image"},
      {GIMP_PDB_DRAWABLE, "drawable", "Drawable to save"},
      {GIMP_PDB_STRING, "filename", "The name of the file to save the image in"},
      {GIMP_PDB_STRING, "raw-filename", "The name entered"},
      {GIMP_PDB_INT32, "super-compression", "?"}};

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