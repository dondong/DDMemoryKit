//
//  dd_macho_codesign.c
//  DDMemoryKit
//
//  Created by 董明东 on 2021/5/20.
//

#include "dd_macho_codesign.h"
#include <malloc/_malloc.h>

enum {
    CSMAGIC_REQUIREMENT = 0xfade0c00,        /* single Requirement blob */
    CSMAGIC_REQUIREMENTS = 0xfade0c01,        /* Requirements vector (internal requirements) */
    CSMAGIC_CODEDIRECTORY = 0xfade0c02,        /* CodeDirectory blob */
    CSMAGIC_EMBEDDED_SIGNATURE = 0xfade0cc0, /* embedded form of signature data */
    CSMAGIC_EMBEDDED_SIGNATURE_OLD = 0xfade0b02,    /* XXX */
    CSMAGIC_EMBEDDED_ENTITLEMENTS = 0xfade7171,    /* embedded entitlements */
    CSMAGIC_DETACHED_SIGNATURE = 0xfade0cc1, /* multi-arch collection of embedded signatures */
    CSMAGIC_BLOBWRAPPER = 0xfade0b01,    /* CMS Signature, among other things */
    
    CS_SUPPORTSSCATTER = 0x20100,
    CS_SUPPORTSTEAMID = 0x20200,

    CSSLOT_CODEDIRECTORY = 0,                /* slot index for CodeDirectory */
    CSSLOT_INFOSLOT = 1,
    CSSLOT_REQUIREMENTS = 2,
    CSSLOT_RESOURCEDIR = 3,
    CSSLOT_APPLICATION = 4,
    CSSLOT_ENTITLEMENTS = 5,

    CSSLOT_ALTERNATE_CODEDIRECTORIES = 0x1000, /* first alternate CodeDirectory, if any */
    CSSLOT_ALTERNATE_CODEDIRECTORY_MAX = 5,        /* max number of alternate CD slots */
    CSSLOT_ALTERNATE_CODEDIRECTORY_LIMIT = CSSLOT_ALTERNATE_CODEDIRECTORIES + CSSLOT_ALTERNATE_CODEDIRECTORY_MAX, /* one past the last */

    CSSLOT_SIGNATURESLOT = 0x10000,            /* CMS Signature */

    CSTYPE_INDEX_REQUIREMENTS = 0x00000002,        /* compat with amfi */
    CSTYPE_INDEX_ENTITLEMENTS = 0x00000005,        /* compat with amfi */

    CS_HASHTYPE_SHA1 = 1,
    CS_HASHTYPE_SHA256 = 2,
    CS_HASHTYPE_SHA256_TRUNCATED = 3,
    CS_HASHTYPE_SHA384 = 4,

    CS_SHA1_LEN = 20,
    CS_SHA256_TRUNCATED_LEN = 20,

    CS_CDHASH_LEN = 20,                        /* always - larger hashes are truncated */
    CS_HASH_MAX_SIZE = 48, /* max size of the hash we'll support */
};

struct CS_CodeDirectory {
    uint32_t magic;                    /* magic number (CSMAGIC_CODEDIRECTORY) */
    uint32_t length;                /* total length of CodeDirectory blob */
    uint32_t version;                /* compatibility version */
    uint32_t flags;                    /* setup and mode flags */
    uint32_t hashOffset;            /* offset of hash slot element at index zero */
    uint32_t identOffset;            /* offset of identifier string */
    uint32_t nSpecialSlots;            /* number of special hash slots */
    uint32_t nCodeSlots;            /* number of ordinary (code) hash slots */
    uint32_t codeLimit;                /* limit to main image signature range */
    uint8_t hashSize;                /* size of each hash in bytes */
    uint8_t hashType;                /* type of hash (cdHashType* constants) */
    uint8_t platform;                /* platform identifier; zero if not platform binary */
    uint8_t    pageSize;                /* log2(page size in bytes); 0 => infinite */
    uint32_t spare2;                /* unused (must be zero) */
    /* Version 0x20100 */
    uint32_t scatterOffset;                /* offset of optional scatter vector */
    /* Version 0x20200 */
    uint32_t teamOffset;                /* offset of optional team identifier */
    /* followed by dynamic content as located by offset fields above */
};

