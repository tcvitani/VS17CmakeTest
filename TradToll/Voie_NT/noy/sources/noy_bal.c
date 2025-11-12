/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL NOYAU
* FICHIER: NOY_BAL.C
* LANGAGE: C
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <stdio.h>

#include "noyau.h"
#include "noy_loc.h"
#include "noy_dbg.h"
#include "reg.h"

/*--------------- RESERVED: ---------------*/
#include "memclass.h"
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/

#define FMT_NEUTRE " { %p / tache %lu / bal %s / retour %s }"
#define FMT_TACHE_NOM_ID_MESS " { tache %lu / %s / bal %lu / max %lu }"
#define FMT_TACHE_NOM " { tache %lu / %s }"
#define FMT_DUMP " { %0lu %0lu }"

#define BAL_HKEY    CSR_REG_KEYi_ROOT
#define BAL_SUB_KEY CSR_REG_KEYn_CSRBASE CSR_REG_KEYn_LANE_BASE CSR_REG_KEYn_DYNAMIC "Mailslots"
#define BAL_COUNTER "BAL_Counter"
#define BAL_COUNTER_TOTAL "BAL_Last_Identifier"
#define BAL_SLOT_NAME "\\\\%s\\mailslot\\CSR_BAL_%s"

/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
/*--------------- VARIABLES: ---------------*/

/*--------------- CODE: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED noyau_enum_retour InitBAL (void)
* PARAMETRES:
* ENTREE:
* RETOUR:
*          - NOYAU_OK
*          - ExitBad()
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction globale.
* ROLE: Initialisation des Boites Aux Lettres
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED noyau_enum_retour InitBAL (void)
{
    noyau_enum_retour retour;
    DWORD ret;
    INT i;
    
    __try
    {
        retour = 0;
        
        // si premier démarrage de la DLL
       if (NOYAU_NB_INSTANCES == 0)
        {
            // on efface la cle 'Mailslots' et toutes ses valeurs
            ret= REG_Efface_Cle (BAL_HKEY, BAL_SUB_KEY, NULL);
            if (ret != ERROR_SUCCESS && ret != ERROR_FILE_NOT_FOUND)
                return retour = __LINE__;
            
            // on recrée la cle 'Mailslot' avec la valeur du compteur à 0
            if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, 0) != ERROR_SUCCESS)
                return retour = __LINE__;

            // on recrée la cle 'Mailslot' avec la valeur du compteur à 0
            if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER_TOTAL, 0) != ERROR_SUCCESS)
                return retour = __LINE__;
        }
        
        // init par défaut du tableau des handles de BAL
        for (i=0; i<MAX_MAILSLOTS; i++)
        { 
            _noyau_.hMailslotRead[i] = INVALID_HANDLE_VALUE;
            _noyau_.hMailslotWrite[i] = INVALID_HANDLE_VALUE;
        }

        // Init du nom de la BAL des chronos
		strncpy_s(_NOYAU_.sMailslotInfos[0].BalName, MAX_PATH, "BL_CHRONO", MAX_PATH);
    }
    
    __finally
    {
        if (retour != 0)
        {
#ifdef ERREURS
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "InitBAL() : echec ligne %d code %ld", retour, GetLastError());
#endif
            
            retour = NOYAU_INIT_BAL_NOK;
        }
        else
            retour = NOYAU_OK;
    }
    
    return retour;
}


PRIVATE int DonnePositionDonneesBAL (noyau_bal_id id_bal)
{
    int i;

    /* recherche l'id de BAL dans le tableau des mailslots et retourne sa position dans le tableau */
    for (i=0; i < MAX_MAILSLOTS; i++)
       if (_NOYAU_.sMailslotInfos[i].bal_id == id_bal && _NOYAU_.sMailslotInfos[i].BalName[0] != '\0'  )
          return i;

    /* Identifiant de BAL non trouve */
    return 0;
}


PRIVATE noyau_bal_id NouveauNomBAL (char *nom_bal)
{
    DWORD indice_bal;
    noyau_bal_id bal_id;
    int retour;
    int i;

    bal_id = 0;

    __try
    {
        retour = 0;
        
        // on récupère le compteur du nombre de bal publiées
        if (REG_Lire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, &indice_bal) != ERROR_SUCCESS)
            return retour = __LINE__;

        // pré-incrémentation car on fait commencer les n° de BAL à 1 (BAL 0 réservée aux chronos)
        indice_bal ++;
        
        // test validité du bal_id
        if (indice_bal < 0 || indice_bal >= MAX_MAILSLOTS)
            return retour = __LINE__;

        // recherche d'un indice libre dans le tableau (c-a-d nom de bal non vide)
        indice_bal = 0;
        for (i=0; (i < MAX_MAILSLOTS) && (indice_bal == 0); i++)
          if (_NOYAU_.sMailslotInfos[i].BalName[0] == '\0')
            indice_bal = i;

        // test validité du indice_bal
        if (indice_bal == 0)
            return retour = __LINE__;

        // on récupère le compteur du nombre de bal publiées depuis le lancement du noyau
        if (REG_Lire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER_TOTAL, &bal_id) != ERROR_SUCCESS)
            return retour = __LINE__;
        bal_id++;
        // si c'est OK, on met à jour le compteur
        if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER_TOTAL, bal_id) != ERROR_SUCCESS)
            return retour = __LINE__;

        // ajoute une entrée dans la registry avec le nom de publication de la BAL
        // et comme valeur son n° d'identification
        if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, nom_bal, bal_id) != ERROR_SUCCESS)
            return retour = __LINE__;

        // mémorisation du nom de la BAL et de son identificateur
		strncpy_s(_NOYAU_.sMailslotInfos[indice_bal].BalName, MAX_PATH, nom_bal, MAX_PATH);
        _NOYAU_.sMailslotInfos[indice_bal].bal_id = bal_id;

        // si c'est OK, on met à jour le compteur
        if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, indice_bal) != ERROR_SUCCESS)
            return retour = __LINE__;

    }
    __finally
    {
        if (retour != 0)
        {
#ifdef ERREURS
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "PublieBAL() : nom_bal %s pour n°%d en echec ligne %d code %ld", 
                nom_bal, bal_id, retour, GetLastError());
#endif
            ExitBad();
        }
    }
    
    return bal_id;
}

