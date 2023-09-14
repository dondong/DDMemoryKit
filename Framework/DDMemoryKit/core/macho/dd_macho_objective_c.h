//
//  dd_macho_objective_c.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/19.
//

#ifndef dd_macho_objective_c_h
#define dd_macho_objective_c_h

#include <dd_macho.h>
#import <objc/runtime.h>

struct dd_macho_function_info
{
    const char *name;
    IMP func_imp;
};


IMP *dd_copy_launch_function_imps_from_macho(struct dd_macho *macho, int *size);
struct dd_macho_function_info *dd_copy_launch_function_infos_from_macho(struct dd_macho *macho, int *size);
char **dd_copy_unrealized_class_names_from_macho(struct dd_macho *macho, int *size);
bool dd_macho_is_class_realized(Class c);
char **dd_copy_cache_function_names_from_class(Class c, int *size);

#endif /* dd_macho_objective_c_h */
