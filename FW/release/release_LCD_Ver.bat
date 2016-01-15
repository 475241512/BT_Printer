@echo off

@echo 请输入当前发布版本的次版本号(形如1.0.0)：
@set/p version_num= >nul

rem 编译V1
echo 正在生成V2版.....
::release_V1_chs.bat
::先调用相应版本的工程配置文件
del ..\src\Project\EWARMv5\T6.ewp
copy ..\src\Project\EWARMv5\T6_ST_LCD_release.ewp ..\src\Project\EWARMv5\T6.ewp


::调用IAR集成环境的工具对源码进行编译，此处也可以将此工具直接添加到环境变量
@echo off
echo 正在进行工程编译...
IarBuild.exe ..\src\Project\EWARMv5\T6.ewp -build Debug -log all

::将工程生成的文件copy到Release目录
copy ..\src\Project\EWARMv5\Debug\Exe\T6.bin .\

::对生产的BIN文件加密处理,采取KT_Xor的加密方式
oslib_syspack.exe T6.bin 1

::写入版本信息
echo %date:~0,4%-%date:~5,2%-%date:~8,2%
Pack_ext.exe update.bin HJBTPrinter V%version_num% %date:~0,4%-%date:~5,2%-%date:~8,2%

::转换为Upgrade.exe下载工具可以下载的文件格式

::转换Update.bin 
File_Convertor.exe update.bin Upgrade.bin 2

::转换resdata.bin 
File_Convertor.exe resdata.bin res_Upgrade.bin 3

rem 将需要发布的文件进行压缩
echo 正在打包发布文件...
set packetname=HJ58B(LCD版本)_生产V"%version_num%"-"%date:~0,4%%date:~5,2%%date:~8,2%".rar
rar a -as -ep1 "%packetname%" "%cd%\HJ58B_BootCode.bin" "%cd%\Upgrade.bin" "%cd%\res_Upgrade.bin" "%cd%\Version.txt" "%cd%\升级说明.txt"

copy %packetname% version\%packetname%

set packetname=HJ58B(LCD版本)_用户V"%version_num%"-"%date:~0,4%%date:~5,2%%date:~8,2%".rar
rar a -as -ep1 "%packetname%" "%cd%\Upgrade.bin" "%cd%\Version.txt" "%cd%\升级说明.txt"

copy %packetname% version\%packetname%


del update.bin
del T6.bin
del res_Upgrade.bin
del Upgrade.bin


::del %packetname%
del *.rar

pause