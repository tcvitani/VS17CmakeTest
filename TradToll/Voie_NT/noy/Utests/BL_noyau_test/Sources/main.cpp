
#include <QtGui>
#include <QtTest/QtTest>
#include "TestBAL_Process.h"
#include "TestMEM.h"


//QTEST_MAIN(TestBAL_Process);
//QTEST_MAIN(TestMEM);


 int main(int argc, char *argv[])
 {
 
 // 	 TestMEM testMEM;
 // 	 QTest::qExec(&testMEM);
 
      TestBAL_Process testProcesses;
 	 QTest::qExec(&testProcesses);
 
 	 
 	return 0;
 }