PUBLIC noyau_bal_id DonneIdBAL (char *nom_bal)
{
    noyau_bal_id bal_id;
    
    // recherche une entrée dans la registry avec le nom de publication de la BAL
    if (REG_Lire_Entier (BAL_HKEY, BAL_SUB_KEY, nom_bal, &bal_id) != ERROR_SUCCESS)
        bal_id = 0;
    
    return bal_id;
}

PUBLIC noyau_enum_retour DonneNomBAL (noyau_bal_id bal_id, char *nom_bal)
{
    int id;

    id = DonnePositionDonneesBAL (bal_id);

    // teste validité du bal_id (bal 0 reservee aux chronos)
    if (id <= 0 || id >= MAX_MAILSLOTS)
        return NOYAU_FAUX;
    
    // teste si BAL existe dans la registry
    if (DonneIdBAL(_NOYAU_.sMailslotInfos[id].BalName) != bal_id)
        return NOYAU_FAUX;

    // recopie du nom
	strncpy_s(nom_bal, MAX_PATH, _NOYAU_.sMailslotInfos[id].BalName, MAX_PATH);

    return NOYAU_VRAI;
}

PUBLIC noyau_bal_id PublieBAL (char *nom_bal, unsigned long nb_messages_max)
{
    CHAR string[_MAX_PATH];
    noyau_bal_id bal_id;
    int retour;
    int id;

    EnterRegion();
    
    __try
    {
        retour = 0;
        
        // on teste si la BAL a déjà été publiée  
        bal_id = DonneIdBAL (nom_bal);
        
        // si n'existe pas
        if (bal_id == 0)
        {
            // publie la BAL  
            bal_id = NouveauNomBAL (nom_bal);
            if (bal_id == 0)
                return retour = __LINE__;
        }
        else
            return retour = __LINE__;
        
        // on recupere l'indice des donnees de la BAL dans le tableau des mailslots
        id = DonnePositionDonneesBAL(bal_id);
        if ( id <= 0 || id >= MAX_MAILSLOTS)
            return retour = __LINE__;

        // si mailslot déjà créé, problème
        if (_noyau_.hMailslotRead[id] != INVALID_HANDLE_VALUE)
            return retour = __LINE__;
        
        // on construit le nom de publication local du mailslot
		_snprintf_s(string, _MAX_PATH, _MAX_PATH, BAL_SLOT_NAME, ".", nom_bal);
        
        // on tente sa création
        _noyau_.hMailslotRead[id] = CreateMailslot (string,
            MAX_TAILLE_MESSAGE,
            MAILSLOT_WAIT_FOREVER,
            NULL);
        
        // teste la création du mailslot
        if (_noyau_.hMailslotRead[id] == INVALID_HANDLE_VALUE)
            return retour = __LINE__;
        
        // Init des variables partagees
        _NOYAU_.sMailslotInfos[id].dwNbMessagesMax = NOYAU_BAL_ILLIMITEE; //nb_messages_max;
        _NOYAU_.sMailslotInfos[id].dwMessageCount = 0L;
        _NOYAU_.sMailslotInfos[id].dwSizeCount = 0L;
        
#ifdef TRACES
        RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "PublieBAL() :" FMT_TACHE_NOM_ID_MESS, 
            GetCurrentThreadId(), nom_bal, bal_id, nb_messages_max);
#endif
        
    }
    __finally
    {
        if (retour != 0)
        {
#ifdef ERREURS
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "PublieBAL() : en echec ligne %d code %ld" FMT_TACHE_NOM_ID_MESS,
                retour, GetLastError(), GetCurrentThreadId(), nom_bal, bal_id, nb_messages_max); 
#endif
            
            ExitBad();
        }
    }
    
    LeaveRegion();
    
    return bal_id;
}

