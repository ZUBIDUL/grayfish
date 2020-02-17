/*-
 * This file is part of Grayfish project. For license details, see the file
 * 'LICENSE.md' in this package.
 */
/*!
** @file libgf/gf_clean.h
** @brief Initialize the system environment (private command).
*/
#ifndef LIBGF_GF_CLEAN_H
#define LIBGF_GF_CLEAN_H

#pragma once

#include <libgf/config.h>

#include <libgf/gf_datatype.h>
#include <libgf/gf_error.h>
#include <libgf/gf_command.h>

typedef struct gf_clean gf_clean;

#define GF_CLEAN_CAST(cmd) ((gf_clean*)(cmd))

/*!
** @brief Create a new clean command object.
**
** @param [out] cmd The pointer to the new clean command object
*/

extern gf_status gf_clean_new(gf_command** cmd);
extern void gf_clean_free(gf_command* cmd);

/*!
** @brief Execute the clean process.
**
** @param [in] cmd Command object
*/

extern gf_status gf_clean_execute(gf_command* cmd);

#endif  /* LIBGF_GF_CLEAN_H */