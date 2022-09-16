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
void load_ktx(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);
void save_ktx(gint nparams, const GimpParam* param, gint* nreturn_vals, GimpParam** return_vals);

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
  gegl_init(NULL, NULL);
  if (strcmp(name, LOAD_PROC) == 0)
    load_ktx(nparams, param, nreturn_vals, return_vals);
  else if (strcmp(name, SAVE_PROC) == 0)
    save_ktx(nparams, param, nreturn_vals, return_vals);
}

G_END_DECLS