#ifndef UTOX_MAIN_H
#define UTOX_MAIN_H

#include "branding.h"

#include <tox/tox.h>

#include <stddef.h>

#if TOX_VERSION_IS_API_COMPATIBLE(0, 2, 0)
// YAY!!
#else
  #error "Unable to compile uToxx with this Toxcore version. uToxx expects v0.2.*!"
#endif

/* Support for large files. */
#define _LARGEFILE_SOURCE
#define _FILE_OFFSET_BITS 64

enum {
    USER_STATUS_AVAILABLE,
    USER_STATUS_AWAY_IDLE,
    USER_STATUS_DO_NOT_DISTURB,
};

/**
 * Takes data and the size of data and writes it to the disk
 *
 * Returns a bool indicating whether a save is needed
 */
bool utox_data_save_tox(uint8_t *data, size_t length);

/**
 * Reads the tox data from the disk and sets size
 *
 * Returns a pointer to the tox data, the caller needs to free it
 * Returns NULL on failure
 */
uint8_t *utox_data_load_tox(size_t *size);

/**
 * Initialize uTox
 */
void utox_init(void);

#endif
