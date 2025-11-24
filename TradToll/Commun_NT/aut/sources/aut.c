/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: AUT
* FICHIER: automate.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE: AUTOMATE
* --------------------------------------------------------------------
* RESUME: Moteur d'automate
* --------------------------------------------------------------------
* DESCRIPTION: Ce module contient le moteur d'automate.
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/Commun_NT/Aut/Sources/AUT.C_v  $
 * 
 *    Rev 1.3   Dec 17 2001 16:20:50   PBOUTELE
 *  
 * 
 *    Rev 1.2   Apr 25 2001 18:30:10   pboutele
 *  
* 
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

#include <csrlc32.h>

#include "csr_aut.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

typedef struct
{
    void *data; // données mémorisé au AUT_Lance et passé a chaque fct de transition
    aut_automate *table_etats;
    aut_transition *extension;
    aut_etat_id etat_initial;
    aut_etat_id nb_etats;
    aut_etat_id etat_courant;
    aut_etat_id etat_precedent;
    aut_event_id event_courant;
    boolean recursion;
    CRITICAL_SECTION cs;
  
	char nom[_MAX_PATH];
} aut_struct_automate;

/*--------------- FUNCTIONS: ---------------*/

/*--------------- VARIABLES: ---------------*/

PUBLIC aut_event AUT_EVT_EN_ENTREE  = AUT_EVT_INIT("AUT_EVT_EN_ENTREE", AUT_ARGS);
PUBLIC aut_event AUT_EVT_EN_SORTIE  = AUT_EVT_INIT("AUT_EVT_EN_SORTIE", AUT_ARGS);
PUBLIC aut_event AUT_EVT_PAR_DEFAUT = AUT_EVT_INIT("AUT_EVT_PAR_DEFAUT", AUT_ARGS);
PUBLIC aut_event AUT_EVT_NULL       = AUT_EVT_INIT("AUT_EVT_NULL", AUT_NO_ARGS);
PUBLIC aut_event AUT_EVT_TIMEOUT    = AUT_EVT_INIT("AUT_EVT_TIMEOUT", AUT_NO_ARGS);

