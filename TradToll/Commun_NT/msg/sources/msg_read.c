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

PRIVATE BOOL MSG_Read_Recurse (HMSG hMsg, HMSG hRootMsg);

PRIVATE long TextToSignedLong( const char * pcText,
							   DWORD        dwDigits );

PRIVATE unsigned long TextToUnsignedLong( const char * pcText,
							              DWORD        dwDigits );

PRIVATE DWORD HexTextToDword( const char * pcText,
					          DWORD        dwDigits );

/*--------------- CODE: ---------------*/    

// LECTURE

// décimal ASCII vers DWORD
PRIVATE BOOL MSG_atodw (CHAR *ascii_chart, DWORD *dw, DWORD nb_digits)
{

	*dw = TextToUnsignedLong( ascii_chart, nb_digits );
    
    return TRUE;
}

// décimal ASCII vers LONG
PRIVATE BOOL MSG_atol (CHAR *ascii_chart, LONG *lg, DWORD nb_digits)
{

	*lg = TextToSignedLong( ascii_chart, nb_digits );
    
    return TRUE;
}

// ASCII vers STRING (Attention ajout systématique d'un zéro en fin de chaine !)
PRIVATE BOOL MSG_atos (CHAR *ascii_chart, CHAR *string, DWORD nb_chars)
{
    // lecture de n-1 caractères
    strncpy_s (string, nb_chars+1, ascii_chart, nb_chars);

    // ajout d'un zéro terminal sur le nième caractère
    string[nb_chars] = '\0';

    return TRUE;
}

PUBLIC BOOL MSG_Read_Dword (HMSG hMsg, DWORD *dword, DWORD min, DWORD max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD max_digits;

    // on compte le nombre de chiffre du champ ASCII
    max_digits = MSG_Count_Digits (max);
    
    // teste si on dépasse la taille max du buffer
    if (p_header->offset + max_digits > p_header->size_max)
    {
        printf ("MSG_Read_Dword : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        return FALSE;
    }

    if (MSG_atodw (&p_header->buffer[p_header->offset], dword, max_digits) == FALSE)
    {
        printf ("MSG_Read_Dword : Data out of range %lu > %lu digits", *dword, max_digits);
        return FALSE;
    }

    // teste le minimax
    if (!(*dword >= min && *dword <= max))
    {
        printf ("MSG_Read_Dword : Data out of range %lu != [%lu, %lu]", *dword, min, max);
        return FALSE;
    }

    // mise à jour de l'offset en lecture
    p_header->offset += max_digits;

    return TRUE;
}

PUBLIC BOOL MSG_Read_String (HMSG hMsg, CHAR *string, DWORD min_chars, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    
    // on ne tient pas compte du zéro de fin de chaine dans le hMsg
    if (p_header->offset + max_chars > p_header->size_max)
    {
        printf ("MSG_Read_String : Unexpected end of buffer %lu > %lu", p_header->offset + max_chars, p_header->size_max);
        return FALSE;
    }

    // ne pas oublier la place du zero de fin de chaine !
    if (MSG_atos (&p_header->buffer[p_header->offset], string, max_chars) == FALSE)
    {
        printf ("MSG_Read_String : Data out of range %s > %lu chars", string, max_chars);
        return FALSE;
    }

    p_header->offset += max_chars;

    return TRUE;
}

PUBLIC BOOL MSG_Read_Var_String (HMSG hMsg, CHAR *string, DWORD min_chars, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD nb_chars;

    // on commence par lire le nombre de caractères de la chaine hors zéro de fin de chaine
    if (MSG_Read_Dword (hMsg, &nb_chars, min_chars, max_chars) == FALSE)
    {
        printf ("MSG_Read_Var_String : MSG_Read_Dword Error");
        return FALSE;
    }

    // pas de zéro de fin de chaine dans hMsg
    if (p_header->offset + nb_chars > p_header->size_max)
    {
        printf ("MSG_Read_Var_String : Unexpected end of buffer %lu > %lu", p_header->offset + nb_chars, p_header->size_max);
        return FALSE;
    }

    // ne pas oublier la place du zéro de fin de chaine !
    if (MSG_atos (&p_header->buffer[p_header->offset], string, nb_chars) == FALSE)
    {
        printf ("MSG_Read_Var_String : Data out of range %s > %lu chars", string, nb_chars);
        return FALSE;
    }

    p_header->offset += nb_chars;

    return TRUE;
}

PUBLIC BOOL MSG_Read_Float (HMSG hMsg, DOUBLE *fl, DWORD max_chars)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    CHAR ascii_float[MAX_DOUBLE];

    if (MSG_Read_String (hMsg, ascii_float, 0, max_chars) == FALSE)
    {
        printf ("MSG_Read_Float : MSG_Read_String Error");
        return FALSE;
    }

    // conversion en float
    *fl = atof (ascii_float);

    return TRUE;
}

