//
//  dd_macho_objective_c.c
//  DDMemoryKit
//
//  Created by dondong on 2021/5/19.
//

#include "dd_macho_objective_c.h"
#include <malloc/_malloc.h>
#include <dlfcn.h>

#define RW_REALIZED           (1<<31)
#define FAST_DATA_MASK 0x00007ffffffffff8UL

struct bucket_t {
    // IMP-first is better for arm64e ptrauth and no worse for arm64.
    // SEL-first is better for armv7* and i386 and x86_64.
#if __arm64__
    IMP _imp;
    SEL _key;
#else
    SEL _key;
    IMP _imp;
#endif
};

#define maskShift    48
#define maskZeroBits 4
#define maxMask     (((uintptr_t)1 << (64 - maskShift)) - 1)
#define bucketsMask (((uintptr_t)1 << (maskShift - maskZeroBits)) - 1)
struct cache_t {
    uint64_t _maskAndBuckets;
    uint32_t _mask_unused;
    uint16_t _flags;
    uint16_t _occupied;
//    static constexpr uintptr_t maskShift = 48;
//
//    // Additional bits after the mask which must be zero. msgSend
//    // takes advantage of these additional bits to construct the value
//    // `mask << 4` from `_maskAndBuckets` in a single instruction.
//    static constexpr uintptr_t maskZeroBits = 4;
//
//    // The largest mask value we can store.
//    static constexpr uintptr_t maxMask = ((uintptr_t)1 << (64 - maskShift)) - 1;
//
//    // The mask applied to `_maskAndBuckets` to retrieve the buckets pointer.
//    static constexpr uintptr_t bucketsMask = ((uintptr_t)1 << (maskShift - maskZeroBits)) - 1;
//
//    uint16_t mask() const {
//        return _maskAndBuckets >> maskShift;
//    }
//
//    struct bucket_t *buckets() {
//        return (bucket_t *)(_maskAndBuckets & bucketsMask);
//    }
};

struct objc_class_t {
    uintptr_t *isa;
    uintptr_t *superclass;
    struct cache_t    cache;
    uintptr_t  data;
};

struct entsize_list_tt {
    uint32_t entsizeAndFlags;
    uint32_t count;
    uintptr_t first;
};

struct class_ro_t {
    uint32_t flags;
    uint32_t instanceStart;
    uint32_t instacneSize;
    uint32_t reserved;
    const uint8_t *ivarLayout;
    const char *name;
    struct entsize_list_tt * baseMethodList;
    uintptr_t * baseProtocols;
    struct entsize_list_tt * ivars;
    uintptr_t * weakIvarLayout;
    struct entsize_list_tt * baseProperties;
};

struct class_rw_t {
    uint32_t flags;
    uint32_t version;
    struct class_ro_t *ro;
};

struct category_t {
    const char *name;
    uintptr_t *cls;
    struct entsize_list_tt *instanceMethods;
    struct entsize_list_tt *classMethods;
    uintptr_t *protocols;
    uintptr_t *instanceProperties;
    // Fields below this point are not always present on disk.
    struct entsize_list_tt *_classProperties;
};

struct method_t {
    char *name;
    const char *types;
    IMP imp;
};

