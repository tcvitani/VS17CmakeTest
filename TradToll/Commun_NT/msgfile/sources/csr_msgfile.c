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

//#include <csr_msg.h>
#include <csr_msgfile.h>
#include <stdio.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"
 
/*--------------- DEFINES: ---------------*/

#define MSGFILE_GET_HEADER(i)  ((struct MSGFILE_Header *) (((BYTE *) i) - sizeof(struct MSGFILE_Header)))
#define MSGFILE_GET_DATA(i)    ((PVOID) (((BYTE *) i) + sizeof(struct MSGFILE_Header)))

/*--------------- TYPEDEFS: ---------------*/

struct MSGFILE_Header
{
    FILE  *hFile;               // handle du fichier
    HLIST hRecord;              // liste des enregistrements reconnus
    BOOL  direct_access;        // acces direct aux enregistrements de taille fixe 
								// et d'un seul type
    DWORD max_buffer_size;      // taille max du buffer de conversion des messages
    DWORD msg_size;             // taille du msg contenu dans le buffer
    BYTE  buffer[];             // buffer de conversion de taille indéfini...
};

/*--------------- VARIABLES: ---------------*/


/*--------------- FUNCTIONS: ---------------*/

/*--------------- CODE: ---------------*/    

// CONSTRUCTEURS / DESTRUCTEURS

EXPORT HMSGFILE WINAPI MSGFILE_Open (CHAR * name, CHAR * mode, DWORD max_buffer_size) 
{
    struct MSGFILE_Header *p_header;
    FILE *hFile;
	errno_t err;

    // on commence par tenter l'ouverture du fichier 
	err = fopen_s(&hFile, name, mode);
	if (hFile == NULL)
        return NULL;

    // on fait précéder la structure de données allouée par un entete
    // caché de l'utilisateur
    if ((p_header = malloc(sizeof(struct MSGFILE_Header) + max_buffer_size)) == NULL)
    {
        fclose (hFile);

        return NULL;
    }

    // RAZ
    memset (p_header, 0, sizeof(struct MSGFILE_Header) + max_buffer_size);
 
    // cet entete contiendra entre autres un lien vers la structure de description
    // des champs du message et la taille max du buffer de conversion
    p_header->hFile = hFile;
    p_header->max_buffer_size = max_buffer_size;

    // jusqu'a preuve du contraire l'accès direct aux enregistrement est autorisé
    p_header->direct_access = TRUE;

    // on retourne l'@ de la zone de données utilisateur
    // a lui de remplir les champs (et de construire les listes)
    return MSGFILE_GET_DATA (p_header);
}

EXPORT BOOL WINAPI MSGFILE_Close (HMSGFILE *hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(*hMsgFile);
    BOOL ret;

    ret = TRUE;

    // on ferme le fichier
    if (fclose(p_header->hFile) != 0)
        ret= FALSE;

    // on libére la mémoire
    *hMsgFile = NULL;
    free (p_header);

    return ret;
}
    
EXPORT BOOL WINAPI MSGFILE_New_Record (HMSGFILE hMsgFile, 
                                       MSG_Compare compare_func, 
                                       PVOID param, 
                                       union MSG_Field *p_field, 
                                       DWORD size_of_message,
                                       CHAR name[MSG_NAME_MAX])
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);
    BOOL ret;

    // ajout a la liste des enregistrements du fichier
    ret = MSG_New_Record (&p_header->hRecord,
                          compare_func,
                          param,
                          p_field,
                          size_of_message,
                          name);

    // si le fichier comporte plus d'un type d'enregistrement
    // ou si l'enregistrement n'est pas de taille fixe
    // les acces directs sont prohibés
    if (ret == TRUE && 
        List_GetCount (p_header->hRecord) > 1 &&
        MSG_Is_Fixed_Size (p_field) == FALSE)
        p_header->direct_access = FALSE;

    return TRUE;
}

EXPORT DWORD WINAPI MSGFILE_Get_Current_Buffer_Size (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);
    
    return p_header->msg_size;
}

EXPORT BYTE * WINAPI MSGFILE_Get_Current_Buffer (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);
    
    return p_header->buffer;
}