PUBLIC noyau_bal_id AttendBALTantQue (char *nom_bal, noyau_delai delai)
{
    noyau_bal_id bal_id;
    CHAR string[_MAX_PATH];
    DWORD retour = ERROR_SUCCESS;
    int id;

    // delai en ticks -> * 55 ms
    RESIZE_TIMEOUT(delai);
    
    EnterRegion();    
    
#ifdef TRACES
    RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "AttendBALTantQue(%d) : Recherche" FMT_TACHE_NOM, 
        delai, GetCurrentThreadId(), nom_bal);
#endif

    // recherche le nom de la BAL dans la registry
    bal_id = DonneIdBAL (nom_bal);
    
    // on boucle indéfiniment tant que la BAL n'a pas été publiée
    do
    {
        // si pas trouvé, alors attend notification de changement de la clé dans la registry
        if (bal_id == 0)
        {
            // ne pas mettre d'attente infinie sur la notification
            // car peut avoir été publié entre la première
            // recherche et l'attente (sur le LeaveRegion) d'ou blocage
            // si l'attente est infinie, on n'attend que 2 secondes mais on ne sortira
            // pas de la boucle si la BAL n'a pas ete publiee (cas du blocage
            // si la BAL est publiée après le DonneIdBAL() ci-desssus et avant
            // le REG_Attendre_Changement() qui suit)
            LeaveRegion();
            if (delai==INFINITE)
                REG_Attendre_Changement (BAL_HKEY, BAL_SUB_KEY, TRUE, 2000); 
            else
                retour = REG_Attendre_Changement (BAL_HKEY, BAL_SUB_KEY, TRUE, delai); 
            EnterRegion();
        }

        // recherche le nom de la BAL dans la registry
        bal_id = DonneIdBAL (nom_bal);
    }    
    while (retour != ERROR_TIMEOUT && bal_id ==0);

    // test si time out
    if (bal_id == 0)
    {
#ifdef TRACES
    RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "AttendBALTantQue(TIMEOUT) :" FMT_TACHE_NOM, 
        GetCurrentThreadId(), nom_bal);
#endif
    }
    else
    {
        // on recupere l'indice des donnees de la BAL dans le tableau des mailslots
        id = DonnePositionDonneesBAL(bal_id);

        // test validité du id
        if (id <= 0 || id >= MAX_MAILSLOTS)
        {
#ifdef ERREURS
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "AttendBAL() : echec code %ld" FMT_TACHE_NOM, 
                GetLastError(), GetCurrentThreadId(), nom_bal);
#endif
            ExitBad();
        }

        // test si le mailslot à déjà été ouvert dans ce process
        if (_noyau_.hMailslotWrite[id] == INVALID_HANDLE_VALUE)
        {
            // on construit le nom de publication du mailslot   
			_snprintf_s(string, _MAX_PATH , _MAX_PATH, BAL_SLOT_NAME, ".", nom_bal);
            
            // puis on tente son ouverture
            _noyau_.hMailslotWrite[id] = CreateFile (string, 
                GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, 
                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
            
            // teste l'ouverture du mailslot
            if (_noyau_.hMailslotWrite[id] == INVALID_HANDLE_VALUE)
            {
#ifdef ERREURS
                RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "AttendBAL() : echec code %ld" FMT_TACHE_NOM, 
                    GetLastError(), GetCurrentThreadId(), nom_bal);
#endif
                ExitBad();
            }
        }

#ifdef TRACES
        RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "AttendBAL() :" FMT_TACHE_NOM_ID_MESS, 
            GetCurrentThreadId(), nom_bal, bal_id, _NOYAU_.sMailslotInfos[id].dwNbMessagesMax);
#endif

    }
    
    LeaveRegion();
    
    return bal_id;
}

PUBLIC noyau_bal_id AttendBAL (char *nom_bal)
{
    return AttendBALTantQue (nom_bal, INFINITE);
}

