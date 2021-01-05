#ifndef MYTCPTHREAD_H
#define MYTCPTHREAD_H

#include <QThread>
#include <QTcpSocket>
#include <QTcpServer>
#include <QDebug>
#include <QDateTime>
#include <QFile>

//每一个新的客户端连接，都会由QTcpServer通过重写的incomingConnection()自动分配一个新的线程。
//线程只接受了一个MyTcpThread提供的socketDescriptor。
//MyTcpThread类的setSocketDescriptor()函数接收这个socketDescriptor，并利用它完成与客户端的连接。
class MyTcpThread : public QThread
{
private:
    QTcpSocket * m_p_socket;
    qintptr m_qint_socket_descriptor;
public:
    MyTcpThread(qintptr socketDescriptor);
    void run(); //重写QThread虚函数 run()
    ~MyTcpThread();
public:
    QString m_qstr_ip;
    QString m_qstr_port;
    QString m_qstr_threadid;
    QFile * m_p_file;
    QTextStream * m_out;
};

#endif // MYTCPTHREAD_H