EXPORT DWORD WINAPI MSGFILE_Copy_Current_Buffer (HMSGFILE hMsgFile, BYTE *p_buffer, DWORD max_buffer_size)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);
    
    // test si le buffer est de taille suffisante
    if (p_header->msg_size > max_buffer_size)
        return 0L;

    // copie le buffer
    memcpy (p_buffer, p_header->buffer, p_header->msg_size);

    // renvoie la taille du buffer copié
    return p_header->msg_size;
}

// WRITING

// acces sequentiel et direct
EXPORT BOOL WINAPI MSGFILE_Write_At_End (HMSGFILE hMsgFile, HMSG hMsg)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // on convertit la structure en un message "ASCII"
    if (MSG_Write (hMsg, p_header->buffer, p_header->max_buffer_size) == FALSE)
        return FALSE;

    // on récupère la taille du message généré
    p_header->msg_size = MSG_Get_Offset (hMsg);

    // on écrit le message sur disque
    return MSGFILE_Write_Buffer_At_End (hMsgFile, p_header->buffer, p_header->msg_size);
}

EXPORT BOOL WINAPI MSGFILE_Write_Buffer_At_End (HMSGFILE hMsgFile, 
                                                BYTE *p_buffer, 
                                                DWORD buffer_size)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

 	// on positionne l'index courant en fin de fichier
	if (fseek (p_header->hFile, 0, SEEK_END) != 0)
        return FALSE;

    // on l'écrit avant le message
    if (fwrite (&buffer_size, sizeof(buffer_size), 1, p_header->hFile) != 1)
        return FALSE;

    // on écrit le message sur disque
    if (fwrite (p_buffer, buffer_size, 1, p_header->hFile) != 1)
        return FALSE;

    // on flushe les données
    fflush (p_header->hFile);

    return TRUE;
}

// acces direct uniquement
EXPORT BOOL WINAPI MSGFILE_Write_At (HMSGFILE hMsgFile,
                                     HMSG hMsg,
									 DWORD index)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // la structure du fichier autorise-t-elle l'acces direct aux enregistrements ?
    if (p_header->direct_access == FALSE)
        return FALSE;

    // on convertit la structure en un message "ASCII"
    if (MSG_Write (hMsg, p_header->buffer, p_header->max_buffer_size) == FALSE)
        return FALSE;

    // on récupère la taille du message généré
    p_header->msg_size = MSG_Get_Offset (hMsg);

    // on écrit le message sur disque
    return MSGFILE_Write_Buffer_At (hMsgFile, p_header->buffer, p_header->msg_size, index);
}

PUBLIC BOOL WINAPI MSGFILE_Write_Buffer_At (HMSGFILE hMsgFile, 
                                            BYTE *p_buffer, 
                                            DWORD buffer_size,
									        DWORD index)                                         
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // la structure du fichier autorise-t-elle l'acces direct aux enregistrements ?
    if (p_header->direct_access == FALSE)
        return FALSE;

	// on positionne l'index dans le fichier
    if (fseek (p_header->hFile, (long) (index * (sizeof(buffer_size) + buffer_size)), SEEK_SET) != 0)
        return FALSE;

    // on l'écrit avant le message
    if (fwrite (&buffer_size, sizeof(buffer_size), 1, p_header->hFile) != 1)
        return FALSE;

    // on écrit le message sur disque
    if (fwrite (p_buffer, buffer_size, 1, p_header->hFile) != 1)
        return FALSE;

    // on flushe les données
    fflush (p_header->hFile);

    return TRUE;
}


// acces sequentiel et direct
EXPORT BOOL WINAPI MSGFILE_Write_Null_At_End (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // taille de message nulle
    p_header->msg_size = 0UL;

 	// on positionne l'index courant en fin de fichier
	if (fseek (p_header->hFile, 0, SEEK_END) != 0)
        return FALSE;

    // on écrit une taille nulle
    if (fwrite (&p_header->msg_size, sizeof(p_header->msg_size), 1, p_header->hFile) != 1)
        return FALSE;

    // on flushe les données
    fflush (p_header->hFile);

    return TRUE;
}

// READING

// acces sequentiel et direct
EXPORT BYTE * WINAPI MSGFILE_Read_First_Buffer (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // recherche de la taille d'un enregistrement
    // en lisant le premier DWORD du fichier
    if (fseek (p_header->hFile, 0, SEEK_SET) != 0)
        return NULL;
    
    if (fread (&p_header->msg_size, sizeof(p_header->msg_size), 1, p_header->hFile) != 1)
        return NULL;

    // teste de cohérence
    if (p_header->msg_size == 0)
        return (PVOID) MSGFILE_READ_NULL;
    if (p_header->msg_size > p_header->max_buffer_size)
        return NULL;

    // lecture de l'enregistrement du 1er rang
    if (fread (p_header->buffer, p_header->msg_size, 1, p_header->hFile) != 1)
        return NULL;

    // renvoie du buffer
    return p_header->buffer; 
}