PRIVATE noyau_enum_booleen TesteBAL (int id)
{
    CHAR string[_MAX_PATH];
    
    // teste validité du bal_id (bal 0 reservee aux chronos)
    if (id <= 0 || id >= MAX_MAILSLOTS)
        return NOYAU_FAUX;
    
    // teste si le mailslot à déjà été ouvert dans ce process
    if (_noyau_.hMailslotWrite[id] != INVALID_HANDLE_VALUE)
        return NOYAU_VRAI;
    
    // teste si BAL existe dans la registry
    if (DonneIdBAL(_NOYAU_.sMailslotInfos[id].BalName) != _NOYAU_.sMailslotInfos[id].bal_id)
        return NOYAU_FAUX;

    // on construit le nom de publication du mailslot   
	_snprintf_s(string, _MAX_PATH, _MAX_PATH, BAL_SLOT_NAME, ".", _NOYAU_.sMailslotInfos[id].BalName);
    
    // puis on tente son ouverture
    _noyau_.hMailslotWrite[id] = CreateFile (string, 
        GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, NULL, 
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    // teste l'ouverture du mailslot
    if (_noyau_.hMailslotWrite[id] == INVALID_HANDLE_VALUE)
        return NOYAU_FAUX;
    
    return NOYAU_VRAI;
}   

PUBLIC noyau_enum_retour SupprimeBAL (char *nom_bal)
{
    noyau_bal_id bl_id;
    noyau_bal_id bl_cnt;
    int id;

    // teste si la bal existe et recupère son ID
    bl_id = DonneIdBAL(nom_bal);

    if (bl_id == 0)
        return NOYAU_NOK;

    /* Recuperer la position des donnees de la BAL dans le tableau des mailslots */
    id = DonnePositionDonneesBAL(bl_id);
    // test validité du bal_id
    if (id <= 0 || id >= MAX_MAILSLOTS)
        return NOYAU_NOK;

    EnterRegion();

    // on n'essaie pas d'effacer les handles en ecriture
    // sur ce mailslot parce qu'ils peuvent être de toute facons répartis
    // dans plusieurs process
    // Logiquement, seul le publicateur de la BAL devrait avoir à la supprimer
    // mais il est impossible de savoir qui demande la supression pour vérifier
    CloseHandle (_noyau_.hMailslotRead[id]);
    _noyau_.hMailslotRead[id] = INVALID_HANDLE_VALUE;

    // tente d'effacer la cle dans la registry
    // sans faire de test car peut être deja effacé
    REG_Efface_Cle (BAL_HKEY, BAL_SUB_KEY, nom_bal);

    // supression du nom de la BAL et de son identificateur
    _NOYAU_.sMailslotInfos[id].BalName[0] = '\0';
    _NOYAU_.sMailslotInfos[id].bal_id = 0;

    // on récupère le compteur du nombre de bal publiées
    if (REG_Lire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, &bl_cnt) != ERROR_SUCCESS)
    {
        LeaveRegion();
        return NOYAU_NOK;
    }

    // on décrémente ce nombre de BAL
    bl_cnt --;
    
    // on remet à jour le compteur dans le registre
    if (REG_Ecrire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, bl_cnt) != ERROR_SUCCESS)
    {
        LeaveRegion();
        return NOYAU_NOK;
    }

    LeaveRegion();

#ifdef TRACES
    RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "SupprimeBAL() :" FMT_TACHE_NOM, 
        GetCurrentThreadId(), nom_bal);
#endif
    
    return NOYAU_OK;
}

