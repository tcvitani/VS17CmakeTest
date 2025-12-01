/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Module de traitement des fichiers de type RFR GENERIC
* FICHIER: RFR GENERIC.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier de reception des fichiers
* --------------------------------------------------------------------
* DESCRIPTION:
*             Ce module est utilis‚ avec le moteur de r‚ception de fichier
*             Il traite les fichiers dont l'identifiant est RFR GENERIC.
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
* LECTURE DE LA RFR GENERIC
*           Ce module utilise un fichier de lecture de RFR GENERIC
* --------------------------------------------------------------------
* STRUCTURE DE DONNEES:
*           Les donn‚es utilis‚es par le moteur sont :
*           - un identifiant de type de fichier
*           - un chemin de copie des fichiers
*           - une fonction de v‚rification (peut ˆtre  NULL)
*           - une fonction d'action (peut ˆtre  NULL)
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
*             ‚chouer temporairement (ex: pas de r‚veil disponible)
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
*        n'a PLUS du tout de RFR GENERIC, et doit cesser de travailler
* --------------------------------------------------------------------
* PEMM:
*      Sous PEMM, ce fichier est employ‚ uniquement en page 1 avec le
*      module COM_LS
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/PRODUITS/Voie_NT/Europe/CCIH/Reference/ocm/Sources/ocm.c_v  $
 * 
 *    Rev 1.0   16 May 2001 17:41:48   FROUGIET
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.0   16 May 2001 10:05:02   FROUGIET
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.4   May 05 2000 18:05:02   gferrado
 *  
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
#include <trc.h>
#include <csr_list.h>

#include <fic.h>
#include <str.h>

#include <rc_struc.h>

#include <rfr_generic.h>

#include <csr_files_tools.h>

#include <rfr_appli.h>
#include <io.h>
#include<fcntl.h>
#include<share.h>
#include<sys\stat.h>

/*--------------- RESERVED: ---------------*/

#include <memclass.h>

/*--------------- PARAMETRE: ---------------*/
/* cette structure est utilis‚e pour chaque it‚ration
 * MAIS chaque it‚ration a sa propre variable
 */
typedef struct
        {
           TIdAlarme       IdReveil;
           TIdMessage      IdMessage;
        } TOcmParam, *TpTOcmParam;

/* cette structure est utilis‚e pour chaque it‚ration
 * et la variable est partag‚e par toutes les it‚rations */
typedef struct
        {
			char				bl_name[MAX_PATH];
			char				la_bal_name[MAX_PATH + 1];
			char				rfr_bal_name[MAX_PATH + 1];
			noyau_bal_id		la_bal_id;
			noyau_bal_id		rfr_bal_id;
			char				ID[MAX_RFR_GEN_ID + 1];
			char				source_path[MAX_PATH + 1];
			noyau_pool_id       pool;
			BOOL				allow_empty_file;
			LONG				file_id;
			dbg_struct_debug	dbg;
			dbg_struct_trace	sTraces;

        } struct_instance_data, *P_INSTANCE_DATA;

/* cette variable est globale a toutes les it‚rations, et permet
 * de d‚terminer quelle it‚ration doit ˆtre activ‚e
 */
PRIVATE HLIST instances = NULL;

/*--------------- METHODES: ---------------*/

PRIVATE void Enregistree(void);
PRIVATE void EnregistreeEx(TpTRfrRecord p_record);

PRIVATE TEnum_Verified    Verifier(TpTIdContext IdContext, char* Commentaire);
PRIVATE TEnum_Done        ReceptionFaire(TpTIdContext IdContext, char* Commentaire);
PRIVATE void EnvoiePoste(noyau_bal_id mbx_dest, noyau_bal_id mbx_src, noyau_pool_id pool, LONG file_id, BOOL manual);

PRIVATE BOOL Instance_CreateList(HLIST* pList_instances);
PRIVATE BOOL Instance_Find(HLIST list_instances, PSTR id, P_INSTANCE_DATA instance);
PRIVATE BOOL Instance_Add(HLIST list_instances, P_INSTANCE_DATA instance);