// acces sequentiel et direct
EXPORT PVOID WINAPI MSGFILE_Read_First (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // lecture ddu buffer du 1er rang
    if (MSGFILE_Read_First_Buffer (hMsgFile) == NULL)
        return NULL;

    // conversion en une structure de données
    return MSG_New_Read_If_Found (p_header->hRecord, p_header->buffer, p_header->msg_size); 
}

// acces sequentiel et direct
EXPORT BYTE * WINAPI MSGFILE_Read_Next_Buffer (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // recherche de la taille d'un enregistrement
    // en lisant le premier DWORD a l'emplacement courant du fichier
    if (fread (&p_header->msg_size, sizeof(p_header->msg_size), 1, p_header->hFile) != 1)
        return NULL;

    // teste de cohérence
    if (p_header->msg_size == 0)
        return (PVOID) MSGFILE_READ_NULL;
    if (p_header->msg_size > p_header->max_buffer_size)
        return NULL;

    // lecture de l'enregistrement courant
    if (fread (p_header->buffer, p_header->msg_size, 1, p_header->hFile) != 1)
        return NULL;

    // renvoie du buffer
    return p_header->buffer; 
}

// acces sequentiel et direct
EXPORT PVOID WINAPI MSGFILE_Read_Next (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // lecture de l'enregistrement courant
    if (MSGFILE_Read_Next_Buffer (hMsgFile) == NULL)
        return NULL;

    // conversion en une structure de données
    return MSG_New_Read_If_Found (p_header->hRecord, p_header->buffer, p_header->msg_size); 
}

// acces direct uniquement
EXPORT PVOID WINAPI MSGFILE_Read_Buffer_At (HMSGFILE hMsgFile, DWORD index)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // la structure du fichier autorise-t-elle l'acces direct aux enregistrements ?
    if (p_header->direct_access == FALSE)
        return NULL;

    // recherche de la taille d'un enregistrement
    // en lisant le premier DWORD du fichier
    if (fseek (p_header->hFile, 0, SEEK_SET) != 0)
        return NULL;

    if (fread (&p_header->msg_size, sizeof(p_header->msg_size), 1, p_header->hFile) != 1)
        return NULL;

    // teste de cohérence
    if (p_header->msg_size == 0)
        return (PVOID) MSGFILE_READ_NULL;
    if (p_header->msg_size > p_header->max_buffer_size)
        return NULL;

    // positionnement au nième enregistrement
    if (fseek (p_header->hFile, (long) (index * (sizeof(p_header->msg_size) + p_header->msg_size)), SEEK_SET) != 0)
        return NULL;
    
    // lecture de l'enregistrement du nième rang
    if (fread (p_header->buffer, p_header->msg_size, 1, p_header->hFile) != 1)
        return NULL;

    // renvoie du buffer
    return p_header->buffer; 
}

// acces direct uniquement
EXPORT PVOID WINAPI MSGFILE_Read_At (HMSGFILE hMsgFile, DWORD index)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // lecture de l'enregistrement courant
    if (MSGFILE_Read_Buffer_At (hMsgFile, index) == NULL)
        return NULL;

    // conversion en une structure de données
    return MSG_New_Read_If_Found (p_header->hRecord, p_header->buffer, p_header->msg_size); 
}

// acces direct et séquentiel
EXPORT PVOID WINAPI MSGFILE_Read_From_Current_Buffer (HMSGFILE hMsgFile)
{
    struct MSGFILE_Header *p_header = MSGFILE_GET_HEADER(hMsgFile);

    // conversion en une structure de données
    return MSG_New_Read_If_Found (p_header->hRecord, p_header->buffer, p_header->msg_size); 
}

// SEARCH

// recherche dichotomique
//EXPORT PVOID WINAPI MSGFILE_Log_Search (HMSGFILE hFile, func);

// recherche séquentielle
//EXPORT PVOID WINAPI MSGFILE_Search (HMSGFILE hFile, func);






