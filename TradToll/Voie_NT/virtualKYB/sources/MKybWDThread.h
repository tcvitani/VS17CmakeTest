#ifndef MKyb_ANI_WD_THREAD_H
#define MKyb_ANI_WD_THREAD_H

#include <QThread>


class MKybWDThread:public QThread
{
    Q_OBJECT
public:
	MKybWDThread();
	~MKybWDThread();
	
	void run();	
signals:
	void exitProgram();
};


#endif


