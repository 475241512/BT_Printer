@echo off

@echo �����뵱ǰ�����汾�Ĵΰ汾��(����1.0.0)��
@set/p version_num= >nul

rem ����V1
echo ��������V2��.....
::release_V1_chs.bat
::�ȵ�����Ӧ�汾�Ĺ��������ļ�
del ..\src\Project\EWARMv5\T6.ewp
copy ..\src\Project\EWARMv5\T6_ST_LCD_release.ewp ..\src\Project\EWARMv5\T6.ewp


::����IAR���ɻ����Ĺ��߶�Դ����б��룬�˴�Ҳ���Խ��˹���ֱ����ӵ���������
@echo off
echo ���ڽ��й��̱���...
IarBuild.exe ..\src\Project\EWARMv5\T6.ewp -build Debug -log all

::���������ɵ��ļ�copy��ReleaseĿ¼
copy ..\src\Project\EWARMv5\Debug\Exe\T6.bin .\

::��������BIN�ļ����ܴ���,��ȡKT_Xor�ļ��ܷ�ʽ
oslib_syspack.exe T6.bin 1

::д��汾��Ϣ
echo %date:~0,4%-%date:~5,2%-%date:~8,2%
Pack_ext.exe update.bin HJBTPrinter V%version_num% %date:~0,4%-%date:~5,2%-%date:~8,2%

::ת��ΪUpgrade.exe���ع��߿������ص��ļ���ʽ

::ת��Update.bin 
File_Convertor.exe update.bin Upgrade.bin 2

::ת��resdata.bin 
File_Convertor.exe resdata.bin res_Upgrade.bin 3

rem ����Ҫ�������ļ�����ѹ��
echo ���ڴ�������ļ�...
set packetname=HJ58B(LCD�汾)_����V"%version_num%"-"%date:~0,4%%date:~5,2%%date:~8,2%".rar
rar a -as -ep1 "%packetname%" "%cd%\HJ58B_BootCode.bin" "%cd%\Upgrade.bin" "%cd%\res_Upgrade.bin" "%cd%\Version.txt" "%cd%\����˵��.txt"

copy %packetname% version\%packetname%

set packetname=HJ58B(LCD�汾)_�û�V"%version_num%"-"%date:~0,4%%date:~5,2%%date:~8,2%".rar
rar a -as -ep1 "%packetname%" "%cd%\Upgrade.bin" "%cd%\Version.txt" "%cd%\����˵��.txt"

copy %packetname% version\%packetname%


del update.bin
del T6.bin
del res_Upgrade.bin
del Upgrade.bin


::del %packetname%
del *.rar

pause