/*------ (v) 1998 CS-Route -------- Droits reserves ------- */
/*
 * $D_HEAD
 * -----------------------------------------------------------------
 * MODULE     : TRACE (TRC)
 * FICHIER    : TRACE.C
 * LANGAGE    : C (VC++ 5.0)
 * -----------------------------------------------------------------
 * KEY WORDS  : Trace, log
 * -----------------------------------------------------------------
 * RESUME     : Module de trace assynchrone permettant la trace distante
 * --------------------------------------------------------------------
 * DESCRIPTION: Trace assynchrone placant en file d'attente les messages
 *              de trace, traités en différé par une tache de priorité
 *              basse.
 *              ATTENTION, l'assynchronisme étant géré par un thread
 *              lancé lors de la création de la premiere trace (terminé
 *              lors de la destruction de la derniere trace). La création
 *              d'un trace, son utilisation ou sa destruction dans
 *              un DllMain pose problème.
 * --------------------------------------------------------------------
 * HISTORIQUE :
 *
 * $Log : $
 *
 * -------------------------------------------------------------------- 
 * $F_HEAD
 */

/*

PRINCIPES DE FONCTIONNEMENT :

  COTE EMISSION DE TRACES

    Ce module fonctionne de manière totalement asynchrone. Un thread d'I/O est
    créé à l'ouverture de la premiere instance de trace, il est arrété lors de
    la fermeture de la derniere instance restante.
    Lors de la création d'une instance, une structure d'instance est allouée
    et l'utilisateur récupère un pointeur sur cette structure (ce pointeur lui
    servant de handle).

    Lorsqu'une trace doit être émise, un buffer est alloué et les données
    relatives à celle-ci y sont stockées (instance émettrice, date et heure
    d'émission, numéro de message etc...). Un pointeur sur ce buffer est placé
    dans une file d'attente (pipe anonyme créé au niveau du DllMain).

    Le thread de gestion des I/O est en attente sur l'autre coté de la file
    d'attente. A la reception d'un message de trace, il est analysé et les
    différentes I/O qui doivent être effectuées sont lancées (en mode
    overlapped si possible). Une fois terminées, le buffer du message est
    désalloué.

    La fermeture d'une instance est également assynchrone, excepté pour la
    dernière :
    Un message spécial de fermeture d'instance est envoyé au thread
    d'I/O. Celui-ci ferme les objets ouverts dans le contexte de cette
    instance et finalement désalloue la structure associée à l'instance.
    De plus, lorsque l'instance à fermer est la dernière, un message d'arret
    est envoyé après la demande de fermeture d'instance, puis l'appelant
    est suspendu jusqu'à arrêt du thread d'I/O. A la reception du message
    d'arret, le thread d'I/O se termine, débloquant ainsi l'appelant.

    D'après ce mode de fonctionnement, terminer le process sans fermer la
    totalité des instances peut entrainer la perte de certaines traces.
    Par contre, fermer toutes les traces ouvertes garantie le traitement
    complet de toutes les traces.

    Pour permettre un peut plus de souplesse. Il est possible d'effectuer
    un pseudo-flush des traces. En fait, l'appelant est bloqué suite à
    l'émission d'un message de "flush" contenant un handle d'évènement
    à signaler.
    Lorsque la tache d'I/O le reçoit, elle signale l'évènement, ce qui
    débloque l'appelant. Ainsi, une fois débloqué, l'appelant est certain
    que toutes les traces d'avant le pseudo-flush ont été traitées.

  COTE ECOUTE DE TRACES

    Une des possibilités de l'emetteur de traces est de pouvoir les
    envoyer par mailslot (local ou distant). Dans ce cas, il faut
    quelqu'un à l'écoute. Il existe donc un ensemble de fonctions
    d'écoute, de reception et d'analyse des messages de trace.

 */

// Pour activation des traces Heap
// #define TRC_DEBUG
// #define TRC_DEBUG_LOC


/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <tchar.h>
#include <time.h>
#include <csr_excpt.h>
#include "trcdbg.h"
#include "trc.h"
#include "htrc.h"
#include "loc_trc.h"

BOOL TRC_Test_Si_Actif( char * pcFileName );
void TRC_ParametrageDefaut();

/*--------------- VARIABLES GLOBALES SPECIFIQUES A LA LIBRAIRIE: ---------------*/
HANDLE ghPipeW = NULL;
HANDLE ghPipeR = NULL;
HANDLE ghThread = NULL;
// HINSTANCE ghDllInst = NULL;

DWORD gdwInit = 0;
BOOL gbDllMainInit = FALSE;
DWORD gdwTailleFichierDefaut = TRC_TAILLE_FICHIER_DEFAUT;
DWORD gdwTailleFifoTrace = TRC_TAILLE_QUEUE;

CRITICAL_SECTION gsCriticalInstances;

// Global structure containing backup directory and filename info
struct FullBackupPath {

