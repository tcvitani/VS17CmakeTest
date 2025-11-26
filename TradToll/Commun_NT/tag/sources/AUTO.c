/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : AUTO
 * FILE       : AUTO.c
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

#include <windows.h>
#include <stdio.h>

#define LOC_DEF
#include "auto.h"
#undef LOC_DEF

#include <memclass.h>



//
// Structure descriptive de l'état d'un automate
//
typedef struct _AUT_STATE
{
    // Identifiant de l'état
    DWORD               dwStateId;

    // Pointeur sur la fonction de transition par défaut (fonction
    // appelée lorsque l'automate reçoit un événement qu'il ne
    // sait pas traiter alors qu'il est dans cet état).
    AUT_TRANSIT       * pfDefaultTransition;
    
    // Index de l'état cible de la transition par défaut dans la
    // table des états.
    DWORD               dwDefaultIndex;

    // Nombre d'événements que l'automate attent lorsqu'il est dans
    // cet état (au maximum de AUT_MAX_TRANSITIONS).
    DWORD               dwEventCount;

    // Liste des identifiants des événement attendus (seuls les
    // dwEventCount premiers éléments sont significatifs).
    DWORD               tdwEvents     [ AUT_MAX_TRANSITIONS ];

    // Liste des pointeurs de fonctions de transition associées aux
    // événement de tdwEvents (seuls les dwEventCount premiers éléments
    // sont significatifs).
    AUT_TRANSIT       * tpfTransitions[ AUT_MAX_TRANSITIONS ];

    // Liste des états cible associés aux événements de
    // tdwEvents (seuls les dwEventCount premiers éléments sont
    // significatifs).
    DWORD               tdwIndex      [ AUT_MAX_TRANSITIONS ];
}
    AUT_STATE;



//
// Structure descriptive de l'état d'un automate complet
//
typedef struct _AUT
{
    // Nombre maximum d'état gérable par l'automate
    DWORD               dwMaxStates;

    // Nombre d'état effectivement utilisés
    DWORD               dwStateCount;

    // Index de l'état courant (dans la table tsStates)
    DWORD               dwCurrentStateIndex;

    // Pointeur de context utilisateur associé à l'instance
    // d'automate.
    void              * pvContext;

    // Table des états de l'automate (seuls les dwStateCount
    // premiers éléments sont significatifs).
    AUT_STATE           tsStates[];
}
    * H_AUT;



    
PRIVATE DWORD WINAPI AutFindState(
            IN          H_AUT               hAut,
            IN          DWORD               dwStateId );

