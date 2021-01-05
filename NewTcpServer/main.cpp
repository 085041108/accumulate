#include <QCoreApplication>

#include "MyTcpServer.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    MyTcpServer server;
    server.listen(QHostAddress::Any,2000);
    qDebug() << "Listening 2000";
    //QThread::sleep(1000);

    //qDebug() << "Connection close";
    //server.close();
    return a.exec();
}
