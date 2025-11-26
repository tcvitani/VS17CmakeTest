/* --------------------------------------------------------------------
 * (C) 2000 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : QFSL7
 * FILE       : QFSL7.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion du mode transparent
 * --------------------------------------------------------------------
 * DESCRIPTION: Implémentation de la couche 7 de premier niveau
 *              pour l'interface QFREE beacon MD5826
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>

#include "csr_tag_qfsl7.h"

#include <memclass.h>




//
// Vérification des pointeurs et de leurs valeurs
//
#define CHK_PTR(x)      if ( (x) == NULL ) __leave; else if ( ! QFSL7_IS_VALID(*(x)) ) __leave;
#define CHK_VAL(x)      if ( ! QFSL7_IS_VALID(x) ) __leave;




//
// Manipulation des buffers
//
#define GET_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { (v) = *(BYTE*)&pbBuf[dwPos] ; dwPos += sizeof(BYTE); }
#define GET_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( (v), pbBuf+dwPos, (s) ) ; dwPos += (s); }
#define ADD_BYTE(v)     if ( ( dwPos + sizeof(BYTE) )  > dwBufLen ) __leave; else { *(BYTE*)&pbBuf[dwPos]  = (BYTE)(v); dwPos += sizeof(BYTE); }
#define ADD_BYTES(s,v)  if ( ( dwPos + (s) ) > dwBufLen ) __leave; else { memcpy( pbBuf+dwPos, (v), (s) ); dwPos += (s); }




typedef struct _QFSL7_INSTANCE
{
    DWORD       dwDummy;
}
    QFSL7_INSTANCE;




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT QFSL7_INSTANCE * WINAPI QFSL7Open()
 * PARAMETRES: Aucun
 * RETOURNE  : Un handle de la nouvelle instance créée ou NULL en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Mise en place d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un MD5826.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT QFSL7_INSTANCE * WINAPI QFSL7Open()
{
    QFSL7_INSTANCE * psInst;

    psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );

    return psInst;
}




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI QFSL7Close(
 *                      IN OUT  QFSL7_INSTANCE         * psInst )
 * PARAMETRES: psInst : Handle d'instance créée avec QFSL7Open().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un MD5826.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI QFSL7Close(
        IN OUT  QFSL7_INSTANCE         * psInst )
{
    HeapFree( GetProcessHeap(), 0, psInst );
}




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
        IN      QFSL7_API         *  psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;
    BYTE            bLen;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // Verifiy the parameters are coherent
        //
        if ( psDef->dwParamBytes > sizeof(psDef->tbParams) )
            __leave;

        bLen = (BYTE)psDef->dwParamBytes + 2;

        ADD_BYTE ( psDef->bMessageType );
        ADD_BYTE ( bLen                );
        ADD_BYTES( psDef->dwParamBytes, psDef->tbParams );

        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwPos;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




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
        IN      QFSL7_INSTANCE   * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     QFSL7_API       * psDef )
{
    BOOL            fReturn         = FALSE;
    DWORD           dwBufLen;
    DWORD           dwPos;
    BYTE            bLen;

    __try
    {
        //
        // S'assurer de la validité des pointeurs et des valeurs
        //
        CHK_PTR( pdwBufBytePos )
        CHK_PTR( pdwBufByteLen )

        dwBufLen = (*pdwBufByteLen);
        dwPos = (*pdwBufBytePos);

        //
        // Get the header
        //
        GET_BYTE( psDef->bMessageType); // Response type
        GET_BYTE( bLen);                // Length

        //
        // Make sure the size is correct
        //
        if ( bLen < 2 )
            __leave;

        psDef->dwParamBytes = (DWORD)bLen - 2;
        GET_BYTES( psDef->dwParamBytes, psDef->tbParams );
        
        (*pdwBufBytePos) += dwPos;
        (*pdwBufByteLen) -= dwBufLen;

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}
