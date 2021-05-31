//
//  dd_macho_symtab.c
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#include "dd_macho_symtab.h"
#include <malloc/_malloc.h>

char **dd_macho_copy_sym_strings(struct dd_macho *macho, int *size)
{
    char **strings = NULL;
    int count = 0;
    char *ptr = (char *)macho->symtab->str_addr;
    int length = 0;
    while (length < macho->symtab->str_size) {
        size_t s = strlen(ptr);
        if (s > 0) {
            ++count;
        }
        ptr += s + 1;
        length += s + 1;
    }
    if (NULL != size) {
        *size = count;
    }
    if (count > 0) {
        strings = malloc(count * sizeof(char *));
        ptr = (char *)macho->symtab->str_addr;
        length = 0;
        for (int i = 0; i < count && length < macho->symtab->str_size;) {
            size_t s = strlen(ptr);
            if (s > 0) {
                strings[i] = ptr;
                ++i;
            }
            ptr += s + 1;
            length += s + 1;
        }
    }
    return strings;
}
