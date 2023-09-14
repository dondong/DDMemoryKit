//
//  dd_macho_function_starts.h
//  DDMemoryKit
//
//  Created by dondong on 2023/9/14.
//

#ifndef dd_macho_function_starts_h
#define dd_macho_function_starts_h

#include "dd_macho.h"

void **dd_macho_copy_function_starts(struct dd_macho *macho, int *size);

#endif /* dd_macho_function_starts_h */
