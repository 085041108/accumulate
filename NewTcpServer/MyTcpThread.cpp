#include "MyTcpThread.h"

MyTcpThread::MyTcpThread(qintptr socketDescriptor) : QThread()
{
    m_qint_socket_descriptor = socketDescriptor;

    m_p_file = new QFile("out.txt");
    if (!m_p_file->open(QIODevice::WriteOnly | QIODevice::Text))
       return;

    m_out =new QTextStream(m_p_file);
}

void MyTcpThread::run() //重写QThread虚函数 run()
{
    QTcpSocket * p_socket = new QTcpSocket;
    m_p_socket = p_socket;
    m_p_socket->setSocketDescriptor(m_qint_socket_descriptor);

    QHostAddress clientaddr = m_p_socket->peerAddress(); //获得IP
    int port = m_p_socket->peerPort();   //获得端口号
    m_qstr_ip = QString(clientaddr.toString());
    m_qstr_port = QString::number(port);
    m_qstr_threadid = QString::number(quintptr(QThread::currentThreadId()));
    qDebug()<<"Current thread is "<<m_qstr_threadid;

    while(m_p_socket->state() && QAbstractSocket::ConnectedState)
    {
        if (m_p_socket->waitForReadyRead(1000)){//如果有新数据可供阅读，则立即返回true，否则错误或超出最大延迟则返回false
            QByteArray array = m_p_socket->readAll();    //接收消息

            QDateTime datetime = QDateTime::currentDateTime();
            QString sendMessage = tr("recv from :") + m_qstr_ip + tr(":") \
                       + m_qstr_port + tr("   ") + datetime.toString("yyyy-M-dd hh:mm:ss");
            sendMessage += QString::fromLocal8Bit(array);//QString::fromStdString(array.toStdString());
            qDebug()<<sendMessage;
            (*m_out) << sendMessage;
            // 编码规则测试
            //sendMessage = QString::fromLocal8Bit("乱码吗");
            m_p_socket->write(sendMessage.toLocal8Bit());
        }
    }
}

MyTcpThread::~MyTcpThread()
{
    m_p_socket->close();
    delete m_p_socket;
}