PRIVATE DWORD WINAPI AutFindEvent(
            IN          AUT_STATE     * psState,
            IN          DWORD               dwEventId );

    



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
            IN          void              * pvContext )
{
    H_AUT               hAut  = NULL;

    __try
    {
        //
        // Allouer la structure d'instance
        //
        hAut = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*hAut) + dwMaxStates * sizeof(AUT_STATE) );
        if ( hAut == NULL )
            __leave;

        //
        // Initialiser la structure
        //
        hAut->dwMaxStates = dwMaxStates;
        hAut->pvContext = pvContext;
        hAut->dwCurrentStateIndex = AUT_INVALID;
    }
    __finally
    {
		;
    }

	return hAut;
}




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
            IN          H_AUT               hAut )
{
    HeapFree( GetProcessHeap(), 0, hAut );
}
    



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
            IN          H_AUT               hAut )
{
    return hAut->pvContext;
}

                                             


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
            IN          H_AUT               hAut,
            IN          DWORD               dwStateId )
{
    BOOL        fReturn = FALSE;
    DWORD       dwIndex;

    __try
    {
        //
        // Si la liste des états est pleine, on provoque
        // une erreur
        //
        if ( hAut->dwStateCount >= hAut->dwMaxStates )
            __leave;

        //
        // Si l'identifiant est déjà utilisé, on provoque
        // également une erreur 
        //
        dwIndex = AutFindState( hAut, dwStateId );
        if ( dwIndex != AUT_INVALID )
            __leave;

        //
        // Initialiser le nouvel état
        //
        dwIndex = hAut->dwStateCount;
        ZeroMemory( &hAut->tsStates[dwIndex], sizeof(hAut->tsStates[dwIndex]) );
        hAut->tsStates[dwIndex].dwStateId = dwStateId;

        //
        // Prendre en compte le nouvel état en incrémentant le nombre d'état
        //
        hAut->dwStateCount ++;

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
            IN          H_AUT               hAut,
            IN          DWORD               dwStateIdInitial,
            IN          DWORD               dwEventId,
            IN          DWORD               dwStateIdResult,
            IN          AUT_TRANSIT       * pfTransition )
{
    BOOL            fReturn = FALSE;
    DWORD           dwIndexInitial;
    DWORD           dwIndexResult;
    DWORD           dwIndexEvent;
    AUT_STATE * psState;

    __try
    {
        //
        // Rechercher l'index des états initiaux et finaux
        //
        dwIndexInitial = AutFindState( hAut, dwStateIdInitial );
        dwIndexResult = AutFindState( hAut, dwStateIdResult );

        //
        // Si un des deux états n'est pas trouvé, on ne peut
        // pas ajouter la transition.
        //
        if ( ( dwIndexInitial == AUT_INVALID ) ||
             ( dwIndexResult  == AUT_INVALID )
           )
            __leave;

        //
        // Pointer sur l'état initial
        //
        psState = &hAut->tsStates[dwIndexInitial];

        //
        // Si l'événement ne correspond par à la transition
        // par défaut.
        //
        if ( dwEventId != AUT_EVENT_ANY )
        {
            //
            // S'assurer que le nombre max de transition pour
            // l'état initial n'est pas atteint.
            //
            if ( psState->dwEventCount >= AUT_MAX_TRANSITIONS )
                __leave;

            //
            // S'assurer que l'événement n'est pas déjà géré pour
            // cet état.
            //
            dwIndexEvent = AutFindEvent( psState, dwEventId );
            if ( dwIndexEvent != AUT_INVALID )
                __leave;

            //
            // Mettre à jour les données de la transition
            //
            dwIndexEvent = psState->dwEventCount;
            psState->tdwEvents     [dwIndexEvent] = dwEventId;
            psState->tdwIndex      [dwIndexEvent] = dwIndexResult;
            psState->tpfTransitions[dwIndexEvent] = pfTransition;

            //
            // Prendre en compte le nouvel événement en incrémentant
            // le nombre d'événements.
            //
            psState->dwEventCount ++;
        }

        //
        // Sinon, il s'agit d'une transition par défaut
        //
        else
        {
            //
            // Mettre à jour les données de la transition par défaut
            //
            psState->dwDefaultIndex    = dwIndexResult;
            psState->pfDefaultTransition = pfTransition;
        }

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
            IN          H_AUT               hAut,
            IN          DWORD               dwStateId )
{
    BOOL            fReturn = FALSE;
    DWORD           dwIndex;

    __try
    {
        //
        // Recherche l'index de l'état
        //
        dwIndex = AutFindState( hAut, dwStateId );
        if ( dwIndex == AUT_INVALID )
            __leave;

        //
        // Changer l'index de l'état courant
        //
        hAut->dwCurrentStateIndex = dwIndex;

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
            IN          H_AUT               hAut )
{
    DWORD           dwState = AUT_INVALID;

    __try
    {
        //
        // S'assurer que l'état courant est valide
        //
        if ( hAut->dwCurrentStateIndex == AUT_INVALID )
            __leave;

        //
        // Récupérer l'identifiant de l'état courant
        //
        dwState = hAut->tsStates[hAut->dwCurrentStateIndex].dwStateId;
    }
    __finally
    {
		;
    }

	return dwState;
}




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
            IN          H_AUT               hAut,
            IN          DWORD               dwEventId,
            IN          void              * pvParam )
{
    BOOL            fReturn = FALSE;
    DWORD           dwIndexEvent;
    AUT_STATE     * psState;

    __try
    {
        //
        // S'assurer que l'état courant est valide
        //
        if ( hAut->dwCurrentStateIndex == AUT_INVALID )
            __leave;

        //
        // Pointer sur la structure de l'état courant
        //
        psState = &hAut->tsStates[hAut->dwCurrentStateIndex];
        
        //
        // Rechercher l'événement
        //
        dwIndexEvent = AutFindEvent( psState, dwEventId );

        //
        // Si l'événement n'a pas été trouvé, c'est une transition
        // par défaut.
        //
        if ( dwIndexEvent == AUT_INVALID )
        {
            //
            // Si aucune transition par défaut n'est prévue, on
            // retourne une erreur.
            //
            if ( psState->pfDefaultTransition == NULL )
                __leave;

            //
            // Le changement d'état courant est fait avant d'appeler
            // la fonction de transition.
            //
            hAut->dwCurrentStateIndex = psState->dwDefaultIndex;

            //
            // Appeler la fonction de transition
            //
            if ( psState->pfDefaultTransition != AUT_EMPTY_FUNCTION )
                psState->pfDefaultTransition( hAut, psState->dwStateId, dwEventId, pvParam );

        }

        //
        // Sinon, l'événement a été trouvé, c'est une transtion normale.
        //
        else
        {
            //
            // Le changement d'état courant est fait avant d'appeler
            // la fonction de transition.
            //
            hAut->dwCurrentStateIndex = psState->tdwIndex[dwIndexEvent];

            //
            // Appeler la fonction de transition
            //
            if ( psState->tpfTransitions[dwIndexEvent] != AUT_EMPTY_FUNCTION )
                psState->tpfTransitions[dwIndexEvent]( hAut, psState->dwStateId, dwEventId, pvParam );
        }

        fReturn = TRUE;
    }
    __finally
    {
		;
    }

	return fReturn;
}




PRIVATE DWORD WINAPI AutFindEvent(
            IN          AUT_STATE         * psState,
            IN          DWORD               dwEventId )
{
    DWORD       dwIndex;

    __try
    {
        //
        // Scanner les événement de l'état
        //
        for ( dwIndex = 0 ; dwIndex < psState->dwEventCount ; dwIndex ++ )
            if ( psState->tdwEvents[dwIndex] == dwEventId )
                __leave;

        //
        // Rien n'a été trouvé, on retourne une erreur
        //
        dwIndex = AUT_INVALID;
    }
    __finally
    {
		;
    }

	return dwIndex;
}


PRIVATE DWORD WINAPI AutFindState(
            IN          H_AUT               hAut,
            IN          DWORD               dwStateId )
{
    DWORD       dwIndex;

    __try
    {
        //
        // Scanner la table des états
        //
        for ( dwIndex = 0 ; dwIndex < hAut->dwStateCount ; dwIndex ++ )
            if ( hAut->tsStates[dwIndex].dwStateId == dwStateId )
                __leave;

        //
        // Rien n'a été trouvé, on retourne une erreur
        //
        dwIndex = AUT_INVALID;
    }
    __finally
    {
		;
    }

	return dwIndex;
}

