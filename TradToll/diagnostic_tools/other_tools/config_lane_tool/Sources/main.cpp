/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
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

#include "MDefines.h"
#include "MGlobalStructs.h"
#include "MTracer.h"
#include "MHelpFuncs.h"
#include "MCfgToolConfig.h"
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
	int iRet=0;
	QApplication app(argc, argv);
	
	QFont f=app.font();
	f.setStyleStrategy(QFont::PreferAntialias);
	app.setFont(f);

	QString sName = MHelpFuncs::getProcessFileName();
	MTracer::Init(QString(), QString(), sName, 1, false, true);
	
	MCfgLaneToolConfig * pCfg = new MCfgLaneToolConfig(TOOL_GEN_SETTINGS_INI_FILE_KEY);
	pCfg->parseCmdlineArgs();
	
	if (!pCfg->loadConfig())
	{
		QMessageBox::critical(NULL, 
			sName,
			QString("Error initializing application! Verify %1 file for details.").arg(MTracer::getErrFilePath()));
	}
	else
	{
 		MTracer::Deinit();
  		MTracer::Init(pCfg->getTrcFilePath(),
 					pCfg->getErrFilePath(),
  					pCfg->getFilePrefix(), 
  					pCfg->getTraceMaxMB(),
  					pCfg->isTrcEnabled(),
 					pCfg->isErrEnabled());

		MMainDialog thr;
		thr.show();

		iRet = app.exec();
	}

	delete pCfg;
	MTracer::Deinit();

	return iRet;
}

/*-------------------------------- END OF FILE ------------------------------*/
