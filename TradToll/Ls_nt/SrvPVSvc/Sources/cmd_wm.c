/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE DE RECEPTION DES MESSAGE DES CMD
* FICHIER: cmd_wm.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter les window messages destiné
*         aux autoamte de CMD  
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/cmd_wm.c_v  $
 * 
 *    Rev 1.5   26 Apr 2002 11:44:50   dsilberm
 * Version issu des modif apportees sur
 * dartford concernant les classes en
 * temps reelles
 * 
 *    Rev 1.3   13 Dec 2000 16:34:46   DSI
 * Ajout de la possibilite de crypter les mots
 * de passe
 * 
 *    Rev 1.0   Nov 22 1999 11:00:30   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/
#include <windows.h>
#include <acom.h>
#include <ntsvc.h>
#include <cmd_acom.h>
#include <cmd_wm.h>
#include <cmd.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

#define MAX_CMD_CONNECTIONS 10

// Nom texte du window message utilisé pour signaler les événements
#define CMD_WM_EVENT_NAME   "WM_CMD_EVENT"

/*--------------- TYPEDEFS: ----------------*/

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

PRIVATE struct
{
    // infos sur le thread avec sa file de "window messages"
    HANDLE hThread;
    DWORD ThreadId;
    
    DWORD WM_ACOM;
    DWORD WM_CMD;
} 
CMD = {0};

/*--------------- FUNCTIONS: ---------------*/

PRIVATE BOOL CMD_Connexion (ACOM_EVT_HANDLE hEvent)
{
    NTSVCInfo( "CMD_Connexion() => open connection" );

    return TRUE;
}

PRIVATE BOOL CMD_Disconnexion (ACOM_EVT_HANDLE hEvent)
{
    NTSVCInfo( "CMD_Disconnexion() => close connection" );

    return TRUE;
}
    
PRIVATE BOOL CMD_Receive (ACOM_EVT_HANDLE hEvent)
{
	DWORD64 dwInstUsrKey;
	DWORD64 hCnxHandle;
    DWORD	dwDataSize;
    void	*pvDataBuffer;
    char	*pcCmd;
  
    // on récupère le handle de connexion et les données associées
    AComGetReceivedEvtParameters (hEvent, 
                                  &dwInstUsrKey, 
                                  &hCnxHandle,
                                  &dwDataSize,
                                  &pvDataBuffer);

    NTSVCInfo( "CMD_Receive() => message received" );
    
    // Allouer un bloc suffisament grand pour pouvoir placer un marqueur de fin de chaine
    pcCmd = HeapAlloc( GetProcessHeap(), 0, dwDataSize + 1 );
    if ( pcCmd != NULL )
    {
        // Recoupier le bloc
        CopyMemory( pcCmd, pvDataBuffer, dwDataSize );
        // Forcer la présence d'un marqueur de fin de chaines
        pcCmd[dwDataSize] = '\0';
        // Donner le bloc à manger au traitement des commandes
        CmdReceive( (DWORD)hCnxHandle, dwDataSize, pcCmd );
        // Un fois traité, le libérer
        HeapFree( GetProcessHeap(), 0, pcCmd );
    }
    else
        // Erreur d'allocation
		NTSVC_ERR1("ERROR: CMD_Receive() => Error (%u) memory allocation: HeapAlloc()!", ERROR_NOT_ENOUGH_MEMORY);

    return TRUE;
}

PRIVATE DWORD WINAPI CMD_WM_Dispatch (PVOID param)
{
    MSG msg;
    INT ret;

    // Création d'une file de window messages pour ce thread
    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

    // boucle de réception des messages
    do
    {
        // attente d'un message
        ret = GetMessage (&msg, NULL, 0, 0);

        // erreur de réception
        if (ret < 0)
            continue;

        // un message a traiter venant des CMD via ACOM ?
        if (msg.message == CMD.WM_ACOM)
        {
            switch (msg.wParam)
            {
            case ACOM_WM_SHUTDOWN:
                NTSVC_ERR ("CMD_WM_Dispatch() => ACOM_WM_SHUTDOWN");
                ret = FALSE;
                break;

            case ACOM_WM_CONNECTION:
                ret = CMD_Connexion (msg.lParam);
                break;

            case ACOM_WM_DISCONNECTION:
                ret = CMD_Disconnexion (msg.lParam);
                break;

            case ACOM_WM_RECEIVED:
                ret = CMD_Receive (msg.lParam);
                break;

            case ACOM_WM_SENT:
                // tant mieux...
                break;

            case ACOM_WM_TIME_SHIFT:
                // TO DO : mise a l'heure ??
                break;

            default:
                NTSVC_ERR ("CMD_WM_Dispatch() => WM_ACOM unknown!");
                break;
            }
            
            // on libère l'event associé au message de ACOM
            AComReleaseEvt (msg.lParam);
        }
        // un message a traiter pour SRV PV ?
        else if (msg.message == CMD.WM_CMD)
        {
            switch (msg.wParam)
            {
            /*
            case CMD_WM_PLAZA_STATE:
                ret = PV_Dispatch_Message (EVT_PV_PLAZA_STATE, (HMSG) msg.lParam);
                break;
            */
            default:
                NTSVC_ERR ("CMD_WM_Dispatch() => WM_CMD unknown!");
                break;
            }
        }
        else if (msg.message == WM_QUIT)
        {
            ret = 0;
            break;
        }
        else
        {
            NTSVC_ERR ("CMD_WM_Dispatch() => WM unknown!");
        }

    }
    // ret == 0 sur réception de WM_QUIT
    while (ret > 0);

   return 0;
}

PROTECTED BOOL CMD_Start (void)
{
    // RAZ de la strucutre interne
    memset (&CMD, 0, sizeof (CMD));

    // recherche des WM du module de COM réseau
    CMD.WM_ACOM = RegisterWindowMessage (ACOM_WM_EVENT_NAME);
    if (CMD.WM_ACOM == 0)
        return FALSE;

    // déclaration des WM du module lui-meme
    CMD.WM_CMD = RegisterWindowMessage (CMD_WM_EVENT_NAME);
    if (CMD.WM_CMD == 0)
        return FALSE;

    // création du thread du module
    CMD.hThread = NTSVCCreateThread (NULL, 0, CMD_WM_Dispatch, NULL, 0, &CMD.ThreadId, "SRVPVSVC CMD Thread");
    if (CMD.hThread == NULL)
        return FALSE;

    // Démarrage de la COM réseau
    if (CMD_ACOM_Start (CMD.ThreadId) == FALSE)
        return FALSE;

    // init de la gestion des paramètres du registre
    return CmdOpenParamList ();
}
    
PROTECTED BOOL CMD_Stop (DWORD TimeOut)
{
    // arret de la gestion des paramètres du registre
    CmdCloseParamList ();

    // Arret de la COM réseau
    if (CMD_ACOM_Terminate () == FALSE)
        return FALSE;

    // on envoie un message au thread lui demandant de se terminer
    if (PostThreadMessage (CMD.ThreadId, WM_QUIT, 0, 0) == FALSE)
        return FALSE;

    // on attend la disparition du thread avec délai
    if (WaitForSingleObject (CMD.hThread, TimeOut) != WAIT_OBJECT_0)
        return FALSE;

    // libération du handle de thread
    CloseHandle( CMD.hThread );

    return TRUE;
}

PROTECTED BOOL CMD_PostMessage (enum CMD_WM_TYPE type, PVOID Param)
{
	return PostThreadMessage(CMD.ThreadId, CMD.WM_CMD, (WPARAM)type, (LPARAM)Param);
}
