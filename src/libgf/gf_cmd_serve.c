/*-
 * This file is part of Grayfish project. For license details, see the file
 * 'LICENSE.md' in this package.
 */
/*!
** @file libgf/gf_cmd_serve.c
** @brief Module serve.
*/

#include <libgf/gf_countof.h>
#include <libgf/gf_memory.h>
#include <libgf/gf_convert.h>
#include <libgf/gf_cmd_serve.h>

#include "gf_local.h"

struct gf_cmd_serve {
  gf_cmd_base   base;
  unsigned int port;
};

/*!
** @brief
**
*/

enum {
  OPT_PORT,
};

static const gf_cmd_base_info info_ = {
  .base = {
    .name        = "serve",
    .description = "Run the local server.",
    .args        = NULL,
    .create      = gf_cmd_serve_new,
    .free        = gf_cmd_serve_free,
    .execute     = gf_cmd_serve_execute,
  },
  .options = {
    {
      .key         = OPT_PORT,
      .opt_short   = 'p',
      .opt_long    = "--port",
      .opt_count   = 1,
      .usage       = "-p <port>, --port=<port>",
      .description = "Specify the port number.",
    },
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

  GF_CMD_SERVE_CAST(cmd)->port = GF_SERVE_PORT_DEFAULT;

  return GF_SUCCESS;
}

static gf_status
prepare(gf_cmd_base* cmd) {
  gf_validate(cmd);

  _(gf_cmd_base_set_info(cmd, &info_));

  return GF_SUCCESS;
}

gf_status
gf_cmd_serve_new(gf_cmd_base** cmd) {
  gf_status rc = 0;
  gf_cmd_base* tmp = NULL;

  _(gf_malloc((gf_ptr*)&tmp, sizeof(gf_cmd_serve)));

  rc = init(tmp);
  if (rc != GF_SUCCESS) {
    gf_free(tmp);
    return rc;
  }
  rc = prepare(tmp);
  if (rc != GF_SUCCESS) {
    gf_cmd_serve_free(tmp);
    return rc;
  }

  *cmd = tmp;
  
  return GF_SUCCESS;
}

void
gf_cmd_serve_free(gf_cmd_base* cmd) {
  if (cmd) {
    /* Clear the base class */
    gf_cmd_base_clear(cmd);
    // do nothing to clear for now.
    gf_free(cmd);
  }
}

gf_status
gf_cmd_serve_execute(gf_cmd_base* cmd) {
  (void)cmd;
  return GF_SUCCESS;
}
