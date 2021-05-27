//
//  DDMacho.h
//  DDMemoryKit
//
//  Created by dondong on 2021/5/19.
//

#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@interface DDMachoSection : NSObject
@property(nonatomic,strong,nonnull) NSString *sectName;
@property(nonatomic,strong,nonnull) NSString *segName;
@property(nonatomic,assign) uintptr_t addr;
@property(nonatomic,assign) uint64_t  size;
@property(nonatomic,assign) uint32_t  flags;
@property(nonatomic,assign) uint32_t  reserved1;    /* reserved (for offset or index) */
@property(nonatomic,assign) uint32_t  reserved2;    /* reserved (for count or sizeof) */
@property(nonatomic,assign) uint32_t  reserved3;    /* reserved */
@end

@interface DDMachoSegment : NSObject
@property(nonatomic,strong,nonnull) NSString *segName;
@property(nonatomic,assign) uintptr_t addr;
@property(nonatomic,assign) uint64_t  size;
@property(nonatomic,assign) uint32_t  flags;
@property(nonatomic,strong) NSArray<DDMachoSection *> *sections;
@end

@interface DDMachoDylib : NSObject
@property(nonatomic,strong,nonnull) NSString *name;
@property(nonatomic,assign) uint32_t timestamp;
@property(nonatomic,assign) uint32_t currentVersion;
@property(nonatomic,assign) uint32_t compatibilityVersion;
@end

@interface DDMachoEncryptionInfo : NSObject
@property(nonatomic,assign) uintptr_t cryptAddr;
@property(nonatomic,assign) uint32_t  cryptSize;
@property(nonatomic,assign) uint32_t  cryptId;
@property(nonatomic,assign) uint32_t  pad;
@end

@interface DDMachoSymtab : NSObject
@property(nonatomic,assign) uintptr_t symAddr;
@property(nonatomic,assign) uint32_t  nsyms;
@property(nonatomic,assign) uintptr_t strAddr;
@property(nonatomic,assign) uint32_t  strSize;
@end

@interface DDMachoLinkedit : NSObject
@property(nonatomic,assign) uintptr_t addr;
@property(nonatomic,assign) uint32_t  size;
@end

@interface DDMacho : NSObject {
    @public
    void *_data;
}
@property(nonatomic,strong,nonnull) NSString *path;
@property(nonatomic,assign) uint64_t  cpuType;
@property(nonatomic,assign) uint64_t  cpuSubtype;
@property(nonatomic,assign) uint32_t  cmdSize;
@property(nonatomic,assign) uint32_t  flags;
@property(nonatomic,strong,nonnull) NSString *uuid;
@property(nonatomic,strong,nullable) NSArray<DDMachoSegment *> *segments;
@property(nonatomic,strong,nullable) NSArray<DDMachoDylib *>   *dylibs;
@property(nonatomic,strong,nullable) NSArray<NSString *>       *rpathes;
@property(nonatomic,strong,nullable) DDMachoEncryptionInfo *encryptionInfo;
@property(nonatomic,strong,nullable) DDMachoSymtab         *symtab;
@property(nonatomic,strong,nullable) DDMachoLinkedit       *codeSignature;
+ (NSUInteger)getMachoCount;
+ (nullable DDMacho *)machoAtIndex:(NSUInteger)index;
@end

NS_ASSUME_NONNULL_END
