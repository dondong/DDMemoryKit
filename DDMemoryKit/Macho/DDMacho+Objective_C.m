//
//  DDMacho+Objective_C.m
//  DDMemoryKit
//
//  Created by dondong on 2021/5/20.
//

#import "DDMacho+Objective_C.h"
#import "dd_macho_objective_c.h"

NSString * const DDMachoFunctionInfoNameKey = @"name";
NSString * const DDMachoFunctionInfoIMPKey  = @"imp";

@implementation DDMacho(Objective_C)
- (nonnull NSArray<NSDictionary *> *)getLaunchFunctionInfos
{
    int count = 0;
    struct dd_macho_function_info *arr = dd_copy_launch_function_infos_from_macho((struct dd_macho *)self->_data, &count);
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:count];
    for (int i = 0; i < count; ++i) {
        [ret addObject:@{DDMachoFunctionInfoNameKey: [NSString stringWithFormat:@"%s", arr[i].name],
                         DDMachoFunctionInfoIMPKey : [NSValue valueWithPointer:arr[i].func_imp]}];
    }
    if (NULL != arr) {
        free(arr);
    }
    return [NSArray arrayWithArray:ret];
}

- (nonnull NSArray<NSString *> *)getUnrealizedClassNames
{
    int count = 0;
    char **arr = dd_copy_unrealized_class_names_from_macho((struct dd_macho *)self->_data, &count);
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:count];
    for (int i = 0; i < count; ++i) {
        [ret addObject:[NSString stringWithFormat:@"%s", arr[i]]];
    }
    if (NULL != arr) {
        free(arr);
    }
    return [NSArray arrayWithArray:ret];
}

+ (nonnull NSArray<NSString *> *)getCacheFunctionNamesFromClass:(Class)c
{
    int count = 0;
    char **arr = dd_copy_cache_function_names_from_class(c, &count);
    NSMutableArray *ret = [NSMutableArray arrayWithCapacity:count];
    for (int i = 0; i < count; ++i) {
        [ret addObject:[NSString stringWithFormat:@"%s", arr[i]]];
    }
    if (NULL != arr) {
        free(arr);
    }
    return [NSArray arrayWithArray:ret];
}
@end
