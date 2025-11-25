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
#ifndef MSG_H
#define MSG_H

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <csr_list.h>

/*--------------- RESERVED: ---------------*/

#ifdef MSG_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

#define MSG_NAME_MAX 64

//////////////////////////////////////////////////
// les 3 defines suivants MSG_DECLARE_REF, 
// MSG_INIT_REF et MSG_OFFSET sont OBSOLETES !
// Ils ne sont conservés que par soucis de compatibilité.
// MSG_DECLARE_REF & MSG_INIT_REF n'ont plus lieu d'exister
// et MSG_OFFSET doit être remplacé par MSG_OFF
#define MSG_DECLARE_REF(a,b) static const struct a REF_##a; \
                             union MSG_Field a[b];

#define MSG_INIT_REF(a,b) memcpy(a, b, sizeof(b));

#define MSG_OFFSET(a,b) ((BYTE *) &REF_##a.b - (BYTE *) &REF_##a)
///////////////////////////////////////////////////

///////////////////////////////////////////////////
// Nouveau define remplacant MSG_OFFSET :
#define MSG_FIELD_SET(a,b) (&(((struct a *) NULL)->b))
///////////////////////////////////////////////////

#define MSG_SIZEOF(a) (sizeof(struct a))

#define HMSG PVOID

#define MSG_VARIANT(a) struct\
{\
    enum MSG_Field_Type type;\
    union\
    {\
        DWORD       Dword;\
        LONG        Long;\
        DOUBLE      Float;\
        SYSTEMTIME  Time;\
        SYSTEMTIME  Now;\
        DWORD       RawSize;\
    };\
    union\
    {\
        BYTE Raw[##a];\
        CHAR String[##a];\
    };\
}\

/*--------------- TYPEDEFS: ---------------*/

typedef BOOL (WINAPI * MSG_Custom_Func)(PVOID data, HMSG hRootMsg);
typedef BOOL (WINAPI * MSG_Custom_Func_New)(PVOID dest, PVOID root);
typedef BOOL (WINAPI * MSG_List_Func)(HMSG hMsg, HMSG hRootMsg);
typedef BOOL (WINAPI * MSG_Delete_Func)(HMSG hMsg, PVOID param);
typedef BOOL (WINAPI * MSG_Compare)(BYTE *p_msg, PVOID param);

#ifdef _WIN64

enum MSG_Field_Type
{
    MSG_FIELD_STOP = 0,
    MSG_FIELD_LONG,
    MSG_FIELD_DWORD,
    MSG_FIELD_FLOAT,
    MSG_FIELD_VARSTR,
    MSG_FIELD_RAW,
    MSG_FIELD_RAWHEX,
    MSG_FIELD_TIME,
    MSG_FIELD_NOW,
    MSG_FIELD_VARIANT,
    MSG_FIELD_RAWHEXSIZE,
    MSG_FIELD_LIST,
    MSG_FIELD_INCLUDE,
    MSG_FIELD_NULL,
    MSG_FIELD_CONST,
    MSG_FIELD_STRING,
    MSG_FIELD_SEPARATOR,
    MSG_FIELD_CUSTOM,
    MSG_FIELD_FIXRAW,
    MSG_FIELD_LIST_HEX,
	MSG_FIELD_HEXBE
};

struct MSG_Field_Stop
{
    enum MSG_Field_Type type;

    // déclarer autant de PVOID que le max de champs de l'union MSG_Field
    PVOID a,b,c,d,e;
};

struct MSG_Field_Null
{
    enum MSG_Field_Type type;
    DWORD nb_bytes;
};

struct MSG_Field_Long
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (LONG *)
    LONG64 min;
    LONG64 max;
};

struct MSG_Field_Dword
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (DWORD64 *)
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_Const
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (LONG *)
    LONG  value;    // LONG
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_Float
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (DOUBLE *)
    DWORD64 max_chars; 
};

struct MSG_Field_String
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (CHAR *)
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_VarStr
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (CHAR *)
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_Separator
{
    enum MSG_Field_Type type;
    DWORD64 value;
};

struct MSG_Field_RawHexSize
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (DWORD64 *)
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_Raw
{
    enum MSG_Field_Type type;
    DWORD64 offset;        // (BYTE *)
    DWORD64 length_offset; // (DWORD64 *)
};

struct MSG_Field_List
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (HLIST *)
    union MSG_Field *p_field;
    DWORD64 size_of_message;
    DWORD64 min;
    DWORD64 max;
};

struct MSG_Field_List_Hex
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (HLIST *)
    union MSG_Field *p_field;
    DWORD64 size_of_message;
    DWORD64 length;
};

struct MSG_Field_Time
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (LPSYSTEMTIME)
};

