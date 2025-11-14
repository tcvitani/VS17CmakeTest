#ifndef MIHM_ANI_WD_THREAD_H
#define MIHM_ANI_WD_THREAD_H

#include <QThread>


class MIhmWDThread:public QThread
{
    Q_OBJECT
public:
	MIhmWDThread();
	~MIhmWDThread();
	
	void run();	
signals:
	void exitProgram();
};


#endif


