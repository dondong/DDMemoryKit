//
//  dd_macho.hpp
//  DDMemoryKit
//
//  Created by 董明东 on 2021/5/18.
//

#ifndef dd_macho_hpp
#define dd_macho_hpp

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <mach-o/dyld.h>


struct dd_macho_section {
    char sect_name[17];
    char seg_name[17];
    uintptr_t addr;
    uint64_t  size;
    uint32_t  flags;
    uint32_t  reserved1;    /* reserved (for offset or index) */
    uint32_t  reserved2;    /* reserved (for count or sizeof) */
    uint32_t  reserved3;    /* reserved */
};

struct dd_macho_segment {
    char seg_name[17];
    uintptr_t addr;
    uint64_t  size;
    uint32_t  flags;
    uint32_t  msections;
    struct dd_macho_section* sections;
};

struct dd_macho_dylib {
    char *   name;
    uint32_t timestamp;
    uint32_t current_version;
    uint32_t compatibility_version;
};

struct dd_macho_encryption_info {
    uintptr_t crypt_addr;
    uint32_t  crypt_size;
    uint32_t  crypt_id;
    uint32_t  pad;
};

struct dd_macho_symtab {
    uintptr_t sym_addr;
    uint32_t  nsyms;
    uintptr_t str_addr;
    uint32_t  str_size;
};

struct dd_macho_linkedit {
    uintptr_t addr;
    uint32_t  size;
};

struct dd_macho
{
    const struct mach_header *header;
    const char *path;
    uint64_t  cpu_type;
    uint64_t  cpu_subtype;
    uint32_t  cmd_size;
    uint32_t  flags;
    uint32_t  msegments;
    uint8_t   uuid[16];
    struct dd_macho_segment *segments;
    uint32_t  mdylibs;
    struct dd_macho_dylib *dylibs;
    uint32_t mrpathes;
    char **  rpathes;
    struct dd_macho_encryption_info *encryption_info;
    struct dd_macho_symtab *symtab;
    struct dd_macho_linkedit *code_signature;
};

int dd_get_macho_count(void);
struct dd_macho *dd_copy_macho_at_index(unsigned int index);
void dd_delete_macho(struct dd_macho *macho);

#endif /* dd_macho_h */
