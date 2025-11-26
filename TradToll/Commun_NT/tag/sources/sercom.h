/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SERCom
 * FICHIER    : SERCom.h
 * LANGAGE    : C
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORIQUE:
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef SERCOM_H
#define SERCOM_H

#include <protect.h>


#pragma pack( push, SERCOM_PACK, 1 )



//
// Définition de la structure d'instance (contenu non exporté).
//
typedef struct _SERCOM_INSTANCE SERCOM_INSTANCE;



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComOpen(
 *                      OUT SERCOM_INSTANCE    ** ppsPort,
 *                      IN const char *        cszListenAddress,
 *                      IN WORD                wListenPort,
 *                      IN const char *        cszPeerAddress,
 *                      IN WORD                wPeerPort )
 * PARAMETRES: ppsPort     : Récupère un "handle" de l'objet créé
 *             cszListenAddress  : Addresse d'écoute, chaine vide ou null
 *                           si écoute sur toutes les interfaces.
 *             wListenPort : Numéro du port UDP d'écoute
 *             cszPeerAddress  : Remote address
 *             wPeerPort : Numéro du port distant
 * RETOURNE  : NO_ERROR si l'objet a été crée, sinon, un code Win32
 * --------------------------------------------------------------------
 * ROLE      : Creation d'une instance de l'objet SERCom. Ce même objet
 *             pourra être détruit avec la fonction SERComClose().
 *             Pour obtenir un handle sur lequel peuvent être effectuée
 *             des opérations du type WaitForXXXX(), utiliser
 *             la fonction SERComGetWaitableHandle().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERComOpen(
        OUT SERCOM_INSTANCE ** ppsPort,
        IN DWORD                dwPort,
        IN char               * pcPortSettings );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED void WINAPI SERComClose(
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
PROTECTED void WINAPI SERComClose(
        IN SERCOM_INSTANCE  * psPort );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED HANDLE WINAPI SERComGetWaitableHandle(
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
PROTECTED HANDLE WINAPI SERComGetWaitableHandle(
        IN SERCOM_INSTANCE  * psPort );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComWrite(
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
PROTECTED DWORD WINAPI SERComWrite(
        IN SERCOM_INSTANCE * psPort,
        IN BYTE           * pbBuffer,
        IN DWORD            dwBufferBytes );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComRead(
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
PROTECTED DWORD WINAPI SERComRead(
        IN SERCOM_INSTANCE       * psPort,
        IN OPTIONAL BYTE      * pbBuffer,
        IN OUT OPTIONAL DWORD * pdwBufferBytes );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComEmptyBuffer(
 *                      IN SER_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SEROpen().
 * RETOURNE  : NO_ERROR si aucune erreur,
 *             Un code d'erreur Win32 sinon
 * --------------------------------------------------------------------
 * ROLE      : Vider le buffer du port comm
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERComEmptyBuffer(
        IN SERCOM_INSTANCE      * psPort );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComGetInputBufferStatus( 
 *                      IN              SERCOM_INSTANCE      * psPort )
 * PARAMETRES: psPort        : "handle" d'objet créé avec SERComOpen().
 * RETOURNE  : Nombre d'octets directement disponible dans le buffer
 *             de reception.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir l'état du buffer de reception.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI SERComGetInputBufferStatus( 
        IN              SERCOM_INSTANCE      * psPort );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI SERComGetLineStatus( 
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
PROTECTED DWORD WINAPI SERComGetLineStatus( 
        IN              SERCOM_INSTANCE      * psPort,
        OPTIONAL OUT    BOOL              * pfCTS,
        OPTIONAL OUT    BOOL              * pfDSR,
        OPTIONAL OUT    BOOL              * pfRing,
        OPTIONAL OUT    BOOL              * pfRLSD );


#pragma pack( pop, SERCOM_PACK )


#endif