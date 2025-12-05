/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Module de traitement des fichiers de type TFT
* FICHIER: TFT.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier de reception des fichiers
* --------------------------------------------------------------------
* DESCRIPTION:
*             Ce module est utilis‚ avec le moteur de r‚ception de fichier
*             Il traite les fichiers dont l'identifiant est TFT.
*
*              Init: Cette fonction est appel‚e avant le main, pour ajouter
*                    les donn‚es dans la liste des fichiers attendus
*              Verifier: Cette fonction est appel‚ pour v‚rifier que le
*                        fichier est au bon format.
*                        Aucune operation autre que la v‚rification ne
*                        doit ˆtre faite.
*              XxxxFaire: Ces fonctions effectuent le travail de
*                         mise en oeuvre de fichiers.
* --------------------------------------------------------------------
* LECTURE DE LA TFT
*           Ce module utilise un fichier de lecture de TFT
* --------------------------------------------------------------------
* STRUCTURE DE DONNEES:
*           Les donn‚es utilis‚es par le moteur sont :
*           - un identifiant de type de fichier
*           - un chemin de copie des fichiers
*           - une fonction de v‚rification ( peut ˆtre  NULL)
*           - une fonction d'action ( peut ˆtre  NULL)
*           - une taille de paramŠtre, et une zone de m‚moire
*
*           - L'ID est l'identifiant du type de fichier.
*             Il est utilis‚ par le moteur pour faire le lien entre l'ID
*             contenu dans le fichier de r‚f‚rence et les traitements
*             a faire
*             Il est unique pour toute l'application.
*           - Le chemin de copie des fichiers est utilis‚ aprŠs la
*             v‚rification du fichier, pour installer le fichier re‡u
*             dans son repertoire de traitement
*
*             A NOTER que le nom des fichiers peut ˆtre le mˆme dans 2
*             fichiers de r‚f‚rence, mais que leur contenu soit diff‚rent.
*             Dans ce cas, les fonctions de mise en oeuvre doivent
*             g‚rer cela pour ‚viter les ‚crasements intempestifs.
*           - La fonction de V‚rification permet de v‚rifier que le fichier
*             est au bon format, et que la VM peut le prendre en compte.
*             Un message est envoy‚ aprŠs cet appel pour signaler au PCS
*             la validit‚ du fichier.
*             Si cette fonction n'existe pas, le fichier est pris
*             comme valide par le moteur.
*           - La fonction d'action est appel‚ pour faire la mise en oeuvre
*             du fichier. Elle peut armer des r‚veils, ou attendre
*             des messages d'autres taches.
*             Elle peut s'ex‚cuter normalement, et r‚pondre DONE, ou
*             ‚chouer temporairement ( ex: pas de r‚veil disponible)
*             et r‚pondre NOT DONE.
*             Si cette fonction n'existe pas, le moteur considŠre que
*             la fonction a r‚pondu DONE.
*
*             DŠs que pour cet identifiant et cette it‚ration, il n'y
*             a plus d'actions … faire, un message de mise en oeuvre
*             est envoy‚ au PCS
*           - La taille de paramŠtre permet au moteur de v‚rifier que
*             la zone allou‚e d'origine au paramŠtre est suffisante.
*             Cette zone permet d'utiliser un paramŠtre r‚manent
*             pour cette ID et cette it‚ration, qui contient les donn‚es
*             n‚cessaires au traitement.
*             Par exemple, il doit contenir les r‚veils qui ont ‚t‚ arm‚s,
*             pour pouvoir les effacer sur ABORT.
*
*             A NOTER, la zone memoire est fournie par le moteur, pendant
*             la dur‚e de l'appel … la fonction. Il est donc IMPOSSIBLE de
*             stocker son adresse, ni de choisir sa taille.
*
*             Si le paramŠtre n'est pas utile, sa taille d‚clar‚e … l'init
*             est 0.
*             Si des donn‚es doivent ˆtre partag‚es entre les diff‚rentes
*             it‚rations, celles-ci doivent ˆtre stock‚es dans une structure
*             locale.
* --------------------------------------------------------------------
* NOTA:
*      Il n'y pas de fichier .h, car le moteur accŠde aux fonctions et
*      aux donn‚es via les champs de la structure d‚clar‚e au d‚marrage
* --------------------------------------------------------------------
* NOTA:
*      La fonction ReceptionFaire peut ˆtre appel‚e sans que soit appeler
*      au pr‚alable la fonction Verifier. De mˆme, les it‚rations ne se
*      suivent pas obligatoirement.
* --------------------------------------------------------------------
* NOTA 2:
*        Il est possible que plusieurs it‚rations changent en mˆme temps,
*        mais que leurs dates d'application soient dans le pass‚
*        Il faut donc conserv‚ une information entre les it‚rations pour
*        d‚tecter celle qui doit ˆtre appliqu‚e.
*        De ce fait, toutes celles dont la date d'application est ant‚rieure
*        a la date du jour r‚pondent NOT_DONE dans la fonction
*        ReceptionFaire.
*
*        ATTENTION
*        Si aucune it‚ration n'est applicable, cela signifie que la VM
*        n'a PLUS du tout de TFT, et doit cesser de travailler
* --------------------------------------------------------------------
* PEMM:
*      Sous PEMM, ce fichier est employ‚ uniquement en page 1 avec le
*      module COM_LS
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/rfr_tft/Sources/tft.c_v  $
 * 
 *    Rev 1.3   22 Mar 2001 10:06:52   FR
 * - correction de l'absence de fermeture du nouveau fichier recu dans la fonction Verifier()
 * 
 *    Rev 1.2   31 Jan 2001 17:16:38   FR
 *  
 * 
 *    Rev 1.1   15 May 2000 19:18:04   afx
 * Creation repertoire destinataire a l'init.
 * 
 *    Rev 1.0   05 May 2000 18:05:50   afx
 * Version initiale.
 * 
 *    Rev 1.3   Jun 03 1997 17:38:48   DPI
 *  
 * 
 *    Rev 1.2   May 12 1997 13:55:00   DPI
 * 1- Gerer les fichiers manquant
 * 2- Locker POSTE en manuel
 * 
 *    Rev 1.1   May 06 1997 09:46:18   DPI
 *  
 * 
 *    Rev 1.0   Apr 28 1997 15:52:16   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <csrlc32.h>
