/*-
 * This file is part of Grayfish project. For license details, see the file
 * 'LICENSE.md' in this package.
 */
/*!
** @file libgf/gf_shell.h
** @brief Shell operations.
*/

#include <assert.h>

#include <windows.h>

#include <libgf/gf_string.h>
#include <libgf/gf_shell.h>
#include <libgf/gf_local.h>

/* -------------------------------------------------------------------------- */

/*
** The common interface for file attributes
*/

static DWORD
shell_get_file_attributes(const gf_path* path) {
  const char* s = NULL;
  DWORD ret = 0;
  
  if (gf_path_is_empty(path)) {
    return INVALID_FILE_ATTRIBUTES;
  }
  s = gf_path_get_string(path);
  assert(!gf_strnull(s));
  ret = GetFileAttributes(s);

  return ret;
}

static gf_bool
shell_has_specified_attributes(DWORD attr, DWORD flags) {
  return ((attr != INVALID_FILE_ATTRIBUTES) && (attr & flags))
    ? GF_TRUE : GF_FALSE;
}

static gf_bool
shell_has_attributes(const gf_path* path, DWORD flags) {
  DWORD attr = 0;

  if (gf_path_is_empty(path)) {
    return GF_FALSE;
  }
  attr = shell_get_file_attributes(path);

  return shell_has_specified_attributes(attr, flags);
}

/* -------------------------------------------------------------------------- */

gf_bool
gf_shell_file_exists(const gf_path* path) {
  DWORD attr = 0;
  
  if (gf_path_is_empty(path)) {
    return GF_FALSE;
  }
  attr = shell_get_file_attributes(path);
  
  return attr != INVALID_FILE_ATTRIBUTES ? GF_TRUE : GF_FALSE;
}

gf_bool
gf_shell_is_directory(const gf_path* path) {
  return shell_has_attributes(path, FILE_ATTRIBUTE_DIRECTORY);
}

gf_bool
gf_shell_is_normal_file(const gf_path* path) {
  DWORD attr = 0;
  const DWORD flags = FILE_ATTRIBUTE_DIRECTORY | FILE_ATTRIBUTE_DEVICE;

  attr = shell_get_file_attributes(path);
  if (attr == INVALID_FILE_ATTRIBUTES) {
    return GF_FALSE;
  } else if (shell_has_attributes(path, flags)) {
    return GF_FALSE;
  } else {
    return GF_TRUE;
  }

  /* not reaches here. */
  return GF_TRUE;
}

int
gf_shell_compare_files(gf_path* f1, gf_path* f2) {
  static const int ERR = -1;
  
  int ret = 0;
  FILE* fp1 = NULL;
  FILE* fp2 = NULL;

  if (gf_path_is_empty(f1)) {
    return ERR;
  }
  if (gf_path_is_empty(f2)) {
    return ERR;
  }
  fp1 = fopen(gf_path_get_string(f1), "rb");
  if (!fp1) {
    return ERR;
  }
  fp2 = fopen(gf_path_get_string(f2), "rb");
  if (!fp2) {
    fclose(fp1);
    return ERR;
  }
    
  while (GF_TRUE) {
    const int ch1 = fgetc(fp1);
    const int ch2 = fgetc(fp2);
    
    if (ch1 != ch2) {
      ret = 1;
      break;
    }
    if (ch1 == EOF) {
      assert(ch2 == EOF);
      ret = 0;
      break;
    }
  }

  fclose(fp1);
  fclose(fp2);

  return ret;
}

gf_status
gf_shell_copy_file(const gf_path* dst, const gf_path* src) {
  BOOL ret = FALSE;
  const char* s = NULL;
  const char* d = NULL;

  static const DWORD flags = 0;
  
  gf_validate(!gf_path_is_empty(dst));
  gf_validate(!gf_path_is_empty(src));

  s = gf_path_get_string(src);
  d = gf_path_get_string(dst);

  ret = CopyFileEx(s, d, NULL, NULL, FALSE, flags);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Failed to copy file (src:%s)(dst:%s)", s, d);
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_make_directory(const gf_path* path) {
  BOOL ret = FALSE;
  const char* s = NULL;

  gf_validate(!gf_path_is_empty(path));

  s = gf_path_get_string(path);
  ret = CreateDirectory(s, NULL);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Failed to create directory. (%s)", s);
  }

  return GF_SUCCESS;
}

