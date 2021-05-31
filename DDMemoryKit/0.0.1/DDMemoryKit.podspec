#
# Be sure to run `pod lib lint DDMemoryKit.podspec' to ensure this is a
# valid spec before submitting.
#
# Any lines starting with a # are optional, but their use is encouraged
# To learn more about a Podspec see https://guides.cocoapods.org/syntax/podspec.html
#

Pod::Spec.new do |s|
  s.name     = 'DDMemoryKit'
  s.version  = '0.0.1'
  s.license  = 'MIT'
  s.summary  = 'A description of DDMemoryKit.'
  s.homepage = 'https://github.com/dondong/DDKit'
  s.authors  = { 'dondong' => 'the-last-choice@qq.com' }
  s.source   = { :git => 'https://github.com/dondong/DDMemoryKit.git', :tag => s.version }
  
  s.platform = :ios
  s.ios.deployment_target = '9.0'

  s.ios.pod_target_xcconfig = { 'PRODUCT_BUNDLE_IDENTIFIER' => 'com.dd.kit.memory' }

  s.source_files = 'DDMemoryKit/Classes/DDMemoryKit.h'
  s.public_header_files = 'DDMemoryKit/Classes/DDMemoryKit.h'
  s.subspec 'Macho' do |ss|
    ss.source_files = 'DDMemoryKit/Classes/Macho/*.{h,m}'
    ss.public_header_files = 'DDMemoryKit/Classes/Macho/*.h'
    ss.dependency 'DDMemoryKit/core/macho'
  end

  s.subspec 'core' do |ss|
    ss.source_files = 'DDMemoryKit/Classes/core/dd_memory_kit.h'
    ss.subspec 'macho' do |sss|
      sss.source_files = 'DDMemoryKit/Classes/core/macho/*.{h,c}'
      sss.public_header_files = 'DDMemoryKit/Classes/core/macho/*.h'
    end
  end

end
