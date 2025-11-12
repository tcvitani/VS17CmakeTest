#ifndef NOY_TEST_MEM_H
#define NOY_TEST_MEM_H

#include <QObject.h>
#include <QProcess.h>
#include "noyau.h"


class TestMEM : public QObject
{
    Q_OBJECT
private slots:
	void initTestCase();

  	void testMemAlloc();

	void cleanupTestCase();

private:	
	DWORD doTestMEM();
	QList <QProcess *> lstConsumerProcess;

};

#endif