//
//  dd_macho_symtab.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#ifndef dd_macho_symtab_h
#define dd_macho_symtab_h

#include <DDMemoryKit/dd_macho.h>

char **dd_macho_copy_sym_strings(struct dd_macho *macho, int *size);

#endif /* dd_macho_symtab_h */
