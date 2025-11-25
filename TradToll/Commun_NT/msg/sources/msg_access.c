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

PRIVATE BOOL MSG_Duplicate_Recurse (HMSG hMsg, HMSG hRootMsg);

/*--------------- CODE: ---------------*/    

// ACCES

// récupère l'offset courant
// en fin de lecture/ecriture, correspond a la taille lu ou ecrite dans le buffer
PUBLIC DWORD MSG_Get_Offset (HMSG hMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    return p_header->offset;
}

PUBLIC CHAR * MSG_Get_Name (HMSG hMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    return p_header->name;
}

PUBLIC BOOL MSG_Is_Fixed_Size (union MSG_Field *p_field)
{
    DWORD f;
    BOOL ret;

    f = 0;
    ret = TRUE; 
    while (p_field[f].Stop.type != MSG_FIELD_STOP && ret == TRUE)
    {
        switch (p_field[f].Stop.type)
        {
        case MSG_FIELD_FLOAT:
        case MSG_FIELD_RAW:
        case MSG_FIELD_LIST:
        case MSG_FIELD_LIST_HEX:
        case MSG_FIELD_CUSTOM:
        case MSG_FIELD_VARSTR:
        case MSG_FIELD_INCLUDE: // A VOIR : parcourir les record et vérifier que tous les message sont de taille fixe
            ret = FALSE;
            break;
        
        default:
            break;
        }

        f++;
    }

    return ret;
}

PUBLIC HMSG MSG_Get_First (HLIST hList)
{
    struct MSG_Header *p_header;

    // on récupère la tete de liste (FIFO avec insertion en queue)
    p_header = List_GetHead (hList);

    if (p_header == NULL)
        return NULL;

    return MSG_GET_DATA (p_header);
}

PUBLIC HMSG MSG_Get_Next (HLIST hList, HMSG hMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    p_header = List_GetNext (hList, p_header);

    if (p_header == NULL)
        return NULL;

    return MSG_GET_DATA (p_header);
}

PUBLIC HMSG MSG_Cast_List_Item (PVOID Item)
{
    if (Item == NULL)
        return NULL;

    return MSG_GET_DATA (Item);
}

PRIVATE BOOL MSG_List_Duplicate (HLIST *hNewList, HLIST hRootList)
{
    struct MSG_Header *p_header;
    HMSG hMsg;
    BOOL ret;
 
    // on parcours toute la liste 
    p_header = List_GetHead (hRootList);

    // on n'oublie pas d'annuler le pointeur de liste qui pointe encore sur la meme liste
    // que la structure source depuis le memcpy !
    *hNewList = NULL;

    while  (p_header != NULL)
    {
        // création d'un nouvel item inséré en tete de liste
        hMsg = MSG_New_List (hNewList, p_header->p_field, p_header->size_of_message);
        if (hMsg == NULL)
            return FALSE;

        // on rentre en récursion
        ret = MSG_Duplicate_Recurse (hMsg, MSG_GET_DATA(p_header));
        if (ret == FALSE)
            return FALSE;

       // on parcours toute la liste jusqu'a la fin
        p_header = List_GetNext (hRootList, p_header);
    }

    return TRUE;
}

// Duplication complete d'un message, listes incluses !
PUBLIC HMSG MSG_Duplicate (HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    HMSG hMsg;

    // allocation d'un nouveau message
    hMsg = MSG_New (p_header->p_field, p_header->size_of_message, p_header->name);
    if (hMsg == NULL)
        return NULL;

    // parcours champ par champ du message
    if (MSG_Duplicate_Recurse (hMsg, hRootMsg) == FALSE)
        return NULL;
        
    return hMsg;
}

// Duplication complete d'un message listable
PUBLIC HMSG MSG_Duplicate_List (HLIST *hList, HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    HMSG hMsg;

    // allocation d'un nouveau message
    hMsg = MSG_New_List (hList, p_header->p_field, p_header->size_of_message);
    if (hMsg == NULL)
        return NULL;

    // parcours champ par champ du message
    if (MSG_Duplicate_Recurse (hMsg, hRootMsg) == FALSE)
        return NULL;
        
    return hMsg;
}
        
PRIVATE BOOL MSG_Duplicate_Recurse (HMSG hMsg, HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    union MSG_Field *p_field = MSG_GET_HEADER(hMsg)->p_field;
    BYTE *p_msg = hMsg;
    BYTE *p_root = hRootMsg;
    HMSG hMsgInc;
    DWORD f;
    BOOL ret;

    // "shallow copy" des deux messages
    memcpy (hMsg, hRootMsg, p_header->size_of_message);

    // puis parcours du tableau de conversion a la recherche de champs dynamiques
    f = 0;
    ret = TRUE; 
    while (p_field[f].Stop.type != MSG_FIELD_STOP && ret == TRUE)
    {
        switch (p_field[f].Stop.type)
        {
        case MSG_FIELD_STOP:
            ret = FALSE;
            break;
    
        case MSG_FIELD_LIST:
        case MSG_FIELD_LIST_HEX:

            ret = MSG_List_Duplicate ((HLIST *) (p_msg + p_field[f].List.offset),
                                      *(HLIST *) (p_root + p_field[f].List.offset));

            break;

        case MSG_FIELD_CUSTOM:
            if (p_field[f].Custom.new_func != NULL)
                ret = p_field[f].Custom.new_func ((p_msg + p_field[f].Custom.offset),
                                                  (p_root + p_field[f].Custom.offset));
            break;

        case MSG_FIELD_INCLUDE:
            // on rentre en récursion sur un nouveau message
            hMsgInc = MSG_Duplicate (*(HMSG *)(p_root + p_field[f].Include.offset));
			if (hMsgInc == NULL)
				ret = FALSE;
			else
    			*(HMSG *)(p_msg + p_field[f].Include.offset) = hMsgInc;
            break;

        default:
            break;
        }

        f++;
    }

    return ret;
}

