/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CMDSVC_tool												 */
/* FILE:     MGlobalStructs.h												 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMDSVC_GLOB_STRUCTS_H
#define CMDSVC_GLOB_STRUCTS_H

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QPushButton>
#include <QLabel>
#include <QString>
#include <QByteArray>
#include <QDateTime>

#include <windows.h>
#include <iphlpapi.h>

extern "C"
{


};


enum enumACOMInstanceType
{
	enuACOM_PIPE_CLIENT = 0,
	enuACOM_TCP_CLIENT,
};


struct _ServiceCmdData
{
	QString sMachineName;
	QString sServiceName;
	QString sDisplayName;
	QString sInstanceFileName;
	enumACOMInstanceType eInstanceType;
	DWORD dwPort;
};


/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#include <undef.h>
#endif

/*-------------------------------- END OF FILE ------------------------------*/