# 微服务治理框架(C++版)windows开发环境搭建与配置
# 1 系统环境  
## 1）操作系统  
`Windows 7 SP1 X64`  
OR   
`Windows2008 SP1`  
## 2）编译软件  
	VS2017 及以上   
	  
## 3）第三方软件  
	Zookeeper 3.4.13+  
    JDK 1.8+
  
# 2 工具安装
## 2.1 Visual Studio 2017 （VS2017）安装（已安装则忽略）

1)	下载最新的免费的Visual Studio 2017 Community（社区版）  
2)	“以管理员身份运行”安装程序(Visual Studio Installer)  
3)	勾选“使用c + +的桌面开发”、修改安装路径（目录）  
参考文献：

[Visual Studio 2017 （VS2017）安装、配置和使用](https://www.jianshu.com/p/320aefbc582d)
                                                     
   
## 2.2 安装JDK（已安装则忽略）
  JDK为zookeeper 安装与运行所需，假如Zookeeper 已安装并运行在可以连通windows的机器上，忽略本章节。
## 2.3 安装Zookeeper

安装Zookeeper，建议使用3.4.13或以上版本。  
 - 1.下载zookeeper  
 - 2.进入conf目录,把zoo_sample.cfg修改成zoo.cfg,必须是叫这个名称。然后对其进行修改。  
 - 3.在解压目录下创建data和log目录  
 - 4.将解压后zookeeper-3.4.13文件夹下的zoo_sample.cfg文件拷贝一份命名为zoo.cfg  
 - 5.修改zoo.cfg文件，添加data和log目录路径  
 - 6.启动ZK 
参考文献：
[windows安装zookeeper说明](https://blog.csdn.net/qq_30764991/article/details/80188652)
 
# 3 获取源码
  - git获取代码
   
		git init                
		git clone  https://github.com/grpc-nebula/grpc-nebula-c.git  //克隆主模块
		git submodule  init    
		git submodule  update          //克隆子模块
  - git 获取zookeeper库和第三方库
    
        //获取zookeeper库和grpc依赖第三方库
        https://github.com/grpc-nebula/grpc-nebula.git
     

# 4 发布包SDK制作
   grpc-c 源代码存放在D:\Repo\grpc下， zookeeper源代码存放在D:\Repo\zookeeper。第三方库保存在D:\Repo\third_part_sdk， 目录可更改，以上为示例。

## 4.1 grpc依赖库编译 
### 4.1.1 grpc third_party libs
使用vs2017打开Cmakelists.txt方式进行各种版本（x64 debug，x64 release，x86 debug，x86 release版本编译）  
将编译后的第三方库拷贝到D:\Repo\third_part_sdk
### 4.1.2 zookeeper库编译

git clone -b branch-3.4.13 https://github.com/apache/zookeeper.git  
使用vs2013编译zookeeper库，或者在grpc-nebula目录下拷贝。

## 4.2 代码编译
### 4.2.1 项目配置  
打开 grpc\vsprojects\grpc.sln  
编译 gpr.lib、grpc.lib、grpc++.lib、orientsec_common.lib、orientsec_consumer.lib、orientsec_provider.lib和orientsec_registry.lib    
(1) gpr项目配置  
配置属性--常规：    
![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.1.png)  

	输出目录：  
	$(SolutionDir)$(Platform)\$(Configuration)\  
	中间目录：  
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\  
	目标文件名：  
	$(ProjectName)    或者  gpr  
	配置属性—调试：  
	默认  
	配置属性—VC++ 目录：  
	默认  

	配置属性-- C/C++ --常规：  
	附加库包含目录：  
	$(SolutionDir)\..\third_party\zlib  
	$(SolutionDir)\..\include  
	$(SolutionDir)\..  
	$(SolutionDir)\..\third_party\boringssl\include  
	$(SolutionDir)\..\third_party\protobuf  
	$(SolutionDir)\..\vsprojects\third_party\zlib  
	$(SolutionDir)\..\third_party\benchmark\include  
	$(SolutionDir)\..\third_party\cares\cares  
	$(SolutionDir)\..\vsprojects\third_party\cares\cares  
	$(SolutionDir)\..\vsprojects\third_party\gflags\include  
	$(SolutionDir)\..\third_party\address_sorting\include  
	$(SolutionDir)\..\third_party\nanopb  
	调试信息格式：程序数据库 (/Zi)  
	警告等级： 等级3（/W3）  

	配置属性-- C/C++ --优化：  
	优化：已禁用 (/Od)  
	内联函数扩展：已禁用 (/Ob0)  

	配置属性-- C/C++ --预处理器：  

	预处理器定义（DEBUG）：  
	WIN32  
	_WINDOWS  
	PB_FIELD_16BIT  
	_WIN32_WINNT=0x600  
	_SCL_SECURE_NO_WARNINGS  
	_CRT_SECURE_NO_WARNINGS  
	_WINSOCK_DEPRECATED_NO_WARNINGS  
	CMAKE_INTDIR="Debug"  

	预处理器定义（Release）：  
	WIN32  
	_WINDOWS  
	NDEBUG  
	PB_FIELD_16BIT  
	_WIN32_WINNT=0x600  
	_SCL_SECURE_NO_WARNINGS  
	_CRT_SECURE_NO_WARNINGS  
	_WINSOCK_DEPRECATED_NO_WARNINGS  
	CMAKE_INTDIR="Release"  

	配置属性-- C/C++ --代码生成：  
	Debug version：  
	启用C++异常：是 (/EHsc)  
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)  
	运行库：多线程调试 DLL (/MDd)  
	Release version：  
	启用C++异常：是 (/EHsc)  
	基本运行时检查：默认值  
	运行库：多线程 DLL (/MD)  

	配置属性-- C/C++ --语言：  
	启用运行时类型信息：是 (/GR)  
	配置属性-- C/C++ --预编译头：  
	预编译头： 不使用预编译头  

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	目标计算机：MachineX64 (/MACHINE:X64)

	配置属性—库管理器 –所有选项：
	附加选项：%(AdditionalOptions) /machine:x64

(2) grpc项目配置  
配置属性--常规：  
 ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.2.png)  

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc

	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	$(SolutionDir)\..\third_party\zlib
	$(SolutionDir)\..\include
	$(SolutionDir)\..
	$(SolutionDir)\..\third_party\boringssl\include
	$(SolutionDir)\..\third_party\protobuf
	$(SolutionDir)\..\vsprojects\third_party\zlib
	$(SolutionDir)\..\third_party\benchmark\include
	$(SolutionDir)\..\third_party\cares\cares
	$(SolutionDir)\..\vsprojects\third_party\cares\cares
	$(SolutionDir)\..\vsprojects\third_party\gflags\include
	$(SolutionDir)\..\third_party\address_sorting\include
	$(SolutionDir)\..\third_party\nanopb
	$(SolutionDir)\..\third_party\orientsec\orientsec_common
	$(SolutionDir)\..\third_party\orientsec\orientsec_registry
	$(SolutionDir)\..\third_party\orientsec\orientsec_consumer
	$(SolutionDir)\..\third_party\orientsec\orientsec_provider

	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)
	内联函数扩展：已禁用 (/Ob0)

	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	WIN32
	_WINDOWS
	PB_FIELD_16BIT
	_WIN32_WINNT=0x600
	_SCL_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_WARNINGS
	_WINSOCK_DEPRECATED_NO_WARNINGS
	CMAKE_INTDIR="Debug"

	预处理器定义（Release）：
	WIN32
	_WINDOWS
	NDEBUG
	PB_FIELD_16BIT
	_WIN32_WINNT=0x600
	_SCL_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_WARNINGS
	_WINSOCK_DEPRECATED_NO_WARNINGS
	CMAKE_INTDIR="Release"

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：
	orientsec_registry.lib
	orientsec_common.lib
	orientsec_consumer.lib
	附加库目录：$(OutDir)
	目标计算机：MachineX64 (/MACHINE:X64)

	配置属性—库管理器 –所有选项：
	附加选项：%(AdditionalOptions) /machine:x64