gf_status
gf_shell_touch(const gf_path* path) {
  HANDLE hFile = INVALID_HANDLE_VALUE;
  const char* s = gf_path_get_string(path);
  
  gf_validate(!gf_path_is_empty(path));

  hFile = CreateFile(
    s,                         /* Filename             */
    GENERIC_WRITE,             /* Access mode          */
    0,                         /* Share mode           */
    NULL,                      /* Security attributes  */
    OPEN_ALWAYS,               /* Creation disposition */
    FILE_ATTRIBUTE_NORMAL,     /* Flags and attributes */
    NULL
    );
  if (hFile == INVALID_HANDLE_VALUE) {
    gf_raise(GF_E_OPEN, "Failed to touch the file (%s)", s);
  }
  CloseHandle(hFile);
  
  return GF_SUCCESS;
}

gf_status
gf_shell_remove_file(const gf_path* path) {
  BOOL ret = FALSE;
  const char* s = NULL;
  
  gf_validate(!gf_path_is_empty(path));

  s = gf_path_get_string(path);
  ret = DeleteFile(s);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Faild to remove file. (%s)", s);
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_remove_directory(const gf_path* path) {
  BOOL ret = FALSE;
  const char* s = NULL;
  
  gf_validate(!gf_path_is_empty(path));

  s = gf_path_get_string(path);
  ret = RemoveDirectory(s);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Failed to remove direcotry. (%s)", s);
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_rename(const gf_path* dst, const gf_path* src) {
  BOOL ret = FALSE;
  const char* s = NULL;
  const char* d = NULL;
  static const DWORD flags = 0;
  
  gf_validate(!gf_path_is_empty(dst));
  gf_validate(!gf_path_is_empty(src));

  s = gf_path_get_string(src);
  d = gf_path_get_string(dst);
  ret = MoveFileEx(s, d, flags);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Failed to move file. (src:%s)(dst:%s)", s, d);
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_move(const gf_path* dst, const gf_path* src) {
  return gf_shell_rename(dst, src);
}

gf_status
gf_shell_traverse_tree(
  const gf_path* path, const gf_path* trace_path, gf_shell_traverse_order order,
  gf_shell_fn fn, gf_ptr data) {
  
  HANDLE finder = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA find_data = { 0 };

  gf_path* find_path = NULL;
  
  static const char WILDCARD[]    = "*.*";
  static const char CURRENT_DIR[] = ".";
  static const char PARENT_DIR[]  = "..";
  
  gf_validate(!gf_path_is_empty(path));
  
  _(gf_path_append_string(&find_path, path, WILDCARD));
  assert(!gf_path_is_empty(find_path));

  /* Search files */
  finder = FindFirstFile(gf_path_get_string(find_path), &find_data);
  if (finder == INVALID_HANDLE_VALUE) {
    gf_path_free(find_path);
    gf_raise(GF_E_SHELL, "Failed to find file. (%s)", gf_path_get_string(path));
  }
  do {
    gf_status rc = 0;
    gf_path* child = NULL;
    gf_path* trace = NULL;

    const char* name = find_data.cFileName;        // alias
    const DWORD attr = find_data.dwFileAttributes; // alias
    
    if (!strcmp(name, CURRENT_DIR) || !strcmp(name, PARENT_DIR)) {
      continue;
    }
    /* Child path */
    rc = gf_path_append_string(&child, path, name);
    if (rc != GF_SUCCESS) {
      gf_path_free(find_path);
      FindClose(finder);
      gf_throw(rc);
    }
    /* Trace path */
    if (!gf_path_is_empty(trace_path)) {
      rc = gf_path_append_string(&trace, trace_path, name);
      if (rc != GF_SUCCESS) {
        gf_path_free(find_path);
        FindClose(finder);
        gf_throw(rc);
      }
    } else {
      rc = gf_path_new(&trace, name);
      if (rc != GF_SUCCESS) {
        gf_path_free(find_path);
        FindClose(finder);
        gf_throw(rc);
      }
    }
    /* Preorder processing */
    if (order == GF_SHELL_TRAVERSE_PREORDER) {
      if (fn) {
        rc = fn(child, trace, &find_data, data);
        if (rc != GF_SUCCESS) {
          gf_path_free(trace);
          gf_path_free(child);
          gf_path_free(find_path);
          FindClose(finder);
          gf_throw(rc);
        }
      }
    }
    /* Dig into the sub-directory */
    if (shell_has_specified_attributes(attr, FILE_ATTRIBUTE_DIRECTORY)) {
      rc = gf_shell_traverse_tree(child, trace, order, fn, data);
      if (rc != GF_SUCCESS) {
        gf_path_free(trace);
        gf_path_free(child);
        gf_path_free(find_path);
        FindClose(finder);
        gf_throw(rc);
      }
    }
    /* Postorder processing */
    if (order == GF_SHELL_TRAVERSE_POSTORDER) {
      if (fn) {
        rc = fn(child, trace, &find_data, data);
        if (rc != GF_SUCCESS) {
          gf_path_free(trace);
          gf_path_free(child);
          gf_path_free(find_path);
          FindClose(finder);
          gf_throw(rc);
        }
      }
    }
    gf_path_free(trace);
    gf_path_free(child);
  } while (FindNextFile(finder, &find_data));

  gf_path_free(find_path);
  FindClose(finder);

  return GF_SUCCESS;
}

static gf_status
shell_copy_callback(
  const gf_path* path, const gf_path* trace, gf_ptr find_data, gf_ptr data) {
  gf_status rc = 0;
  const WIN32_FIND_DATA* fd = (WIN32_FIND_DATA*)find_data;
  gf_path* dst = NULL;

  gf_validate(!gf_path_is_empty(path));
  gf_validate(!gf_path_is_empty(trace));
  gf_validate(data);
  gf_validate(find_data);

  /* alias */
  DWORD attr = fd->dwFileAttributes;

  _(gf_path_append_string(&dst, (gf_path*)data, gf_path_get_string(trace)));
  assert(!gf_path_is_empty(dst));

  if (shell_has_specified_attributes(attr, FILE_ATTRIBUTE_DIRECTORY)) {
    if (!gf_shell_file_exists(dst)) {
      rc = gf_shell_make_directory(dst);
      gf_path_free(dst);
      if (rc != GF_SUCCESS) {
        gf_throw(rc);
      }
    } else {
      /* Do nothing */
    }
  } else {
    rc = gf_shell_copy_file(dst, path);
    gf_path_free(dst);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_copy_tree(const gf_path* dst, const gf_path* src) {
  gf_status rc = 0;
  gf_path* path = NULL;
  
  gf_validate(!gf_path_is_empty(dst));
  gf_validate(!gf_path_is_empty(src));

  if (gf_shell_is_directory(src)) {
    rc = gf_path_clone(&path, dst);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
    rc = gf_shell_make_directory(path);
    if (rc != GF_SUCCESS) {
      gf_path_free(path);
      gf_throw(rc);
    }
    rc = gf_shell_traverse_tree(
      src, NULL, GF_SHELL_TRAVERSE_PREORDER, shell_copy_callback, path);
    gf_path_free(path);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
  } else {
    _(gf_shell_copy_file(dst, src));
  }

  return GF_SUCCESS;
}

static gf_status
shell_remove_callback(
  const gf_path* path, const gf_path* trace, gf_ptr find_data, gf_ptr data) {
  const WIN32_FIND_DATA* fd = (WIN32_FIND_DATA*)find_data;
  DWORD attr = 0;

  (void)data;
  (void)trace;
  
  gf_validate(!gf_path_is_empty(path));

  /* alias */
  attr = fd->dwFileAttributes;

  if (shell_has_specified_attributes(attr, FILE_ATTRIBUTE_DIRECTORY)) {
    _(gf_shell_remove_directory(path));
  } else {
    _(gf_shell_remove_file(path));
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_remove_tree(const gf_path* path) {
  gf_status rc = 0;
  
  gf_validate(!gf_path_is_empty(path));

  if (gf_shell_is_directory(path)) {
    rc = gf_shell_traverse_tree(
      path, NULL, GF_SHELL_TRAVERSE_POSTORDER, shell_remove_callback, NULL);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
    rc = gf_shell_remove_directory(path);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
  } else {
    rc = gf_shell_remove_file(path);
    if (rc != GF_SUCCESS) {
      gf_throw(rc);
    }
  }
  
  return GF_SUCCESS;
}

gf_status
gf_shell_change_directory(const gf_path* path) {
  BOOL ret = FALSE;
  const char* s = NULL;
  
  gf_validate(!gf_path_is_empty(path));
  s = gf_path_get_string(path);
  ret = SetCurrentDirectory(s);
  if (!ret) {
    gf_raise(GF_E_SHELL, "Failed to change directory. (%s)", s);
  }

  return GF_SUCCESS;
}