PUBLIC noyau_enum_retour StatistiquesBAL (noyau_bal_id id_bal,
                                          size_t size,
                                          noyau_bal_stats *stats_bal)
{
    noyau_enum_retour retour = NOYAU_OK;
    int bal = 0;
    int balmax = 0;
    unsigned short use_rate = 0;

    if (stats_bal == NULL)
    {
        return NOYAU_NOK;
    }

    EnterRegion();


    // If id_bal is 0, return the stats of the most used bal
    if (id_bal == 0)
    {
        for (bal=1; bal<MAX_MAILSLOTS; bal++)
        {
            if ( _NOYAU_.sMailslotInfos[bal].bal_id != 0 )
            {
                if ((balmax == 0) ||
                    (_NOYAU_.sMailslotInfos[bal].dwMessageCount > 
                     _NOYAU_.sMailslotInfos[balmax].dwMessageCount))
                {
                    balmax = bal;
                }
            }
        }
        bal = balmax;

        // Check that at least one bal was found
        if (bal == 0)
        {
            retour = NOYAU_BAL_STOP;
        }
    }
    else
    {
        bal = DonnePositionDonneesBAL(id_bal);
        if (TesteBAL (bal) == NOYAU_FAUX)
        {
            retour = NOYAU_BAL_STOP;
        }
    }

    // The bal index is known, copy the values in the returned structure
    if (retour == NOYAU_OK)
    {
        stats_bal->dwMessageCount = _NOYAU_.sMailslotInfos[bal].dwMessageCount;
        stats_bal->dwSizeCount = _NOYAU_.sMailslotInfos[bal].dwSizeCount;
        stats_bal->dwNbMessagesMax = _NOYAU_.sMailslotInfos[bal].dwNbMessagesMax;
        stats_bal->bal_id = _NOYAU_.sMailslotInfos[bal].bal_id;
    }
    
    LeaveRegion();    
    
    TestsPeriodiques ();
    
    return retour;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour Envoie (unsigned char bal,
*                                           unsigned char bal_retour,
*                                           struct_neutre *p_neutre)
* PARAMETRES: - numero de boite aux lettres destinatrice
*             - numero de la boite aux lettres de retour
*             - pointeur sur structure de type neutre
* RETOUR:
*             - NOYAU_BAL_PLEINE : BAL pleine
*             - NOYAU_OK : Envoie du message Ok
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui envoie un message dans une boite aux lettres. Elle
*       met egalement a jour le champ <bal_id> et <bal_retour> de la structure neutre.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour Envoie (noyau_bal_id id_bal,
                                 noyau_bal_id bal_retour,
                                 struct_neutre *p_neutre)
{
    noyau_enum_retour retour;
    DWORD written;
    int id;

    EnterRegion();

    /* Mise a jour du numero de la BAL destinatrice */
    p_neutre->bl_id = id_bal;
    
    /* Mise a jour du numero de la BAL emettrice */
    p_neutre->bl_retour = bal_retour;
    
    /* Forcer la valeur de retour a pleine */
    retour = NOYAU_BAL_STOP;

    /* Recuperer la position des donnees de la BAL dans le tableau des mailslots */
    id = DonnePositionDonneesBAL(id_bal);
    
    /* teste si le n° de Bal est valide et si un handle de mailslot y est associé
    et sinon tente de le créer */  
    if (TesteBAL (id) == NOYAU_FAUX)
    {
#ifdef TRACES
        if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
        RtcFichierTrace (NOY_BAL, &_noyau_.dbg, "Envoie() : echec bal inconnue" FMT_NEUTRE, p_neutre, GetCurrentThreadId(), 
            _NOYAU_.sMailslotInfos[id].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName);
#endif
        LeaveRegion();
        return retour;
    }

// Block removed so the Envoie can be used with chronos
//     if (bal_retour == 0)
//     {
// #ifdef TRACES
//         if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
//         RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "Envoie() : echec bal retour inconnue" FMT_NEUTRE, p_neutre, GetCurrentThreadId(), 
//             _NOYAU_.sMailslotInfos[id].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName);
// #endif
//         LeaveRegion();
//         return retour;
//     }
    
//     // si le nombres de messages pending sont limités...
//     if (_NOYAU_.sMailslotInfos[id].dwNbMessagesMax > NOYAU_BAL_ILLIMITEE && 
//         _NOYAU_.sMailslotInfos[id].dwMessageCount >= _NOYAU_.sMailslotInfos[id].dwNbMessagesMax)
//     {
// #ifdef TRACES
//         if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
//             RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "Envoie() : echec bal destination pleine" FMT_NEUTRE, p_neutre, GetCurrentThreadId(), 
//                 _NOYAU_.sMailslotInfos[id].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName);
// #endif
//         LeaveRegion();    
//         return NOYAU_BAL_PLEINE;
//     }

    /* recopie du buffer dans le mailslot */
    if (WriteFile (_noyau_.hMailslotWrite[id], p_neutre, 
        DonneTailleBloc (p_neutre),
        &written, NULL))
    {
        /* Forcer la valeur de retour message present */
#ifdef TRACES
        if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
            RtcFichierTrace (NOY_BAL,
                             &_noyau_.dbg,
                             "Envoie() :" FMT_NEUTRE " taille=%d, envoye=%d",
                             p_neutre,
                             GetCurrentThreadId(),
                             _NOYAU_.sMailslotInfos[id].BalName,
                             _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName,
                             DonneTailleBloc (p_neutre),
                             written);
#endif
        /* libération du buffer apres ecriture dans le mailslot */
        ExitLibere (&p_neutre);
        
//         InterlockedIncrement(&(_NOYAU_.sMailslotInfos[id].dwMessageCount));
//         InterlockedExchangeAdd(&(_NOYAU_.sMailslotInfos[id].dwSizeCount),
// 							   written);
        
        retour = NOYAU_OK;
    }
    else
    {
        /* fermeture du handle (si possible) */
        CloseHandle(_noyau_.hMailslotWrite[id]);
        _noyau_.hMailslotWrite[id] = INVALID_HANDLE_VALUE;

        /* generation d'un nouveau handle */
        if (TesteBAL (id) == NOYAU_FAUX)
        {
#ifdef TRACES
            if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
              RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "Envoie() : echec bal inconnue" FMT_NEUTRE, p_neutre, GetCurrentThreadId(), 
                  _NOYAU_.sMailslotInfos[id].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName);
#endif
           LeaveRegion();
           return retour;
        }

        /* 2eme tentative d'ecriture du message dans le mailslot */
        if (WriteFile (_noyau_.hMailslotWrite[id], p_neutre, 
                       DonneTailleBloc (p_neutre),
                       &written, NULL))
        {
            /* Forcer la valeur de retour message present */
#ifdef TRACES
            if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
               RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "Envoie(2eme tentative) :" FMT_NEUTRE, p_neutre, GetCurrentThreadId(), 
                        _NOYAU_.sMailslotInfos[id].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName);
#endif
            /* libération du buffer apres ecriture dans le mailslot */
            ExitLibere (&p_neutre);
        
//             InterlockedIncrement(&(_NOYAU_.sMailslotInfos[id].dwMessageCount));
//             InterlockedExchangeAdd(&(_NOYAU_.sMailslotInfos[id].dwSizeCount),
// 								   written);
        
            retour = NOYAU_OK;
        }
        else
        {
#ifdef TRACES
            if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
               RtcFichierDebug (
                    NOY_BAL,
                    &_noyau_.dbg,
                    "Envoie() :" FMT_NEUTRE FMT_STATUS" en echec ecriture 2eme tentative",
                    p_neutre,
                    GetCurrentThreadId(), 
                    _NOYAU_.sMailslotInfos[id].BalName,
                    _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName,
                    GetLastError());
#endif
        }
    }
    
    LeaveRegion();    
    
    TestsPeriodiques ();
    
    /* Retour du compte rendu du Send */
    return retour;
}