/*--------------- CODE: ---------------*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_Lance (aut_parametres *parametres,
*                                            noyau_pool_id pool,
*                                            aut_automate_id *id)
* PARAMETRES: - pointeur sur la table parametre de l'automate,
*             - pool d'allocation des varaibles de l'automate,
*             - pointeur vers les variables allouées.
* RETOUR: AUT_OK sinon erreur.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE: Creation de l'automate.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC aut_enum_retour AUT_Lance (aut_automate_id *id,
                                  void *data,
                                  aut_etat_id etat_initial,
                                  aut_etat_id nb_etats,
                                  aut_automate table,
                                  aut_etat extension,
                                  char *nom)
{
    aut_struct_automate *automate;
    aut_enum_retour retour = AUT_OK;
    const aut_transition *etat;
    int transition_courante;
    aut_etat_id i;
    const aut_transition *en_entree;
    const aut_transition *en_sortie;

    // par defaut
    *id = NULL;

    do {
        // allocation des variables de l'automate dans un pool
        automate = malloc (sizeof(aut_struct_automate));
        if (automate == NULL)
        {
            ////AUT_Erreur (NULL, "AUT_Lance(%s) : echec d'allocation de l'automate (taille = %u)", nom, sizeof(aut_struct_automate));
            retour = AUT_ALLOC_NOK;
            break;
        }

        // renvoie d'un pointeur anonyme sur l'automate
        *id = automate;

        // initialisations
        memset (automate, 0, sizeof (aut_struct_automate));
        automate->data = data;
        automate->table_etats = (aut_automate *) table;
        automate->extension = (aut_transition *) extension;
        automate->etat_initial = etat_initial;
        automate->etat_courant = etat_initial;
        automate->etat_precedent = etat_initial;
        automate->event_courant = AUT_EVT_NULL;
        automate->nb_etats = nb_etats;
        automate->recursion = FALSE;
		strncpy_s(automate->nom, sizeof(automate->nom)-1, nom, _MAX_PATH);
    
        InitializeCriticalSection (&automate->cs);
    
         if (table == NULL)
        {
            retour = AUT_TABLE_NOK;
            break;
        }
    
        if (etat_initial < 0)
        {
            retour = AUT_ETAT_INITIAL_NOK;
            break;
        }
    
        if (nb_etats < 0)
        {
            retour = AUT_NB_ETATS_NOK;
            break;
        }

        // on recherche d'éventuels evenements d'entree/sortie par defaut
        en_entree = en_sortie = NULL;
        if (extension != NULL)
        {
            transition_courante = 0;
            while (extension[transition_courante].event != (aut_event_id) AUT_EVT_NULL)
            {
                if (extension[transition_courante].event == (aut_event_id) AUT_EVT_EN_ENTREE)
                    en_entree = (aut_transition *) &extension[transition_courante].event;

                if (extension[transition_courante].event == (aut_event_id) AUT_EVT_EN_SORTIE)
                    en_sortie = (aut_transition *) &extension[transition_courante].event;

                transition_courante++;
            }
        }

        // pour tous les etats de l'automate, on recherche des evt d'E/S
        for (i = 0 ; i  < nb_etats ; i++)
        {
            // on teste si le pointeur est non NULL (cas des alarmes du PV)
            etat = table[i].etat;
            if (etat == NULL)
                continue;

            // on commence par rechercher dans les transitions de l'etat
            transition_courante = 0;
            table[i].en_entree = table[i].en_sortie = NULL;
            while (etat[transition_courante].event != (aut_event_id) AUT_EVT_NULL)
            {
                if (etat[transition_courante].event == (aut_event_id) AUT_EVT_EN_ENTREE)
                    table[i].en_entree = (aut_transition *) &etat[transition_courante].event;

                if (etat[transition_courante].event == (aut_event_id) AUT_EVT_EN_SORTIE)
                    table[i].en_sortie = (aut_transition *) &etat[transition_courante].event;

                transition_courante++;
            }

            // on complete eventuellement par les evt par defaut
            if (table[i].en_entree == NULL)
                table[i].en_entree = en_entree;
            if (table[i].en_sortie == NULL)
                table[i].en_sortie = en_sortie;
        }
    }
    while(FALSE);
       
    if (retour != AUT_OK)
       AUT_Arret(id);
    
    return retour;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_Arret (aut_automate_id *automate)
* PARAMETRES: - pointeur vers un automate
* RETOUR: AUT_OK sinon erreur
* --------------------------------------------------------------------
* TYPE: Procedure
* ROLE: Liberation memoire des variables d'un automate
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC aut_enum_retour AUT_Arret (aut_automate_id *id)
{
    aut_struct_automate *automate;
    
    if (id == NULL)
    {
        ////AUT_Erreur(NULL, "AUT_Arret() : automate_id incorrect (nul)");
        
        return AUT_AUTOMATE_ID_NOK;
    }
    
    automate = *id;

    //AUT_Trace (automate, "Arret de '%s' etat courant = %d\n", automate->nom, automate->etat_courant);

    DeleteCriticalSection(&automate->cs);
    
     
    free (*id);
    
    *id = NULL;
    
    return AUT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_Envoie (aut_automate_id id,
*                                             aut_event_id event
*                                             void *args)
* PARAMETRES:
* - pointeur vers un automate,
* - evenement,
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE:
*   Cette fonction appelle l'action correspondante a l'evenement
*   courant et effectue des verifications sur la validite des etats et
*   des evenements.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC aut_enum_retour AUT_Envoie (aut_automate_id id, aut_event_id event, void *args)
{
    aut_struct_automate  *automate;
    const aut_transition *etat;
    const aut_transition *transition = NULL;
    const aut_transition *transition_par_defaut = NULL;
    const aut_transition *tmp = NULL;
    int transition_courante;
    aut_etat_id etat_precedent;
    char *event_string = (char *) event;
    BOOL ret = TRUE;
    
    /* l'id de l'automate est-il valide ? */
    if (id == NULL)
    {
        ////AUT_Erreur(NULL, "AUT_Envoie() : automate_id incorrect (nul) !");
        
        return AUT_AUTOMATE_ID_NOK;
    }
    
    /* on le caste pour l'exploiter */
    automate = id;
    
    EnterCriticalSection(&automate->cs);
    
    /* on inscrit son nom dans le fichier de traces */
    //AUT_Trace (automate, "%s :", automate->nom);
    
    /* on sauvegarde l'evenement */
    automate->event_courant = event;
    
    /* si l'evenement est nul, il n'y a rien a faire */
    if (event == (aut_event_id) AUT_EVT_NULL)
    {
        //AUT_Trace (automate, "Etat courant = %d", automate->etat_courant);
        //AUT_Trace (automate, "Evt nul");
        
        LeaveCriticalSection(&automate->cs);
        
        return AUT_OK;
    }
    
    /* teste si l'evenement reclame des arguments,
    si absent -> erreur */
    if (event_string[0] == '@' && args == NULL)
    {
        //AUT_Erreur (automate, "Etat courant = %d", automate->etat_courant);
        //AUT_Erreur (automate, "Evt manque ARG = '%s' (%p) !", automate->event_courant, event);
        
        LeaveCriticalSection(&automate->cs);
        
        return AUT_ARGS_NULL;
    }
    
    ///* teste si l'evenement ne reclame pas d'arguments,
    //si present -> traces */
    //if (event_string[0] == '_' && args != NULL)
    //{
    //    AUT_Trace (automate, "Etat courant = %d", automate->etat_courant);
    //    AUT_Trace (automate, "Evt arg inattendu = '%s' (%p)", automate->event_courant, event);
    //}
    
    /* on recherche pour cet etat la transition qui correspond
    a l'evenement dans le tableau des events de l'etat ou sinon la transition par defaut */
    transition_courante = 0;
    etat = (*automate->table_etats)[automate->etat_courant].etat;
    while (etat[transition_courante].event != (aut_event_id) AUT_EVT_NULL)
    {
        /* est-ce l'evt recu ? */
        if (etat[transition_courante].event == event)
        {
            transition = &etat[transition_courante];
            
            break;
        }
        /* on memorise l'evt par defaut qd on le trouve */
        if (etat[transition_courante].event == (aut_event_id) AUT_EVT_PAR_DEFAUT)
            transition_par_defaut = &etat[transition_courante];
        
        transition_courante++;
    }
    
    /* si l'on n'a pas trouve de transition pour l'evenement,
    ni de transition par defaut, on recherche dans le tableau d'extension */
    etat = automate->extension;
    if (transition == NULL && 
        transition_par_defaut == NULL &&
        etat != NULL)
    {
        transition_courante = 0;
        while (etat[transition_courante].event != (aut_event_id) AUT_EVT_NULL)
        {
            /* est-ce l'evt recu ? */
            if (etat[transition_courante].event == event)
            {
                transition = &etat[transition_courante];
            
                break;
            }
            /* on memorise l'evt par defaut qd on le trouve */
            if (etat[transition_courante].event == (aut_event_id) AUT_EVT_PAR_DEFAUT)
                transition_par_defaut = &etat[transition_courante];
        
            transition_courante++;
        }
    }
    
    // si l'on n'a toujours rien trouvé, on sort NOK
    if (transition == NULL && transition_par_defaut == NULL)
    {
        //AUT_Trace (automate, "Etat courant = %d", automate->etat_courant);
        //AUT_Trace (automate, "Evt inconnu  = '%s' (%p)", automate->event_courant, event);
        
        LeaveCriticalSection(&automate->cs);
        
        return AUT_EVT_INCONNU;
    }
    
    /* si l'on n'a pas trouve de transition pour l'evenement,
    mais que l'on a un evt par defaut, on le prend */
    if (transition == NULL)
        transition = transition_par_defaut;

    // si l'on ne change pas d'etat avec AUT_ETAT_COURANT comme etat suivant
    // on n'execute pas les evenements d'entree et de sortie
    if (transition->etat_suivant == AUT_ETAT_COURANT)
    {
        /* quelques traces... */
        //AUT_Trace (automate, "AUT_ETAT_COURANT");
        //if (transition->event == (aut_event_id) AUT_EVT_PAR_DEFAUT)
        //    AUT_Trace (automate, "Evt par defaut sur '%s' (%p)", automate->event_courant, event);
        //else
        //    AUT_Trace (automate, "Evenement    = '%s' (%p)", automate->event_courant, event);
    
        /* en cas de reentrance, on positionne un flag statique
        a false pendant la descente de recursion... cf AUT_EnRecursion() */
        automate->recursion = FALSE;
        etat_precedent = automate->etat_precedent = automate->etat_courant;

        /* on effectue l'action associee a la transition */
        if( transition->action != AUT_ACTION_NULL)
        {
            ret = transition->action (automate->data, args);
            //if (ret == FALSE)
                //AUT_Erreur (automate, "Erreur en retour d'action !");
        }
    
        /* on passe a true le flag de l'eventuelle recursion lors de la remontée. */
        automate->recursion = TRUE;
        automate->etat_precedent = etat_precedent;
    }
    else
    {

        /* si l'etat suivant est invalide, on sort NOK */
        if (transition->etat_suivant > automate->nb_etats)
        {
            //AUT_Erreur(automate, "AUT_Envoie(%s) : etat_suivant nø%d inconnu !! (>= nb_etats) pour etat %d / event %s (%p)",
            //    automate->nom, transition->etat_suivant, automate->etat_courant, automate->event_courant, event);
            AUT_Reset (id);
        
            LeaveCriticalSection(&automate->cs);
        
            return AUT_ETAT_SUIVANT_NOK;
        }
    
        /* en cas de reentrance, on positionne un flag statique
        a false pendant la descente de recursion... cf AUT_EnRecursion() */
        automate->recursion = FALSE;
        etat_precedent = automate->etat_precedent = automate->etat_courant;

        // si un evenement de sortie existe, on execute l'action associée avant de changer d'etat
        tmp = (*automate->table_etats)[automate->etat_courant].en_sortie;
        if (tmp != NULL)
        {
            if (tmp->action != AUT_ACTION_NULL)
            {
                //AUT_Trace (automate, "Evt de sortie sur '%s' (%p)", automate->event_courant, event);
                //if (
				tmp->action(automate->data, args);
					// == FALSE)
                    //AUT_Erreur (automate, "Erreur en retour d'action sur l'evt de sortie !");
            }
        }

        ///* quelques traces... */
        //AUT_Trace (automate, "Etat courant = %d", automate->etat_courant);
        //if (transition->event == (aut_event_id) AUT_EVT_PAR_DEFAUT)
        //    AUT_Trace (automate, "Evt par defaut sur '%s' (%p)", automate->event_courant, event);
        //else
        //    AUT_Trace (automate, "Evenement    = '%s' (%p)", automate->event_courant, event);

        /* REMARQUE : on commence par changer d'etat avant d'appeler
        la fonction de transition pour la réentrance */
        automate->etat_courant = transition->etat_suivant;

        //AUT_Trace (automate, "Etat suivant = %d", transition->etat_suivant);
    
        /* on effectue l'action associee a la transition */
        if( transition->action != AUT_ACTION_NULL)
        {
            ret = transition->action (automate->data, args);
            //if (ret == FALSE)
                //AUT_Erreur (automate, "Erreur en retour d'action !");
        }

        automate->etat_precedent = automate->etat_courant;

        // si un evenement d'entree existe pour l'etat suivant, 
        // on execute l'action associée
        tmp = (*automate->table_etats)[automate->etat_courant].en_entree;
        if (tmp != NULL)
        {
            if (tmp->action != AUT_ACTION_NULL)
            {
                //AUT_Trace (automate, "Evt d'entree sur '%s' (%p)", automate->event_courant, event);
               // if (
				tmp->action(automate->data, args);
						//== FALSE)
                    //AUT_Erreur (automate, "Erreur en retour d'action sur l'evt d'entree !");
            }
        }

        /* on passe a true le flag de l'eventuelle recursion lors de la remontée. */
        automate->recursion = TRUE;
        automate->etat_precedent = etat_precedent;
    }
    
    LeaveCriticalSection(&automate->cs);
    
    /* dernier test sur la fonction de transition */
    if (ret == FALSE)
        return AUT_TRANSITION_NOK;

    return AUT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_RetourEtatPrecedent( aut_automate_id id )
