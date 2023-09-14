//
//  DDMacho+Function_Starts.m
//  DDMemoryKit
//
//  Created by dondong on 2023/9/15.
//

#import "DDMacho+Function_Starts.h"
#import "dd_macho_function_starts.h"

@implementation DDMacho(Function_Starts)
- (nonnull NSArray<NSValue *> *)allFunctionStarts
{
    NSMutableArray *ret = [NSMutableArray array];
    int size = 0;
    void **funcList = dd_macho_copy_function_starts((struct dd_macho *)self->_data, &size);
    if (NULL != funcList) {
        for (int i = 0; i < size; ++i) {
            [ret addObject:[NSValue valueWithPointer:funcList[i]]];
        }
        free(funcList);
    }
    return [NSArray arrayWithArray:ret];
}
@end