PUBLIC BOOL MSG_Read_RawHex (HMSG hMsg, BYTE *raw, DWORD length)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD i;

    if (p_header->offset + length * 2 > p_header->size_max)
    {
        printf ("MSG_Read_Raw : Insufficient buffer space %lu > %lu", p_header->offset + length * 2, p_header->size_max);
        return FALSE;
    }

    for (i = 0; i < length; i++)
		raw[i] = (BYTE)HexTextToDword( &p_header->buffer[p_header->offset + i * 2], 2 );
	
    p_header->offset += length * 2;

    return TRUE;
}

PUBLIC BOOL MSG_Read_Raw (HMSG hMsg, BYTE *raw, DWORD length)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + length > p_header->size_max)
    {
        printf ("MSG_Read_Raw : Insufficient buffer space %lu > %lu", p_header->offset + length, p_header->size_max);
        return FALSE;
    }

    memcpy (raw, &p_header->buffer[p_header->offset], length);

    p_header->offset += length;

    return TRUE;
}

PUBLIC BOOL MSG_Read_HexBE (HMSG hMsg, BYTE *raw, DWORD length)
{
	DWORD	i;
    struct	MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + length > p_header->size_max)
    {
        printf ("MSG_Read_HexBE : Insufficient buffer space %lu > %lu", p_header->offset + length, p_header->size_max);
        return FALSE;
    }

	for (i=0; i<length; i++)
	{
		memcpy (&raw[length - 1 - i], &p_header->buffer[p_header->offset + i], 1);
	}

    p_header->offset += length;

    return TRUE;
}

PUBLIC BOOL MSG_Read_Long (HMSG hMsg, LONG *lg, LONG min, LONG max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    DWORD max_digits;

    max_digits = MSG_Count_Digits (max);
    
    if (p_header->offset + max_digits > p_header->size_max)
    {
        printf ("MSG_Read_Long : Unexpected end of buffer %lu > %lu", p_header->offset + max_digits, p_header->size_max);
        return FALSE;
    }

    if (MSG_atol (&p_header->buffer[p_header->offset], lg, max_digits) == FALSE)
    {
        printf ("MSG_Read_Long : Data out of range %ld > %lu digits", *lg, max_digits);
        return FALSE;
    }

    if (!(*lg >= min && *lg <= max))
    {
        printf ("MSG_Read_Long : Data out of range %ld != [%ld, %ld]", *lg, min, max);
        return FALSE;
    }

    p_header->offset += max_digits;

    return TRUE;
}

// lecture sans conversion (deplacement de l'offset)
PUBLIC BOOL MSG_Read_Null (HMSG hMsg, DWORD nb_bytes)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    if (p_header->offset + nb_bytes > p_header->size_max)
    {
        printf ("MSG_Read_Null : Unexpected end of buffer %lu > %lu", p_header->offset + nb_bytes, p_header->size_max);
        return FALSE;
    }

    p_header->offset += nb_bytes;
    
    return TRUE;
}