#define RfrGenericFichierDebug RFR_GENERIC_FILE=__FILE__,\
                        RFR_GENERIC_LINE=__LINE__,\
                        DEFINE_RfrGenericFichierDebug

PROTECTED char *RFR_GENERIC_FILE;
PROTECTED int RFR_GENERIC_LINE;

PROTECTED enum_instance_result RfrGenericInitTrace(struct_instance_data *p_instance_data, char * pcBal);
PROTECTED void DEFINE_RfrGenericFichierDebug(struct_instance_data *p_instance_data, char *fmt, ...);
PROTECTED void RfrGenericFichierTrace(struct_instance_data *p_instance_data, char *fmt, ...);
PROTECTED enum_instance_result RfrGenericDeinitTrace(struct_instance_data *p_instance_data);

/*--------------- PRIVATE: ---------------*/

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
 * SYNTAX    : EXPORT DWORD WINAPI MODLance (char * pcKey, char * pcBalName, noyau_bal_id * piBalId)
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
EXPORT enum_instance_result WINAPI MODLance(char * pcKey, char * pcBalNam, noyau_bal_id * piBalId)
{
	return RfrGenericLance (pcKey, pcBalNam, piBalId);
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT enum_instance_result WINAPI MODArret (IN noyau_bal_id iBalId)
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
	return RfrGenericArret (iBalId);
}

/*--------------- FUNCTIONS: ---------------*/
EXPORT enum_instance_result WINAPI RfrGenericLance(char * pcKey, char * pcBalNam, noyau_bal_id * piBalId)
{
	DWORD    dwLen;
	DWORD	 dwObl;
	char     *RegKey = pcKey;
	struct_rfr_record rfr_record = {0};
	DWORD	 dwTemp = 0;
	BOOL	bIterable = FALSE;

	struct_instance_data instance_data = { 0 };

	// initialisation du mode trace 
	if (RfrGenericInitTrace(&instance_data, pcBalNam) != INST_INIT_OK)
		return INST_INIT_ERR_FICHIER_DEBUG;

	if(instances == NULL)
	{
		if(Instance_CreateList(&instances) == FALSE)
		{
			RfrGenericFichierDebug(&instance_data, "RfrGenericLance: Failed creating instances data list! %s ", pcBalNam);
			return INST_INIT_ERR_LANCE;
		}
	}



	// Booleen obligatory
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_OBL, &dwObl) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	%s obligatory", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	else
	{
		if(dwObl > 1) /* 'cold start' doit etre 0 ou 1 */
		{
			RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	RFR GENERIC obligatory");
			return INST_INIT_ERR_REGISTRE;
		}
	}
	
	// Pool (reserved)
	dwLen = sizeof(instance_data.pool);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_POOL, instance_data.pool, &dwLen) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	%s Pool", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Name of Reference mailbox
	dwLen = sizeof(instance_data.rfr_bal_name);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_RFR_BAL, instance_data.rfr_bal_name, &dwLen) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry! %s RFR BAL", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Name of Lane app. mailbox
	dwLen = sizeof(instance_data.la_bal_name);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_LA_BAL, instance_data.la_bal_name, &dwLen) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry! %s LA BAL", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}

	// ID for RFR GENERIC
	dwLen = sizeof(instance_data.ID);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_ID, instance_data.ID, &dwLen) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry! %s ID", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	
	// Source path -- send to Reference module as location for copying RFR GENERIC files
	dwLen = sizeof(instance_data.source_path);
	if(REG_Lire_Chaine(CSR_REG_KEYi_ROOT, RegKey, MOD_REG_KEYv_TEMP_PATH, instance_data.source_path, &dwLen) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry! %s TEMP PATH", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	
	// File can be empty
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_ALLOW_EMPTY, &instance_data.allow_empty_file) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	%s ALLOW EMPTY", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}
	
	// File id
	if(REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_FILE_ID, &instance_data.file_id) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	%s FILE ID", pcBalNam);
		return INST_INIT_ERR_REGISTRE;
	}

	// Booleen iterable
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, MOD_REG_KEYv_ITERABLE, &dwTemp) != ERROR_SUCCESS)
	{
		RfrGenericFichierDebug(&instance_data, "RfrGenericLance:Error reading registry!	%s Iterable", pcBalNam);
		bIterable = FALSE;
	}
	else
	{
		if (dwTemp > 0)
			bIterable = TRUE;
	}

	instance_data.la_bal_id=AttendBAL(instance_data.la_bal_name);
	instance_data.rfr_bal_id=AttendBAL(instance_data.rfr_bal_name);

	if(instance_data.la_bal_id<=0)
		return INST_INIT_ERR_LANCE;
	if(instance_data.rfr_bal_id<=0) 
		return INST_INIT_ERR_LANCE;

	// Sauvegarder nom BAL pour la publier apres enregistrement du module
	strcpy_s(instance_data.bl_name, sizeof(instance_data.bl_name), pcBalNam);
	*piBalId = -1; // BAL non valide

	
	if(Instance_Add(instances, &instance_data) == FALSE)
	{
		return INST_INIT_ERR_LANCE;
	}

	strcpy_s(rfr_record.IdName, sizeof(rfr_record.IdName), instance_data.ID);
	strcpy_s(rfr_record.Path, sizeof(rfr_record.Path), instance_data.source_path);
	rfr_record.Iterable = bIterable;//FALSE /*TRUE pour gestion multi-RFR GENERIC*/;
	rfr_record.Obligatoire = (dwObl == 1);
	rfr_record.Enregistree = Enregistree;
	rfr_record.EnregistreeEx = EnregistreeEx;
	rfr_record.Verifier = Verifier;
	rfr_record.Faire = ReceptionFaire;
	rfr_record.TailleArgs = sizeof (TOcmParam);

	/* Enregistrement du module applicatif RFR GENERIC dans le module de Reference */
	/* ATTENTION LA STRUCTURE RFR_RECORD DOIT ETRE STATIQUE (=pas locale a cette fonction) */
	if(!RFRNewTableauId (&rfr_record))
		return INST_INIT_ERR_LANCE;

	return INST_INIT_OK;
}

