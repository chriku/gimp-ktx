#include <vulkan/vulkan.h>

#include <ktx.h>
#include <ktxvulkan.h>

#include <libgimp/gimp.h>

#include <libgimp/gimpui.h>
#include <string.h>

#define LOAD_PROC "file-ktx2-load"
#define SAVE_PROC "file-ktx2-save"
#define PLUG_IN_BINARY "file-ktx2"

typedef struct {
  gint super_compression;
} SaveOptions;

static const SaveOptions DEFAULT_SAVE_OPTIONS = {0};
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

typedef struct {
  gint32 drawable_ID;
  GeglBuffer* drawable;
  ktxTexture2* texture;
  const Babl* format;
} mip_map_userdata_t;

KTX_error_code mipmap_export(
    int miplevel, int face, int width, int height, int depth, ktx_uint64_t faceLodSize, void* pixels, void* userdata) {
  mip_map_userdata_t* ud = (mip_map_userdata_t*)userdata;
  GeglRectangle rect = {.x = 0, .y = 0, .width = width, .height = height};
  GeglBuffer* mbuf = gegl_buffer_new(&rect, gimp_drawable_get_format(ud->drawable_ID));
  gegl_render_op(ud->drawable,
      mbuf,
      "gegl:scale-size",
      "x",
      (double)width,
      "y",
      (double)height,
      "sampler",
      GEGL_SAMPLER_CUBIC, // TODFO: Setting
      NULL);
  gegl_buffer_get(mbuf, NULL, 1, ud->format, pixels, ktxTexture_GetRowPitch(ktxTexture(ud->texture), miplevel), GEGL_ABYSS_NONE);
  g_object_unref(mbuf);
  return KTX_SUCCESS;
}

void save_ktx(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals) {
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
    switch ((unsigned)precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8_UNORM;
      break;
    case 150: // GIMP_PRECISION_U8_NON_LINEAR
    case 175: // GIMP_PRECISION_U8_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R8_SRGB;
      break;
    case GIMP_PRECISION_U16_LINEAR:
    case 250: // GIMP_PRECISION_U16_NON_LINEAR
    case 275: // GIMP_PRECISION_U16_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
    case 350: // GIMP_PRECISION_U32_NON_LINEAR
    case 375: // GIMP_PRECISION_U32_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
    case 550: // GIMP_PRECISION_HALF_NON_LINEAR
    case 575: // GIMP_PRECISION_HALF_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
    case 650: // GIMP_PRECISION_FLOAT_NON_LINEAR
    case 675: // GIMP_PRECISION_FLOAT_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_GRAYA_IMAGE:
    switch ((unsigned)precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8_UNORM;
      break;
    case 150: // GIMP_PRECISION_U8_NON_LINEAR
    case 175: // GIMP_PRECISION_U8_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R8G8_SRGB;
      break;
    case GIMP_PRECISION_U16_LINEAR:
    case 250: // GIMP_PRECISION_U16_NON_LINEAR
    case 275: // GIMP_PRECISION_U16_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
    case 350: // GIMP_PRECISION_U32_NON_LINEAR
    case 375: // GIMP_PRECISION_U32_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
    case 550: // GIMP_PRECISION_HALF_NON_LINEAR
    case 575: // GIMP_PRECISION_HALF_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
    case 650: // GIMP_PRECISION_FLOAT_NON_LINEAR
    case 675: // GIMP_PRECISION_FLOAT_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_RGB_IMAGE:
    switch ((unsigned)precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8B8_UNORM;
      break;
    case 150: // GIMP_PRECISION_U8_NON_LINEAR
    case 175: // GIMP_PRECISION_U8_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R8G8B8_SRGB;
      break;
    case GIMP_PRECISION_U16_LINEAR:
    case 250: // GIMP_PRECISION_U16_NON_LINEAR
    case 275: // GIMP_PRECISION_U16_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16B16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
    case 350: // GIMP_PRECISION_U32_NON_LINEAR
    case 375: // GIMP_PRECISION_U32_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32B32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
    case 550: // GIMP_PRECISION_HALF_NON_LINEAR
    case 575: // GIMP_PRECISION_HALF_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16B16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
    case 650: // GIMP_PRECISION_FLOAT_NON_LINEAR
    case 675: // GIMP_PRECISION_FLOAT_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32B32_SFLOAT;
      break;
    default:
      ret_values[1].data.d_string = "Unhandled image precision";
      return;
    }
    break;
  case GIMP_RGBA_IMAGE:
    switch ((unsigned)precision) {
    case GIMP_PRECISION_U8_LINEAR:
      create_info.vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
      break;
    case 150: // GIMP_PRECISION_U8_NON_LINEAR
    case 175: // GIMP_PRECISION_U8_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
      break;
    case GIMP_PRECISION_U16_LINEAR:
    case 250: // GIMP_PRECISION_U16_NON_LINEAR
    case 275: // GIMP_PRECISION_U16_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16B16A16_UNORM;
      break;
    case GIMP_PRECISION_U32_LINEAR:
    case 350: // GIMP_PRECISION_U32_NON_LINEAR
    case 375: // GIMP_PRECISION_U32_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32B32A32_UINT;
      break;
    case GIMP_PRECISION_HALF_LINEAR:
    case 550: // GIMP_PRECISION_HALF_NON_LINEAR
    case 575: // GIMP_PRECISION_HALF_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
      break;
    case GIMP_PRECISION_FLOAT_LINEAR:
    case 650: // GIMP_PRECISION_FLOAT_NON_LINEAR
    case 675: // GIMP_PRECISION_FLOAT_PERCEPTUAL
      create_info.vkFormat = VK_FORMAT_R32G32B32A32_SFLOAT;
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
  GLuint max_dim = create_info.baseWidth > create_info.baseHeight ? create_info.baseWidth : create_info.baseHeight;
  create_info.numLevels = log2(max_dim) + 1;
  // create_info.numLevels = 1; // TODO: Setting
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
  mip_map_userdata_t mip_map_userdata;
  mip_map_userdata.drawable_ID = drawable_ID;
  mip_map_userdata.drawable = drawable;
  mip_map_userdata.texture = texture;
  mip_map_userdata.format = format;
  result = ktxTexture_IterateLevelFaces(ktxTexture(texture), &mipmap_export, &mip_map_userdata);
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