PUBLIC BOOL MSG_Read_List (HMSG hRootMsg, 
                           HLIST *hList, 
                           union MSG_Field *p_field,
                           DWORD size_of_message,
                           MSG_List_Func read_func, 
                           DWORD min, DWORD max)
{
    HMSG hMsg;
    DWORD nb_items, i;

    // on commence par lire le nombre d'element de la liste
    if (MSG_Read_Dword (hRootMsg, &nb_items, min, max) == FALSE)
    {
        printf ("MSG_Read_List : MSG_Read_Dword Error");
        return FALSE;
    }

    for ( i = 0; i < nb_items; i++)
    {
        // création et ajout d'un d'un element (creation automatique du handle de liste)
        // la variable p_field est du meme type que celle utilisée pour les messages
        // Un element de liste est percu comme un sous-message
        hMsg = MSG_New_List (hList, p_field, size_of_message);
        if (hMsg == NULL)
        {
            printf ("MSG_Read_List : MSG_New_List Error");
            return FALSE;
        }

        // un element de liste est considérer comme un sous-message
        // avec un entete dont seul le champ p_field est utilisé
        // on peut alors utilisé MSG_Read_Recurse comme read_func...
        if (read_func (hMsg, hRootMsg) == FALSE)
        {
            printf ("MSG_Read_List : MSG_Read Error");
            return FALSE;
        }
    }

    return TRUE;
}

PUBLIC BOOL MSG_Read_List_Hex (	HMSG hRootMsg, 
								HLIST *hList, 
								union MSG_Field *p_field,
								DWORD size_of_message,
								MSG_List_Func read_func, 
								DWORD length)
{
    HMSG	hMsg;
    DWORD	i;
	BYTE	nb_items[4] = {0};

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

    // on commence par lire le nombre d'element de la liste
    if (MSG_Read_Raw (hRootMsg, nb_items, length) == FALSE)
    {
        printf ("MSG_Read_List_Hex : MSG_Read_Raw Error");
        return FALSE;
    }

	one_dword.four_byte.b0 = nb_items[0];
	one_dword.four_byte.b1 = nb_items[1];
	one_dword.four_byte.b2 = nb_items[2];
	one_dword.four_byte.b3 = nb_items[3];

    for ( i = 0; i < one_dword.dw; i++)
    {
        // création et ajout d'un d'un element (creation automatique du handle de liste)
        // la variable p_field est du meme type que celle utilisée pour les messages
        // Un element de liste est percu comme un sous-message
        hMsg = MSG_New_List (hList, p_field, size_of_message);
        if (hMsg == NULL)
        {
            printf ("MSG_Read_List : MSG_New_List Error");
            return FALSE;
        }

        // un element de liste est considérer comme un sous-message
        // avec un entete dont seul le champ p_field est utilisé
        // on peut alors utilisé MSG_Read_Recurse comme read_func...
        if (read_func (hMsg, hRootMsg) == FALSE)
        {
            printf ("MSG_Read_List : MSG_Read Error");
            return FALSE;
        }
    }

    return TRUE;
}

PUBLIC BOOL MSG_Read_Time (HMSG hMsg, LPSYSTEMTIME time)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);
    FILETIME filetime;
	BYTE * pbData;

    if (p_header->offset + TIME_FORMAT_LENGTH > p_header->size_max)
    {
        printf ("MSG_Read_Time : Unexpected end of buffer %lu > %lu", p_header->offset + TIME_FORMAT_LENGTH, p_header->size_max);
        return FALSE;
    }

    // lecture d'une date au format "YYYYMMDDHHMMSS"

	pbData = p_header->buffer + p_header->offset;
	time->wYear   = (unsigned short)TextToUnsignedLong( pbData + 0 , 4 );
	time->wMonth  = (unsigned short)TextToUnsignedLong( pbData + 4 , 2 );
	time->wDay    = (unsigned short)TextToUnsignedLong( pbData + 6 , 2 );
	time->wHour   = (unsigned short)TextToUnsignedLong( pbData + 8 , 2 );
	time->wMinute = (unsigned short)TextToUnsignedLong( pbData + 10, 2 );
	time->wSecond = (unsigned short)TextToUnsignedLong( pbData + 12, 2 );

    // test date NULL -> wYear = 0000
    if (time->wYear == 0L)
        memset (time, 0, sizeof(SYSTEMTIME));
    else
    {
        // conversion SYSTEMTIME en LONGLONG (teste la validté)
        if (SystemTimeToFileTime (time, &filetime) == FALSE)
        {
            printf ("MSG_Read_Time : Unexpected data.");
            return FALSE;
        }

        // puis du LONGLONG vers le meme SYSTEMTIME pour recalculer le champ "jour julien" !
        if (FileTimeToSystemTime (&filetime, time) == FALSE)
        {
            printf ("MSG_Read_Time : Unexpected data.");
            return FALSE;
        }
    }

    p_header->offset += TIME_FORMAT_LENGTH;

    return TRUE;
}

