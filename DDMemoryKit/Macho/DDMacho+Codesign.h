//
//  DDMacho+Codesign.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/21.
//

#import <DDMemoryKit/DDMacho.h>

NS_ASSUME_NONNULL_BEGIN

@interface DDMacho(Codesign)
- (nonnull NSString *)identifier;
- (nonnull NSString *)entitlements;
@end

NS_ASSUME_NONNULL_END
