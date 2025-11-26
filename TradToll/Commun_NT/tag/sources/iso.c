/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : ISO
 * FILE       : ISO.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Couche 2 ISO (ISO7816) - Liaison TGB.
 * --------------------------------------------------------------------
 * DESCRIPTION: Couche protocolaire de niveau 2 ISO pour la liaison
 *              entre hôte et communicateur en utilisant une liaison
 *              série en niveau 1 ISO.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <csr_excpt.h>
#include <csr_queue.h>

#include "csr_tag_ser.h"
#include "auto.h"

#include "csr_tag_iso.h"

#include "dll_l7.h"

#include <memclass.h>



//
// Indice des objets sur lesquels on opère un WaitForMultipleObjects et qui
// serviront à provoquer des événements sur l'automate protocolaire
//
#define ISO_END_INDEX            0
#define ISO_IRP_INDEX            1
#define ISO_EVT_COUNT            2


//
// Taille des buffer de travail : doivent pouvoir contenir un
// message complet (avec entête, corps, fin et LRC).
//
#define ISO_IN_BUFFER_BYTES      256
#define ISO_OUT_BUFFER_BYTES     256
#define ISO_HOOK_BUFFER_BYTES    256


//
// Hook utilisé pour l'espionnage de la liaison
//
#define ISO_CALL_HOOK(ctx,size,ptr)  if ( psInst->sParams.pfHook != NULL ) ISOTrack( psInst, ctx, size, ptr ); else



//
// Définition d'un packet pour la file des messages en sortie
// et en entrée
//
typedef struct _ISO_PACKET
{
    void              * pvContext;
    DWORD             * pdwErrCode;
    DWORD               dwBufferBytes;
    BYTE                tbBuffer[];
}
    ISO_PACKET;

    
//
// Définition des données associées à une instance de connexion
// avec le protocole
//
typedef struct _ISO_INSTANCE
{
    // Paramètres de l'instance
    ISO_PARAMS          sParams;

    HMODULE             hL7Dll;
    HMODULE             hL2Dll;

    CRITICAL_SECTION    sSerialize;

    // Pointeurs de fonctions couches basses
    LPFNOPEN            pfOpenComPort;
    LPFNCLOSE           pfCloseComPort;
    LPFNWRITEREAD       pfWriteReadData;
    LPFNGETVERSION      pfGetVersion;
    LPFNGETBEACONID     pfGetBeaconId;
    LPFNSETDOWNLOADFILE pfSetDownloadFile;

    // Handle du thread de gestion du protocole
    HANDLE              hThread;

    // Flag indiquant que le port a été ouvert
    BOOLEAN             fPortOpen;

    // File d'attente pour les messages sortant
    QUEUE_INSTANCE    * psWriteQueue;

    // File d'attente pour les messages entrant
    QUEUE_INSTANCE    * psReadQueue;

    // Indicateur que le prochain message est le
    // dernier d'une séquence.
    BOOL                fLastMessage;

    // Evénement signalé si la file psReadQueue a au moins un élément
    // et non signalé si la psReadQueue est vide
    HANDLE              hEndEvent;

    // Tableau des handles signalables servant à animer l'automate
    HANDLE              thEvents[ISO_EVT_COUNT];

    // Codes d'erreur courant du protocole
    DWORD               dwLastError;
    char                szLastError[256];

    // En phase d'émission, packet courant à émettre
    ISO_PACKET        * psCurrentPacket;

    // Taille occupée dans le buffer de réception
    DWORD               dwInBytes;

    // Buffer de réception
    BYTE                tbIn[ ISO_IN_BUFFER_BYTES ];

    // Context courant du hook
    DWORD               dwHookContext;

    // Nombre d'octets dans le context courant du hook
    DWORD               dwHookBytes;

    // Octets dans le context courant du hook
    BYTE                tbHook[ ISO_HOOK_BUFFER_BYTES ];


}
    ISO_INSTANCE;


