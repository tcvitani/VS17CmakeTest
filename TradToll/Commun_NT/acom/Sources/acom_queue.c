/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_queue.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Gestion d'une file d'attente de blocs de commandes
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
#include <acom_queue.h>
#undef LOC_DEF

#include <memclass.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_QUEUE * AComQueueOpen ( DWORD dwQueueItems )
 * PARAMETERS: DWORD dwQueueItems : Nombre maximum d'éléments en attente dans la file
 * RETURN    : Un pointeur sur une structure de file, NULL sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création et l'initialisation d'une structure de
 *             file d'attente de blocs de commandes.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_QUEUE * AComQueueOpen( DWORD dwQueueItems )
{
    DWORD           dwSize;
    ACOM_QUEUE    * psQueue = NULL;

    AComDbgInfo( __FILE__, __LINE__, "AComQueueOpen(%d)", dwQueueItems );

    if ( dwQueueItems > 0 )
    {
        dwQueueItems ++;
        dwSize = sizeof(ACOM_QUEUE) + ( dwQueueItems * sizeof( ACOM_BLOCK* ) );
        if ( DMEM_ZALLOC_BUFFER( psQueue, dwSize ) != NULL )
            psQueue->dwQueueItems = dwQueueItems;
        else
            AComDbgError( 0, __FILE__, __LINE__, "DMEM_ZALLOC_BUFFER" );
    }
    
    AComDbgInfo( __FILE__, __LINE__, "AComQueueOpen return 0x%016X", psQueue );
    return psQueue;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL AComQueueIsEmpty ( ACOM_QUEUE * psQueue )
 * PARAMETERS: ACOM_QUEUE * psQueue : Structure de file d'attente
 * RETURN    : TRUE si la file est vide, FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Détermine si une file est vide ou non
 *             ATTENTION : Cette fonction ne supporte pas les accès concurents
 *             sur la même structure.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL AComQueueIsEmpty( ACOM_QUEUE * psQueue )
{
    BOOL bResult = ( psQueue->dwFirst == psQueue->dwLast );
   // AComDbgInfo( __FILE__, __LINE__, "AComQueueIsEmpty(0x%016X)", psQueue );
   // AComDbgInfo( __FILE__, __LINE__, "AComQueueIsEmpty return %s", bResult ? "TRUE" : "FALSE" );
    return bResult;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : BOOL AComQueueIsFull ( ACOM_QUEUE * psQueue )
 * PARAMETERS: PROTECTED ACOM_QUEUE * psQueue : Structure de file d'attente
 * RETURN    : TRUE si la file est pleine, FALSE sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Détermine si une file est pleine ou non
 *             ATTENTION : Cette fonction ne supporte pas les accès concurents
 *             sur la même structure.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL AComQueueIsFull( ACOM_QUEUE * psQueue )
{
    BOOL bResult = ( psQueue->dwFirst == ( ( psQueue->dwLast + 1 ) % psQueue->dwQueueItems ) );
    
	if (bResult)
		AComDbgInfo(__FILE__, __LINE__, "AComQueueIsFull return %s", bResult ? "TRUE" : "FALSE");

    return bResult;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_BLOCK * AComQueueGetItem ( ACOM_QUEUE * psQueue )
 * PARAMETERS: ACOM_QUEUE * psQueue : Structure de file d'attente
 * RETURN    : Un pointeur sur un bloc de commande, NULL si file vide
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lit l'élèment en tête de liste sans l'enlever de la liste
 *             ATTENTION : Cette fonction ne supporte pas les accès concurents
 *             sur la même structure.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_BLOCK * AComQueueGetItem( ACOM_QUEUE * psQueue )
{
    ACOM_BLOCK * psBlock = NULL;
    AComDbgInfo( __FILE__, __LINE__, "AComQueueGetItem(0x%016X)", psQueue );
    if ( ! AComQueueIsEmpty( psQueue ) )
        psBlock = psQueue->tsQueue[psQueue->dwFirst];
    AComDbgInfo( __FILE__, __LINE__, "AComQueueGetItem return 0x%016X", psBlock );
    return psBlock;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_BLOCK * AComQueueRemoveItem ( ACOM_QUEUE * psQueue )
 * PARAMETERS: ACOM_QUEUE * psQueue : Structure de file d'attente
 * RETURN    : Un pointeur sur un bloc de commande, NULL si file vide
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lit l'élèment en tête de liste en le supprimant de la liste
 *             ATTENTION : Cette fonction ne supporte pas les accès concurents
 *             sur la même structure.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_BLOCK * AComQueueRemoveItem( ACOM_QUEUE * psQueue )
{
    ACOM_BLOCK * psBlock = NULL;
    AComDbgInfo( __FILE__, __LINE__, "AComQueueRemoveItem(0x%016X)", psQueue );
    if ( ! AComQueueIsEmpty( psQueue ) )
    {
        psBlock = psQueue->tsQueue[psQueue->dwFirst];
        psQueue->dwFirst = ( ( psQueue->dwFirst + 1 ) % psQueue->dwQueueItems );
    }
    AComDbgInfo( __FILE__, __LINE__, "AComQueueRemoveItem return 0x%016X", psBlock );
    return psBlock;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL AComQueueAddItem ( ACOM_QUEUE * psQueue, ACOM_BLOCK * psBlock )
 * PARAMETERS: ACOM_QUEUE * psQueue : Structure de file d'attente
 *             ACOM_BLOCK * psBlock : Bloc de commande
 * RETURN    : TRUE si ajouté, FALSE si liste saturée
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un élèment en fin de liste.
 *             ATTENTION : Cette fonction ne supporte pas les accès concurents
 *             sur la même structure.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL AComQueueAddItem( ACOM_QUEUE * psQueue, ACOM_BLOCK * psBlock )
{
    BOOL bResult = FALSE;
    AComDbgInfo( __FILE__, __LINE__, "AComQueueAddItem(0x%016X,0x%016X)", psQueue, psBlock );
    if ( ! AComQueueIsFull( psQueue ) )
    {
        psQueue->tsQueue[psQueue->dwLast] = psBlock;
        psQueue->dwLast = ( ( psQueue->dwLast + 1 ) % psQueue->dwQueueItems );
        bResult = TRUE;
    }
    AComDbgInfo( __FILE__, __LINE__, "AComQueueAddItem return %s", bResult ? "TRUE" : "FALSE" );
    return bResult;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComQueueClose ( ACOM_QUEUE * psQueue )
 * PARAMETERS: ACOM_QUEUE * psQueue : Structure de file d'attente
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la fermeture de la liste. Si elle contient des
 *             blocs de commandes, ceux-ci sont également libérés.
 * --------------------------------------------------------------------
 */
PROTECTED void AComQueueClose( ACOM_QUEUE * psQueue )
{
    ACOM_BLOCK * psBlock;

    AComDbgInfo( __FILE__, __LINE__, "AComQueueClose(0x%016X)", psQueue );

    if ( psQueue != NULL )
    {

        while ( ( psBlock = AComQueueRemoveItem( psQueue ) ) != NULL )
            DMEM_FREE( psBlock );

        DMEM_FREE( psQueue );
    }
}


/* -------------  FIN DU FICHIER : acom_queue.c ------------- */ 
