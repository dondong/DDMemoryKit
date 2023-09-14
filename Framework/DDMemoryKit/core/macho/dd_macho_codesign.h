//
//  dd_macho_codesign.h
//  DDMemoryKit
//
//  Created by 董明东 on 2021/5/20.
//

#ifndef dd_macho_codesign_h
#define dd_macho_codesign_h


#include <dd_macho.h>

char *dd_macho_get_identifier(struct dd_macho *macho);
char *dd_macho_copy_entitlements(struct dd_macho *macho);

#endif /* dd_macho_codesign_h */
