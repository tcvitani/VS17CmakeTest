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
#include <limits.h>

/*--------------- DEFINES: ---------------*/

/*--------------- TYPEDEFS: ---------------*/

/*--------------- VARIABLES: ---------------*/

/*--------------- FUNCTIONS: ---------------*/

// do not remove
#pragma warning(push)
#pragma warning(disable:4996)

PRIVATE BOOL MSG_Write_Recurse (HMSG hMsg, HMSG hRootMsg);

/*--------------- CODE: ---------------*/    

// ECRITURE

// LONG codé décimal ASCII
PRIVATE BOOL MSG_ltoa (LONG lg, CHAR *ascii_chart, DWORD nb_digits)
{
	if (lg < 0)
        return _snprintf (ascii_chart, nb_digits, "%0*.*ld", nb_digits, nb_digits-1, lg) == (int) nb_digits;

    return _snprintf (ascii_chart, nb_digits, "%0*.*ld", nb_digits, nb_digits, lg) == (int) nb_digits;
}

// DWORD codé décimal ASCII
PRIVATE BOOL MSG_dwtoa (DWORD dw, CHAR *ascii_chart, DWORD nb_digits)
{
	return _snprintf (ascii_chart, nb_digits, "%0*.*lu", nb_digits, nb_digits, dw) == (int) nb_digits;
}

// STRING vers ASCII (pas forcement de zero terminal)
PRIVATE BOOL MSG_stoa (CHAR *string, CHAR *ascii_chart, DWORD nb_chars)
{
	return _snprintf (ascii_chart, nb_chars, "%- *.*s", nb_chars, nb_chars, string) == (int) nb_chars;
}