// compte le nombre de chiffres d'un nombre en base 10    
PROTECTED DWORD MSG_Count_Digits (DWORD dw)
{
    DWORD digits;

    digits = 0UL;

    do
    {
        digits ++;
        dw /= 10UL;
    }
    while (dw > 0UL);

    return digits;
}

#define MAX_DUMP_STRING 16384
PROTECTED BOOL MSG_Dump_Write (HANDLE stream, char *fmt, ...)
{
    va_list ap;
    DWORD written;
    BOOL bRet;
    char string[MAX_DUMP_STRING];
    
    if (stream == NULL)
        return FALSE;

    va_start (ap,fmt);
    
    _vsnprintf_s (string, MAX_DUMP_STRING, MAX_DUMP_STRING-1, fmt, ap);
    
    bRet = WriteFile (stream, string, (DWORD)strlen(string), &written, NULL);

    va_end(ap);    

    return bRet;
}

PUBLIC void MSG_Dump_Start (HMSG hRootMsg, HANDLE stream)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);

    p_header->depth = 0;
    p_header->last_offset = 0;
    p_header->stream = stream;
}

PUBLIC HANDLE MSG_Dump_Stop (HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    HANDLE stream;

    stream = p_header->stream;
    p_header->stream = NULL;

    return stream;
}

PROTECTED void MSG_Dump_Recurse_In (HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    DWORD i;

    if (p_header->stream != NULL)
    {    
        MSG_Dump_Write (p_header->stream, "\n");

        for (i = 0; i < p_header->depth; i++)
            MSG_Dump_Write (p_header->stream, " ");

        MSG_Dump_Write (p_header->stream, "{");

        p_header->depth ++;
    }
}

PROTECTED void MSG_Dump_Recurse_Out (HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    DWORD i;

    if (p_header->depth > 0)
    {    
        p_header->depth --;

        MSG_Dump_Write (p_header->stream, "\n");

        for (i = 0; i < p_header->depth; i++)
            MSG_Dump_Write (p_header->stream, " ");

        MSG_Dump_Write (p_header->stream, "}");
    }
}

PROTECTED void MSG_Dump_Gap (HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);

    p_header->last_offset = p_header->offset;
}

PROTECTED BOOL MSG_Dump (HMSG hRootMsg, 
                         DWORD f, 
                         DWORD field_type)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    CHAR *type = "";
    DWORD i;
    BOOL bRet = TRUE;

    if (p_header->depth == 0)
        return TRUE;

    switch (field_type)
    {
    case MSG_FIELD_STOP:
        type = "STOP";
        bRet = FALSE;
        break;

    case MSG_FIELD_NULL:
        type = "NULL";
        break;

    case MSG_FIELD_LONG:
        type = "LONG";
        break;

    case MSG_FIELD_DWORD:
        type = "DWORD";
        break;

    case MSG_FIELD_CONST:
        type = "CONST";
        break;

    case MSG_FIELD_FLOAT:
        type = "FLOAT";
        break;

    case MSG_FIELD_VARSTR:
        type = "VARSTR";
        break;

    case MSG_FIELD_STRING:
        type = "STRING";
        break;

    case MSG_FIELD_SEPARATOR:
        type = "SEP";
        break;

    case MSG_FIELD_RAWHEXSIZE:
        type = "RAWSIZE";
        break;

    case MSG_FIELD_RAWHEX:
        type = "RAWHEX";
        break;

    case MSG_FIELD_RAW:
        type = "RAW";
        break;

    case MSG_FIELD_FIXRAW:
        type = "FIXRAW";
        break;

    case MSG_FIELD_HEXBE:
        type = "HEXBE";
        break;

    case MSG_FIELD_LIST:
        bRet = FALSE;
        type = "LIST";
        break;

    case MSG_FIELD_LIST_HEX:
        bRet = FALSE;
        type = "LIST_HEX";
        break;

    case MSG_FIELD_TIME:
        type = "TIME";
        break;

    case MSG_FIELD_NOW:
        type = "NOW";
        break;

    case MSG_FIELD_INCLUDE:
        bRet = FALSE;
        type = "INCLUDE";
        break;

    case MSG_FIELD_CUSTOM:
        type = "CUSTOM";
        break;

    case MSG_FIELD_VARIANT:
        bRet = FALSE;
        type = "VARIANT";
        break;

    default:
        printf("MSG_Dump : Unknown type %lu", field_type);
        bRet = FALSE;
        break;
    }

    if (bRet == TRUE)
    {
        MSG_Dump_Write (p_header->stream, "\n");

        for (i = 0; i < p_header->depth; i++)
            MSG_Dump_Write (p_header->stream, " ");

        MSG_Dump_Write (p_header->stream, "%02lu:", f);
        MSG_Dump_Write (p_header->stream, " %s\t'", type);

        for (i = p_header->last_offset; i < p_header->offset; i++)
            MSG_Dump_Write (p_header->stream, "%c", p_header->buffer[i]);
        
        MSG_Dump_Write (p_header->stream, "'");
    }
    else
    {
        MSG_Dump_Write (p_header->stream, " %s", type);
    }

    p_header->last_offset = p_header->offset;

    return bRet;
}
