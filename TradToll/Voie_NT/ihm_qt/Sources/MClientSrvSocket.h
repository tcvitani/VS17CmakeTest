/****************************************************************************
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** Licensees holding a valid Qt License Agreement may use this file in
** accordance with the rights, responsibilities and obligations
** contained therein.  Please consult your licensing agreement or
** contact sales@trolltech.com if any conditions of this licensing
** agreement are not clear to you.
**
** Further information about Qt licensing is available at:
** http://www.trolltech.com/products/qt/licensing.html or by
** contacting info@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MClientSrvSocket_H
#define MClientSrvSocket_H

#include <QTcpSocket>
#include <QTimer>


class MClientSrvSocket : public QTcpSocket
{
    Q_OBJECT

public:
    MClientSrvSocket(QObject *parent = 0);
    ~MClientSrvSocket();

    void Initialize();

signals:
	void receivedData(const QByteArray &);
	void message(const QString &);

public slots:
	void sendData(const QString &sOutData);

private slots:
	void onConnected();
	void onError( QAbstractSocket::SocketError socketError);
	void onReadyRead();
	
private:

};


#endif
