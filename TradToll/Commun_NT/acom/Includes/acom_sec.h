/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_sec.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion de la sécurité sur les IPC
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_SEC_H
#define ACOM_SEC_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED SECURITY_ATTRIBUTES * AComSecOpen (  )
 * PARAMETERS: Aucun
 * RETURN    : Un pointeur sur une structure d'attributs de sécurité nulle.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Créer une structure d'attributs de sécurité nulle prète
 *             à être utilisée par un pipe ou un mailslot.
 * --------------------------------------------------------------------
 */
PROTECTED SECURITY_ATTRIBUTES * AComSecOpen();


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComSecClose ( SECURITY_ATTRIBUTES * psSA )
 * PARAMETERS: SECURITY_ATTRIBUTES * psSA : Pointe sur une structure d'attributs de sécurité
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Désalloue une structure d'attributs de sécurités ouverte avec AComSecOpen().
 * --------------------------------------------------------------------
 */
PROTECTED void AComSecClose( SECURITY_ATTRIBUTES * psSA );


#endif


/* -------------  FIN DU FICHIER : acom_sec.h ------------- */ 
