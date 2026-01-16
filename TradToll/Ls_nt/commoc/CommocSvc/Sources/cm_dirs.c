/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_dirs.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Gestion des répertoires par l'intermédiaire des librairie
 *              d'extension.
 *              Le but de cette librairie est de minimiser les I/O pour
 *              le suivi de l'état des répertoires. En particulier pour
 *              éviter un trop grand nombre d'énumérations supplémentaire,
 *              on maintient pour chaque répertoire une image de son
 *              contenu qui est mise à jour à chauque opération de copie,
 *              déplacement ou effacement.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <ntsvc.h>
#include <col.h>
#include <acom.h>


#include <cmhost.h>
#include <cmwork.h>
#include <cm_glob.h>
#include <cm_hosts.h>
#include <cm_utl.h>
#include <cm_text.h>


#define LOC_DEF
#include <cm_dirs.h>
#undef LOC_DEF

#include <memclass.h>




//
// PROTOTYPES PRIVES
//

PRIVATE int __cdecl _CMSortCompare( const void * pvItem1, const void * pvItem2 );
PRIVATE BOOL _CMDirAdd( CM_DIR * psDir, CM_FILE * psFile );
PRIVATE BOOL _CMDirRemove( CM_DIR * psDir, char * szName );
PRIVATE DWORD _CMDirFind( CM_DIR * psDir, char * szName );
PRIVATE ULONGLONG _CMSortFileNbr( char * szNbrMask, char * szFileName );