EXPORT enum_instance_result WINAPI RfrGenericArret(noyau_bal_id iBalId)
{
	return INST_ARRET_OK;
}

PRIVATE void Enregistree(void)
{
}

PRIVATE void EnregistreeEx(TpTRfrRecord p_record)
{
	struct_instance_data instance_data = {0};

	if(Instance_Find(instances, p_record->IdName, &instance_data) == TRUE)
	{
		noyau_bal_id iBalId;
		// Publier la BAL apres enregistrement aupres du module REFERENCE
		iBalId = PublieBAL (instance_data.bl_name, NOYAU_BAL_ILLIMITEE);
		if(iBalId <= 0)
		{
			ExitBad();
		}
	}
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

PRIVATE TEnum_Verified Verifier(TpTIdContext IdContext, char *Commentaire)
{
	int hFileHandle = 0;
	struct_instance_data instance_data = {0};
	errno_t err;

	Instance_Find(instances, IdContext->Id, &instance_data);

	/* Opening file to extract file size */
	hFileHandle = FIC_open(IdContext->NouveauFichier, _O_RDWR, _S_IREAD | _S_IWRITE);

	if(hFileHandle == 0)
	{
		STR_strncpy(RFR_MAX_COMMENTAIRE, Commentaire, "File not readable");
		return(NACK);
	}

	/* Reading file size */
	if(FIC_filelength(hFileHandle) == 0 && instance_data.allow_empty_file == FALSE)
	{		
		sprintf_s(Commentaire, RFR_MAX_COMMENTAIRE, "Incoherent %s", IdContext->Id);
	   FIC_close(hFileHandle);
	   return(NACK);
	}

	/* Closing file */
	if(FIC_close(hFileHandle) != 0)
	{
	   STR_strncpy(RFR_MAX_COMMENTAIRE, Commentaire, "File not closable");
	   return(NACK);
	}

    return(ACK);
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
*       A NOTER, si la fonction ne peut finir son travail,(ex: il n' y
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
PRIVATE TEnum_Done ReceptionFaire(TpTIdContext IdContext, char *Commentaire)
{
   TpTOcmParam          param;
   char				    CurrentFile[MAX_PATH] = {0};		
   enum_files_return	retval;	
      
   char	 new_drive[MAX_PATH]	= {0};
   char	 new_dir[MAX_PATH]		= {0};	
   char	 new_file[MAX_PATH]		= {0};
   char	 new_ext[MAX_PATH]		= {0};	

   char	 dest_file[MAX_PATH]	= {0};
   char	 src_file[MAX_PATH]		= {0};
   char	 path[MAX_PATH]			= {0};

   struct_instance_data instance_data = {0};

   if(Instance_Find(instances, IdContext->Id, &instance_data) == FALSE)
   {
	   RfrGenericFichierDebug(&instance_data, "ReceptionFaire: Faire : id = '%s', it = '%d', ch = '%d'\n", IdContext->Id, IdContext->Iteration, IdContext->Change);
	   return(DONE);
   }

   //-------------------------------------------------------------
   RfrGenericFichierDebug(&instance_data, "ReceptionFaire: Faire : id ='%s', it ='%d', ch='%d' ", IdContext->Id, IdContext->Iteration, IdContext->Change);


   param = (TpTOcmParam) IdContext->BufferParam;

   //initialiser la structure parametre 
   param->IdReveil  = NO_ALARME;
   param->IdMessage = NO_MESSAGE;

   /* ---------------------------------------------------------
    * if new reference file arrived with new RFR GENERIC file we stop
    * processing the old RFR GENERIC file
    * 
    */
   if(IdContext->AppelRaison == ABORT)
   {
	   RfrGenericFichierDebug(&instance_data, "ReceptionFaire:  ABORT\n", IdContext->Change);

      return(DONE);
   }

   // ---------------------------------------------------------
   // if there is an error we stop processing
   if(IdContext->AppelRaison == RC_ERROR)
   {
	   RfrGenericFichierDebug(&instance_data, "ReceptionFaire:  ERROR\n", IdContext->Change);

      return(DONE);
   }

   	// if application is starting check if current file exists
   // if current file exists or reg entry for current is blank return OK
   if(IdContext->AppelRaison == START)
   {
	   retval = FileGetFullNameCurrent(instance_data.ID, CurrentFile);
	   if((retval == FILE_BLANK_DATA) || (retval == FILE_OK))
		   return (DONE);
	   else 
   	       return(NOT_DONE);
   }   

     // notification in case file was created by application 
   if(IdContext->AppelRaison == MANUAL)
	   RfrGenericFichierDebug(&instance_data, "ReceptionFaire: Faire : id ='%s', it ='%d', ch='%d' MANUAL\n", IdContext->Id, IdContext->Iteration, IdContext->Change);


   //************************************************
   RfrGenericFichierDebug(&instance_data, "ReceptionFaire:  DO_IT\n", IdContext->Change);

   FIC_splitpath(IdContext->NouveauFichier, new_drive, new_dir, new_file, new_ext);	

   // determine source file - if NouveauFichier is passed with drive and directory use that file
   // else create filename from temp dir. + filename
   if(strcmp(new_drive, "\0") == 0)
		FIC_makepath(src_file, instance_data.source_path, NULL, new_file, new_ext);
   else
	   strcpy_s(src_file, sizeof(src_file), IdContext->NouveauFichier);

   // determine destination file - if there is an error with get specific path
   // return NOT_DONE
   if(FileGetSpecificPath(instance_data.ID, path) != FILE_OK)
	   return (NOT_DONE);
   
   FIC_makepath(dest_file, path, NULL, new_file, new_ext);

   if(_access(dest_file, 0) == 0)
   {
	   STR_strncpy(RFR_MAX_COMMENTAIRE, Commentaire, RFR_FILE_ALREADY_PRESENT);
	   return(DONE);
   }

   if(FIC_copy(dest_file, src_file, FIC_COPY_WITH_DATE))
   {
	   FIC_makepath(dest_file, NULL, NULL, new_file, new_ext);
	   FileSetNew(instance_data.ID, dest_file);
	   EnvoiePoste(instance_data.la_bal_id, instance_data.rfr_bal_id, instance_data.pool, instance_data.file_id, IdContext->AppelRaison == MANUAL);
   }
   else{
	   sprintf_s(Commentaire, RFR_MAX_COMMENTAIRE, "%s File Not copied", instance_data.ID);
	   return (NOT_DONE);
	}

   return(DONE);
}

/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/

/* --------------------------------------------------------------------
* SYNTAXE: PRIVATE void EnvoiePoste (BOOL manual)
*
* PARAMETRES:
*     entree: 
*     retour: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Local function
* ROLE: message emission to POSTE (lane application)
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void EnvoiePoste(noyau_bal_id mbx_dest, noyau_bal_id mbx_src, noyau_pool_id pool, LONG file_id,  BOOL manual)
{
	struct_rfr_appli_msg *p_msg = NULL;
	

	/* memory allocation for the message */
	ExitAlloue ((struct_neutre **)(&p_msg),sizeof(struct_rfr_appli_msg), pool);

	/* service sent to POSTE */
	p_msg->entete.service = M_RFR_APPLI_FILE;
	p_msg->entete.type = RFR_APPLI_NEW;
	p_msg->u.file.file_id = file_id;
	p_msg->u.file.manual = manual;
	
	/* send message to mailbox */
	ExitEnvoie (mbx_dest, mbx_src,(struct_neutre *)p_msg);
}

PRIVATE BOOL Instance_Find(HLIST list_instances, PSTR id, P_INSTANCE_DATA instance)
{
	BOOL result = FALSE;

	if(instances != NULL)
	{
		P_INSTANCE_DATA item;
		LONG count = List_GetCount(list_instances);

		for(item = List_GetHead(list_instances); item != NULL; item = List_GetNext(list_instances, item))
		{
			if(strcmp(item->ID, id) == 0)
			{
				memcpy(instance, item, sizeof(struct_instance_data));
				result = TRUE;
				break;
			}
		}
	}

	return result;
}

PRIVATE BOOL Instance_Add(HLIST list_instances, P_INSTANCE_DATA instance)
{
	P_INSTANCE_DATA item;
	BOOL result = FALSE;

	if(list_instances != NULL)
	{
		item = List_ItemNew(list_instances, sizeof(struct_instance_data));
		if(item != NULL)
		{
			memcpy(item, instance, sizeof(struct_instance_data));

			List_AddTail(list_instances, item);
			result = TRUE;
		}
	}

	return result;
}

PRIVATE BOOL Instance_CreateList(HLIST* pList_instances)
{
	BOOL result;

	*pList_instances = List_New();

	result = (*pList_instances != NULL)?(TRUE):(FALSE);

	return result;
}


//---------------------------------------------------------------------------


PRIVATE int CreatePath(char *path)
{
	char *token;
	char full_path[_MAX_PATH];


	if (_fullpath(full_path, path, _MAX_PATH) == NULL)
		return FALSE;

	// recherche du premier '\' normalement juste apres le ':' avec un path absolu
	token = strchr(full_path, '\\');
	token++;

	// création des sous-répertoires si inexistant
	while ((token = strchr(token, '\\')) != NULL)
	{
		*token = '\0';

		if (_access(full_path, 0) != 0)
			if (!CreateDirectoryA(full_path, NULL))
			{
				*token = '\\';
				return FALSE;
			}

		*token = '\\';
		token++;
	}

	// création du répertoire final si inexistant
	if (_access(full_path, 0) != 0)
		if (!CreateDirectoryA(full_path, NULL))
			return FALSE;

	return TRUE;
}


PROTECTED enum_instance_result RfrGenericInitTrace(struct_instance_data *p_instance_data, char * pcBal)
{
	DWORD dwLen;
	DWORD dwTailleMax;
	char pcKey[MAX_PATH];
	dbg_struct_debug *debug;
	dbg_struct_trace *tab_traces;
	char				path[MAX_PATH] = { 0 };

	debug = &p_instance_data->dbg;
	tab_traces = &p_instance_data->sTraces;

	sprintf_s(pcKey, sizeof(pcKey), "%s%s%s", CSR_REG_KEYn_CSRBASE, CSR_REG_KEYn_LANE_BASE, CSR_REG_KEYn_CONFIG);

	// chemin traces
	dwLen = sizeof(debug->rep_fichiers_traces);
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_TRACEPATH, debug->rep_fichiers_traces, &dwLen) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;

	// chemin erreurs
	dwLen = sizeof(debug->rep_fichier_erreurs);
	if (REG_Lire_Chaine(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_ERRORPATH, debug->rep_fichier_erreurs, &dwLen) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;

	// Taille max des fichiers
	if (REG_Lire_Entier(CSR_REG_KEYi_ROOT, pcKey, CSR_REG_KEYv_FILEMAXSIZE, &dwTailleMax) != ERROR_SUCCESS)
		return INST_INIT_ERR_FICHIER_DEBUG;
	debug->taille_limite = dwTailleMax;

	/* creation du chemin complet vers le fichier d'erreurs */
	_snprintf_s(path, sizeof(path), sizeof(path), "%s\\%s" NOYAU_EXTENSION_ERREURS, debug->rep_fichier_erreurs, pcBal);
	strcpy_s(debug->nom_fichier_erreurs, sizeof(debug->nom_fichier_erreurs), path);

	/* creation du chemin complet vers le fichier de traces avec ajout de l'extension par defaut */
	_snprintf_s(tab_traces->nom, sizeof(tab_traces->nom), sizeof(tab_traces->nom), "%s\\%s" NOYAU_EXTENSION_TRACES, debug->rep_fichiers_traces, pcBal);

	/* creation du repertoire d'erreurs */
	if (_access(debug->rep_fichier_erreurs, 0) != 0)
	{
		if (!CreatePath(debug->rep_fichier_erreurs))
			return INST_INIT_ERR_FICHIER_DEBUG;
	}

	/* creation du repertoire de traces */
	if (_access(debug->rep_fichiers_traces, 0) != 0)
	{
		if (!CreatePath(debug->rep_fichiers_traces))
			return INST_INIT_ERR_FICHIER_DEBUG;
	}

	/* initialise le fichier de debug */
	TRC_Initialise_Trace("DEBUG", debug->nom_fichier_erreurs, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION | TRC_OPT_TEXTE_SEUL, (TRC_EMETTEUR *)&debug->priv.emet);
	TRC_Taille_Max_Fichier(debug->priv.emet, debug->taille_limite);

	/* teste le fichier de debug */
	TRC_Trace_Texte(debug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, " => Debut du debug");

	TRC_Initialise_Trace("TRACE", tab_traces->nom, TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, (TRC_EMETTEUR *)&tab_traces->emet);
	TRC_Taille_Max_Fichier(tab_traces->emet, debug->taille_limite);

	return INST_INIT_OK;
}


PROTECTED enum_instance_result RfrGenericDeinitTrace(struct_instance_data *p_instance_data)
{
	TRC_Termine_Trace(p_instance_data->dbg.priv.emet);

	TRC_Termine_Trace(p_instance_data->sTraces.emet);

	return INST_INIT_OK;
}


PROTECTED void RfrGenericFichierTrace(struct_instance_data *p_instance_data, char *fmt, ...)
{
	va_list args;

	va_start(args, fmt);

	TRC_Trace_V(p_instance_data->sTraces.emet, TRC_OPT_MASK, NULL, 0, fmt, args);

	va_end(args);
}


PROTECTED void DEFINE_RfrGenericFichierDebug(struct_instance_data *p_instance_data, char *fmt, ...)
{
	va_list				args;
	dbg_struct_debug	*pDebug;

	pDebug = &p_instance_data->dbg;

	va_start(args, fmt);

	TRC_Trace_V(p_instance_data->sTraces.emet, TRC_OPT_MASK, NULL, 0, fmt, args);
	TRC_Direct_Trace_V(pDebug->priv.emet, TRC_OPT_CREER_FICHIER | TRC_OPT_FICHIER | TRC_OPT_NUMEROTATION, NULL, 0, fmt, args);

	va_end(args);
}