#include <noyau.h>
#include <debug.h>
#include <run.h>
#include <reg.h>

#include <fic.h>
#include "str.h"

#define IGNORE_GENERIC_EXPORTS
#include <tft_modular_interface.h>
#undef IGNORE_GENERIC_EXPORTS

#include "rc_struc.h"

#include <csr_files_tools.h>

#include <tft.h>
#include<fcntl.h>
#include<share.h>
#include<sys\stat.h>

#define LOC_DEF
#include <tft_glob.h>
#undef LOC_DEF


/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- PARAMETRE: ---------------*/
/* cette structure est utilis‚e pour chaque it‚ration
 * MAIS chaque it‚ration a sa propre variable
 */
typedef struct
        {
           TIdAlarme       IdReveil;
           TIdMessage      IdMessage;
        } TTftParam, *TpTTftParam;

/* cette structure est utilis‚e pour chaque it‚ration
 * et la variable est partag‚e par toutes les it‚rations */
typedef struct
        {

			char				rfr_bal_name[MAX_PATH + 1];
			noyau_bal_id		rfr_bal_id;
			
			char				ID[MAX_TFT_ID + 1];
			char				source_path[MAX_PATH + 1];
			char				dest_path[MAX_PATH + 1];

        } TGlobalParam;

/* cette variable est globale a toutes les it‚rations, et permet
 * de d‚terminer quelle it‚ration doit ˆtre activ‚e
 */
static TGlobalParam           GlobParam;

/*--------------- METHODES: ---------------*/

