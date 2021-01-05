#include "MyTcpServer.h"

MyTcpServer::MyTcpServer(QObject * parent) : QTcpServer(parent)
{
    if(!m_qlist_mytcpthread.isEmpty())
    {
        m_qlist_mytcpthread.clear();
       qDebug()<< "The thread list is not empty,please check!";
    }
    // 启动定时查询线程状态
    m_p_qtimer = new QTimer(this);
    connect(m_p_qtimer, SIGNAL(timeout()), this, SLOT(slotCheckThread()));
    m_p_qtimer->start(TIMER_TIMEOUT);
    qDebug()<< "Construct MyTcpServer!";
}

MyTcpServer::~MyTcpServer()
{
    // 销毁所有线程
    while (!m_qlist_mytcpthread.isEmpty())
    {
        MyTcpThread* thread = m_qlist_mytcpthread.takeFirst();
        qDebug()<<"Destroy thread "<<thread->m_qstr_threadid<<" ip is "<<
                  thread->m_qstr_ip<<" port is "<<thread->m_qstr_port;
        thread->quit();
        thread->wait();
        delete thread;
    }

    qDebug()<< "Destroy MyTcpServer!";
}

void MyTcpServer::sendData()
{

}

void MyTcpServer::slotDisplayError(QAbstractSocket::SocketError socketError)
{
    qDebug()<< "Error is " <<socketError;
}

void MyTcpServer::slotDisconnect()
{
    qDebug()<<"Disconnect the client!";
}

void MyTcpServer::incomingConnection(qintptr socketDescriptor)
{
    //qDebug()<<"incoming incomingconnection!";
    //将socketDescriptor传入新创建的线程对象，线程启动后QTcpSocket会使用socketDescriptor
    MyTcpThread * thread = new MyTcpThread(socketDescriptor);

    //保存新创建的线程对象指针到链表
    m_qlist_mytcpthread.append(thread);

    //启动线程 自动会执行run()函数
    thread->start();
}

void MyTcpServer::slotCheckThread()
{

    MyTcpThread * tcp_thread;
    foreach(tcp_thread, m_qlist_mytcpthread)
    {
        if(!tcp_thread->isRunning())
        {
          qDebug()<<"Thread is not running,Destroy thread "<<tcp_thread->m_qstr_threadid<<" ip is "<<
                     tcp_thread->m_qstr_ip<<" port is "<<tcp_thread->m_qstr_port;
          m_qlist_mytcpthread.removeOne(tcp_thread);
          MyTcpThread *thread = tcp_thread;
          delete thread;
          thread = nullptr;
        }
    }
}
