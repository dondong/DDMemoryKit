//
//  DDMacho+Codesign.m
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#import "DDMacho+Codesign.h"
#import "dd_macho_codesign.h"

@implementation DDMacho(Codesign)
- (nonnull NSString *)identifier
{
    char *str = dd_macho_get_identifier((struct dd_macho *)self->_data);
    if (NULL != str) {
        return [NSString stringWithFormat:@"%s", str];
    } else {
        return @"";
    }
}

- (nonnull NSString *)entitlements
{
    char *str = dd_macho_copy_entitlements((struct dd_macho *)self->_data);
    if (NULL != str) {
        NSString *ret = [NSString stringWithFormat:@"%s", str];
        free(str);
        return ret;
    } else {
        return @"";
    }
}
@end