struct CS_BlobIndex {
    uint32_t type;                    /* type of entry */
    uint32_t offset;                /* offset of entry */
};

struct CS_SuperBlob {
    uint32_t magic;                    /* magic number */
    uint32_t length;                /* total length of SuperBlob */
    uint32_t count;                    /* number of index entries following */
    struct CS_BlobIndex index[];            /* (count) entries */
    /* followed by Blobs in no particular order as indicated by offsets in index */
} ;

#define KERNEL_HAVE_CS_GENERICBLOB 1
struct CS_GenericBlob {
    uint32_t magic;                /* magic number */
    uint32_t length;            /* total length of blob */
    char data[];
};

struct SC_Scatter {
    uint32_t count;            // number of pages; zero for sentinel (only)
    uint32_t base;            // first page number
    uint64_t targetOffset;        // offset in target
    uint64_t spare;            // reserved
};

#define Swap32(x) (true == swap ? _OSSwapInt32(x) : x)
char *dd_macho_get_identifier(struct dd_macho *macho)
{
    char *name = NULL;
    if (NULL != macho->code_signature) {
        struct CS_SuperBlob *super_blob = (struct CS_SuperBlob *)macho->code_signature->addr;
        if ((CSMAGIC_EMBEDDED_SIGNATURE == super_blob->magic && super_blob->length <= macho->code_signature->size) ||
            (CSMAGIC_EMBEDDED_SIGNATURE == _OSSwapInt32(super_blob->magic) && _OSSwapInt32(super_blob->length) <= macho->code_signature->size)) {
            bool swap = (CSMAGIC_EMBEDDED_SIGNATURE != super_blob->magic);
            int count = Swap32(super_blob->count);
            for (int i = 0; i < count; ++i) {
                struct CS_BlobIndex *blob_index = &super_blob->index[i];
                char *ptr = (char *)macho->code_signature->addr + Swap32(blob_index->offset);
                if (CSMAGIC_CODEDIRECTORY == Swap32(*(uint32_t *)ptr)) {
                    struct CS_CodeDirectory *code_directory = (struct CS_CodeDirectory *)ptr;
                    name = ptr + Swap32(code_directory->identOffset);
                    break;
                }
            }
        }
    }
    return name;
}

char *dd_macho_copy_entitlements(struct dd_macho *macho)
{
    char *entitlements = NULL;
    if (NULL != macho->code_signature) {
        struct CS_SuperBlob *super_blob = (struct CS_SuperBlob *)macho->code_signature->addr;
        if ((CSMAGIC_EMBEDDED_SIGNATURE == super_blob->magic && super_blob->length <= macho->code_signature->size) ||
            (CSMAGIC_EMBEDDED_SIGNATURE == _OSSwapInt32(super_blob->magic) && _OSSwapInt32(super_blob->length) <= macho->code_signature->size)) {
            bool swap = (CSMAGIC_EMBEDDED_SIGNATURE != super_blob->magic);
            int count = Swap32(super_blob->count);
            for (int i = 0; i < count; ++i) {
                struct CS_BlobIndex *blob_index = &super_blob->index[i];
                char *ptr = (char *)macho->code_signature->addr + Swap32(blob_index->offset);
                if (CSMAGIC_EMBEDDED_ENTITLEMENTS == Swap32(*(uint32_t *)ptr)) {
                    struct CS_GenericBlob *blob = (struct CS_GenericBlob *)ptr;
                    uint32_t length = Swap32(blob->length);
                    if (length > sizeof(uint32_t) * 2) {
                        uint32_t size = length - sizeof(uint32_t) * 2 + 1;
                        entitlements = malloc(size);
                        memset(entitlements, 0, size);
                        memcpy(entitlements, ptr + sizeof(uint32_t) * 2, size - 1);
                    }
                    break;
                }
            }
        }
    }
    return entitlements;
}
