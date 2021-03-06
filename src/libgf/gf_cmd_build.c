/*-
 * This file is part of Grayfish project. For license details, see the file
 * 'LICENSE.md' in this package.
 */
/*!
** @file libgf/gf_cmd_build.c
** @brief Module build.
*/
#include <string.h>

#include <libgf/gf_countof.h>
#include <libgf/gf_memory.h>
#include <libgf/gf_string.h>
#include <libgf/gf_path.h>
#include <libgf/gf_cmd_config.h>
#include <libgf/gf_site.h>
#include <libgf/gf_system.h>
#include <libgf/gf_convert.h>
#include <libgf/gf_cmd_build.h>

#include "gf_local.h"

struct gf_cmd_build {
  gf_cmd_base      base;
  gf_site*         site;
  gf_convert_ctxt* ctxt;
};

/*!
** @brief
**
*/

enum {
  OPT_BUILD_OPTIONS,
};

static const gf_cmd_base_info info_ = {
  .base = {
    .name        = "build",
    .description = "Build the static website",
    .args        = NULL,
    .create      = gf_cmd_build_new,
    .free        = gf_cmd_build_free,
    .execute     = gf_cmd_build_execute,
  },
  .options = {
    /* Terminate */
    GF_OPTION_NULL,
  },
};

/*!
**
*/

static gf_status
init(gf_cmd_base* cmd) {
  gf_validate(cmd);

  _(gf_cmd_base_init(cmd));
  
  GF_CMD_BUILD_CAST(cmd)->site = NULL;
  GF_CMD_BUILD_CAST(cmd)->ctxt = NULL;

  return GF_SUCCESS;
}

static gf_status
build_read_style_file(gf_cmd_build* cmd) {
  gf_status rc = 0;
  gf_path* style_path = NULL;
  
  gf_validate(cmd);

  if (cmd->ctxt) {
    gf_convert_ctxt_free(cmd->ctxt);
    cmd->ctxt = NULL;
  }

  _(gf_convert_ctxt_new(&cmd->ctxt, GF_CONVERT_TYPE_XSLT));
  _(gf_path_get_style_path(&style_path));
  
  rc = gf_convert_ctxt_read_template(cmd->ctxt, gf_path_get_string(style_path));
  gf_path_free(style_path);
  if (rc != GF_SUCCESS) {
    gf_throw(rc);
  }
    
  return GF_SUCCESS;
}

static gf_status
prepare(gf_cmd_base* cmd) {
  gf_validate(cmd);

  _(gf_cmd_base_set_info(cmd, &info_));

  return GF_SUCCESS;
}

gf_status
gf_cmd_build_new(gf_cmd_base** cmd) {
  gf_status rc = 0;
  gf_cmd_base* tmp = NULL;

  _(gf_malloc((gf_ptr*)&tmp, sizeof(gf_cmd_build)));

  rc = init(tmp);
  if (rc != GF_SUCCESS) {
    gf_free(tmp);
    return rc;
  }
  rc = prepare(tmp);
  if (rc != GF_SUCCESS) {
    gf_cmd_build_free(tmp);
    return rc;
  }

  *cmd = tmp;
  
  return GF_SUCCESS;
}

void
gf_cmd_build_free(gf_cmd_base* cmd) {
  if (cmd) {
    /* Clear the base class */
    gf_cmd_base_clear(cmd);
    /* Clear and deallocate this class */
    if (GF_CMD_BUILD_CAST(cmd)->site) {
      gf_site_free(GF_CMD_BUILD_CAST(cmd)->site);
      GF_CMD_BUILD_CAST(cmd)->site = NULL;
    }
    if (GF_CMD_BUILD_CAST(cmd)->ctxt) {
      gf_convert_ctxt_free(GF_CMD_BUILD_CAST(cmd)->ctxt);
      GF_CMD_BUILD_CAST(cmd)->site = NULL;
    }

    gf_free(cmd);
  }
}

static gf_status
build_read_site_file(gf_cmd_build* cmd) {
  gf_validate(cmd);

  if (!gf_path_file_exists(GF_CMD_BASE_CAST(cmd)->site_path)) {
    gf_raise(GF_E_STATE, "Here is not a project directory. (%s)",
             gf_path_get_string(GF_CMD_BASE_CAST(cmd)->site_path));
  }
  _(gf_site_read_file(&cmd->site, GF_CMD_BASE_CAST(cmd)->site_path));

  return GF_SUCCESS;
}

static gf_status
build_copy_directory(gf_cmd_build* cmd, gf_site_node* node) {
  gf_validate(cmd);
  gf_validate(node);

  
  
  return GF_SUCCESS;
}

