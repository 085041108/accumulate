@echo off
title "�����м������(windows����)"�ػ����̣���������ʱ����ʱ�رմ˽���
rem ����ѭ�����ʱ��ͼ��ķ���
set secs=10
set exename="testLibrdkafka.exe"
set exepath="E:\\sourcedemo\\testLibrdkafka\\Debug\\testLibrdkafka.exe"
set showname="�����м������"

echo.
echo ========================================
echo ==     ��ѯ����������״̬          ==
echo ==     ÿ���%secs%���ֽ���һ�β�ѯ  ==
echo ==     �緢����ֹͣ������������      ==
echo ========================================
echo.
echo �˽ű����ķ����ǣ�%showname%
echo.

if %exename%. == . goto end
:chkit
set svrst=0
for /f %%i in ('tasklist') do if /I "%%i" == %exename% set svrst=1
if %svrst% == 0 (
	echo win�������������� %time% ,����ϵͳ��־ >> restart_service.txt
	start "" %exepath%
	)
set svrst=
rem ���������������ʱ��������ܻᵼ��cpu�����������ء�
ping -n %secs% 127.0.0.1 > nul
goto chkit
:end