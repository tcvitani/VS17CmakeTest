/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement du fichier de r‚f‚rence
* FICHIER: refer.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION: Ce module traite le fichier de r‚f‚rence.
* --------------------------------------------------------------------
* NOTA
*     Si dans le fichier de r‚f‚rence, un identifiant indique un fichier
*     faisant partie de la r‚f‚rence, alors que cet identifiant n'est
*     pas attendu par l'applicatif, il n'y a aucun traitement, la demande
*     est oubli‚
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/refer.c_v  $
 * 
 *    Rev 1.2   Sep 24 2002 10:54:22   OBADOCHE
 *  
 * 
 *    Rev 1.1   22 Mar 2001 10:03:54   FR
 * - correction du blocage du module suite à une erreur de transfert de liste
 * - correction de la gestion de l'état de prise en compte d'une nouvelle référence
 * 
 * 
 *    Rev 1.0   14 Dec 1999 15:17:52   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.15   03 Nov 1997 16:07:32   DPI
 * Version 4.00
 * 
 *    Rev 1.14   30 Oct 1997 11:57:20   DPI
 * Integration de FICTR_copy de OUTILS_C 2.00
 * 
 *    Rev 1.13   Jul 29 1997 15:30:14   DPI
 * Correction bug sur comparaison des fichiers
 * de reference
 * 
 *    Rev 1.12   Jul 28 1997 18:30:36   DPI
 * Gestion de la version
 * 
 *    Rev 1.11   Jun 13 1997 11:06:34   DPI
 *  
 *
 *    Rev 1.10   Jun 13 1997 11:05:58   DPI
 * Suppression de la levee d'une erreur fatale
 * quand le iteration attendue est fausse
 * 
 *    Rev 1.9   Jun 12 1997 18:52:52   DPI
 * correction
 * 
 *    Rev 1.8   Jun 12 1997 14:03:10   DPI
 * Conserver le fichier de ref meme erronee
 * 
 *    Rev 1.7   Jun 11 1997 17:50:58   DPI
 * Amelioration des traces
 * 
 *    Rev 1.6   Jun 04 1997 19:13:16   DPI
 *  
 * 
 *    Rev 1.5   Jun 03 1997 16:20:26   DPI
 * Modif au demarrage sur l'emplacement 
 * des fichiers
 * 
 *    Rev 1.4   Jun 03 1997 15:25:20   DPI
 * Gestion de l'emplacement du fichier
 * saisi manuellement
 * 
 *    Rev 1.3   Jun 03 1997 15:06:24   DPI
 * Supprimer dans EnregistrerActionReception
 * le param Enregistrer qui est obsolete
 * 
 *    Rev 1.2   May 15 1997 14:28:46   DPI
 *  
 * 
 *    Rev 1.1   May 12 1997 13:44:40   DPI
 * 1- Generer les actions obligatoires
 * 2- Sur Manuel, gerer comme cas standard
 * 
 *    Rev 1.0   Apr 24 1997 15:02:24   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define  REFER_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>


#include "err.h"
#include "fic.h"
#include "str.h"

#include "rc_def.h"
#include "fic_gere.h"

#include "fic_ref.h"
#include "fic_id.h"

#include "referenc.h"

#include "comp_ref.h"
#include "rc_glob.h"
#include "rc_emis.h"
#include "action.h"
#include "path.h"
#include "rc_conf.h"

/* ### FICTR_xxx remplace par FIC_xxx */
#include "fic.h"

#include "refer.h"

// MFR start 2007/5/22
#include <csr_files_tools.h>
#include <reg.h>
#include <tchar.h>
#include <winreg.h>
// MFR end 2007/5/22

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/

/*--------------- RESERVED: ---------------*/

PRIVATE FIC_enum_retour TraiterItemDifferent( TGereFic          *GereFic,
                                              FIC_enum_retour   etat,
                                              long              Position,
                                              TParam            *Param,
                                              TFichier          *Val);
PRIVATE FIC_enum_retour TraiterItemIdentique( TGereFic          *GereFic,
                                              FIC_enum_retour   etat,
                                              long              Position,
                                              TParam            *Param,
                                              TFichier          *Val);
PRIVATE FIC_enum_retour TraiterItemDisparu( TGereFic          *GereFic,
                                            FIC_enum_retour   etat,
                                            long              Position,
                                            TParam            *Param,
                                            TFichier          *Val);
PRIVATE FIC_enum_retour EnregistrerItem( TGereFic          *GereFic,
                                         FIC_enum_retour   etat,
                                         long              Position,
                                         TParam            *Param,
                                         TFichier          *Val);
PRIVATE boolean VerifierCoherenceIteration( boolean Iterable,
                                            TIteration *Iteration);

PRIVATE boolean CheckIfsRegKeyIsUnused(TpTTacheContext Context, char *szRegValueName);
PRIVATE void ExtractListNameFromFileName(IN char *p_FileName, IN OUT char *szName, IN DWORD dwNameSize);
PRIVATE void CopyAllFileNamesInLIstInString(char *pcFileName, TpTTacheContext    Context);
PRIVATE void CopyAllMultiFileNamesInString(char *pcId, TpTTacheContext    Context);

/*--------------- DEFINE: ---------------*/
#define MULTI_FILE_MAX_FILES		20

