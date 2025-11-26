/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SER
 * FILE       : SER.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : Communication série semi assynchrone
 * --------------------------------------------------------------------
 * DESCRIPTION: La communication est semi assynchrone dans la mesure où
 *              l'écriture est synchrone, mais la lecture est assynchrone.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include <reg.h>

#include "ser.h"
#include "sercom.h"
#include "serudp.h"
#include "csr_tag_ser.h"

#include <memclass.h>




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SEROpen(
 *                      OUT SER_INSTANCE    ** ppsPort,
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
EXPORT DWORD WINAPI SEROpen(
        OUT SER_INSTANCE    ** ppsPort,
        IN DWORD                dwPort,
        IN char               * pcPortSettings )
{
    char szRegPath[256];
    char szListenIpAddr[256];
    char szPeerIpAddr[256];
    WORD wListenUdpPort;
    WORD wPeerUdpPort;
    DWORD dwLen;
    DWORD dwValue;
    DWORD dwEmulationEnabled = 0;
    DWORD dwErr = ERROR_INVALID_PARAMETER;

    sprintf( szRegPath, "SOFTWARE\\CSRoute\\SERUdp\\COM%u", dwPort );
    if ( REG_Lire_Entier( HKEY_LOCAL_MACHINE, szRegPath, "EmulationEnabled", &dwEmulationEnabled ) != ERROR_SUCCESS )
        dwEmulationEnabled = 0;


    __try
    {
        if ( dwEmulationEnabled != 0 )
        {
            dwLen = sizeof(szListenIpAddr);
            if ( REG_Lire_Chaine( HKEY_LOCAL_MACHINE, szRegPath, "ListenIpAddr", szListenIpAddr, &dwLen ) != ERROR_SUCCESS )
                szListenIpAddr[0] = 0;

            if ( REG_Lire_Entier( HKEY_LOCAL_MACHINE, szRegPath, "ListenUdpPort", &dwValue ) != ERROR_SUCCESS )
                __leave;
            else
                wListenUdpPort = (WORD)dwValue;
             
            dwLen = sizeof(szPeerIpAddr);
            if ( REG_Lire_Chaine( HKEY_LOCAL_MACHINE, szRegPath, "PeerIpAddr", szPeerIpAddr, &dwLen ) != ERROR_SUCCESS )
                __leave;

            if ( REG_Lire_Entier( HKEY_LOCAL_MACHINE, szRegPath, "PeerUdpPort", &dwValue ) != ERROR_SUCCESS )
                wPeerUdpPort = 232;
            else
                wPeerUdpPort = (WORD)dwValue;

            dwErr = SERUdpOpen( (SERUDP_INSTANCE**)ppsPort, szListenIpAddr, wListenUdpPort, szPeerIpAddr, wPeerUdpPort );
        }
        else
            dwErr = SERComOpen( (SERCOM_INSTANCE**)ppsPort, dwPort, pcPortSettings );
    }
    __finally
    {
    }

    return dwErr;
}




    

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI SERClose(
 *                      IN SER_INSTANCE  * psPort )
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
EXPORT void WINAPI SERClose(
        IN SER_INSTANCE  * psPort )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            SERComClose( (SERCOM_INSTANCE*)psPort );
        else if ( psPort->eType == SER_UDP )
            SERUdpClose( (SERUDP_INSTANCE*)psPort );
    }
}







/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT HANDLE WINAPI SERGetWaitableHandle(
 *                      IN SER_INSTANCE  * psPort )
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

EXPORT HANDLE WINAPI SERGetWaitableHandle(
        IN SER_INSTANCE  * psPort )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComGetWaitableHandle( (SERCOM_INSTANCE*)psPort );
        else if ( psPort->eType == SER_UDP )
            return SERUdpGetWaitableHandle( (SERUDP_INSTANCE*)psPort );
    }
    return NULL;
}





/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERWrite(
 *                      IN SER_INSTANCE  * psPort,
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
EXPORT DWORD WINAPI SERWrite(
        IN SER_INSTANCE * psPort,
        IN BYTE           * pbBuffer,
        IN DWORD            dwBufferBytes )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComWrite( (SERCOM_INSTANCE*)psPort, pbBuffer, dwBufferBytes );
        else if ( psPort->eType == SER_UDP )
            return SERUdpWrite( (SERUDP_INSTANCE*)psPort, pbBuffer, dwBufferBytes );
    }
    return ERROR_INVALID_PARAMETER;
}


            

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERRead(
 *                      IN SER_INSTANCE      * psPort,
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
EXPORT DWORD WINAPI SERRead(
        IN SER_INSTANCE       * psPort,
        IN OPTIONAL BYTE      * pbBuffer,
        IN OUT OPTIONAL DWORD * pdwBufferBytes )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComRead( (SERCOM_INSTANCE*)psPort, pbBuffer, pdwBufferBytes );
        else if ( psPort->eType == SER_UDP )
            return SERUdpRead( (SERUDP_INSTANCE*)psPort, pbBuffer, pdwBufferBytes );
    }
    return ERROR_INVALID_PARAMETER;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SEREmptyBuffer(
 *                      IN SER_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Vider le buffer du port comm
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI SEREmptyBuffer(
        IN SER_INSTANCE      * psPort )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComEmptyBuffer( (SERCOM_INSTANCE*)psPort );
        else if ( psPort->eType == SER_UDP )
            return SERUdpEmptyBuffer( (SERUDP_INSTANCE*)psPort );
    }
    return ERROR_INVALID_PARAMETER;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERGetLineStatus( 
 *                      IN              SER_INSTANCE      * psPort,
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
EXPORT DWORD WINAPI SERGetLineStatus( 
        IN              SER_INSTANCE      * psPort,
        OPTIONAL OUT    BOOL              * pfCTS,
        OPTIONAL OUT    BOOL              * pfDSR,
        OPTIONAL OUT    BOOL              * pfRing,
        OPTIONAL OUT    BOOL              * pfRLSD )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComGetLineStatus( (SERCOM_INSTANCE*)psPort, pfCTS, pfDSR, pfRing, pfRLSD );
        else if ( psPort->eType == SER_UDP )
            return SERUdpGetLineStatus( (SERUDP_INSTANCE*)psPort, pfCTS, pfDSR, pfRing, pfRLSD );
    }
    return ERROR_INVALID_PARAMETER;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI SERGetInputBufferStatus( 
 *                      IN              SER_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : Nombre d'octets directement disponible dans le buffer
 *             de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état du buffer de reception.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT DWORD WINAPI SERGetInputBufferStatus( 
        IN              SER_INSTANCE      * psPort )
{
    if ( psPort != NULL )
    {
        if ( psPort->eType == SER_COM )
            return SERComGetInputBufferStatus( (SERCOM_INSTANCE*)psPort );
        else if ( psPort->eType == SER_UDP )
            return SERUdpGetInputBufferStatus( (SERUDP_INSTANCE*)psPort );
    }
    return 0;
}