	char pcFilenameExtensions[FILE_EXTENSIONS][20];
	char pcFileTag[FILE_EXTENSIONS][20];
	char pcPlaza[20];
	char pcLaneID[20];
	int iNumOfCharacters[FILE_EXTENSIONS];
	BOOL bBackupExists;
} gFullBackupPath;
/*--------------- CODE: ---------------*/

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
 * PARAMETRES: cf Win32 API
 * RETOUR    : cf Win32 API
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Initialiser la DLL à son chargement, et terminer proprement à son
 *             déchargement.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwWhy, LPVOID pvJunk )
{
	BOOL bOK = TRUE;

	switch ( dwWhy )
	{

	// Lorsque le process fait le premier LoadLibrary consernant cette DLL
	case DLL_PROCESS_ATTACH :
		{
            // Aller chercher dans le registre les valeurs par défaut des paramètres
            TRC_ParametrageDefaut();

			// Créer le pipe de comme interne permettant l'échange des pointeurs de messages
			if ( ! CreatePipe( &ghPipeR, &ghPipeW, NULL, gdwTailleFifoTrace * sizeof(void*) ) )
				ghPipeR = ghPipeW = INVALID_HANDLE_VALUE;

			// Si une erreur s'est produite, on ferme tout
			if ( ghPipeR == NULL || ghPipeW == NULL )
			{
				if ( ghPipeR    != NULL ) CloseHandle( ghPipeR );
				if ( ghPipeW    != NULL ) CloseHandle( ghPipeW );
				bOK = FALSE;
			}
			else
            {
                HTRC_Initialise();
				InitializeCriticalSection( &gsCriticalInstances );
				gbDllMainInit = TRUE;
			}

			break;
		}

	// Lorsque le process fait le dernier FreeLibrary consernant cett DLL
	case DLL_PROCESS_DETACH :
		{
			if ( gbDllMainInit )
			{
				CloseHandle( ghPipeR );
				CloseHandle( ghPipeW );
				DeleteCriticalSection( &gsCriticalInstances );
                HTRC_Termine();
			}
			break;
		}

	}

	return bOK;
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void AppendOneCharacter(char* s, char c)
* PARAMETRES: 
* RETOUR    : 
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Appends one charachter to string
* --------------------------------------------------------------------
* $F_FCTN
*/
void AppendOneCharacter(char* s, char c) {
	size_t len = strlen(s);
	s[len] = c;
	s[len + 1] = '\0';
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   :BOOL DirectoryExists(LPCTSTR pcAbsolutePath)
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Checks if directory with absolutepath name exists
* --------------------------------------------------------------------
* $F_FCTN
*/
BOOL DirectoryExists(LPCTSTR pcAbsolutePath)
{
	if (_taccess_s(pcAbsolutePath, 0) == 0)
	{
		struct _stat status;
		_tstat(pcAbsolutePath, &status);
		return (status.st_mode & S_IFDIR) != 0;
	}

	return FALSE;
}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   :BOOL CreatePath(char *pcPath)
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Creates full path directory
* --------------------------------------------------------------------
* $F_FCTN
*/
BOOL CreatePath(char *pcPath)
{
	char *pcToken;
	char pcFullPath[_MAX_PATH];


	if (_fullpath(pcFullPath, pcPath, _MAX_PATH) == NULL)
		return FALSE;

	pcToken = strchr(pcFullPath, '\\');
	pcToken++;

	while ((pcToken = strchr(pcToken, '\\')) != NULL)
	{
		*pcToken = '\0';

		if (_access(pcFullPath, 0) != 0)
		if (!CreateDirectoryA(pcFullPath, NULL))
		{
			*pcToken = '\\';
			return FALSE;
		}

		*pcToken = '\\';
		pcToken++;
	}

	if (_access(pcFullPath, 0) != 0)
	if (!CreateDirectoryA(pcFullPath, NULL))
		return FALSE;

	return TRUE;
}


/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : DWORD WINAPI IsBackupMovingActive(char *pcFichier, TRC_EMETTEUR pEmetteur)
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Checks if moving old files to backup folder is active
* --------------------------------------------------------------------
* $F_FCTN
*/
DWORD WINAPI IsBackupMovingActive(char *pcFichier, TRC_EMETTEUR pEmetteur) {

	// Get file name
	char *pcFileName;
	pcFileName = strrchr(pcFichier, '\\');
	if (pcFileName == NULL)
		pcFileName = strrchr(pcFichier, ':');
	if (pcFileName == NULL)
		pcFileName = pcFichier;
	else
		pcFileName++;

	// Extract registry value for saving old files
	DWORD dwValue;
	HKEY hKey;
	LONG lReturnStatus;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = 4;
	lReturnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_ALL_ACCESS, &hKey);
	if (lReturnStatus == ERROR_SUCCESS)
	{
		lReturnStatus = RegQueryValueEx(hKey, TEXT(pcFileName), NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
		if (lReturnStatus == ERROR_SUCCESS)
		{
			return dwValue;
		}

		else {
			return ERROR_INVALID_PARAMETER;
		}
	}
	else {
		return ERROR_INVALID_PARAMETER;
	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI DoesFileFormatExist()
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Checks if file format actually exists in registry
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI DoesFileFormatExist(){

	char pcTemporary[100] = { 0 };
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 100;
	returnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_ALL_ACCESS, &hKey);
	if (returnStatus == ERROR_SUCCESS) {

		returnStatus = RegQueryValueEx(hKey, TEXT("_FileNameFormat"), NULL, &dwType, (LPBYTE)&pcTemporary, &dwSize);

		if (returnStatus == ERROR_SUCCESS) {

			gFullBackupPath.bBackupExists = TRUE;

		}

		else {

			gFullBackupPath.bBackupExists = FALSE;

		}

	}

	else {

		gFullBackupPath.bBackupExists = FALSE;

	}

	if (strlen(pcTemporary) == 0){

		gFullBackupPath.bBackupExists = FALSE;

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI DoesBackupFolderExists()
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Checks if backupfolder actually exists
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI DoesBackupFolderExists(){

	char pcTemporary[100] = { 0 };
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 100;
	returnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_ALL_ACCESS, &hKey);
	if (returnStatus == ERROR_SUCCESS) {

		returnStatus = RegQueryValueEx(hKey, TEXT("_BackupFolder"), NULL, &dwType, (LPBYTE)&pcTemporary, &dwSize);

		if (returnStatus == ERROR_SUCCESS) {
			
			gFullBackupPath.bBackupExists = TRUE;

		}

		else {

			gFullBackupPath.bBackupExists = FALSE;
			
		}

	}

	else {

		gFullBackupPath.bBackupExists = FALSE;
		
	}

	if (strlen(pcTemporary) == 0){

		gFullBackupPath.bBackupExists = FALSE;

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI GetBackupDirectory(TRC_EMETTEUR pEmetteur)
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Gets the name of backup directory
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI GetBackupDirectory(TRC_EMETTEUR pEmetteur) {

	char pcTemporary[100] = { 0 };
	HKEY hKey;
	LONG returnStatus;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 100;
	returnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_ALL_ACCESS, &hKey);
	if (returnStatus == ERROR_SUCCESS) {

		returnStatus = RegQueryValueEx(hKey, TEXT("_BackupFolder"), NULL, &dwType, (LPBYTE)&pcTemporary, &dwSize);

		if (returnStatus == ERROR_SUCCESS) {
			strcpy_s(pEmetteur->pcFichierOld, MAX_PATH + 5, pcTemporary);
		}

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI GetPlazaNumber()
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Gets the plaza ID from registry
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI GetPlazaNumber() {

	DWORD dwValue;
	HKEY hKey;
	LONG lReturnStatus;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = 4;
	lReturnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\LaneController\\Project\\Parameters\\GeneralData", 0, KEY_ALL_ACCESS, &hKey);
	if (lReturnStatus == ERROR_SUCCESS)
	{
		lReturnStatus = RegQueryValueEx(hKey, TEXT("PlazaNumber"), NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
		if (lReturnStatus == ERROR_SUCCESS)
		{
			sprintf_s(gFullBackupPath.pcPlaza, sizeof(gFullBackupPath.pcPlaza), "%d", dwValue);
		}

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI GetLaneNumber()
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Gets the lane ID from registry
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI GetLaneNumber() {

	DWORD dwValue;
	HKEY hKey;
	LONG lReturnStatus;
	DWORD dwType = REG_DWORD;
	DWORD dwSize = 4;
	lReturnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\LaneController\\Project\\Parameters\\GeneralData", 0, KEY_ALL_ACCESS, &hKey);
	if (lReturnStatus == ERROR_SUCCESS)
	{
		lReturnStatus = RegQueryValueEx(hKey, TEXT("LaneNumber"), NULL, &dwType, (LPBYTE)&dwValue, &dwSize);
		if (lReturnStatus == ERROR_SUCCESS)
		{
			sprintf_s(gFullBackupPath.pcLaneID, sizeof(gFullBackupPath.pcLaneID), "%d", dwValue);
		}

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void WINAPI GetFileFormat()
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Gets file format
* --------------------------------------------------------------------
* $F_FCTN
*/
void WINAPI GetFileFormat() {

	char cTemporary[100] = { 0 };
	HKEY hKey;
	LONG lReturnStatus;
	DWORD dwType = REG_SZ;
	DWORD dwSize = 100;
	lReturnStatus = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_ALL_ACCESS, &hKey);
	if (lReturnStatus == ERROR_SUCCESS) {

		lReturnStatus = RegQueryValueEx(hKey, TEXT("_FileNameFormat"), NULL, &dwType, (LPBYTE)&cTemporary, &dwSize);

		if (lReturnStatus == ERROR_SUCCESS) {
			char * pcCh, *next_token;
			pcCh = strtok_s(cTemporary, ";", &next_token);
			int n = 0;
			while (pcCh != NULL){
				strcpy_s(gFullBackupPath.pcFilenameExtensions[n], sizeof(gFullBackupPath.pcFilenameExtensions[n]), pcCh);
				pcCh = strtok_s(NULL, ";", &next_token);
				n++;
			}

			char *pch2, *next_token2;

			for (int i = 0; i < n; i++) {

				pch2 = strtok_s(gFullBackupPath.pcFilenameExtensions[i], ":", &next_token2);
				strcpy_s(gFullBackupPath.pcFileTag[i], sizeof(gFullBackupPath.pcFileTag[i]), pch2);

				pch2 = strtok_s(NULL, ":", &next_token2);
				gFullBackupPath.iNumOfCharacters[i] = atoi(pch2);

			}

		}

	}

}

/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE   : void MoveToBackupFolder(TRC_EMETTEUR pEmetteur)
* PARAMETRES:
* RETOUR    :
* --------------------------------------------------------------------
* VARIABLES :
* --------------------------------------------------------------------
* ROLE      : Move old file to backup folder
* --------------------------------------------------------------------
* $F_FCTN
*/
void MoveToBackupFolder(TRC_EMETTEUR pEmetteur) {

	char pcTemporary[MAX_PATH + 5] = { 0 };
	int iLength = 0;

	// Get local time
	SYSTEMTIME stTime = { 0 };
	GetLocalTime(&stTime);

	// strings to store date and time
	char pcHours[3] = { 0 };
	char pcMinutes[3] = { 0 };
	char pcSeconds[3] = { 0 };
	char pcMiliseconds[5] = { 0 };
	char pcDay[3] = { 0 };
	char pcMonth[3] = { 0 };
	char pcYear[5] = { 0 };

	// Fill string with date
	sprintf_s(pcHours, sizeof(pcHours), "%d", (int)stTime.wHour);
	sprintf_s(pcMinutes, sizeof(pcMinutes), "%d", (int)stTime.wMinute);
	sprintf_s(pcSeconds, sizeof(pcSeconds), "%d", (int)stTime.wSecond);
	sprintf_s(pcMiliseconds, sizeof(pcMiliseconds), "%d", (int)stTime.wMilliseconds);
	sprintf_s(pcDay, sizeof(pcDay), "%d", (int)stTime.wDay);
	sprintf_s(pcMonth, sizeof(pcMonth), "%d", (int)stTime.wMonth);
	sprintf_s(pcYear, sizeof(pcYear), "%d", (int)stTime.wYear);

	strcpy_s(pcTemporary, MAX_PATH + 5, pEmetteur->pcFichierOld);

	// Create directory if it doesnt exist
	if (!DirectoryExists(pcTemporary)) {

		CreatePath(pcTemporary);

	}

	AppendOneCharacter(pcTemporary, '\\');

	if (1) {

		// Get original file name
		char * pcFileName;
		pcFileName = strrchr(pEmetteur->pcFichier, '\\');
		if (pcFileName == NULL)
			pcFileName = strrchr(pEmetteur->pcFichier, ':');
		if (pcFileName == NULL)
			pcFileName = pEmetteur->pcFichier;
		else
			pcFileName++;

		// Add file extension to file name
		for (int i = 0; i < FILE_EXTENSIONS; i++) {

			if (strcmp(gFullBackupPath.pcFileTag[i], "PLAZA") == 0) {

				iLength = (int)strlen(gFullBackupPath.pcPlaza);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, gFullBackupPath.pcPlaza);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "LANE") == 0) {

				iLength = (int)strlen(gFullBackupPath.pcLaneID);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, gFullBackupPath.pcLaneID);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "YEAR") == 0) {

				iLength = (int)strlen(pcYear);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcYear);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "MONTH") == 0) {

				iLength = (int)strlen(pcMonth);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcMonth);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "DAY") == 0) {

				iLength = (int)strlen(pcDay);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcDay);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "HOUR") == 0) {

				iLength = (int)strlen(pcHours);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcHours);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "MINUTE") == 0) {

				iLength = (int)strlen(pcMinutes);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcMinutes);

			}

			if (strcmp(gFullBackupPath.pcFileTag[i], "SECOND") == 0) {

				iLength = (int)strlen(pcSeconds);
				for (int j = 0; j < gFullBackupPath.iNumOfCharacters[i] - iLength; j++) {

					AppendOneCharacter(pcTemporary, '0');

				}

				strcat_s(pcTemporary, MAX_PATH + 5, pcSeconds);

			}

		}

		AppendOneCharacter(pcTemporary, '_');
		strcat_s(pcTemporary, MAX_PATH + 5, pcFileName);

	}

	MoveFileEx(pEmetteur->pcFichier, pcTemporary, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Vider_Traces( DWORD dwDelai)
 * PARAMETRES: dwDelai : Delai d'attente maxi pour le vidage de la trace
 *                       INFINITE pour attendre toujours
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Attendre que toutes les trace en traitement assynchrone
 *             soient émise
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Vider_Traces( DWORD dwDelai)
{
	DWORD dwErreur = NO_ERROR;
	DWORD dwFoo;
	DWORD dwRes;
	HANDLE hEvent;
	TRC_MESSAGE psMsg;

	EnterCriticalSection( &gsCriticalInstances );

	if ( gdwInit > 0 )
	{
		psMsg = HeapAlloc( GetProcessHeap(), 0, sizeof(TRC_MESSAGE_STRUCT) );
		hEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

		if ( hEvent != NULL && psMsg != NULL )
		{
			psMsg->dwTaille = TRC_SIGNALER_EVENEMENT;
			((HANDLE)(psMsg->psEmetteur)) = hEvent;
			if ( WriteFile( ghPipeW, &psMsg, sizeof(psMsg),&dwFoo,NULL) )
			{
				LeaveCriticalSection( &gsCriticalInstances );
				dwRes = WaitForSingleObject( hEvent, dwDelai );
				if ( dwRes == WAIT_OBJECT_0 )
					dwErreur = NO_ERROR;
				else if ( dwRes = WAIT_TIMEOUT )
					dwErreur = ERROR_TIMEOUT;
				else
					dwErreur = GetLastError();
				psMsg = NULL; // Désallocation faite par le thread d'émission
			}
			else
			{
				dwErreur = GetLastError();
				LeaveCriticalSection( &gsCriticalInstances );
				HeapFree( GetProcessHeap(), 0, psMsg );
			}
		}
		else
		{
			dwErreur = GetLastError();
			LeaveCriticalSection( &gsCriticalInstances );
		}

		if ( hEvent != NULL ) CloseHandle( hEvent );
	}
	else
	{
		LeaveCriticalSection( &gsCriticalInstances );
		dwErreur = NO_ERROR;
	}

    SetLastError( dwErreur );

	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Initialise_Trace( char * pcCle, char * pcFichier, DWORD dwOptions, TRC_EMETTEUR * ppsEmetteur )
 * PARAMETRES: pcCle    : Clé servant a identifier l'instance de trace
 *             pcFichier: Nom du fichier de trace.
 *             dwOptions: Options de trace.
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             ppsEmetteur: Pointe sur un "handle" qui recevra les données de l'instance
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Créer une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Initialise_Trace( char * pcCle, char * pcFichier, DWORD dwOptions, TRC_EMETTEUR * ppsEmetteur )
{
	
#ifdef _WIN64
	DWORD64 dwLen;
#else
	DWORD dwLen;
#endif

	DWORD dwErreur = NO_ERROR;
	DWORD dwId;
	TRC_EMETTEUR psEmetteur = NULL;
	// HINSTANCE hDllInst;

	// On passe par une section critique pour éviter la double création du thread de réemission
	// dans le cas ou le premier appel à cette fonction est effectué simultanément par deux threads.
	// (c'est au cours du premier appel que le thread est crée).
	EnterCriticalSection( &gsCriticalInstances );
	// Vérification de la cohérence des paramètres
	dwLen = strlen( pcFichier == NULL ? "" : pcFichier );
	if	( ! TRC_Option_Valide( dwOptions, dwLen > 0 ) )
	{
		SetLastError( ERROR_INVALID_PARAMETER );
		return ERROR_INVALID_PARAMETER;
	}

	psEmetteur = HTRC_Creer_Handle( sizeof(TRC_EMETTEUR_STRUCT), TRC_TYPE_EMISSION );
	if ( psEmetteur != NULL )
	{
        psEmetteur->bActive = TRC_Test_Si_Actif( pcFichier );
        psEmetteur->bFileMissing = FALSE;

		// Récupération des options
		psEmetteur->dwOptions = dwOptions;

		// Initialisation du compteur
		psEmetteur->dwNum = 0;

		// Taille max fichier
		psEmetteur->llTailleFichier = gdwTailleFichierDefaut;

		// Inutile de protéger les handles par gsCriticalInstances ca l'appelant ne
		// dispose pas encore du pointeur qui pourrait lui permettre de mener des
		// actions sur cette instance.

		// Si option MAILSLOT
		psEmetteur->hMailSlot = INVALID_HANDLE_VALUE;
		if ( dwOptions & (TRC_OPT_MAILSLOT_LOCAL|TRC_OPT_MAILSLOT_DISTANT) )
		{
			strcpy_s( psEmetteur->pcMailSlot,
					MAX_PATH+1,
					( dwOptions & TRC_OPT_MAILSLOT_LOCAL ) ?
						TRC_PREFIXE_MAILSLOT_EMISSION_LOCALE :
						TRC_PREFIXE_MAILSLOT_EMISSION_DISTANTE );
			strncat_s( psEmetteur->pcMailSlot, MAX_PATH+1, pcCle, sizeof(psEmetteur->pcMailSlot) );
			psEmetteur->pcMailSlot[sizeof(psEmetteur->pcMailSlot)-1] = '\0';
		}
		else
			psEmetteur->pcMailSlot[0] = '\0';

		// Si option FICHIER
		psEmetteur->hFichier = INVALID_HANDLE_VALUE;
		if ( dwOptions & TRC_OPT_FICHIER )
		{
			strncpy_s( psEmetteur->pcFichier, MAX_PATH+1, pcFichier, sizeof(psEmetteur->pcFichier) );
			psEmetteur->pcFichier[sizeof(psEmetteur->pcFichier)-1] = '\0';

			// Checking if backup folder is defined in registry
			DoesBackupFolderExists();

			// If backup folder exists check file format
			if (gFullBackupPath.bBackupExists)
				DoesFileFormatExist();

			// Checking if moving old files to backup is active
			if (IsBackupMovingActive(pcFichier, psEmetteur) == BACKUP_MOVING_ACTIVE && gFullBackupPath.bBackupExists){

				// Get data necessary for backup transfer
				GetBackupDirectory(psEmetteur);
				GetFileFormat();
				GetPlazaNumber();
				GetLaneNumber();

			}

			else{

				strcpy_s(psEmetteur->pcFichierOld, MAX_PATH + 5, psEmetteur->pcFichier);
				strcat_s(psEmetteur->pcFichierOld, MAX_PATH + 5, TRC_SUFFIXE_FICHIER);

			}

            if ( dwOptions & TRC_OPT_TOUJOURS_OUVERT )
            {
                psEmetteur->hFichier = CreateFile(
				    psEmetteur->pcFichier,
			        GENERIC_READ|GENERIC_WRITE,
			        FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			        NULL,
				    ( psEmetteur->dwOptions & TRC_OPT_CREER_FICHIER ) ? OPEN_ALWAYS : OPEN_EXISTING,
			        FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			        NULL );
                psEmetteur->bFileMissing = ( psEmetteur->hFichier == INVALID_HANDLE_VALUE );
            }
            else
                psEmetteur->hFichier = INVALID_HANDLE_VALUE;
		}
		else 
			psEmetteur->pcFichier[0] = '\0';

		// Si option EVENTLOG
		psEmetteur->hEventLog = NULL;
		if ( dwOptions & TRC_OPT_EVENTLOG )
		{
			strcpy_s( psEmetteur->pcEventLog, MAX_PATH+1, TRC_PREFIXE_EVENTLOG );
			strncat_s( psEmetteur->pcEventLog, MAX_PATH+1, pcCle, sizeof(psEmetteur->pcEventLog) );
		}
		else
			psEmetteur->pcEventLog[0] = '\0';

		// Si option CONSOLE
		psEmetteur->hConsole = INVALID_HANDLE_VALUE;

		// Attention, si des resources ont été ouvertes et que dwErreur != NO_ERROR,
		// ces resources doivent être libérées avant la fin du bloc.
		if ( dwErreur != NO_ERROR )
		{
			if ( psEmetteur->hFichier != INVALID_HANDLE_VALUE )
				CloseHandle( psEmetteur->hFichier );
			HTRC_Detruire_Handle( psEmetteur );
			psEmetteur = NULL;
		}
	}
	else
		dwErreur = ERROR_NOT_ENOUGH_MEMORY;

	if ( dwErreur == NO_ERROR && gdwInit == 0 )
	{
		// Créer le thread de réémission des messages de trace

		// Attention, pour que le DllMain(DLL_PROCESS_DETACH) ne soit appelé
		// que lorsque le thread s'est terminé, on incrément l'usage de celle-ci.
        // hDllInst = TRC_Incrementer_Usage_DLL( ghDllInst );
		ghThread = ExcptCreateThread(
			NULL, 
			0, 
			(LPTHREAD_START_ROUTINE )TRC_Thread_Ecriture, 
			(LPVOID)/* hDllInst */NULL,
			0,
			&dwId,
            "TRACE_WRITER_THREAD" );
		if ( ghThread == NULL )
		{
			dwErreur = GetLastError();

			// Le thread n'est pas créé, on ramène le compteur de chargement
			// de la DLL a son état initial
			// FreeLibrary( hDllInst );
			if ( psEmetteur->hFichier != INVALID_HANDLE_VALUE )
				CloseHandle( psEmetteur->hFichier );
			HTRC_Detruire_Handle( psEmetteur );
			psEmetteur = NULL;
		}
	}

    if ( dwErreur == NO_ERROR )
    {
        TRC_Trace_Texte(
			psEmetteur,
			TRC_OPT_MASK&(~TRC_OPT_NUMEROTATION),
			"Démarrage de l'instance de trace %s", pcCle );
    }

	if ( dwErreur == NO_ERROR )
		gdwInit ++;

	LeaveCriticalSection( &gsCriticalInstances );

	(*ppsEmetteur) = psEmetteur;

    SetLastError( dwErreur );

	return dwErreur;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL TRC_Fichier_Actif ( TRC_EMETTEUR psEmetteur )
 * PARAMETERS: psEmetteur   : valeur obtenue avec TRC_Initialise. Doit avoir l'option
 *                     TRC_OPT_FICHIER active.
 * RETURN    : TRUE : le fichier existe, est ouvert et en cours d'utilisation pour
 *               l'instance de trace testée.
 *             FALSE: soit le fichier n'existe pas, soit il n'a pas pu être ouvert, soit
 *               l'option TRC_OPT_FICHIER n'est pas active.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si le fichier précisé à l'initilisation de l'instance de trace
 *             reçoit effectivement les trace émises.
 * --------------------------------------------------------------------
 */
BOOL WINAPI TRC_Fichier_Actif( TRC_EMETTEUR psEmetteur )
{
	BOOL bRes = FALSE;

    EnterCriticalSection( &gsCriticalInstances );

    if ( HTRC_Etat_Handle( psEmetteur ) == HTRC_VALID_HANDLE )
	    bRes = ( ( ( psEmetteur->dwOptions & TRC_OPT_FICHIER ) != 0 ) && ( ! psEmetteur->bFileMissing ) );

	LeaveCriticalSection( &gsCriticalInstances );

    return bRes;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Taille_Max_Fichier( TRC_EMETTEUR psEmetteur, LONGLONG llTailleFichier )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise. Doit avoir l'option
 *                     TRC_OPT_FICHIER active.
 *             llTailleFichier : Taille maximum du fichier de trace.
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Definir la taille maximum d'un fichier de trace.
 *             La valeur par défaut étant TRC_TAILLE_FICHIER_DEFAUT
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Taille_Max_Fichier( TRC_EMETTEUR psEmetteur, LONGLONG llTailleFichier )
{
	DWORD dwErreur = NO_ERROR;

    EnterCriticalSection( &gsCriticalInstances );

    if ( HTRC_Etat_Handle( psEmetteur ) == HTRC_VALID_HANDLE )
 	    if ( ( llTailleFichier > ( TRC_MAX_TAILLE_LIGNE_TEXTE + 2 ) ) && ( ( psEmetteur->dwOptions & TRC_OPT_FICHIER ) != 0 ) )
		    psEmetteur->llTailleFichier = llTailleFichier - TRC_MAX_TAILLE_LIGNE_TEXTE - 2 ;
	    else
		    dwErreur = ERROR_INVALID_PARAMETER;
    else
        dwErreur = ERROR_INVALID_HANDLE;

    LeaveCriticalSection( &gsCriticalInstances );

    SetLastError( dwErreur );
	return dwErreur;
}





/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat : comme vprintf
 *             lParam   : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
{
	DWORD dwErreur = NO_ERROR;
	DWORD dwTaille;
	TRC_MESSAGE_STRUCT sMsg;
	TRC_MESSAGE psMsg;
	ULONGLONG ullCount;
    ULONGLONG ullFreq;
	char pcTexte[ TRC_MAX_TEXTE + 1 ];

    EnterCriticalSection( &gsCriticalInstances );

	if ( HTRC_Etat_Handle( psEmetteur ) == HTRC_VALID_HANDLE )
	{
        if ( ! psEmetteur->bActive )
        {
            LeaveCriticalSection( &gsCriticalInstances );
            return NO_ERROR;
        }
		// Récuperation dans une structure contenant l'entete de message des
		// paramètre de ce message.
		sMsg.dwOptions = dwOptions & psEmetteur->dwOptions;   // Option de message

		sMsg.psEmetteur = psEmetteur;
        sMsg.bDirect = FALSE;

        if ( QueryPerformanceCounter( (LARGE_INTEGER*)&ullCount ) )
        {
            if ( QueryPerformanceFrequency( (LARGE_INTEGER*)&ullFreq ) )
            {
                if ( ullFreq != 0 )
                {
                    ullCount = ( ullCount * (ULONGLONG)1000000 ) / ullFreq;
                    sMsg.dwTick = (DWORD)ullCount;
                }
                else
                    sMsg.dwTick = GetTickCount() * 1000;
            }
            else
                sMsg.dwTick = GetTickCount() * 1000;
        }
        else 
		    sMsg.dwTick = GetTickCount() * 1000;

        /* Suppression des fonctions d'ajustement très consommatrices en CPU
		GetSystemTime( &(sMsg.sDate) );                   // Heure courante GMT
        */
        GetLocalTime( &(sMsg.sDate) );                    // Heure courante locale
        
		sMsg.dwDonnees = dwDonnees;                       // Taille du bloc de donnees
		// Si flag numérotation, alors incrémenter l'index et le mettre
		// à jour dans l'entète, sinon, le laisser à 0.
		if ( sMsg.dwOptions & TRC_OPT_NUMEROTATION )
		{
			psEmetteur->dwNum ++;
			sMsg.dwNum = psEmetteur->dwNum;
		}
		else
			sMsg.dwNum = 0;
		
		// Remplissage du bloc de texte et récupération de la taille de celui-ci
		sMsg.dwTexte = (DWORD)(_vsnprintf_s(
			pcTexte,
			TRC_MAX_TEXTE + 1,
			sizeof(pcTexte)-1,
			pcFormat,
			lParm ) );
		if ( sMsg.dwTexte == ((DWORD)(-1)) )	// Si texte tronqué
			sMsg.dwTexte = sizeof(pcTexte)-1;
		pcTexte[sMsg.dwTexte] = '\0';

		// Calcul de la taille totale du message
		// Du fait que sMsg.pbDonnees soit déjà dimensionné à 1, on a donc en fait au moins un octet
		// (potentiellement plus en fonction de l'option PACK) en fin de message qui n'est pas
		// utilisé (celui servira à forcer la fin de chaine de caractère).
		sMsg.dwTaille = sizeof(sMsg) + sMsg.dwDonnees + sMsg.dwTexte;

		// Allocation du message
		if ( ( psMsg = HeapAlloc( GetProcessHeap(), 0, sMsg.dwTaille ) ) == NULL )
			dwErreur = ERROR_NOT_ENOUGH_MEMORY;
		else
		{
			// Récupération de l'entète
			(*psMsg) = sMsg;

			// Copie zone de données
			CopyMemory( &(psMsg->pbDonnees[0]), pbDonnees, sMsg.dwDonnees );

			// Copie zone de texte
			CopyMemory( &(psMsg->pbDonnees[sMsg.dwDonnees]), pcTexte, sMsg.dwTexte + 1 );

			// Ecriture dans le pipe, non pas du message, mais du pointeur sur le message
			if ( ! WriteFile( ghPipeW, &(psMsg), sizeof(psMsg), &dwTaille, NULL ) )
			{
				dwErreur = GetLastError();

				// En cas d'erreur, on détruit le message
				HeapFree( GetProcessHeap(), 0, psMsg );
			}
		}
	}
	else
		dwErreur = ERROR_INVALID_DATA;

    LeaveCriticalSection( &gsCriticalInstances );

    SetLastError( dwErreur );

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat : comme vprintf
 *             lParam   : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace directement : au retour de
 *             la fonction, le système garantie que les données de la
 *             trace ont effectivement été traitées.
 *             Attention, la seule restriction est de ne pas appeler
 *             TRC_Termine_Trace() pour cette instance dans un autre thread
 *             en même temps que TRC_Direct_Trace_V() (possibilité
 *             d'exception).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
// BPH 10/01/01
DWORD WINAPI TRC_Direct_Trace_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, va_list lParm )
{
    // TO DO : VERIFIER CORRECTION
	DWORD dwErreur = NO_ERROR;
    DWORD dwTailleWrite;
    DWORD dwTailleMax = sizeof( TRC_MESSAGE_STRUCT ) + dwDonnees + TRC_MAX_TEXTE + 1;
    DWORD dwTailleTexte;
    DWORD dwIoError;
    HANDLE hFichier = INVALID_HANDLE_VALUE;
    HANDLE hMailSlot = INVALID_HANDLE_VALUE;
    HANDLE hConsole = INVALID_HANDLE_VALUE;
    HANDLE hEventLog = NULL;
	ULONGLONG ullCount;
	ULONGLONG ullFreq;
    LARGE_INTEGER liTailleFichier;
	char * pcTexte;
    TRC_MESSAGE psMsg = NULL;
	OVERLAPPED sFichier = {0};
	OVERLAPPED sMailSlot = {0};
	char pcTrace[TRC_MAX_TAILLE_LIGNE_TEXTE];

    // Fait hors section critique, car les handles utilisés sont tous ouverts et
    // fermés dans cette fonction.
    do {
        // On vérifie quand même la validité du handle, ainsi que le success de l'allocation
	    if ( HTRC_Etat_Handle( psEmetteur ) != HTRC_VALID_HANDLE )
        {
		    dwErreur = ERROR_INVALID_DATA;
            break;
        }

        if ( ! psEmetteur->bActive )
        {
            dwErreur = NO_ERROR;
            break;
        }

        psMsg = HeapAlloc( GetProcessHeap(), 0, dwTailleMax ); // Préallouer un pseudo-message
        if ( psMsg == NULL )
        {
            dwErreur = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        // Attention : Pas de numérotation en trace directe
	    dwOptions = dwOptions & psEmetteur->dwOptions & (~TRC_OPT_NUMEROTATION);   // Option de message

        // CONSTRUCTION D'UN PSEUDO MESSAGE DE TRACE

        // Récupération des options de trace
	    psMsg->dwOptions = dwOptions;

        // Pas de lien sur l'emtteur
        psMsg->psEmetteur = NULL;

        // Flag mode direct
        psMsg->bDirect = TRUE;

        // Pas de numérotation
        psMsg->dwNum = 0;

        // Remplissage de bloc de données
        psMsg->dwDonnees = dwDonnees;
        if ( dwDonnees > 0 ) memcpy( psMsg->pbDonnees, pbDonnees, dwDonnees );

        if ( QueryPerformanceCounter( (LARGE_INTEGER*)&ullCount ) )
        {
            if ( QueryPerformanceFrequency( (LARGE_INTEGER*)&ullFreq ) )
            {
                if ( ullFreq != 0 )
                {
                    ullCount = ( ullCount * (ULONGLONG)1000000 ) / ullFreq;
                    psMsg->dwTick = (DWORD)ullCount;
                }
                else
                    psMsg->dwTick = GetTickCount() * 1000;
            }
            else
                psMsg->dwTick = GetTickCount() * 1000;
        }
        else 
		    psMsg->dwTick = GetTickCount() * 1000;

        /* Suppression des fonctions d'ajustement très consommatrices en CPU
        // Datation Heure courante GMT
	    GetSystemTime( &psMsg->sDate );          
        */
	    GetLocalTime( &psMsg->sDate );          
        
        // Placer le pointeur de texte au bon endroit
        pcTexte = &( psMsg->pbDonnees[dwDonnees] );

        // Remplissage du bloc de texte et récupération de la taille de celui-ci
	    psMsg->dwTexte = (DWORD)(_vsnprintf_s(
		    pcTexte,
		    TRC_MAX_TEXTE,
			TRC_MAX_TEXTE,
		    pcFormat,
		    lParm ) );
	    if ( psMsg->dwTexte == ((DWORD)(-1)) )	// Si texte tronqué
		    psMsg->dwTexte = TRC_MAX_TEXTE;
	    pcTexte[psMsg->dwTexte] = '\0';

        // Taille utile de la structure
        psMsg->dwTaille = sizeof(*psMsg) + psMsg->dwDonnees + psMsg->dwTexte;

        // Construire le contenu de la trace complete dans un buffer d'après le
        // pseudo message construit.
        dwTailleTexte = TRC_Ecrire_Message( psMsg, pcTrace, sizeof(pcTrace) );

        // Si option fichier, on tente de l'ouvrir
	    if ( ( dwOptions & TRC_OPT_FICHIER ) != 0 &&
		      psEmetteur->pcFichier[0] != '\0' )
        {
            hFichier = CreateFile(
			    psEmetteur->pcFichier,
			    GENERIC_READ|GENERIC_WRITE,
			    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			    NULL,
                ( ( dwOptions & TRC_OPT_CREER_FICHIER ) != 0 ) ? OPEN_ALWAYS : OPEN_EXISTING,
			    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
        	    NULL );
            psEmetteur->bFileMissing = ( hFichier == INVALID_HANDLE_VALUE );
            if ( hFichier != INVALID_HANDLE_VALUE )
            {
                liTailleFichier.LowPart = GetFileSize(
    			    hFichier,
	    		    &(liTailleFichier.HighPart) );
                if ( liTailleFichier.QuadPart > psEmetteur->llTailleFichier )
                {
                    CloseHandle( hFichier );

					if (IsBackupMovingActive(psEmetteur->pcFichier, psEmetteur) == BACKUP_MOVING_ACTIVE && gFullBackupPath.bBackupExists){

						// Move file to backup file
						MoveToBackupFolder(psEmetteur);

					}

					else{

						MoveFileEx(psEmetteur->pcFichier, psEmetteur->pcFichierOld, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH);

					}

        	        hFichier = CreateFile(
			            psEmetteur->pcFichier,
			            GENERIC_READ|GENERIC_WRITE,
			            FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			            NULL,
                        ( ( dwOptions & TRC_OPT_CREER_FICHIER ) != 0 ) ? OPEN_ALWAYS : OPEN_EXISTING,
			            FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
        	            NULL );
                    psEmetteur->bFileMissing = ( hFichier == INVALID_HANDLE_VALUE );
                }
            }
        }


	    // Si option mailslot, ouverture
	    if ( ( dwOptions & ( TRC_OPT_MAILSLOT_DISTANT | TRC_OPT_MAILSLOT_LOCAL ) ) != 0 &&
		       psEmetteur->pcMailSlot[0] != '\0' )
		    hMailSlot = CreateFile(
			    psEmetteur->pcMailSlot,
			    GENERIC_WRITE,
			    FILE_SHARE_READ|FILE_SHARE_WRITE,
			    NULL,
			    CREATE_ALWAYS,
			    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			    NULL );

	    // Si option console, on le handle de la sortie console
	    if ( ( dwOptions & TRC_OPT_CONSOLE ) != 0 )
		    hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

        // Si option event log
	    if ( ( dwOptions & TRC_OPT_EVENTLOG ) != 0 &&
		       psEmetteur->pcEventLog[0] != '\0' )
		    hEventLog = RegisterEventSource( NULL, psEmetteur->pcEventLog );

	    // ECRITURE DANS LE FICHIER EN OVERLAPPED I/O
	    // On commence par cette opération car potentiellement la plus longue
	    if ( hFichier != INVALID_HANDLE_VALUE )
	    {
		    sFichier.Offset = GetFileSize(
			    hFichier,
			    &(sFichier.OffsetHigh) );
		    if ( ! WriteFile( hFichier, pcTrace, dwTailleTexte, NULL, &sFichier ) )
		    {
                dwIoError = GetLastError();
                if ( dwIoError != ERROR_IO_PENDING )
                {
			        CloseHandle( hFichier );
			        hFichier = INVALID_HANDLE_VALUE;
                }
		    }
	    }

	    // ECRITURE VERS MAILSLOT EN OVERLAPPED I/O
	    if ( hMailSlot != INVALID_HANDLE_VALUE )
		    if ( ! WriteFile( hMailSlot, psMsg, psMsg->dwTaille, NULL, &sMailSlot ) )
		    {
                dwIoError = GetLastError();
                if ( dwIoError != ERROR_IO_PENDING )
                {
			        CloseHandle( hMailSlot );
			        hMailSlot = INVALID_HANDLE_VALUE;
                }
		    }

	    // ECRITURE SUR LA CONSOLE (PENDAND QUE LES I/O MAILSLOT ET FICHIER CONTINUENT)
	    if ( hConsole != INVALID_HANDLE_VALUE )
		    WriteFile( hConsole, pcTrace, dwTailleTexte, &dwTailleWrite, NULL );

	    // ENVOIE AU LOG EVENT (PENDAND QUE LES I/O MAILSLOT ET FICHIER CONTINUENT)
	    if ( hEventLog != NULL )
	    {
		    ReportEvent(
			    hEventLog,
			    EVENTLOG_INFORMATION_TYPE,
			    0,
			    0,
			    NULL,
			    1,
			    psMsg->dwDonnees,
			    &pcTexte,
			    psMsg->pbDonnees );
            DeregisterEventSource( hEventLog );
	    }

	    // ATTENDRE FIN I/O MAILSLOT
	    // Potentiellement le plus court
	    if ( hMailSlot != INVALID_HANDLE_VALUE )
	    {
		    GetOverlappedResult( hMailSlot, &sMailSlot, &dwTailleWrite, TRUE );
		    CloseHandle( hMailSlot );
            hMailSlot = INVALID_HANDLE_VALUE;
	    }

	    // ATTENDRE FIN I/O FICHIER
	    if ( hFichier != INVALID_HANDLE_VALUE )
	    {
		    GetOverlappedResult( hFichier, &sFichier, &dwTailleWrite, TRUE );
            if ( ( psMsg->dwOptions & TRC_OPT_IMMEDIAT ) != 0 )
			    FlushFileBuffers( hFichier );
    	    CloseHandle( hFichier );
		    hFichier = INVALID_HANDLE_VALUE;
	    }
    }
    while (FALSE);

    if ( psMsg != NULL )
        HeapFree( GetProcessHeap(), 0, psMsg );

    SetLastError( dwErreur );

	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
{
	va_list lParm;

	// Pour les derniers paramètres en nombre variable (...)
	va_start( lParm, pcFormat );

	return TRC_Trace_V( psEmetteur, dwOptions, pbDonnees, dwDonnees, pcFormat, lParm );
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcFormat, ... )
{
	va_list lParm;

	// Pour les derniers paramètres en nombre variable (...)
	va_start( lParm, pcFormat );

	return TRC_Direct_Trace_V( psEmetteur, dwOptions, pbDonnees, dwDonnees, pcFormat, lParm );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcTexte  : texte de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
{
	return TRC_Trace( psEmetteur, dwOptions, pbDonnees, dwDonnees, "%s", pcTexte );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 *             pcTexte  : texte de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace_Simple( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees, char * pcTexte )
{
	return TRC_Direct_Trace( psEmetteur, dwOptions, pbDonnees, dwDonnees, "%s", pcTexte );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm ))
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pcFormat, lParm : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm )
{
	return TRC_Trace_V( psEmetteur, dwOptions, NULL, 0, pcFormat, lParm );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm ))
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pcFormat, lParm : comme vprintf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace_Texte_V( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, va_list lParm )
{
	return TRC_Direct_Trace_V( psEmetteur, dwOptions, NULL, 0, pcFormat, lParm );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
{
	va_list lParm;


	// Pour les derniers paramètres en nombre variable (...)
	va_start( lParm, pcFormat );

	return TRC_Trace_V( psEmetteur, dwOptions, NULL, 0, pcFormat, lParm );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pcFormat,... : comme printf
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace_Texte( TRC_EMETTEUR psEmetteur, DWORD dwOptions, char * pcFormat, ... )
{
	va_list lParm;


	// Pour les derniers paramètres en nombre variable (...)
	va_start( lParm, pcFormat );

	return TRC_Direct_Trace_V( psEmetteur, dwOptions, NULL, 0, pcFormat, lParm );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
{
	return TRC_Trace( psEmetteur, dwOptions, pbDonnees, dwDonnees, "" );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             pbDonnees: Pointe sur une zone de donnees à dumper (peut etre NULL si dwDonnees = 0)
 *             dwDonnees: Taille de la zone pointee
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace_Donnees( TRC_EMETTEUR psEmetteur, DWORD dwOptions, BYTE * pbDonnees, DWORD dwDonnees )
{
	return TRC_Direct_Trace( psEmetteur, dwOptions, pbDonnees, dwDonnees, "" );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwErreur )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             dwErreur    : Code d'erreur win32
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwCodeWin32 )
{
	char pcErreur[ TRC_MAX_TEXTE+1 ];

	if ( FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwCodeWin32,
		0,
		pcErreur,
		TRC_MAX_TEXTE,
		NULL ) != 0 )
		pcErreur[TRC_MAX_TEXTE] = '\0';
	else
		strcpy_s( pcErreur, TRC_MAX_TEXTE+1, TRC_ERREUR_TEXTE );

	return TRC_Trace( psEmetteur, dwOptions, (BYTE*)(&dwCodeWin32), sizeof(dwCodeWin32), "%s", pcErreur );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Direct_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwErreur )
 * PARAMETRES: psEmetteur   : valeur obtenue avec TRC_Initialise
 *             dwOptions: Permet de modifier certaines des options
 *                OU bit à bit entre certaines des valeurs suivantes :
 *                     TRC_OPT_MAILSLOT_LOCAL,
 *                     TRC_OPT_MAILSLOT_DISTANT,
 *                     TRC_OPT_FICHIER,
 *                     TRC_OPT_EVENTLOG,
 *                     TRC_OPT_CONSOLE,
 *                     TRC_OPT_NUMEROTATION
 *                     TRC_OPT_IMMEDIAT
 *                     TRC_OPT_CREER_FICHIER
 *                     TRC_OPT_TEXTE_SEUL
 *                     TRC_OPT_TOUJOURS_OUVERT
 *             dwErreur    : Code d'erreur win32
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Ecrire dans une instance de trace en mode direct
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Direct_Trace_Erreur_Win32( TRC_EMETTEUR psEmetteur, DWORD dwOptions, DWORD dwCodeWin32 )
{
	char pcErreur[ TRC_MAX_TEXTE+1 ];

	if ( FormatMessage( 
		FORMAT_MESSAGE_FROM_SYSTEM|FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		dwCodeWin32,
		0,
		pcErreur,
		TRC_MAX_TEXTE,
		NULL ) != 0 )
		pcErreur[TRC_MAX_TEXTE] = '\0';
	else
		strcpy_s( pcErreur, TRC_MAX_TEXTE+1, TRC_ERREUR_TEXTE );

	return TRC_Direct_Trace( psEmetteur, dwOptions, (BYTE*)(&dwCodeWin32), sizeof(dwCodeWin32), "%s", pcErreur );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Trace( TRC_EMETTEUR psEmetteur )
 * PARAMETRES: psEmetteur : instance de la trace
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Terminer une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Termine_Trace( TRC_EMETTEUR psEmetteur )
{
	DWORD dwErreur = NO_ERROR;
	DWORD dwTaille;
	TRC_MESSAGE psMsg;

    EnterCriticalSection( &gsCriticalInstances );

	if ( HTRC_Etat_Handle( psEmetteur ) == HTRC_VALID_HANDLE )
	{
		psMsg = HeapAlloc( GetProcessHeap(), 0, sizeof(TRC_MESSAGE_STRUCT) );
		if ( psMsg != NULL )
		{
            HTRC_Invalider_Handle( psEmetteur );
			psMsg->dwTaille = 0;
			psMsg->psEmetteur = psEmetteur;
			if ( ! WriteFile( ghPipeW, &psMsg, sizeof( psMsg ), &dwTaille, NULL ) )
			{
				dwErreur = GetLastError();
				HeapFree( GetProcessHeap(), 0, psMsg );
			}
			else
            {
				gdwInit --;
			    if ( gdwInit == 0 )
			    {
				    psMsg = NULL;
				    // Envoyer au thread de réémission un message de fin de thread (pointeur NULL)
				    if ( ! WriteFile( ghPipeW, &psMsg, sizeof( psMsg ), &dwTaille, NULL ) )
					    dwErreur = GetLastError();
				    else
				    {
					    WaitForSingleObject( ghThread, INFINITE );
					    CloseHandle( ghThread );
				    }
			    }
            }
		}
		else
			dwErreur = ERROR_NOT_ENOUGH_MEMORY;
	}
	else
		dwErreur = ERROR_INVALID_DATA;

	LeaveCriticalSection( &gsCriticalInstances );

    SetLastError( dwErreur );

	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Tout( )
 * PARAMETRES: 
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Terminer toutes les instances de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Termine_Tout()
{
    DWORD dwErreur = NO_ERROR;
    TRC_EMETTEUR psEmetteur;

    EnterCriticalSection( &gsCriticalInstances );

    while ( ( (void *)psEmetteur = HTRC_Trouver_Handle( HTRC_VALID_HANDLE, TRC_TYPE_EMISSION ) ) != NULL)
    {
        dwErreur = TRC_Termine_Trace( psEmetteur );
        if ( dwErreur != NO_ERROR )
            break;
    }

    LeaveCriticalSection( &gsCriticalInstances );

    SetLastError( dwErreur );

    return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Initialise_Ecoute( char * pcCle, TRC_RECEVEUR * ppsReceveur )
 * PARAMETRES: pcCle    : Clé de création de la trace
 *             psReceveur : Pointeur sur une structre qui recevra les infos de
 *                        l'écoute.
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Créer un instance d'écoute
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Initialise_Ecoute( char * pcCle, TRC_RECEVEUR * ppsReceveur )
{
	DWORD dwErreur = NO_ERROR;
	char pcMailSlot[MAX_PATH+1] = TRC_PREFIXE_MAILSLOT_RECEPTION;

	(*ppsReceveur) = HTRC_Creer_Handle( sizeof(TRC_RECEVEUR_STRUCT), TRC_TYPE_RECEPTION );

	if ( (*ppsReceveur) != NULL )
	{

		// Construire le nom du mailslot
		strncat_s( pcMailSlot, MAX_PATH+1, pcCle, sizeof(pcMailSlot) );
		pcMailSlot[sizeof(pcMailSlot)-1] = '\0';

		// Créer le mailslot
		(*ppsReceveur)->hMailSlot = CreateMailslot( pcMailSlot, 0, 0, NULL );
		if ( (*ppsReceveur)->hMailSlot == INVALID_HANDLE_VALUE )
		{
			dwErreur = GetLastError();
			HTRC_Detruire_Handle( *ppsReceveur );
		}
		else
			InitializeCriticalSection( &((*ppsReceveur)->sCritical) );
	}
	else
		dwErreur = ERROR_NOT_ENOUGH_MEMORY;

	SetLastError( dwErreur );
	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Ecoute_Message( TRC_RECEVEUR psReceveur, TRC_MESSAGE * ppsMsg )
 * PARAMETRES: psReceveur : Instance d'écoute
 *             ppsMsg   : Récupère un pointeur sur un message. Si NULL, pas de message
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Receptionne des messages de trace.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Ecoute_Message( TRC_RECEVEUR psReceveur, TRC_MESSAGE * ppsMsg)
{
	DWORD dwErreur = NO_ERROR;
	DWORD dwMax, dwProchain, dwNb, dwDelai;

	(*ppsMsg) = NULL;

    if ( HTRC_Etat_Handle( psReceveur ) == HTRC_VALID_HANDLE )
    {
	    EnterCriticalSection( &psReceveur->sCritical );
	    // Test la présence d'un message et détermine sa taille
	    if ( ! GetMailslotInfo( psReceveur->hMailSlot, &dwMax, &dwProchain, &dwNb, &dwDelai ) )
		    dwErreur = GetLastError();
	    else if ( dwProchain != MAILSLOT_NO_MESSAGE )
	    {
		    // Alloue la place nécessaire.
		    (*ppsMsg) = HeapAlloc( GetProcessHeap(), 0, dwProchain );
		    if ( (*ppsMsg) != NULL )
		    {
			    // Lit le message.
			    // La section critique garantie qu'entre le GetMailslotInfo et le ReadFile, aucun
			    // autre thread n'est venu dépilé le message attendu.
			    if ( ! ReadFile( psReceveur->hMailSlot, (*ppsMsg), dwProchain, &dwNb, NULL ) )
			    {
				    dwErreur = GetLastError();
				    HeapFree( GetProcessHeap(), 0, (*ppsMsg) );
				    (*ppsMsg) = NULL;
			    }
		    }
	    }
	    LeaveCriticalSection( &psReceveur->sCritical );
    }
    else
        dwErreur = ERROR_INVALID_HANDLE;

	SetLastError( dwErreur );
	return dwErreur;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Libere_Message( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Libère la mémoire allouée pour la réception d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Libere_Message( TRC_MESSAGE psMsg )
{
	DWORD dwErreur = NO_ERROR;

	// Puisque TRC_Ecoute_Message effectue une allocation,
	// il faut pouvoir désallouer.
	if ( ! HeapFree( GetProcessHeap(), 0, psMsg ) )
		dwErreur = ERROR_INVALID_DATA;

	SetLastError( dwErreur );
	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Termine_Ecoute( TRC_RECEVEUR psReceveur )
 * PARAMETRES: pcCle    : Clé de création de la trace
 *             psReceveur : Pointeur sur une l'instance d'écoute
 * RETOUR    : Un code d'erreur Win32 (NO_ERROR si OK).
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Termine une instance d'écoute
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Termine_Ecoute( TRC_RECEVEUR psReceveur )
{
	DWORD dwErreur = NO_ERROR;

    if ( HTRC_Etat_Handle( psReceveur ) == HTRC_VALID_HANDLE )
    {
        EnterCriticalSection( &psReceveur->sCritical );
    	if ( ! CloseHandle( psReceveur->hMailSlot ) )
		    dwErreur = GetLastError();
	    DeleteCriticalSection( &psReceveur->sCritical );
	    HTRC_Detruire_Handle( psReceveur );
    }
    else
        dwErreur = ERROR_INVALID_HANDLE;

	SetLastError( dwErreur );
	return dwErreur;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL TRC_Message_Direct( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : TRUE si le message a été généré par envoie direct
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le mode d'envoie du message de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL WINAPI TRC_Message_Direct( TRC_MESSAGE psMsg )
{
	return psMsg->bDirect;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void TRC_Message_Date_GMT( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
 *             void TRC_Message_Date_Locale( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
 * PARAMETRES: psMsg   : Message reçu
 *             psDate  : Pointe sur une structure recevant la date GMT ou local
 *                       du message
 * RETOUR    : 
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère la date du message.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
void WINAPI TRC_Message_Date_GMT( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
{
	(*psDate) = psMsg->sDate;
}
void WINAPI TRC_Message_Date_Locale( TRC_MESSAGE psMsg, SYSTEMTIME * psDate )
{
    /* Suppression des fonctions d'ajustement très consommatrices en CPU
	SystemTimeToTzSpecificLocalTime( NULL, &psMsg->sDate, psDate );
    */
    (*psDate) = psMsg->sDate;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Num( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Numéro d'ordre du message, 0 si pas de numérotation.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le numéro d'ordre d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Message_Num( TRC_MESSAGE psMsg )
{
	return psMsg->dwNum;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Tick( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Nombre de ms écoulées depuis dernier reboot.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère le nombre de ms depuis dernier reboot
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Message_Tick( TRC_MESSAGE psMsg )
{
	return psMsg->dwTick;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BYTE * TRC_Message_Donnees( TRC_MESSAGE psMsg )
 *             char * TRC_Message_Texte( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : un pointeur sur les données / le texte d'un message
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère un pointeur sur les données / le texte d'un message.
 *             Attention, si le message est libéré, la valeur retournée
 *             n'est plus utilisable.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BYTE * WINAPI TRC_Message_Donnees( TRC_MESSAGE psMsg )
{
	return psMsg->pbDonnees;
}
char * WINAPI TRC_Message_Texte( TRC_MESSAGE psMsg )
{
	return &(psMsg->pbDonnees[psMsg->dwDonnees]);
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Message_Taille_Donnees( TRC_MESSAGE psMsg )
 *             DWORD TRC_Message_Taille_Texte( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg   : Message reçu
 * RETOUR    : Taille des données / du texte d'un message
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Récupère la taille des données / du texte d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Message_Taille_Donnees( TRC_MESSAGE psMsg )
{
	return psMsg->dwDonnees;
}
DWORD WINAPI TRC_Message_Taille_Texte( TRC_MESSAGE psMsg )
{
	return psMsg->dwTexte;
}





/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Thread_Ecriture( void * pvParm )
 * PARAMETRES: Cf Win32 spec.
 * RETOUR    : Cf Win32 spec.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Thread en boucle sur la lecture dans le pipe interne.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD WINAPI TRC_Thread_Ecriture( void * pvParm )
{
	DWORD dwErreur = NO_ERROR;
	DWORD dwTaille;
	DWORD dwPipe;
	DWORD dwPos;
	// HINSTANCE hDllInst = (HINSTANCE)pvParm;
	TRC_MESSAGE psMsg;

	// Ce thread devant perturber au minimum le reste du process,
	// on lui donner une priorité basse.
	SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_BELOW_NORMAL );

    // Boucle tant qu'aucune erreur n'est arrivée
	while ( dwErreur == NO_ERROR )
	{
		// Ici, la taille du bloc recu est la taille d'un pointeur
		dwTaille = sizeof(psMsg);
		dwPos = 0;
		
		// Tant que la taille totale lu ne constitue pas un pointeur,
		// On lit dans le pipe.
		while ( dwTaille > 0 && dwErreur == NO_ERROR )
		{
			if ( ! ReadFile( 
						ghPipeR,
						((BYTE*)(&psMsg))+dwPos,
						dwTaille,
						&dwPipe,
						NULL )
				)
				dwErreur = GetLastError();
			else
			{
                dwPos += dwPipe;
				if ( ( dwTaille -= dwPipe ) == 0 )
					break;
			}
			Sleep(40);
		}

		// Si pointeur non NULL et aucune erreur
		if ( psMsg != NULL && dwErreur == NO_ERROR )
		{
			if ( psMsg->dwTaille == TRC_LIBERER_INSTANCE )
			{
				// Inutile de proteger l'acces aux handles par gsCriticalInstances,
				// l'appellant a déjà fait un TRC_Terminer_Trace
				if ( psMsg->psEmetteur->hFichier != INVALID_HANDLE_VALUE )
					CloseHandle( psMsg->psEmetteur->hFichier );
				if ( psMsg->psEmetteur->hMailSlot != INVALID_HANDLE_VALUE )
					CloseHandle( psMsg->psEmetteur->hMailSlot );
				if ( psMsg->psEmetteur->hEventLog != NULL )
					DeregisterEventSource( psMsg->psEmetteur->hEventLog );
				HTRC_Detruire_Handle( psMsg->psEmetteur );
			}
			else if ( psMsg->dwTaille == TRC_SIGNALER_EVENEMENT )
            {
				SetEvent( (HANDLE)(psMsg->psEmetteur) );
            }
			else
            {
				TRC_Envoyer_Message( psMsg );
            }
			HeapFree( GetProcessHeap(), 0, psMsg );
		}

		// Si reception de pointeur NULL sans erreur, la fin du thread est
		// demandée.
		if ( psMsg == NULL && dwErreur == NO_ERROR )
        {
			break;
        }
	}

    // Pour permettre au DLL main de s'exécuter
	/* FreeLibraryAndExitThread( hDllInst, */ ExitThread( dwErreur );
	return dwErreur;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : DWORD TRC_Ecrire_Message( TRC_MESSAGE psMsg, char * pcTrace, DWORD dwTrace )
 * PARAMETRES: psMsg     : Message à ecrire sous sa forme texte
 *             pcTrace   : Chaine où écrire le message
 *             dwTrace   : Taille du buffer pointé par pcTrace
 * RETOUR    : Taille du texte (sans le '\0')
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ecriture d'un message sous sa forme texte dans un fichier par i/o win32
 * --------------------------------------------------------------------
 * $F_FCTN
 */
DWORD TRC_Ecrire_Message( TRC_MESSAGE psMsg, char * pcTrace, DWORD dwTrace )
{
	DWORD dwTaille;
	DWORD dwDonnees;
	int iNb;
	DWORD dwSpc;
	BYTE * pbDonnees;
	SYSTEMTIME sDate;

	// Diminuer de 1 le buffer pour prévoir le '\0'
	dwTrace --;

    /* Suppression des fonctions d'ajustement très consommatrices en CPU
	// Conversion de la date GMT au fuseau local pour affichage console ou fichier
	SystemTimeToTzSpecificLocalTime( NULL, &psMsg->sDate, &sDate );
    */
    sDate = psMsg->sDate;

	dwTaille = 0;

    if ( ( psMsg->dwOptions & TRC_OPT_TEXTE_SEUL ) == 0 )
    {
#if(_MSC_VER == 1200)
	    // NUMERO DE SEQUENCE
        if ( psMsg->bDirect )
            iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "%-10s;", "DIRECT" );
	    else if ( psMsg->dwNum != 0 )
		    iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "0x%08X;", psMsg->dwNum );
	    else
		    iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "          ;", psMsg->dwNum );
#else
	    // NUMERO DE SEQUENCE
        if ( psMsg->bDirect )
            iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "%-10s;", "DIRECT" );
	    else if ( psMsg->dwNum != 0 )
		    iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "0x%08X;", psMsg->dwNum );
	    else
		    iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "          ;");
		
#endif
	    // DATE/HEURE/TICK
	    if ( iNb > 0 )
	    {
		    dwTaille += iNb;
#if(_MSC_VER == 1200)
			iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "%04d/%02d/%02d;%02d:%02d:%02d.%03d;%010u;",
				    (int)(sDate.wYear),
				    (int)(sDate.wMonth),
				    (int)(sDate.wDay),
				    (int)(sDate.wHour),
				    (int)(sDate.wMinute),
				    (int)(sDate.wSecond),
				    (int)(sDate.wMilliseconds),
				    psMsg->dwTick );
#else
			iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "%04d/%02d/%02d;%02d:%02d:%02d.%03d;%010u;",
				    (int)(sDate.wYear),
				    (int)(sDate.wMonth),
				    (int)(sDate.wDay),
				    (int)(sDate.wHour),
				    (int)(sDate.wMinute),
				    (int)(sDate.wSecond),
				    (int)(sDate.wMilliseconds),
				    psMsg->dwTick );
#endif
		}
	    // TEXTE
	    if ( iNb > 0 )
		    dwTaille += iNb;
    }
    else
        iNb = 1;
#if(_MSC_VER == 1200)
	// TEXTE
	if ( iNb > 0 )
	{
		if ( psMsg->dwTexte > 0 )
			iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "\"%s\";",
						&(psMsg->pbDonnees[psMsg->dwDonnees]) );
		else
			iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, ";" );
	}
#else
	// TEXTE
	if ( iNb > 0 )
	{
		if ( psMsg->dwTexte > 0 )
			iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "\"%s\";",
						&(psMsg->pbDonnees[psMsg->dwDonnees]) );
		else
			iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, ";" );
	}
#endif

	// DONNEES
	if ( iNb > 0 )
	{
		if ( psMsg->dwDonnees > 0 )
		{
			dwTaille += iNb;
			dwSpc = 0;
			pbDonnees = psMsg->pbDonnees;
			dwDonnees = psMsg->dwDonnees;
#if(_MSC_VER == 1200)
			iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "[ " );
#else
			iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "[ " );
#endif
			while ( dwDonnees > 0 && iNb > 0 )
			{
				dwTaille += iNb;
				dwDonnees --;
				dwSpc = ( dwSpc + 1 ) % 4;
#if(_MSC_VER == 1200)
				iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "%02X %s", 
					(int)(*pbDonnees),
					( dwSpc == 0 && dwDonnees > 0 ) ? "- " : "" );

#else
				iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "%02X %s", 
					(int)(*pbDonnees),
					( dwSpc == 0 && dwDonnees > 0 ) ? "- " : "" );
#endif
				pbDonnees ++;
			}
			if ( iNb > 0 )
			{
				dwTaille += iNb;
#if(_MSC_VER == 1200)
				iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille, "]" );
#else
				iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille, "]" );
#endif
			}
		}
	}

	// FIN (CR+LF)
	if ( iNb > 0 )
	{
		dwTaille += iNb;
#if(_MSC_VER == 1200)
		iNb = _snprintf( pcTrace+dwTaille, dwTrace-dwTaille,"\r\n" );
#else
		iNb = _snprintf_s( pcTrace+dwTaille, dwTrace-dwTaille+1, dwTrace-dwTaille,"\r\n" );
#endif
	}

	if ( iNb > 0 )
		dwTaille += iNb;
	else
		dwTaille = dwTrace;

	pcTrace[dwTaille] = '\0';

	return dwTaille;
}

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void TRC_Envoyer_Message( TRC_MESSAGE psMsg )
 * PARAMETRES: psMsg     : Message à envoyer
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Envoie d'un message
 * --------------------------------------------------------------------
 * $F_FCTN
 */
void TRC_Envoyer_Message( TRC_MESSAGE psMsg )
{
	DWORD dwTaille = 0;
	DWORD dwDonnees;
    DWORD dwIoError;
	char * pcTexte;
	BOOL bOver;
	LARGE_INTEGER liTaille;
	OVERLAPPED sFichier = {0};
	OVERLAPPED sMailSlot = {0};
	char pcTrace[TRC_MAX_TAILLE_LIGNE_TEXTE];

    // Tout peut etre fait hors section critique puisqu'un unique
    // thread fait appel a cette fonction et que c'est lui qui
    // est chargé de la désallocation des structures pointées par
    // le handle psMsg->psEmetteur

	// Si option fichier et fichier non ouvert, on tente de l'ouvrir
	if ( ( psMsg->dwOptions & TRC_OPT_FICHIER ) != 0 &&
		  psMsg->psEmetteur->hFichier == INVALID_HANDLE_VALUE &&
		  psMsg->psEmetteur->pcFichier[0] != '\0' )
    {
        psMsg->psEmetteur->hFichier = CreateFile(
			psMsg->psEmetteur->pcFichier,
			GENERIC_READ|GENERIC_WRITE,
			FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			NULL,
			( psMsg->psEmetteur->dwOptions & TRC_OPT_CREER_FICHIER ) ? OPEN_ALWAYS : OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			NULL );
        psMsg->psEmetteur->bFileMissing = ( psMsg->psEmetteur->hFichier == INVALID_HANDLE_VALUE );
    }

	// Si le fichier est trop grand, on change de fichier
	if ( ( psMsg->dwOptions & TRC_OPT_FICHIER ) != 0 &&
		  psMsg->psEmetteur->hFichier != INVALID_HANDLE_VALUE &&
		  psMsg->psEmetteur->pcFichier[0] != '\0' )
	{
		liTaille.LowPart = GetFileSize( psMsg->psEmetteur->hFichier, &liTaille.HighPart );
		if ( liTaille.LowPart == 0xFFFFFFFF && GetLastError() != NO_ERROR )
		{
			CloseHandle( psMsg->psEmetteur->hFichier );
			psMsg->psEmetteur->hFichier = INVALID_HANDLE_VALUE;
		}
		else if ( liTaille.QuadPart > psMsg->psEmetteur->llTailleFichier )
		{
			CloseHandle( psMsg->psEmetteur->hFichier );
			psMsg->psEmetteur->hFichier = INVALID_HANDLE_VALUE;

			if (IsBackupMovingActive(psMsg->psEmetteur->pcFichier, psMsg->psEmetteur) == BACKUP_MOVING_ACTIVE && gFullBackupPath.bBackupExists){

				MoveToBackupFolder(psMsg->psEmetteur);

			}

			else{

				TRC_Change_Fichier(psMsg->psEmetteur->pcFichier, psMsg->psEmetteur->pcFichierOld);

			}

            psMsg->psEmetteur->hFichier = CreateFile(
				psMsg->psEmetteur->pcFichier,
			    GENERIC_READ|GENERIC_WRITE,
			    FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE,
			    NULL,
				OPEN_ALWAYS,
			    FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
			    NULL );
            psMsg->psEmetteur->bFileMissing = ( psMsg->psEmetteur->hFichier == INVALID_HANDLE_VALUE );
		}
	}

	// Si option mailslot, et mailslot non ouvert, ouverture
	if ( ( psMsg->dwOptions & ( TRC_OPT_MAILSLOT_DISTANT | TRC_OPT_MAILSLOT_LOCAL ) ) != 0 &&
		  psMsg->psEmetteur->hMailSlot == INVALID_HANDLE_VALUE &&
		  psMsg->psEmetteur->pcMailSlot[0] != '\0' )
		psMsg->psEmetteur->hMailSlot = CreateFile(
				psMsg->psEmetteur->pcMailSlot,
				GENERIC_WRITE,
				FILE_SHARE_READ|FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS,
				FILE_ATTRIBUTE_NORMAL|FILE_FLAG_OVERLAPPED,
				NULL );

	// Si option console, et que le handel de la sortie standard n'est pas encore récupéré, on
	// le récupère
	if ( ( psMsg->dwOptions & TRC_OPT_CONSOLE ) != 0 &&
		  psMsg->psEmetteur->hConsole == INVALID_HANDLE_VALUE )
		psMsg->psEmetteur->hConsole = GetStdHandle( STD_OUTPUT_HANDLE );

	if ( ( psMsg->dwOptions & TRC_OPT_EVENTLOG ) != 0 &&
		  psMsg->psEmetteur->hEventLog == NULL &&
		  psMsg->psEmetteur->pcEventLog[0] != '\0' )
		psMsg->psEmetteur->hEventLog = RegisterEventSource( NULL, psMsg->psEmetteur->pcEventLog );

	if ( psMsg->dwOptions & ( TRC_OPT_FICHIER | TRC_OPT_CONSOLE ) )
		dwTaille = TRC_Ecrire_Message( psMsg, pcTrace, sizeof(pcTrace) );

	// ECRITURE DANS LE FICHIER EN OVERLAPPED I/O
	// On commence par cette opération car potentiellement la plus longue
	if ( ( psMsg->dwOptions & TRC_OPT_FICHIER ) != 0 && psMsg->psEmetteur->hFichier != INVALID_HANDLE_VALUE )
	{
		sFichier.Offset = GetFileSize(
			psMsg->psEmetteur->hFichier,
			&(sFichier.OffsetHigh) );
		if ( ! WriteFile( psMsg->psEmetteur->hFichier, pcTrace, dwTaille, NULL, &sFichier ) )
		{
            dwIoError = GetLastError();
            if ( dwIoError != ERROR_IO_PENDING )
            {
			    CloseHandle( psMsg->psEmetteur->hFichier );
			    psMsg->psEmetteur->hFichier = INVALID_HANDLE_VALUE;
            }
		}
	}

	// ECRITURE VERS MAILSLOT EN OVERLAPPED I/O
	if ( ( psMsg->dwOptions & ( TRC_OPT_MAILSLOT_DISTANT | TRC_OPT_MAILSLOT_LOCAL ) ) != 0 && psMsg->psEmetteur->hMailSlot != INVALID_HANDLE_VALUE )
		if ( ! WriteFile( psMsg->psEmetteur->hMailSlot, psMsg, psMsg->dwTaille, NULL, &sMailSlot ) )
		{
            dwIoError = GetLastError();
            if ( dwIoError != ERROR_IO_PENDING )
            {
			    CloseHandle( psMsg->psEmetteur->hMailSlot );
			    psMsg->psEmetteur->hMailSlot = INVALID_HANDLE_VALUE;
            }
		}

	// ECRITURE SUR LA CONSOLE (PENDAND QUE LES I/O MAILSLOT ET FICHIER CONTINUENT)
	if ( ( psMsg->dwOptions & TRC_OPT_CONSOLE ) != 0 )
		WriteFile( psMsg->psEmetteur->hConsole, pcTrace, dwTaille, &dwDonnees, NULL );

	// ENVOIE AU LOG EVENT (PENDAND QUE LES I/O MAILSLOT ET FICHIER CONTINUENT)
	if ( ( psMsg->dwOptions & TRC_OPT_EVENTLOG ) != 0 && psMsg->psEmetteur->hEventLog != NULL )
	{
		pcTexte = &(psMsg->pbDonnees[psMsg->dwDonnees]);
		ReportEvent(
			psMsg->psEmetteur->hEventLog,
			EVENTLOG_INFORMATION_TYPE,
			0,
			0,
			NULL,
			1,
			psMsg->dwDonnees,
			&pcTexte,
			psMsg->pbDonnees );
	}

	// ATTENDRE FIN I/O MAILSLOT
	// Potentiellement le plus cout
	if ( ( psMsg->dwOptions & ( TRC_OPT_MAILSLOT_DISTANT | TRC_OPT_MAILSLOT_LOCAL ) ) != 0 && psMsg->psEmetteur->hMailSlot != INVALID_HANDLE_VALUE )
	{
		bOver = GetOverlappedResult( psMsg->psEmetteur->hMailSlot, &sMailSlot, &dwDonnees, TRUE );
		if ( ( ! bOver ) || dwDonnees != psMsg->dwTaille )
		{
			CloseHandle( psMsg->psEmetteur->hMailSlot );
			psMsg->psEmetteur->hMailSlot = INVALID_HANDLE_VALUE;
		}
	}

	// ATTENDRE FIN I/O FICHIER
	if ( ( psMsg->dwOptions & TRC_OPT_FICHIER ) != 0 && psMsg->psEmetteur->hFichier != INVALID_HANDLE_VALUE )
	{
		bOver = GetOverlappedResult( psMsg->psEmetteur->hFichier, &sFichier, &dwDonnees, TRUE );
		if ( ( ! bOver ) || dwDonnees != dwTaille )
		{
			CloseHandle( psMsg->psEmetteur->hFichier );
			psMsg->psEmetteur->hFichier = INVALID_HANDLE_VALUE;
		}
		else
        {
            if ( ( psMsg->dwOptions & TRC_OPT_IMMEDIAT ) != 0 )
			    FlushFileBuffers( psMsg->psEmetteur->hFichier );
	        // Si pas option toujours ouvert
	        if ( ( psMsg->dwOptions & TRC_OPT_TOUJOURS_OUVERT ) == 0 )
		    {
			    CloseHandle( psMsg->psEmetteur->hFichier );
			    psMsg->psEmetteur->hFichier = INVALID_HANDLE_VALUE;
		    }
        }
	}
}
	



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : BOOL TRC_Option_Valide( DWORD dwOptions, BOOL bFichier )
 * PARAMETRES: dwOptions : Masque d'option d'une instance de trace
 *             bFichier  : Indique si un nom de fichier a été défini
 * RETOUR    : TRUE si les options sont valides, FALSE sinon.
 * --------------------------------------------------------------------
 * VARIABLES :
 * --------------------------------------------------------------------
 * ROLE      : Test les options pour une instance de trace
 * --------------------------------------------------------------------
 * $F_FCTN
 */
BOOL TRC_Option_Valide( DWORD dwOptions, BOOL bFichier )
{
	return !(
		( dwOptions == 0 ) || ( dwOptions == TRC_OPT_NUMEROTATION ) ||
		( dwOptions & ( ~TRC_OPT_MASK ) ) ||
		( ( ! ( dwOptions & TRC_OPT_FICHIER ) ) && bFichier ) ||
		( ( dwOptions & TRC_OPT_FICHIER ) && ( !bFichier) ) ||
		( ( dwOptions & TRC_OPT_IMMEDIAT ) && ( ! ( dwOptions & TRC_OPT_FICHIER ) ) ) ||
		( ( dwOptions & TRC_OPT_TOUJOURS_OUVERT ) && ( ! ( dwOptions & TRC_OPT_FICHIER ) ) ) ||
		( ( dwOptions & TRC_OPT_CREER_FICHIER ) && ( ! ( dwOptions & TRC_OPT_FICHIER ) ) ) ||
		( dwOptions & !( TRC_OPT_FICHIER|TRC_OPT_MAILSLOT_LOCAL|TRC_OPT_MAILSLOT_DISTANT|TRC_OPT_CONSOLE ) ) ||
		( ( dwOptions & TRC_OPT_MAILSLOT_LOCAL ) && ( dwOptions & TRC_OPT_MAILSLOT_DISTANT ) )
			);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL WINAPI TRC_Ferme_Proc ( void * pvHandle, void * pvContext )
 * PARAMETERS: void * pvHandle  : 
 *             void * pvContext : 
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 * --------------------------------------------------------------------
 */
BOOL WINAPI TRC_Ferme_Proc( void * pvHandle, void * pvContext )
{
    char * pcFichier = (char *)pvContext;
    TRC_EMETTEUR psEmetteur = pvHandle;

    if ( _stricmp( psEmetteur->pcFichier, pcFichier ) == 0 )
    {
        CloseHandle( psEmetteur->hFichier );
        psEmetteur->hFichier = INVALID_HANDLE_VALUE;
    }
    return TRUE;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : void TRC_Change_Fichier ( char * pcFichier, char * pcFichierOld )
 * PARAMETERS: char * pcFichier    : nom du fichier à fermer complètement
 *             char * pcFichierOld : nom du fichier de backup
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme toutes les instances ouvertes d'un fichier donné
 * --------------------------------------------------------------------
 */
void TRC_Change_Fichier( char * pcFichier, char * pcFichierOld )
{
    HTRC_Enum_Handle( TRC_TYPE_EMISSION, &TRC_Ferme_Proc, (void *)pcFichier );

    MoveFileEx( pcFichier, pcFichierOld, MOVEFILE_REPLACE_EXISTING|MOVEFILE_WRITE_THROUGH);

}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : HINSTANCE TRC_Incrementer_Usage_DLL ( HINSTANCE hinst )
 * PARAMETERS: HINSTANCE hinst : Instance de la DLL a recharger
 * RETURN    : Instance de la DLL rechargée
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Incrémente l'usage d'une DLL
 * --------------------------------------------------------------------
 */
HINSTANCE TRC_Incrementer_Usage_DLL(HINSTANCE hinst)
{
	char pcModule[_MAX_PATH];
	GetModuleFileName(hinst, pcModule, _MAX_PATH);
	return LoadLibrary(pcModule);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL TRC_Test_Si_Actif( char * pcFichier )
 * PARAMETERS: pcFichier : Chemin+Nom du fichier
 * RETURN    : TRUE si la trace associée à ce fichier est activée
 *             FALSE si elle est désactivée.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Détermine si une trace associée à un fichier est
 *             activée ou non.
 * --------------------------------------------------------------------
 */
BOOL TRC_Test_Si_Actif( char * pcFichier )
{
    HKEY hKey;
    DWORD dwType;
    DWORD dwSize;
    BYTE tbBuffer[100];
    DWORD * pdwData = (DWORD*)tbBuffer;
    char * pcName;

    if ( pcFichier == NULL )
        return TRUE;

    pcName = strrchr( pcFichier, '\\' );
    if ( pcName == NULL )
        pcName = strrchr( pcFichier, ':' );
    if ( pcName == NULL )
        pcName = pcFichier;
    else
        pcName ++;
    
    if ( (*pcName) == '\0' )
        return TRUE;

    if ( RegCreateKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, NULL, 0, KEY_ALL_ACCESS, NULL, &hKey, NULL ) != ERROR_SUCCESS )
        return TRUE;
    
    dwSize = sizeof(tbBuffer);
    dwType = REG_DWORD;
    if ( RegQueryValueEx( hKey, pcName, NULL, &dwType, (LPBYTE)pdwData, &dwSize ) != ERROR_SUCCESS )
    {
        (*pdwData) = 1;
        dwSize = sizeof(*pdwData);
        dwType = REG_DWORD;
        RegSetValueEx( hKey, pcName, 0, dwType, (LPBYTE)pdwData, dwSize );
        RegCloseKey( hKey );
        return TRUE;
    }
    RegCloseKey( hKey );

    if ( dwSize != sizeof(*pdwData) )
        return TRUE;

    return ( (*pdwData) != 0 );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : void TRC_ParametrageDefaut()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère le paramétrage par défaut tel que défini dans le
 *             registre.
 * --------------------------------------------------------------------
 */
void TRC_ParametrageDefaut()
{
    DWORD dwMaxPipeItems = TRC_TAILLE_QUEUE;
    DWORD dwRegDword;
    DWORD dwSize;
    DWORD dwType;
    HKEY  hKey;

    // Accés au registre en lecture
    if ( RegOpenKeyEx( HKEY_LOCAL_MACHINE, "SOFTWARE\\CSRoute\\Trc", 0, KEY_READ, &hKey ) != ERROR_SUCCESS )
    {
        // On ne change rien
        return;
    }

    // Aller chercher la taille de la FIFO dans le registre
    dwSize = sizeof(dwRegDword);
    dwType = REG_DWORD;
    if ( RegQueryValueEx( hKey, "MaxPipeItems", NULL, &dwType, (LPBYTE)&dwRegDword, &dwSize ) == ERROR_SUCCESS )
    {
        if ( ( dwType == REG_DWORD ) && ( dwSize == sizeof(dwRegDword) ) )
            gdwTailleFifoTrace = dwRegDword;
    }
	else
		gdwTailleFifoTrace = 0xFFFF; //set default pipe items to a big number 

    dwSize = sizeof(dwRegDword);
    dwType = REG_DWORD;
    if ( RegQueryValueEx( hKey, "DefaultMaxFileSize", NULL, &dwType, (LPBYTE)&dwRegDword, &dwSize ) == ERROR_SUCCESS )
        if ( ( dwType == REG_DWORD ) && ( dwSize == sizeof(dwRegDword) ) )
            gdwTailleFichierDefaut = dwRegDword;

    RegCloseKey( hKey );
}



/*---------------------------- FIN DU FICHIER -------------------------*/
