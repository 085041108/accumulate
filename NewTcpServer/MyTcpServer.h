#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include <QTcpServer>
#include <QList>
#include <QTimer>

#include "MyTcpThread.h"

#define TIMER_TIMEOUT   (5*1000)

class MyTcpServer : public QTcpServer
{
  Q_OBJECT
public:
    MyTcpServer(QObject * parent = 0);
    ~MyTcpServer() override;
public:
    void sendData();
private slots:
    void slotDisplayError(QAbstractSocket::SocketError socketError);
    void slotDisconnect();
    void slotCheckThread();
protected:
    void incomingConnection(qintptr socketDescriptor) override;
private:
    QList<MyTcpThread*> m_qlist_mytcpthread;
    QTimer* m_p_qtimer;
};
#endif // MYTCPSERVER_H