PRIVATE void Enregistree(void);

PRIVATE TEnum_Verified    Verifier( TpTIdContext   IdContext,
                                    char           *Commentaire);
PRIVATE TEnum_Done        ReceptionFaire( TpTIdContext   IdContext,
                                          char           *Commentaire);
PRIVATE void EnvoiePoste (BOOL manual);

/*--------------- PRIVATE: ---------------*/

PRIVATE struct_rfr_record rfr_record = {0};


extern int APIENTRY DllMain (HINSTANCE hInstance, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason) 
    {  
        // The DLL is loading due to process 
        // initialization or a call to LoadLibrary.  
        case DLL_PROCESS_ATTACH:
           break;
        
        // The attached process creates a new thread.  
        case DLL_THREAD_ATTACH:  
            break; 

         // The thread of the attached process terminates. 
        case DLL_THREAD_DETACH:  
            break;  

        // The DLL unloading due to process termination or call to FreeLibrary. 
         case DLL_PROCESS_DETACH:  
            break;          

         default:             
            break;     
    }      
    
    return TRUE; 

    UNREFERENCED_PARAMETER(hInstance);     
    UNREFERENCED_PARAMETER(lpReserved); 
} 

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI MODLance ( char * pcKey, char * pcBalName, noyau_bal_id * piBalId )
 * PARAMETERS: IN char * pcKey           : Nom de la clé du registre où aller chercher les paramètres
 *                                         de lancement de l'instance.
 *             IN char * pcBalName       : Nom de la boite à lettre à creer pour l'instance
 *             OUT noyau_bal_id * piBalId: Pointe sur une variable recevant l'id de la boite à lettre
 *                                         créée pour l'instance (et servant d'identifiant d'instance.
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour démarrage
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{
	return TFTLance (pcKey, pcBalNam, piBalId);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI MODArret ( IN noyau_bal_id iBalId )
 * PARAMETERS: IN noyau_bal_id iBalId : Id de la bal identifiant l'instance
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré d'un module pour arret
 * --------------------------------------------------------------------
 */
EXPORT enum_instance_result WINAPI MODArret(noyau_bal_id iBalId)
{
	return TFTArret (iBalId);
}

/*--------------- FUNCTIONS: ---------------*/
EXPORT enum_instance_result WINAPI TFTLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId )
{

	noyau_enum_retour     cr_lance;
	
	DWORD		dwLen;
	DWORD		dwNum;
	DWORD		dwObl;
	char		pcStr[ MAX_PATH + 1 ];
	char		*RegKey = pcKey;


	// initialisation du mode trace 
	if( RfrTftInitTrace( pcBalNam ) != INST_INIT_OK )
		return INST_INIT_ERR_FICHIER_DEBUG;

	// Booleen obligatory
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_OBL, &dwObl ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry!	TFT obligatory");
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if (dwObl > 1) // 'cold start' doit etre 0 ou 1 
		{
			RfrTftFichierDebug("Error reading registry!	TFT obligatory");
			return INST_INIT_ERR_REGISTRE;
		}
	}	

	// Pool (reserved)
	dwLen = sizeof( pcStr );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_POOL, pcStr, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! RFR TFT Pool");
		return INST_INIT_ERR_REGISTRE;
	}

	// Save pool id 
	RFR_TFT.pool = NOYAU_GetPoolId( pcStr );

	// Priorité initiale max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_PRIO_INIT, &dwNum ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("CSR_TFT_INIT ***** CsrTftLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_INIT );
		return INST_INIT_ERR_REGISTRE;
	}

	// Save max thread priority 
	RFR_TFT.priorite_init = NOYAU_MapPriority( dwNum );

	// Priorité max
	if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_PRIO_MAX, &dwNum ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("CSR_TFT_INIT ***** CsrTftLance() => Erreur registre : [%s]:%s", pcKey, MOD_REG_KEYv_PRIO_MAX );
		return INST_INIT_ERR_REGISTRE;
	}

	// Save max thread priority 
	RFR_TFT.priorite_max = NOYAU_MapPriority( dwNum );

	
	// Name of Reference mailbox
	dwLen = sizeof( GlobParam.rfr_bal_name );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_RFR_BAL, GlobParam.rfr_bal_name, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! TFT RFR BAL");
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Name of csr_tft mailbox
	dwLen = sizeof( RFR_TFT.csr_tft_bal_name );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_LA_BAL, RFR_TFT.csr_tft_bal_name, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! CSR_TFT BAL NAME");
		return INST_INIT_ERR_REGISTRE;
	}

	// ID for TFT
	dwLen = sizeof( GlobParam.ID );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_ID, GlobParam.ID, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! TFT ID");
		return INST_INIT_ERR_REGISTRE;
	}

	// Source path -- send to Reference module as location for copying TFT files
	dwLen = sizeof( GlobParam.source_path );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_TEMP_PATH, GlobParam.source_path, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! TFT TEMP PATH");
		return INST_INIT_ERR_REGISTRE;
	}

	// Dest path 
	dwLen = sizeof( GlobParam.dest_path );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_DEST_PATH, GlobParam.dest_path, &dwLen ) != ERROR_SUCCESS )
	{
		RfrTftFichierDebug("Error reading registry! TFT DEST PATH");
		return INST_INIT_ERR_REGISTRE;
	}
    else
    {
        if (_access(GlobParam.dest_path, 0) != 0)
            FIC_CreatePath(GlobParam.dest_path);
    }

	RFR_TFT.csr_tft_bal_id = AttendBAL(RFR_TFT.csr_tft_bal_name);
	GlobParam.rfr_bal_id = AttendBAL(GlobParam.rfr_bal_name);

	if (RFR_TFT.csr_tft_bal_id<=0) 
				return INST_INIT_ERR_LANCE;
	if (GlobParam.rfr_bal_id<=0) 
				return INST_INIT_ERR_LANCE;


	strcpy_s(rfr_record.IdName, sizeof(rfr_record.IdName), GlobParam.ID);
	strcpy_s(rfr_record.Path, sizeof(rfr_record.Path), GlobParam.source_path);
	rfr_record.Iterable = TRUE /*TRUE pour gestion multi-TFT*/;
	rfr_record.Obligatoire = (dwObl == 1);
	rfr_record.Enregistree = Enregistree;
	rfr_record.Verifier = Verifier;
	rfr_record.Faire = ReceptionFaire;
	rfr_record.TailleArgs = sizeof (TTftParam);

	// Sauvegarder nom BAL pour la publier apres enregistrement du module
	strcpy_s(RFR_TFT.rfr_tft_bal_name, sizeof(RFR_TFT.rfr_tft_bal_name), pcBalNam);
	*piBalId = -1; // BAL non valide

	// Enregistrement du module applicatif TFT dans le module de Reference 
	// ATTENTION LA STRUCTURE RFR_RECORD DOIT ETRE STATIQUE (=pas locale a cette fonction) 
	if (!RFRNewTableauId (&rfr_record))
		return INST_INIT_ERR_LANCE;
	
	
	strcpy_s(RFR_TFT.NewFiles, sizeof(RFR_TFT.NewFiles), "");

	// Tache RFR_TFT
	sprintf_s(pcStr, sizeof(pcStr), "RFR_TFT (%s)", pcBalNam);
	NOYAU_INIT_TACHE(
			RFR_TFT.taches[0],
			TRUE,
			RFR_TFT.priorite_max,
			2048,
			(LPTHREAD_START_ROUTINE)(RfrTft),
			NULL,
			NULL,
            pcStr );

	// Plus d'autre tache
	NOYAU_VIDE_TACHE( RFR_TFT.taches[1] );

	// lancement 
	cr_lance = LanceTache( RFR_TFT.taches );
	if (cr_lance != NOYAU_OK)
		return INST_INIT_ERR_LANCE;
	
    // Initialisation des BAL du module 
    *piBalId = AttendBAL( pcBalNam );
    if( *piBalId <= 0 )
    {
		RfrTftFichierDebug( "RfrTftLance: Pb InitBal() pour RFR_TFT");
		return INST_INIT_ERR_LANCE;
    }
	
	// redirection trace 
	RfrTftFichierTrace("RFR_TFT_INIT: TFTLance() => init OK ");
	
	return INST_INIT_OK;
}

