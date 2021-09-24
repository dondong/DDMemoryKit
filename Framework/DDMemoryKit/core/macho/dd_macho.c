//
//  dd_macho.cpp
//  DDMemoryKit
//
//  Created by 董明东 on 2021/5/18.
//

#include "dd_macho.h"
#include <malloc/_malloc.h>

struct dd_macho *dd_copy_main_macho()
{
    int count = _dyld_image_count();
    for (int i = 0; i < count; ++i) {
        const struct mach_header *header = _dyld_get_image_header(i);
        if (MH_EXECUTE == header->filetype) {
            return dd_copy_macho_at_index(i);
        }
    }
    return NULL;
}

int dd_get_macho_count(void)
{
    return _dyld_image_count();
}

struct dd_macho *dd_copy_macho_at_index(unsigned int index)
{
    if (index < 0 || _dyld_image_count() <= index) {
        return NULL;
    }
    struct dd_macho *macho = malloc(sizeof(struct dd_macho));
    macho->segments = NULL;
    macho->dylibs   = NULL;
    macho->rpathes  = NULL;
    macho->encryption_info = NULL;
    macho->symtab          = NULL;
    macho->dysymtab        = NULL;
    macho->dyld_info       = NULL;
    macho->code_signature  = NULL;
    const struct mach_header *header = _dyld_get_image_header((uint32_t)index);
    macho->header = header;
    macho->path  = _dyld_get_image_name((uint32_t)index);
    macho->cpu_type    = header->cputype;
    macho->cpu_subtype = header->cpusubtype;
    macho->cmd_size    = header->sizeofcmds + (header->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header));
    macho->flags       = header->flags;
    
    char *cmdPtr = (char *)header + (header->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header));
    macho->msegments = 0;
    macho->mdylibs   = 0;
    macho->mrpathes  = 0;
    for (int i = 0; i < header->ncmds; ++i) {
        struct load_command *loadCmd = (struct load_command *)cmdPtr;
        switch(loadCmd->cmd){
            case LC_SEGMENT:
            case LC_SEGMENT_64:
                macho->msegments++;
                break;
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
            case LC_REEXPORT_DYLIB:
                macho->mdylibs++;
                break;
            case LC_RPATH:
                macho->mrpathes++;
                break;
        }
        cmdPtr += loadCmd->cmdsize;
    }
    if (macho->msegments > 0) {
        macho->segments = malloc(macho->msegments * sizeof(struct dd_macho_segment));
    }
    if (macho->mdylibs > 0) {
        macho->dylibs = malloc(macho->mdylibs * sizeof(struct dd_macho_dylib));
    }
    if (macho->mrpathes > 0) {
        macho->rpathes = malloc(macho->mrpathes * sizeof(char *));
    }
    int seg_index = 0;
    int dyl_index = 0;
    int rpa_index = 0;
    uintptr_t slide       = _dyld_get_image_vmaddr_slide((uint32_t)index);
    uintptr_t file_offset = 0;
    uintptr_t vmaddr      = 0;
    cmdPtr = (char *)header + (header->magic == MH_MAGIC_64 ? sizeof(struct mach_header_64) : sizeof(struct mach_header));
    for (int i = 0; i < header->ncmds; ++i) {
        struct load_command *loadCmd = (struct load_command *)cmdPtr;
        switch(loadCmd->cmd){
            case LC_SEGMENT:
            {
                struct dd_macho_segment *segment = &macho->segments[seg_index];
                seg_index++;
                struct segment_command *command = (struct segment_command *)cmdPtr;
                memset(segment->seg_name, 0, 17);
                memcpy(segment->seg_name, command->segname, 16);
                segment->addr  = command->vmaddr + slide;
                segment->size  = command->vmsize;
                segment->flags = command->flags;
                segment->msections = command->nsects;
                segment->sections  = NULL;
                if (0 == strcmp(segment->seg_name, "__LINKEDIT")) {
                    file_offset = command->fileoff;
                    vmaddr      = command->vmaddr;
                }
                if (command->nsects > 0) {
                    segment->sections = malloc(command->nsects * sizeof(struct dd_macho_section));
                    char *sectionPtr = cmdPtr + sizeof(struct segment_command);
                    for (int j = 0; j < command->nsects; ++j) {
                        struct dd_macho_section *section = &segment->sections[j];
                        struct section *sec = (struct section *)sectionPtr;
                        memset(section->seg_name, 0, 17);
                        memcpy(section->seg_name, sec->segname, 16);
                        memset(section->sect_name, 0, 17);
                        memcpy(section->sect_name, sec->sectname, 16);
                        section->addr  = sec->addr + slide;
                        section->size  = sec->size;
                        section->flags = sec->flags;
                        section->reserved1 = sec->reserved1;
                        section->reserved2 = sec->reserved2;
                        section->reserved3 = 0;
                        sectionPtr += sizeof(struct section);
                    }
                }
            }
                break;
            case LC_SEGMENT_64:
            {
                struct dd_macho_segment *segment = &macho->segments[seg_index];
                seg_index++;
                struct segment_command_64 *command = (struct segment_command_64 *)cmdPtr;
                memset(segment->seg_name, 0, 17);
                memcpy(segment->seg_name, command->segname, 16);
                segment->addr  = (uintptr_t)command->vmaddr + slide;
                segment->size  = command->vmsize;
                segment->flags = command->flags;
                segment->msections = command->nsects;
                segment->sections  = NULL;
                if (0 == strcmp(segment->seg_name, "__LINKEDIT")) {
                    file_offset = (uintptr_t)command->fileoff;
                    vmaddr      = (uintptr_t)command->vmaddr;
                }
                if (command->nsects > 0) {
                    segment->sections = malloc(command->nsects * sizeof(struct dd_macho_section));
                    char *sectionPtr = cmdPtr + sizeof(struct segment_command_64);
                    for (int j = 0; j < command->nsects; ++j) {
                        struct dd_macho_section *section = &segment->sections[j];
                        struct section_64 *sec = (struct section_64 *)sectionPtr;
                        sectionPtr += sizeof(struct section_64);
                        memset(section->seg_name, 0, 17);
                        memcpy(section->seg_name, sec->segname, 16);
                        memset(section->sect_name, 0, 17);
                        memcpy(section->sect_name, sec->sectname, 16);
                        section->addr  = (uintptr_t)sec->addr + slide;
                        section->size  = sec->size;
                        section->flags = sec->flags;
                        section->reserved1 = sec->reserved1;
                        section->reserved2 = sec->reserved2;
                        section->reserved3 = sec->reserved3;
                    }
                }
            }
                break;
            case LC_LOAD_DYLIB:
            case LC_LOAD_WEAK_DYLIB:
            case LC_LOAD_UPWARD_DYLIB:
            case LC_REEXPORT_DYLIB:
            {
                struct dd_macho_dylib *dylib = &macho->dylibs[dyl_index];
                dyl_index++;
                struct dylib_command *command = (struct dylib_command *)cmdPtr;
                dylib->name      = (char *)(cmdPtr + command->dylib.name.offset);
                dylib->timestamp = command->dylib.timestamp;
                dylib->current_version       = command->dylib.current_version;
                dylib->compatibility_version = command->dylib.compatibility_version;

            }
                break;
            case LC_RPATH:
            {
                struct rpath_command *command = (struct rpath_command *)cmdPtr;
                macho->rpathes[rpa_index] = cmdPtr + command->path.offset;
                rpa_index++;
            }
                break;
            case LC_UUID:
            {
                struct uuid_command *command = (struct uuid_command *)cmdPtr;
                for (int i = 0; i < 16; ++i) {
                    macho->uuid[i] = command->uuid[i];
                }
            }
                break;
            case LC_ENCRYPTION_INFO:
            {
                if (NULL == macho->encryption_info) {
                    macho->encryption_info = malloc(sizeof(struct dd_macho_encryption_info));
                    struct encryption_info_command *command =  (struct encryption_info_command *)cmdPtr;
                    macho->encryption_info->crypt_addr = command->cryptoff;
                    macho->encryption_info->crypt_size = command->cryptsize;
                    macho->encryption_info->crypt_id   = command->cryptid;
                    macho->encryption_info->pad        = 0;
                }
            }
                break;
            case LC_ENCRYPTION_INFO_64:
            {
                if (NULL == macho->encryption_info) {
                    macho->encryption_info = malloc(sizeof(struct dd_macho_encryption_info));
                    struct encryption_info_command_64 *command =  (struct encryption_info_command_64 *)cmdPtr;
                    macho->encryption_info->crypt_addr = command->cryptoff;
                    macho->encryption_info->crypt_size = command->cryptsize;
                    macho->encryption_info->crypt_id   = command->cryptid;
                    macho->encryption_info->pad        = command->pad;
                }
            }
                break;
            case LC_SYMTAB:
            {
                if (NULL == macho->symtab) {
                    macho->symtab = malloc(sizeof(struct dd_macho_symtab));
                    struct symtab_command *command = (struct symtab_command *)cmdPtr;
                    macho->symtab->sym_addr = command->symoff;
                    macho->symtab->nsyms    = command->nsyms;
                    macho->symtab->str_addr = command->stroff;
                    macho->symtab->str_size = command->strsize;
                }
            }
                break;
            case LC_DYSYMTAB:
            {
                if (NULL == macho->dysymtab) {
                    macho->dysymtab = malloc(sizeof(struct dd_macho_dysymtab));
                    struct dysymtab_command *command = (struct dysymtab_command *)cmdPtr;
                    macho->dysymtab->ilocalsym = command->ilocalsym;
                    macho->dysymtab->nlocalsym = command->nlocalsym;
                    macho->dysymtab->iextdefsym = command->iextdefsym;
                    macho->dysymtab->nextdefsym = command->nextdefsym;
                    macho->dysymtab->iundefsym = command->iundefsym;
                    macho->dysymtab->nundefsym = command->nundefsym;
                    macho->dysymtab->tocaddr = command->tocoff;
                    macho->dysymtab->ntoc    = command->ntoc;
                    macho->dysymtab->modtabaddr = command->modtaboff;
                    macho->dysymtab->nmodtab    = command->nmodtab;
                    macho->dysymtab->extrefsymaddr = command->extrefsymoff;
                    macho->dysymtab->nextrefsyms   = command->nextrefsyms;
                    macho->dysymtab->indirectsymaddr = command->indirectsymoff;
                    macho->dysymtab->nindirectsyms   = command->nindirectsyms;
                    macho->dysymtab->extreladdr = command->extreloff;
                    macho->dysymtab->nextrel    = command->nextrel;
                    macho->dysymtab->locreladdr = command->locreloff;
                    macho->dysymtab->nlocrel    = command->nlocrel;
                }
            }
                break;
            case LC_DYLD_INFO:
            case LC_DYLD_INFO_ONLY:
            {
                if (NULL == macho->dyld_info) {
                    macho->dyld_info = malloc(sizeof(struct dd_macho_dyld_info));
                    struct dyld_info_command *command = (struct dyld_info_command *)cmdPtr;
                    macho->dyld_info->rebase_addr = command->rebase_off;
                    macho->dyld_info->rebase_size = command->rebase_size;
                    macho->dyld_info->bind_addr = command->bind_off;
                    macho->dyld_info->bind_size = command->bind_size;
                    macho->dyld_info->weak_bind_addr = command->weak_bind_off;
                    macho->dyld_info->weak_bind_size = command->weak_bind_size;
                    macho->dyld_info->lazy_bind_addr = command->lazy_bind_off;
                    macho->dyld_info->lazy_bind_size = command->lazy_bind_size;
                    macho->dyld_info->export_addr = command->export_off;
                    macho->dyld_info->export_size = command->export_size;
                }
            }
                break;
            case LC_CODE_SIGNATURE:
            {
                if (NULL == macho->code_signature) {
                    macho->code_signature = malloc(sizeof(struct dd_macho_linkedit));
                    struct linkedit_data_command *command = (struct linkedit_data_command *)cmdPtr;
                    macho->code_signature->addr = command->dataoff;
                    macho->code_signature->size = command->datasize;
                }
            }
                break;
        }
        cmdPtr += loadCmd->cmdsize;
    }
    if (NULL != macho->encryption_info) {
        macho->encryption_info->crypt_addr += slide;
    }
    if (NULL != macho->symtab) {
        if (0 != file_offset) {
            macho->symtab->sym_addr = vmaddr + macho->symtab->sym_addr - file_offset + slide;
            macho->symtab->str_addr = vmaddr + macho->symtab->str_addr - file_offset + slide;
        } else {
            free(macho->symtab);
            macho->symtab = NULL;
        }
    }
    if (NULL != macho->code_signature) {
        if (0 != file_offset) {
            macho->code_signature->addr = vmaddr + macho->code_signature->addr - file_offset + slide;
        } else {
            free(macho->code_signature);
            macho->code_signature = NULL;
        }
    }
    return macho;
}

