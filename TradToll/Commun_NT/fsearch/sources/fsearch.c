/* --------------------------------------------------------------------
 * (C) 2003 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : FSEARCH
 * FILE       : FSEARCH.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Recherche de données dans une structure formatés
 * --------------------------------------------------------------------
 * DESCRIPTION: Cette librairie permet d'effectuer des recherches
 *              d'enregistrements dans des fichiers ou des blocs
 *              mémoire.
 *
 *              Le format attendu pour un fichier ou un bloc mémoire est :
 *              
 *              +------------+------------+-------------------------------------+
 *              | Nom        | Taille     | Commentaires                        |
 *              |            | octets     |                                     |
 * +------------+------------+------------+-------------------------------------+
 * | Entête     | HeaderSize | 4          | Taille des données de l'entête      |
 * |(Facultatif)|            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | HeaderData | HeaderSize | Données de l'entête                 |
 * +------------+------------+------------+-------------------------------------+
 * | Record 0   | RecordSize | 4          | Taille des données du record        |
 * |            |            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | RecordData | RecordSize | Données du record                   |
 * +------------+------------+------------+-------------------------------------+
 * |                                                                            |
 * | ...........................                                                |
 * |                                                                            |
 * +------------+------------+------------+-------------------------------------+
 * | Record n   | RecordSize | 4          | Taille des données du record        |
 * |            |            |            | Entier binaire poids faible en tête |
 * |            +------------+------------+-------------------------------------+
 * |            | RecordData | RecordSize | Données du record                   |
 * +------------+------------+------------+-------------------------------------+
 * | EndMark    | EndMark    | 4          | Equivaut à un record de taille 0    |
 * |(Facultatif)|            |            | (les 4 octets valent 0)             |
 * +------------+------------+------------+-------------------------------------+
 *
 *              L'entête est facultatif, ainsi que le marqueur de fin.
 *              Lorsque les records sont de taille constante et triés par ordre 
 *              croissant sur la clé utilisée pour la recherche, la librairie est
 *              capable d'effectuer une recherche dichotomique, sinon, la recherche
 *              est linéraire.
 *              Dans tous les cas, la taille d'entête reste libre.
 *              La librairie gère trois type de données pour la clé :
 *                + ASCII : Il s'agit d'une comparaison texte (octet 0 = fin de
 *                        chaine). Il est possible (optionnellement) d'utiliser
 *                        un joker multiple ('*') et des jokers simples ('?') dans
 *                        les données du fichier et / ou la clé à rechercher.
 *                        La comparaison peut être "case sensitive" ou non.
 *                + BCD : Il s'agit d'une suite de digits codés à 2 par octets
 *                        (1 digit par quartet). Dans un octet, le premier digit
 *                        est codé dans le quartet de poids fort.
 *                        Il est possible (optionnellement) d'utiliser un joker
 *                        multiple (0x0A) et des jokers simples (0x0E) dans les
 *                        données du fichier et / ou la clé à rechercher.
 *                + BINAIRE : Il s'agit d'une suite brut d'octets. Il n'est pas
 *                        possible d'utiliser de jocker.
 *
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdlib.h>
#include <search.h>


#ifndef FSEARCH_EXPORTS
#   define FSEARCH_EXPORTS
#endif
#include <CSR_FSEARCH.h>


#define LOC_DEF
#include <FSEARCH_loc.h>
#undef LOC_DEF

#include <memclass.h>






PRIVATE BOOL WINAPI _FSearchKeyDwordSizedFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound,
		IN OUT         DWORD             * pdwPosition);

PRIVATE BOOL WINAPI _FSearchKeyTextFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound );

PRIVATE BOOL WINAPI _FSearchKeyRawFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound,
        IN          DWORD               dwRecordBytes );

PRIVATE void WINAPI _FSearchInit();

PRIVATE int __cdecl _FSearchCompare( 
        IN          BYTE              * pbItem1,
        IN          BYTE              * pbItem2 );

PRIVATE int WINAPI _FSearchCompareAsciiJoker( 
        IN          BYTE              * pbItem1,
        IN          BYTE              * pbItem2, 
        IN          SEARCH_CONTEXT    * psContext );

PRIVATE int WINAPI _FSearchCompareBcdJoker( 
        IN          BYTE              * pbItem1,
        IN          BYTE              * pbItem2, 
        IN          SEARCH_CONTEXT    * psContext );

PRIVATE BYTE WINAPI _FSearchUpper(
        IN          BYTE                cChar );

PRIVATE void * WINAPI _FSearchFindDwordSizedFile(
        IN          BYTE              * pbSearchRecord, 
        IN          SEARCH_CONTEXT    * psContext );

PRIVATE void * WINAPI _FSearchFindTextFile(
        IN          BYTE              * pbSearchRecord, 
        IN          SEARCH_CONTEXT    * psContext );

PRIVATE DWORD _FSearchFindChar( 
        IN          BYTE              * pbBuffer, 
        IN          DWORD               dwBufferBytes, 
        IN          BYTE                bChar );

PRIVATE DWORD _FSearchTextToNumber( 
        IN          BYTE              * pbBuffer, 
        IN          DWORD               dwBufferBytes );

        
        
        




EXPORT BOOL WINAPI FSearchKeyEx( 
        IN          DWORD               dwType,
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound )
{
    BOOL                fResult;
    SEARCH_FILE_TYPE    eType;

	DWORD pdwPosition = 0;

    eType = ( dwType & SEARCH_FILE_MASK );

    switch( eType )
    {
    case SEARCH_FILE_DWORD_SIZED_RECORD :

        fResult = _FSearchKeyDwordSizedFile( psParams, pbSearchRecord, dwKeyOffset, dwKeyBytes, pdwFound, pbFound,&pdwPosition);
        break;

    case SEARCH_FILE_TEXT :

        fResult = _FSearchKeyTextFile( psParams, pbSearchRecord, dwKeyOffset, dwKeyBytes, pdwFound, pbFound );
        break;

    case SEARCH_FILE_RAW_ZERO :

        fResult = _FSearchKeyRawFile( psParams, pbSearchRecord, dwKeyOffset, dwKeyBytes, pdwFound, pbFound, dwType & SEARCH_FILE_NMASK );
        break;

    default :
        SetLastError( ERROR_INVALID_PARAMETER );
        fResult = FALSE;
        break;
    }

    return fResult;
}
        
        


EXPORT BOOL WINAPI FSearchKey( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound )
{
	DWORD pdwPosition = 0;
    return _FSearchKeyDwordSizedFile( psParams, pbSearchRecord, dwKeyOffset, dwKeyBytes, pdwFound, pbFound,&pdwPosition);
}

EXPORT BOOL WINAPI FSearchKeyPosition( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound ,
		IN OUT         DWORD             * pdwRead)
{
    return _FSearchKeyDwordSizedFile( psParams, pbSearchRecord, dwKeyOffset, dwKeyBytes, pdwFound, pbFound, pdwRead);
}


PRIVATE BOOL WINAPI _FSearchKeyDwordSizedFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound,
		IN OUT         DWORD             * pdwPosition)
{
    DWORD           dwErr       = NO_ERROR;
    DWORD           dwCount;
    DWORD           dwBytes;
    BOOL            fSuccess    = FALSE;
    HANDLE          hFile       = INVALID_HANDLE_VALUE;
    HANDLE          hMap        = NULL;
    void          * pvMap       = NULL;
    void          * pvResult;
    SEARCH_CONTEXT  sContext;

#define __leave_err(err)    if (TRUE) { dwErr = (err); __leave; } else

    __try
    {
        //
        // S'assurer de l'initialisation des données globales utilisées
        // par la recherche (en particulier la valeur d'index TLS)
        //
        _FSearchInit();

        //
        // Vérification de la cohérence des options.
        //
        if ( ( ! psParams->fAsciiSearch ) && ( ! psParams->fBinarySearch ) && ( ! psParams->fBCDSearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ( psParams->fAsciiSearch && psParams->fBinarySearch ) || 
             ( psParams->fAsciiSearch && psParams->fBCDSearch    ) || 
             ( psParams->fBCDSearch   && psParams->fBinarySearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fCaseSensitive && ( ! psParams->fAsciiSearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ( ! psParams->fFromFile ) && ( ! psParams->fFromMemory ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fFromFile && psParams->fFromMemory )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fBinarySearch && ( psParams->fMultiJokerAllowed || psParams->fSingleJokerAllowed ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( dwKeyBytes == 0 )
            __leave_err( ERROR_INVALID_PARAMETER );

        sContext.dwKeyOffset = dwKeyOffset;
        sContext.dwKeyBytes = dwKeyBytes;
        sContext.psParams = psParams;

        if ( psParams->fFromFile )
        {
            //
            // Si les données sont dans un fichier, il faut ouvrir le fichier, et créer
            // un mapping de ce fichier pour pouvoir le virtualiser.
            // La taille du fichier ainsi que le pointeur sur la zone mappée
            // viennent compléter les informations manquantes du bloc de
            // paramétrage de la recherche.
            //
            hFile = CreateFile( psParams->szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
            if ( hFile == INVALID_HANDLE_VALUE )
                __leave_err( GetLastError() );

            sContext.dwBufferBytes = GetFileSize( hFile, NULL );
			sContext.dwBufferBytes -=(* pdwPosition);
            if ( sContext.dwBufferBytes == 0xFFFFFFFF )
                __leave_err( GetLastError() );

            hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, sContext.dwBufferBytes, NULL );
            if ( hMap == NULL )
                __leave_err( GetLastError() );

            pvMap = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, sContext.dwBufferBytes );
            if ( pvMap == NULL )
                __leave_err( GetLastError() );

            sContext.pbBufferBytes = pvMap;
			sContext.pbBufferBytes += (* pdwPosition);
        }
        else
        {
            //
            // Si les données sont en mémoire, on récupère juste les infos sur leur
            // localisation et leur taille.
            //
			psParams->dwBufferBytes -= (* pdwPosition);
			psParams->pbBufferBytes += (* pdwPosition);
            sContext.dwBufferBytes = psParams->dwBufferBytes;
            sContext.pbBufferBytes = psParams->pbBufferBytes;
        }

		//mma
		sContext.dwRecordBytesPosition = 0;

        if ( psParams->fHeaderPresent )
        {
            //
            // Si le fichier a un entète, il faut le "supprimer" ...
            // S'assurer que le fichier a assez de place pour héberger un entête minimum
            // et lire la taille de l'entête.
            //
            if ( sContext.dwBufferBytes < sizeof(dwBytes) )
                __leave_err( ERROR_INVALID_DATA );

            dwBytes = sizeof(dwBytes) + *(DWORD*)sContext.pbBufferBytes;

            //
            // S'assurer que l'entête rentre dans le fichier
            //
            if ( dwBytes >= sContext.dwBufferBytes )
                __leave_err( ERROR_INVALID_DATA );

            //
            // "Supprimer" l'entête en diminuant la taille du buffer et en
            // faisant pointer juste après.
            //
            sContext.dwBufferBytes -= dwBytes;
            sContext.pbBufferBytes += dwBytes;
        }

        //
        // Vérifier qu'il y a au moins un record dans lequel faire la recherche.
        //
        if ( sContext.dwBufferBytes < sizeof(sContext.dwRecordBytes) )
             __leave_err( ERROR_INVALID_DATA );

        //
        // Pour éviter que deux recherches en parallèle ne se mordent la
        // queue, on a alloué un slot TLS spécifiquement pour la recherche.
        // On utilise ce slot pour stocker un pointeur sur les paramètres
        // de la recherche. Ce slot TLS sera utilisé comme contexte par
        // les fonctions de comparaison utilisées en callback.
        //
        TlsSetValue( gdwSearchContext, (void*)&sContext );

        if ( psParams->fFixedLength )
        {
            //
            // La taille du prochain record va définir la taille de tous les records
            // ainsi que le nombre de records. On vérifie également que cette taille
            // est compatible avec l'offset de la clé et sa longueur.
            //
            sContext.dwRecordBytes = sizeof(sContext.dwRecordBytes) + *(DWORD*)sContext.pbBufferBytes;
            if ( sContext.dwRecordBytes < ( sContext.dwKeyOffset + sContext.dwKeyBytes ) )
                __leave_err( ERROR_INVALID_DATA );

            //
            // Calculer le nombre total de records entiers et vérifier qu'on
            // en a au moins 1.
            //
            dwCount = sContext.dwBufferBytes / sContext.dwRecordBytes;
            if ( dwCount == 0 )
                __leave_err( ERROR_INVALID_DATA );

            //
            // Ajuster la taille du buffer de sorte qu'il corresponde à un
            // nombre de record exact (supprime la fin du buffer si un record
            // partiel se trouve à la fin).
            //
            sContext.dwBufferBytes = dwCount * sContext.dwRecordBytes;


            if ( psParams->fSortedRecords )
            {
                //
                // La liste est triée, on peut effectuer une recherche
                // binaire (ou dichotomique)
                //
                pvResult = bsearch(
                    pbSearchRecord, 
                    sContext.pbBufferBytes,
                    dwCount,
                    sContext.dwRecordBytes,
                    (void*)&_FSearchCompare );
            }
            else
            {
                //
                // La liste n'est pas triée, on doit effectuer une recherche
                // complète par parcour complet.
                //
                pvResult = _lfind(
                    pbSearchRecord, 
                    sContext.pbBufferBytes,
                    &dwCount,
                    sContext.dwRecordBytes,
                    (void*)&_FSearchCompare );
            }
        }
        else
        {
            pvResult = _FSearchFindDwordSizedFile(
                    pbSearchRecord, 
                    &sContext );
        }

        //
        // Renseigner les paramètres en sortie
        //
        if ( pvResult != NULL )
        {
            if ( (*pdwFound) < sContext.dwRecordBytes )
                __leave_err( ERROR_INSUFFICIENT_BUFFER );

            memcpy( pbFound, pvResult, sContext.dwRecordBytes );
            (*pdwFound) = sContext.dwRecordBytes;
			(*pdwPosition)= sContext.dwRecordBytesPosition;
        }
        else
            (*pdwFound) = 0;

        fSuccess = TRUE;
    }
    __finally
    {
        //
        // Libérer toutes les ressources allouées pour
        // la recherche.
        //
        if ( pvMap != NULL )
            UnmapViewOfFile( pvMap );
        if ( hMap != NULL )
            CloseHandle( hMap );
        if ( hFile != INVALID_HANDLE_VALUE )
            CloseHandle( hFile );

        SetLastError( dwErr );
    }

	return fSuccess;
}
        


PRIVATE BOOL WINAPI _FSearchKeyTextFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound )
{
    DWORD           dwErr       = NO_ERROR;
    DWORD           dwCount;
    DWORD           dwBytes;
    BOOL            fSuccess    = FALSE;
    HANDLE          hFile       = INVALID_HANDLE_VALUE;
    HANDLE          hMap        = NULL;
    void          * pvMap       = NULL;
    void          * pvResult;
    SEARCH_CONTEXT  sContext;

#define __leave_err(err)    if (TRUE) { dwErr = (err); __leave; } else

    __try
    {
        //
        // S'assurer de l'initialisation des données globales utilisées
        // par la recherche (en particulier la valeur d'index TLS)
        //
        _FSearchInit();

        //
        // Vérification de la cohérence des options.
        //
        if ( ( ! psParams->fAsciiSearch ) || psParams->fBinarySearch || psParams->fBCDSearch )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ( ! psParams->fFromFile ) && ( ! psParams->fFromMemory ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fFromFile && psParams->fFromMemory )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( dwKeyBytes == 0 )
            __leave_err( ERROR_INVALID_PARAMETER );

        sContext.dwKeyOffset = dwKeyOffset;
        sContext.dwKeyBytes = dwKeyBytes;
        sContext.psParams = psParams;

        if ( psParams->fFromFile )
        {
            //
            // Si les données sont dans un fichier, il faut ouvrir le fichier, et créer
            // un mapping de ce fichier pour pouvoir le virtualiser.
            // La taille du fichier ainsi que le pointeur sur la zone mappée
            // viennent compléter les informations manquantes du bloc de
            // paramétrage de la recherche.
            //
            hFile = CreateFile( psParams->szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
            if ( hFile == INVALID_HANDLE_VALUE )
                __leave_err( GetLastError() );

            sContext.dwBufferBytes = GetFileSize( hFile, NULL );
            if ( sContext.dwBufferBytes == 0xFFFFFFFF )
                __leave_err( GetLastError() );

            hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, sContext.dwBufferBytes, NULL );
            if ( hMap == NULL )
                __leave_err( GetLastError() );

            pvMap = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, sContext.dwBufferBytes );
            if ( pvMap == NULL )
                __leave_err( GetLastError() );

            sContext.pbBufferBytes = pvMap;
        }
        else
        {
            //
            // Si les données sont en mémoire, on récupère juste les infos sur leur
            // localisation et leur taille.
            //
            sContext.dwBufferBytes = psParams->dwBufferBytes;
            sContext.pbBufferBytes = psParams->pbBufferBytes;
        }

        if ( psParams->fHeaderPresent )
        {
            //
            // Si le fichier a un entète, il faut le "supprimer" ...
            // On recherche donc le CR de fin de ligne.
            //
            dwBytes = _FSearchFindChar( sContext.pbBufferBytes, sContext.dwBufferBytes, 0x0D );
            if ( dwBytes == 0xFFFFFFFF )
            {
                //
                // Il n'y a pas de CR, c'est peut-être juste un LF
                //
                dwBytes = _FSearchFindChar( sContext.pbBufferBytes, sContext.dwBufferBytes, 0x0A );
                if ( dwBytes == 0xFFFFFFFF )
                    __leave_err( ERROR_INVALID_DATA );
            }
            else
            {
                //
                // Vérifier si on a un LF après le CR
                //
                if ( ( dwBytes + 1 ) <= sContext.dwBufferBytes )
                {
                    if ( sContext.pbBufferBytes[dwBytes+1] == 0x0A )
                        dwBytes ++;
                }
            }
            dwBytes ++;            

            //
            // "Supprimer" l'entête en diminuant la taille du buffer et en
            // faisant pointer juste après.
            //
            sContext.dwBufferBytes -= dwBytes;
            sContext.pbBufferBytes += dwBytes;
        }

        //
        // Pour éviter que deux recherches en parallèle ne se mordent la
        // queue, on a alloué un slot TLS spécifiquement pour la recherche.
        // On utilise ce slot pour stocker un pointeur sur les paramètres
        // de la recherche. Ce slot TLS sera utilisé comme contexte par
        // les fonctions de comparaison utilisées en callback.
        //
        TlsSetValue( gdwSearchContext, (void*)&sContext );

        if ( psParams->fFixedLength )
        {
            //
            // La taille du prochain record va définir la taille de tous les records
            // ainsi que le nombre de records.
            //
            sContext.dwRecordBytes = _FSearchFindChar( sContext.pbBufferBytes, sContext.dwBufferBytes, 0x0D );
            if ( sContext.dwRecordBytes == 0xFFFFFFFF )
            {
                //
                // Il n'y a pas de CR, c'est peut-être juste un LF
                //
                sContext.dwRecordBytes = _FSearchFindChar( sContext.pbBufferBytes, sContext.dwBufferBytes, 0x0A );
                if ( sContext.dwRecordBytes == 0xFFFFFFFF )
                    __leave_err( ERROR_INVALID_DATA );
            }
            else
            {
                //
                // Vérifier si on a un LF après le CR
                //
                if ( ( sContext.dwRecordBytes + 1 ) <= sContext.dwBufferBytes )
                {
                    if ( sContext.pbBufferBytes[sContext.dwRecordBytes+1] == 0x0A )
                        sContext.dwRecordBytes ++;
                }
            }
            sContext.dwRecordBytes ++;            

            //
            // On vérifie également que cette taille est compatible avec l'offset de la clé et sa longueur.
            //
            if ( sContext.dwRecordBytes < ( sContext.dwKeyOffset + sContext.dwKeyBytes ) )
                __leave_err( ERROR_INVALID_DATA );

            //
            // Calculer le nombre total de records entiers et vérifier qu'on
            // en a au moins 1.
            //
            dwCount = sContext.dwBufferBytes / sContext.dwRecordBytes;
            if ( dwCount == 0 )
                __leave_err( ERROR_INVALID_DATA );

            //
            // Ajuster la taille du buffer de sorte qu'il corresponde à un
            // nombre de record exact (supprime la fin du buffer si un record
            // partiel se trouve à la fin).
            //
            sContext.dwBufferBytes = dwCount * sContext.dwRecordBytes;


            if ( psParams->fSortedRecords )
            {
                //
                // La liste est triée, on peut effectuer une recherche
                // binaire (ou dichotomique)
                //
                pvResult = bsearch(
                    pbSearchRecord, 
                    sContext.pbBufferBytes,
                    dwCount,
                    sContext.dwRecordBytes,
                    (void*)&_FSearchCompare );
            }
            else
            {
                //
                // La liste n'est pas triée, on doit effectuer une recherche
                // complète par parcour complet.
                //
                pvResult = _lfind(
                    pbSearchRecord, 
                    sContext.pbBufferBytes,
                    &dwCount,
                    sContext.dwRecordBytes,
                    (void*)&_FSearchCompare );
            }
        }
        else
        {
            pvResult = _FSearchFindTextFile(
                    pbSearchRecord, 
                    &sContext );
        }

        //
        // Renseigner les paramètres en sortie
        //
        if ( pvResult != NULL )
        {
            if ( (*pdwFound) < sContext.dwRecordBytes )
                __leave_err( ERROR_INSUFFICIENT_BUFFER );

            memcpy( pbFound, pvResult, sContext.dwRecordBytes );
            (*pdwFound) = sContext.dwRecordBytes;
        }
        else
            (*pdwFound) = 0;

        fSuccess = TRUE;
    }
    __finally
    {
        //
        // Libérer toutes les ressources allouées pour
        // la recherche.
        //
        if ( pvMap != NULL )
            UnmapViewOfFile( pvMap );
        if ( hMap != NULL )
            CloseHandle( hMap );
        if ( hFile != INVALID_HANDLE_VALUE )
            CloseHandle( hFile );

        SetLastError( dwErr );
    }
	return fSuccess;
}




PRIVATE BOOL WINAPI _FSearchKeyRawFile( 
        IN          SEARCH_PARAMS     * psParams,
        IN          BYTE              * pbSearchRecord,
        IN          DWORD               dwKeyOffset,
        IN          DWORD               dwKeyBytes,
        IN OUT      DWORD             * pdwFound,
        OUT         BYTE              * pbFound,
        IN          DWORD               dwRecordBytes )
{
    DWORD           dwErr       = NO_ERROR;
    DWORD           dwCount;
    BOOL            fSuccess    = FALSE;
    HANDLE          hFile       = INVALID_HANDLE_VALUE;
    HANDLE          hMap        = NULL;
    void          * pvMap       = NULL;
    void          * pvResult;
    SEARCH_CONTEXT  sContext;

#define __leave_err(err)    if (TRUE) { dwErr = (err); __leave; } else

    __try
    {
        //
        // S'assurer de l'initialisation des données globales utilisées
        // par la recherche (en particulier la valeur d'index TLS)
        //
        _FSearchInit();

        //
        // Vérification de la cohérence des options.
        //
        if ( ( ! psParams->fAsciiSearch ) && ( ! psParams->fBinarySearch ) && ( ! psParams->fBCDSearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ( psParams->fAsciiSearch && psParams->fBinarySearch ) || 
             ( psParams->fAsciiSearch && psParams->fBCDSearch    ) || 
             ( psParams->fBCDSearch   && psParams->fBinarySearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fCaseSensitive && ( ! psParams->fAsciiSearch ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ( ! psParams->fFromFile ) && ( ! psParams->fFromMemory ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fFromFile && psParams->fFromMemory )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fBinarySearch && ( psParams->fMultiJokerAllowed || psParams->fSingleJokerAllowed ) )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( psParams->fHeaderPresent )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( ! psParams->fFixedLength )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( dwKeyBytes == 0 )
            __leave_err( ERROR_INVALID_PARAMETER );

        if ( dwRecordBytes == 0 )
            __leave_err( ERROR_INVALID_PARAMETER );

        sContext.dwKeyOffset = dwKeyOffset;
        sContext.dwKeyBytes = dwKeyBytes;
        sContext.psParams = psParams;

        if ( psParams->fFromFile )
        {
            //
            // Si les données sont dans un fichier, il faut ouvrir le fichier, et créer
            // un mapping de ce fichier pour pouvoir le virtualiser.
            // La taille du fichier ainsi que le pointeur sur la zone mappée
            // viennent compléter les informations manquantes du bloc de
            // paramétrage de la recherche.
            //
            hFile = CreateFile( psParams->szPath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL );
            if ( hFile == INVALID_HANDLE_VALUE )
                __leave_err( GetLastError() );

            sContext.dwBufferBytes = GetFileSize( hFile, NULL );
            if ( sContext.dwBufferBytes == 0xFFFFFFFF )
                __leave_err( GetLastError() );

            hMap = CreateFileMapping( hFile, NULL, PAGE_READONLY, 0, sContext.dwBufferBytes, NULL );
            if ( hMap == NULL )
                __leave_err( GetLastError() );

            pvMap = MapViewOfFile( hMap, FILE_MAP_READ, 0, 0, sContext.dwBufferBytes );
            if ( pvMap == NULL )
                __leave_err( GetLastError() );

            sContext.pbBufferBytes = pvMap;
        }
        else
        {
            //
            // Si les données sont en mémoire, on récupère juste les infos sur leur
            // localisation et leur taille.
            //
            sContext.dwBufferBytes = psParams->dwBufferBytes;
            sContext.pbBufferBytes = psParams->pbBufferBytes;
        }

        //
        // Pour éviter que deux recherches en parallèle ne se mordent la
        // queue, on a alloué un slot TLS spécifiquement pour la recherche.
        // On utilise ce slot pour stocker un pointeur sur les paramètres
        // de la recherche. Ce slot TLS sera utilisé comme contexte par
        // les fonctions de comparaison utilisées en callback.
        //
        TlsSetValue( gdwSearchContext, (void*)&sContext );

        sContext.dwRecordBytes = dwRecordBytes;

        //
        // Calculer le nombre total de records entiers et vérifier qu'on
        // en a au moins 1.
        //
        dwCount = sContext.dwBufferBytes / sContext.dwRecordBytes;
        if ( dwCount == 0 )
            __leave_err( ERROR_INVALID_DATA );

        //
        // Ajuster la taille du buffer de sorte qu'il corresponde à un
        // nombre de record exact (supprime la fin du buffer si un record
        // partiel se trouve à la fin).
        //
        sContext.dwBufferBytes = dwCount * sContext.dwRecordBytes;

        if ( psParams->fSortedRecords )
        {
            //
            // La liste est triée, on peut effectuer une recherche
            // binaire (ou dichotomique)
            //
            pvResult = bsearch(
                pbSearchRecord, 
                sContext.pbBufferBytes,
                dwCount,
                sContext.dwRecordBytes,
                (void*)&_FSearchCompare );
        }
        else
        {
            //
            // La liste n'est pas triée, on doit effectuer une recherche
            // complète par parcour complet.
            //
            pvResult = _lfind(
                pbSearchRecord, 
                sContext.pbBufferBytes,
                &dwCount,
                sContext.dwRecordBytes,
                (void*)&_FSearchCompare );
        }

        //
        // Renseigner les paramètres en sortie
        //
        if ( pvResult != NULL )
        {
            if ( (*pdwFound) < sContext.dwRecordBytes )
                __leave_err( ERROR_INSUFFICIENT_BUFFER );

            memcpy( pbFound, pvResult, sContext.dwRecordBytes );
            (*pdwFound) = sContext.dwRecordBytes;
        }
        else
            (*pdwFound) = 0;

        fSuccess = TRUE;
    }
    __finally
    {
        //
        // Libérer toutes les ressources allouées pour
        // la recherche.
        //
        if ( pvMap != NULL )
            UnmapViewOfFile( pvMap );
        if ( hMap != NULL )
            CloseHandle( hMap );
        if ( hFile != INVALID_HANDLE_VALUE )
            CloseHandle( hFile );

        SetLastError( dwErr );
    }

	return fSuccess;
}




PRIVATE void WINAPI _FSearchInit()
{
    DWORD dwInc;

    //
    // Le but de cette fonction est d'effectuer l'initialisation tout en s'assurant
    // d'une élection lorsque plusieurs threads tentent de la faire en même temps.
    //

    //
    // On récupère la valeur du flag une fois incrémenté.
    //
    dwInc = InterlockedIncrement( &gdwSearchInit );

    if      ( dwInc == 1 )
    {
        //
        // Le flag incrémenté vaut 1, le thread courant est le premier.
        // Tout autre thread obtiendra désormais une valeur différente.
        //

        //
        // Effectuer les opération d'initialisation :
        //  - Allocation d'un slot TLS utilisé comme contexte lors des recherches.
        //
        gdwSearchContext = TlsAlloc();
        
        //
        // On peut maintenant faire passer le flag à l'état "initialisé"/
        //
        InterlockedExchange( &gdwSearchInit, 0x80000000 );
    }

    else if ( dwInc > 0x80000000 )
    {
        //
        // L'initialisation a déjà été effectuée, on peut repasser le flag à
        // l'état "initialisé".
        //
        InterlockedExchange( &gdwSearchInit, 0x80000000 );
    }

    else
    {
        //
        // L'initialisation est en cours, on attend que le flag atteigne
        // l'état "initialisé".
        //
        while ( InterlockedCompareExchange( (LONG*)&gdwSearchInit, (LONG)0x80000000, (LONG)0x80000000 ) != (LONG)0x80000000 )
            Sleep(1);
    }
}




PRIVATE int __cdecl _FSearchCompare( 
        IN          BYTE              * pbItem1,
        IN          BYTE              * pbItem2 )
{
    SEARCH_CONTEXT * psContext;
    
    //
    // Récupérer le contexte à utiliser pour la comparaison.
    // Attention, ceci suppose que cette fonction est appelée
    // en callback par la fonction de recherche. Dans le cas
    // contraire, le slot TLS risque de ne pas avoir été alloué.
    //
    psContext = (SEARCH_CONTEXT*)TlsGetValue( gdwSearchContext );

    //
    // Lorsqu'on récupère les pointeurs sur les éléments, on les ajuste
    // pour qu'ils pointent sur le premiere octet de leur clé.
    //
    pbItem1 += psContext->dwKeyOffset;
    pbItem2 += psContext->dwKeyOffset;
	psContext->dwRecordBytesPosition +=psContext->dwRecordBytes;

    if ( psContext->psParams->fMultiJokerAllowed || psContext->psParams->fSingleJokerAllowed )
    {
        if ( psContext->psParams->fAsciiSearch )
        {
            //
            // Recherche ASCII avec jocker
            //
            return _FSearchCompareAsciiJoker( 
                pbItem1, 
                pbItem2, 
                psContext );
        }
        else
        {
            //
            // Recherche BCD avec jocker (le binaire a été exclu par les testes
            // de cohérence de la fonction de recherche).
            //
            return _FSearchCompareBcdJoker( 
                pbItem1, 
                pbItem2, 
                psContext );
        }
    }
    else
    {
        if ( psContext->psParams->fAsciiSearch )
        {
            //
            // Recherche ASCII sans jocker
            //
            if ( psContext->psParams->fCaseSensitive )
                return strncmp( pbItem1, pbItem2, psContext->dwKeyBytes );
            else
                return _strnicmp( pbItem1, pbItem2, psContext->dwKeyBytes );
        }
        else
            //
            // Recherche binaire ou BCD sans jocker (identique).
            //
            return memcmp( pbItem1, pbItem2, psContext->dwKeyBytes );
    }
}






PRIVATE int WINAPI _FSearchCompareAsciiJoker( 
        IN          BYTE              * pbItem1,
        IN          BYTE              * pbItem2, 
        IN          SEARCH_CONTEXT    * psContext )
{
    DWORD           dwIndex;
    BYTE            cCur1;
    BYTE            cCur2;

    //
    // Par défaut, on scanne la totalité des octets des buffers
    // à comparer.
    //
    for ( dwIndex = 0 ; dwIndex < psContext->dwKeyBytes ; dwIndex ++ )
    {
        cCur1 = pbItem1[dwIndex];
        cCur2 = pbItem2[dwIndex];

        //
        // Si la recherche n'est pas case-sensitive, on force les caractères
        // alphabétique en majuscule pour effectuer les comparaisons.
        //
        if ( ! psContext->psParams->fCaseSensitive )
        {
            cCur1 = _FSearchUpper( cCur1 );
            cCur2 = _FSearchUpper( cCur2 );
        }

        if      ( psContext->psParams->fMultiJokerAllowed && ( ( cCur1 == (BYTE)'*' ) || ( cCur2 == (BYTE)'*' ) ) )
        {
            //
            // On est tombé sur un jocker multiple, inutile d'aller plus loin,
            // on considère que les chaines sont identiques.
            //
            return 0;
        }

        else if ( psContext->psParams->fSingleJokerAllowed && ( ( cCur1 == (BYTE)'?' ) || ( cCur2 == (BYTE)'?' ) ) )
        {
            //
            // On est tombé sur un jocker simple,
            // on considère que les caractères sont identiques et
            // on passe donc au suivant (rien à faire de particulier)
            //
            continue;
        }

        else if ( cCur1 != cCur2 )
        {
            //
            // Les caractères sont différents. Il peut s'agir d'une différence de
            // caractère ou de la fin d'une des chaines. Dans tous les cas,
            // les chaines donc différentes.
            //
            return ( cCur1 < cCur2 ? -1 : 1 );
        }

        else if ( cCur1 == (BYTE)0   )
        {
            //
            // Les caractères sont identique et valent 0, on est donc tombé sur
            // une double fin de chaine, inutile d'aller plus loin, les chaines
            // sont identiques.
            //
            return 0;
        }
    }

    //
    // En fin de buffer, tous les caractères sont identiques,
    // les chaines sont donc identiques.
    //
    return 0;
}            




PRIVATE int WINAPI _FSearchCompareBcdJoker( 
        IN          BYTE              * pbItem1, 
        IN          BYTE              * pbItem2, 
        IN          SEARCH_CONTEXT    * psContext )
{
    DWORD           dwIndex;
    DWORD           dwMax;
    BYTE            cCur1;
    BYTE            cCur2;

    //
    // Attention, on scanne du BCD codensé, c'est à dire
    // des digits codés sur des quartets.
    //
    dwMax =  psContext->dwKeyBytes * 2;

    //
    // Par défaut, on scanne la totalité des quartets des buffers
    // à comparer.
    //
    for ( dwIndex = 0 ; dwIndex < dwMax ; dwIndex ++ )
    {
        if ( ( dwIndex & 0x00000001 ) == 0 )
        {
            //
            // Position paire (N*2), on prend le quartet de poids fort (premier digit de l'octet N)
            //
            cCur1 = pbItem1[dwIndex/2] >> 4;
            cCur2 = pbItem2[dwIndex/2] >> 4;
        }
        else
        {
            //
            // Position impaire (N*2+1), on prend le quartet de poids faible (dernier digit de l'octet N)
            //
            cCur1 = pbItem1[dwIndex/2] & (BYTE)0x0F;
            cCur2 = pbItem2[dwIndex/2] & (BYTE)0x0F;
        }

        if      ( psContext->psParams->fMultiJokerAllowed && ( ( cCur1 == (BYTE)0x0A ) || ( cCur2 == (BYTE)0x0A ) ) )
        {
            //
            // On est tombé sur un jocker multiple, inutile d'aller plus loin,
            // on considère que les séquences sont identiques.
            //
            return 0;
        }

        else if ( psContext->psParams->fSingleJokerAllowed && ( ( cCur1 == (BYTE)0x0E ) || ( cCur2 == (BYTE)0x0E ) ) )
        {
            //
            // On est tombé sur un jocker simple,
            // on considère que les caractères sont identiques et
            // on passe donc au suivant (rien à faire de particulier)
            //
            continue;
        }

        else if ( cCur1 != cCur2 )
        {
            //
            // Les digits sont différents.
            // Les séquences sont donc différentes.
            //
            return ( cCur1 < cCur2 ? -1 : 1 );
        }
    }

    //
    // En fin de buffer, tous les digits sont identiques,
    // les séquences sont donc identiques.
    //
    return 0;
}            




PRIVATE BYTE WINAPI _FSearchUpper(
        IN          BYTE                cChar ) 
{
    if ( ( cChar >= (BYTE)'a' ) && ( cChar <= (BYTE)'z' ) )
        cChar -= (BYTE)('a'-'A');
    return cChar;
}





PRIVATE void * WINAPI _FSearchFindDwordSizedFile(
        IN          BYTE              * pbSearchRecord, 
        IN          SEARCH_CONTEXT    * psContext )
{
    DWORD           dwPosition = 0;
    DWORD           dwRemains;
    int             iResult;

    //
    // Tant qu'on a pas atteint la fin du bloc
    //
    while ( dwPosition < psContext->dwBufferBytes )
    {
        dwRemains = psContext->dwBufferBytes - dwPosition;

        //
        // Vérifier qu'il y a la place pour une taille et récupérer
        // la taille du record
        //
        if ( dwRemains < sizeof(psContext->dwRecordBytes) )
            return NULL;

        psContext->dwRecordBytes = sizeof(psContext->dwRecordBytes) + *(DWORD*)(psContext->pbBufferBytes+dwPosition);

        //
        // Vérifier que le record rentre dans le bloc
        //
        if ( psContext->dwRecordBytes > dwRemains )
            return NULL;

        //
        // Vérifier que la clé peut être placée dans le bloc. Si le
        // record ne le permet pas, il est ignoré.
        //
        if ( ( psContext->dwKeyOffset + psContext->dwKeyBytes ) <= psContext->dwRecordBytes )
        {
            //
            // Effectuer la comparaison
            //
            iResult = _FSearchCompare( pbSearchRecord, psContext->pbBufferBytes + dwPosition );

            if      ( iResult == 0 )
            {
                //
                // Les clés sont identiques, on arrête la recherche
                //
                return psContext->pbBufferBytes + dwPosition;
            }

            else if ( psContext->psParams->fSortedRecords && ( iResult < 0 ) )
            {
                //
                // La clé recherché est supérieurs à celle du record courant
                // alors que les records sont classés, on peut donc
                // abandonner la recherche.
                //
                return NULL;
            }
        }
        
        //
        // Passer au record suivant
        //
        dwPosition = dwPosition + psContext->dwRecordBytes;
    }

    //
    // La fin du bloc a été atteinte et rien n'a été trouvé
    //
    return NULL;
}




PRIVATE void * WINAPI _FSearchFindTextFile(
        IN          BYTE              * pbSearchRecord, 
        IN          SEARCH_CONTEXT    * psContext )
{
    DWORD           dwPosition = 0;
    DWORD           dwRemains;
    int             iResult;

    //
    // Tant qu'on a pas atteint la fin du bloc
    //
    while ( dwPosition < psContext->dwBufferBytes )
    {
        dwRemains = psContext->dwBufferBytes - dwPosition;

        psContext->dwRecordBytes = _FSearchFindChar( psContext->pbBufferBytes + dwPosition, dwRemains, 0x0D );
        if ( psContext->dwRecordBytes == 0xFFFFFFFF )
        {
            //
            // Il n'y a pas de CR, c'est peut-être juste un LF
            //
            psContext->dwRecordBytes = _FSearchFindChar( psContext->pbBufferBytes + dwPosition, dwRemains, 0x0A );
            if ( psContext->dwRecordBytes == 0xFFFFFFFF )
                return NULL;
        }
        else
        {
            //
            // Vérifier si on a un LF après le CR
            //
            if ( ( psContext->dwRecordBytes + 1 ) <= dwRemains )
            {
                if ( psContext->pbBufferBytes[dwPosition+psContext->dwRecordBytes+1] == 0x0A )
                    psContext->dwRecordBytes ++;
            }
        }
        psContext->dwRecordBytes ++;            

        //
        // Vérifier que la clé peut être placée dans le bloc. Si le
        // record ne le permet pas, il est ignoré.
        //
        if ( ( psContext->dwKeyOffset + psContext->dwKeyBytes ) <= psContext->dwRecordBytes )
        {
            //
            // Effectuer la comparaison
            //
            iResult = _FSearchCompare( pbSearchRecord, psContext->pbBufferBytes + dwPosition );

            if      ( iResult == 0 )
            {
                //
                // Les clés sont identiques, on arrête la recherche
                //
                return psContext->pbBufferBytes + dwPosition;
            }

            else if ( psContext->psParams->fSortedRecords && ( iResult < 0 ) )
            {
                //
                // La clé recherché est supérieurs à celle du record courant
                // alors que les records sont classés, on peut donc
                // abandonner la recherche.
                //
                return NULL;
            }
        }
        
        //
        // Passer au record suivant
        //
        dwPosition = dwPosition + psContext->dwRecordBytes;
    }

    //
    // La fin du bloc a été atteinte et rien n'a été trouvé
    //
    return NULL;
}



PRIVATE DWORD _FSearchFindChar( 
        IN          BYTE              * pbBuffer, 
        IN          DWORD               dwBufferBytes, 
        IN          BYTE                bChar )
{
    register DWORD dwIndex = 0;

    while ( dwIndex < dwBufferBytes )
    {
        if ( pbBuffer[dwIndex] == bChar )
            return dwIndex;
        dwIndex ++;
    }
    return 0xFFFFFFFF;
}



PRIVATE DWORD _FSearchTextToNumber( 
        IN          BYTE              * pbBuffer, 
        IN          DWORD               dwBufferBytes )
{
    register DWORD dwIndex = 0;
    register BYTE  bChar;
    DWORD dwValue = 0;

    while ( dwIndex < dwBufferBytes )
    {
        bChar = pbBuffer[dwIndex];
        if ( isdigit( bChar ) )
            dwValue = ( dwValue * 10 ) + (DWORD)bChar - (DWORD)'0';
        else
            break;
    }
    return dwValue;
}