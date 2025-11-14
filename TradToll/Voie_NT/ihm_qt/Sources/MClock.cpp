

 #include <QTimer>
 #include <QTime>
 #include "MClock.h"

 MClock::MClock(QObject *parent)
     : QObject(parent)
 {
     connect(&timer, SIGNAL(timeout()), this, SLOT(onTimeout()));

 }



 MClock::~MClock()
 {
	timer.stop();
 }

 void MClock::start()
 {
     timer.start(2000);
     onTimeout();
 }

 void MClock::onTimeout()
 {
	 QDate dtNew = QDate::currentDate();

	 if(dtNew!=m_dtOldDate)
		 emit dateChange();

	 m_dtOldDate = dtNew;

 }