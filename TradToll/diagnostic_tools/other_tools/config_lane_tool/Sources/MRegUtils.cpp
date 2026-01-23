/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 congif_lane_tool												 */
/* FILE:	 MRegUtils.cpp													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include "MRegUtils.h"

extern "C"
{
	#include <reg.h>
};

/*-------------------------------- RESERVED:  -------------------------------*/


/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

const int MRegUtils::SUCCESS = 0;
const int MRegUtils::FAIL = 1;

/*---------------------------------- CODE: ----------------------------------*/

MRegUtils::MRegUtils()
{

}

MRegUtils::~MRegUtils()
{

}

int MRegUtils::updateStringKey(char *node, char *key, char *newValue)
{
	DWORD					dwLen;
	char					stringValue[MAX_PATH + 1];

    dwLen = sizeof(stringValue);
	if(REG_Lire_Chaine(HKEY_LOCAL_MACHINE, node, key, stringValue, &dwLen) != ERROR_SUCCESS)
	{
		// safe value to force creation of the key
		stringValue[0] = '\0';
	}

	if(strcmp(newValue, stringValue))
	{
		if (REG_Ecrire_Chaine(HKEY_LOCAL_MACHINE, node, key, newValue)  != ERROR_SUCCESS)
			return FAIL;
	}

	return SUCCESS;
}

int MRegUtils::updateDwordKey(char *node, char *key, DWORD newValue)
{
	DWORD oldValue = 0;
	bool isNonExistent = false;
	
	if( REG_Lire_Entier( HKEY_LOCAL_MACHINE, node, key, &oldValue) != ERROR_SUCCESS)
	{
		isNonExistent = true;
	}

	if( newValue != oldValue || isNonExistent)
	{
		if(REG_Ecrire_Entier(HKEY_LOCAL_MACHINE, node, key, newValue)  != ERROR_SUCCESS)
			return FAIL;
	}

	return SUCCESS;
}

QString MRegUtils::readStringKey(char *node, char *key, int *status)
{
	DWORD					dwLen;
	char					stringValue[MAX_PATH + 1];

    dwLen = sizeof(stringValue);
	if(REG_Lire_Chaine(HKEY_LOCAL_MACHINE, node, key, stringValue, &dwLen) != ERROR_SUCCESS)
	{
		*status = FAIL;
		return QString();
	}

	*status = SUCCESS;
	return QString::fromLocal8Bit(stringValue);
}

uint MRegUtils::readDwordKey(char *node, char *key, int *status)
{
	DWORD readValue = 0;

	if( REG_Lire_Entier( HKEY_LOCAL_MACHINE, node, key, &readValue) != ERROR_SUCCESS)
	{
		*status = FAIL;
		return 0;
	}

	*status = SUCCESS;
	return readValue;
}

/*-------------------------------- END OF FILE ------------------------------*/
