//
//  DDMacho.m
//  DDMemoryKit
//
//  Created by dondong on 2021/5/19.
//

#import "DDMacho.h"
#import "dd_macho.h"

@implementation DDMacho
+ (NSUInteger)getMachoCount
{
    return dd_get_macho_count();
}

+ (nullable DDMacho *)machoAtIndex:(NSUInteger)index
{
    DDMacho *ret = nil;
    struct dd_macho *macho = dd_copy_macho_at_index((unsigned int)index);
    if (NULL != macho) {
        ret = [[self alloc] init];
        ret->_data = macho;
        ret.path = [NSString stringWithFormat:@"%s", macho->path];
        ret.cpuType = macho->cpu_type;
        ret.cpuSubtype = macho->cpu_subtype;
        ret.cmdSize    = macho->cmd_size;
        ret.flags      = macho->flags;
        ret.uuid = [NSString stringWithFormat:@"%X%X%X%X-%X%X-%X%X-%X%X-%X%X%X%X%X%X", macho->uuid[0], macho->uuid[1], macho->uuid[2], macho->uuid[3], macho->uuid[4], macho->uuid[5], macho->uuid[6], macho->uuid[7], macho->uuid[8], macho->uuid[9], macho->uuid[10], macho->uuid[11], macho->uuid[12], macho->uuid[13], macho->uuid[14], macho->uuid[15]];
        if (NULL != macho->segments) {
            NSMutableArray *array = [NSMutableArray arrayWithCapacity:macho->msegments];
            char nameBuf[17];
            for (int i = 0; i < macho->msegments; ++i) {
                DDMachoSegment *segment = [[DDMachoSegment alloc] init];
                memset(nameBuf, 0, 17);
                memcpy(nameBuf, macho->segments[i].seg_name, 16);
                segment.segName = [NSString stringWithFormat:@"%s", nameBuf];
                segment.addr  = macho->segments[i].addr;
                segment.size  = macho->segments[i].size;
                segment.flags = macho->segments[i].flags;
                NSMutableArray *arr = [NSMutableArray arrayWithCapacity:macho->segments[i].msections];
                for (int j = 0; j < macho->segments[i].msections; ++j) {
                    DDMachoSection *section = [[DDMachoSection alloc] init];
                    memset(nameBuf, 0, 17);
                    memcpy(nameBuf, macho->segments[i].sections[j].sect_name, 16);
                    section.sectName = [NSString stringWithFormat:@"%s", nameBuf];
                    memset(nameBuf, 0, 17);
                    memcpy(nameBuf, macho->segments[i].sections[j].seg_name, 16);
                    section.segName = [NSString stringWithFormat:@"%s", nameBuf];
                    section.addr      = macho->segments[i].sections[j].addr;
                    section.size      = macho->segments[i].sections[j].size;
                    section.flags     = macho->segments[i].sections[j].flags;
                    section.reserved1 = macho->segments[i].sections[j].reserved1;
                    section.reserved2 = macho->segments[i].sections[j].reserved2;
                    section.reserved3 = macho->segments[i].sections[j].reserved3;
                    [arr addObject:section];
                }
                segment.sections = [NSArray arrayWithArray:arr];
                [array addObject:segment];
            }
            ret.segments = [NSArray arrayWithArray:array];
        }
        if (NULL != macho->dylibs) {
            NSMutableArray *array = [NSMutableArray arrayWithCapacity:macho->mdylibs];
            for (int i = 0; i < macho->mdylibs; ++i) {
                DDMachoDylib *dylib = [[DDMachoDylib alloc] init];
                dylib.name = [NSString stringWithFormat:@"%s", macho->dylibs[i].name];
                dylib.currentVersion       = macho->dylibs[i].current_version;
                dylib.compatibilityVersion = macho->dylibs[i].compatibility_version;
                [array addObject:dylib];
            }
            ret.dylibs = [NSArray arrayWithArray:array];
        }
        if (NULL != macho->rpathes) {
            NSMutableArray *array = [NSMutableArray arrayWithCapacity:macho->mrpathes];
            for (int i = 0; i < macho->mrpathes; ++i) {
                NSString *rpath = [NSString stringWithFormat:@"%s", macho->rpathes[i]];
                [array addObject:rpath];
            }
            ret.rpathes = [NSArray arrayWithArray:array];
        }
        if (NULL != macho->encryption_info) {
            ret.encryptionInfo = [[DDMachoEncryptionInfo alloc] init];
            ret.encryptionInfo.cryptAddr = macho->encryption_info->crypt_addr;
            ret.encryptionInfo.cryptSize = macho->encryption_info->crypt_size;
            ret.encryptionInfo.cryptId   = macho->encryption_info->crypt_id;
            ret.encryptionInfo.pad       = macho->encryption_info->pad;
        }
        if (NULL != macho->symtab) {
            ret.symtab = [[DDMachoSymtab alloc] init];
            ret.symtab.symAddr = macho->symtab->sym_addr;
            ret.symtab.nsyms   = macho->symtab->nsyms;
            ret.symtab.strAddr = macho->symtab->str_addr;
            ret.symtab.strSize = macho->symtab->str_size;
        }
        if (NULL != macho->code_signature) {
            ret.codeSignature = [[DDMachoLinkedit alloc] init];
            ret.codeSignature.addr = macho->code_signature->addr;
            ret.codeSignature.size = macho->code_signature->size;
        }
    }
    return ret;
}

- (void)dealloc
{
    if (NULL != self->_data) {
        dd_delete_macho((struct dd_macho *)self->_data);
    }
}
@end

@implementation DDMachoSection
@end
@implementation DDMachoSegment
@end
@implementation DDMachoDylib
@end
@implementation DDMachoEncryptionInfo
@end
@implementation DDMachoSymtab
@end
@implementation DDMachoLinkedit
@end
