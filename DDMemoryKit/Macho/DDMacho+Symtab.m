//
//  DDMacho+Symtab.m
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#import "DDMacho+Symtab.h"
#import "dd_macho_symtab.h"

@implementation DDMacho(Symtab)
- (nonnull NSArray<NSString *> *)allSynStrings
{
    NSMutableArray *ret = [NSMutableArray array];
    int size = 0;
    char **strs = dd_macho_copy_sym_strings((struct dd_macho *)self->_data, &size);
    if (NULL != strs) {
        for (int i = 0; i < size; ++i) {
            [ret addObject:[NSString stringWithFormat:@"%s", strs[i]]];
        }
        free(strs);
    }
    return [NSArray arrayWithArray:ret];
}
@end