PUBLIC BOOL MSG_Read_Variant (HMSG hMsg, 
                              struct MSG_VARIANT *variant, 
                              DWORD max)
{
    CHAR ascii_float[MAX_DOUBLE];
    DWORD size;
    BOOL ret;

    // enregistrement du type du variant
    ret = MSG_Read_Dword (hMsg, 
                          (DWORD *) &variant->type,
                          0UL,
                          99UL);

    // selon ce type, faire la conversion
    switch (variant->type)
    {
    case MSG_FIELD_STOP:
        // cas particulier du variant non renseigné
        ret = MSG_Read_Dword (hMsg, 
                              &size,
                              0UL,
                              MAX_VARIANT);
        break;

    case MSG_FIELD_LONG:
        ret = MSG_Read_Dword (hMsg, 
                              &size,
                              0UL,
                              MAX_VARIANT);

        ret = MSG_Read_Long (hMsg, 
                             &variant->Long,
                             LONG_MIN,
                             LONG_MAX);
        break;

    case MSG_FIELD_DWORD:
        ret = MSG_Read_Dword (hMsg, 
                              &size,
                              0UL,
                              MAX_VARIANT);
        
        ret = MSG_Read_Dword (hMsg, 
                              &variant->Dword,
                              0UL,
                              ULONG_MAX);
        break;

    case MSG_FIELD_FLOAT:
        ret = MSG_Read_Var_String (hMsg, ascii_float, 0UL, MAX_VARIANT);

        // conversion en float
        variant->Float = atof (ascii_float);
        break;

    case MSG_FIELD_VARSTR:
        ret = MSG_Read_Var_String (hMsg, 
                                   variant->String,
                                   0UL,
                                   MAX_VARIANT);
        break;

    case MSG_FIELD_RAWHEX:
        // la conversion du type RAW en hexa codée ASCII multiplie par
        // deux la taille finale du buffer ASCII par rapport au nombre
        // d'octets du tableau initial
        ret = MSG_Read_Dword (hMsg, 
                              &variant->RawSize,
                              0UL,
                              MAX_VARIANT);
        variant->RawSize /= 2;

        ret = MSG_Read_RawHex (hMsg, 
                               variant->Raw,
                               variant->RawSize);
        break;

    case MSG_FIELD_RAW:
        // la conversion du type RAW en hexa codée ASCII multiplie par
        // deux la taille finale du buffer ASCII par rapport au nombre
        // d'octets du tableau initial
        ret = MSG_Read_Dword (hMsg, 
                              &variant->RawSize,
                              0UL,
                              MAX_VARIANT);

        ret = MSG_Read_Raw (hMsg, 
                            variant->Raw,
                            variant->RawSize);
        break;

    case MSG_FIELD_TIME:
        ret = MSG_Read_Dword (hMsg, 
                              &size,
                              0UL,
                              MAX_VARIANT);

        ret = MSG_Read_Time (hMsg, 
                             &variant->Time);
        break;

    case MSG_FIELD_NOW:
        ret = MSG_Read_Dword (hMsg, 
                              &size,
                              0UL,
                              MAX_VARIANT);

        ret = MSG_Read_Time (hMsg, 
                             &variant->Now);
        break;

    default:
        printf("MSG_Read_Variant : Unknown type %lu", variant->type);
        ret = FALSE;
        break;
    }

    // trace si erreur
    if (ret == FALSE)
    {
        printf("MSG_Read_Variant : Error type %lu", variant->type);
    }

    return ret;
}

