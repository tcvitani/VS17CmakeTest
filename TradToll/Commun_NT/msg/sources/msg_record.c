/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: 
* FICHIER: 
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <msg_loc.h>

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

struct MSG_Record
{
    IN  MSG_Compare compare_func;
    IN  PVOID param;
    union MSG_Field *p_field;   // description des champs du message
    DWORD size_of_message;      // taille de la structure du message
    CHAR name[MSG_NAME_MAX];    // nom du message pour traces
};

struct MSG_Compare_Record
{
    IN  BYTE *buffer;            
    IN  DWORD size_max;
    OUT HMSG hMsg;
};

/*--------------- VARIABLES: ---------------*/

PRIVATE HLIST hRecord;

/*--------------- FUNCTIONS: ---------------*/

PRIVATE BOOL MSG_Duplicate_Recurse (HMSG hMsg, HMSG hRootMsg);

/*--------------- CODE: ---------------*/    

// RECORD

// Une fonctionnalité du module MSG est de pouvoir rechercher un type de message connu 
// contenu dans un buffer quelconque et d'en extraire automatiquement une strucutre de données
// qui correspond.
// Pour cela chaque module de message doit s'enregistré aupres du moteur.
// Par défaut l'enregistrement est fait dans le dll main de chaque ddl de message 
// en passant un identifiant de liste d'enregistrement NULL. Dans ce cas une liste interne et
// regroupant toute les DLL est créée par défaut par MSG.
// En s'enregistrrant la DLL de message doit fournir une fonction de comparaison 
// et une fonction de traitement du buffer retournant une structure remplie.

// enregistrement d'un type de message auprès du module MSG
PUBLIC BOOL MSG_New_Record (HLIST *hList, 
                            MSG_Compare compare_func, 
                            PVOID param, 
                            union MSG_Field *p_field, 
                            DWORD size_of_message,
                            CHAR name[MSG_NAME_MAX])
{
    struct MSG_Record *p_record;

    // si une liste n'est pas fournie, prendre la liste interne par défaut
    if (hList == NULL)
        hList = &hRecord;

    // si la liste n'est pas initialisée, le faire
    if (*hList == NULL)
        *hList = List_New();

    // Créer un nouvel element
    p_record = List_ItemNew (*hList, sizeof(struct MSG_Record));
    if (p_record == NULL)
        return FALSE;

    // Initialiser l'item
    memset (p_record, 0, sizeof(struct MSG_Record));
    p_record->compare_func = compare_func; // renvoie TRUE si le type de message est reconnu
    p_record->param = param;               // generalement le CD du message 
    p_record->p_field = p_field;           // tableau de conversion du message
    p_record->size_of_message = size_of_message; // taille de la structure du message
    if (name != NULL)
        strncpy_s (p_record->name, MSG_NAME_MAX, name, MSG_NAME_MAX-1);

    // insertion de l'item record en queue de liste
    if (List_AddTail (*hList, p_record) == FALSE)
    {
        List_ItemDelete (*hList, &p_record);
        return FALSE;
    }

    return TRUE;
}

PUBLIC BOOL MSG_Delete_All_Records (HLIST *hList)
{
    // si une liste n'est pas fournie, prendre la liste interne par défaut
    if (hList == NULL)
        hList = &hRecord;

    // si la liste n'est pas initialisée, rien a faire
    if (*hList == NULL)
        return TRUE;

    // supprimmer toute les items et le handle de liste
    return List_DeleteAll (hList, NULL, NULL);
}

PRIVATE BOOL MSG_Find_Record (struct MSG_Record *p_record, struct MSG_Compare_Record *p_compare)
{
    // on appelle la fonction de comparaison en lui passant le message et son paramètre
    // si cette fonction n'est pas définie, le premier (et unique ?) type de message
    // est utilisé...
    if (p_record->compare_func == NULL ||
        p_record->compare_func (p_compare->buffer, p_record->param) == TRUE)
    {
        // si le message correspond, on crée une structure
        p_compare->hMsg = MSG_New (p_record->p_field, p_record->size_of_message, p_record->name);

        // Trouvé ou pas, on arrete le parcours
        return FALSE;
    }

    // ca ne correspond pas, on passe au Record suivant
    return TRUE;
}

// fonction de recherche d'un message connu et création d'une structure du meme type
PUBLIC HMSG MSG_New_Read_If_Found (HLIST hList, BYTE *p_msg, DWORD msg_size_max)
{
    struct MSG_Compare_Record compare;

    // si une liste n'est pas fournie, prendre la liste interne par défaut
    if (hList == NULL)
        hList = hRecord;

    // si la liste n'est pas initialisée, erreur !
    if (hList == NULL)
        return NULL;

    // préparation de la structure de recherche
    compare.buffer = p_msg;
    compare.size_max = msg_size_max;
    compare.hMsg = NULL;
    
    // si renvoie TRUE alors on a parcouru toute la liste et on a pas trouvé, hMsg reste NULL
    // sinon renvoie FALSE, on s'est arrete dans le parcours, 
    // on a trouvé, hMsg est rempli, ca a foiré hMsg est NULL
    List_ForEachItem (hList, MSG_Find_Record, &compare);

    // si un message est reconnu, on essaye d'en extraire une structure de données
    if (compare.hMsg != NULL)
        if (MSG_Read (compare.hMsg, compare.buffer, compare.size_max) == FALSE)
            MSG_Delete_All (&compare.hMsg);

	return compare.hMsg;
}


