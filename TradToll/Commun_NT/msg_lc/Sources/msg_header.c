/*****************(v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     MSG_LC_HEADER.C												 */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <msg_lc_header.h>
#include <stdio.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include "memclass.h"
   
/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/ 

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- CODE:      -------------------------------*/
// DECLARATION STRUCTURELLE ET COMPORTEMENTALE


// MACRO nécessaires aux calculs des offset par MSG_OFFSET et a l'initialisation
// locale de structures de type union MSG_Field[] par MSG_INIT_REF
MSG_DECLARE_REF(MSG_HEADER, MSG_HEADER_NB_FIELDS + 1)

// on utilise une fonction pour pouvoir faire des déclarations de tableaux initialisés
// avec des données "variables" pour le compilateur(champs utilisant MSG_OFFSET)
// Impossible a faire sur une variable initialisée globale(erreur de compilation)
// Meilleure lisibilité que l'initialisation par affectation champ par champ
PROTECTED void MSG_HEADER_Init(void)
{
    // suppression(localisée) du "warning C4047: 'initializing' : 'void *' differs in levels of indirection from XXX"
    // causé par l'initialisation d'une structure d'unions(?) avec des champs anonymes de type PVOID
    // rem : les types utilisés lors de l'initialisation d'une union sont ceux du premier champ de cette union(ici  struct MSG_Field_Stop)
    #pragma warning( push )
    #pragma warning( disable : 4047 )  

    // Attention pas de déclaration CONST du cd !
    union MSG_Field LOCAL_MSG_HEADER[MSG_HEADER_NB_FIELDS + 1] =  
    {
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, id),                0UL,          999UL },
        { MSG_FIELD_LONG,   MSG_OFFSET(MSG_HEADER, cd),                0UL,          999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, plaza_number),      0UL,         9999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, lane_number),       0UL,         9999UL },
		{ MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, M_number),          0UL,        99999UL },
        { MSG_FIELD_NOW,    MSG_OFFSET(MSG_HEADER, time_of_message) },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, toll_collector_id), 0UL,       999999UL },
        { MSG_FIELD_DWORD,  MSG_OFFSET(MSG_HEADER, active_vault_id),   0UL,       999999UL },

        { MSG_FIELD_STOP }
    };


    
    #pragma warning( pop )  

    // recopie des déclarations locale précédentes dans des varaibles globales du meme type
    // et déjà déclaraées par les macros MSG_DECLARE_REF
    MSG_INIT_REF(MSG_HEADER,                    LOCAL_MSG_HEADER);
}

// CONSTRUCTEURS / DESTRUCTEURS

// REM GENERALES : Ne jamais utiliser une structure de message 
// qui ne provient pas d'un des constructeurs suivants en parametre de fonction
// sous peine de violation d'acces !(ajout d'un entete devant la structure)

// constructeur d'un message de paiement
PUBLIC struct MSG_HEADER *MSG_HEADER_New(void)
{
    // la variable MSG_HEADER de type union MSG_Field[] est globale au module 
    // et a ete declare et initialise par les macros MSG_DECLARE_REF et MSG_INIT_REF
    return MSG_New(MSG_HEADER, sizeof(struct MSG_HEADER), "MSG_HEADER");
}



// suppression COMPLETE d'un message de event
// Une passe récursive est effectuée sur chaque champ du message 
//(champs des elements de listes compris) pour appeler les fonctions
// delete_func des champs de type "custom"
// toutes les listes(handles + elements) sont supprimes
PUBLIC BOOL MSG_HEADER_Delete_All(struct MSG_HEADER *p_header)
{
    return MSG_Delete_All(&p_header);
}

// LECTURE / ECRITURE

// La variable p_msg contient en retour le mesasge sous forme ASCII

PUBLIC BOOL MSG_HEADER_Write(struct MSG_HEADER *p_header, 
                               BYTE *p_msg, 
                               DWORD msg_size_max, 
                               DWORD *final_msg_size)
{
    *final_msg_size = MSG_Write(p_header, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// Selon le meme principe de parcours récursif que lors d'une écriture
//.la chaine p_msg est passée a la moulinette selon la description des champs 
// des structures union MSG_Field[] pour remplir une structure de données
// de type struct MSG_HEADER
PUBLIC BOOL MSG_HEADER_Read(struct MSG_HEADER *p_header, 
                              BYTE *p_msg, 
                              DWORD msg_size_max, 
                              DWORD *final_msg_size)
{
    *final_msg_size = MSG_Read(p_header, p_msg, msg_size_max);

    return *final_msg_size != 0UL;
}

// ENREGISTREMENT / RECHERCHE
PRIVATE BOOL StringToLong( unsigned char * szStr, int iLen, LONG * plValue );

// la fonction de comparaison extrait du buffer le champ "cd"
PUBLIC BOOL MSG_HEADER_Compare_CD(BYTE *p_msg, LONG Const)
{
    LONG cd;

	if( ! StringToLong( p_msg, 3, NULL ) )
		return FALSE;

	if( ! StringToLong( p_msg + 3, 3, &cd ) )
		return FALSE;

    // verification par rapport a MSG_SV_HEADER_CD
    return cd == Const;
}

PRIVATE BOOL StringToLong( unsigned char * szStr, int iLen, LONG * plValue )
{
	BOOL fCvrt = FALSE;	// Flag indiquant qu'au moins un digit a été traité

	if( plValue )
		*plValue = 0;

	//
	// Ignorer les premiers espaces et autres caractères
	// de contrôle(tab, saut de ligne etc.) dans la limite
	// de la taille du buffer.
	//
	while( iLen > 0 )
		if( *szStr == 0 )
			break;
		else if( *szStr <= ' ' )
		{
			szStr ++;
			iLen --;
		}
		else
			break;

	//
	// Tant qu'on a des digits, on les traite(dans la limite
	// de la taille du buffer).
	//
	while( iLen > 0 )
		if( *szStr == 0 )
			break;
		else if( isdigit( *szStr) )
		{
			fCvrt = TRUE;
			if( plValue ) 
				*plValue =( *plValue * 10 ) +((LONG)(*szStr)) -(LONG)'0';
			szStr++;
			iLen --;
		}
		else break;

    //
	// On s'assure que s'il reste des caractères dans le buffer, 
	// ceux-ci sont également convertis
	//
	while( iLen > 0 )
		if( *szStr == 0 )
			break;
		else if( *szStr <= ' ' )
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
	if( iLen == 0 )
		return fCvrt;
	else
		return( fCvrt &&( *szStr == 0 ) );
}

/*-------------------------------- END OF FILE ------------------------------*/
