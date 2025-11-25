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

#include <csr_msg.h>
#include <stdio.h>

/*--------------- RESERVED: ---------------*/

#include "memclass.h"
 
/*--------------- DEFINES: ---------------*/

#define MSG_GET_HEADER(i)  ((struct MSG_Header *) (((BYTE *) i) - sizeof(struct MSG_Header)))
#define MSG_GET_DATA(i)    ((PVOID) (((BYTE *) i) + sizeof(struct MSG_Header)))

#define TIME_FORMAT "%04u%02u%02u%02u%02u%02u"
#define TIME_FORMAT_LENGTH 14

#define MAX_DOUBLE 99

#define MAX_VARIANT_SIZE 4
#define MAX_VARIANT 4000

/*--------------- TYPEDEFS: ---------------*/

struct MSG_Header
{
    DWORD offset;               // index courant dans le buffer
    BYTE  *buffer;              // buffer contenant le message ASCII
    DWORD size_max;             // taille limite du buffer
    union MSG_Field *p_field;   // description des champs du message
    DWORD size_of_message;      // taille de la structure du message
    CHAR name[MSG_NAME_MAX];    // nom du message pour traces

    // pour le dump des infos
    DWORD depth;
    DWORD last_offset;
    HANDLE stream;
};

struct MSG_VARIANT
{
    enum MSG_Field_Type type;
    union
    {
        DWORD       Dword;
        LONG        Long;
        DOUBLE      Float;
        SYSTEMTIME  Time;
        SYSTEMTIME  Now;
        DWORD       RawSize;
    };

    union
    {
        BYTE Raw[];
        CHAR String[];
    };
};

/*--------------- FUNCTIONS: ---------------*/

PROTECTED DWORD MSG_Count_Digits (DWORD dw);

PROTECTED void MSG_Dump_Recurse_In (HMSG hRootMsg);

PROTECTED void MSG_Dump_Recurse_Out (HMSG hRootMsg);

PROTECTED void MSG_Dump_Gap (HMSG hRootMsg);

PROTECTED BOOL MSG_Dump (HMSG hRootMsg, 
                         DWORD f, 
                         DWORD field_type);

PROTECTED BOOL MSG_Dump_Write (HANDLE stream, char *fmt, ...);