PUBLIC DWORD MSG_Read (HMSG hMsg,
                       BYTE *p_buffer, 
                       DWORD buffer_size_max)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hMsg);

    // Initialisation du message
    p_header->offset = 0;
    p_header->buffer = p_buffer;
    p_header->size_max = buffer_size_max;

    // parcours champ par champ du buffer
    // remarquez le passage de hRootMsg dans les deux parametres...
    if (MSG_Read_Recurse (hMsg, hMsg) == FALSE)
    {
        printf("MSG_Read : Error msg %s", p_header->name);

        return 0UL;
    }
        
    return p_header->offset;
}

PRIVATE BOOL MSG_Read_Recurse (HMSG hMsg, HMSG hRootMsg)
{
    struct MSG_Header *p_header = MSG_GET_HEADER(hRootMsg);
    union MSG_Field *p_field = MSG_GET_HEADER(hMsg)->p_field;
    BYTE *p_struct = hMsg;
    DWORD f;
    LONG Const;
    HMSG *p_msg;
    BOOL ret;

    // cf commentaires de MSG_Write_Recurse

    f = 0;
    ret = TRUE; 
    while (p_field[f].Stop.type != MSG_FIELD_STOP && ret == TRUE)
    {
        switch (p_field[f].Stop.type)
        {
        case MSG_FIELD_STOP:
            ret = FALSE;
            break;
    
        case MSG_FIELD_NULL:
            ret = MSG_Read_Null(hRootMsg, p_field[f].Null.nb_bytes);
            break;

        case MSG_FIELD_LONG:
            ret = MSG_Read_Long (hRootMsg, 
                                 (LONG *) (p_struct + p_field[f].Long.offset),
                                 (LONG)p_field[f].Long.min,
                                 (LONG)p_field[f].Long.max);
            break;

        case MSG_FIELD_DWORD:
            ret = MSG_Read_Dword (hRootMsg, 
                                  (DWORD *) (p_struct + p_field[f].Dword.offset),
                                  (DWORD)p_field[f].Dword.min,
                                  (DWORD)p_field[f].Dword.max);
            break;

        case MSG_FIELD_CONST:
            ret = MSG_Read_Long (hRootMsg, 
                                 &Const,
                                 (LONG)p_field[f].Const.min,
                                 (LONG)p_field[f].Const.max);

            // on compare la valeur lue a celle stockée dans le descriptif du champ
            // si c'est OK, on la renvoie a l'utilisateur
            if (Const != p_field[f].Const.value)
                ret = FALSE;
            else
                *(LONG *) (p_struct + p_field[f].Const.offset) = Const;
            
            break;

        case MSG_FIELD_FLOAT:
            ret = MSG_Read_Float (hRootMsg, 
                                  (DOUBLE *) (p_struct + p_field[f].Float.offset),
                                  (DWORD)p_field[f].Float.max_chars);
            break;

        case MSG_FIELD_VARSTR:
            ret = MSG_Read_Var_String (hRootMsg, 
                                       (CHAR *) (p_struct + p_field[f].VarStr.offset),
                                       (DWORD)p_field[f].VarStr.min,
                                       (DWORD)p_field[f].VarStr.max);
            break;

        case MSG_FIELD_STRING:
            ret = MSG_Read_String (hRootMsg, 
                                   (CHAR *) (p_struct + p_field[f].String.offset),
                                   (DWORD)p_field[f].String.min,
                                   (DWORD)p_field[f].String.max);
            break;

        case MSG_FIELD_SEPARATOR:
            ret = MSG_Read_Null (hRootMsg, 1);
            break;

        case MSG_FIELD_RAWHEXSIZE:
            // la conversion du type RAW en hexa codée ASCII multiplie par
            // deux la taille finale du buffer ASCII par rapport au nombre
            // d'octets du tableau initial
            ret = MSG_Read_Dword (hRootMsg, 
                                  (DWORD *) (p_struct + p_field[f].Dword.offset),
                                  (DWORD)p_field[f].Dword.min,
                                  (DWORD)p_field[f].Dword.max * 2);

            (*(DWORD *) (p_struct + p_field[f].Dword.offset)) /= 2;
            break;

        case MSG_FIELD_RAWHEX:
            // la conversion du type RAW en hexa codée ASCII multiplie par
            // deux la taille finale du buffer ASCII par rapport au nombre
            // d'octets du tableau initial
            ret = MSG_Read_RawHex (hRootMsg, 
                                  (BYTE *) (p_struct + p_field[f].Raw.offset),
                                  *(DWORD *) (p_struct + p_field[f].Raw.length_offset));
            break;

        case MSG_FIELD_RAW:
            ret = MSG_Read_Raw (hRootMsg, 
                                (BYTE *) (p_struct + p_field[f].Raw.offset),
                                *(DWORD *) (p_struct + p_field[f].Raw.length_offset));
            break;

        case MSG_FIELD_FIXRAW:
            ret = MSG_Read_Raw (hRootMsg, 
                                (BYTE *) (p_struct + p_field[f].FixRaw.offset),
                                (DWORD)p_field[f].FixRaw.length);
            break;

        case MSG_FIELD_HEXBE:
            ret = MSG_Read_HexBE (hRootMsg, 
                                (BYTE *) (p_struct + p_field[f].HexBE.offset),
                                (DWORD)p_field[f].HexBE.length);
            break;

        case MSG_FIELD_LIST:
            ret = MSG_Read_List (hRootMsg, 
                                 (HLIST *) (p_struct + p_field[f].List.offset),
                                 p_field[f].List.p_field,
                                 (DWORD)p_field[f].List.size_of_message,
                                 MSG_Read_Recurse, 
                                 (DWORD)p_field[f].List.min,
                                 (DWORD)p_field[f].List.max);
            break;

        case MSG_FIELD_LIST_HEX:
            ret = MSG_Read_List_Hex (hRootMsg, 
                                 (HLIST *) (p_struct + p_field[f].ListHex.offset),
                                 p_field[f].ListHex.p_field,
                                 (DWORD)p_field[f].ListHex.size_of_message,
                                 MSG_Read_Recurse, 
                                 (DWORD)p_field[f].ListHex.length);
            break;

        case MSG_FIELD_TIME:
            ret = MSG_Read_Time (hRootMsg, 
                                 (LPSYSTEMTIME) (p_struct + p_field[f].Now.offset));
            break;

        case MSG_FIELD_NOW:
            ret = MSG_Read_Time (hRootMsg, 
                                 (LPSYSTEMTIME) (p_struct + p_field[f].Time.offset));
            break;

        case MSG_FIELD_INCLUDE:
            // on rentre en récursion sur un nouveau message
            p_msg = MSG_New_Read_If_Found (p_field[f].Include.record,
                                           &p_header->buffer[p_header->offset],
                                           p_header->size_max - p_header->offset);
            if (p_msg == NULL)
                ret = FALSE;
            else
            {
                *(HMSG *)(p_struct + p_field[f].Include.offset) = p_msg;
                p_header->offset += MSG_Get_Offset (p_msg);
                ret = TRUE;
            }

            break;

        case MSG_FIELD_CUSTOM:
            ret = p_field[f].Custom.read_func ((p_struct + p_field[f].Custom.offset),
                                                hRootMsg);
            break;

        case MSG_FIELD_VARIANT:
            ret = MSG_Read_Variant (hRootMsg, 
                                    (struct MSG_VARIANT *) (p_struct + p_field[f].Variant.offset),
                                    (DWORD)p_field[f].Variant.max);
            break;

        default:
            printf("MSG_Read_Recurse : Unknown type %lu", p_field[f].Stop.type);
            ret = FALSE;
            break;
        }

        f++;
    }

    // trace si erreur
    if (ret == FALSE)
    {
        printf("MSG_Read_Recurse : Error field %lu, buffer offset %lu", f-1, p_header->offset );
    }

    return ret;
}