(3) grpc++项目配置  
配置属性--常规：  
  ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.3.png)  

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc++

	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	$(SolutionDir)\..\third_party\zlib
	$(SolutionDir)\..\include
	$(SolutionDir)\..
	$(SolutionDir)\..\third_party\boringssl\include
	$(SolutionDir)\..\third_party\protobuf
	$(SolutionDir)\..\vsprojects\third_party\zlib
	$(SolutionDir)\..\third_party\benchmark\include
	$(SolutionDir)\..\third_party\cares\cares
	$(SolutionDir)\..\vsprojects\third_party\cares\cares
	$(SolutionDir)\..\vsprojects\third_party\gflags\include
	$(SolutionDir)\..\third_party\address_sorting\include
	$(SolutionDir)\..\third_party\nanopb
	$(SolutionDir)\..\third_party\orientsec\orientsec_common
	$(SolutionDir)\..\third_party\orientsec\orientsec_registry
	$(SolutionDir)\..\third_party\orientsec\orientsec_consumer
	$(SolutionDir)\..\third_party\orientsec\orientsec_provider

	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)
	内联函数扩展：已禁用 (/Ob0)

	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	WIN32
	_WINDOWS
	PB_FIELD_16BIT
	_WIN32_WINNT=0x600
	_SCL_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_WARNINGS
	_WINSOCK_DEPRECATED_NO_WARNINGS
	CMAKE_INTDIR="Debug"

	预处理器定义（Release）：
	WIN32
	_WINDOWS
	NDEBUG
	PB_FIELD_16BIT
	_WIN32_WINNT=0x600
	_SCL_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_WARNINGS
	_WINSOCK_DEPRECATED_NO_WARNINGS
	CMAKE_INTDIR="Release"

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：orientsec_provider.lib
	附加库目录：$(OutDir)
	目标计算机：MachineX64 (/MACHINE:X64)

	配置属性—库管理器 –所有选项：
	附加选项：%(AdditionalOptions) /machine:x64
