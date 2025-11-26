/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SERCom
 * FILE       : SERCom.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include "ser.h"
#define LOC_DEF
#include "SERCom.h"
#undef LOC_DEF

#include <memclass.h>




//
// Définition de l'objet SER, seul le nom du type de la structure est
// exporté, la définition complète reste privée
//
typedef struct _SERCOM_INSTANCE
{
    SER_TYPE            eType;

    // Numéro du port série
    DWORD               dwPort;

    // Handle du fichier ouvert
    HANDLE              hPort;

    // Handle de l'événement utilisé pour les overlapped i/o
    HANDLE              hEventRead;

    // Structure utilisée pour les overlapped i/o
    OVERLAPPED          sOverRead;

    // Handle de l'événement utilisé pour les overlapped i/o
    HANDLE              hEventWrite;
    
    // Structure utilisée pour les overlapped i/o
    OVERLAPPED          sOverWrite;
    
    // Bloc de contrôle du port
    DCB                 sDcb;

    // Paramètres mémorisés avant utilisation du port,
    // pour restitution à postériori
    DCB                 sBackupDcb;
    COMMTIMEOUTS        sBackupTO;
    DWORD               dwBackupMask;
    BOOL                fBackup;

    // Indicateur d'input pending
    BOOL                fPending;

    // Position du prochain octet à lire
    DWORD               dwNextByte;

    // Nombre d'octets à partir de la position du prochain
    DWORD               dwBytes;

    // Buffer de reception
    BYTE                tbInBuffer[128];

}
    SERCOM_INSTANCE;




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComOpen(
 *                      OUT SERCOM_INSTANCE    ** ppsPort,
 *                      IN DWORD                dwPort,
 *                      IN char               * pcPortSettings )
 * PARAMETRES: ppsPort        : Récupère un "handle" de l'objet créé
 *             dwPort         : Numéro du port
 *             pcPortSettings : Chaine d'init du port au format de la commande MODE
 * RETOURNE  : NO_ERROR si le l'objet a été crée, sinon, un code Win32
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une instance de l'objet SER. Ce même objet
 *             pourra être détruit avec la fonction SERClose().
 *             Pour obtenir un handle sur lequel peuvent être effectuée
 *             des opérations du type WaitForXXXX(), utiliser
 *             la fonction SERGetWaitableHandle().
 * REMARQUE  : La syntaxe de pcPortSetings est la suivante :
 *              "[BAUD=b] [PARITY=p] [DATA=d] [STOP=s] [to=on|off] [xon=on|off] [odsr=on|off]
 *               [octs=on|off] [dtr=on|off|hs] [rts=on|off|hs|tg] [idsr=on|off]"
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD SERComOpen(
        OUT SERCOM_INSTANCE    ** ppsPort,
        IN DWORD                dwPort,
        IN char               * pcPortSettings )
{
    BOOL            bResult;
    DWORD           dwErr       = NO_ERROR;     // Code d'erreur courant
    SERCOM_INSTANCE * psPort      = NULL;         // Pointeur de travail
    COMMTIMEOUTS    sTimeouts;
    char            szName[32]  = "\\\\.\\COM"; // Pour construire le chemin d'accés au port série

    __try
    {
        //
        // Allouer un bloc de mémoire pour stocker les données de la nouvelle
        // instance de timer, puis le remplir avec les données nécessaire
        //
        psPort = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psPort ) );
        if ( ! psPort ) 
        { 
            //
            // La tentative d'allocation d'un bloc a échoué
            //
            dwErr = ERROR_NOT_ENOUGH_MEMORY; 
            __leave; 
        }

        psPort->eType = SER_COM;

        //
        // On doit créer un événement pour chaque type d'i/o overlapped
        // qu'on va utiliser. Dans notre cas : READ / WRITE
        //
        psPort->hEventRead = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( ! psPort->hEventRead ) 
        {
            //
            // Impossible de créer l'événement de gestions des
            // lectures en mode overlapped
            //
            dwErr = GetLastError(); 
            __leave; 
        }
        psPort->hEventWrite = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( ! psPort->hEventWrite ) 
        { 
            //
            // Impossible de créer l'événement de gestions des
            // écriture en mode overlapped
            //
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // Contruire le chemin d'accés au port série
        //
        _itoa( dwPort, &szName[strlen(szName)], 10 );

        //
        // Ouverture du port en mode overlapped
        //
        psPort->hPort = CreateFile(
            szName,
            GENERIC_READ | GENERIC_WRITE,
            0,
            NULL,
            OPEN_EXISTING,
            FILE_ATTRIBUTE_NORMAL |  FILE_FLAG_OVERLAPPED ,
            NULL );
        if ( psPort->hPort == INVALID_HANDLE_VALUE )
        { 
            //
            // L'ouverture du port a échoué
            //
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // Avant de configurer le reste des paramètres de fonctionnement du port,
        // on mémorise les paramètres courants.
        //

        psPort->sBackupDcb.DCBlength = sizeof(psPort->sBackupDcb);
        if ( ! GetCommState( psPort->hPort, &psPort->sBackupDcb ) )
        { 
            dwErr = GetLastError(); 
            __leave; 
        }

        if ( ! GetCommTimeouts( psPort->hPort, &psPort->sBackupTO ) )
        { 
            dwErr = GetLastError(); 
            __leave; 
        }

        if ( ! GetCommMask( psPort->hPort, &psPort->dwBackupMask ) )
        { 
            dwErr = GetLastError(); 
            __leave; 
        }

        psPort->fBackup = TRUE;

        //
        // Configurer la taille des buffer d'entrée et de sortie du port.
        //
        if ( ! SetupComm( psPort->hPort, 2048, 2048 ) )
        { 
            //
            // Le port série refuse ces tailles de buffer
            //
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // Configurer le reste des paramètres de fonctionnement du port.

        //
        // Prendre un bloc de configuration vide
        //
        ZeroMemory( &psPort->sDcb, sizeof(psPort->sDcb) );

        psPort->sDcb.DCBlength = sizeof(psPort->sDcb);
        
        //
        // Y appliquer les données de configuration qui sont dans la
        // chaine passée en paramètre.
        //
        if ( ! BuildCommDCB( pcPortSettings, &psPort->sDcb ) )
        { 
            //
            // La chaine de configuration du port n'est pas acceptée
            //
            dwErr = GetLastError(); 
            __leave; 
        }


        // psPort->sDcb.fBinary = TRUE;
        psPort->sDcb.XonChar = 2;
        psPort->sDcb.XoffChar = 3;

        //
        // Utiliser le bloc obtenu pour configurer le port
        //
        if ( ! SetCommState( psPort->hPort, &psPort->sDcb ) )
        { 
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // Définir les timeout par défaut. Dans la mesure où la réactivité
        // qu'on souhaite avoir est maximum. Et dans la mesure ou la taille
        // mini d'un envoi est 1 octet (caractères de contrôle), on desactive
        // touts le mécanismes de timeout.
        //
        sTimeouts.ReadIntervalTimeout = MAXDWORD;
        sTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
        sTimeouts.ReadTotalTimeoutConstant = MAXDWORD - 1;
        sTimeouts.WriteTotalTimeoutMultiplier = 0;
        sTimeouts.WriteTotalTimeoutConstant = 0;
        if ( ! SetCommTimeouts( psPort->hPort, &sTimeouts ) )
        { 
            //
            // Le port refuse la désactivation des timeouts
            //
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // On laisse le soin au port de gérer l'ensemble des événement
        // autre que les i/o. C'est à dire DSR / CTS / RTS etc.
        // Cela signifie donc que la chaine de configuration du port
        // doit en tenir compte. Par exemple, si l'état de la ligne
        // DSR est utilisé, cela doit être signifié dans la chaine de
        // configuration avec un "odsr=on"
        //
        if ( ! SetCommMask( psPort->hPort, 0 ) )
        { 
            //
            // Le port refuse la desactivation du masque d'événements
            //
            dwErr = GetLastError(); 
            __leave; 
        }

        //
        // Finalement, on peut amorcer l'attente de caractère
        // par un lecture overlapped du port.
        //

        //
        // Initialiser la structure overlapped pour la lecture
        //
        ZeroMemory( &psPort->sOverRead, sizeof( psPort->sOverRead ) );
        ResetEvent( psPort->sOverRead.hEvent = psPort->hEventRead );

        //
        // Lancer la lecture
        //
        bResult = ReadFile(
            psPort->hPort,
            psPort->tbInBuffer,
            sizeof(psPort->tbInBuffer),
            NULL,
            &psPort->sOverRead );
        if ( ( dwErr = ( bResult ? NO_ERROR : GetLastError() ) ) != NO_ERROR )
        {
            //
            // Une erreur a eu lieu
            //
            if ( dwErr != ERROR_IO_PENDING )
                __leave;
            //
            // Si l'erreur signale une I/O mise en attente, c'est le comportement
            // attendu, il n'y a donc pas d'erreur.
            //
            dwErr = NO_ERROR;
        }

        psPort->fPending = TRUE;
    }
    __finally
    {
        if ( dwErr != NO_ERROR )
        {
            //
            // Une erreur a eu lieu on libère tout ce qui a été alloué
            //
            SERComClose( psPort );

            psPort = NULL;
        }

        //
        // Retourner le handle de l'objet créé ou NULL
        (*ppsPort) = psPort;

    }

    return dwErr;
}


    

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI SERComClose(
 *                      IN SERCOM_INSTANCE  * psPort )
 * PARAMETRES: psPort : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Destruction d'une instance de l'objet SER créé
 *             avec SEROpen(). Si un handle a été obtenu par l'intermédiaire
 *             de l'objet avec la fontion SERGetWaitableHandle(), celui-ci
 *             est également fermé.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED void SERComClose(
        IN SERCOM_INSTANCE  * psPort )
{
    //
    // On s'assure d'abord que le bloc de l'objet a effectivement été alloué
    //
    if ( psPort != NULL )
    {

        //
        // Cette fonction peut être appelé aussi bien par l'utilisateur
        // de la libraire que par SEROpen lorsque l'ouverture a échoué
        // en cours d'initialisation. Il ne faut donc libérer que les
        // ressources réellement allouées.
        //

        if ( psPort->fBackup )
        {
            //
            // Restauration des valeurs de paramètres mémorisées
            //
            SetCommState( psPort->hPort, &psPort->sBackupDcb );
            SetCommTimeouts( psPort->hPort, &psPort->sBackupTO );
            GetCommMask( psPort->hPort, &psPort->dwBackupMask );
        }

        if ( psPort->hPort != NULL && psPort->hPort != INVALID_HANDLE_VALUE )
        {
            //
            // Le port a été ouvert. On annule donc tout i/o en court et
            // surtout, on referme le port
            //
            CancelIo( psPort->hPort );
            CloseHandle( psPort->hPort );
        }

        //
        // Fermeture des événements pour les overlapped i/o
        //
        if ( psPort->hEventRead != NULL )
            CloseHandle( psPort->hEventRead );
        if ( psPort->hEventWrite != NULL )
            CloseHandle( psPort->hEventWrite );

        //
        // Finalement libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psPort );
    }
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT HANDLE WINAPI SERComGetWaitableHandle(
 *                      IN SERCOM_INSTANCE  * psPort )
 * PARAMETRES: psPort : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Un handle d'objet utilisable avec les fontions WaitForXXX
 *             qui passe à l'état signalé lorsque des caractères sont
 *             présents dans le buffer de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir un handle lié au port sur lequel on peut
 *             utiliser les fontions WaitForXXX
 * --------------------------------------------------------------------
 * $F_FCTN
 */

PROTECTED HANDLE SERComGetWaitableHandle(
        IN SERCOM_INSTANCE  * psPort )
{
    return psPort->hEventRead;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComWrite(
 *                      IN SERCOM_INSTANCE  * psPort,
 *                      IN BYTE           * pbBuffer,
 *                      IN DWORD            dwBufferBytes )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pbBuffer      : pointe sur le buffer contenant les octets à envoyer
 *             dwBufferBytes : Nombre d'octets pointés par pbBuffer
 * RETOURNE  : NO_ERROR lorsque tous les octets ont été envoyées,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Envoyer une série d'octets sur une liaison série
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD SERComWrite(
        IN SERCOM_INSTANCE * psPort,
        IN BYTE           * pbBuffer,
        IN DWORD            dwBufferBytes )
{
    DWORD   dwErr           = NO_ERROR; // Code d'erreur courant
    DWORD   dwSentBytes;                // Nombre d'octets envoyées
    BOOL    bResult;                    // Résultat d'appel système
    DWORD   dwBytesToSend   = dwBufferBytes;
    BYTE *  pbBufferToSend  = pbBuffer;

    __try
    {

        //
        // Boucler tant qu'il y a des octets à envoyer
        //
        while ( dwBufferBytes > 0 )
        {
            //
            // Initialiser la structure overlapped d'écriture
            //
            ZeroMemory( &psPort->sOverWrite, sizeof( psPort->sOverWrite ) );

            //
            // S'assurer que l'événement est non signalé
            //
            ResetEvent( psPort->sOverWrite.hEvent = psPort->hEventWrite );

            //
            // Lance l'opération d'écriture
            //
            bResult = WriteFile(
                psPort->hPort,
                pbBuffer,
                dwBufferBytes,
                NULL,
                &psPort->sOverWrite );
            if ( ( dwErr = ( bResult ? NO_ERROR : GetLastError() ) ) != NO_ERROR )
            {
                //
                // Un erreur a eu lieu
                //
                if ( dwErr != ERROR_IO_PENDING )
                    __leave; 

                //
                // Si l'erreur signale une I/O mise en attente, c'est le comportement
                // attendu, il n'y a donc pas d'erreur.
                //
                dwErr = NO_ERROR;
            }

            //
            // Attente de la fin de l'i/o
            //
            bResult = GetOverlappedResult(
                psPort->hPort,
                &psPort->sOverWrite,
                &dwSentBytes,
                TRUE );
            if ( ! bResult ) 
            { 
                //
                // Une erreur a eu lieu pendant l'émission
                //
                dwErr = GetLastError(); 
                __leave; 
            }

            //
            // Attention, si 0 octet émis, c'est une erreur
            //
            if ( dwSentBytes == 0 )
            { 
                dwErr = ERROR_OPERATION_ABORTED; 
                __leave; 
            };

            //
            // Mettre à jour le nombre d'octet restant à émettre,
            // ainsi que le pointeur du premier octet à émettre.
            //
            dwBufferBytes -= dwSentBytes;
            pbBuffer += dwSentBytes;
        }
    }
    __finally
    {
    }

    return dwErr;
}
            

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComRead(
 *                      IN SERCOM_INSTANCE      * psPort,
 *                      IN OPTIONAL BYTE      * pbBuffer,
 *                      IN OUT OPTIONAL DWORD * pdwBufferBytes )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pbBuffer      : pointe sur le buffer récupérant les octets
 *             pdwBufferBytes: En entrée, nombre max d'octets dans le buffer
 *                             En sortie, nombre d'octets reçus
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Recevoir des octets sur la liaison série.
 *             Si aucun octet disponible, (*pdwBufferBytes) = 0
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD SERComRead(
        IN SERCOM_INSTANCE       * psPort,
        IN OPTIONAL BYTE      * pbBuffer,
        IN OUT OPTIONAL DWORD * pdwBufferBytes )
{
    DWORD   dwErr       = NO_ERROR; // Code d'erreur courant
    DWORD   dwBytes     = 0;        // Nombre d'octets à placer dans le buffer utilisteur
    DWORD   dwRead;                 // Nombre d'octets à placer dans le buffer utilisteur
    DWORD   dwRemain;               // Reste à combler
    BOOL    bResult;                // Résultat d'appel système

    __try
    {
        //
        // Tant que le buffer de lecture n'est pas saturé
        //
        while ( dwBytes < *pdwBufferBytes )
        {
            //
            // Si le buffer série contient déjà des infos, on les récupère
            //
            if ( psPort->dwBytes > 0 )
            {
                //
                // Nombre d'octets restant à récupérer
                //
                dwRemain = ( *pdwBufferBytes - dwBytes );

                if ( dwRemain < psPort->dwBytes )
                {
                    //
                    // Il y a suffisament d'octets, on récupère ce dont on a besoin
                    // et on sort.
                    //

                    CopyMemory( pbBuffer + dwBytes, 
                                psPort->tbInBuffer + psPort->dwNextByte, 
                                dwRemain );
                    dwBytes += dwRemain;
                    psPort->dwNextByte += dwRemain;
                    psPort->dwBytes -= dwRemain;

                    dwErr = NO_ERROR;
                    __leave;
                }
                else
                {
                    //
                    // Le buffer n'a pas assez d'infos demandées, on récupère ce qu'on
                    // peut sans sortir pour ensuite aller voir ce qu'on a en résultat
                    // overlapped.
                    //

                    CopyMemory( pbBuffer + dwBytes, 
                                psPort->tbInBuffer + psPort->dwNextByte, 
                                psPort->dwBytes );
                    dwBytes += psPort->dwBytes;
                    psPort->dwBytes = 0;
                    psPort->dwNextByte = 0;
                }
            }       

            //
            // A ce niveau, le buffer est vide.
            //
            psPort->dwNextByte = 0;
            psPort->dwBytes = 0;


            if ( psPort->fPending )
            {
                //
                // Si on est pending, on teste l'état de la lecture assynchrone
                //
                bResult = GetOverlappedResult(
                    psPort->hPort,
                    &psPort->sOverRead,
                    &dwRead,
                    FALSE );
                if ( ( dwErr = ( bResult ? NO_ERROR : GetLastError() ) ) != NO_ERROR )
                {
                    //
                    // Un erreur a eu lieu
                    //
                    if ( ( dwErr == ERROR_IO_PENDING ) || ( dwErr == ERROR_IO_INCOMPLETE ) )
                    {
                        //
                        // Si l'i/o est en attente ou en cours, il ne s'agit pas d'une erreur,
                        // mais aucun caractère n'est à lire, on peut donc sortir de la boucle
                        //
                        break;
                    }
                    else
                    {

                        //
                        // Si l'erreur autre chose qu'une I/O mise en attente ou en cours
                        //
                        dwBytes = 0;
                        // CancelIo( psPort->hPort ); 
                        __leave;
                    }
                }

                psPort->dwBytes = dwRead;
                psPort->fPending = FALSE;

            }

            if ( ( psPort->dwBytes == 0 ) && ( ! psPort->fPending ) )
            {
                //
                // On est pas pending et le buffer est vide, on doit
                // donc réamorcer la lecture
                //
                ZeroMemory( &psPort->sOverRead, sizeof( psPort->sOverRead ) );
                psPort->sOverRead.hEvent = psPort->hEventRead;
                ResetEvent( psPort->sOverRead.hEvent );

                //
                // Lancer la lecture
                //
                bResult = ReadFile(
                    psPort->hPort,
                    psPort->tbInBuffer,
                    sizeof(psPort->tbInBuffer),
                    NULL,
                    &psPort->sOverRead );
                if ( ( dwErr = ( bResult ? NO_ERROR : GetLastError() ) ) != NO_ERROR )
                {
                    //
                    // Un erreur a eu lieu
                    //
                    if ( dwErr != ERROR_IO_PENDING )
                    {
                        //
                        // Si l'erreur est autre chose qu'une I/O mise en attente
                        //
                        dwBytes = 0;
                        __leave;
                    }
    
                    psPort->fPending = TRUE;
                    break;
                }
    
                //
                // Si ReadFile retourne OK, l'événement est lié à la structure
                // overlapped est passé à l'état signalé, on conserve donc
                // l'état pending qui va permettre de faire un appel à GetOverlappedResult().
                //

                psPort->fPending = TRUE;
            }
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        //
        // Mettre à jour l'état du buffer
        //
        (*pdwBufferBytes) = dwBytes;
    }
        
    return dwErr;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComEmptyBuffer(
 *                      IN SERCOM_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Vider le buffer du port comm
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD SERComEmptyBuffer(
        IN SERCOM_INSTANCE      * psPort )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur courant
    DWORD       dwCount;            // Nombre de caractères à lire
    static BYTE tcChar[sizeof(psPort->tbInBuffer) + 1];         // Caractères lus

    do
    {
        //
        // Eliminer les caractères immédiatement disponibles
        //
        dwCount = sizeof(tcChar);
        dwErr = SERComRead( psPort, tcChar, &dwCount );
        if ( dwErr != NO_ERROR )
            break;
    }
    while ( dwCount == ( sizeof(psPort->tbInBuffer) + 1 ) );
    
    return dwErr;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComGetLineStatus( 
 *                      IN              SERCOM_INSTANCE      * psPort,
 *                      OPTIONAL OUT    BOOL              * pfCTS,
 *                      OPTIONAL OUT    BOOL              * pfDSR,
 *                      OPTIONAL OUT    BOOL              * pfRing,
 *                      OPTIONAL OUT    BOOL              * pfRLSD )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 *             pfCTS         : Récupère l'état de la ligne CTS  (TRUE=ON / FALSE=OFF)
 *             pfDSR         : Récupère l'état de la ligne DSR  (TRUE=ON / FALSE=OFF)
 *             pfRing        : Récupère l'état de la ligne RING (TRUE=ON / FALSE=OFF)
 *             pfRLSD        : Récupère l'état de la ligne RLSD (TRUE=ON / FALSE=OFF)
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état des lignes d'entrée d'un port série :
 *              CTS  : Clear To Send
 *              DSR  : Data Set Ready
 *              RING : Indicateur de sonnerie
 *              RLSD : Receive Line Signal Detect
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD SERComGetLineStatus( 
        IN              SERCOM_INSTANCE      * psPort,
        OPTIONAL OUT    BOOL              * pfCTS,
        OPTIONAL OUT    BOOL              * pfDSR,
        OPTIONAL OUT    BOOL              * pfRing,
        OPTIONAL OUT    BOOL              * pfRLSD )
{
    DWORD   dwErr = NO_ERROR;   // Code d'erreur courant
    DWORD   dwStatus;           // Etat des signaux

    if ( ! GetCommModemStatus( psPort->hPort, &dwStatus ) )
        dwErr = GetLastError();
    else
    {
        if ( pfCTS  != NULL )
            *pfCTS = ( ( dwStatus  & MS_CTS_ON  ) != 0 );

        if ( pfDSR  != NULL )
            *pfDSR = ( ( dwStatus  & MS_DSR_ON  ) != 0 );

        if ( pfRing != NULL )
            *pfRing = ( ( dwStatus & MS_RING_ON ) != 0 );

        if ( pfRLSD != NULL )
            *pfRLSD = ( ( dwStatus & MS_RLSD_ON ) != 0 );
    }

    return dwErr;
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERComGetInputBufferStatus( 
 *                      IN              SERCOM_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Nombre d'octets directement disponible dans le buffer
 *             de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état du buffer de reception.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERComGetInputBufferStatus( 
        IN              SERCOM_INSTANCE      * psPort )
{
    return psPort->dwBytes;
}


//
// PRIVATE FUNCTIONS
//




