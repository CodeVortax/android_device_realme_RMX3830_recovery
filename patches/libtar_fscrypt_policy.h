/*
 * Shadow of system/vold/fscrypt_policy.h for TWRP android-14.1 builds.
 *
 * The workflow copies this file to bootable/recovery/libtar/fscrypt_policy.h,
 * so libtar's `#include "fscrypt_policy.h"` resolves here instead of to
 * system/vold/fscrypt_policy.h.
 *
 * Why: TeamWin's android-14.1 libtar (97b849918 "libtar: dynamically choose
 * fscrypt policy") uses a (union) fscrypt_policy type plus the helpers
 * get_policy(), get_policy_size(), get_policy_descriptor(),
 * fscrypt_policy_size() and get_policy_content(). TeamWin's android-14.1
 * system/vold dropped the union and every one of those helpers, and bionic's
 * <linux/fscrypt.h> additionally does `#define fscrypt_policy
 * fscrypt_policy_v1`, which breaks `fscrypt_policy *fep;` with
 * "must use 'struct' tag to refer to type 'fscrypt_policy_v1'".
 *
 * The non-inline functions below are still resolved from libvold, whose
 * definitions are extern "C" and take a pointer, so the pointer ABI is
 * identical (the v1/v2 structs both live at offset 0 of the union).
 */

#ifndef TWRP_LIBTAR_FSCRYPT_POLICY_COMPAT_H
#define TWRP_LIBTAR_FSCRYPT_POLICY_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/cdefs.h>
#include <linux/fs.h>
#include <linux/fscrypt.h>

/* bionic aliases this name to fscrypt_policy_v1; we want the union. */
#ifdef fscrypt_policy
#undef fscrypt_policy
#endif

#ifndef FS_KEY_DESCRIPTOR_SIZE
#define FS_KEY_DESCRIPTOR_SIZE FSCRYPT_KEY_DESCRIPTOR_SIZE
#endif
#ifndef FSCRYPT_KEY_DESCRIPTOR_SIZE
#define FSCRYPT_KEY_DESCRIPTOR_SIZE 8
#endif
#ifndef FSCRYPT_KEY_IDENTIFIER_SIZE
#define FSCRYPT_KEY_IDENTIFIER_SIZE 16
#endif
#ifndef FSCRYPT_POLICY_V1
#define FSCRYPT_POLICY_V1 0
#endif
#ifndef FSCRYPT_POLICY_V2
#define FSCRYPT_POLICY_V2 2
#endif

/* modes not supported by upstream kernel, so not in <linux/fs.h> */
#ifndef FS_ENCRYPTION_MODE_AES_256_HEH
#define FS_ENCRYPTION_MODE_AES_256_HEH 126
#endif
#ifndef FS_ENCRYPTION_MODE_PRIVATE
#define FS_ENCRYPTION_MODE_PRIVATE 127
#endif

#define FS_KEY_DESCRIPTOR_SIZE_HEX (2 * FS_KEY_DESCRIPTOR_SIZE + 1)
#define FSCRYPT_KEY_IDENTIFIER_HEX_SIZE ((2 * FSCRYPT_KEY_IDENTIFIER_SIZE) + 1)

#ifdef USE_FSCRYPT_POLICY_V1
#define USER_CE_FSCRYPT_POLICY           "0CE"
#define USER_DE_FSCRYPT_POLICY           "0DE"
#define SYSTEM_DE_FSCRYPT_POLICY         "0DK"
#else
#define USER_CE_FSCRYPT_POLICY           "2CE"
#define USER_DE_FSCRYPT_POLICY           "2DE"
#define SYSTEM_DE_FSCRYPT_POLICY         "2DK"
#endif

#define HEX_LOOKUP "0123456789abcdef"

__BEGIN_DECLS

typedef union fscrypt_policy {
	uint8_t version;
	struct fscrypt_policy_v1 v1;
	struct fscrypt_policy_v2 v2;
} fscrypt_policy;

static inline uint8_t* get_policy_descriptor(fscrypt_policy* fep)
{
	if (!fep)
		return NULL;
	switch (fep->version) {
	case FSCRYPT_POLICY_V1:
		return fep->v1.master_key_descriptor;
	case FSCRYPT_POLICY_V2:
		return fep->v2.master_key_identifier;
	default:
		return NULL;
	}
}

static inline uint8_t get_policy_size(fscrypt_policy* fep, bool hex)
{
	if (!fep)
		return 0;
	switch (fep->version) {
	case FSCRYPT_POLICY_V1:
		return hex ? FS_KEY_DESCRIPTOR_SIZE_HEX : FSCRYPT_KEY_DESCRIPTOR_SIZE;
	case FSCRYPT_POLICY_V2:
		return hex ? FSCRYPT_KEY_IDENTIFIER_HEX_SIZE : FSCRYPT_KEY_IDENTIFIER_SIZE;
	default:
		return 0;
	}
}

static inline void* get_policy(const fscrypt_policy* fep)
{
	if (!fep)
		return NULL;
	switch (fep->version) {
	case FSCRYPT_POLICY_V1:
		return (void*)&(fep->v1);
	case FSCRYPT_POLICY_V2:
		return (void*)&(fep->v2);
	default:
		return NULL;
	}
}

static inline int fscrypt_policy_size(const fscrypt_policy* fep)
{
	if (!fep)
		return 0;
	switch (fep->version) {
	case FSCRYPT_POLICY_V1:
		return sizeof(fep->v1);
	case FSCRYPT_POLICY_V2:
		return sizeof(fep->v2);
	default:
		return 0;
	}
}

static inline void get_policy_content(fscrypt_policy* fep, char* content)
{
	if (!fep || !content)
		return;
	switch (fep->version) {
	case FSCRYPT_POLICY_V1:
		sprintf(content, "%i %i %i %i %s", (int)fep->v1.version,
			(int)fep->v1.contents_encryption_mode,
			(int)fep->v1.filenames_encryption_mode,
			(int)fep->v1.flags,
			(const char*)fep->v1.master_key_descriptor);
		break;
	case FSCRYPT_POLICY_V2:
		sprintf(content, "%i %i %i %i %s", (int)fep->v2.version,
			(int)fep->v2.contents_encryption_mode,
			(int)fep->v2.filenames_encryption_mode,
			(int)fep->v2.flags,
			(const char*)fep->v2.master_key_identifier);
		break;
	}
}

/*
 * libvold's own header was not included (the normal case for libtar): pull in
 * its declarations, restated with the union type so libtar's call sites match.
 */
#ifndef _FS_CRYPT_H_
bool fscrypt_set_mode(void);
bool lookup_ref_key(fscrypt_policy* fep, uint8_t* policy_type);
bool lookup_ref_tar(fscrypt_policy* fep, uint8_t* policy);
bool fscrypt_policy_get_struct(const char* directory, fscrypt_policy* fep);
bool fscrypt_policy_set_struct(const char* directory, fscrypt_policy* fep);
void bytes_to_hex(const uint8_t* bytes, size_t num_bytes, char* hex);
#endif

__END_DECLS

#endif /* TWRP_LIBTAR_FSCRYPT_POLICY_COMPAT_H */
