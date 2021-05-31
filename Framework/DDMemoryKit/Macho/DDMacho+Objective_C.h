//
//  DDMacho+Objective_C.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/20.
//

#import <DDMemoryKit/DDMacho.h>

NS_ASSUME_NONNULL_BEGIN

extern NSString * _Nonnull const DDMachoFunctionInfoNameKey;
extern NSString * _Nonnull const DDMachoFunctionInfoIMPKey;

@interface DDMacho(Objective_C)
- (nonnull NSArray<NSDictionary<NSString *, NSValue *> *> *)getLaunchFunctionInfos;
- (nonnull NSArray<NSString *> *)getUnrealizedClassNames;
+ (nonnull NSArray<NSString *> *)getCacheFunctionNamesFromClass:(Class)c;
@end

NS_ASSUME_NONNULL_END