// NBL : 28/06/2002 : Ajout de la fonction pour remplacer le sscanf
//                    qui peut poser des problèmes avec des buffer sans
//                    caractère de fin ('\0').
PRIVATE long TextToSignedLong( const char * pcText,
							   DWORD        dwDigits )
{
	DWORD			dwIndex;
	long			lValue = 0;
	BOOL			fStarted = FALSE;
	BOOL			fNeg = FALSE;
	unsigned char   ucItem;

	for ( dwIndex = 0 ; dwIndex < dwDigits ; dwIndex ++ )
	{
		ucItem = pcText[dwIndex];

		if ( ( ucItem == '-' ) && ( ! fStarted ) )
		{
			fStarted = TRUE;
			fNeg = TRUE;
		}
		else if ( ( ucItem == '+' ) && ( ! fStarted ) )
		{
			fStarted = TRUE;
			fNeg = FALSE;
		}
		else if ( ( ucItem >= '0' ) && ( ucItem <= '9' ) )
		{
			lValue = ( lValue * 10 ) + (long)( ucItem - (unsigned char)'0' );
			fStarted = TRUE;
		}
		else if ( ( ( ucItem == ' ' ) || ( ucItem == '\t' ) ) && ( ! fStarted ) )
			continue;
		else
			break;
	}

	return fNeg ?  ( - lValue ) : lValue;
}



