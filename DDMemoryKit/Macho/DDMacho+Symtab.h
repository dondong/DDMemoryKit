//
//  DDMacho+Symtab.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#import <DDMemoryKit/DDMacho.h>

NS_ASSUME_NONNULL_BEGIN

@interface DDMacho(Symtab)
- (nonnull NSArray<NSString *> *)allSynStrings;
@end

NS_ASSUME_NONNULL_END
