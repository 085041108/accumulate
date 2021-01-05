@echo off
title "数据中间件服务(windows窗体)"守护进程，发布服务时请临时关闭此进程
rem 定义循环间隔时间和监测的服务：
set secs=10
set exename="testLibrdkafka.exe"
set exepath="E:\\sourcedemo\\testLibrdkafka\\Debug\\testLibrdkafka.exe"
set showname="数据中间件服务"

echo.
echo ========================================
echo ==     查询计算机服务的状态          ==
echo ==     每间隔%secs%秒种进行一次查询  ==
echo ==     如发现其停止，则立即启动      ==
echo ========================================
echo.
echo 此脚本监测的服务是：%showname%
echo.

if %exename%. == . goto end
:chkit
set svrst=0
for /f %%i in ('tasklist') do if /I "%%i" == %exename% set svrst=1
if %svrst% == 0 (
	echo win程序重新启动于 %time% ,请检查系统日志 >> restart_service.txt
	start "" %exepath%
	)
set svrst=
rem 下面的命令用于延时，否则可能会导致cpu单个核心满载。
ping -n %secs% 127.0.0.1 > nul
goto chkit
:end