from threading import Timer
import subprocess
import datetime
import os
import configparser
import smtplib
from email.mime.text import MIMEText

inifile = "E:\\sourcedemo\\daemon\\config.ini"

def main():
    global path_exe
    try:
        path_exe = get_config(inifile, "file", "path")
        timer = Timer(20, restart_process, ('start "" "{0}"'.format(path_exe),))
        timer.start()
    except Exception:
        print("Please check your ini file,path is {0}".format(inifile))

def send_mail(body):
    mail_host = path_exe = get_config(inifile, "mail", "server")
    mail_user = path_exe = get_config(inifile, "mail", "user")
    mail_pass = path_exe = get_config(inifile, "mail", "password")
    sender = path_exe = get_config(inifile, "mail", "sender")
    receivers = path_exe = get_config(inifile, "mail", "receivers")
    
    msg = MIMEText(body, 'html')
    msg['subject'] = 'Hello world'
    msg['from'] = "Your friend<" + sender + ">"
    msg['to'] = "My dear!<" + receivers + ">"
    
    print("*"*45)
    print(msg)
    print("*"*45)
    print(mail_host + " " + mail_pass + " " + mail_user)
    try:
        smtpObj = smtplib.SMTP() 
        smtpObj.connect(mail_host, 25)    # 25 为 SMTP 端口号
        smtpObj.login(mail_user,mail_pass)  
        smtpObj.sendmail(sender, receivers, msg.as_string())
        print("邮件发送成功")
        return True
    except smtplib.SMTPException as e:
        print ("Error: 无法发送邮件,error is ::{0}".format(e))
        return False
    
def get_config(file, section, key):
    # create the object of file
    con = configparser.ConfigParser()
    # read file
    con.read(file, encoding='utf-8')
    # get all the section
    sections = con.sections() #['url', 'email']
    # get the section
    items = con.items(section) 
    items = dict(items)
    #print(items)
    #print(items[key])
    path = None
    if len(items[key]) > 0:
        path = items[key]
    return path

def restart_process(process_name):
    red = subprocess.Popen('tasklist', stdout=subprocess.PIPE, stderr=subprocess.PIPE, shell=True)
    tasklist_str = red.stdout.read().decode(encoding='gbk')
    re_path = process_name.split("\\")[-1]
    re_path =re_path.split('"')[0]  # delete the " 
    formattime = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')
    
    if re_path not in tasklist_str:
        #print("-"*50)
        #print(re_path)
        #print(tasklist_str)
        #print("-"*50)

        global count
        count += 1
        print(formattime + '第' + str(count) + '次检测发现异常重连')
        os.system(process_name)
        name_exe = path_exe[path_exe.rindex("\\")+1:-1]
        send_mail(formattime + ':监视程序:' + name_exe + '未运行，启动程序!')
        print('yes,connected')
    else:
        global error_count
        error_count += 1
        print(formattime + '第' + str(error_count) + '次检测正在运行中')
    global timer
    timer = Timer(20, restart_process, ('start "" "{0}"'.format(path_exe),))
    timer.start()

count = 0
error_count = 0
main()
#send_mail("whether do test the mail module!")