// NBL : 28/06/2002 : Ajout de la fonction pour remplacer le sscanf
//                    qui peut poser des problèmes avec des buffer sans
//                    caractère de fin ('\0').
PRIVATE unsigned long TextToUnsignedLong( const char * pcText,
							              DWORD        dwDigits )
{
	DWORD			dwIndex;
	unsigned long   ulValue = 0;
	BOOL			fStarted = FALSE;
	unsigned char   ucItem;

	for ( dwIndex = 0 ; dwIndex < dwDigits ; dwIndex ++ )
	{
		ucItem = pcText[dwIndex];

		if ( ( ucItem == '+' ) && ( ! fStarted ) )
		{
			fStarted = TRUE;
		}
		else if ( ( ucItem >= '0' ) && ( ucItem <= '9' ) )
		{
			ulValue = ( ulValue * 10 ) + (unsigned long)( ucItem - (unsigned char)'0' );
			fStarted = TRUE;
		}
		else if ( ( ( ucItem == ' ' ) || ( ucItem == '\t' ) ) && ( ! fStarted ) )
			continue;
		else
			break;
	}

	return ulValue;
}



// NBL : 28/06/2002 : Ajout de la fonction pour remplacer le sscanf
//                    qui peut poser des problèmes avec des buffer sans
//                    caractère de fin ('\0').
PRIVATE DWORD HexTextToDword( const char * pcText,
					          DWORD        dwDigits )
{
	DWORD			dwIndex;
	DWORD			dwValue = 0;
	BOOL			fStarted = FALSE;
	unsigned char   ucItem;

	for ( dwIndex = 0 ; dwIndex < dwDigits ; dwIndex ++ )
	{
		ucItem = pcText[dwIndex];

		if ( ( ucItem >= '0' ) && ( ucItem <= '9' ) )
		{
			dwValue = ( dwValue * 16 ) + (DWORD)( ucItem - (unsigned char)'0' );
			fStarted = TRUE;
		}
		else if ( ( ucItem >= 'a' ) && ( ucItem <= 'f' ) )
		{
			dwValue = ( dwValue * 16 ) + (DWORD)( ucItem - (unsigned char)'a' ) + 10;
			fStarted = TRUE;
		}
		else if ( ( ucItem >= 'A' ) && ( ucItem <= 'F' ) )
		{
			dwValue = ( dwValue * 16 ) + (DWORD)( ucItem - (unsigned char)'A' ) + 10;
			fStarted = TRUE;
		}
		else if ( ( ( ucItem == ' ' ) || ( ucItem == '\t' ) ) && ( ! fStarted ) )
			continue;
		else
			break;
	}

	return dwValue;
}
