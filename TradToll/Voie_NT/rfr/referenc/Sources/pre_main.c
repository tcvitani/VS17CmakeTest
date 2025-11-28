/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  FICHIER IMPLEMENTANT LE m‚canisme d'enregistrement
*          automatique des ID.
* FICHIER: PRE_MAIN.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Initialisation du fichier dans lequel sont stock‚s les ID que
*         reconnait l'application dans un fichier de r‚f‚rence.
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/pre_main.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.5   03 Nov 1997 16:07:28   DPI
 * Version 4.00
 * 
 *    Rev 1.4   Jun 04 1997 19:13:12   DPI
 *  
 * 
 *    Rev 1.3   May 16 1997 08:24:58   DPI
 *  
 * 
 *    Rev 1.2   May 14 1997 15:42:24   DPI
 *  
 * 
 *    Rev 1.1   May 12 1997 13:44:38   DPI
 * 1- Generer les actions obligatoires
 * 2- Sur Manuel, gerer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 15:02:20   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#define PRE_MAIN_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <direct.h>

#include "str.h"
#include "err.h"
#include "fic.h"

#include "rc_def.h"
#include "fic_gere.h"
#include "fic_id.h"
#include "rc_glob.h"

#include "pre_main.h"


/*--------------- RESERVED: ---------------*/

#include "memclass.h"


/*--------------- EXTERNALS: --------------*/



/*--------------- DEFINES: ----------------*/
#define           ATTRIBUTS_PREMIER    "w+b"
#define           ATTRIBUTS            "ab"

/*--------------- FUNCTIONS: --------------*/

PRIVATE boolean RFRWaitEndOfInitAndRecord (TpTRfrRecord p_record);
PRIVATE DWORD WINAPI RFRThreadWaitEndOfInitAndRecord (PVOID param);

/*--------------- VARIABLES: --------------*/

PRIVATE boolean wait_init = FALSE;
PRIVATE struct_tache taches[2]; /* Prevoir un element en plus pour la structure vide */

/*--------------- CODE: -------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean RFRNewTableauId
* RETOUR: TRUE  : op‚ration r‚ussie
*         FALSE : op‚ration ‚chou‚e
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture et configuration d'un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT boolean WINAPI RFRNewTableauId(TpTRfrRecord p_record)
{
   PRIVATE boolean   premier = TRUE;

   TGereFic          gereFic;
   FIC_enum_retour   retour;
   TReception        item;
   boolean           ok;
   int               longueur;

   /* Si le module ne s'est pas encore initialise (ReferenceLance() n'a pas ete appelee) */
	if (!RFR.init)
	{
		ReferenceFichierTrace ("L'enregistrement du module applicatif %s est retarde.", p_record->IdName);

		/* Attendre la fin de l'init avant l'enregistrement du module applicatif */
		if (!RFRWaitEndOfInitAndRecord (p_record))
			return FALSE;
		else
			return TRUE;
	}

	ReferenceFichierTrace ("Debut enregistrement du module applicatif %s.", p_record->IdName);

   /* verifier que l'espace disponible est suffisant */
   if( p_record->TailleArgs > RFR_MAX_BUFFER_PARAM)
       ERR_ErreurFatale();

   if( premier)
   {
      premier = FALSE;

      CreerRepertoires();
   }

   ok = InitFichierIdReception( &gereFic, ATTRIBUTS);
   if( ! ok)
       ERR_ErreurFatale();

   STR_strcpy( RFR_MAX_PATH, item.Path, p_record->Path);
   STR_strcpy( RFR_MAX_ID, item.Id, p_record->IdName);
   item.Iterable = p_record->Iterable;
   item.Obligatoire = p_record->Obligatoire;
   item.Verifier = p_record->Verifier;
   item.Faire    = p_record->Faire;
   item.PremiereAction = NO_ACTION;
   item.DerniereAction = NO_ACTION;

   retour = AjouterPosIdReception( &gereFic, &item);

   ok = FermerFichierIdReception( &gereFic);
   if( ! ok)
       ERR_ErreurFatale();

	

   /* on cr‚e le path s'il n'existe pas */
   longueur = (int)strlen( item.Path);
   if( item.Path[ longueur - 1] == '\\')
      item.Path[ longueur - 1] = '\0';

   	if ((RFR.util.ColdStart) && (_access(item.Path,0)))
		FIC_deltree(item.Path);

   if( _access( item.Path, 0))
      FIC_CreatePath( item.Path);

    p_record->Enregistree();

	if (p_record->EnregistreeEx != NULL)
		p_record->EnregistreeEx(p_record);

	ReferenceFichierTrace ("Fin enregistrement du module applicatif %s.", p_record->IdName);

   return( retour == FIC_OK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean CreerRepertoires
* RETOUR: TRUE  : op‚ration r‚ussie
*         FALSE : op‚ration ‚chou‚e
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Ouverture et configuration d'un fichier
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void       CreerRepertoires( void)
{
   char              repertoire[ RFR_MAX_PATH];

   TGereFic          gereFic;
   boolean           ok;

   /* creer les repertoires sur le disque virtuel */
   FIC_makepath( repertoire, RFR.disque_virtuel, RFR.path_reference, NULL,NULL);
   /* s'il existe d‚j…, et mˆme s'il n'est pas vide, on continue */
   FIC_CreatePath( repertoire);

   /* s'il existe d‚j…, et mˆme s'il n'est pas vide, on continue */
   FIC_CreatePath( RFR.path_transfert);

   ok = InitFichierIdReception( &gereFic, ATTRIBUTS_PREMIER);
   ERR_EstVrai( ok);

   ok = FermerFichierIdReception( &gereFic);
   ERR_EstVrai( ok);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE boolean RFRWaitEndOfInitAndRecord ( TpTRfrRecord p_record )
 * PARAMETERS: TpTRfrRecord p_record : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PRIVATE boolean RFRWaitEndOfInitAndRecord (TpTRfrRecord p_record)
{
	noyau_enum_retour	cr_lance;
    char				NomTache[MAX_PATH + 1];

	/* Initialisation thread d'attente de la fin d'init de RFR */
	
	sprintf_s(NomTache, sizeof(NomTache), "RFR WAIT INIT (%s)", p_record->IdName);
	NOYAU_INIT_TACHE(
		taches[0],
		TRUE,
		THREAD_PRIORITY_NORMAL,
		8096,
		(LPTHREAD_START_ROUTINE)(RFRThreadWaitEndOfInitAndRecord),
		(PVOID)p_record,
		NULL,
		NomTache);
	
	// Plus d'autre tache
	NOYAU_VIDE_TACHE( taches[1] );
	
   /* lancement de la tache */
   cr_lance = LanceTache(taches);
   if( cr_lance != NOYAU_OK )
		return FALSE;

	return TRUE;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD WINAPI RFRThreadWaitEndOfInitAndRecord ( PVOID param )
 * PARAMETERS: PVOID param : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
PRIVATE DWORD WINAPI RFRThreadWaitEndOfInitAndRecord (PVOID param)
{
	TpTRfrRecord p_record = (TpTRfrRecord)param;
	boolean ok;

	/* Attente de la fin d'initialisation du module RFR */
	if (AttenteEvt (RFR.wait_init_evt, NOYAU_ATTENTE_INFINIE) != NOYAU_OK)
		ERR_ErreurFatale();

	/* Enregistrement du module applicatif dans le module de Reference */
	DebutRegion();
	ok = RFRNewTableauId(p_record);
	FinRegion();
	if (!ok)
		ERR_ErreurFatale();

	return 0;
}
