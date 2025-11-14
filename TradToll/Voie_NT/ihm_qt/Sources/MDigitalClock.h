#ifndef MDIGITALCLOCK_H
#define MDIGITALCLOCK_H

#include <QLabel>
#include <QDate>
#include <QString>
#include <QTimer>

class MDigitalClock : public QLabel
{
 Q_OBJECT

public:
	MDigitalClock(QWidget *parent = 0);
	~MDigitalClock();

	void setTimeFormat(QString sTimeFormat)
					{m_sTimeFormat = sTimeFormat;};
signals:
	void dateChange();
	
private slots:
	void showTime();
	
private:
	QDate m_dtOldDate;
	QString m_sTimeFormat;
	QTimer m_timer;
};
 
#endif