struct MSG_Field_Now
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (LPSYSTEMTIME)
};

struct MSG_Field_Include
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (HMSG)
    HLIST record;
};

struct MSG_Field_Custom
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (VOID)
    MSG_Custom_Func write_func;
    MSG_Custom_Func read_func;
    MSG_Custom_Func delete_func;
    MSG_Custom_Func_New new_func;
};

struct MSG_Field_Variant
{
    enum MSG_Field_Type type;
    DWORD64 offset;      // (MSG_VARIANT(max) variant)
    DWORD64 max;
};

struct MSG_Field_FixRaw
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (BYTE *)
    DWORD64 length;
};

struct MSG_Field_HexBE
{
    enum MSG_Field_Type type;
    DWORD64 offset;   // (BYTE *)
    DWORD64 length;
};

union MSG_Field
{
    struct MSG_Field_Stop       Stop;   // doit etre le 1er champ de l'union
    struct MSG_Field_Null       Null;
    struct MSG_Field_Long       Long;
    struct MSG_Field_Dword      Dword;
    struct MSG_Field_Const      Const; 
    struct MSG_Field_Float      Float;
    struct MSG_Field_String     String;
    struct MSG_Field_VarStr     VarStr;
    struct MSG_Field_Separator  Separator;
    struct MSG_Field_RawHexSize RawHexSize;
    struct MSG_Field_Raw        Raw;
    struct MSG_Field_List       List;
    struct MSG_Field_Time       Time;
    struct MSG_Field_Now        Now;
    struct MSG_Field_Include    Include;
    struct MSG_Field_Custom     Custom;
    struct MSG_Field_Variant    Variant;
    struct MSG_Field_FixRaw     FixRaw;
    struct MSG_Field_List_Hex   ListHex;
    struct MSG_Field_HexBE      HexBE;
};

#else

enum MSG_Field_Type
{
    MSG_FIELD_STOP = 0,
    MSG_FIELD_LONG,
    MSG_FIELD_DWORD,
    MSG_FIELD_FLOAT,
    MSG_FIELD_VARSTR,
    MSG_FIELD_RAW,
    MSG_FIELD_RAWHEX,
    MSG_FIELD_TIME,
    MSG_FIELD_NOW,
    MSG_FIELD_VARIANT,
    MSG_FIELD_RAWHEXSIZE,
    MSG_FIELD_LIST,
    MSG_FIELD_INCLUDE,
    MSG_FIELD_NULL,
    MSG_FIELD_CONST,
    MSG_FIELD_STRING,
    MSG_FIELD_SEPARATOR,
    MSG_FIELD_CUSTOM,
    MSG_FIELD_FIXRAW,
    MSG_FIELD_LIST_HEX,
	MSG_FIELD_HEXBE
};

struct MSG_Field_Stop
{
    enum MSG_Field_Type type;

    // déclarer autant de PVOID que le max de champs de l'union MSG_Field
    PVOID a,b,c,d,e;
};

struct MSG_Field_Null
{
    enum MSG_Field_Type type;
    DWORD nb_bytes;
};

struct MSG_Field_Long
{
    enum MSG_Field_Type type;
    DWORD offset;   // (LONG *)
    LONG min;
    LONG max;
};

struct MSG_Field_Dword
{
    enum MSG_Field_Type type;
    DWORD offset;   // (DWORD *)
    DWORD min;
    DWORD max;
};

struct MSG_Field_Const
{
    enum MSG_Field_Type type;
    DWORD offset;   // (LONG *)
    LONG  value;    // LONG
    DWORD min;
    DWORD max;
};

struct MSG_Field_Float
{
    enum MSG_Field_Type type;
    DWORD offset;   // (DOUBLE *)
    DWORD max_chars; 
};

struct MSG_Field_String
{
    enum MSG_Field_Type type;
    DWORD offset;   // (CHAR *)
    DWORD min;
    DWORD max;
};

struct MSG_Field_VarStr
{
    enum MSG_Field_Type type;
    DWORD offset;   // (CHAR *)
    DWORD min;
    DWORD max;
};

struct MSG_Field_Separator
{
    enum MSG_Field_Type type;
    DWORD value;
};

struct MSG_Field_RawHexSize
{
    enum MSG_Field_Type type;
    DWORD offset;   // (DWORD *)
    DWORD min;
    DWORD max;
};

struct MSG_Field_Raw
{
    enum MSG_Field_Type type;
    DWORD offset;        // (BYTE *)
    DWORD length_offset; // (DWORD *)
};

struct MSG_Field_List
{
    enum MSG_Field_Type type;
    DWORD offset;   // (HLIST *)
    union MSG_Field *p_field;
    DWORD size_of_message;
    DWORD min;
    DWORD max;
};

