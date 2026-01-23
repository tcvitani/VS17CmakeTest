/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:     MRegUtils.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CFG_LANE_TOOL_REG_UTILS_H
#define CFG_LANE_TOOL_REG_UTILS_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/*-------------------------------- INCLUDES:  -------------------------------*/

#include <QString>
#include <windows.h>

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MRegUtils
{
	public:
		static const int SUCCESS;
		static const int FAIL;

		MRegUtils();
		virtual ~MRegUtils();

		static int updateStringKey(char *node, char *key, char *newValue);
		static int updateDwordKey(char *node, char *key, DWORD newValue);

		static QString readStringKey(char *node, char *key, int *status);
		static uint readDwordKey(char *node, char *key, int *status);
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/
