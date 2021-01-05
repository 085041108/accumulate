@echo off
:::::::::::::: 参数设置::::::::::::::
set from=http://mail.gwm.cn/
set user=GW00180107
set pass=as248035
set to=815705386@qq.com
set subj=测试
set mail=restart_service.txt
set attach=restart_service.txt
set server=smtp.163.com
set debug=-debug -log blat.log -timestamp
::::::::::::::::: 运行blat :::::::::::::::::
blat %mail% -to %to% -base64 -charset Gb2312 -subject %subj% -attach %attach% -server %server% -f %from% -u %user% -pw %pass% %debeg%