PUBLIC BOOL MSG_Write_Dword (HMSG hMsg, DWORD dword, DWORD min, DWORD max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD max_digits;

    // teste les minimax
    if (!(dword >= min && dword <= max))
    {
        printf ("MSG_Write_Dword : Data out of range %lu != [%lu, %lu]", dword, min, max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Dword : Data out of range %lu != [%lu, %lu]", dword, min, max);
        return FALSE;
    }

    // compte le nombre de chiffres
    max_digits = MSG_Count_Digits (max);
    
    // teste si il reste assez de place dans le buffer
    // pour l'y convertir en ASCII
    if (p_header->offset + max_digits > p_header->size_max)
    {
        printf ("MSG_Write_Dword : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Dword : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        return FALSE;
    }

    // conversion
    if (MSG_dwtoa (dword, &p_header->buffer[p_header->offset], max_digits) == FALSE)
    {
        printf ("MSG_Write_Dword : Data out of range %lu > %lu digits", dword, max_digits);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Dword : Data out of range %lu > %lu digits", dword, max_digits);
        return FALSE;
    }

    // décalage de l'index courant
    p_header->offset += max_digits;

    return TRUE;
}

PUBLIC BOOL MSG_Write_Long (HMSG hMsg, LONG lg, LONG min, LONG max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD max_digits;

    if (!(lg >= min && lg <= max))
    {
        printf ("MSG_Write_Long : Data out of range %ld != [%ld, %ld]", lg, min, max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Long : Data out of range %ld != [%ld, %ld]", lg, min, max);
        return FALSE;
    }

    max_digits = MSG_Count_Digits (max);
    
    if (p_header->offset + max_digits > p_header->size_max)
    {
        printf ("MSG_Write_Long : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Long : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        return FALSE;
    }

    if (MSG_ltoa (lg, &p_header->buffer[p_header->offset], max_digits) == FALSE)
    {
        printf ("MSG_Write_Long : Data out of range %ld > %lu digits", lg, max_digits);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Long : Data out of range %ld > %lu digits", lg, max_digits);
        return FALSE;
    }

    p_header->offset += max_digits;

    return TRUE;
}

PUBLIC BOOL MSG_Write_Float (HMSG hMsg, DOUBLE fl, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    CHAR ascii_float[MAX_DOUBLE];

    // conversion du float en ascii (il semble que max.17 chiffres entier+dec soient
    // affichables ex : "0.12345678901234566" et "12345678901234568" d'ou 19 car. max)

	_snprintf (ascii_float, sizeof(ascii_float), "%*.*g", max_chars, max_chars - 1, fl);

    if (MSG_Write_String (hMsg, ascii_float, 0, max_chars) == FALSE)
    {
        printf ("MSG_Write_Float : MSG_Write_String Error");
        MSG_Dump_Write (p_header->stream, "MSG_Write_Float : MSG_Write_String Error");
        return FALSE;
    }

    return TRUE;
}

PUBLIC BOOL MSG_Write_String (HMSG hMsg, CHAR *string, DWORD min_chars, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD nb_chars;

    // calcul la taille de la chaine sans le zéro de fin de chaine
    nb_chars  = (DWORD)strlen(string);
    
    if (!(nb_chars >= min_chars && nb_chars <= max_chars))
    {
        printf ("MSG_Write_String : Data out of range %lu != [%lu, %lu]", nb_chars, min_chars, max_chars);
        MSG_Dump_Write (p_header->stream, "MSG_Write_String : Data out of range %lu != [%lu, %lu]", nb_chars, min_chars, max_chars);
        return FALSE;
    }

    // ne pas tenir compte du zéro de fin de chaine dans hMsg (inclu dans max_chars)
    if (p_header->offset + max_chars > p_header->size_max)
    {
        printf ("MSG_Write_String : Unexpected end of buffer %lu > %lu", p_header->offset + max_chars, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_String : Unexpected end of buffer %lu > %lu", p_header->offset + max_chars, p_header->size_max);
        return FALSE;
    }

    if (MSG_stoa (string, &p_header->buffer[p_header->offset], max_chars) == FALSE)
    {
        printf ("MSG_Write_String : Data out of range %s > %lu chars", string, max_chars);
        MSG_Dump_Write (p_header->stream, "MSG_Write_String : Data out of range %s > %lu chars", string, max_chars);
        return FALSE;
    }

    p_header->offset += max_chars;

    return TRUE;
}

PUBLIC BOOL MSG_Write_Var_String (HMSG hMsg, CHAR *string, DWORD min_chars, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD nb_chars;

    // calcul la taille de la chaine sans le zéro de fin de chaine
    nb_chars  = (DWORD)strlen(string);
    
    // on stocke le nombre de caractères de la chaine
    if (MSG_Write_Dword (hMsg, nb_chars, min_chars, max_chars) == FALSE)
    {
        printf ("MSG_Write_Var_String : MSG_Write_Dword Error");
        MSG_Dump_Write (p_header->stream, "MSG_Write_Var_String : MSG_Write_Dword Error");
        return FALSE;
    }
    
    MSG_Dump_Gap (hMsg);

    // pas de zéro de fin de chaine dans hMsg
    if (p_header->offset + nb_chars > p_header->size_max)
    {
        printf ("MSG_Write_Var_String : Unexpected end of buffer %lu > %lu", p_header->offset + max_chars, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Var_String : Unexpected end of buffer %lu > %lu", p_header->offset + max_chars, p_header->size_max);
        return FALSE;
    }

    if (MSG_stoa (string, &p_header->buffer[p_header->offset], nb_chars) == FALSE)
    {
        printf ("MSG_Write_Var_String : Data out of range %s > %lu chars", string, max_chars);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Var_String : Data out of range %s > %lu chars", string, max_chars);
        return FALSE;
    }

    p_header->offset += nb_chars;

    return TRUE;
}

PUBLIC BOOL MSG_Write_RawHex (HMSG hMsg, BYTE *raw, DWORD length)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD i;

    if (p_header->offset + length * 2 > p_header->size_max)
    {
        printf ("MSG_Write_Raw : Unexpected end of buffer %lu > %lu", p_header->offset + length * 2, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Raw : Unexpected end of buffer %lu > %lu", p_header->offset + length * 2, p_header->size_max);
        return FALSE;
    }

    for (i = 0; i < length; i++)
		 sprintf (&p_header->buffer[p_header->offset + i * 2], "%02X", raw[i]);

    p_header->offset += length * 2;

    return TRUE;
}

PUBLIC BOOL MSG_Write_Raw (HMSG hMsg, BYTE *raw, DWORD length)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + length > p_header->size_max)
    {
        printf ("MSG_Write_Raw : Unexpected end of buffer %lu > %lu", p_header->offset + length, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Raw : Unexpected end of buffer %lu > %lu", p_header->offset + length, p_header->size_max);
        return FALSE;
    }

    memcpy (&p_header->buffer[p_header->offset], raw, length);

    p_header->offset += length;

    return TRUE;
}

PUBLIC BOOL MSG_Write_HexBE (HMSG hMsg, BYTE *raw, DWORD length)
{
	DWORD	i;
    struct	MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + length > p_header->size_max)
    {
        printf ("MSG_Write_HexBE : Unexpected end of buffer %lu > %lu", p_header->offset + length, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_HexBE : Unexpected end of buffer %lu > %lu", p_header->offset + length, p_header->size_max);
        return FALSE;
    }

	for (i=0; i<length; i++)
	{
		memcpy (&p_header->buffer[p_header->offset + i], &raw[length - 1 - i], 1);
	}

    p_header->offset += length;

    return TRUE;
}

PUBLIC BOOL MSG_Write_Time (HMSG hMsg, LPSYSTEMTIME time)
{
    struct	MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    CHAR string[TIME_FORMAT_LENGTH + 1];
    FILETIME filetime;

    // test date NULL -> wYear = 0000
    if (time->wYear == 0L)
        memset (time, 0, sizeof(SYSTEMTIME));
    else
    {
        // conversion SYSTEMTIME en LONGLONG pour validation du format
        if (SystemTimeToFileTime (time, &filetime) == FALSE)
        {
            printf ("MSG_Write_Time : Unexpected data.");
            MSG_Dump_Write (p_header->stream, "MSG_Write_Time : Unexpected data.");
            return FALSE;
        }
    }

    // conversion en "YYYYMMDDHHMMSS"
    _snprintf(string, 
			sizeof(string), 
			TIME_FORMAT, 
             time->wYear,
             time->wMonth,
             time->wDay,
             time->wHour,
             time->wMinute,
             time->wSecond);
    
    if (MSG_Write_String (hMsg, 
                          string, 
                          TIME_FORMAT_LENGTH,
                          TIME_FORMAT_LENGTH) == FALSE)
    {
        printf ("MSG_Write_Time : MSG_Write_String Error.");
        MSG_Dump_Write (p_header->stream, "MSG_Write_Time : MSG_Write_String Error.");
        return FALSE;
    }

    return TRUE;
}

// décalage de l'offset courant sans ecriture
PUBLIC BOOL MSG_Write_Null (HMSG hMsg, DWORD nb_bytes)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + nb_bytes > p_header->size_max)
    {
        printf ("MSG_Write_Null : Unexpected end of buffer %lu > %lu", p_header->offset + nb_bytes, p_header->size_max);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Null : Unexpected end of buffer %lu > %lu", p_header->offset + nb_bytes, p_header->size_max);
        return FALSE;
    }

    p_header->offset += nb_bytes;
    
    return TRUE;
}

PUBLIC BOOL MSG_Write_List (HMSG hRootMsg, HLIST hList, MSG_List_Func write_func, DWORD min, DWORD max)
{
    struct	MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);

    // si la liste n'existe pas, on la crée
    if (hList == NULL)
    {
        // le nombre d'elements de la liste vaut 0
        if (MSG_Write_Dword (hRootMsg, 0UL, min, max) == FALSE)
        {
            printf ("MSG_Write_List : MSG_Write_Dword Error");
            MSG_Dump_Write (p_header->stream, "MSG_Write_List : MSG_Write_Dword Error");
            return FALSE;
        }

        return TRUE;
    }
        
    // on stocke le nombre d'elements de la liste
    if (MSG_Write_Dword (hRootMsg, List_GetCount(hList), min, max) == FALSE)
    {
        printf ("MSG_Write_List : MSG_Write_Dword Error");
        MSG_Dump_Write (p_header->stream, "MSG_Write_List : MSG_Write_Dword Error");
        return FALSE;
    }

    // on commence par l'element de tete
    p_header = List_GetHead(hList);
    while (p_header != NULL)
    {
        // un element de liste est considérer comme un sous-message
        // avec un entete dont seul le champ p_field est utilisé
        // on peut alors utilisé MSG_Write_Recurse comme write_func...
        if (write_func (MSG_GET_DATA(p_header), hRootMsg) == FALSE)
        {
            printf ("MSG_Write_List : MSG_Write Error");
            MSG_Dump_Write (p_header->stream, "MSG_Write_List : MSG_Write Error");
            return FALSE;
        }

        p_header = List_GetNext(hList, p_header);
    }

    return TRUE;
}

PUBLIC BOOL MSG_Write_List_Hex (HMSG hRootMsg, HLIST hList, MSG_List_Func write_func, DWORD length)
{
    struct	MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
	BYTE	no_list = 0;
	
	union
	{
		DWORD	dw;
		struct
		{
			BYTE	b0;
			BYTE	b1;
			BYTE	b2;
			BYTE	b3;
		}four_byte;
	}
	one_dword;

    // si la liste n'existe pas, on la crée
    if (hList == NULL)
    {
        // le nombre d'elements de la liste vaut 0
		if (MSG_Write_Raw (hRootMsg, &no_list, length) == FALSE)
        {
            printf ("MSG_Write_List_Hex : MSG_Write_RawHex Error");
            MSG_Dump_Write (p_header->stream, "MSG_Write_List_Hex : MSG_Write_RawHex Error");
            return FALSE;
        }

        return TRUE;
    }

	one_dword.dw = List_GetCount(hList);
        
    // on stocke le nombre d'elements de la liste
    if (MSG_Write_Raw (hRootMsg, (BYTE *)&one_dword, length) == FALSE)
    {
        printf ("MSG_Write_List_Hex : MSG_Write_RawHex Error");
        MSG_Dump_Write (p_header->stream, "MSG_Write_List_Hex : MSG_Write_RawHex Error");
        return FALSE;
    }

    // on commence par l'element de tete
    p_header = List_GetHead(hList);
    while (p_header != NULL)
    {
        // un element de liste est considérer comme un sous-message
        // avec un entete dont seul le champ p_field est utilisé
        // on peut alors utilisé MSG_Write_Recurse comme write_func...
        if (write_func (MSG_GET_DATA(p_header), hRootMsg) == FALSE)
        {
            printf ("MSG_Write_List_Hex : MSG_Write Error");
            MSG_Dump_Write (p_header->stream, "MSG_Write_List_Hex : MSG_Write Error");
            return FALSE;
        }

        p_header = List_GetNext(hList, p_header);
    }

    return TRUE;
}

PUBLIC BOOL MSG_Write_Variant (HMSG hMsg, 
                               struct MSG_VARIANT *variant, 
                               DWORD max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    CHAR ascii_float[MAX_DOUBLE];
    SYSTEMTIME systime;
    BOOL ret;

    // enregistrement du type du variant
    ret = MSG_Write_Dword (hMsg, 
                           variant->type,
                           0UL,
                           99UL);

    // selon ce type, faire la conversion
    switch (variant->type)
    {
    case MSG_FIELD_STOP:
        // cas particulier du variant non renseigné
        ret = MSG_Write_Dword (hMsg, 
                               0UL,
                               0UL,
                               MAX_VARIANT);
        break;

    case MSG_FIELD_LONG:
        ret = MSG_Write_Dword (hMsg, 
                               MSG_Count_Digits (LONG_MAX),
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);

        ret = MSG_Write_Long (hMsg, 
                              variant->Long,
                              LONG_MIN,
                              LONG_MAX);
        break;

    case MSG_FIELD_DWORD:
        ret = MSG_Write_Dword (hMsg, 
                               MSG_Count_Digits (ULONG_MAX),
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);
        
        ret = MSG_Write_Dword (hMsg, 
                               variant->Dword,
                               0UL,
                               ULONG_MAX);
        break;

    case MSG_FIELD_FLOAT:
        // conversion du float en ascii
		_snprintf(ascii_float, sizeof(ascii_float), "%g", variant->Float);

        ret = MSG_Write_Var_String (hMsg, ascii_float, 0UL, MAX_VARIANT);
        break;

    case MSG_FIELD_VARSTR:
        ret = MSG_Write_Var_String (hMsg, 
                                    variant->String,
                                    0UL,
                                    MAX_VARIANT);
        break;

    case MSG_FIELD_RAWHEX:
        // la conversion du type RAW en hexa codée ASCII multiplie par
        // deux la taille finale du buffer ASCII par rapport au nombre
        // d'octets du tableau initial
        ret = MSG_Write_Dword (hMsg, 
                               variant->RawSize * 2,
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);

        ret = MSG_Write_RawHex (hMsg, 
                                variant->Raw,
                                variant->RawSize);
        break;

    case MSG_FIELD_RAW:
        // la conversion du type RAW en hexa codée ASCII multiplie par
        // deux la taille finale du buffer ASCII par rapport au nombre
        // d'octets du tableau initial
        ret = MSG_Write_Dword (hMsg, 
                               variant->RawSize,
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);

        ret = MSG_Write_Raw (hMsg, 
                             variant->Raw,
                             variant->RawSize);
        break;

    case MSG_FIELD_TIME:
        ret = MSG_Write_Dword (hMsg, 
                               TIME_FORMAT_LENGTH,
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);

        ret = MSG_Write_Time (hMsg, 
                              &variant->Time);
        break;

    case MSG_FIELD_NOW:

        ret = MSG_Write_Dword (hMsg, 
                               TIME_FORMAT_LENGTH,
                               0UL,
                               MAX_VARIANT);
        
        MSG_Dump_Gap (hMsg);

        // on ignore l'heure fournie par l'utilisateur
        // et on récupère celle du systeme
        GetLocalTime (&systime);

        ret = MSG_Write_Time (hMsg, 
                              &systime);
        break;

    default:
        printf("MSG_Write_Variant : Unknown type %lu", variant->type);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Variant : Unknown type %lu", variant->type);
        ret = FALSE;
        break;
    }
    
    MSG_Dump (hMsg, 0, variant->type);

    // trace si erreur
    if (ret == FALSE)
    {
        printf("MSG_Write_Variant : Error type %lu", variant->type);
        MSG_Dump_Write (p_header->stream, "MSG_Write_Variant : Error type %lu", variant->type);
    }

    return ret;
}

// Conversion dans un buffer ASCII d'une structure de message
PUBLIC DWORD MSG_Write (HMSG hRootMsg,
                        BYTE *p_buffer, 
                        DWORD buffer_size_max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);

    // Initialisation du message
    p_header->offset = 0;
    p_header->buffer = p_buffer;
    p_header->size_max = buffer_size_max;

    // parcours champ par champ du message
    // remarquez le passage de hRootMsg dans les deux parametres...
    if (MSG_Write_Recurse (hRootMsg, hRootMsg) == FALSE)
    {
        printf("MSG_Write : Error msg %s", p_header->name);
        MSG_Dump_Write (p_header->stream, "MSG_Write : Error msg %s", p_header->name);

        return 0UL;
    }
        
    return p_header->offset;
}
        
PRIVATE BOOL MSG_Write_Recurse (HMSG hMsg, HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    union MSG_Field *p_field = MSG_GET_HEADER(hMsg)->p_field;
    BYTE *p_struct = hMsg;
    DWORD f;
	HMSG p_msg;
    BOOL ret = TRUE;
    SYSTEMTIME systime;

    if (ret == TRUE)
        MSG_Dump_Recurse_In(hRootMsg);

    // hMsg fournit la structure de description des champs union Field
    // et les données utilisateur p_struct
    // hMsgRoot fournit le buffer et son offset
    //
    // A l'appel dans MSG_Write hMsg = hMsgRoot
    //
    // Sur appel récursif d'une liste hMsgRoot est inchangé (car on ecrit toujours 
    // dans le meme buffer) mais hMsg correspond a un element de la liste 
    // (d'ou le meme entete pour les elements de liste et les messages)

    f = 0;
    ret = TRUE; 
    while (p_field[f].Stop.type != MSG_FIELD_STOP && ret == TRUE)
    {
        MSG_Dump_Gap (hRootMsg);

        switch (p_field[f].Stop.type)
        {
        case MSG_FIELD_STOP:
            ret = FALSE;
            break;
    
        case MSG_FIELD_NULL:
            ret = MSG_Write_Null(hRootMsg, p_field[f].Null.nb_bytes);
            break;

        case MSG_FIELD_LONG:
            ret = MSG_Write_Long (hRootMsg, 
                                  *(LONG *) (p_struct + p_field[f].Long.offset),
                                  (LONG)p_field[f].Long.min,
                                  (LONG)p_field[f].Long.max);
            break;

        case MSG_FIELD_DWORD:
            ret = MSG_Write_Dword (hRootMsg, 
                                   *(DWORD *) (p_struct + p_field[f].Dword.offset),
                                   (DWORD)p_field[f].Dword.min,
                                   (DWORD)p_field[f].Dword.max);
            break;

        case MSG_FIELD_FLOAT:
            ret = MSG_Write_Float (hRootMsg, 
                                   *(DOUBLE *) (p_struct + p_field[f].Float.offset),
                                   (DWORD)p_field[f].Float.max_chars);
            break;

        case MSG_FIELD_CONST:
            // la valeur fournie par l'utilisateur est ignorée
            ret = MSG_Write_Long (hRootMsg, 
                                  p_field[f].Const.value,
                                  (LONG)p_field[f].Const.min,
                                  (LONG)p_field[f].Const.max);
            break;

        case MSG_FIELD_VARSTR:
            ret = MSG_Write_Var_String (hRootMsg, 
                                        (CHAR *) (p_struct + p_field[f].VarStr.offset),
                                        (DWORD)p_field[f].VarStr.min,
                                        (DWORD)p_field[f].VarStr.max);
            break;

        case MSG_FIELD_STRING:
            ret = MSG_Write_String (hRootMsg, 
                                    (CHAR *) (p_struct + p_field[f].String.offset),
                                    (DWORD)p_field[f].String.min,
                                    (DWORD)p_field[f].String.max);
            break;

        case MSG_FIELD_SEPARATOR:
            ret = MSG_Write_String (hRootMsg, 
                                    (CHAR *) &(p_field[f].Separator.value),
                                    1,
                                    1);
            break;

        case MSG_FIELD_RAWHEXSIZE:
            // la conversion du type RAW en hexa codée ASCII multiplie par
            // deux la taille finale du buffer ASCII par rapport au nombre
            // d'octets du tableau initial
            ret = MSG_Write_Dword (hRootMsg, 
                                   (*(DWORD *) (p_struct + p_field[f].Dword.offset)) * 2,
                                   (DWORD)p_field[f].Dword.min,
                                   (DWORD)p_field[f].Dword.max * 2);
            break;

        case MSG_FIELD_RAWHEX:
            // la conversion du type RAW en hexa codée ASCII multiplie par
            // deux la taille finale du buffer ASCII par rapport au nombre
            // d'octets du tableau initial
            ret = MSG_Write_RawHex (hRootMsg, 
                                    (BYTE *) (p_struct + p_field[f].Raw.offset),
                                    *(DWORD *) (p_struct + p_field[f].Raw.length_offset));
            break;

        case MSG_FIELD_RAW:
            ret = MSG_Write_Raw (hRootMsg, 
                                 (BYTE *) (p_struct + p_field[f].Raw.offset),
                                 *(DWORD *) (p_struct + p_field[f].Raw.length_offset));
            break;

        case MSG_FIELD_FIXRAW:
            ret = MSG_Write_Raw (hRootMsg, 
                                 (BYTE *) (p_struct + p_field[f].FixRaw.offset),
                                 (DWORD)p_field[f].FixRaw.length);
            break;

        case MSG_FIELD_HEXBE:
            ret = MSG_Write_HexBE (hRootMsg, 
                                  (BYTE *) (p_struct + p_field[f].HexBE.offset),
                                  (DWORD)p_field[f].HexBE.length);
            break;

        case MSG_FIELD_LIST:
            
            MSG_Dump_Recurse_In(hRootMsg);

            // on rentre en récursion en passant MSG_Write_Recurse en paramètre
            ret = MSG_Write_List (hRootMsg, 
                                  *(HLIST *) (p_struct + p_field[f].List.offset),
                                  MSG_Write_Recurse, 
                                  (DWORD)p_field[f].List.min,
                                  (DWORD)p_field[f].List.max);
            
            MSG_Dump_Recurse_Out(hRootMsg);

            break;

        case MSG_FIELD_LIST_HEX:
            
            MSG_Dump_Recurse_In(hRootMsg);

            // on rentre en récursion en passant MSG_Write_Recurse en paramètre
            ret = MSG_Write_List_Hex (hRootMsg, 
                                  *(HLIST *) (p_struct + p_field[f].ListHex.offset),
                                  MSG_Write_Recurse, 
                                  (DWORD)p_field[f].ListHex.length);
            
            MSG_Dump_Recurse_Out(hRootMsg);

            break;

        case MSG_FIELD_TIME:
            ret = MSG_Write_Time (hRootMsg, 
                                  (LPSYSTEMTIME) (p_struct + p_field[f].Time.offset));
            break;

        case MSG_FIELD_NOW:

            // on ignore l'heure fournie par l'utilisateur
            // et on récupère celle du systeme
            GetLocalTime (&systime);
            
            ret = MSG_Write_Time (hRootMsg, 
                                  &systime);
            break;

        case MSG_FIELD_INCLUDE:
            // on rentre en récursion sur un nouveau message
			p_msg = *(HMSG *)(p_struct + p_field[f].Include.offset);
			if (p_msg == NULL)
				ret = FALSE;
			else
				ret = MSG_Write_Recurse (p_msg, hRootMsg);
            break;

        case MSG_FIELD_CUSTOM:
            ret = p_field[f].Custom.write_func ((p_struct + p_field[f].Custom.offset),
                                                hRootMsg);
            break;

        case MSG_FIELD_VARIANT:
            ret = MSG_Write_Variant (hRootMsg, 
                                     (struct MSG_VARIANT *) (p_struct + p_field[f].Variant.offset),
                                     (DWORD)p_field[f].Variant.max);
            break;

        default:
            printf("MSG_Write_Recurse : Unknown type %lu", p_field[f].Stop.type);
            MSG_Dump_Write (p_header->stream, "MSG_Write_Recurse : Unknown type %lu", p_field[f].Stop.type);
            ret = FALSE;
            break;
        }
        
        MSG_Dump (hRootMsg, f, p_field[f].Stop.type);

        f++;
    }

    if (ret == TRUE)
        MSG_Dump_Recurse_Out(hRootMsg);

    // trace si erreur
    if (ret == FALSE)
    {
        printf("MSG_Write_Recurse : Error field %lu, buffer offset %lu", f-1, p_header->offset );
        MSG_Dump_Write (p_header->stream, "MSG_Write_Recurse : Error field %lu, buffer offset %lu", f-1, p_header->offset );
    }

    return ret;
}

#pragma warning(pop)