(4) orientsec_common项目配置    
配置属性--常规：    
   ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.4.png)  

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc++
	配置类型：静态库(.lib)
	字符集： 使用Unicode字符集

	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	../../../include
	../../../
	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)
	内联函数扩展：已禁用 (/Ob0)

	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	_DEBUG
	_LIB
	WIN64
	_WINSOCK_DEPRECATED_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
	_CRT_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_DEPRECATE
	_WIN32_WINNT=0x600
	预处理器定义（Release）：
	_LIB
	WIN64
	_WINSOCK_DEPRECATED_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
	_CRT_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_DEPRECATE
	_WIN32_WINNT=0x600

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：gpr.lib;
	附加库目录：..\..\..\vsprojects\x64\Debug
	目标计算机：MachineX64 (/MACHINE:X64)

	配置属性—库管理器 –所有选项：
	附加选项：%(AdditionalOptions) /machine:x64

(5) orientsec_consumer项目配置   
配置属性--常规：    
  ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.5.png)  

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc++
	配置类型：静态库(.lib)
	字符集： 使用Unicode字符集

	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	../../../
	../../../include
	../../../../zookeeper/include
	../orientsec_common
	../orientsec_registry
	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)

	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	_DEBUG
	_CONSOLE
	_CRT_SECURE_NO_WARNINGS
	_WIN32_WINNT=0x600
	_WIN64
	预处理器定义（Release）：
	_CONSOLE
	_CRT_SECURE_NO_WARNINGS
	_WIN32_WINNT=0x600
	_WIN64

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	Debug Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：
	gpr.lib
	zookeeper_d.lib
	orientsec_common.lib
	附加库目录：
	..\..\..\vsprojects\x64\Debug
	..\..\..\..\zookeeper\libs\debug
	目标计算机：MachineX64 (/MACHINE:X64)

	Release Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：
	gpr.lib
	zookeeper.lib
	orientsec_common.lib
	附加库目录：
	..\..\..\vsprojects\x64\Release
	..\..\..\..\zookeeper\libs\release
	目标计算机：MachineX64 (/MACHINE:X64)

