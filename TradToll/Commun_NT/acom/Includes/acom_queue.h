/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_queue.h
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

#ifndef ACOM_QUEUE_H
#define ACOM_QUEUE_H

#include <protect.h>


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
PROTECTED ACOM_QUEUE * AComQueueOpen( DWORD dwQueueItems );


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
PROTECTED BOOL AComQueueIsEmpty( ACOM_QUEUE * psQueue );


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
PROTECTED BOOL AComQueueIsFull( ACOM_QUEUE * psQueue );


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
PROTECTED ACOM_BLOCK * AComQueueGetItem( ACOM_QUEUE * psQueue );


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
PROTECTED ACOM_BLOCK * AComQueueRemoveItem( ACOM_QUEUE * psQueue );


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
PROTECTED BOOL AComQueueAddItem( ACOM_QUEUE * psQueue, ACOM_BLOCK * psBlock );


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
PROTECTED void AComQueueClose( ACOM_QUEUE * psQueue );

#endif


/* -------------  FIN DU FICHIER : acom_queue.h ------------- */ 
