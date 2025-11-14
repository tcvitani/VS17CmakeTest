#ifndef MCLOCK_H
#define MCLOCK_H

#include <QObject>
#include <QDate>
#include <QString>
#include <QTimer>


class MClock : public QObject
{
 Q_OBJECT

public:
	MClock(QObject *parent = 0);
	~MClock();

	void start();
	
signals:
	void dateChange();
		
private slots:
	 void onTimeout();

private:
	QDate m_dtOldDate;
	QTimer timer;
};
 
#endif