* PARAMETRES: - identificateur de l'automate
* RETOUR: AUT_AUTOMATE_ID_NOK ou AUT_OK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE: Remet l'automate dans son etat precdent.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC aut_enum_retour AUT_RetourEtatPrecedent( aut_automate_id id )
{
    aut_struct_automate *automate;
    
    if( id == NULL )
    {
        //AUT_Erreur(NULL, "AUT_RetourEtatPrecedent() : automate_id incorrect (nul)");
        
        return AUT_AUTOMATE_ID_NOK;
    }
    
    automate = id;
    
    EnterCriticalSection(&automate->cs);
    
    //AUT_Trace (automate, "AUT_RetourEtatPrecedent '%s' etat courant %u -> precedent = %u\n", automate->nom, automate->etat_courant, automate->etat_precedent);
    
    automate->etat_courant = automate->etat_precedent;

    LeaveCriticalSection(&automate->cs);
    
    return AUT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_Reset( aut_automate_id id )
* PARAMETRES: - identificateur de l'automate
* RETOUR: AUT_AUTOMATE_ID_NOK ou AUT_OK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE: Remet l'automate dans son etat initial.
* --------------------------------------------------------------------
* $F_FCTN
*/

PUBLIC aut_enum_retour AUT_Reset( aut_automate_id id )
{
    aut_struct_automate *automate;
    
    if( id == NULL )
    {
        //AUT_Erreur(NULL, "AUT_Reset() : automate_id incorrect (nul)");
        
        return AUT_AUTOMATE_ID_NOK;
    }
    
    automate = id;
    
    EnterCriticalSection(&automate->cs);
    
    automate->etat_courant = automate->etat_initial;
    
    //AUT_Trace (automate, "Reset '%s' etat initial = %u\n", automate->nom, automate->etat_courant);
    
    LeaveCriticalSection(&automate->cs);
    
    return AUT_OK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_DonneEtatCourant (aut_automate_id id,
*                                                       aut_etat_id *etat_courant)
* PARAMETRES:
* - pointeur vers un automate,
* RETOUR: AUT_OK ou AUT_AUTOMATE_ID_NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE:
*   Cette focntion renvoie l'état courant d'un automate
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC aut_etat_id AUT_DonneEtatCourant (aut_automate_id id)
{
    aut_struct_automate *automate;
    
    if (id == NULL)
    {
        //AUT_Erreur (NULL, "AUT_DonneEtatCourant() : automate_id incorrect (nul)");
        
        return -1;
    }
    
    automate = id;
    
    return automate->etat_courant;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC aut_enum_retour AUT_DonneEventCourant (aut_automate_id id)
* PARAMETRES:
* - pointeur vers un automate,
* RETOUR: evenement courant
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE:
*   Dans une fonction de transition, renvoie l'evenement qui a declenché
* son appel par l'automate.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC aut_event_id AUT_DonneEventCourant (aut_automate_id id)
{
    aut_struct_automate *automate;
    
    if (id == NULL)
    {
        //AUT_Erreur (NULL, "AUT_DonneEventCourant() : automate_id incorrect (nul)");
        
        return NULL;
    }
    
    automate = id;
    
    return automate->event_courant;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC boolean AUT_EnRecursion (aut_automate_id id)
* PARAMETRES:
* - pointeur vers un automate
* RETOUR: FALSE ou TRUE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: A utiliser dans une fonction de transition.
* ROLE: Renvoie vrai si en retour d'une fonction AUT_Envoie() dans une
* fonction de transition d'un automate, un appel … une autre fonction
* de transition du mˆme automate … eu lieu (recursion)
* (les variables de l'automate ont pu ˆtre modifiées et
* peuvent etre differentes d'avant l'appel … AUT_Envoie).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean AUT_EnRecursion (aut_automate_id id)
{
    aut_struct_automate *automate;
    
    if (id == NULL)
    {
        //AUT_Erreur (NULL, "AUT_EnRecursion() : automate_id incorrect (nul)");
        
        return TRUE;
    }
    
    automate = id;
    
    //AUT_Trace (automate, "AUT_EnRecursion() : %d", automate->recursion);
    
    return automate->recursion;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC char * AUT_DonneNom (aut_automate_id id)
* PARAMETRES:
* - pointeur vers un automate,
* RETOUR: pointeur vers la chaine du nom
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC char * AUT_DonneNom (aut_automate_id id)
{
    aut_struct_automate *automate;
    
    if (id == NULL)
    {
        //AUT_Erreur (NULL, "AUT_DonneNom() : automate_id incorrect (nul)");
        
        return NULL;
    }
    
    automate = id;
    
    return automate->nom;
}