PUBLIC void DEFINE_ExitEnvoie (const char *file,
                               int line,
                               noyau_bal_id bal,
                               noyau_bal_id bal_retour,
                               struct_neutre *p_neutre)
{
    noyau_enum_retour retour;
    retour = Envoie (bal, bal_retour, p_neutre);
    if (retour != NOYAU_OK)
    {
#ifdef ERREURS
        RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "ExitEnvoie(%s -> %s) :" FMT_NEUTRE ", retour = %d, "FMT_STATUS,
            _NOYAU_.sMailslotInfos[bal_retour].BalName, _NOYAU_.sMailslotInfos[bal].BalName, 
            p_neutre, GetCurrentThreadId(), _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal)].BalName,
            _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName, retour, GetLastError());
#endif
        DEFINE_ExitBad (file, line);
    }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour TestRecoit (unsigned char bal,
*                                               struct_neutre **pp_neutre)
* PARAMETRES: - numero de boite aux lettres
*             - pointeur de pointeur sur structure de type neutre
* RETOUR:
*             - NOYAU_BALL_VIDE : BAL vide
*             - NOYAU_BALL_MESS : message pr‚sent dans la BAL
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui tente de recevoir un message dans une boite aux lettres.
*       REMARQUE : Si un message est present dans la B.A.L., il est lu.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour TestRecoit (noyau_bal_id bal,
                                     struct_neutre **pp_neutre)
{
    noyau_enum_retour status;   /* valeur de retour du receive */
    LONG lu;
    DWORD a_lire, nb_messages;
#ifdef TRACES
    DWORD *dump;
#endif
    int id;

    status = NOYAU_BAL_VIDE;

    /* Recuperer la position des donnees de la BAL dans le tableau des mailslots */
    id = DonnePositionDonneesBAL(bal);
    // test validité du bal_id
    if (id <= 0 || id >= MAX_MAILSLOTS)
        return NOYAU_BAL_STOP;
    
    /* teste si des données sont prêtes dans le mailslot */
    if (!GetMailslotInfo (_noyau_.hMailslotRead[id], NULL, &a_lire, &nb_messages, NULL)) 
        return NOYAU_BAL_STOP;
    
    if (nb_messages > 0)
    {
        // teste d'un cas foireux d'acces concurrent en lecture du mailslot
        // qui fait que a_lire = 0 && nb_messages = 1...
        if (a_lire == 0)
        {
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "TestRecoit(%lu) : en echec de lecture concurrent", bal);
            ExitBad();
        }

        /* allocation d'un buffer de la taille des données à lire dans le tas du process */
        ExitAlloue (pp_neutre, a_lire, GetProcessHeap());
        
        /* recopie des données dans le buffer par le mailslot */
        if (!ReadFile (_noyau_.hMailslotRead[id], *pp_neutre, a_lire, &lu, NULL))
        {
#ifdef ERREURS
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "TestRecoit() :" FMT_NEUTRE "en echec de lecture",
                     *pp_neutre, 
                     GetCurrentThreadId(),
                     _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_id)].BalName, 
                     _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_retour)].BalName);
#endif
            return NOYAU_BAL_STOP;
        }

// 	EnterRegion();
//         InterlockedDecrement(&(_NOYAU_.sMailslotInfos[id].dwMessageCount));
//         InterlockedExchangeAdd(&(_NOYAU_.sMailslotInfos[id].dwSizeCount), -lu);
// 	LeaveRegion();
        
        status = NOYAU_BAL_MESS;
#ifdef TRACES
        dump = (DWORD *) (*pp_neutre + 1);
        
        if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
            RtcFichierTrace(NOY_BAL, &_noyau_.dbg, "TestRecoit() :" FMT_NEUTRE FMT_DUMP,
                  *pp_neutre, 
                  GetCurrentThreadId(),
                  _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_id)].BalName, 
                  _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_retour)].BalName,
                  dump[0],
                  dump[1]);
#endif
    }
    
    TestsPeriodiques ();
    
    /*** Indiquer si message pr‚sent dans la BAL ***/
    return (status);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour Recoit (unsigned char bal,
*                                           struct_neutre **pp_neutre)
*                                           long int timeout);
* PARAMETRES: - numero de boite aux lettres
*             - pointeur de pointeur sur structure de type neutre
*             - temps max d'attente (unite = 55 ms) (si 0, timeout infini)
* RETOUR:
*             - NOYAU_BAL_MESS : message pr‚sent dans la BAL
*             - NOYAU_BAL_TIME  : sortie sur time out
*             - NOYAU_BAL_STOP  : sortie sur stop task
*             - NOYAU_NOK       : code retour RTC errone
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui tente de recevoir un message dans une boite aux lettres
*       jusqu'a <timeout> ticks horloge max.
*       REMARQUE : Cette fonction ne doit pas etre appelee dans une region
*                  critique.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour  Recoit (noyau_bal_id bal,
                                  struct_neutre **pp_neutre,
                                  noyau_delai timeout)
{
    RESIZE_TIMEOUT(timeout);   
	
	return RecoitMs(bal, pp_neutre, timeout);
}