(6) orientsec_provider项目配置  
配置属性--常规：  
 ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.6.png)   

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc++
	配置类型：静态库(.lib)
	字符集： 使用Unicode字符集

	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	../../../
	../../../include
	../orientsec_common
	../orientsec_registry
	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)

	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	_DEBUG
	_LIB
	WIN64
	_WINSOCK_DEPRECATED_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
	_CRT_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_DEPRECATE
	_WIN32_WINNT=0x600
	预处理器定义（Release）：
	_LIB
	WIN64
	_WINSOCK_DEPRECATED_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE
	_CRT_SECURE_NO_WARNINGS
	_CRT_SECURE_NO_DEPRECATE
	_WIN32_WINNT=0x600

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	Debug Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：orientsec_registry.lib
	附加库目录：
	..\..\..\vsprojects\x64\Debug
	目标计算机：MachineX64 (/MACHINE:X64)

	Release Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：orientsec_registry.lib
	附加库目录：
	..\..\..\vsprojects\x64\Release
	目标计算机：MachineX64 (/MACHINE:X64)

(7) orientsec_registry项目配置  
配置属性--常规：  
  ![编译图](https://raw.githubusercontent.com/grpc-nebula/grpc-nebula/master/images/build4.2.1.7.png)  

	输出目录
	$(SolutionDir)$(Platform)\$(Configuration)\
	中间目录：
	$(MSBuildProjectName).dir\$(Platform)\$(Configuration)\
	目标文件名：
	$(ProjectName)    或者  grpc++
	配置类型：静态库(.lib)
	字符集： 使用Unicode字符集
	
	配置属性—调试：
	默认
	配置属性—VC++ 目录：
	默认

	配置属性-- C/C++ --常规：
	附加库包含目录：
	../../../include
	../../../../zookeeper/include
	../../../
	../orientsec_common
	调试信息格式：程序数据库 (/Zi)
	警告等级： 等级3（/W3）

	配置属性-- C/C++ --优化：
	优化：已禁用 (/Od)
	
	配置属性-- C/C++ --预处理器：

	预处理器定义（DEBUG）：
	_DEBUG
	_LIB
	WIN64
	_WIN32_WINNT=0x600
	_CRT_SECURE_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE

	预处理器定义（Release）：
	_LIB
	WIN64
	_WIN32_WINNT=0x600
	_CRT_SECURE_NO_WARNINGS
	_CRT_NONSTDC_NO_DEPRECATE

	配置属性-- C/C++ --代码生成：
	Debug version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：两者(/RTC1，等同于 /RTCsu) (/RTC1)
	运行库：多线程调试 DLL (/MDd)
	Release version：
	启用C++异常：是 (/EHsc)
	基本运行时检查：默认值
	运行库：多线程 DLL (/MD)

	配置属性-- C/C++ --语言：
	启用运行时类型信息：是 (/GR)
	配置属性-- C/C++ --预编译头：
	预编译头： 不使用预编译头

	配置属性-- C/C++ --输出文件：
	ASM列表位置： $(IntDir)
	对象文件名： $(IntDir)
	程序数据库文件名：$(IntDir)$(ProjectName).pdb

	配置属性-- C/C++ --浏览信息：
	默认
	配置属性-- C/C++ --高级：
	编译为：编译为 C++ 代码 (/TP)
	禁用特定警告：4065;4506;4200;4291;4244;4267;4987;4774;4819;4996;4619

	配置属性—库管理器 –常规：
	Debug Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：
	gpr.lib
	zookeeper_d.lib
	orientsec_common.lib
	附加库目录：
	..\..\..\vsprojects\x64\Debug
	..\..\..\..\zookeeper\libs\debug
	目标计算机：MachineX64 (/MACHINE:X64)

	Release Version：
	输出文件：$(OutDir)$(TargetName)$(TargetExt)
	附加依赖库：
	gpr.lib
	zookeeper.lib
	orientsec_common.lib
	附加库目录：
	..\..\..\vsprojects\x64\Release
	..\..\..\..\zookeeper\libs\release
	目标计算机：MachineX64 (/MACHINE:X64)

### 4.2.2 grpc-c库编译
右键解决方案-->生成解决方法（F7）
### 4.2.3 生成lib

目录grpc\vsprojects\x64\Debug：

	gpr.lib
	grpc.lib
	grpc++.lib
	orientsec_common.lib
	orientsec_consumer.lib
	orientsec_provider.lib
	orientsec_registry.lib










