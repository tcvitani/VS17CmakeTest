/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_block.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion des blocs de commande
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
#include <acom_dmem.h>

#include <acom.h>
#include <acom_priv.h>
#include <acom_dbg.h>
#define LOC_DEF
#include <acom_block.h>
#undef LOC_DEF

#include <memclass.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_BLOCK * AComBlockOpenWithHeader ( 
 *                      DWORD dwBlockType,
 *                      ACOM_CONNECTION * psCnx,
 *                      ACOM_CNX_HANDLE hCnxHandle, 
 *                      DWORD64 dwCnxUsrKey, 
 *                      DWORD64 dwMsgUsrKey, 
 *                      DWORD bDisable, 
 *                      DWORD dwHeaderSize,
 *                      void  * pvHeader,
 *                      DWORD dwDataSize, 
 *                      void  * pvData )
 * PARAMETERS: DWORD dwBlockType           : Type du bloc de commande (ACOM_COMMAND_XXXX) 
 *             ACOM_CONNECTION * psCnx     : Référence à la connexion concernée
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion
 *             DWORD64 dwCnxUsrKey           : Clé utilisateur de connexion
 *             DWORD64 dwMsgUsrKey           : Clé utilisateur de message
 *             DWORD bDisable              : Cas des déconnexion : TRUE la connexion tentera de se
 *                                           reconnecter. FALSE : la connexion est désactivée.
 *             DWORD dwHeaderSize          : Taille des données pointée par pvHeader
 *             void  * pvHeader            : Données de l'entête. Si pvHeader == NULL et dwHeaderSize != 0,
 *                                           les données du bloc seront allouée selon dwHeaderSize et
 *                                           initialisées à 0.
 *             DWORD dwDataSize            : Taille des données pointée par pvData
 *             void  * pvData              : Données. Si pvData == NULL et dwDataSize != 0, les
 *                                           données du bloc seront allouée selon dwDataSize et
 *                                           initialisées à 0.
 * RETURN    : NULL si erreur, un pointeur de bloc sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue l'allocation et l'initialisation d'un bloc de commande
 *             avec les données contenant un entête
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_BLOCK * AComBlockOpenWithHeader( 
    DWORD dwBlockType,
    ACOM_CONNECTION * psCnx,
    ACOM_CNX_HANDLE hCnxHandle,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
    DWORD bDisable,
    DWORD dwHeaderSize,
    void  * pvHeader,
    DWORD dwDataSize,
    void  * pvData )
{
    ACOM_BLOCK * psBlock;

    AComDbgInfo( __FILE__, __LINE__, "AComBlockOpenWithHeader()" );

    if ( DMEM_ALLOC_BUFFER( psBlock, sizeof(ACOM_BLOCK) + dwHeaderSize + dwDataSize) != NULL )
    {
        psBlock->dwBlockType = dwBlockType;
        psBlock->hCnxHandle = hCnxHandle;
        psBlock->dwCnxUsrKey = dwCnxUsrKey;
        psBlock->psCnx = psCnx;
        psBlock->dwMsgUsrKey = dwMsgUsrKey;
        psBlock->bDisable = bDisable;
        psBlock->dwDataSize = dwHeaderSize + dwDataSize;
        
        if ( dwHeaderSize != 0 ) 
            if ( pvHeader != NULL )
                CopyMemory( &psBlock->tdwDataBlock[1], pvHeader, dwHeaderSize );
            else
                ZeroMemory( &psBlock->tdwDataBlock[1], dwHeaderSize );

        if ( dwDataSize != 0 ) 
			if (pvData != NULL)
			{
				CopyMemory(((BYTE*)(&psBlock->tdwDataBlock[1])) + dwHeaderSize, pvData, dwDataSize);
			
				AComDbgTraceHexBuffer("AComBlockOpenWithHeader ", psBlock->dwDataSize, (BYTE*)(AComBlockDataPtr(psBlock)));
			}
			else
                ZeroMemory( ((BYTE*)(&psBlock->tdwDataBlock[1]))+dwHeaderSize, dwDataSize );
 

	}

    AComDbgInfo( __FILE__, __LINE__, "AComBlockOpenWithHeader() return 0x%016X", psBlock );
    return psBlock;
}    

    


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComBlockClose ( ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_BLOCK * psBlock : Bloc à désallouer
 * RETURN    : rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la désallocation d'un bloc alloué avec AComBlockOpen().
 * --------------------------------------------------------------------
 */
PROTECTED void AComBlockClose( ACOM_BLOCK * psBlock )
{
    AComDbgInfo( __FILE__, __LINE__, "AComBlockClose()" );

    DMEM_FREE( psBlock );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void * AComBlockDataPtr ( ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_BLOCK * psBlock : Bloc concerné
 * RETURN    : Un pointeur sur la zone de données du bloc.
 *             Attention, si le bloc ne contient pas de données, le pointeur
 *             retourné pointe juste après le bloc.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère un pointeur sur la zone de données d'un bloc de commande
 * --------------------------------------------------------------------
 */
PROTECTED void * AComBlockDataPtr( ACOM_BLOCK * psBlock )
{
    void * pvData;

   // AComDbgInfo( __FILE__, __LINE__, "AComBlockDataPtr()" );

    pvData = (void*)(&psBlock->tdwDataBlock[1]);

    //AComDbgInfo( __FILE__, __LINE__, "AComBlockDataPtr return 0x%016X", pvData );
    return pvData;
}






/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD AComBlockPost ( 
 *                      HANDLE hCompletion, 
 *                      DWORD dwBlockType, 
 *                      ACOM_CONNECTION * psCnx, 
 *                      ACOM_CNX_HANDLE hCnxHandle, 
 *                      DWORD64 dwCnxUsrKey, 
 *                      DWORD64 dwMsgUsrKey, 
 *                      BOOL  bDisable, 
 *                      DWORD dwHeaderSize,
 *                      void  * pvHeader,
 *                      DWORD dwDataSize, 
 *                      void  * pvData )
 * PARAMETERS: HANDLE hCompletion          : I/O completion port où le bloc de commande doit
 *             DWORD dwBlockType           : Type du bloc de commande (ACOM_COMMAND_XXXX) 
 *             ACOM_CONNECTION * psCnx     : Référence à la connexion concernée
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion
 *             DWORD64 dwCnxUsrKey           : Clé utilisateur de connexion
 *             DWORD64 dwMsgUsrKey           : Clé utilisateur de message
 *             DWORD bDisable              : Cas des déconnexion : TRUE la connexion tentera de se
 *                                           reconnecter. FALSE : la connexion est désactivée.
 *             DWORD dwHeaderSize          : Taille des données pointée par pvHeader
 *             void  * pvHeader            : Données de l'entête. Si pvHeader == NULL et dwHeaderSize != 0,
 *                                           les données du bloc seront allouée selon dwHeaderSize et
 *                                           initialisées à 0.
 *             DWORD dwDataSize            : Taille des données pointée par pvData
 *             void  * pvData              : Données. Si pvData == NULL et dwDataSize != 0, les
 *                                           données du bloc seront allouée selon dwDataSize et
 *                                           initialisées à 0.
 * RETURN    : NO_ERROR si posté, sinon, un code d'erreur win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Alloue, rempli et post un bloc de commande dans un i/o completion port.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD AComBlockPost( 
    HANDLE hCompletion,
    DWORD dwBlockType,
    ACOM_CONNECTION * psCnx,
    ACOM_CNX_HANDLE hCnxHandle,
	DWORD64 dwCnxUsrKey,
	DWORD64 dwMsgUsrKey,
    BOOL  bDisable,
    DWORD dwHeaderSize,
    void  * pvHeader,
    DWORD dwDataSize,
    void  * pvData )
{
    BOOL bPost;
    DWORD dwErr = NO_ERROR;
    ACOM_BLOCK * psBlock;
    
	AComDbgInfo(__FILE__, __LINE__, "AComBlockPost: dwBlockType:0X%08X - %s", dwBlockType, GetNameForBlockType(dwBlockType));
	if (dwHeaderSize>0)
		AComDbgTraceHexBuffer("AComBlockPost: Header:", dwHeaderSize, (BYTE*)pvHeader);
	if (dwDataSize>0)
		AComDbgTraceHexBuffer("AComBlockPost: Data:", dwDataSize, (BYTE*)pvData);


    psBlock = AComBlockOpenWithHeader(
        dwBlockType, 
        psCnx, 
        hCnxHandle, 
        dwCnxUsrKey, 
        dwMsgUsrKey, 
        bDisable,
        dwHeaderSize,
        pvHeader,
        dwDataSize,
        pvData );
    if ( psBlock != NULL )
    {
        bPost = PostQueuedCompletionStatus(
            hCompletion,
            0,
            0,
            (LPOVERLAPPED)psBlock );
        if ( ! bPost )
        {
            dwErr = GetLastError();
            AComDbgError( dwErr, __FILE__, __LINE__, "PostQueuedCompletionStatus" );
            AComBlockClose( psBlock );
        }
    }
    else
        dwErr = ERROR_NOT_ENOUGH_MEMORY;

    AComDbgInfo( __FILE__, __LINE__, "AComBlockPost return %d", dwErr );
    return dwErr;
}





/* -------------  FIN DU FICHIER : acom_block.c ------------- */ 