static gf_status
build_convert_file(
  gf_cmd_build* cmd, gf_path* src_path, gf_path* dst_path, const char* filename) {
  gf_status rc = 0;
  gf_path* cur_path = NULL;
  const char* src = NULL;
  const char* dst = NULL;
  
  gf_validate(cmd);
  gf_validate(!gf_path_is_empty(src_path));
  gf_validate(!gf_path_is_empty(dst_path));

  if (!cmd->ctxt) {
    gf_raise(GF_E_STATE, "Invalid converter state.");
  }

  src = gf_path_get_string(src_path);
  dst = filename;

  _(gf_path_get_current_path(&cur_path));
  
  gf_path_change_directory(dst_path);
  rc = gf_convert_file(cmd->ctxt, dst, src);
  gf_path_change_directory(cur_path);
  gf_path_free(cur_path);
  if (rc != GF_SUCCESS) {
    gf_throw(rc);
  }
    
  return GF_SUCCESS;
}

/*!
** @brief Convert the document directory.
**
*/

static gf_status
build_convert_index(gf_cmd_build* cmd, gf_site_node* node) {
  gf_status rc = 0;
  gf_path* doc_path = NULL;
  gf_path* dst_path = NULL;
  gf_path* src_path = NULL;
  gf_path* tree_path = NULL;
  
  gf_validate(cmd);
  gf_validate(node);

  _(gf_site_node_get_full_path(&doc_path, node));

  /* Source file path */
  rc = gf_path_append_string(&src_path, doc_path, "index.dbk");
  gf_path_free(doc_path);
  if (rc != GF_SUCCESS) {
    gf_throw(rc);
  }
  if (!gf_path_file_exists(src_path)) {
    gf_path_free(src_path);
    gf_raise(GF_E_STATE, "Document file does not exists. (%s)",
             gf_path_get_string(src_path));
  }
  /* Destination file path */
  rc = gf_path_append_string(&dst_path, GF_CMD_BASE_CAST(cmd)->root_path, "pub");
  if (rc != GF_SUCCESS) {
    gf_path_free(src_path);
    gf_throw(rc);
  }
  rc = gf_site_node_get_tree_path(&tree_path, node);
  if (rc != GF_SUCCESS) {
    gf_path_free(src_path);
    gf_path_free(dst_path);
    gf_throw(rc);
  }
  rc = gf_path_append(dst_path, tree_path);
  gf_path_free(tree_path);
  if (rc != GF_SUCCESS) {
    gf_path_free(src_path);
    gf_path_free(dst_path);
    gf_throw(rc);
  }
  /* Create directory */
  if (!gf_path_is_directory(dst_path)) {
    rc = gf_path_create_directory(dst_path);
    if (rc != GF_SUCCESS) {
      gf_path_free(dst_path);
      gf_path_free(src_path);
      gf_throw(rc);
    }
  }
  /* Convert file */
  rc = build_convert_file(cmd, src_path, dst_path, "index.html");
  gf_path_free(src_path);
  gf_path_free(dst_path);
  if (rc != GF_SUCCESS) {
    gf_throw(rc);
  }

  return GF_SUCCESS;
}

static gf_status
build_convert_document(gf_cmd_build* cmd, gf_site_node* node) {
  gf_validate(cmd);
  gf_validate(node);

  _(build_convert_index(cmd, node));
  _(build_copy_directory(cmd, node));
  
  return GF_SUCCESS;
}

static gf_status
build_convert_files(gf_cmd_build* cmd) {
  gf_status rc = 0;
  char* name = NULL;
  gf_site_node* node = NULL;
  
  gf_validate(cmd);
  gf_validate(cmd->site);
  gf_validate(cmd->ctxt);
  
  _(gf_site_get_root(&node, cmd->site));
  if (!node) {
    gf_raise(GF_E_STATE, "Site file is empty. (%s)",
             gf_path_get_string(GF_CMD_BASE_CAST(cmd)->site_path));
  }
  /* Site root */
  rc = build_convert_document(cmd, node);
  if (rc != GF_SUCCESS) {
    gf_warn("Failed to convert the root index.");
  }
  /* Process children */
  for (gf_site_node* n = gf_site_node_child(node); !gf_site_node_is_null(n); gf_site_node_next(n)) {
    _(gf_site_node_get_file_name(&name, n));
    if (!name) {
      gf_warn("Document name is empty.");
      continue;
    }
    if (gf_site_node_is_directory(n) && strcmp(name, "_")) {
      rc = build_convert_document(cmd, node);
      if (rc != GF_SUCCESS) {
        gf_warn("Failed to convert document. (%s)", name);
        continue;
      }
    }
  }
  
  return GF_SUCCESS;
}

static gf_status
build_process(gf_cmd_build* cmd) {
  gf_validate(cmd);

  /* Read the existing site file */
  _(build_read_site_file(cmd));
  /* Read the style file */
  _(build_read_style_file(GF_CMD_BUILD_CAST(cmd)));
  /* Convert files */
  _(build_convert_files(cmd));

  return GF_SUCCESS;
}

gf_status
gf_cmd_build_execute(gf_cmd_base* cmd) {
  gf_validate(cmd);

  gf_msg("Compiling documents ...");

  _(build_process(GF_CMD_BUILD_CAST(cmd)));
  
  gf_msg("Done.");
  
  return GF_SUCCESS;
}
