/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	CMDSVC_tool													 */
/* FILE:	 main.cpp														 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QApplication>
#include <QMessageBox>
#include <QTextCodec.h>
#include <QString.h>
#include <QSettings>
#include <QFile>


#include "MTracer.h"
#include "MHelpFuncs.h"
#include "MMainDialog.h"

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

/*---------------------------------- CODE: ----------------------------------*/



int main(int argc, char *argv[])
{
	int iRet = 0;
	QApplication app(argc, argv);

	QString sName = MHelpFuncs::getProcessFileName();
	MTracer::Init(QString(), QString(), sName, 1, true, true);

	MMainDialog thr;
	thr.show();

	//--------------
	iRet = app.exec();
	//--------------

	MTracer::Deinit();


	return iRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
