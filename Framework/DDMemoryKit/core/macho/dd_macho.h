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

struct dd_macho_dyld_info {
    uint32_t   rebase_addr;    /* file offset to rebase info  */
    uint32_t   rebase_size;    /* size of rebase info   */
    uint32_t   bind_addr;    /* file offset to binding info   */
    uint32_t   bind_size;    /* size of binding info  */
    uint32_t   weak_bind_addr;    /* file offset to weak binding info   */
    uint32_t   weak_bind_size;  /* size of weak binding info  */
    uint32_t   lazy_bind_addr;    /* file offset to lazy binding info */
    uint32_t   lazy_bind_size;  /* size of lazy binding infs */
    uint32_t   export_addr;    /* file offset to lazy binding info */
    uint32_t   export_size;    /* size of lazy binding infs */
};

struct dd_macho_dysymtab {
    uint32_t ilocalsym;    /* index to local symbols */
    uint32_t nlocalsym;    /* number of local symbols */
    uint32_t iextdefsym;/* index to externally defined symbols */
    uint32_t nextdefsym;/* number of externally defined symbols */
    uint32_t iundefsym;    /* index to undefined symbols */
    uint32_t nundefsym;    /* number of undefined symbols */
    uint32_t tocaddr;    /* file offset to table of contents */
    uint32_t ntoc;    /* number of entries in table of contents */
    uint32_t modtabaddr;    /* file offset to module table */
    uint32_t nmodtab;    /* number of module table entries */
    uint32_t extrefsymaddr;    /* offset to referenced symbol table */
    uint32_t nextrefsyms;    /* number of referenced symbol table entries */
    uint32_t indirectsymaddr; /* file offset to the indirect symbol table */
    uint32_t nindirectsyms;  /* number of indirect symbol table entries */
    uint32_t extreladdr;    /* offset to external relocation entries */
    uint32_t nextrel;    /* number of external relocation entries */
    uint32_t locreladdr;    /* offset to local relocation entries */
    uint32_t nlocrel;    /* number of local relocation entries */
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
    struct dd_macho_dysymtab *dysymtab;
    struct dd_macho_dyld_info *dyld_info;
    struct dd_macho_linkedit *function_starts;
    struct dd_macho_linkedit *code_signature;
};

int dd_get_macho_count(void);
struct dd_macho *dd_copy_macho_at_index(unsigned int index);
struct dd_macho *dd_copy_main_macho(void);
void dd_delete_macho(struct dd_macho *macho);

#endif /* dd_macho_h */