/*--------------- FUNCTION: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   CreerFichierReferenceManuel
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: Configure les gestionnaires de fichiers, et cr‚e un fichier
*       avec une entr‚e manuel.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void CreerFichierReferenceManuel( TpTTacheContext  Context)
{
   char                 fichier_tmp[ RFR_MAX_FIC];
   FIC_enum_retour      retour;
   TReception           *id;
   TIteration           iteration;
   TFichier             val;
   boolean              ok;

   /* configurer les gestionnaires */
   FIC_makepath( fichier_tmp, RFR.disque_virtuel, RFR.path_reference,
                               FICHIER_REFERENCE_TMP, NULL);
   if( ! EchangerFichierReference( Context->GereCreer, fichier_tmp, "w+t"))
      ERR_ErreurFatale();

   /* on ajoute l'identification du fichier de r‚f‚rence
    * dans le fichier temporaire cr‚e
    * Signalons qu'on conserve la trace du pr‚c‚dent fichier de ref
    * dont est d‚riv‚ celui-ci
    */
   EcrirePosReference( Context->GereCreer, REFERENCE_ID, 0,
                                           Context->NomFichierNouveau,
                                           Context->ReferenceFichierNouveau,
                                           "");

   /* on parcours l'ancien fichier pour cr‚er le nouveau */
   retour = FIC_EN_COURS;
   while( retour == FIC_EN_COURS)
   {
     retour = TraverseReferenceFichier( Context->GereAncien,
                                        EnregistrerItem,
                                        Context);
   }

   /* est-ce qu'on attends ce type de fichier */
   retour = LireIdIdReception( Context->GereId,
                               Context->action.u.manuel.Id,
                               &id);
   ERR_EstVrai( retour == FIC_OK);

   if( id->Iterable)
       iteration = 1;
   else
       iteration = 0;

   /* On ‚crit le nouvel item */
   retour = EcrirePosReference( Context->GereCreer,
                                Context->action.u.manuel.Id,
                                iteration,
                                Context->action.u.manuel.Fichier,
                                ID_REFERENCE_MANUELLE,
                                "");
   ERR_EstVrai( retour == FIC_OK);

   /*-------------------------------*/
   /* on cr‚e le fichier sur disque */
   if( ! FermerFichierReference( Context->GereCreer))
      ERR_ErreurFatale();

   if( _access( Context->NomFichierNouveau, 0) == 0)
       FIC_unlink( Context->NomFichierNouveau);

  /* ### FICTR_xxx remplace par FIC_xxx */
   ok = FIC_copy( Context->NomFichierNouveau, fichier_tmp,
                    FIC_COPY_WITH_DATE/*, RFR_TIMEOUT_RAPIDE*/);
   ERR_EstVrai( ok);

   /* on le rouvre pour pouvoir l'acquitter */
   if( ! EchangerFichierReference( Context->GereCreer, fichier_tmp, "a+t"))
      ERR_ErreurFatale();
   if( ! EchangerFichierReference( Context->GereNouveau, fichier_tmp, "a+t"))
      ERR_ErreurFatale();

   /*--------------------------------*/
   /* il faut enregistrer une action */
   STR_strcpy( RFR_MAX_ID, val.Id, Context->action.u.manuel.Id);
   STR_strcpy( RFR_MAX_FIC, val.Fichier, Context->action.u.manuel.Fichier);
   STR_strcpy( RFR_MAX_REF, val.Reference, ID_REFERENCE_MANUELLE);
   val.Iteration = iteration;
   val.Hostname[ 0] = '\0';

   EnregistrerActionReception( Context, &val, TRUE, FALSE, TRUE);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ComparerFichiersReference
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: Verifie le format du fichier de r‚f‚rence, puis compare les deux
*       fichiers de r‚f‚rence, en remplissant un nouveau
* --------------------------------------------------------------------
* NOTA:
*      Si la r‚f‚rence est correcte, on efface imm‚diatement toutes
*      les actions en cours
* --------------------------------------------------------------------
* $F_FCTN
*/
boolean ComparerFichiersReference( TpTTacheContext       Context)
{
   boolean  ok;
   boolean  parReference;

   ok = TRUE;

   /* on ajoute l'identification du nouveau fichier de r‚f‚rence
    * dans le fichier temporaire cr‚e
    */
   EcrirePosReference( Context->GereCreer, REFERENCE_ID, 0,
                                           Context->NomFichierNouveau,
                                           Context->ReferenceFichierNouveau,
                                           "");

   /* on compare les fichiers */
   ReferenceFichierTrace("------------------------------------\n");
   ReferenceFichierTrace(" Comparer les fichiers de r‚f‚rence \n");

   /* on compare le nouveau et l'ancien par reference */
   parReference = TRUE;
   ComparerReferences( Context->GereNouveau, Context->GereAncien,
                       TraiterItemDifferent, TraiterItemIdentique,
                       parReference, Context);

   /* on compare l'ancien et le nouveau par Id/It‚ration */
   parReference = FALSE;
   ComparerReferences( Context->GereAncien, Context->GereNouveau,
                       TraiterItemDisparu, NULL,
                       parReference, Context);
   ReferenceFichierTrace("------------------------------------\n");

   /* le boolean de retour servirait si on voulait g‚rer des cas d'echec
    * du fichier de reference, quand un des fichiers est en panne
    */
   return( ok);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RecevoirFichierReference
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: Configure les gestionnaires de fichiers
* --------------------------------------------------------------------
* $F_FCTN
*/
void RecevoirFichierReference( TpTTacheContext  Context)
{
   char        fichier_dest[ RFR_MAX_PATH];

   if( ! EchangerFichierReference( Context->GereNouveau, Context->NomFichierNouveau, "r+t"))
      ERR_ErreurFatale();

   FIC_makepath( fichier_dest, RFR.disque_virtuel, RFR.path_reference,
                               FICHIER_REFERENCE_TMP, NULL);
   if( ! EchangerFichierReference( Context->GereCreer, fichier_dest, "w+t"))
      ERR_ErreurFatale();

   Context->TousFichiersValides = TRUE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   DemanderFichierReference
* PARAMETRES: - le contexte de la tache
*             - le nom et le path du fichier sur le LS
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: V‚rifie que le nouveau fichier de r‚f‚rence n'est pas l'ancien
*       et demande le chargement si n‚cessaire
* --------------------------------------------------------------------
* $F_FCTN
*/
boolean DemanderFichierReference( TpTTacheContext  Context)
{
   boolean              demande;

  if( Context->ReferenceFichierCourant[ 0] != '\0')
   {
// Debut FR 20/03/01
		// Si le telechargement precedent c'est mal termine, le chargement est necessaire
      if( Context->TousFichiersValides == FALSE )
         demande = TRUE;
		else
// Fin FR 20/03/01
	      demande = ( _stricmp( Context->ReferenceFichierCourant,
   	                        Context->ReferenceFichierNouveau) != 0);
   }
   else
   {
      /* s'il n'y avait pas de fichier de ref avant */
       demande = TRUE;
   }

   return( demande);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AcquitterFichierReference
* PARAMETRES: - le contexte de la tache
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE: AprŠs le traitement de tous les fichiers qui ont chang‚, on
*       remplace l'ancien fichier par celui qui a ‚t‚ cr‚‚
* --------------------------------------------------------------------
* $F_FCTN
*/
void AcquitterFichierReference( TpTTacheContext   Context,
                                boolean Acquittement)
{
   char        fichier_source[ RFR_MAX_PATH];
   char        fichier_dest[ RFR_MAX_PATH];
   int         longueur;
   int         retour;

   if( Acquittement)
   {
      /* fermer les gestionnaires */
      if( ! FermerFichierReference( Context->GereAncien))
          ERR_ErreurFatale();
      if( ! FermerFichierReference( Context->GereNouveau))
          ERR_ErreurFatale();
      if( ! FermerFichierReference( Context->GereCreer))
          ERR_ErreurFatale();

      /* renommer le fichier cr‚e en fichier courant */
      FIC_makepath( fichier_source, RFR.disque_virtuel, RFR.path_reference,
                                    FICHIER_REFERENCE_TMP, NULL);
      FIC_makepath( fichier_dest, RFR.disque_virtuel, RFR.path_reference,
                                  FICHIER_REFERENCE_COURANT, NULL);
      FIC_unlink( fichier_dest);
      FIC_rename( fichier_source, fichier_dest);

      /* rouvrir le fichier de reference en cours */
      FIC_makepath( fichier_dest, RFR.disque_virtuel, RFR.path_reference,
                                  FICHIER_REFERENCE_COURANT, NULL);
      if( ! EchangerFichierReference( Context->GereAncien, fichier_dest, "r+t"))
          ERR_ErreurFatale();

      /* mettre a jour la configuration */
      RC_EcrireNouvelleConfiguration( Context);

      STR_strcpy( RFR_MAX_FIC, Context->NomFichierCourant,
                           Context->NomFichierNouveau);
      STR_strcpy( RFR_MAX_REF, Context->ReferenceFichierCourant,
                           Context->ReferenceFichierNouveau);

	  // MFR start 2007/5/22
	  ReferenceFichierTrace("<--| Message file =%s", Context->IdFichierReference);
	  // MFR end 2007/5/22	

      if( Context->TousFichiersValides)
	  {
          RC_EmettreMessageUpdate( Context->util.BalMessage,
                                   Context->util.BalLocale,
                                   Context->util.NumeroPool,
                                   Context->IdFichierReference,
                                   Context->ReferenceFichierCourant,
                                   ACTIVATED_FILE);
		  // MFR start 2007/5/22
		  ReferenceFichierTrace("<--| Message ACTIVATED_FILE, file =%s", Context->IdFichierReference);
		  // MFR end 2007/5/22		  
	  }
      else
          RC_EmettreMessageUpdate( Context->util.BalMessage,
                                   Context->util.BalLocale,
                                   Context->util.NumeroPool,
                                   Context->IdFichierReference,
                                   Context->ReferenceFichierCourant,
// Debut FR 20/03/01
//                                   PARTIAL_ACTIVATION_FILE);
                                   UNLOAD_FILE);
// Fin FR 20/03/01
   }
   else
   {
      /* le fichier de r‚ference n'est pas acquittable, i.e. il est incorrect,
       * donc on ne change pas le fichier de r‚f‚rence courant
       */
      if( ! FermerFichierReference( Context->GereNouveau))
          ERR_ErreurFatale();
      if( ! FermerFichierReference( Context->GereCreer))
          ERR_ErreurFatale();

      /* effacer le fichier cr‚e */
      FIC_makepath( fichier_dest, RFR.disque_virtuel, RFR.path_reference,
                                  FICHIER_REFERENCE_TMP, NULL);
      FIC_unlink( fichier_dest);

      /* renommer le fichier transf‚r‚ erron‚ en changeant le dernier
       * caractere
       */
      STR_strcpy( RFR_MAX_PATH, fichier_dest, Context->NomFichierNouveau);
      longueur = (int)strlen( fichier_dest);
      fichier_dest[longueur-1] = '~';

      /* si le unlink ou le rename echoue, tant pis */
      retour = _unlink( fichier_dest);
      if( retour == 0)
         retour = rename( Context->NomFichierNouveau, fichier_dest);

      if( retour != 0)
         FIC_unlink( Context->NomFichierNouveau);

      RC_EmettreMessageUpdate( Context->util.BalMessage,
                               Context->util.BalLocale,
                               Context->util.NumeroPool,
                               Context->IdFichierReference,
                               Context->ReferenceFichierNouveau,
                               INVALID_FILE);
   }

   Context->NomFichierNouveau[ 0] = '\0';
   Context->ReferenceFichierNouveau[ 0] = '\0';
}







/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
PRIVATE boolean IsCurrentActiveFile(char *pcId, char *pcFileName)
{
	// MFR start 2007/5/22
	char szRefFileDir[MAX_PATH]	= {0};
	char szRefFilePath[MAX_PATH]	= {0};
	char szActiveFile[MAX_PATH]	= {0};
	enum_files_return enRet;

	CHAR		szRegPathList[MAX_PATH]	= {0};
	CHAR		szRegValueName[MAX_PATH]= {0};
	DWORD		dwRegValIndex			= 0;
	DWORD		dwRegValueNameSize		= MAX_PATH;
	DWORD		dwRegValueType;
	CHAR		chRegValBuffer[MAX_PATH]= {0};
	DWORD		dwRegValBufferSize		= MAX_PATH;
	BOOL		bMultiFiles				= FALSE;

	ReferenceFichierTrace( "<---| Cheching existance of the file, type = %s, file = %s", pcId, pcFileName );

	if( strcmp(pcId, "TFT") != 0 )
	{// If file type is not TFT
		enRet = FileGetCurrent(pcId, szActiveFile, sizeof(szActiveFile));

		FileGetMultiFilesBoolean(pcId, &bMultiFiles);

		ReferenceFichierTrace( "<---| Current active file = %s", szActiveFile );

		if (bMultiFiles)
		{

			ReferenceFichierTrace("<---| File is multi files!");
			
			char			*ptr = NULL, *ptr_old = NULL;
			char			TempStr[MAX_PATH + 1] = { 0 };
			BOOL			bFileOK = FALSE;

			FileGetSpecificPath(pcId, szRefFileDir);

			ptr_old = szActiveFile;

			while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
			{
				memcpy(TempStr, ptr_old, ptr - ptr_old);
				TempStr[(int)(ptr - ptr_old)] = '\0';
				ptr_old = ptr;
				ptr_old++;

				if (strcmp(pcFileName, TempStr) == 0)
				{
					ReferenceFichierTrace("<---| File is same as currently active");

					FIC_makepath(szRefFilePath, NULL, szRefFileDir, TempStr, NULL);

					ReferenceFichierTrace("<---| Checking existance of the file, path = %s", szRefFilePath);

					if (_access(szRefFilePath, 00) == 0)
					{// If file is at correct place
						// No need for CHECKED_FILE message
						ReferenceFichierTrace("<---| File is already present in correct dir, aborting sending CHECKED_FILE message");
						return TRUE;
					}
					else
					{
						ReferenceFichierTrace("<---| File is missing");
						return FALSE;
					}
				}
			}

		}
		else
		{
			if (strcmp(pcFileName, szActiveFile) == 0)
			{// If active file in registry is same as current file (same version)
				ReferenceFichierTrace("<---| File is same as currently active");
				// Create full reference file path
				FileGetSpecificPath(pcId, szRefFileDir);
				FIC_makepath(szRefFilePath, NULL, szRefFileDir, szActiveFile, NULL);

				ReferenceFichierTrace("<---| Checking existance of the file, path = %s", szRefFilePath);

				if (_access(szRefFilePath, 00) == 0)
				{// If file is at correct place
					// No need for CHECKED_FILE message
					ReferenceFichierTrace("<---| File is already present in correct dir, aborting sending CHECKED_FILE message");
					return TRUE;
				}
				else
				{
					ReferenceFichierTrace("<---| File is missing");
					return FALSE;
				}
			}
		}
	}
	else
	{// TFT files works bit different

		// TODO : read mailbox name from TFT key

		// Create path to TFT list
		sprintf_s(szRegPathList, sizeof(szRegPathList),
				"%s%s%s%s%s%s\\%s",
				CSR_REG_KEYn_CSRBASE,
				CSR_REG_KEYn_LANE_BASE,
				CSR_REG_KEYn_CONFIG,
				MOD_REG_KEYn_MODULES,
				"TFT\\",
				RFR.szTftMailboxName,
				"List\\"
				);

		   // Debug
		   // TODO : Create path by reading registry
//		   sprintf( szRegPathList, "SOFTWARE\\CsRoute\\LaneController\\Config\\Modules\\TFT\\BL_TFT\\List\\");
		   // /Debug

		ReferenceFichierTrace( "<---| Searching trough list of TFT files" );

		while( REG_Enum_Valeurs(CSR_REG_KEYi_ROOT,
								szRegPathList,
								dwRegValIndex,
								szRegValueName,
								&dwRegValueNameSize,
								&dwRegValueType,
								(LPVOID)chRegValBuffer,
								&dwRegValBufferSize ) == ERROR_SUCCESS)
		{// Search trough TFT registry list for value (file name) that is same as our current file (includingg file version)
			ReferenceFichierTrace( "<---| Cheching registy, name = %s, value = %s", szRegValueName, chRegValBuffer );
			if( strcmp( chRegValBuffer, pcFileName ) == 0 )
			{// Registry value with same name found
				ReferenceFichierTrace( "<---| Registry value with same FILE name found" );
				// Create full reference file path
				FileGetSpecificPath( pcId, szRefFileDir );
				FIC_makepath( szRefFilePath, NULL, szRefFileDir, pcFileName, NULL );

				ReferenceFichierTrace( "<---| Checking existance of the file, path = %s", szRefFilePath );

				if( _access( szRefFilePath, 00 ) == 0 )
				{// If file is at correct place
					// No need for CHECKED_FILE message
					ReferenceFichierTrace( "<---| File is already present in correct dir, aborting sending CHECKED_FILE message" );						
					return TRUE;
				}
				else
				{
					ReferenceFichierTrace( "<---| File is missing" );
					return FALSE;
				}
			}
				// Get ready for next loop
			dwRegValueNameSize = MAX_PATH;
			dwRegValBufferSize = MAX_PATH;
			dwRegValIndex++;
		}
	}	   
	// MFR end 2007/5/22

	return FALSE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   TraiterItemDifferent
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* REMARQUE : Appel‚ quand les items dans l'ancien et le nouveau fichier
*            de r‚f‚rence sont diff‚rent.
* --------------------------------------------------------------------
* NOTA: Si on passe dans cette fonction, cela implique qu'on a une
*       nouvelle reference, mais on n'a peut ˆtre pas d'ancienne
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE FIC_enum_retour TraiterItemDifferent( TGereFic          *GereFic,
                                              FIC_enum_retour   Etat,
                                              long              Position,
                                              TParam            *Param,
                                              TFichier          *Val)
{
   TpTTacheContext       context;
   TReception           *id;
   FIC_enum_retour      retour;
   char                 fichier_dest[ RFR_MAX_PATH];

   NO_WARNING( GereFic);
   NO_WARNING( Position);

   context = ( TpTTacheContext) Param;
   ReferenceFichierTrace("DIFF  - Id = '%s',", Val->Id);

   /* est-ce qu'on attends ce type de fichier */
   retour = LireIdIdReception( context->GereId, Val->Id, &id);
   if ( retour != FIC_OK)      /* ce type de fichier n'est pas trait‚ */
   {
      /* ceci gŠre le REFERENCE_ID */
      ReferenceFichierTrace("non attendu\n");
      return( FIC_OK);
   }
   ReferenceFichierTrace(" It = '%d', File = '%s'", Val->Iteration, Val->Fichier);

   /* on verifie que l'it‚ration est correcte */
   if( ! VerifierCoherenceIteration( id->Iterable, &Val->Iteration))
   {
      return( FIC_OK);
   }

   if( context->util.Demarrage)
   {
      /* au demarrage, on ne fait pas de requete au LS. */

      /* il faut quand meme verifier que le fichier est sur le disque */
      ChangerPath( fichier_dest, Val->Fichier, id->Path);
//      if( access( fichier_dest, 0) != 0)
	  if(IsCurrentActiveFile(Val->Id, Val->Fichier) == FALSE)
      {
         /* le fichier n'existe pas, il est donc invalide */
         ReferenceFichierTrace(" non disponible sur le disque au d‚marrage\n");
         RC_EmettreMessageUpdate( context->util.BalMessage,
                                  context->util.BalLocale,
                                  context->util.NumeroPool,
                                  Val->Id,
                                  Val->Reference,
                                  UNLOAD_FILE);
         context->TousFichiersValides = FALSE;
      }
      else
      {
         /* il existe a son emplacement d‚finitif */
         ReferenceFichierTrace(" OK\n");
         RC_EmettreMessageUpdate( context->util.BalMessage,
                                  context->util.BalLocale,
                                  context->util.NumeroPool,
                                  Val->Id,
                                  Val->Reference,
                                  ALREADY_CHECKED_FILE);
		 CopyAllFileNamesInLIstInString(Val->Fichier, context);
		 CopyAllMultiFileNamesInString(Val->Id, context);
         retour = EcrirePosReference( context->GereCreer, Val->Id, Val->Iteration,
                                                 Val->Fichier,
                                                 Val->Reference,
                                                 Val->Hostname);
         ERR_EstVrai( retour == FIC_OK);

         /* il faut enregistrer une action */
         EnregistrerActionReception( context, Val, FALSE, TRUE, FALSE);
      }
   }
   else
   {
	   if(IsCurrentActiveFile(Val->Id, Val->Fichier) == TRUE)
	   {
		   ReferenceFichierTrace(" OK\n");
		   RC_EmettreMessageUpdate( context->util.BalMessage,
                                    context->util.BalLocale,
                                    context->util.NumeroPool,
                                    Val->Id,
                                    Val->Reference,
                                    ALREADY_CHECKED_FILE);
		   CopyAllFileNamesInLIstInString(Val->Fichier, context);
		   CopyAllMultiFileNamesInString(Val->Id, context);
           retour = EcrirePosReference( context->GereCreer, Val->Id, Val->Iteration,
                                              Val->Fichier,
                                              Val->Reference,
                                              Val->Hostname);
           ERR_EstVrai( retour == FIC_OK);

           /* il faut enregistrer une action */
           EnregistrerActionReception( context, Val, FALSE, TRUE, FALSE);
	   }
	   else
	   {
	      RC_DemanderChargementFichier( context->util.BalFichier,
		                                context->util.BalLocale,
		                                context->util.NumeroPool,
		                                Val->Fichier,
		                                RFR.path_transfert);
                                        //Val->Hostname);
		  CopyAllFileNamesInLIstInString(Val->Fichier, context);
		  CopyAllMultiFileNamesInString(Val->Id, context);
		  context->NbFichiersReclames ++;
	   }
   }

   return( Etat);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   TraiterItemIdentique
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* REMARQUE : Appel‚ quand les items dans l'ancien et le nouveau fichier
*            de r‚f‚rence sont identiques, et que le fichier existe
*            dans son repertoire d‚finitif.
*            Dans ce cas, il n'y a pas de copie du fichier. De mˆme, il
*            n'y a pas de v‚rification, puisqu'elle a d‚j… eu lieu.
* --------------------------------------------------------------------
* NOTA: Si on passe dans cette fonction, cela implique qu'on a une
*       ancienne reference et une nouvelle
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE FIC_enum_retour TraiterItemIdentique( TGereFic          *GereFic,
                                              FIC_enum_retour   Etat,
                                              long              Position,
                                              TParam            *Param,
                                              TFichier          *Val)
{
   TpTTacheContext       context;
   TReception           *id;
   FIC_enum_retour      retour;
   char                 fichier_dest[ RFR_MAX_PATH];

   NO_WARNING( GereFic);
   NO_WARNING( Position);

   context = ( TpTTacheContext) Param;
   ReferenceFichierTrace("IDENT - Id = '%s',", Val->Id);

   /* est-ce qu'on attends ce type de fichier */
   retour = LireIdIdReception( context->GereId, Val->Id, &id);
   if ( retour != FIC_OK)      /* ce type de fichier n'est pas trait‚ */
   {
      /* ceci gŠre le REFERENCE_ID */
      ReferenceFichierTrace("non attendu\n");
      return( FIC_OK);
   }
   ReferenceFichierTrace(" It = '%d', File = '%s'", Val->Iteration, Val->Fichier);

   /* on verifie que l'it‚ration est correcte */
   if( ! VerifierCoherenceIteration( id->Iterable, &Val->Iteration))
   {
      return( FIC_OK);
   }

   /* il faut verifier que le fichier est sur le disque */
   ChangerPath( fichier_dest, Val->Fichier, id->Path);
   //if( access( fichier_dest, 0) != 0)
   if(IsCurrentActiveFile(Val->Id, Val->Fichier) == FALSE)
   {
      /* le fichier n'existe pas, il faut donc le charger,
       * on le traite donc comme si il ‚tait nouveau
       * NOTA: cela indique soit une suppression manuelle du fichier
       *                    soit que le fichier etatit invalide
       */
      ReferenceFichierTrace(" non disponible sur le disque -> Traiter comme DIFF\n");
      TraiterItemDifferent( GereFic, FIC_NOK, Position, Param, Val);
   }
   else
   {
       ReferenceFichierTrace(" OK\n");
       RC_EmettreMessageUpdate( context->util.BalMessage,
                                context->util.BalLocale,
                                context->util.NumeroPool,
                                Val->Id,
                                Val->Reference,
                                ALREADY_CHECKED_FILE);
      retour = EcrirePosReference( context->GereCreer, Val->Id, Val->Iteration,
                                              Val->Fichier,
                                              Val->Reference,
                                              Val->Hostname);
      ERR_EstVrai( retour == FIC_OK);

      /* il faut enregistrer une action */
      EnregistrerActionReception( context, Val, FALSE, TRUE, FALSE);
   }

   return( Etat);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   TraiterItemDisparu
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* REMARQUE : Appel‚ quand les items dans l'ancien et le nouveau fichier
*            de r‚f‚rence sont diff‚rents, et en particulier quand un
*            item existait dans le fichier ancien et pas dans le nouveau
* --------------------------------------------------------------------
* NOTA: Si on passe dans cette fonction, cela implique qu'on a une
*       ancienne reference et une nouvelle
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE FIC_enum_retour TraiterItemDisparu( TGereFic          *GereFic,
                                            FIC_enum_retour   Etat,
                                            long              Position,
                                            TParam            *Param,
                                            TFichier          *Val)
{
   TpTTacheContext       context;
   TReception           *id;
   FIC_enum_retour      retour;

   NO_WARNING( GereFic);
   NO_WARNING( Position);

   context = ( TpTTacheContext) Param;
   ReferenceFichierTrace("DISP  - Id = '%s',", Val->Id);

   /* est-ce qu'on attends ce type de fichier */
   retour = LireIdIdReception( context->GereId, Val->Id, &id);
   if ( retour != FIC_OK)      /* ce type de fichier n'est pas trait‚ */
   {
      /* ceci gŠre le REFERENCE_ID */
      ReferenceFichierTrace("non attendu\n");
      return( Etat);
   }
   ReferenceFichierTrace(" It = '%d', File = '%s'", Val->Iteration, Val->Fichier);

   /* on verifie que l'it‚ration est correcte */
   if( ! VerifierCoherenceIteration( id->Iterable, &Val->Iteration))
   {
      return( Etat);
   }

   /* cet item a disparu */
   if( Val->Iteration == 0)
   {
      /* Si l'iteration est nulle, cela n'est pas admissible.
       * On continue a utiliser l'ancienne r‚f‚rence
       */
      ReferenceFichierTrace("-> utiliser '%s' ref '%s'\n", Val->Fichier, Val->Reference);
      retour = EcrirePosReference( context->GereCreer, Val->Id,
                                                       Val->Iteration,
                                                       Val->Fichier,
                                                       Val->Reference,
                                                       Val->Hostname);
      ERR_EstVrai( retour == FIC_OK);
      /* il faut enregistrer une action */
      EnregistrerActionReception( context, Val, FALSE, TRUE, FALSE);
   }
   else
   {
      ReferenceFichierTrace("OK, iteration perdue\n");
   }

   return( Etat);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   EnregistrerItem
* PARAMETRES: - le fichier de r‚f‚rence
*             - l'‚tat de lecture dans le fichier
*             - la position dans le fichier
*             - un paramŠtre d‚cast‚
*             - la valeur de l'item.
* RETOUR:     l'etat de lecture du fichier.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* REMARQUE : Appel‚ quand un item change de valeur manuellement, par
*            action operateur
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE FIC_enum_retour EnregistrerItem( TGereFic          *GereFic,
                                         FIC_enum_retour   Etat,
                                         long              Position,
                                         TParam            *Param,
                                         TFichier          *Val)
{
   TpTTacheContext       context;
   FIC_enum_retour       retour;

   NO_WARNING( GereFic);
   NO_WARNING( Position);

   context = ( TpTTacheContext) Param;

   if( Etat != FIC_OK)
     return( Etat);

   if( strcmp( Val->Id, context->action.u.manuel.Id) != 0)
   {
      /* cet id n'est pas celui qu'on modifie */
      retour = EcrirePosReference( context->GereCreer, Val->Id, Val->Iteration,
                                           Val->Fichier,
                                           Val->Reference,
                                           Val->Hostname);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( Etat);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   VerifierCoherenceIteration
* PARAMETRES: - le boolean Iterable
*             - le numero d'it‚ration
* RETOUR:     TRUE si c'est coherent
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE boolean VerifierCoherenceIteration( boolean Iterable,
                                            TIteration *Iteration)
{
   /* on verifie que l'it‚ration est correcte */
   if ( Iterable && *Iteration == 0)
   {
      ReferenceFichierTrace(" Id iterable sans iteration\n");
      //return( FALSE);
	   *Iteration = 1;
	  return( TRUE);
   }

   if ( ! Iterable && *Iteration != 0)
   {
      ReferenceFichierTrace(" Id non iterable avec iteration\n");
      return( FALSE);
   }

   return(TRUE);
}

PRIVATE boolean CheckIfsRegKeyIsUnused(TpTTacheContext Context, char *szRegValueName)
{
	char				szListName[MAX_PATH + 1] = { 0 };
	char				*ptr = NULL, *ptr_old = NULL;
	char				TempStr[MAX_PATH + 1] = { 0 };
	boolean				bRegKeyNotInList = FALSE;

	ptr_old = Context->szFileNamesForComparingWithReg;

	while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
	{ 
		if (sizeof(TempStr) >= (size_t)(ptr - ptr_old))
		{
			memcpy(TempStr, ptr_old, ptr - ptr_old);
			TempStr[(int)(ptr - ptr_old)] = '\0';
			ptr_old = ptr;
			ptr_old++;

			if (strcmp(szRegValueName, TempStr) == 0)
			{
				ReferenceFichierTrace("<---| Registy key is valid, name = %s", szRegValueName);
				bRegKeyNotInList = FALSE;
				break;
			}
			else
			{
				bRegKeyNotInList = TRUE;
			}
		}
		else
			ReferenceFichierTrace("CheckIfsRegKeyIsUnused(): Missmatch in pointer size");
		
	}

	return bRegKeyNotInList;
}

PRIVATE void ExtractListNameFromFileName(IN char *p_FileName, IN OUT char *szName, IN DWORD dwNameSize)
{
#define FILE_SUBKEY_SEP		"."
#define FILE_SUBKEY_NB_SEP	3
	char	*p_filename = NULL;
	char	*p_ptr1 = NULL;
	char	*p_ptr2 = NULL;
	int		count = 0;
	char	szSubkey[MAX_PATH + 1] = { 0 };

	p_filename = p_FileName;

	count = 0;
	while (((p_filename = strstr(p_filename, FILE_SUBKEY_SEP)) != NULL) && (count < (FILE_SUBKEY_NB_SEP - 2)))
	{
		p_filename++;
		count++;
	}

	// subkey name is placed between second and third point character ('.') of the filename
	if (p_filename != NULL)
	{
		p_filename++;
		p_ptr1 = p_filename;
		count = 0;
		p_filename = p_FileName;
		while (((p_filename = strstr(p_filename, FILE_SUBKEY_SEP)) != NULL) && (count < FILE_SUBKEY_NB_SEP - 1))
		{
			p_filename++;
			count++;
		}

		p_ptr2 = p_filename;

		if (sizeof(szSubkey) >= (size_t)(p_ptr2 - p_ptr1))
		{
			memcpy(szSubkey, p_ptr1, p_ptr2 - p_ptr1);
			szSubkey[p_ptr2 - p_ptr1] = '\x0';

			strcpy_s(szName, dwNameSize, szSubkey);
		}
		else
		{
			ReferenceFichierTrace("ExtractListNameFromFileName(): Missmatch in pointer size");
		}
		
	}
}

PUBLIC void DeleteUnusedRegKey(TpTTacheContext Context)
{
	char				*ptr = NULL, *ptr_old = NULL;
	char				TempStr[MAX_PATH + 1] = { 0 };
	char				pcKey[MAX_PATH + 1] = { 0 };
	CHAR				szRegValueName[MAX_PATH] = { 0 };
	DWORD				dwRegValIndex = 0;
	DWORD				dwRegValueNameSize = MAX_PATH;
	DWORD				dwRegValueType;
	CHAR				chRegValBuffer[MAX_PATH] = { 0 };
	DWORD				dwRegValBufferSize = MAX_PATH;
	DWORD				dwReturnErrorCode;
	HKEY				hKey;
	

	ptr_old = Context->szMultiFilesNames;

	while ((ptr = strstr(ptr_old, FILES_TOOLS_SEP_CHAR)) != NULL)
	{
		if (sizeof(TempStr) >= (size_t)(ptr - ptr_old))
		{
			memcpy(TempStr, ptr_old, ptr - ptr_old);
			TempStr[(int)(ptr - ptr_old)] = '\0';
			ptr_old = ptr;
			ptr_old++;

			// Create path to multi file list
			sprintf_s(pcKey, sizeof(pcKey), "%s%s%s%s%s%s\\%s", CSR_REG_KEYn_CSRBASE,
				CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG, CSR_REG_KEYn_PCS,
				CSR_REG_KEYn_PCSFiles, TempStr, CSR_REG_KEYn_LIST);

			// go through all reg keys in VTW list and check if any reg key is unused
			// if key is unused, delete that key
			while (REG_Enum_Valeurs(CSR_REG_KEYi_ROOT,
				pcKey,
				dwRegValIndex,
				szRegValueName,
				&dwRegValueNameSize,
				&dwRegValueType,
				(LPVOID)chRegValBuffer,
				&dwRegValBufferSize) == ERROR_SUCCESS)
			{
				if (CheckIfsRegKeyIsUnused(Context, szRegValueName))
				{

					dwReturnErrorCode = RegOpenKeyEx(CSR_REG_KEYi_ROOT, _T(pcKey), 0, KEY_ALL_ACCESS, &hKey);
					dwReturnErrorCode = RegDeleteValue(hKey, szRegValueName);
					dwReturnErrorCode = RegCloseKey(hKey);
					if (dwReturnErrorCode == ERROR_SUCCESS)
					{
						ReferenceFichierTrace("DeleteUnusedRegKey() -> Deleted reg value %s", szRegValueName);
						// registry string is deleted so we need to decrease index value to prevent skipping some registry string
						dwRegValIndex--;
					}
					else
						ReferenceFichierTrace("DeleteUnusedRegKey() -> Could not delete reg value %s, error code %d", pcKey, dwReturnErrorCode);

				}
				// Set values for next loop
				dwRegValueNameSize = MAX_PATH;
				dwRegValBufferSize = MAX_PATH;
				dwRegValIndex++;
			}
		}
		else
			ReferenceFichierTrace("DeleteUnusedRegKey(): Missmatch in pointer size");
	}
}


PRIVATE void CopyAllFileNamesInLIstInString(char *pcFileName, TpTTacheContext    Context)
{
	char	szListName[MAX_PATH + 1] = { 0 };

		ExtractListNameFromFileName(pcFileName, szListName, sizeof(szListName));
		strncat_s(Context->szFileNamesForComparingWithReg, sizeof(Context->szFileNamesForComparingWithReg), szListName, _TRUNCATE);
		strncat_s(Context->szFileNamesForComparingWithReg, sizeof(Context->szFileNamesForComparingWithReg), FILES_TOOLS_SEP_CHAR, _TRUNCATE);
}

PRIVATE void CopyAllMultiFileNamesInString(char *pcId, TpTTacheContext    Context)
{
	char	szListName[MAX_PATH + 1] = { 0 };
	BOOL	bMultiFiles = FALSE;

	if (strcmp(pcId, "TFT") != 0)
	{
		strcpy_s(Context->szCurrentFileId, sizeof(Context->szCurrentFileId), pcId);
		FileGetMultiFilesBoolean(pcId, &bMultiFiles);
		if (bMultiFiles)
		{
			if (strcmp(Context->szCurrentFileId, Context->szOldFileId) != 0)
			{
				strncat_s(Context->szMultiFilesNames, sizeof(Context->szMultiFilesNames), pcId, _TRUNCATE);
				strncat_s(Context->szMultiFilesNames, sizeof(Context->szMultiFilesNames), FILES_TOOLS_SEP_CHAR, _TRUNCATE);
				strcpy_s(Context->szOldFileId, sizeof(Context->szOldFileId), Context->szCurrentFileId);
			}
		}
	}
}