#ifndef M_IHM_HTTP_SERVER_H
#define M_IHM_HTTP_SERVER_H


#include <QTcpServer>

//A class that implements the simple HTTP server.
class MIhmHttpServer : public QTcpServer
{
    Q_OBJECT
public:
    MIhmHttpServer(quint16 port, QObject* parent = 0);
    ~MIhmHttpServer();

signals:
	void newConnection(int);

public slots:
	void stop();
	virtual void incomingConnection (int socketDescriptor);

private:


};

#endif
