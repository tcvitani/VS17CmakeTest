/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_PV_HEADER.C                                                */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*              	:	Creation of file for project                         */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/
 
#include <msg_pv_header.h>
#include <stdio.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
 
/*-------------------------------- DEFINES:   -------------------------------*/


/*-------------------------------- TYPEDEFS:  -------------------------------*/


/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- CODE:      -------------------------------*/

// ENREGISTREMENT / RECHERCHE

PRIVATE BOOL StringToLong( unsigned char * szStr, int iLen, LONG * plValue );

// la fonction de comparaison extrait du buffer le champ "cd"
PUBLIC BOOL MSG_PV_HEADER_Compare_CD (BYTE *p_msg, LONG Const)
{
    LONG cd;

	if ( ! StringToLong( p_msg, 3, NULL ) )
		return FALSE;

	if ( ! StringToLong( p_msg + 3, 3, &cd ) )
		return FALSE;

    // verification par rapport a MSG_PV_HEADER_CD
    return cd == Const;
}

PRIVATE BOOL StringToLong( unsigned char * szStr, int iLen, LONG * plValue )
{
	BOOL fCvrt = FALSE;	// Flag indiquant qu'au moins un digit a été traité

	if ( plValue )
		*plValue = 0;

	//
	// Ignorer les premiers espaces et autres caractères
	// de contrôle (tab, saut de ligne etc.) dans la limite
	// de la taille du buffer.
	//
	while ( iLen > 0 )
		if ( *szStr == 0 )
			break;
		else if ( *szStr <= ' ' )
		{
			szStr ++;
			iLen --;
		}
		else
			break;

	//
	// Tant qu'on a des digits, on les traite (dans la limite
	// de la taille du buffer).
	//
	while ( iLen > 0 )
		if ( *szStr == 0 )
			break;
		else if ( isdigit( *szStr) )
		{
			fCvrt = TRUE;
			if ( plValue ) 
				*plValue = ( *plValue * 10 ) + ((LONG)(*szStr)) - (LONG)'0';
			szStr++;
			iLen --;
		}
		else break;

    //
	// On s'assure que s'il reste des caractères dans le buffer, 
	// ceux-ci sont également convertis
	//
	while ( iLen > 0 )
		if ( *szStr == 0 )
			break;
		else if ( *szStr <= ' ' )
		{
			szStr ++;
			iLen --;
		}
		else
			break;

	//
	// Si on est arrivé au bout du buffer et qu'au moins
	// un digit a été traité, la conversion est bonne.
	// Elle est également OK si un au moins un digit a été
	// traité et qu'on a atteint la fin de la chaine avant
	// la fin du buffer.
	//
	if ( iLen == 0 )
		return fCvrt;
	else
		return ( fCvrt && ( *szStr == 0 ) );
}

/*-------------------------------- END OF FILE ------------------------------*/

