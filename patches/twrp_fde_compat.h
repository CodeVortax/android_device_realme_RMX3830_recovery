/*
 * FDE compat stubs for TWRP android-14.1 on an FBE-only device.
 *
 * bootable/recovery partition.cpp / partitionmanager.cpp still call the legacy
 * FDE helpers, but android-14.1 system/vold no longer ships them (its cryptfs.h
 * only declares cryptfs_setup_ext_volume/cryptfs_get_keygen). RMX3830 uses
 * FBE, so these report "no FDE footer / not decryptable via FDE".
 */
#ifndef TWRP_FDE_COMPAT_H
#define TWRP_FDE_COMPAT_H
#ifdef __cplusplus
#ifdef TW_INCLUDE_CRYPTO

static inline void set_partition_data(const char*, const char*) {}
static inline int cryptfs_check_footer() { return -1; }
static inline int cryptfs_get_password_type() { return 0; /* CRYPT_TYPE_PASSWORD */ }
static inline int cryptfs_check_passwd(const char*) { return -1; }
static inline int delete_crypto_blk_dev(char*) { return 0; }

#endif
#endif
#endif