//
// Autres fonctions privées
//
PRIVATE void WINAPI ISOMakeError( ISO_INSTANCE * psInst, DWORD dwError, char * pcText );
PRIVATE void WINAPI ISOHandleIRP( ISO_INSTANCE * psInst );
PRIVATE DWORD WINAPI ISORunThread( IN ISO_INSTANCE * psInst );
PRIVATE void WINAPI ISOTerminateCurrentPacket( ISO_INSTANCE * psInst, DWORD dwErr );
PRIVATE void WINAPI ISOTrack( ISO_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes );
PRIVATE DWORD WINAPI ISOVerifyOpenPort( ISO_INSTANCE * psInst );
PRIVATE DWORD WINAPI ISOTranslateError( int iL7Status );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISOOpen( 
 *                      OUT ISO_INSTANCE  ** ppsInst,
 *                      IN  ISO_PARAMS     * psParams )
 * PARAMETERS: ppsInst  : Retourne un handle de la liaison ouverte
 *             psParams : Pointe sur une structure contenant les paramètres de la liaison
 * RETURN    : NO_ERROR en cas de succés, un code d'erreur standard sinon.
 * --------------------------------------------------------------------
 * ROLE      : Création d'une nouvelle instance du protocole de communication ISO
 *             (utilisé pour la communication HOTE<->BADGE par liaison filaire)
 *             Dés sa création, l'instance bufferise les message reçus dans une
 *             file d'attente. Si cette file est pleine, les nouveaux messages
 *             arrivant sont systématiquement refusés.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI ISOOpen( 
        OUT ISO_INSTANCE  ** ppsInst,
        IN  ISO_PARAMS     * psParams )
{
    DWORD               dwErr       = NO_ERROR;
    ISO_INSTANCE      * psInst      = NULL;
    DWORD               dwId;
    int                 iL7Status;

    __try
    {
        //
        // Valeur par défaut retournée : NULL
        //
        (*ppsInst) = NULL;

        //
        // Allouer la place pour la structure d'instance et la remplir de 0
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(ISO_INSTANCE) );
        if ( psInst == NULL ) 
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        InitializeCriticalSection( &psInst->sSerialize );

        //
        // Reporter les valeurs des paramètres une fois qu'on s'est assuré que la
        // taille de la structure est compatible.
        //
        if ( psParams->dwStructBytes != sizeof(*psParams) )
        {
            dwErr = ERROR_INVALID_PARAMETER;
            __leave;
        }
        psInst->sParams = (*psParams);

        //
        // Charger la DLL couches basses
        //
        psInst->hL2Dll = LoadLibrary( psInst->sParams.szLowLevelDll );
        if ( psInst->hL2Dll == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }
        psInst->hL7Dll = LoadLibrary( psInst->sParams.szTransportDll );
        if ( psInst->hL7Dll == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Charger les pointeurs de fonctions
        //
        if      ( ( (FARPROC)psInst->pfOpenComPort = GetProcAddress( psInst->hL7Dll, FNOPEN ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( (FARPROC)psInst->pfCloseComPort = GetProcAddress( psInst->hL7Dll, FNCLOSE ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( (FARPROC)psInst->pfWriteReadData = GetProcAddress( psInst->hL7Dll, FNWRITEREAD ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( (FARPROC)psInst->pfGetVersion = GetProcAddress( psInst->hL7Dll, FNGETVERSION ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( (FARPROC)psInst->pfGetBeaconId = GetProcAddress( psInst->hL7Dll, FNGETBEACONID ) ) == NULL )
            dwErr = GetLastError();
        else if ( ( (FARPROC)psInst->pfSetDownloadFile = GetProcAddress( psInst->hL7Dll, FNSETDOWNLOADFILE ) ) == NULL )
            dwErr = GetLastError();
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Créer l'événement utilisé pour signaler la fin.
        //
        psInst->hEndEvent = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psInst->hEndEvent == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Initialisation des files d'attente
        //
        dwErr = QueueOpen( &psInst->psReadQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;
        dwErr = QueueOpen( &psInst->psWriteQueue, psInst->sParams.dwMaxPendingMsg );
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Initialisation de la liaison
        //
        iL7Status = psInst->pfSetDownloadFile( psInst->sParams.szDownloadFile );
        if ( iL7Status != DLL_L7_SUCCESS )
        {
            dwErr = ISOTranslateError( iL7Status );
            __leave;
        }

        dwErr = ISOVerifyOpenPort( psInst );
        if ( dwErr != NO_ERROR )
            __leave;

        //
        // Remplissage de la table des handle pour l'attente multiple du thread
        // d'animation de l'automate protocolaire.
        //
        psInst->thEvents[ISO_END_INDEX] = psInst->hEndEvent;
        psInst->thEvents[ISO_IRP_INDEX] = QueueGetWaitableHandle( psInst->psWriteQueue );

        //
        // Lancement du thread d'animation du protocole
        //
        // psInst->hThread = ExcptCreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ISORunThread, (void*)psInst, 0, &dwId, "TAG_ISO_PROTOCOL" );
        psInst->hThread = CreateThread( NULL, 0, (LPTHREAD_START_ROUTINE)ISORunThread, (void*)psInst, 0, &dwId );
        if ( psInst->hThread == NULL )
        {
            dwErr = GetLastError();
            __leave;
        }

        //
        // Pour améliorer la réactivité globale, on diminue le délai de préemption
        // à une milliseconde.
        //
        timeBeginPeriod( 1 );

        //
        // Mettre à jour les données à retourner
        //
        (*ppsInst) = psInst;
        psInst = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psInst != NULL )
            ISOClose( psInst, 0 );
    }
     
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI ISOClose( 
 *                      IN ISO_INSTANCE   * psInst,
 *                      IN DWORD            dwTimeout )
 * PARAMETERS: psInst    : Handle retourné par ISOOpen
 *             dwTimeout : Temps imparti en ms pour une fermeture propre.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance du protocole de communication créé avec
 *             ISOOpen(). Tous les message en attente (entrant ou sortant)
 *             sont annulés. Si la fonction n'est pas capable d'effectuer
 *             l'annulation dans le délai imparti, l'instance est fermée
 *             brutalement et certaines ressources peuvent ne pas avoir
 *             été libérées.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI ISOClose( 
        IN ISO_INSTANCE   * psInst,
        IN DWORD            dwTimeout )
{
    if ( psInst != NULL )
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( psInst->hThread != NULL )
        {
            timeEndPeriod( 1 );
            SetEvent( psInst->hEndEvent );
            WaitForSingleObject( psInst->hThread, dwTimeout );
            TerminateThread( psInst->hThread, NO_ERROR );
            CloseHandle( psInst->hThread );
        }
        if ( psInst->psReadQueue != NULL )
            QueueClose( psInst->psReadQueue );
        if ( psInst->psWriteQueue != NULL )
            QueueClose( psInst->psWriteQueue );
        if ( psInst->fPortOpen )
            psInst->pfCloseComPort();
        if ( psInst->hEndEvent != NULL )
            CloseHandle( psInst->hEndEvent );
        if ( psInst->hL7Dll != NULL )
            FreeLibrary( psInst->hL7Dll );
        if ( psInst->hL2Dll != NULL )
            FreeLibrary( psInst->hL2Dll );

        LeaveCriticalSection( &psInst->sSerialize );
        DeleteCriticalSection( &psInst->sSerialize );

        HeapFree( GetProcessHeap(), 0, psInst );
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISOSendMessage( 
 *                      IN              ISO_INSTANCE  * psInst, 
 *                      IN              BYTE          * pbMsg, 
 *                      IN              DWORD           dwMsgBytes, 
 *                      OPTIONAL IN     HANDLE          hEvent, 
 *                      OPTIONAL OUT    DWORD         * pdwErr )
 * PARAMETERS: psInst      : Handle retourné par ISOOpen
 *             pbMsg       : Pointe sun un buffer contenant les données à émettre.
 *             dwMsgBytes  : Taille des données à émettre
 *             hEvent      : Handle d'événement à signaler lorsque le message a été réellement envoyé.
 *                           Ce paramètre peut être NULL.
 *             pdwErr      : Pointe sur un DWORD qui reçoit 0xFFFFFFFF lors de la sortie de
 *                           la fonction et qui est mis à jours lorsque l'émission est terminée.
 *                           Si la valeur est alors NO_ERROR, l'émission a réussie, sinon, elle
 *                           a échoué.
 *                           Ce paramètre peut être NULL.
 * RETURN    : NO_ERROR si le message a été placé dans la file d'attente d'émission.
 *             Sinon, un code d'erreur standard.
 * --------------------------------------------------------------------
 * ROLE      : Place un message dans la file d'attente d'émission de l'instance.
 *             Lorsque le message est effectivement envoyé, l'événement (s'il est fourni) est
 *             signalé et le résultat (si pdwErr n'est pas NULL) est NO_ERROR.
 *             Si le message ne peut être envoyé, l'événement (s'il est fourni) est
 *             signalé et le résultat (si pdwErr n'est pas NULL) est une erreur standard.
 *             Lorsque la file d'attente d'émission est pleine, la fonction bloque jusqu'à ce
 *             que le message ait pu y être placé.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI ISOSendMessage( 
        IN              ISO_INSTANCE  * psInst, 
        IN              BYTE          * pbMsg, 
        IN              DWORD           dwMsgBytes, 
        OPTIONAL IN     HANDLE          hEvent, 
        OPTIONAL OUT    DWORD         * pdwErr )
{
    DWORD           dwErr = ERROR_NOT_ENOUGH_MEMORY;
    ISO_PACKET    * psPacket = NULL;

    __try
    {
        //
        // Mise à jour immédiate du code de complétion à 0xFFFFFFFF
        //
        if ( pdwErr != NULL )
            (*pdwErr) = 0xFFFFFFFF;

        //
        // Prepare un "packet" contenant le message à émettre
        //
        psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + dwMsgBytes + 5);
        if ( psPacket == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }
        psPacket->dwBufferBytes = dwMsgBytes;
        psPacket->pdwErrCode = pdwErr;
        psPacket->pvContext = hEvent;
        CopyMemory( psPacket->tbBuffer, pbMsg, dwMsgBytes );

        //
        // Placer le "packet" en file d'attente (attente infinie en cas de saturation
        // de la file).
        //
        dwErr = QueueWriteItem( psInst->psWriteQueue, psPacket, INFINITE );
        if ( dwErr != NO_ERROR )
            __leave;

        psPacket = NULL;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psPacket != NULL )
            HeapFree( GetProcessHeap(), 0, psPacket );
    }

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI ISOGetWaitableHandle(
 *                      IN ISO_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par ISOOpen
 * RETURN    : Un handle sur lequel un opération d'attente peut être effectuée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle permettant d'éffectuer des réceptions asynchrones.
 *             Ce handle est signalé lorsqu'au moins un message est dans la file
 *             de réception. Il est automatiquement réinitialisé lorsque le file
 *             est vide.
 *             Ce handle peut être utilisé avec les fonctions WaitForXXX de l'API Win32.
 *             Attention, ne pas essayer de réinitialiser ce handle.
 * --------------------------------------------------------------------
 */
EXPORT HANDLE WINAPI ISOGetWaitableHandle(
        IN ISO_INSTANCE * psInst )
{
    return QueueGetWaitableHandle( psInst->psReadQueue );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT SER_INSTANCE * WINAPI ISOGetCommInstance(
 *                      IN ISO_INSTANCE * psInst )
 * PARAMETERS: psInst : Handle retourné par ISOOpen
 * RETURN    : Un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 * ROLE      : Récupère un handle de l'instance SER utilisée.
 * --------------------------------------------------------------------
 */
EXPORT void * WINAPI ISOGetCommInstance(
        IN ISO_INSTANCE * psInst )
{
    return NULL;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI ISOReceiveMessage(
 *                      IN      ISO_INSTANCE  * psInst,
 *                      OUT     BYTE         ** ppbMsg,
 *                      OUT     DWORD         * pdwMsgBytes,
 *                      IN      DWORD           dwTimeout )
 * PARAMETERS: psInst      : Handle retourné par ISOOpen
 *             ppbMsg      : Retourne un pointeur sur le buffer où seront stockées
 *                           les données reçues.
 *             pdwMsgBytes : Retourne la taille des données reçues
 *             dwTimeout   : Si aucun message n'est présent dans la file d'attente, la fonction
 *                           attent un message pendant ce délai (en ms).
 * RETURN    : NO_ERROR si un message a été récu ou en cas de dépassement de délai.
 *             Sinon, un code d'erreur standard.
 * --------------------------------------------------------------------
 * ROLE      : Récupère le message suivant dans la file de réception. Si le délai imparti
 *             est dépassé, la fonction retourne sans erreur et avec (*pbMsg) = NULL
 *             et (*pdwMsgBytes) = 0.
 *             Si un message a été recu, le buffer doit être désalloué par un appel
 *             à ISOFreeMessage().
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI ISOReceiveMessage(
        IN      ISO_INSTANCE  * psInst,
        OUT     BYTE         ** ppbMsg,
        OUT     DWORD         * pdwMsgBytes,
        IN      DWORD           dwTimeout )
{
    DWORD           dwErr = NO_ERROR;
    ISO_PACKET    * psPacket;

    __try
    {
        //
        // Initialiser avec les valeurs par défaut (aucun message en attente).
        //
        (*ppbMsg) = NULL;
        (*pdwMsgBytes) = 0;

        //
        // Extraire un élément de la file d'attente
        //
        dwErr = QueueReadItem( psInst->psReadQueue, &psPacket, dwTimeout );
        if ( dwErr != NO_ERROR )
        {
            if ( dwErr == WAIT_TIMEOUT )
                dwErr = NO_ERROR;
            __leave;
        }

        //
        // Mise à jour de la valeur de retour
        //
        (*ppbMsg) = psPacket->tbBuffer;
        (*pdwMsgBytes) = psPacket->dwBufferBytes;
    }
    __finally
    {
    }

    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI ISOFreeMessage(
 *                      IN BYTE * pbMsg )
 * PARAMETERS: pbMsg : Message retourné par ISOReceiveMessage.
 * RETURN    : Rein
 * --------------------------------------------------------------------
 * ROLE      : Libère un buffer créé par un appel à ISOReceiveMessage.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI ISOFreeMessage(
        IN BYTE * pbMsg )
{
    ISO_PACKET * psPacket = NULL;

    //
    // Réajustement de la position pointeur
    //
    pbMsg -= (DWORD)(&psPacket->tbBuffer[0]);

    //
    // Libération du bloc
    //
    HeapFree( GetProcessHeap(), 0, pbMsg );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI ISOSetOption(
 *                      IN ISO_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD          dwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par ISOOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour ISO)
 *             dwValue     : Valeur de l'option (si entier) ou taille
 *                           pointée par pvValue
 *             pvValue     : Si non entier, pointe sur la valeur de
 *                           l'option, la taille du bloc pointé étant
 *                           donnée par dwValue.
 * RETURN    : NO_ERROR si l'option existe et a été mise à jour correctement.
 * --------------------------------------------------------------------
 * ROLE      : Change une option de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI WINAPI ISOSetOption(
        IN ISO_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD          dwValue,
        IN void         * pvValue )
{
    if ( dwOptionId == ISO_OPTION_F_LAST_MESSAGE )
    {
        if ( pvValue != NULL )
        {
            if      ( dwValue == 1 )
                psInst->fLastMessage = ( *(BYTE*)pvValue != 0 );
            else if ( dwValue == 2 )
                psInst->fLastMessage = ( *(WORD*)pvValue != 0 );
            else if ( dwValue == 4 )
                psInst->fLastMessage = ( *(DWORD*)pvValue != 0 );
            else
                return ERROR_INVALID_PARAMETER;
        }
        else
            psInst->fLastMessage = (dwValue != 0 );

        return NO_ERROR;
    }
    else
    {
        return ERROR_INVALID_DATA;
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WINAPI ISOGetOption(
 *                      IN ISO_INSTANCE * psInst,
 *                      IN DWORD          dwOptionId,
 *                      IN DWORD        * pdwValue,
 *                      IN void         * pvValue )
 * PARAMETERS: psInst      : Handle retourné par ISOOpen
 *             dwOptionId  : Identifiant d'option (aucun dispo pour ISO)
 *             pdwValue    : Si pvValue est NULL, récupère la valeur de l'option
 *                           lorsque celle-ci est entière.
 *                           Sinon, en entrée : taille du bloc pointé par pvValue,
 *                           en sortie : taille de l'option écrite dans pvValue.
 *             pvValue     : Si non NULL, pointe sur le buffer récupérant l'option.
 * RETURN    : NO_ERROR si l'option existe et a été lue correctement.
 * --------------------------------------------------------------------
 * ROLE      : Lit une option de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI WINAPI ISOGetOption(
        IN ISO_INSTANCE * psInst,
        IN DWORD          dwOptionId,
        IN DWORD        * pdwValue,
        IN void         * pvValue )
{
    int   iL7Status;
    DWORD dwErr = NO_ERROR;
    char  szVDllL7[DLL_L7_MAX_SIZE_VERSION];
    char  szVDllL2[DLL_L7_MAX_SIZE_VERSION];
    char  szAnyStr[DLL_L7_MAX_SIZE_VERSION];


    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        dwErr = ISOVerifyOpenPort( psInst );
        if ( dwErr != NO_ERROR )
            __leave;

        if ( ! psInst->fPortOpen )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

        if ( dwOptionId == ISO_OPTION_F_LAST_MESSAGE )
        {
            if ( pvValue != NULL )
            {
                if      ( *pdwValue == 1 )
                {
                    *(BYTE*)pvValue = ( psInst->fLastMessage ? 1 : 0 );
                }
                else if ( *pdwValue == 2 )
                {
                    *(WORD*)pvValue = ( psInst->fLastMessage ? 1 : 0 );
                }
                else if ( *pdwValue > 4 )
                {
                    *(DWORD*)pvValue  = ( psInst->fLastMessage ? 1 : 0 );
                    *pdwValue = 4;
                }
                else
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
            }
            else
                *pdwValue = ( psInst->fLastMessage ? 1 : 0 );
        }

        else if ( dwOptionId == ISO_OPTION_SZ_VERSION )
        {
            if ( pvValue == NULL )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            if ( *pdwValue == 0 )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                __leave;
            }

            iL7Status = psInst->pfGetVersion( szVDllL7, szVDllL2, szAnyStr );
            dwErr = ISOTranslateError( iL7Status );
            if ( dwErr != NO_ERROR )
            {
                if ( iL7Status == DLL_L7_EQUIPMENT_ERROR )
                {
                    psInst->pfCloseComPort();
                    psInst->fPortOpen = FALSE;
                }
                __leave;
            }

            _snprintf( (char*)pvValue, *pdwValue, "L7(%s) - L2(%s) - EQT(%s)",
                szVDllL7, szVDllL2, szAnyStr );
            ((char*)pvValue)[(*pdwValue)-1] = 0;
            *pdwValue = strlen( (char*)pvValue ) + 1;
        }

        else if ( dwOptionId == ISO_OPTION_DW_BEACON_ID )
        {
            if ( pvValue != NULL )
            {
                if ( *pdwValue < 4 )
                {
                    dwErr = ERROR_INVALID_PARAMETER;
                    __leave;
                }
            }

            iL7Status = psInst->pfGetBeaconId( szAnyStr );
            dwErr = ISOTranslateError( iL7Status );
            if ( dwErr != NO_ERROR )
            {
                if ( iL7Status == DLL_L7_EQUIPMENT_ERROR )
                {
                    psInst->pfCloseComPort();
                    psInst->fPortOpen = FALSE;
                }
                __leave;
            }

            szAnyStr[6] = 0;
            if ( pvValue == NULL )
                *pdwValue = atol( szAnyStr );
            else
            {
                *(DWORD*)pvValue = atol( szAnyStr );
                *pdwValue = 4;
            }
        }

        else
        {
            dwErr = ERROR_INVALID_DATA;
            __leave;
        }

        dwErr = NO_ERROR;
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );

        
    }

	return dwErr;
}




PRIVATE void WINAPI ISOMakeError( ISO_INSTANCE * psInst, DWORD dwError, char * pcText )
{
    //
    // Annuler le packet en cours si packet présent.
    //
    ISOTerminateCurrentPacket( psInst, dwError );

    psInst->dwLastError = dwError;
    strcpy( psInst->szLastError, pcText );
}




PRIVATE void WINAPI ISOHandleIRP( ISO_INSTANCE * psInst )
{
    DWORD        dwErr;
    ISO_PACKET * psPacket;
    int          iL7Status;

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( psInst->psCurrentPacket == NULL )
        {
            //
            // Si le message n'a pas encore été construit, on l'extrait de la file d'attente
            //
            dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
            if ( dwErr != NO_ERROR )
            {
                ISOMakeError( psInst, dwErr, "ISOHandleIRP : QueueReadItem" );
                __leave;
            }
        }

        //
        // Dans le mode où les buffers sont vidés automatiquement,
        // on vide la file de réception chaque fois qu'on émet un
        // nouveau message (mode question/réponse).
        //
        if ( psInst->sParams.dwAutoClearBuffers != 0 )
        {
            while ( ! QueueIsEmpty( psInst->psReadQueue ) )
            {
                dwErr = QueueReadItem( psInst->psReadQueue, &psPacket, 0 );
                if ( dwErr == NO_ERROR )
                    HeapFree( GetProcessHeap(), 0, psPacket );
            }
        }

        dwErr = ISOVerifyOpenPort( psInst );
        if ( dwErr != NO_ERROR )
        {
            ISOMakeError( psInst, dwErr, "ISOHandleIRP : QueueReadItem" );
            __leave;
        }
        if ( ! psInst->fPortOpen )
        {
            ISOMakeError( psInst, ERROR_DEVICE_NOT_CONNECTED, "ISOHandleIRP : ISOVerifyOpenPort" );
            __leave;
        }

        //
        // Envoyer le message sur le port
        //
        psInst->dwInBytes = sizeof( psInst->tbIn );
        iL7Status = psInst->pfWriteReadData( psInst->psCurrentPacket->tbBuffer,
                                             psInst->psCurrentPacket->dwBufferBytes,
                                             psInst->tbIn,
                                             &psInst->dwInBytes,
                                             (char)( psInst->fLastMessage ? 1 : 0 ) );
        psInst->fLastMessage = FALSE;

        dwErr = ISOTranslateError( iL7Status );
        if ( ( dwErr == NO_ERROR ) && ( psInst->dwInBytes != 0 ) )
        {
            //
            // Le message a été émis et une réponse a été reçue
            //
            ISO_CALL_HOOK( ISO_HOOK_OUTPUT | ISO_HOOK_DATA | ISO_HOOK_FLUSH,
                psInst->psCurrentPacket->dwBufferBytes, 
                psInst->psCurrentPacket->tbBuffer );
            ISOTerminateCurrentPacket( psInst, NO_ERROR );

            ISO_CALL_HOOK( ISO_HOOK_INPUT | ISO_HOOK_DATA | ISO_HOOK_FLUSH,
                psInst->dwInBytes, 
                psInst->tbIn );

            //
            // Préparer le packet de lecture
            //
            psInst->dwInBytes = sizeof( psInst->tbIn );
            psPacket = HeapAlloc( GetProcessHeap(), 0, sizeof(*psPacket) + psInst->dwInBytes );
            if ( psPacket != NULL )
            {
                //
                // Mettre à zéro l'entête du packet
                //
                ZeroMemory( psPacket, sizeof( *psPacket ) );

                psPacket->dwBufferBytes = psInst->dwInBytes;
                CopyMemory( psPacket->tbBuffer, psInst->tbIn, psPacket->dwBufferBytes );

                //
                // Repartir à zéro avec le buffer et l'état de reception du message
                //
                psInst->dwInBytes = 0;

                //
                // Ajouter l'item dans la file d'attente
                //
                dwErr = QueueWriteItem( psInst->psReadQueue, psPacket, INFINITE );
                if ( dwErr != NO_ERROR )
                {
                    //
                    // Echec dans la file d'attente : On libère le buffer alloué
                    // La reception a échouée, on provoque une erreur
                    //
                    HeapFree( GetProcessHeap(), 0, psPacket );
                    ISOMakeError( psInst, dwErr, "ISOHandleIRP : QueueWriteItem" );
                }
            }
        }
        else if ( (            dwErr == ERROR_READ_FAULT                ) ||
                  ( ( dwErr == NO_ERROR ) && ( psInst->dwInBytes == 0 ) ) )
        {
            //
            // Le message a été émis mais aucune réponse n'a été renvoyée
            //
            ISO_CALL_HOOK( ISO_HOOK_OUTPUT | ISO_HOOK_DATA | ISO_HOOK_FLUSH,
                psInst->psCurrentPacket->dwBufferBytes, 
                psInst->psCurrentPacket->tbBuffer );
            ISOTerminateCurrentPacket( psInst, NO_ERROR );
        }
        else
        {
            if ( iL7Status == DLL_L7_EQUIPMENT_ERROR )
            {
                psInst->pfCloseComPort();
                psInst->fPortOpen = FALSE;
            }

            //
            // L'envoi a échoué, on provoque une erreur, on annule l'IRP en cours
            //
            ISOMakeError( psInst, dwErr, "ISOHandleIRP : WriteReadData" );
        }
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }
}







PRIVATE DWORD WINAPI ISORunThread( 
        IN ISO_INSTANCE * psInst )
{
    DWORD       dwErr;
    DWORD       dwRes;
    DWORD       dwIndex;

    SetThreadPriorityBoost( GetCurrentThread(), TRUE );
    SetThreadPriority( GetCurrentThread(), psInst->sParams.dwPriority );

    //
    // Animer l'automate
    //
    while ( TRUE )
    {
        dwRes =  WaitForMultipleObjects( 2, psInst->thEvents, FALSE, INFINITE );
        dwIndex = dwRes - WAIT_OBJECT_0;

        if ( dwIndex == ISO_END_INDEX )
        {
            //
            // Signal de fin reçu, on sort de la boucle
            //
            break;
        }

        else if ( dwIndex == ISO_IRP_INDEX )
        {
            ISOHandleIRP( psInst );
        }

        else
        {
            //
            // Ignorer
            //
        }
    }

    //
    // Avant la fin du thread, vider la file d'attente en emission
    //

    if ( psInst->psCurrentPacket == NULL )
    {
        //
        // Si aucun packet en cours, récuperer le suivant si présent
        //
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
    }

    //
    // Tant qu'il reste des packets pour l'envoi de messages
    //
    while ( psInst->psCurrentPacket != NULL && dwErr == NO_ERROR )
    {
        //
        // Terminer le packet avec signal d'annulation
        //
        ISOTerminateCurrentPacket( psInst, ERROR_OPERATION_ABORTED );

        //
        // Passer à l'élément suivant de la file
        //
        dwErr = QueueReadItem( psInst->psWriteQueue, &psInst->psCurrentPacket, 0 );
    }

    //
    // Vider la file d'attente en reception :
    // Récupérer le premier élément de la file de réception
    //
    dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentPacket, 0 );

    //
    // Tant qu'il y des packets
    //
    while ( psInst->psCurrentPacket != NULL && dwErr == NO_ERROR )
    {
        //
        // Libérer le packet
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentPacket );

        //
        // Récupérer le suivant
        //
        dwErr = QueueReadItem( psInst->psReadQueue, &psInst->psCurrentPacket, 0 );
    }

    ExitThread( NO_ERROR );

    return NO_ERROR;
}




PRIVATE void WINAPI ISOTerminateCurrentPacket( ISO_INSTANCE * psInst, DWORD dwErr )
{
    //
    // Si un packet est en cours d'émission
    //
    if ( psInst->psCurrentPacket != NULL )
    {
        //
        // Signaler le code de résultat associé
        //
        if ( psInst->psCurrentPacket->pdwErrCode != NULL )
            (*(psInst->psCurrentPacket->pdwErrCode)) = dwErr;

        //
        // Signaler l'événement associé
        //
        if ( psInst->psCurrentPacket->pvContext != NULL )
            SetEvent( psInst->psCurrentPacket->pvContext );

        //
        // Libérer le bloc
        //
        HeapFree( GetProcessHeap(), 0, psInst->psCurrentPacket );

        //
        // Plus de packet en cours
        //
        psInst->psCurrentPacket = NULL;
    }
}



PRIVATE void WINAPI ISOTrack( ISO_INSTANCE * psInst, DWORD dwContext, DWORD dwBytes, BYTE * pbBytes )
{
    BOOL    fFlush;
    DWORD   dwRemain;

    fFlush = ( ( dwContext & ISO_HOOK_FLUSH ) != 0 );
    dwContext = ( dwContext & ( ~ ISO_HOOK_FLUSH ) );

    if ( dwContext != psInst->dwHookContext )
    {
        if ( ( psInst->dwHookBytes    > 0    ) && 
             ( psInst->dwHookContext != 0    ) )
        {
            psInst->sParams.pfHook(
                psInst->sParams.pvHookContext,
                psInst->dwHookContext,
                psInst->tbHook,
                psInst->dwHookBytes );
        }

        psInst->dwHookBytes = 0;
        psInst->dwHookContext = dwContext;
    }

    if ( psInst->dwHookBytes < sizeof(psInst->tbHook) )
    {
        dwRemain = sizeof(psInst->tbHook) - psInst->dwHookBytes;
        if ( dwBytes > dwRemain )
            dwBytes = dwRemain;
        memcpy( &psInst->tbHook[psInst->dwHookBytes], pbBytes, dwBytes );
        psInst->dwHookBytes += dwBytes;
    }

    if ( fFlush )
    {
        if ( ( psInst->dwHookBytes    > 0    ) && 
             ( psInst->dwHookContext != 0    ) )
        {
            psInst->sParams.pfHook(
                psInst->sParams.pvHookContext,
                psInst->dwHookContext,
                psInst->tbHook,
                psInst->dwHookBytes );
        }

        psInst->dwHookBytes = 0;
        psInst->dwHookContext = dwContext;
    }
}



PRIVATE DWORD WINAPI ISOVerifyOpenPort( ISO_INSTANCE * psInst )
{
    DWORD dwErr = NO_ERROR;
    int   iL7Status;
    char  szCom[32];

    __try
    {
        EnterCriticalSection( &psInst->sSerialize );

        if ( ! psInst->fPortOpen )
        {
            sprintf( szCom, "COM%u", psInst->sParams.dwPort );

            iL7Status = psInst->pfOpenComPort( szCom, psInst->sParams.dwDebit );
            if ( ( iL7Status != DLL_L7_SUCCESS ) && ( iL7Status != DLL_L7_ERROR_IS_OPENED ) )
            {
                psInst->fPortOpen = FALSE;

                if ( ( iL7Status != DLL_L7_PORTCOM_NOT_OPENED    ) &&
                     ( iL7Status != DLL_L7_ERROR_DLL_L2          ) &&
                     ( iL7Status != DLL_L7_ERROR_DOWNLOAD_FAILED ) &&
                     ( iL7Status != DLL_L7_EQUIPMENT_ERROR       ) )
                {
                    dwErr = ISOTranslateError( iL7Status );
                    __leave;
                }
            }
            else
                psInst->fPortOpen = TRUE;
        }
    }
    __finally
    {
        LeaveCriticalSection( &psInst->sSerialize );
    }

    return dwErr;
}



PRIVATE DWORD WINAPI ISOTranslateError( int iL7Status )
{
    switch ( iL7Status )
    {
        
    case DLL_L7_SUCCESS                          : return NO_ERROR;
    case DLL_L7_RECEIVE_TIMEOUT                  : return ERROR_READ_FAULT;
    case DLL_L7_SEND_FAILED                      : return ERROR_WRITE_FAULT;
    case DLL_L7_SUCCESS_WITH_DOUBLE_RUN_POSSIBLE : return NO_ERROR;
    case DLL_L7_ILLEGAL_RATE                     : return ERROR_INVALID_PARAMETER;
    case DLL_L7_PORTCOM_NOT_OPENED               : return ERROR_OPEN_FAILED;
    case DLL_L7_ERROR_SYSTEM                     : return ERROR_GEN_FAILURE;
    case DLL_L7_ERROR_DLL_L2                     : return ERROR_BAD_COMMAND;
    case DLL_L7_ERROR_MAX_SIZE                   : return ERROR_BAD_LENGTH;
    case DLL_L7_ERROR_LAST_FRAME                 : return ERROR_INVALID_DATA;
    case DLL_L7_ERROR_DOWNLOAD_FAILED            : return ERROR_FILE_NOT_FOUND;
    case DLL_L7_ERROR_DOWNLOAD_FILE              : return ERROR_BAD_FORMAT;
    case DLL_L7_ERROR_IS_OPENED                  : return ERROR_ALREADY_INITIALIZED;
    case DLL_L7_ERROR_NOT_OPENED                 : return ERROR_BAD_ENVIRONMENT;
    case DLL_L7_EQUIPMENT_ERROR                  : return ERROR_BAD_UNIT;
    case DLL_L7_ILLEGAL_FRAME                    : return ERROR_CRC;

    }

    return 0xE000FFFF;

}