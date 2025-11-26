/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFSL7
 * FILE       : 
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion du mode transparent
 * --------------------------------------------------------------------
 * DESCRIPTION: Définitions exportées
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_QFSL7_H
#define CSR_TAG_QFSL7_H

#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif


#pragma pack( push, TAG_QFSL7_PACK, 1 )


//
// Test de validité d'indexes
//
#define QFSL7_IS_VALID(x)         ((x)<0x80000000)
#define QFSL7_INVALID             0xFFFFFFFF


//
// Possible QFSL7 commands
//
typedef enum
{
    
	QFSL7_ACTION_QFSL7_INIT				= 'I',
    QFSL7_ACTION_QFSL7_FRAME			= 'F',
    QFSL7_ACTION_QFSL7_STOP				= 'S',
    QFSL7_ACTION_QFSL7_TIMEOUT			= 'T',
    QFSL7_ACTION_QFSL7_ERROR			= 'E',
    QFSL7_ACTION_QFSL7_QUERY            = 'Q',
    QFSL7_ACTION_QFSL7_CONFIG			= 'C',
    QFSL7_ACTION_QFSL7_L2FRAME			= 'L'

}
    QFSL7_ACTIONS;



//
// Constantes diverses
//
#define QFSL7_MAX_DATAGRAM        256





typedef struct 
{
    BYTE                bMessageType;
    
    DWORD               dwParamBytes;

    BYTE                tbParams[QFSL7_MAX_DATAGRAM-2];
}
    QFSL7_API;


		




typedef struct _QFSL7_INSTANCE QFSL7_INSTANCE;




// ------------------ PROTOTYPES EXPORTES ---------------------


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT QFSL7_INSTANCE * WINAPI QFSL7Open()
 * PARAMETRES: Aucun
 * RETOURNE  : Un handle de la nouvelle instance créée ou NULL en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Mise en place d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT QFSL7_INSTANCE * WINAPI QFSL7Open();




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI QFSL7Close(
 *                      IN OUT  QFSL7_INSTANCE         * psInst )
 * PARAMETRES: psInst : Handle d'instance créée avec QFSL7Open().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI QFSL7Close(
        IN OUT  QFSL7_INSTANCE   * psInst );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT BOOL WINAPI QFSL7BuildMessage( 
 *                      IN      QFSL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      QFSL7_API         * psDef )
 * PARAMETRES: psInst        : Handle d'instance créée avec QFSL7Open().
 *             pbBuf         : Pointe sur le buffer destiné à recevoir le message généré
 *             pdwBufBytePos : En entrée, position du premiere octet du buffer
 *                      à utiliser. En sortie, position dans le buffer de l'octet
 *                      immédiatement aprés le message généré.
 *             pdwBufByteLen : En entrée, nombre d'octet disponibles dans le
 *                      buffer à partir de la position donnée.
 *             psDef         : Définition du message à générer.
 * RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à la génération.
 * --------------------------------------------------------------------
 * ROLE      : Construit un message à destination d'un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI QFSL7BuildMessage( 
        IN      QFSL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      QFSL7_API         * psDef );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT BOOL WINAPI QFSL7AnalyseMessage( 
 *                      IN      QFSL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      OUT     QFSL7_API         * psDef )
 * PARAMETRES: psInst        : Handle d'instance créée avec QFSL7Open().
 *             pbBuf         : Pointe sur le buffer contenant le message reçu
 *             pdwBufBytePos : En entrée, position du premiere octet du buffer
 *                      à utiliser. En sortie, position dans le buffer de l'octet
 *                      immédiatement aprés le message lu.
 *             pdwBufByteLen : En entrée, nombre d'octets disponibles dans le
 *                      buffer à partir de la position donnée.
 *             psDef         : Définition du message analysé.
 * RETOURNE  : TRUE en cas de succées, FALSE en cas d'erreur à l'analyse.
 * --------------------------------------------------------------------
 * ROLE      : Analyse un message à reçu d'un communicateur.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT BOOL WINAPI QFSL7AnalyseMessage( 
        IN      QFSL7_INSTANCE    * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     QFSL7_API         * psDef );


#pragma pack( pop, TAG_QFSL7_PACK )


#endif
