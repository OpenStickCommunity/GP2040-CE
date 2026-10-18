#ifndef FSCUSTOM_H_
#define FSCUSTOM_H_

#include "fs.h"

#ifdef __cplusplus
extern "C" {
#endif

int fs_open_custom(struct fs_file *file, const char *name);
void fs_close_custom(struct fs_file *file);
// Frees closed custom files that lwIP no longer sends from; call from the network loop.
void fs_release_custom(void);

#ifdef __cplusplus
}
#endif

#endif