EXPORT enum_instance_result WINAPI TFTArret(noyau_bal_id iBalId)
{
	return INST_ARRET_OK;
}

PRIVATE void Enregistree(void)
{
	RfrTftFichierTrace("RFR_TFT : Publication BAL %s", RFR_TFT.rfr_tft_bal_name);

	// Publier la BAL apres enregistrement aupres du module REFERENCE
	RFR_TFT.rfr_tft_bal_id = PublieBAL (RFR_TFT.rfr_tft_bal_name, NOYAU_BAL_ILLIMITEE);
	if (RFR_TFT.rfr_tft_bal_id <= 0)
      ExitBad();

	// opening FILE service of CSR_TFT
	EnvoiCsrTft(TFT_DEBUT);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   Verifier
* PARAMETRES: - le contexte de l'appel
*             - le commentaire en retour
* RETOUR:     - ACK si le fichier est correct
*             - NACK si le fichier est incorrect
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Methode
* ROLE: V‚rifie que le fichier nouveau est bien au format attendu.
*       Le commentaire en retour est transmis au PCS.
*
*       Il n'est pas utile de tester la raison d'appel, car elle est
*       forc‚ment a VERIFY.
* --------------------------------------------------------------------
* NOTA: Le fichier se trouve dans un r‚pertoire de r‚ception
*       Il n'est copi‚ dans le r‚pertoire d‚sign‚ que si la v‚rification
*       r‚ussit.
* --------------------------------------------------------------------
* $F_FCTN
*/

PRIVATE TEnum_Verified    Verifier( TpTIdContext   IdContext,
                                    char           *Commentaire)
{

	int hFile = FIC_open(IdContext->NouveauFichier, _O_RDWR, _S_IREAD | _S_IWRITE);



   /* Opening file to extract file size */
   if (hFile == INVALID_HANDLE_VALUE)
   {
        STR_strncpy( RFR_MAX_COMMENTAIRE, Commentaire, "File not readable");
        return( NACK);
   }
   
   /* Reading file size */
    if (FIC_filelength(hFile) == 0)
    {
       FIC_close(hFile);	// FR 21/03/01

       STR_strncpy( RFR_MAX_COMMENTAIRE, Commentaire, "Incoherent tft");
       return( NACK);
    }
   
	/* Closing file */
   if (FIC_close(hFile) != 0)
   {
       STR_strncpy( RFR_MAX_COMMENTAIRE, Commentaire, "File not closable");
       return( NACK);
   }

    return( ACK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   ReceptionFaire
* PARAMETRES: - le contexte de l'appel
*             - le commentaire en retour
* RETOUR:     - DONE si la fonction a fait tout ce qu'elle voulait faire
*             - NOT_DONE sinon
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Methode
* ROLE: Participe … la mise en oeuvre du fichier contenu dans le contexte
*       d'appel.
*       Cette mise en oeuvre peut n‚c‚ssiter l'attente d'un message ou d'un
*       r‚veil.
*       Le commentaire en retour est transmis au PCS si la mise en oeuvre
*       est termin‚e.
*       Si la fonction retourne NOT_DONE, elle sera rappel‚ de nouveau,
*       aprŠs un d‚lai variable.
*
*       A NOTER, si la fonction ne peut finir son travail,( ex: il n' y
*                a pas de r‚veil disponible), elle doit d‚faire ce qu'elle
*                a fait, puis retourner NOT_DONE
*
*       Cette fonction est appeler aprŠs la reception du fichier par le
*       moteur
* --------------------------------------------------------------------
* NOTA: 
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE TEnum_Done        ReceptionFaire( TpTIdContext   IdContext,
                                          char           *Commentaire)
{
   TpTTftParam          param;
   //enum_files_return	retval;	
      
   char			  new_tft_drive[MAX_PATH];
   char			  new_tft_dir[MAX_PATH];	
   char			  new_tft_file[MAX_PATH];
   char			  new_tft_ext[MAX_PATH];	

   char			  TftDestFile[MAX_PATH];
   char			  TftSrcFile[MAX_PATH];

   
   // Checking if call was properly placed
   if(_stricmp( IdContext->Id, GlobParam.ID) != 0)
   {
	   RfrTftFichierTrace("Faire : id ='%s', it ='%d', ch='%d'\n",
           IdContext->Id, IdContext->Iteration, IdContext->Change);
     return( DONE);
   }


   //-------------------------------------------------------------
   RfrTftFichierTrace("Faire : id ='%s', it ='%d', ch='%d' ",
           IdContext->Id, IdContext->Iteration, IdContext->Change);


   param = ( TpTTftParam) IdContext->BufferParam;

   //initialiser la structure parametre 
   param->IdReveil  = NO_ALARME;
   param->IdMessage = NO_MESSAGE;

   /* ---------------------------------------------------------
    * if new reference file arrived with new TFT file we stop
    * processing the old TFT file
    * 
    */
   if( IdContext->AppelRaison == ABORT)
   {
	   RfrTftFichierTrace(" ABORT\n", IdContext->Change);

      return( DONE);
   }

   // ---------------------------------------------------------
   // if there is an error we stop processing
   if( IdContext->AppelRaison == RC_ERROR)
   {
	   RfrTftFichierTrace(" ERROR\n", IdContext->Change);

      return( DONE);
   }

   	// if application is starting check if dir exists
   if( IdContext->AppelRaison == START)
   {
	   if (_access(GlobParam.dest_path, 0) != 0)
	   {
		   RfrTftFichierTrace("_access: %s NOT_DONE\n", GlobParam.dest_path);
		   return NOT_DONE;
	   }
	   else
	   {
		   RfrTftFichierTrace("_access: %s DONE\n", GlobParam.dest_path);
		   return DONE;
	   }
   }   

     // notification in case file was created by application 
   if( IdContext->AppelRaison == MANUAL)
	   RfrTftFichierTrace("Faire : id ='%s', it ='%d', ch='%d' MANUAL\n",
			IdContext->Id, IdContext->Iteration, IdContext->Change);



   //************************************************
   //RfrTftFichierTrace(" DO_IT\n", IdContext->Change);

	if (IdContext->Iteration == BEFORE_ITERATIONS)
	{
		strcpy_s(RFR_TFT.NewFiles, sizeof(RFR_TFT.NewFiles), "");
		return DONE;
	}

	if(IdContext->Iteration == AFTER_ITERATIONS)
	{
		EnvoiCsrTft(TFT_NEW_FILE);
		return DONE;
	}

	RfrTftFichierTrace("Faire : id ='%s', it ='%d', ch='%d' MANUAL NouveauFichier[%s]\n",
		IdContext->Id, IdContext->Iteration, IdContext->Change, IdContext->NouveauFichier);

   FIC_splitpath(IdContext->NouveauFichier, new_tft_drive, new_tft_dir, new_tft_file, new_tft_ext);	

   // determine source file - if NouveauFichier is passed with drive and directory use that file
   // else create filename from temp dir. + filename
   if (strcmp(new_tft_drive, "\0") == 0)
		FIC_makepath(TftSrcFile, GlobParam.source_path, NULL, new_tft_file, new_tft_ext);
   else
	   strcpy_s(TftSrcFile, sizeof(TftSrcFile), IdContext->NouveauFichier);

   FIC_makepath(TftDestFile, GlobParam.dest_path, NULL, new_tft_file, new_tft_ext);

   if (_access(TftDestFile, 0) == 0)	{
       // FR 31/01/01 : preciser au module REFERENCE que le fichier est deja present
       STR_strncpy( RFR_MAX_COMMENTAIRE, Commentaire, RFR_FILE_ALREADY_PRESENT);
	   return(DONE);
   }
   
   if (FIC_copy(TftDestFile, TftSrcFile, FIC_COPY_WITH_DATE))
   {
	   FIC_makepath(TftDestFile, NULL, NULL, new_tft_file, new_tft_ext);
	   if (strcmp(RFR_TFT.NewFiles, "") == 0)
		   strcpy_s(RFR_TFT.NewFiles, sizeof(RFR_TFT.NewFiles), TftDestFile);
		else
		{
			strcat_s(RFR_TFT.NewFiles, sizeof(RFR_TFT.NewFiles), ";");
			strcat_s(RFR_TFT.NewFiles, sizeof(RFR_TFT.NewFiles), TftDestFile);
		}
			
   }
   else
	   return (NOT_DONE);

   return( DONE );
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_instance_result RfrTftInitTrace (char * pcBal)
* PARAMETRES:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: 
* ROLE: 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_instance_result RfrTftInitTrace (char * pcBal )
{
    DWORD dwLen;
    DWORD dwTailleMax;
    char pcKey[MAX_PATH];
	dbg_struct_debug *debug;
	dbg_struct_trace *tab_traces;

	debug = &RFR_TFT.dbg;
	tab_traces = &RFR_TFT.tab_traces[RFR_TFT_TRC];

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);

  	// chemin traces
    dwLen = sizeof( debug->rep_fichiers_traces );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, debug->rep_fichiers_traces, &dwLen ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;

  	// chemin erreurs
    dwLen = sizeof( debug->rep_fichier_erreurs );
	if ( REG_Lire_Chaine( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, debug->rep_fichier_erreurs, &dwLen ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
    // Taille max des fichiers
    if ( REG_Lire_Entier( CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax ) != ERROR_SUCCESS )
		return INST_INIT_ERR_FICHIER_DEBUG;
	debug->taille_limite = dwTailleMax;
	    
	strcpy_s(debug->nom_fichier_traces_ecran, sizeof(debug->nom_fichier_traces_ecran), pcBal);

	strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), pcBal);

	strcpy_s(tab_traces->nom, sizeof(tab_traces->nom), pcBal);

	debug->tab_traces = tab_traces;
	debug->nb_fichiers_traces = RFR_TFT_NB_TRACES;
	
	if( DBG_Lance(debug) != DBG_OK )
		return INST_INIT_ERR_FICHIER_DEBUG;
	
	return INST_INIT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void RfrTftFichierTrace (char *fmt,...)
* PARAMETRES:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: 
* ROLE: 
* --------------------------------------------------------------------
* $F_FCTN
*/
#define MAX_STRING 16384

PROTECTED void RfrTftFichierTrace (char *fmt,...)
{
	va_list args;
	char szString[MAX_STRING];

	va_start(args, fmt);
	_vsnprintf_s(szString, sizeof(szString), sizeof(szString), fmt, args);
	
	DBG_EcritFichierTraces(RFR_TFT_TRC, &RFR_TFT.dbg, szString);
	
	va_end (args);
	
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void DEFINE_RfrTftFichierDebug (char *fmt,...)
* PARAMETRES:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: 
* ROLE: 
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DEFINE_RfrTftFichierDebug (char *fmt,...)
{
	va_list args;
	char string[200];
	
	va_start (args,fmt);
	vsprintf_s(string, sizeof(string), fmt, args);
	
	DBG_FILE = RFR_TFT_FILE;
	DBG_LINE = RFR_TFT_LINE;
	
	DEFINE_DBG_EcritFichierErreurs(RFR_TFT_TRC, &RFR_TFT.dbg, string);
	
	va_end (args);
}
