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

/*--------------- VARIABLES: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

PRIVATE BOOL MSG_Delete_List (HLIST *hList, MSG_Delete_Func delete_func, PVOID param);
PRIVATE BOOL MSG_Delete_Recurse (HMSG hMsg, PVOID param);

/*--------------- CODE: ---------------*/    

// CONSTRUCTEURS / DESTRUCTEURS

// création d'un message (Root)
PUBLIC HMSG MSG_New (union MSG_Field *p_field, DWORD size_of_message, CHAR name[MSG_NAME_MAX])
{           
    struct MSG_Header *p_header;
    
    // on fait précéder la structure de données allouée par un entete
    // caché de l'utilisateur
    if ((p_header = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(struct MSG_Header) + size_of_message)) == NULL)
        return NULL;

//    memset (p_header, 0, sizeof(struct MSG_Header) + size_of_message);
 
    // cet entete contiendra entre autres un lien vers la structure de description
    // des champs du message et la taille du message
    p_header->p_field = p_field;
    p_header->size_of_message = size_of_message;
    if (name != NULL)
        strncpy_s (p_header->name, MSG_NAME_MAX, name, MSG_NAME_MAX-1);

    // on retourne l'@ de la zone de données utilisateur
    // a lui de remplir les champs (et de construire les listes)
    return MSG_GET_DATA (p_header);
}

// création d'une liste attachée à un message (champ de type liste)
PUBLIC HMSG MSG_New_List (HLIST *hList, union MSG_Field *p_field, DWORD size_of_message)   
{
    struct MSG_Header *p_header;

    // allocation automatique du handle de liste
    if (*hList == NULL)
        *hList = List_New();

    // création d'un nouvel element avec un entete identique a celui d'un message
    p_header = List_ItemNew (*hList, sizeof(struct MSG_Header) + size_of_message);
    if (p_header == NULL)
        return NULL;

    memset (p_header, 0, sizeof(struct MSG_Header) + size_of_message);
    // un element de liste est considérer comme un sous-message
    // avec un entete dont seul le champ p_field est utilisé
    p_header->p_field = p_field;
    p_header->size_of_message = size_of_message;

    // ajout de l'element en queue de liste (FIFO avec extraction en tete)
    if (List_AddTail (*hList, p_header) == FALSE)
    {
        List_ItemDelete (*hList, &p_header);
        return NULL;
    }

    // retourne l'@ de la zone de données utilisateur
    return MSG_GET_DATA (p_header);
}

PUBLIC BOOL MSG_Delete_All_List (HLIST *hList)
{
    return MSG_Delete_List (hList, MSG_Delete_Recurse, NULL);
}

// suppression d'une liste entiére et de tous ses elements
PRIVATE BOOL MSG_Delete_List (HLIST *hList, MSG_Delete_Func delete_func, PVOID param)
{
    BOOL ret;
    struct MSG_Header *p_header;

    // si la liste n'a pas ete créé
    // il n'y rien a faire, c'est OK
    if (*hList == NULL)
        return TRUE;
    
    // Sinon, pour chaque item...
    ret = TRUE;
    while ((p_header = List_RemoveHead(*hList)) != NULL)
    {
        // si une fonction de destruction est fournie
        if (delete_func != NULL)
        {
            // lui passer les données utilisateur
            if (delete_func (MSG_GET_DATA(p_header), param) == FALSE)
                ret = FALSE; // en cas d'erreur, on n'arrete pas mais on mémorise
        }

        // destruction de l'element
		List_ItemDelete (*hList, &p_header);
    }

    // destruction du handle de liste
    List_Delete (hList);

    return ret;
}

// Supression d'un message entier
PUBLIC BOOL MSG_Delete_All (HMSG *hMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(*hMsg);
    BOOL ret;

    // par précaution...
    if (hMsg == NULL || *hMsg == NULL)
        return FALSE;

    // on appelle une fonction de destruction récursive (sur les champs de type liste)
    // qui scrute champ par champ si des actions sont a effectuées avant destruction
    // (cas des types de champs Custom et Liste)
    ret = MSG_Delete_Recurse (*hMsg, NULL);

    // finalement on libére la mémoire
    *hMsg = NULL;
    HeapFree (GetProcessHeap(), 0, p_header);

    return ret;
}

// destruction d'un message champ par champ
PRIVATE BOOL MSG_Delete_Recurse (HMSG hMsg, PVOID param)
{
    BYTE *p_struct = hMsg;
    union MSG_Field *p_field = MSG_GET_HEADER(hMsg)->p_field;
    DWORD f;
	HMSG *p_msg;
    BOOL ret;

    f = 0;
    ret = TRUE; 
    // on s'arrete sur un champ de type Stop ou sur erreur
    while (p_field[f].Stop.type != MSG_FIELD_STOP && ret == TRUE)
    {
        switch (p_field[f].Stop.type)
        {
        case MSG_FIELD_STOP:
            ret = FALSE; // on ne devrait pas arriver ici !
            break;
    
        case MSG_FIELD_LIST:
        case MSG_FIELD_LIST_HEX:
            // suppression d'une liste par appel récursif de cette fonction
            ret = MSG_Delete_List ((HLIST *) (p_struct + p_field[f].List.offset),
                                   MSG_Delete_Recurse,
                                   param);
            break;

        case MSG_FIELD_CUSTOM:
            // appel de la fonction utilisateur si dispo
            if (p_field[f].Custom.delete_func != NULL)
                ret = p_field[f].Custom.delete_func ((p_struct + p_field[f].Custom.offset),
                                                      param);
            break;

        case MSG_FIELD_INCLUDE:
            // appel récursif de la fonction
            // on rentre en récursion sur un nouveau message
			p_msg = (HMSG *)(p_struct + p_field[f].Include.offset);
			if (*p_msg == NULL)
				ret = FALSE;
			else
				ret = MSG_Delete_All (p_msg);
            break;

        default:
            break;
        }

        f++;
    }

    return ret;
}