struct MSG_Field_List_Hex
{
    enum MSG_Field_Type type;
    DWORD offset;   // (HLIST *)
    union MSG_Field *p_field;
    DWORD size_of_message;
    DWORD length;
};

struct MSG_Field_Time
{
    enum MSG_Field_Type type;
    DWORD offset;   // (LPSYSTEMTIME)
};

struct MSG_Field_Now
{
    enum MSG_Field_Type type;
    DWORD offset;   // (LPSYSTEMTIME)
};

struct MSG_Field_Include
{
    enum MSG_Field_Type type;
    DWORD offset;   // (HMSG)
    HLIST record;
};

struct MSG_Field_Custom
{
    enum MSG_Field_Type type;
    DWORD offset;   // (VOID)
    MSG_Custom_Func write_func;
    MSG_Custom_Func read_func;
    MSG_Custom_Func delete_func;
    MSG_Custom_Func_New new_func;
};

struct MSG_Field_Variant
{
    enum MSG_Field_Type type;
    DWORD offset;      // (MSG_VARIANT(max) variant)
    DWORD max;
};

struct MSG_Field_FixRaw
{
    enum MSG_Field_Type type;
    DWORD offset;   // (BYTE *)
    DWORD length;
};

struct MSG_Field_HexBE
{
    enum MSG_Field_Type type;
    DWORD offset;   // (BYTE *)
    DWORD length;
};

union MSG_Field
{
    struct MSG_Field_Stop       Stop;   // doit etre le 1er champ de l'union
    struct MSG_Field_Null       Null;
    struct MSG_Field_Long       Long;
    struct MSG_Field_Dword      Dword;
    struct MSG_Field_Const      Const; 
    struct MSG_Field_Float      Float;
    struct MSG_Field_String     String;
    struct MSG_Field_VarStr     VarStr;
    struct MSG_Field_Separator  Separator;
    struct MSG_Field_RawHexSize RawHexSize;
    struct MSG_Field_Raw        Raw;
    struct MSG_Field_List       List;
    struct MSG_Field_Time       Time;
    struct MSG_Field_Now        Now;
    struct MSG_Field_Include    Include;
    struct MSG_Field_Custom     Custom;
    struct MSG_Field_Variant    Variant;
    struct MSG_Field_FixRaw     FixRaw;
    struct MSG_Field_List_Hex   ListHex;
    struct MSG_Field_HexBE      HexBE;
};
#endif



/*--------------- FUNCTIONS: ---------------*/

// CONSTRUCTORS / DESTRUCTORS

EXPORT HMSG WINAPI MSG_New (IN union MSG_Field *p_field, IN DWORD size_of_message, IN CHAR name[MSG_NAME_MAX]);

EXPORT HMSG WINAPI MSG_New_List (IN OUT HLIST *hList, IN union MSG_Field *p_field, IN DWORD size_of_message);

EXPORT BOOL WINAPI MSG_Delete_All_List (IN OUT HLIST *hList);

EXPORT BOOL WINAPI MSG_Delete_All (IN OUT HMSG *hMsg);

// ACCESS

EXPORT DWORD WINAPI MSG_Get_Offset (IN HMSG hMsg);

EXPORT CHAR * WINAPI MSG_Get_Name (IN HMSG hMsg);

EXPORT BOOL WINAPI MSG_Is_Fixed_Size (IN union MSG_Field *p_field);

EXPORT HMSG WINAPI MSG_Get_First (IN HLIST hList);

EXPORT HMSG WINAPI MSG_Get_Next (IN HLIST hList, IN HMSG hMsg);

EXPORT HMSG WINAPI MSG_Cast_List_Item (IN PVOID Item);

// Duplication complete d'un message : listes, includes, customs inclus !
EXPORT HMSG WINAPI MSG_Duplicate (IN HMSG hMsg);

EXPORT HMSG WINAPI MSG_Duplicate_List (HLIST *hList, HMSG hRootMsg);

// Fontions de debug : dump vers la console lors d'un MSG_Write()
EXPORT void WINAPI MSG_Dump_Start (IN HMSG hMsg, IN HANDLE stream);

EXPORT HANDLE WINAPI MSG_Dump_Stop (IN HMSG hMsg);

// WRITING

EXPORT DWORD WINAPI MSG_Write (IN OUT HMSG hMsg,
                               IN OUT BYTE *p_buffer, 
                               IN DWORD buffer_size_max);