IMP *dd_copy_launch_function_imps_from_macho(struct dd_macho *macho, int *size)
{
    IMP *function_array = NULL;
    int count = 0;
    int index = 0;
    for (int i = 0; i < macho->msegments; ++i) {
        struct dd_macho_segment segment = macho->segments[i];
        if (0 == strcmp(segment.seg_name, "__DATA")) {
            for (int j = 0; j < segment.msections; ++j) {
                struct dd_macho_section section = segment.sections[j];
                if (0 == strcmp(section.sect_name, "__objc_nlclslist")) {
                    uintptr_t *bast_ptr = (uintptr_t *)section.addr;
                    int class_count = (int)section.size / sizeof(uintptr_t);
                    for (int k = 0; k < class_count; ++k) {
                        struct objc_class_t *class_ptr = (struct objc_class_t *)*(bast_ptr + k);
                        if (NULL != class_ptr && NULL != class_ptr->isa) {
                            struct objc_class_t *metal_class_ptr = (struct objc_class_t *)class_ptr->isa;
                            struct class_rw_t *rw_ptr = (struct class_rw_t *)(metal_class_ptr->data & FAST_DATA_MASK);
                            struct class_ro_t *ro_ptr = NULL;
                            if (0 != (RW_REALIZED & rw_ptr->flags)) {
                                ro_ptr = rw_ptr->ro;
                            } else {
                                ro_ptr = (struct class_ro_t *)rw_ptr;
                            }
                            if (NULL != ro_ptr && NULL != ro_ptr->baseMethodList) {
                                int32_t entry_size = ro_ptr->baseMethodList->entsizeAndFlags;
                                for (int l = 0; l < ro_ptr->baseMethodList->count; ++l) {
                                    struct method_t *method_ptr = (struct method_t *)(&(ro_ptr->baseMethodList->first) + (entry_size / sizeof(struct method_t)) * l);
                                    if (0 == strcmp(method_ptr->name, "load")) {
                                        count++;
                                    }
                                }
                            }
                        }
                    }
                    
                } else if (0 == strcmp(section.sect_name, "__objc_nlcatlist")) {
                    uintptr_t *bast_ptr = (uintptr_t *)section.addr;
                    int category_count = (int)section.size / sizeof(uintptr_t);
                    for (int k = 0; k < category_count; ++k) {
                        struct category_t *category_ptr = (struct category_t *)*(bast_ptr + k);
                        if (NULL != category_ptr->classMethods) {
                            int32_t entry_size = category_ptr->classMethods->entsizeAndFlags;
                            for (int l = 0; l < category_ptr->classMethods->count; ++l) {
                                struct method_t *method_ptr = (struct method_t *)(&(category_ptr->classMethods->first) + (entry_size / sizeof(uintptr_t *)) * l);
                                if (0 == strcmp(method_ptr->name, "load")) {
                                    count++;
                                }
                            }
                        }
                    }
                    
                } else if (0 == strcmp(section.sect_name, "__mod_init_func")) {
                    int func_count = (int)section.size / sizeof(uintptr_t);
                    count += func_count;
                    
                }
            }
            break;
        }
    }
    if (NULL != size) {
        *size = count;
    }
    if (count > 0) {
        function_array = malloc(count * sizeof(IMP));
        for (int i = 0; i < macho->msegments; ++i) {
            struct dd_macho_segment segment = macho->segments[i];
            if (0 == strcmp(segment.seg_name, "__DATA")) {
                for (int j = 0; j < segment.msections; ++j) {
                    struct dd_macho_section section = segment.sections[j];
                    if (0 == strcmp(section.sect_name, "__objc_nlclslist")) {
                        uintptr_t *bast_ptr = (uintptr_t *)section.addr;
                        int class_count = (int)section.size / sizeof(uintptr_t);
                        for (int k = 0; k < class_count; ++k) {
                            struct objc_class_t *class_ptr = (struct objc_class_t *)*(bast_ptr + k);
                            if (NULL != class_ptr && NULL != class_ptr->isa) {
                                struct objc_class_t *metal_class_ptr = (struct objc_class_t *)class_ptr->isa;
                                struct class_rw_t *rw_ptr = (struct class_rw_t *)(metal_class_ptr->data & FAST_DATA_MASK);
                                struct class_ro_t *ro_ptr = NULL;
                                if (0 != (RW_REALIZED & rw_ptr->flags)) {
                                    ro_ptr = rw_ptr->ro;
                                } else {
                                    ro_ptr = (struct class_ro_t *)rw_ptr;
                                }
                                if (NULL != ro_ptr && NULL != ro_ptr->baseMethodList) {
                                    int32_t entry_size = ro_ptr->baseMethodList->entsizeAndFlags;
                                    for (int l = 0; l < ro_ptr->baseMethodList->count; ++l) {
                                        struct method_t *method_ptr = (struct method_t *)(&(ro_ptr->baseMethodList->first) + (entry_size / sizeof(struct method_t)) * l);
                                        if (0 == strcmp(method_ptr->name, "load")) {
                                            function_array[index] = method_ptr->imp;
                                            index++;
                                        }
                                    }
                                }
                            }
                        }
                        
                    } else if (0 == strcmp(section.sect_name, "__objc_nlcatlist")) {
                        uintptr_t *bast_ptr = (uintptr_t *)section.addr;
                        int category_count = (int)section.size / sizeof(uintptr_t);
                        for (int k = 0; k < category_count; ++k) {
                            struct category_t *category_ptr = (struct category_t *)*(bast_ptr + k);
                            if (NULL != category_ptr->classMethods) {
                                int32_t entry_size = category_ptr->classMethods->entsizeAndFlags;
                                for (int l = 0; l < category_ptr->classMethods->count; ++l) {
                                    struct method_t *method_ptr = (struct method_t *)(&(category_ptr->classMethods->first) + (entry_size / sizeof(uintptr_t *)) * l);
                                    if (0 == strcmp(method_ptr->name, "load")) {
                                        function_array[index] = method_ptr->imp;
                                        index++;
                                    }
                                }
                            }
                        }
                        
                    } else if (0 == strcmp(section.sect_name, "__mod_init_func")) {
                        uintptr_t *bast_ptr = (uintptr_t *)section.addr;
                        int func_count = (int)section.size / sizeof(uintptr_t);
                        for (int k = 0; k < func_count; ++k) {
                            IMP func_ptr = (IMP)*(bast_ptr + k);
                            function_array[index] = func_ptr;
                            index++;
                        }
                        
                    }
                }
                break;
            }
        }
    }
    return function_array;
}

