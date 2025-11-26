/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : AUTO
 * FILE       : CSR_AUTO.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : automate, allégé
 * --------------------------------------------------------------------
 * SUMMARY    : Version allégé d'une gestion d'automate.
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */


#ifndef CSR_AUTO_H
#define CSR_AUTO_H


#include <protect.h>


//
// Nombre max de transitions par état
//
#define AUT_MAX_TRANSITIONS 16


//
// Valeur indiquant une erreur
//
#define AUT_INVALID         0xFFFFFFFF


//
// Valeur indiquant un événement par défaut
//
#define AUT_EVENT_ANY       0xFFFFFFFF


//
// Valeur indiquant une fonction de transation sans effet
//
#define AUT_EMPTY_FUNCTION  ((AUT_TRANSIT*)0xFFFFFFFF)


//
// Définition d'un handler
//
typedef struct _AUT * H_AUT;


//
// Définition d'une fonction de transition
//
typedef void WINAPI AUT_TRANSIT( H_AUT hAut, DWORD dwInitialStateId, DWORD dwEvent, void * pvParam );




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED H_AUT WINAPI AutOpen(
 *                      IN          DWORD               dwMaxStates,
 *                      IN          void              * pvContext )
 * PARAMETRES: dwMaxStates : Nombre maximum d'état gérables par l'automate.
 *             pvContext   ; Context utilisateur associé au nouvel automate.
 * RETOURNE  : NULL en cas d'erreur, un handle d'automate sinon.
 * --------------------------------------------------------------------
 * ROLE      : Crée une nouvelle instance d'automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED H_AUT WINAPI AutOpen(
            IN          DWORD               dwMaxStates,
            IN          void              * pvContext );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED void WINAPI AutClose(
 *                      IN          H_AUT               hAut )
 * PARAMETRES: hAut : Handle obtenu avec AutOpen().
 * RETOURNE  : Rien
 * --------------------------------------------------------------------
 * ROLE      : Détruit une instance créée avec AutOpen().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED void WINAPI AutClose(
            IN          H_AUT           hAut );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED void * WINAPI AutGetContext(
 *                      IN          H_AUT               hAut )
 * PARAMETRES: hAut : Handle obtenu avec AutOpen().
 * RETOURNE  : La valeur du context utilisateur fournie en paramètre
 *             lors de l'appel à AutOpen().
 * --------------------------------------------------------------------
 * ROLE      : Obtient la valeur du contexte utilisateur associé à un
 *             automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED void * WINAPI AutGetContext(
            IN          H_AUT           hAut );
                                             

/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED BOOL WINAPI AutAddState(
 *                      IN          H_AUT               hAut,
 *                      IN          DWORD               dwStateId )
 * PARAMETRES: hAut     : Handle obtenu avec AutOpen().
 *             dwStateId: Identifiant unique de l'état à ajouter à l'automate.
 * RETOURNE  : TRUE : Succés.
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un nouvel état à l'automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED BOOL WINAPI AutAddState(
            IN          H_AUT           hAut,
            IN          DWORD               dwStateId );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED BOOL WINAPI AutAddTransition(
 *                      IN          H_AUT               hAut,
 *                      IN          DWORD               dwStateIdInitial,
 *                      IN          DWORD               dwEventId,
 *                      IN          DWORD               dwStateIdResult,
 *                      IN          AUT_TRANSIT       * pfTransition )
 * PARAMETRES: hAut            : Handle obtenu avec AutOpen().
 *             dwStateIdInitial: Identifiant unique de l'état initial de la transition
 *             dwEventId       : Identifiant unique de l'événement provoquant la transition
 *             dwStateIdResult : Identifiant unique de l'état initial de la transition
 *             pfTranstion     : Fonction de transition associée à l'événement ou
 *                               AUT_EMPTY_FUNCTION si aucun traitement n'est associé à la
 *                               transition.
 * RETOURNE  : TRUE : Succés.
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Ajoute une nouvelle transition à l'automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED BOOL WINAPI AutAddTransition(
            IN          H_AUT           hAut,
            IN          DWORD               dwStateIdInitial,
            IN          DWORD               dwEventId,
            IN          DWORD               dwStateIdResult,
            IN          AUT_TRANSIT       * pfTransition );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED BOOL WINAPI AutSetCurrentState(
 *                      IN          H_AUT               hAut,
 *                      IN          DWORD               dwStateId )
 * PARAMETRES: hAut      : Handle obtenu avec AutOpen().
 *             dwStateId : Identifiant unique de nouvel état courant à affecter à l'automate
 * RETOURNE  : TRUE : Succés.
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Force le changement d'état d'un automate (sans gérer de transitions).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED BOOL WINAPI AutSetCurrentState(
            IN          H_AUT           hAut,
            IN          DWORD               dwStateId );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED DWORD WINAPI AutGetCurrentState(
 *                      IN          H_AUT               hAut )
 * PARAMETRES: hAut      : Handle obtenu avec AutOpen().
 * RETOURNE  : Identifiant unique de l'état courant de l'automate.
 *             AUT_INVALID en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Détermine l'état courant d'un automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED DWORD WINAPI AutGetCurrentState(
            IN          H_AUT           hAut );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PROTECTED BOOL WINAPI AutSendEvent(
 *                      IN          H_AUT               hAut,
 *                      IN          DWORD               dwEventId,
 *                      IN          void              * pvParam )
 * PARAMETRES: hAut      : Handle obtenu avec AutOpen().
 *             dwEventId : Identifiant unique de l'événement
 *             pvParam   : Paramètre utilisateur associé à l'événement
 * RETOURNE  : TRUE : Succés
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Provoque une transition dans un automate.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PROTECTED BOOL WINAPI AutSendEvent(
            IN          H_AUT           hAut,
            IN          DWORD               dwEventId,
            IN          void              * pvParam );


#endif