PUBLIC noyau_enum_retour WINAPI RecoitMs (noyau_bal_id bal,
                                        struct_neutre ** pp_neutre,
                                        int iTimeoutMs)
{
    noyau_enum_retour status;   /* valeur de retour du receive */
    LONG lu;
    DWORD a_lire, nb_messages;
#ifdef TRACES
    DWORD *dump;
#endif
    int id;
    BOOL fResult = FALSE;
    char cDummy;

    
    status = NOYAU_BAL_VIDE;

    /* Recuperer la position des donnees de la BAL dans le tableau des mailslots */
    id = DonnePositionDonneesBAL(bal);
    // test validité du bal_id
    if (id <= 0 || id >= MAX_MAILSLOTS)
        return NOYAU_BAL_STOP;
    
    /* change le timeout en lecture du mailslot */
    if (!SetMailslotInfo (_noyau_.hMailslotRead[id], iTimeoutMs))
        return NOYAU_BAL_STOP;

    /* Attend message dans le mailslot */
    fResult = ReadFile (_noyau_.hMailslotRead[id], &cDummy, 0, &lu, NULL);
    
    if (fResult == TRUE)
    {
        RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "Recoit() :" FMT_NEUTRE "Reception inattendue d'un message vide",
            *pp_neutre, 
            GetCurrentThreadId(),
            _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal)].BalName, 
            _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(0)].BalName);
        ExitBad();
    }

    /* selon "l'erreur" */
    switch( GetLastError() )
    {
        /* message present */
    case ERROR_INSUFFICIENT_BUFFER:
        
        // Erreur volontaire traitée
        SetLastError (ERROR_SUCCESS);

        /* recupère la taille du message à lire */
        if (!GetMailslotInfo (_noyau_.hMailslotRead[id], NULL, &a_lire, &nb_messages, NULL)) 
            return NOYAU_BAL_STOP;

        // teste d'un cas foireux d'acces concurrent en lecture du mailslot
        if (a_lire == MAILSLOT_NO_MESSAGE || a_lire == 0 || nb_messages == 0)
        {
            RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "Recoit() :" FMT_NEUTRE "en echec de lecture concurrent. Nouveau message annonce mais taille du prochain message : %d, nombre de messages : %d",
                *pp_neutre, 
                GetCurrentThreadId(),
                _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal)].BalName, 
                _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(0)].BalName,
                a_lire,
                nb_messages);
            ExitBad();
        }
        
        /* allocation d'un buffer de la taille à lire dans le tas du process */ 
        ExitAlloue (pp_neutre, a_lire, GetProcessHeap());
        
        /* recopie dans le buffer par le mailslot */
        if (!ReadFile (_noyau_.hMailslotRead[id], *pp_neutre, a_lire, &lu, NULL))
        {
#ifdef ERREURS
              RtcFichierDebug(NOY_BAL, &_noyau_.dbg, "Recoit() :" FMT_NEUTRE "en echec de lecture",
                          *pp_neutre, 
                          GetCurrentThreadId(),
                          _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal)].BalName, 
                          _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(0)].BalName);
#endif
            return NOYAU_BAL_STOP;
        }
        
//    EnterRegion();
//         InterlockedDecrement(&(_NOYAU_.sMailslotInfos[id].dwMessageCount));
//         InterlockedExchangeAdd(&(_NOYAU_.sMailslotInfos[id].dwSizeCount), -lu);
//    LeaveRegion();
        
        status = NOYAU_BAL_MESS;
#ifdef TRACES
        dump = (DWORD *) (*pp_neutre + 1);
        
        if (DBG_FichierTracesPresent(NOY_BAL, &_noyau_.dbg))
             RtcFichierTrace(NOY_BAL, &_noyau_.dbg, "Recoit() :" FMT_NEUTRE FMT_DUMP ". Number of messages in mailslot : %d",
                     *pp_neutre, 
                     GetCurrentThreadId(),
                     _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_id)].BalName, 
                     _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL((*pp_neutre)->bl_retour)].BalName,
                     dump[0],
                     dump[1],
                     nb_messages);
