/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : SER
 * FICHIER    : CSR_TAG_SER.h
 * LANGAGE    : C
 * --------------------------------------------------------------------
 * MOT-CLE    : 
 * --------------------------------------------------------------------
 * RESUME     : Communication série semi assynchrone
 * --------------------------------------------------------------------
 * DESCRIPTION: Définitions publiques.
 * --------------------------------------------------------------------
 * HISTORIQUE:
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_SER_H
#define CSR_TAG_SER_H

#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif



#pragma pack( push, TAG_SER_PACK, 1 )



//
// Définition de la structure d'instance (contenu non exporté).
//
typedef struct _SER_INSTANCE SER_INSTANCE;






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
        IN char               * pcPortSettings );




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
        IN SER_INSTANCE  * psPort );




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
        IN SER_INSTANCE  * psPort );




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
        IN SER_INSTANCE  * psPort,
        IN BYTE           * pbBuffer,
        IN DWORD            dwBytes );




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
        IN SER_INSTANCE  * psPort,
        IN BYTE           * pbBuffer,
        OUT DWORD         * pdwBytes );




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
        IN SER_INSTANCE      * psPort );




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
        OPTIONAL OUT    BOOL              * pfRLSD );



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
        IN              SER_INSTANCE      * psPort );


#pragma pack( pop, TAG_SER_PACK )


#endif