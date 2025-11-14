

 #include <QTimer>
 #include <QTime>
 #include <QString>
 #include "MDigitalClock.h"

 MDigitalClock::MDigitalClock(QWidget *parent)
     : QLabel(parent)
 {
     m_sTimeFormat = "hh:mm:ss";
     connect(&m_timer, SIGNAL(timeout()), this, SLOT(showTime()));
     m_timer.start(1000);

     showTime();
 }

 MDigitalClock::~MDigitalClock()
 {
	m_timer.stop();
 }

 
 void MDigitalClock::showTime()
 {
     QTime time = QTime::currentTime();
     QString text = time.toString(m_sTimeFormat);
     
	 QDate dtNew = QDate::currentDate();

	 if(dtNew!=m_dtOldDate)
		 emit dateChange();

	 m_dtOldDate = dtNew;

     this->setText(text);
 }