#endif
        break;
        
    case ERROR_SEM_TIMEOUT:
        status = NOYAU_BAL_TIME; 
        break;
        
    default:
        status = NOYAU_BAL_STOP;
        break;
    }
    
    TestsPeriodiques ();
    
    /*** Indiquer si message pr‚sent dans la BAL ***/
    return (status);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PRIVATE noyau_bloc_id VideBal (noyau_bal_id bal)
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Vidage des BALs dans le fichier trace pour expertise
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE DWORD VideBal (noyau_bal_id bal)
{
    struct_neutre *p;
    DWORD n;
    
    RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "   BAL %02d =>", bal);
    
    n = 0 ;    // Nb messages
    while (TestRecoit(bal, &p) == NOYAU_BAL_MESS)
    {
        RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "      Message %03d =" FMT_NEUTRE,
                  ++n, p, GetCurrentThreadId(), 
                  _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(p->bl_id)].BalName, 
                  _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(p->bl_retour)].BalName);
        ExitLibere (&p);
    }
    
    return n;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC void far TraceBALs ()
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Vidage des BALs dans le fichier trace pour expertise
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void TraceBALs (void)
{
    WORD bal;
    DWORD total_count = 0 ;
    DWORD total_size = 0 ;
    DWORD nb_bals = 0;
    int id;
    
    EnterRegion();

    // inhibe les erreurs 
    DBG_DebutDebugSansErreurs (&_noyau_.dbg);
    
    // on récupère le compteur du nombre de bal publiées
    REG_Lire_Entier (BAL_HKEY, BAL_SUB_KEY, BAL_COUNTER, &nb_bals);
    
    RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "TraceBALs() : nb BALs = %02ld", nb_bals);
    
    for (bal=1; bal<MAX_MAILSLOTS; bal++)
    {
    /* teste si le n° de Bal est valide et si un handle de mailslot y est associé
        et sinon tente de le créer */  
        if ( _NOYAU_.sMailslotInfos[bal].bal_id != 0 )
        {
           id = DonnePositionDonneesBAL(_NOYAU_.sMailslotInfos[bal].bal_id);

           if (TesteBAL(id))
           {
               total_count += _NOYAU_.sMailslotInfos[id].dwMessageCount;
               total_size += _NOYAU_.sMailslotInfos[id].dwSizeCount;
            
               RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "   BAL %s (%02ld) => %lu messages (sur %lu) / %lu octets", 
                      _NOYAU_.sMailslotInfos[id].BalName, bal, _NOYAU_.sMailslotInfos[id].dwMessageCount,
                      _NOYAU_.sMailslotInfos[id].dwNbMessagesMax, _NOYAU_.sMailslotInfos[id].dwSizeCount);
           }
           else
           {
               RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "   BAL %s (%02ld) => n'est pas accessible (count %lu / size %lu)", 
                      _NOYAU_.sMailslotInfos[id].BalName, bal, _NOYAU_.sMailslotInfos[id].dwSizeCount);
           }
        }
        else
        {
        }
    }
    
    RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "   Nb total messages = %02lu / total taille = %02lu octets", total_count, total_size);
    
    DBG_FinDebugSansErreurs(&_noyau_.dbg);
    
    LeaveRegion();
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC HANDLE DonneHandleEcritureBAL (noyau_bal_id bal_id)

*
* PARAMETRES: - numero de boite aux lettres concernée
*
* RETOUR:
*             - INVALID_HANDLE_VALUE si NOK
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: Renvoie un handle en écriture sur la BAL à utiliser avec 
* la fonction FastEnvoie()
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC HANDLE DonneHandleEcritureBAL (noyau_bal_id bal_id)
{
    int id;

    /* Recuperer la position des donnees de la BAL dans le tableau des mailslots */
    id = DonnePositionDonneesBAL(bal_id);

    /* teste si le n° de Bal est valide et si un handle de mailslot y est associé
    et sinon tente de le créer */  
    if (TesteBAL (id) == NOYAU_FAUX)
        return NULL;

    /* renvoie le handle valide ou non */
    return (PVOID) id;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC noyau_enum_retour FastEnvoie (HANDLE hBalEcriture,
*                                    noyau_bal_id bal_id,
*                                   noyau_bal_id bal_retour,
*                                    struct_neutre *p_neutre,
*                                    noyau_taille_bloc taille)
*
* PARAMETRES: - handle du mailslot en écriture
*             - numero de boite aux lettres destinatrice
*             - numero de la boite aux lettres de retour
*             - pointeur sur structure de type neutre
*             - taille du bloc à recopier
* RETOUR:
*             - NOYAU_BAL_STOP : Echec d'écriture
*             - NOYAU_OK : Envoie du message Ok
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: ATTENTION contrairement à la fonction Envoie classique :
* - pas de region impliquée 
* - p_neutre n'est plus un bloc dynamique libéré par la fonction
* - le nombre de messages envoyé dans une BAL n'est plus limité
* - il n'y a plus de traces, ni de debug
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC noyau_enum_retour FastEnvoie (HANDLE hBalEcriture,
                                     noyau_bal_id bal_id,
                                     noyau_bal_id bal_retour,
                                     struct_neutre *p_neutre,
                                     noyau_taille_bloc taille)
{
    DWORD written;
    DWORD id = (DWORD) hBalEcriture;

    /* Mise a jour du numero de la BAL destinatrice */
    p_neutre->bl_id = bal_id;
    
    /* Mise a jour du numero de la BAL emettrice */
    p_neutre->bl_retour = bal_retour;

    /* pour debug bl_retour à zero
    if (bal_retour == 0 && GetCurrentThreadId() != _noyau_.ThreadChronoId)
        RtcFichierDebug (NOY_BAL, &_noyau_.dbg, "FastEnvoie() : echec bal retour inconnue" FMT_NEUTRE " process = %d", p_neutre, GetCurrentThreadId(), 
            _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_id)].BalName, _NOYAU_.sMailslotInfos[DonnePositionDonneesBAL(bal_retour)].BalName,
            GetCurrentProcessId());
    */

    /* recopie du buffer dans le mailslot */
    if (WriteFile (_noyau_.hMailslotWrite[id], 
                   p_neutre, 
                   taille,
                   &written, 
                   NULL))
    {
//         InterlockedIncrement(&(_NOYAU_.sMailslotInfos[id].dwMessageCount));
//         InterlockedExchangeAdd(&(_NOYAU_.sMailslotInfos[id].dwSizeCount),
// 							   written);

        return NOYAU_OK;
    }
    
    return NOYAU_BAL_STOP;
}
 