//
// CODE PROTEGE
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirInit( CM_DIR * psDir, CM_HOST * psHost, char * szPath, char * szNameMask )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire non initialisée
 *             psHost : Pointeur sur une struture descriptive d'un host initialisé
 *             szPath : Chemin d'accés au répertoire (dans la syntaxe du host).
 *             szNameMask : Si NULL ou chaine vide, tous les fichiers sont traités.
 *                          Sinon, masque de sélection des fichiers en utilisant
 *                          les métacaractère '*' et '?'
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Initialise une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirInit( CM_DIR * psDir, CM_HOST * psHost, char * szPath, char * szNameMask )
{
    NTSVCInfo( "CMDirInit() : [%s]:%s", psHost->szName, szPath );
    ZeroMemory( psDir, sizeof(psDir[0]) );

    CM_STRNCPY( psDir->szPath, szPath, sizeof(psDir->szPath) );
    CM_STRNCPY( psDir->szMask, szPath, sizeof(psDir->szMask) );
    
	//an invalid string (for example, an empty string or a string that is missing the terminating null character), or end in a trailing backslash (\)
	CMTrim(szNameMask);

	if ( szNameMask == NULL )
    {
        CM_STRNCAT( psDir->szMask, "\\*" , sizeof(psDir->szMask) );
    }
    else if ( *szNameMask == 0 )
    {
        CM_STRNCAT( psDir->szMask, "\\*" , sizeof(psDir->szMask) );
    }
    else
    {
        CM_STRNCAT( psDir->szMask, "\\"       , sizeof(psDir->szMask) );
        CM_STRNCAT( psDir->szMask, szNameMask , sizeof(psDir->szMask) );
    }

    psDir->psHost = psHost;

    InitializeCriticalSection( &psDir->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirTerminate( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les ressources allouées pour une structure descriptive d'un répertoire
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirTerminate( CM_DIR * psDir )
{
    NTSVCInfo( "CMDirTerminate() : [%s]:%s", psDir->psHost->szName, psDir->szPath );

    if ( psDir->psFiles != NULL )
        HeapFree( GetProcessHeap(), 0, psDir->psFiles );

    DeleteCriticalSection( &psDir->sProtect );

    ZeroMemory( psDir, sizeof(psDir[0]) );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirLock( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Vérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirLock( CM_DIR * psDir )
{
    EnterCriticalSection( &psDir->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CMDirUnlock( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Dévérouille l'accés à la structure pour les accés concurents
 * --------------------------------------------------------------------
 */
PROTECTED void CMDirUnlock( CM_DIR * psDir )
{
    LeaveCriticalSection( &psDir->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD CMDirExist( CM_DIR * psDir, char * szFileName  )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szFileName : Nom du fichier (sans le chemin)
 * --------------------------------------------------------------------
 * RETURN    : CM_EXISTS si le fichier existe, CM_NOT_FOUND si le répertoire est
 *             online, mais que le fichier n'existe pas et CM_INVALID si une
 *             erreur fait qu'il est impossible de savoir si le fichier
 *             existe ou nom (par exemple, le répertoire est offline).
 * --------------------------------------------------------------------
 * ROLE      : Détermine si un fichier se trouve dans un répertoire.
 *             Attention, le répertoire doit être raffraichi et online.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD CMDirExist( CM_DIR * psDir, char * szFileName  )
{
    DWORD dwIndex;
    DWORD dwReturn;

    CMDirLock( psDir );

    if ( psDir->dwState == CM_STATE_SYNCHRONIZED )
    {
        dwIndex = _CMDirFind( psDir, szFileName );
        if ( dwIndex == CM_INVALID )
        {
            dwReturn = CM_NOT_FOUND;
            NTSVCInfo( "CMDirExist() : Le fichier %s n'est pas dans [%s]:%s", szFileName, psDir->psHost->szName, psDir->szPath );
        }
        else
        {
            dwReturn = CM_EXISTS ;
            NTSVCInfo( "CMDirExist() : Le fichier %s existe dans [%s]:%s", szFileName, psDir->psHost->szName, psDir->szPath );
        }
    }
    else
    {
        dwReturn = CM_INVALID;
        NTSVCInfo( "CMDirExist() : [%s]:%s n'est pas online", psDir->psHost->szName, psDir->szPath );
    }

    CMDirUnlock( psDir );


    return dwReturn;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED CM_FILE * CMDirFind( CM_DIR * psDir, char * szFileName, BOOL fLock )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szFileName : Nom du fichier (sans le chemin)
 *             fLock      : Si TRUE, lorsque le fichier est trouvé, le vérou sur le
 *                          répertoire est conservé.
 * --------------------------------------------------------------------
 * RETURN    : NULL si erreur, répertoire offline ou fichier non trouvé.
 *             Un pointeur sur une structure fichier si le fichier a été trouvé.
 * --------------------------------------------------------------------
 * ROLE      : Recherche un fichier dans un répertoire.
 * --------------------------------------------------------------------
 */
PROTECTED CM_FILE * CMDirFind( CM_DIR * psDir, char * szFileName, BOOL fLock )
{
    DWORD dwIndex;
    CM_FILE * psReturn = NULL;

    CMDirLock( psDir );

    if ( psDir->dwState == CM_STATE_SYNCHRONIZED )
    {
        dwIndex = _CMDirFind( psDir, szFileName );
        if ( dwIndex == CM_INVALID )
        {
            NTSVCInfo( "CMDirFind() : Le fichier %s n'a pas été trouvé dans [%s]:%s", szFileName, psDir->psHost->szName, psDir->szPath );
        }
        else
        {
            psReturn = &psDir->psFiles[dwIndex];
            NTSVCInfo( "CMDirFind() : Le fichier %s a été troubé dans [%s]:%s", szFileName, psDir->psHost->szName, psDir->szPath );
        }
    }
    else
    {
        NTSVCInfo( "CMDirFind() : [%s]:%s n'est pas online", psDir->psHost->szName, psDir->szPath );
    }

    if ( ( psReturn != NULL ) && ( ! fLock ) )
        CMDirUnlock( psDir );

    return psReturn;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirRefresh( CM_DIR * psDir )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir      : Pointeur sur une struture descriptive d'un répertoire initialisée
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le répertoire est rafraichi et online
 * --------------------------------------------------------------------
 * ROLE      : Rafraichie l'image local d'un répertoire.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirRefresh( CM_DIR * psDir )
{
    DWORD dwIndex;
    DWORD dwNewFiles;
    DWORD dwNewMaxFiles;
    DWORD dwOldPriority;
    BOOL bResult;
    WIN32_FIND_DATA * psFind;
    CM_HOST * psHost;
    CM_FILE * psNewFiles;
    CM_FILE * psNewFile;

    __try
    {
        NTSVCInfo( "CMDirRefresh() : [%s]:%s", psDir->psHost->szName, psDir->szPath );

        CMDirLock( psDir );

        //
        // Mémoriser la priorité actuelle
        //
        dwOldPriority = GetThreadPriority( GetCurrentThread() );

        //
        // Pointer directement sur l'host correspondant
        //
        psHost = psDir->psHost;

        //
        // Par défaut, pas de fichiers listés
        //
        psFind = NULL;

		if (!CMHostCnxEnum(psHost, psDir->szMask, &psFind, &dwNewFiles))
        {
            //
            // L'énumération est impossible. Cela peut être du au fait de la
            // connexion n'est pas online, où qu'elle est en erreur. Dans tous
            // les cas, le status du répertoire passe en erreur et on ne va
            // pas plus loin.
            //
            psDir->dwState = CM_STATE_ERROR;
            psDir->dwChanges = 0;
            __leave;
        }

        //
        // Passer dans la priorité la plus basse dans la mesure où
        // il ne s'agit que de calcul pur.
        //
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

        //
        // Vérifier que le nombre de fichiers obtenus est trop grand pour
        // rentrer dans le tableau alloué.
        //
        if ( dwNewFiles > psDir->dwMaxFiles )
        {
            //
            // Les fichiers listés rentrent pas.
            //

            dwNewMaxFiles = dwNewFiles + CM_LIST_INCREMENT;

            //
            // Allouer un tableau à la nouvelle taille.
            //
            psNewFiles = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(psNewFiles[0]) * dwNewMaxFiles);
            if ( psNewFiles == NULL )
            {
                psDir->dwState = CM_STATE_ERROR;
                psDir->dwChanges = 0;
                __leave;
            }

            //
            // Si le tableau existant a été alloué, on le désalloue
            //
            if ( psDir->psFiles != NULL )
                HeapFree( GetProcessHeap(), 0, psDir->psFiles );

            psDir->psFiles = psNewFiles;
            psDir->dwMaxFiles = dwNewMaxFiles;
        }
        psDir->dwFiles = dwNewFiles;
        psDir->sSort.fFirstQuarter = FALSE;
        psDir->sSort.fMiddle = FALSE;
        psDir->sSort.fLastQuarter = FALSE;

        //
        // Recopier les données des fichiers listés dans le tableau
        //
        for ( dwIndex = 0 ; dwIndex < dwNewFiles ; dwIndex ++ )
        {
            psNewFile = &psDir->psFiles[dwIndex];

			if (psFind != NULL)
			{
				CM_STRNCPY(psNewFile->szName, psFind[dwIndex].cFileName, sizeof(psNewFile->szName));
				psNewFile->dwAttr = psFind[dwIndex].dwFileAttributes;
				psNewFile->ullLastWrite = *(ULONGLONG*)&psFind[dwIndex].ftLastWriteTime;
				psNewFile->ullSize = (ULONGLONG)psFind[dwIndex].nFileSizeHigh << 32
					| (ULONGLONG)psFind[dwIndex].nFileSizeLow;
			}

            if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_NUMBER )
            {
                psNewFile->ullScore = _CMSortFileNbr( psDir->sSort.szNbrMask, psNewFile->szName );
                if ( psNewFile->ullScore < psDir->sSort.ullNbrFirstQuarter )
                    psDir->sSort.fFirstQuarter = TRUE;
                else if ( psNewFile->ullScore > psDir->sSort.ullNbrLastQuarter )
                    psDir->sSort.fLastQuarter = TRUE;
                else
                    psDir->sSort.fMiddle = TRUE;
            }
        }

        //
        // Trier les fichiers dans l'ordre croissant.
        //
        TlsSetValue( gsSvcWork.dwTLS, (void*)psDir );
        qsort( psDir->psFiles, psDir->dwFiles, sizeof(psDir->psFiles[0]), _CMSortCompare );

        //
        // Mettre à jour l'état du répertoire
        //
        psDir->dwState = CM_STATE_SYNCHRONIZED;
        psDir->dwChanges = 0;
    }
    __finally
    {
        //
        // Libérer ce qui a été alloué
        //
        if ( psFind != NULL )
            CMHostCnxFree( psHost, psFind );

        //
        // Restaurer la priorité mémorisée
        //
        SetThreadPriority( GetCurrentThread(), dwOldPriority );

        //
        // Calculer le code de retour alors qu'on est encore en section critique
        //
        bResult = ( psDir->dwState == CM_STATE_SYNCHRONIZED );

        CMDirUnlock( psDir );

        NTSVCInfo( "CMDirRefresh() : [%s]:%s est %s", psDir->psHost->szName, psDir->szPath, bResult ? "ONLINE" : "OFFLINE" );   
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirTransmit( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName )
 * --------------------------------------------------------------------
 * PARAMETERS: psDirSrc : Pointeur sur une struture descriptive d'un répertoire initialisée (source)
 *             psDirDst : Pointeur sur une struture descriptive d'un répertoire initialisée (destination)
 *             szName   : Nom du fichier.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si la transmission a eu lieu sans erreur, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une transmission de fichier d'un répertoire vers un autre.
 *             Si le fichier destination existe déjà, la transmission échoue.
 *             Pour réussir, les répertoires sources et destination doivent vérifier
 *             une des conditions suivantes :
 *                 CONDITION 1 :     Source locale    et    Destination locale
 *                 CONDITION 2 :     Source distante  et    Destination locale
 *                 CONDITION 3 ;     Source locale    et    Destination distante
 *             La combinaison source distante et destination distante est donc interdite.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirTransmit( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName )
{
    DWORD dwIndexSrc;
    DWORD dwIndexDst;
    BOOL bResult = TRUE;
    CM_HOST * psHostSrc;
    CM_HOST * psHostDst;
    char szPathSrc[CM_MAX_STR];
    char szPathDst[CM_MAX_STR];
    char szErr[CM_MAX_STR];

    __try
    {
        CMDirLock( psDirSrc );
        CMDirLock( psDirDst );

        //
        // Pointer directement sur les hosts correspondant et s'assurer
        // que celui annoncé comme local est local, et que celui annoncé
        // comme distant est distant.
        //
        psHostSrc = psDirSrc->psHost;
        psHostDst = psDirDst->psHost;
        if ( ( !psHostSrc->bLocal ) && ( !psHostDst->bLocal ) )
        {
            bResult = FALSE;
			strcpy_s(szErr, sizeof(szErr), "Hosts source et destination sont distants");
            __leave;
        }
    
        dwIndexSrc = _CMDirFind( psDirSrc, szName );
        if ( dwIndexSrc == CM_INVALID )
        {
            //
            // Le fichier ne se trouve pas dans le répertoire source
            // ou l'host est offline
            //
            bResult = FALSE;
			strcpy_s(szErr, sizeof(szErr), "Fichier source absent ou répertoire source offline");
            __leave;        
        }

        dwIndexDst = _CMDirFind( psDirDst, szName );
        if ( dwIndexDst == CM_INVALID )
        {
            //
            // Le fichier ne se trouve pas dans le répertoire destination.
            // S'assurer que c'est bien du à son absence et pas à
            // cause de l'état offline du répertoire.
            //
            if ( GetLastError() != ERROR_FILE_NOT_FOUND )
            {
                bResult = FALSE;
				strcpy_s(szErr, sizeof(szErr), "Répertoire destination offline");
                __leave;
            }
        }
        else
        {
            //
            // Le fichier existe déjà, ce n'est pas la peine de le copier.
            //
            bResult = TRUE;
            __leave;
        }

        //
        // Construire les chemin des fichiers source et destination puis
        // effectuer la copie distance
        //
        CMMakePath( psDirSrc->szPath, szName, szPathSrc, sizeof(szPathSrc) );
        CMMakePath( psDirDst->szPath, szName, szPathDst, sizeof(szPathDst) );
        if ( psHostSrc->bLocal )
            bResult = CMHostCnxPut( psHostDst, szPathSrc, szPathDst );
        else
            bResult = CMHostCnxGet( psHostSrc, szPathSrc, szPathDst );
        if ( ! bResult )
        {
			strcpy_s(szErr, sizeof(szErr), "Echec de l'opération i/o");
            __leave;
        }

        //
        // Ajouter le fichier à la liste du répertoire destination
        //
        bResult = _CMDirAdd( psDirDst, &psDirSrc->psFiles[dwIndexSrc] );
        if ( ! bResult )
        {
            psDirDst->dwState = CM_STATE_ERROR;
            psDirDst->dwChanges = 0;
			strcpy_s(szErr, sizeof(szErr), "Echec sur mise à jour du répertoire virtuel destination");
            __leave;
        }
    }
    __finally
    {
        CMDirUnlock( psDirDst );
        CMDirUnlock( psDirSrc );

        if ( bResult )
		{
			
			NTSVCInfo("%s | %s | [%s]:%s | [%s]:%s",
                TextFind( gsSvcWork.hText, "INF_TRANSMIT", "INF_TRANSMIT" ),
                szName,
                psDirSrc->psHost->szName, psDirSrc->szPath, 
                psDirDst->psHost->szName, psDirDst->szPath );
        }
        else
        {
			NTSVCError("CMDirTransmit() : Erreur durant la transmission de %s de [%s]:%s à [%s]:%s / %s",
                szName,
                psDirSrc->psHost->szName, psDirSrc->szPath, 
                psDirDst->psHost->szName, psDirDst->szPath,
                szErr );
        }       
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMDirMove( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: psDirSrc : Pointeur sur une struture descriptive d'un répertoire initialisée (source)
 *             psDirDst : Pointeur sur une struture descriptive d'un répertoire initialisée (destination)
 *                        ou NULL pour un effacement.
 *             szName   : Nom du fichier.
 *             bCopy    : TRUE pour effectuer une copie, FALSE pour un déplacement.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le deplacement/copie/effacement a eu lieu sans erreur, FALSE sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une copie, un déplacement ou un effacement de fichier.
 *             En cas de copie ou de déplacement, si le fichier destination existe déjà,
 *             la fonction échoue.
 *             En cas de copie ou de déplacement, l'host de la source doit être le même
 *             que celui de la destination.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMDirMove( CM_DIR * psDirSrc, CM_DIR * psDirDst, char * szName, BOOL bCopy )
{
    DWORD dwIndexSrc;
    DWORD dwIndexDst;
    BOOL bResult = TRUE;
    CM_HOST * psHostSrc;
    CM_HOST * psHostDst;
    char szPathSrc[CM_MAX_STR];
    char szPathDst[CM_MAX_STR];
    char szErr[CM_MAX_STR] = "";

    __try
    {
        //
        // Attention, dans tout ce qui suit, l'utilisation de psDirDst doit
        // être conditionné au fait que la valeur est non nulle. Si la
        // valeur est nulle, il s'agit en fait d'une demande d'effacement.
        //

        CMDirLock( psDirSrc );
        if ( psDirDst != NULL ) CMDirLock( psDirDst );

        //
        // Pointer directement sur les hosts correspondant et s'assurer
        // que ceux-ci sont identiques.
        //
        psHostSrc = psDirSrc->psHost;
        if ( psDirDst != NULL ) 
            psHostDst = psDirDst->psHost;
        else
            psHostDst = NULL;

        if ( ( psHostSrc != psHostDst ) && ( psHostDst != NULL ) )
        {
            bResult = FALSE;
			strcpy_s(szErr, sizeof(szErr), "Hosts source et destination sont différents");
            __leave;
        }
    
        dwIndexSrc = _CMDirFind( psDirSrc, szName );
        if ( dwIndexSrc == CM_INVALID )
        {
            //
            // Le fichier ne se trouve pas dans le répertoire source
            // ou l'host est offline
            //
            bResult = FALSE;
			strcpy_s(szErr, sizeof(szErr), "Fichier source absent ou répertoire source offline");
            __leave;        
        }

        if ( psDirDst != NULL )
        {
            dwIndexDst = _CMDirFind( psDirDst, szName );
            if ( dwIndexDst == CM_INVALID )
            {
                //
                // Le fichier ne se trouve pas dans le répertoire destination.
                // S'assurer que c'est bien du à son absence et pas à
                // cause de l'état offline du répertoire.
                //
                if ( GetLastError() != ERROR_FILE_NOT_FOUND )
                {
                    bResult = FALSE;
					strcpy_s(szErr, sizeof(szErr), "Répertoire destination offline");
                    __leave;
                }
            }
            else
            {
                //
                // Le fichier existe déjà, ce n'est pas la peine de le copier.
                //
                bResult = TRUE;
                __leave;
            }
        }

        //
        // Construire les chemin des fichiers source et destination puis
        // effectuer la copie distance
        //
        CMMakePath( psDirSrc->szPath, szName, szPathSrc, sizeof(szPathSrc) );
        if ( psDirDst != NULL )
        {
            CMMakePath( psDirDst->szPath, szName, szPathDst, sizeof(szPathDst) );
            bResult = CMHostCnxMove( psHostSrc, szPathSrc, szPathDst, bCopy );
        }
        else
        {
            bResult = CMHostCnxMove( psHostSrc, szPathSrc, NULL, bCopy );
        }
        if ( ! bResult )
        {
			strcpy_s(szErr, sizeof(szErr), "Echec de l'opération i/o");
            __leave;
        }

        if ( psDirDst != NULL )
        {
            //
            // Ajouter le fichier à la liste du répertoire destination
            //
            bResult = _CMDirAdd( psDirDst, &psDirSrc->psFiles[dwIndexSrc] );
            if ( ! bResult )
            {
                psDirDst->dwState = CM_STATE_ERROR;
                psDirDst->dwChanges = 0;
				strcpy_s(szErr, sizeof(szErr), "Echec sur mise à jour du répertoire virtuel destination");
                __leave;
            }
        }

        if ( ( psDirDst == NULL ) || ! bCopy )
        {
            //
            // Supprimer le fichier de la liste du répertoire source
            //
            bResult = _CMDirRemove( psDirSrc, szName );
            if ( ! bResult )
            {
                psDirSrc->dwState = CM_STATE_ERROR;
                psDirSrc->dwChanges = 0;
				strcpy_s(szErr, sizeof(szErr), "Echec sur mise à jour du répertoire virtuel source");
                __leave;
            }
        }
    }
    __finally
    {
        if ( psDirDst != NULL ) CMDirUnlock( psDirDst );
        CMDirUnlock( psDirSrc );

        if ( bResult )
        {
            if ( psDirDst == NULL )
                NTSVCInfo( "%s | %s | [%s]:%s",
                    TextFind( gsSvcWork.hText, "INF_DELETE", "INF_DELETE" ),
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath );

            else if ( ! bCopy )
                NTSVCInfo( "%s | %s | [%s]:%s | [%s]:%s",
                    TextFind( gsSvcWork.hText, "INF_MOVE", "INF_MOVE" ),
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath, 
                    psDirDst->psHost->szName, psDirDst->szPath );

            else
                NTSVCInfo( "%s | %s | [%s]:%s | [%s]:%s",
                    TextFind( gsSvcWork.hText, "INF_COPY", "INF_COPY" ),
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath, 
                    psDirDst->psHost->szName, psDirDst->szPath );
        }
        else
        {
            if ( psDirDst == NULL )
				NTSVCError( "CMDirMove() : Erreur durant l'effacement de %s de [%s]:%s / %s", 
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath,
                    szErr );

            else if ( ! bCopy )
                NTSVCError( "CMDirMove() : Erreur durant le déplacement de %s de [%s]:%s vers [%s]:%s / %s", 
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath, 
                    psDirDst->psHost->szName, psDirDst->szPath,
                    szErr );

            else
                NTSVCError( "CMDirMove() : Erreur durant la copie de %s de [%s]:%s vers [%s]:%s / %s", 
                    szName,
                    psDirSrc->psHost->szName, psDirSrc->szPath, 
                    psDirDst->psHost->szName, psDirDst->szPath,
                    szErr );
        }
    }

	return bResult;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BOOL CMAnalyseSortParams( char * szParams, CM_SORT * psSort )
 * --------------------------------------------------------------------
 * PARAMETERS: szParam : Paramètre de tri
 *             psSort  : structure de contexte de tri
 * --------------------------------------------------------------------
 * RETURN    : TRUE si OK, FALSE si erreur
 * --------------------------------------------------------------------
 * ROLE      : Préparation d'un contexte de tri en fonction d'une chaine
 *             de paramètres.
 * --------------------------------------------------------------------
 */
PROTECTED BOOL CMAnalyseSortParams( char * szParams, CM_SORT * psSort )
{
    BOOL  fSuccess = FALSE;
    DWORD dwOffset;

    __try
    {
        if      ( _stricmp( szParams, "INC:NAME" ) == 0 )
            psSort->dwMethod = CM_SORT_INCREASING | CM_SORT_NAME;
        else if ( _stricmp( szParams, "DEC:NAME" ) == 0 )
            psSort->dwMethod = CM_SORT_DECREASING | CM_SORT_NAME;
        else if ( _stricmp( szParams, "INC:DATE" ) == 0 )
            psSort->dwMethod = CM_SORT_INCREASING | CM_SORT_DATE;
        else if ( _stricmp( szParams, "DEC:DATE" ) == 0 )
            psSort->dwMethod = CM_SORT_DECREASING | CM_SORT_DATE;
        else if ( _strnicmp( szParams, "INC:NBR:", 8 ) == 0 )
        {
            psSort->dwMethod = CM_SORT_INCREASING | CM_SORT_NUMBER;
            psSort->fLoop = FALSE;
            dwOffset = 8;
        }
        else if ( _strnicmp( szParams, "DEC:NBR:", 8 ) == 0 )
        {
            psSort->dwMethod = CM_SORT_DECREASING | CM_SORT_NUMBER;
            psSort->fLoop = FALSE;
            dwOffset = 8;
        }
        else if ( _strnicmp( szParams, "INC:LNBR:", 9 ) == 0 )
        {
            psSort->dwMethod = CM_SORT_INCREASING | CM_SORT_NUMBER;
            psSort->fLoop = TRUE;
            dwOffset = 9;
        }
        else if ( _strnicmp( szParams, "DEC:LNBR:", 9 ) == 0 )
        {
            psSort->dwMethod = CM_SORT_DECREASING | CM_SORT_NUMBER;
            psSort->fLoop = TRUE;
            dwOffset = 9;
        }
        else
            __leave;

        if ( ( psSort->dwMethod & CM_SORT_NUMBER ) != 0 )
        {
            strncpy( psSort->szNbrMask,  
                     szParams + dwOffset,
                     sizeof(psSort->szNbrMask) );
            psSort->szNbrMask[sizeof(psSort->szNbrMask)-1] = 0;

            psSort->ullNbrMax = _CMSortFileNbr( psSort->szNbrMask, SORT_MAX_FILE_NAME );

            psSort->ullNbrFirstQuarter = psSort->ullNbrMax / (ULONGLONG)4;
            psSort->ullNbrLastQuarter = psSort->ullNbrMax - psSort->ullNbrFirstQuarter;
        }

        fSuccess = TRUE;
    }   
    __finally
    {

    }

	return fSuccess;
}


//
// CODE PRIVE
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE int __cdecl _CMSortCompare( const void * pvItem1, const void * pvItem2 )
 * --------------------------------------------------------------------
 * PARAMETERS: cf. doc. runtimes microsoft
 * --------------------------------------------------------------------
 * RETURN    : cf. doc. runtimes microsoft
 * --------------------------------------------------------------------
 * ROLE      : fonction callback utilisée par qsort.
 * --------------------------------------------------------------------
 */
PRIVATE int __cdecl _CMSortCompare( const void * pvItem1, const void * pvItem2 )
{
    CM_DIR     * psDir = (CM_DIR*)TlsGetValue( gsSvcWork.dwTLS );
    register int iResult;
    ULONGLONG    ullNbr1;
    ULONGLONG    ullNbr2;
    BOOL         fComp = FALSE;
    
    if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_NAME )
        iResult = _strnicmp( ((CM_FILE*)pvItem1)->szName,
                            ((CM_FILE*)pvItem2)->szName,
                            sizeof(((CM_FILE*)pvItem1)->szName) );
    else if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_DATE )
        iResult = ((CM_FILE*)pvItem1)->ullLastWrite < ((CM_FILE*)pvItem2)->ullLastWrite 
                ? - 1
                : ( ((CM_FILE*)pvItem1)->ullLastWrite > ((CM_FILE*)pvItem2)->ullLastWrite
                  ? 1
                  : 0 );
    else if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_NUMBER )
    {
        ullNbr1 = ((CM_FILE*)pvItem1)->ullScore;
        ullNbr2 = ((CM_FILE*)pvItem2)->ullScore;

        if ( psDir->sSort.fLoop )
        {
            if ( ( psDir->sSort.fFirstQuarter ) &&
                 ( ! psDir->sSort.fMiddle     ) &&
                 ( psDir->sSort.fLastQuarter  ) )
            {
                if ( ( ullNbr1 < psDir->sSort.ullNbrFirstQuarter ) &&
                     ( ullNbr2 > psDir->sSort.ullNbrLastQuarter  ) )
                    iResult = 1;
                else if ( ( ullNbr2 < psDir->sSort.ullNbrFirstQuarter ) &&
                          ( ullNbr1 > psDir->sSort.ullNbrLastQuarter  ) )
                    iResult = -1;
                else
                    fComp = TRUE;
            }
            else
                fComp = TRUE;
        }
        else
            fComp = TRUE;

        if ( fComp )
        {
            iResult = ullNbr1 < ullNbr2
                    ? -1
                    : ( ullNbr1 > ullNbr2
                      ? 1
                      : 0 );
        }
    }
    
    if ( ( psDir->sSort.dwMethod & CM_SORT_ORDER ) == CM_SORT_DECREASING )
        iResult = -iResult;

    return iResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL _CMDirAdd( CM_DIR * psDir, CM_FILE * psFile )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             psFile : Structure contenant les infos de fichier à rajouter à la liste des fichiers.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le fichier a été ajouté. FALSE en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un fichier à la liste image des fichier d'un répertoire.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL _CMDirAdd( CM_DIR * psDir, CM_FILE * psFile )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwNewFiles;
    DWORD dwNewMaxFiles;
    DWORD dwOldPriority;
    CM_FILE * psNewFiles;
    CM_FILE * psNewFile;

    __try
    {
        //
        // Mémoriser la priorité actuelle et passer dans la priorité la plus
        // basse dans la mesure où il ne s'agit que de calcul pur.
        //
        dwOldPriority = GetThreadPriority( GetCurrentThread() );
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

        //
        // S'assurer que l'état du répertoire est ok
        //
        if ( psDir->dwState == CM_STATE_ERROR )
        {
            dwErr = ERROR_NOT_READY;
            __leave;
        }

        //
        // Rechercher un éventuel doublon et reporter une erreur
        // si le fichier existe déjà. En sortie de boucle, la
        // valeur de l'index correpond à la position ou le nouveau
        // fichier doit être ajouté.
        //
        for ( dwIndex = 0 ; dwIndex < psDir->dwFiles ; dwIndex ++ )
            if ( _stricmp( psDir->psFiles[dwIndex].szName, psFile->szName ) == 0 )
            {
                dwErr = ERROR_FILE_EXISTS;
                __leave;
            }

        //
        // Vérifier que si le nombre actuel de fichiers plus un
        // est trop grand pour rentrer dans le tableau alloué.
        //
        if ( ( psDir->dwFiles + 1 ) > psDir->dwMaxFiles )
        {
            //
            // Le nouveau fichier ne rentre pas.
            //

            //
            // Calculer le nouveau nombre de fichiers et la nouvelle
            // taille de tableau nécessaires. Pour le tableau, on prend
            // en compte un marge d'évolution de CM_LIST_INCREMENT fichiers.
            //
            dwNewFiles = psDir->dwFiles + 1;
            dwNewMaxFiles = psDir->dwFiles + CM_LIST_INCREMENT;

            //
            // Allouer un tableau à la nouvelle taille.
            //
            psNewFiles = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(psNewFiles[0]) * dwNewMaxFiles);
            if ( psNewFiles == NULL )
            {
                dwErr =  ERROR_NOT_ENOUGH_MEMORY;
                __leave;
            }

            //
            // Si des fichiers sont déjà présents dans le tableau existant,
            // on effectue la recopie de ceux-ci dans le nouveau tableau.
            //
            if ( psDir->dwFiles > 0 )
                memcpy( psNewFiles, psDir->psFiles, sizeof(psNewFiles[0]) * psDir->dwFiles );

            //
            // Si le tableau existant a été alloué, on le désalloue
            //
            if ( psDir->psFiles != NULL )
                HeapFree( GetProcessHeap(), 0, psDir->psFiles );

            psDir->psFiles = psNewFiles;
            psDir->dwMaxFiles = dwNewMaxFiles;
            psDir->dwFiles = dwNewFiles;
        }
        else
        {
            //
            // Le nouveau rentre dans le tableau existant. On ne
            // fait donc qu'ajuster le nombre de fichiers dans le tableau.
            //
            psDir->dwFiles ++;
        }

        //
        // Recopier les données du fichier dans l'élément du tableau
        //
        psNewFile = &psDir->psFiles[dwIndex];
        *psNewFile = *psFile;

        if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_NUMBER )
        {
            psNewFile->ullScore = _CMSortFileNbr( psDir->sSort.szNbrMask, psNewFile->szName );
            if ( psNewFile->ullScore < psDir->sSort.ullNbrFirstQuarter )
                psDir->sSort.fFirstQuarter = TRUE;
            else if ( psNewFile->ullScore > psDir->sSort.ullNbrLastQuarter )
                psDir->sSort.fLastQuarter = TRUE;
            else
                psDir->sSort.fMiddle = TRUE;
        }

        //
        // Trier les fichiers dans l'ordre croissant.
        //
        TlsSetValue( gsSvcWork.dwTLS, (void*)psDir );
        qsort( psDir->psFiles, psDir->dwFiles, sizeof(psDir->psFiles[0]), _CMSortCompare );

        psDir->dwChanges ++;
    }
    __finally
    {
        //
        // Restaurer la priorité mémorisée
        //
        SetThreadPriority( GetCurrentThread(), dwOldPriority );

        SetLastError( dwErr );
    }

	return dwErr == NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL _CMDirRemove( CM_DIR * psDir, char * szName )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szName : Nom de fichier à élimier de la liste des fichiers.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le fichier a été enlevé. FALSE en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Enlève un fichier de la liste image des fichier d'un répertoire.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL _CMDirRemove( CM_DIR * psDir, char * szName )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwOldPriority;
    BOOL bFound;

    __try
    {
        //
        // Mémoriser la priorité actuelle et passer dans la priorité la plus
        // basse dans la mesure où il ne s'agit que de calcul pur.
        //
        dwOldPriority = GetThreadPriority( GetCurrentThread() );
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

        //
        // S'assurer que l'état du répertoire est ok
        //
        if ( psDir->dwState == CM_STATE_ERROR )
        {
            dwErr = ERROR_NOT_READY;
            __leave;
        }

        //
        // Rechercher le fichier dans la boucle et à partir du moment où il a été
        // trouvé, décaler les fichiers suivants.
        //
        for ( bFound = FALSE, dwIndex = 0 ; dwIndex < psDir->dwFiles ; dwIndex ++ )
        {
            if ( bFound )
                psDir->psFiles[dwIndex-1] = psDir->psFiles[dwIndex];
            else
                bFound = ( _stricmp( psDir->psFiles[dwIndex].szName, szName ) == 0 );
        }

        if ( !bFound )
        {
            dwErr = ERROR_FILE_NOT_FOUND;
            __leave;
        }

        //
        // Décrementer le nombre de fichiers
        //
        psDir->dwFiles --;

        //
        // Rafraichir les booléens de répartition si nécessaire.
        //
        if ( ( psDir->sSort.dwMethod & CM_SORT_CRITERIA ) == CM_SORT_NUMBER )
        {
            psDir->sSort.fFirstQuarter = FALSE;
            psDir->sSort.fMiddle = FALSE;
            psDir->sSort.fLastQuarter = FALSE;
            for ( dwIndex = 0 ; dwIndex < psDir->dwFiles ; dwIndex ++ )
                if ( psDir->psFiles[dwIndex].ullScore < psDir->sSort.ullNbrFirstQuarter )
                    psDir->sSort.fFirstQuarter = TRUE;
                else if ( psDir->psFiles[dwIndex].ullScore > psDir->sSort.ullNbrLastQuarter )
                    psDir->sSort.fLastQuarter = TRUE;
                else
                    psDir->sSort.fMiddle = TRUE;
        }

        //
        // Il est inutile de trier les fichiers dans la mesure où
        // on est parti d'une liste triée dans laquelle on a effectué
        // une suppression puis un décalage. L'ordre est donc conservé.
        //

        psDir->dwChanges ++;
    }
    __finally
    {
        //
        // Restaurer la priorité mémorisée
        //
        SetThreadPriority( GetCurrentThread(), dwOldPriority );

        SetLastError( dwErr );
    }

	return dwErr == NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD _CMDirFind( CM_DIR * psDir, char * szName )
 * --------------------------------------------------------------------
 * PARAMETERS: psDir  : Pointeur sur une struture descriptive d'un répertoire initialisée
 *             szName : Nom de fichier à rechercher dans la liste des fichiers.
 * --------------------------------------------------------------------
 * RETURN    : CM_INVALID si le fichier n'a pas été trouvé. Sinon, la fonction
 *             retourne l'index du fichier.
 *             Si le fichier n'a pas été trouvé, GetLastError() permet d'en
 *             connaitre la raison :
 *                ERROR_NOT_READY      : l'image du répertoire n'est pas prète.
 *                ERROR_FILE_NOT_FOUND : le fichier n'existe pas.
 * --------------------------------------------------------------------
 * ROLE      : Enlève un fichier de la liste image des fichier d'un répertoire.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD _CMDirFind( CM_DIR * psDir, char * szName )
{
    DWORD dwErr = NO_ERROR;
    DWORD dwIndex;
    DWORD dwOldPriority;
    BOOL bFound;

    __try
    {
        //
        // Mémoriser la priorité actuelle et passer dans la priorité la plus
        // basse dans la mesure où il ne s'agit que de calcul pur.
        //
        dwOldPriority = GetThreadPriority( GetCurrentThread() );
        SetThreadPriority( GetCurrentThread(), THREAD_PRIORITY_IDLE );

        //
        // S'assurer que l'état du répertoire est ok
        //
        if ( psDir->dwState == CM_STATE_ERROR )
        {
            dwErr = ERROR_NOT_READY;
            __leave;
        }

        //
        // Rechercher le fichier dans la boucle.
        //
        for ( bFound = FALSE, dwIndex = 0 ; dwIndex < psDir->dwFiles ; dwIndex ++ )
        {
            bFound = ( _stricmp( psDir->psFiles[dwIndex].szName, szName ) == 0 );
            if ( bFound )
                break;
        }

        if ( ! bFound )
            dwErr = ERROR_FILE_NOT_FOUND;
    }
    __finally
    {
        //
        // Restaurer la priorité mémorisée
        //
        SetThreadPriority( GetCurrentThread(), dwOldPriority );

        SetLastError( dwErr );
    }

	return (dwErr == NO_ERROR ? dwIndex : CM_INVALID);
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD _CMSortFileNbr( char * szNbrMask, char * szFileName )
 * --------------------------------------------------------------------
 * PARAMETERS: szNbrMask : masque d'identification des digits
 *             szFileName : nom du fichier
 * --------------------------------------------------------------------
 * RETURN    : Value du score
 * --------------------------------------------------------------------
 * ROLE      : Effectue un scoring sur un nom de fichier.
 * --------------------------------------------------------------------
 */
PRIVATE ULONGLONG _CMSortFileNbr( char * szNbrMask, char * szFileName )
{
    char * pcMask;
    char * pcFile;
    char cMask;
    char cFile;
    ULONGLONG ullValue = 0;

    pcMask = szNbrMask;
    pcFile = szFileName;

    while ( ( cMask = *(pcMask++) ) != 0 )
    {
        if ( pcFile == SORT_MIN_FILE_NAME )
            cFile = '0';
        else if ( pcFile == SORT_MAX_FILE_NAME )
            cFile = 'F';
        else
        {
            cFile = *pcFile;
            if ( cFile == 0 )
                cFile = '0';
            else
                pcFile ++;
        }

        if ( cMask == '9' )
        {
            if ( ( cFile >= '0' ) && ( cFile <= '9' ) )
                ullValue = ( ullValue * (ULONGLONG)10 ) + (ULONGLONG)(cFile - '0');
            else
                ullValue = ( ullValue * (ULONGLONG)10 ) + (ULONGLONG)9;
        }
        else if ( ( cMask == 'F' ) || ( cMask == 'f' ) )
        {
            if ( ( cFile >= '0' ) && ( cFile <= '9' ) )
                ullValue = ( ullValue * (ULONGLONG)16 ) + (ULONGLONG)(cFile - '0');
            else if ( ( cFile >= 'A' ) && ( cFile <= 'F' ) )
                ullValue = ( ullValue * (ULONGLONG)16 ) + (ULONGLONG)(cFile - 'A') + (ULONGLONG)10;
            else if ( ( cFile >= 'a' ) && ( cFile <= 'f' ) )
                ullValue = ( ullValue * (ULONGLONG)16 ) + (ULONGLONG)(cFile - 'a') + (ULONGLONG)10;
            else
                ullValue = ( ullValue * (ULONGLONG)16 ) + (ULONGLONG)15;
        }
    }

    return ullValue;
}