EXPORT BOOL WINAPI MSG_Write_Long (IN OUT HMSG hMsg, IN LONG lg, IN LONG min, IN LONG max);
EXPORT BOOL WINAPI MSG_Write_Dword (IN OUT HMSG hMsg, IN DWORD dword, IN DWORD min, IN DWORD max);
EXPORT BOOL WINAPI MSG_Write_Float (HMSG hMsg, DOUBLE fl, DWORD max_chars);
EXPORT BOOL WINAPI MSG_Write_String (IN OUT HMSG hMsg, IN CHAR *string, IN DWORD min_chars, IN DWORD max_chars);
EXPORT BOOL WINAPI MSG_Write_Var_String (IN OUT HMSG hMsg, IN CHAR *string, IN DWORD min_chars, IN DWORD max_chars);
EXPORT BOOL WINAPI MSG_Write_RawHex (IN HMSG hMsg, IN BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Write_Raw (IN OUT HMSG hMsg, IN BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Write_HexBE (IN OUT HMSG hMsg, IN BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Write_Time (IN OUT HMSG hMsg, IN LPSYSTEMTIME time);
EXPORT BOOL WINAPI MSG_Write_List (IN OUT HMSG hMsg, 
                                   IN HLIST hList, 
                                   IN MSG_List_Func write_func, 
                                   IN DWORD min, IN DWORD max);
EXPORT BOOL WINAPI MSG_Write_List_Hex (	IN OUT HMSG hMsg, 
										IN HLIST hList, 
										IN MSG_List_Func write_func, 
										IN DWORD length);
EXPORT BOOL WINAPI MSG_Write_Null (IN OUT HMSG hMsg, IN DWORD nb_bytes);
EXPORT BOOL WINAPI MSG_Write_Variant (IN OUT HMSG hMsg, 
                                      IN struct MSG_VARIANT *variant, 
                                      IN DWORD max);

// READING

EXPORT DWORD WINAPI MSG_Read (IN OUT HMSG hMsg,
                              IN BYTE *p_buffer, 
                              IN DWORD buffer_size_max);

EXPORT BOOL WINAPI MSG_Read_Long (IN OUT HMSG hMsg, OUT LONG *lg, IN LONG min, IN LONG max);
EXPORT BOOL WINAPI MSG_Read_Dword (IN OUT HMSG hMsg, OUT DWORD *dword, IN DWORD min, IN DWORD max);
EXPORT BOOL WINAPI MSG_Read_Float (HMSG hMsg, DOUBLE *fl, DWORD max_chars);
EXPORT BOOL WINAPI MSG_Read_String (IN OUT HMSG hMsg, OUT CHAR *string, IN DWORD min_chars, IN DWORD max_chars);
EXPORT BOOL WINAPI MSG_Read_Var_String (IN OUT HMSG hMsg, OUT CHAR *string, IN DWORD min_chars, IN DWORD max_chars);
EXPORT BOOL WINAPI MSG_Read_RawHex (IN OUT HMSG hMsg, OUT BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Read_Raw (IN OUT HMSG hMsg, OUT BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Read_HexBE (IN OUT HMSG hMsg, OUT BYTE *raw, IN DWORD length);
EXPORT BOOL WINAPI MSG_Read_Time (IN OUT HMSG hMsg, IN LPSYSTEMTIME time);
EXPORT BOOL WINAPI MSG_Read_List (IN OUT HMSG hRootMsg, 
                                  IN OUT HLIST *hList, 
                                  IN union MSG_Field *p_field,
                                  IN DWORD size_of_message,
                                  IN MSG_List_Func read_func, 
                                  IN DWORD min, IN DWORD max);
EXPORT BOOL WINAPI MSG_Read_List_Hex (IN OUT HMSG hRootMsg, 
                                  IN OUT HLIST *hList, 
                                  IN union MSG_Field *p_field,
                                  IN DWORD size_of_message,
                                  IN MSG_List_Func read_func, 
                                  IN DWORD length);
EXPORT BOOL WINAPI MSG_Read_Null (IN OUT HMSG hMsg, IN DWORD nb_bytes);
EXPORT BOOL WINAPI MSG_Read_Variant (IN OUT HMSG hMsg, 
                                     OUT struct MSG_VARIANT *variant, 
                                     IN DWORD max);

// RECORD

EXPORT BOOL WINAPI MSG_New_Record (IN OUT HLIST *hList, 
                                   IN MSG_Compare compare_func, 
                                   IN PVOID param, 
                                   IN union MSG_Field *p_field, 
                                   IN DWORD size_of_message,
                                   IN CHAR name[MSG_NAME_MAX]);

EXPORT BOOL WINAPI MSG_Delete_All_Records (HLIST *hList);

EXPORT HMSG WINAPI MSG_New_Read_If_Found (IN HLIST hList, IN BYTE *p_msg, IN DWORD msg_size_max);

/*--------------- VARIABLES: ---------------*/

#undef I
#undef INIT
#undef PUBLIC
#endif /* MSG_H */