void dd_delete_macho(struct dd_macho *macho)
{
    if (NULL != macho) {
        if (NULL != macho->segments) {
            for (int i = 0; i < macho->msegments; ++i) {
                if (NULL != macho->segments[i].sections) {
                    free(macho->segments[i].sections);
                    macho->segments[i].msections = 0;
                    macho->segments[i].sections  = NULL;
                }
            }
            free(macho->segments);
            macho->msegments = 0;
            macho->segments  = NULL;
        }
        if (NULL != macho->dylibs) {
            free(macho->dylibs);
            macho->mdylibs = 0;
            macho->dylibs  = NULL;
        }
        if (NULL != macho->rpathes) {
            free(macho->rpathes);
            macho->mrpathes = 0;
            macho->rpathes  = NULL;
        }
        if (NULL != macho->encryption_info) {
            free(macho->encryption_info);
            macho->encryption_info = NULL;
        }
        if (NULL != macho->symtab) {
            free(macho->symtab);
            macho->symtab = NULL;
        }
        if (NULL != macho->dysymtab) {
            free(macho->dysymtab);
            macho->dysymtab = NULL;
        }
        if (NULL != macho->dyld_info) {
            free(macho->dyld_info);
            macho->dyld_info = NULL;
        }
        if (NULL != macho->code_signature) {
            free(macho->code_signature);
            macho->code_signature = NULL;
        }
        free(macho);
    }
}
