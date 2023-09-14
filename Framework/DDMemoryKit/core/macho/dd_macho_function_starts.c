//
//  dd_macho_function_starts.c
//  DDMemoryKit
//
//  Created by dondong on 2023/9/14.
//

#include "dd_macho_function_starts.h"
#include <malloc/_malloc.h>
int _scan_function_starts_value(struct dd_macho *macho, void **buffer)
{
    int size = 0;
    uint8_t *funcPtr = NULL;
    for (int i = 0; i < macho->msegments; ++i) {
        if (0 == strcmp(macho->segments[i].seg_name, "__TEXT")) {
            funcPtr = (uint8_t *)macho->segments[i].addr;
            break;
        }
    }
    if (NULL == funcPtr) {
        return 0;
    }
    uint8_t *valuePtr = (uint8_t *)macho->function_starts->addr;
    uint64_t func_offset = 0;
    uint64_t value_offset = 0;
    while (value_offset < macho->function_starts->size) {
        uint8_t *start = valuePtr + value_offset;
        uint8_t *ptr = start;
        uint64_t value = 0;
        int bit = 0;
        do {
            uint64_t slice = *ptr & 0x7f;
            if (bit >= 64 || slice << bit >> bit != slice) {
                // assert
            } else {
                value |= (slice << bit);
                bit += 7;
            }
        } while (*ptr++ & 0x80);
        value_offset += ptr - start;
        func_offset += value;
        
        if (NULL != buffer) {
            buffer[size] = (void *)(funcPtr + func_offset);
        }
        size++;
    }
    return size;
}

void **dd_macho_copy_function_starts(struct dd_macho *macho, int *size)
{
    int count = _scan_function_starts_value(macho, NULL);
    if (NULL != size) {
        *size = count;
    }
    void **result = malloc(sizeof(void *) * count);
    _scan_function_starts_value(macho, result);
    return result;
}

