/*------   (v) 1999 CS-Route  ------------   Droits reserves   ------*/
/* 
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL MSG
* FICHIER: csr_msg.h
* MSGGAGE: C
* --------------------------------------------------------------------
* RESUME: Fichier d'interface du module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations des constantes et des variables
*              d'interface avec le module MSG
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef MSGFILE_H
#define MSGFILE_H

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <csr_msg.h>

/*--------------- RESERVED: ---------------*/

#ifdef FILES_EXPORTS
#   include "public.h"
#else
#   include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define HMSGFILE PVOID

#define MSGFILE_READ_NULL -1

/*--------------- TYPEDEFS: ---------------*/


/*--------------- FUNCTIONS: ---------------*/

// CONSTRUCTORS / DESTRUCTORS

EXPORT HMSGFILE WINAPI MSGFILE_Open (CHAR * name, CHAR * mode, DWORD max_buffer_size);

EXPORT BOOL WINAPI MSGFILE_Close (HMSGFILE *hMsgFile);

// ACCES

EXPORT DWORD WINAPI MSGFILE_Get_Current_Buffer_Size (HMSGFILE hMsgFile);

EXPORT BYTE * WINAPI MSGFILE_Get_Current_Buffer (HMSGFILE hMsgFile);

EXPORT DWORD WINAPI MSGFILE_Copy_Current_Buffer (HMSGFILE hMsgFile, BYTE *p_buffer, DWORD max_buffer_size);

// ENREGISTREMENT

EXPORT BOOL WINAPI MSGFILE_New_Record (HMSGFILE hMsgFile, 
                                       MSG_Compare compare_func, 
                                       PVOID param, 
                                       union MSG_Field *p_field, 
                                       DWORD size_of_message,
                                       CHAR name[MSG_NAME_MAX]);
// WRITING

// acces sequentiel et direct
EXPORT BOOL WINAPI MSGFILE_Write_At_End (HMSGFILE hMsgFile,
                                         HMSG hMsg);

EXPORT BOOL WINAPI MSGFILE_Write_Buffer_At_End (HMSGFILE hMsgFile, 
                                                BYTE *p_buffer, 
                                                DWORD buffer_size);

// acces direct uniquement
EXPORT BOOL WINAPI MSGFILE_Write_At (HMSGFILE hMsgFile,
                                     HMSG hMsg,
                                     DWORD index);

EXPORT BOOL WINAPI MSGFILE_Write_Buffer_At (HMSGFILE hMsgFile, 
                                            BYTE *p_buffer, 
                                            DWORD buffer_size,
									        DWORD index);                                        

// acces sequentiel et direct
EXPORT BOOL WINAPI MSGFILE_Write_Null_At_End (HMSGFILE hMsgFile);

// READING

// acces sequentiel et direct
EXPORT PVOID WINAPI MSGFILE_Read_First (HMSGFILE hMsgFile);

EXPORT BYTE * WINAPI MSGFILE_Read_First_Buffer (HMSGFILE hMsgFile);

// acces sequentiel et direct
EXPORT PVOID WINAPI MSGFILE_Read_Next (HMSGFILE hMsgFile);

EXPORT BYTE * WINAPI MSGFILE_Read_Next_Buffer (HMSGFILE hMsgFile);

// acces direct uniquement
EXPORT PVOID WINAPI MSGFILE_Read_At (HMSGFILE hMsgFile, DWORD index);

EXPORT PVOID WINAPI MSGFILE_Read_Buffer_At (HMSGFILE hMsgFile, DWORD index);

// acces direct et séquentiel
EXPORT PVOID WINAPI MSGFILE_Read_From_Current_Buffer (HMSGFILE hMsgFile);

// SEARCH

// recherche dichotomique
//EXPORT PVOID WINAPI MSGFILE_Log_Search (HMSGFILE hFile, func);

// recherche séquentielle
//EXPORT PVOID WINAPI MSGFILE_Search (HMSGFILE hFile, func);


/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSGFILE_H */