struct dd_macho_function_info *dd_copy_launch_function_infos_from_macho(struct dd_macho *macho, int *size)
{
    struct dd_macho_function_info *ret = NULL;
    int count = 0;
    IMP *arr = dd_copy_launch_function_imps_from_macho(macho, &count);
    if (NULL != size) {
        *size = count;
    }
    if (NULL != arr) {
        ret = malloc(count * sizeof(struct dd_macho_function_info));
        for (int i = 0; i < count; ++i) {
            //获取函数信息
            Dl_info info;
            dladdr(arr[i], &info);
            ret[i].func_imp = arr[i];
            ret[i].name = info.dli_sname;
        }
        free(arr);
    }
    return ret;
}

char **dd_copy_unrealized_class_names_from_macho(struct dd_macho *macho, int *size)
{
    char **name_array = NULL;
    int count = 0;
    for (int i = 0; i < macho->msegments; ++i) {
        struct dd_macho_segment segment = macho->segments[i];
        if (0 == strcmp(segment.seg_name, "__DATA")) {
            for (int j = 0; j < segment.msections; ++j) {
                struct dd_macho_section section = segment.sections[j];
                if (0 == strcmp(section.sect_name, "__objc_classlist")) {
                    uintptr_t *base_ptr = (uintptr_t *)section.addr;
                    int class_count = (int)section.size / sizeof(uintptr_t *);
                    for (int k = 0; k < class_count; ++k) {
                        struct objc_class_t *class_ptr = (struct objc_class_t *)*(base_ptr + k);
                        struct class_rw_t *rw_ptr = (struct class_rw_t *)(class_ptr->data & FAST_DATA_MASK);
                        if (0 == (RW_REALIZED & rw_ptr->flags)) {
                            count++;
                        }
                    }
                    break;
                }
            }
            break;
        }
    }
    if (NULL != size) {
        *size = count;
    }
    if (count > 0) {
        name_array = malloc(count * sizeof(char *));
        int index = 0;
        for (int i = 0; i < macho->msegments; ++i) {
            struct dd_macho_segment segment = macho->segments[i];
            if (0 == strcmp(segment.seg_name, "__DATA")) {
                for (int j = 0; j < segment.msections; ++j) {
                    struct dd_macho_section section = segment.sections[j];
                    if (0 == strcmp(section.sect_name, "__objc_classlist")) {
                        uintptr_t *base_ptr = (uintptr_t *)section.addr;
                        int class_count = (int)section.size / sizeof(uintptr_t);
                        for (int k = 0; k < class_count; ++k) {
                            struct objc_class_t *class_ptr = (struct objc_class_t *)*(base_ptr + k);
                            struct class_rw_t *rw_ptr = (struct class_rw_t *)(class_ptr->data & FAST_DATA_MASK);
                            if (0 == (RW_REALIZED & rw_ptr->flags)) {
                                name_array[index] = (char *)((struct class_ro_t *)rw_ptr)->name;
                                index++;
                            }
                        }
                        break;
                    }
                }
                break;
            }
        }
    }
    return name_array;
}

bool dd_macho_is_class_realized(Class c)
{
    struct objc_class_t *class_ptr = (struct objc_class_t *)c;
    struct class_rw_t *rw_ptr = (struct class_rw_t *)(class_ptr->data & FAST_DATA_MASK);
    if (0 == (RW_REALIZED & rw_ptr->flags)) {
        return false;
    } else {
        return true;
    }
}

char **dd_copy_cache_function_names_from_class(Class c, int *size)
{
    char **name_array = NULL;
    struct objc_class_t *class_ptr = (struct objc_class_t *)c;
    if (NULL != size) {
        *size = class_ptr->cache._occupied;
    }
    if (class_ptr->cache._occupied > 0) {
        name_array = malloc(class_ptr->cache._occupied * sizeof(char *));
        int index = 0;
        uint16_t mask = class_ptr->cache._maskAndBuckets >> maskShift;
        struct bucket_t *buckets = (struct bucket_t *)(class_ptr->cache._maskAndBuckets & bucketsMask);
        for (int i = 0; i < mask; ++i) {
            if (NULL != buckets[i]._key) {
                name_array[index] = (char *)buckets[i]._key;
                ++index;
            }
        }
    }
    return name_array;
}
