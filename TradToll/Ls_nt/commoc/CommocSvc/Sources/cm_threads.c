/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_treads.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Thread de gestion des transferts
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <col.h>
#include <reg.h>
#include <acom.h>
#include <ntsvc.h>

#include <cmhost.h>
#include <cmwork.h>
#include <cm_glob.h>
#include <cm_dirs.h>
#include <cm_hosts.h>
#include <cm_utl.h>
#include <cm_text.h>

#define LOC_DEF
#include <cm_threads.h>
#undef LOC_DEF

#include <memclass.h>

//
// REMARQUE TRES IMPORTANTES SUR LES TRAITEMENTS :
// Pour éviter tout deadlock à cause des vérous sur les répertoires,
// ceux-ci devront toujours être locké DANS LEUR ORDRE ALPHABÉTIQUE.
//




//
// PROTOTYPES PRIVES
//

PRIVATE void DoExport( CM_LINK * psLink );
PRIVATE void DoImport( CM_LINK * psLink );
PRIVATE void PurgeDir( CM_DIR * psDir, DWORD dwMaxFiles, DWORD dwMaxKB );



//
// CODE DES FONCTIONS PROTEGEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadLink( DWORD dwLink )
 * --------------------------------------------------------------------
 * PARAMETERS: dwLink : Index du lien que le thread doit gérer
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion d'un lien COMMOC.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadLink( DWORD dwLink )
{
    BOOL bChoice = FALSE;
    CM_LINK * psLink = &gsSvcWork.tsLinks[dwLink];

    NTSVCInfo( "CMThreadLink(), Démarrage du thread de gestion du lien %s", psLink->szName );

    while ( NTSVCWaitForEnd( psLink->dwPollingPeriod ) == WAIT_TIMEOUT )
    {
        if ( ! gsSvcWork.fPaused )
        {
            bChoice = ! bChoice;
            bChoice ? DoExport( psLink ) : DoImport( psLink );
            if ( NTSVCWaitForEnd( 0 ) != WAIT_TIMEOUT )
            bChoice ? DoImport( psLink ) : DoExport( psLink );
        }
    }

    NTSVCInfo( "CMThreadLink(), Arrêt du thread de gestion du lien %s", psLink->szName );

    ExitThread( NO_ERROR );
    return NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadDispatch( DWORD dwFoo )
 * --------------------------------------------------------------------
 * PARAMETERS: dwFoo : Non utilisé
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion du répertoire de
 *             dispatching.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadDispatch( DWORD dwFoo )
{
    DWORD dwIndex;
    DWORD dwLink;
    DWORD dwTotalCount;
    DWORD dwAckCount;
    BOOL bContinue;
    CM_LINK * psLink;
    CM_DIR * psDirEA   = &gsSvcWork.sDirEA;
    CM_DIR * psDirEAB;
    CM_DIR * psDirEABS;
    char  szFile[CM_MAX_STR];
	
    NTSVCInfo( "CMThreadDispatch(), Démarrage du thread de dispatching" );

    while ( NTSVCWaitForEnd( gsSvcWork.dwDispatchPeriod ) == WAIT_TIMEOUT )
    {
        if ( gsSvcWork.fPaused )
            continue;

        //
        // Rafraichir le contenu du répertoire EA. Le contenu des autres
        // répertoires étant rafraichi par les taches de gestion des links.
        //
        CMDirRefresh( psDirEA );

        CMDirLock( psDirEA );
        
        //
        // Etapes E100 et E500 (partiellement) combinées
        // Etape E100 : Dispatching des fichiers de EA vers les EAB
        //              Pour tout fichier de EA, pour chaque link
        //              pour lequel le fichier n'existe ni dans EAB,
        //              ni dans EABS, copier le fichier de EA vers EAB.
        // Etape E500 : (E501 à E505) acquittement de transfert.
        //              Pour tout fichier de EA, si pour tous les links
        //              en export, le fichier existe dans EABS, supprimer
        //              le fichier de EA.
        //
        do
        {
            for ( dwLink = 0 ; dwLink < gsSvcWork.dwLinks ; dwLink ++ )
                gsSvcWork.tsLinks[dwLink].fFlag = FALSE;

            for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEA->dwFiles ; dwIndex ++ )
            {
                if ( psDirEA->dwState  == CM_STATE_ERROR )
                     break;

                CM_STRNCPY( szFile, psDirEA->psFiles[dwIndex].szName, sizeof(szFile) );

                dwTotalCount = 0;
                dwAckCount = 0;
                for ( dwLink = 0 ; dwLink < gsSvcWork.dwLinks ; dwLink ++ )
                {
                    psLink = &gsSvcWork.tsLinks[dwLink];
                    if ( psLink->fFlag )
                        continue;

                    if ( psLink->bExport || psLink->bDispatch )
                    {
                        dwTotalCount ++;

                        psDirEAB  = &psLink->sDirEAB;
                        psDirEABS = &psLink->sDirEABS;

                        CMDirLock( psDirEAB );
                        CMDirLock( psDirEABS );

                        if ( ( psDirEA->dwState    == CM_STATE_ERROR ) ||
                             ( psDirEAB->dwState   == CM_STATE_ERROR ) ||
                             ( psDirEABS->dwState  == CM_STATE_ERROR )
                           )
                        {
                            CMDirUnlock( psDirEABS );
                            CMDirUnlock( psDirEAB );
                            dwTotalCount = 0;
                            psLink->fFlag = ( psLink->dwEnsureOrder != 0 );
                            break;
                        }

                        if ( CMDirExist( psDirEABS, szFile ) == CM_EXISTS )
                            dwAckCount ++;

                        if ( ( CMDirExist( psDirEAB , szFile ) == CM_NOT_FOUND ) &&
                             ( CMDirExist( psDirEABS, szFile ) == CM_NOT_FOUND )
                           )
                        {
                            if ( ! CMDirMove( psDirEA, psDirEAB, szFile, TRUE ) )
                            {
                                CMDirUnlock( psDirEABS );
                                CMDirUnlock( psDirEAB );
                                dwTotalCount = 0;
                                psLink->fFlag = ( psLink->dwEnsureOrder != 0 );
                                break;
                            }
                        }
                        CMDirUnlock( psDirEABS );
                        CMDirUnlock( psDirEAB );
                    }
                }

                if ( ( dwTotalCount != 0 ) && ( dwAckCount == dwTotalCount ) )
                {
                    bContinue = CMDirMove( psDirEA, NULL, szFile, FALSE );
                    break;
                }
            }
        }
        while ( bContinue );

        // Etape E500 : (E506 à E509) Nettoyage acquittement de transfert.
        //              Pour tous les links, pour tout fichier de EABS, si
        //              le fichier n'existe pas dans EA, supprimer de EABS.
        //
        for ( dwLink = 0 ; dwLink < gsSvcWork.dwLinks ; dwLink ++ )
        {
            psLink = &gsSvcWork.tsLinks[dwLink];
            if ( psLink->bExport || psLink->bDispatch )
            {
                psDirEABS = &psLink->sDirEABS;
                CMDirLock( psDirEABS );
                do
                {
                    for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEABS->dwFiles ; dwIndex ++ )
                    {
                        if ( ( psDirEA->dwState    == CM_STATE_ERROR ) ||
                             ( psDirEABS->dwState   == CM_STATE_ERROR ) )
                             break;

                        CM_STRNCPY( szFile, psDirEABS->psFiles[dwIndex].szName, sizeof(szFile) );
                        
                        if ( CMDirExist( psDirEA, szFile ) == CM_NOT_FOUND )
                        {
                            bContinue = CMDirMove( psDirEABS, NULL, szFile, FALSE );
                            break;
                        }
                    }
                }
                while ( bContinue );
                CMDirUnlock( psDirEABS );
            }
        }

        CMDirUnlock( psDirEA );
    }

    NTSVCInfo( "CMThreadDispatch(), Arrêt du thread de dispatching" );

    ExitThread( NO_ERROR );
    return NO_ERROR;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD WINAPI CMThreadWorker( DWORD dwWorker )
 * --------------------------------------------------------------------
 * PARAMETERS: dwWorker : Index du worker que le thread doit gérer
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entrée du thread de gestion d'un worker.
 *             Le thread s'arréte sur signalement de l'événemement
 *             d'arret du service.
 * --------------------------------------------------------------------
 */
PROTECTED DWORD WINAPI CMThreadWorker( DWORD dwWorker )
{
    DWORD dwErr;
    DWORD dwIndex;
    DWORD dwStatus;
    DWORD dwInc;
    BOOL  fRetry;
    CM_WORKER * psWorker = &gsSvcWork.tsWorkers[dwWorker];
    CM_DIR * psDirRAB   = &gsSvcWork.tsWorkers[dwWorker].sDirRAB;
    CM_DIR * psDirRABS  = &gsSvcWork.tsWorkers[dwWorker].sDirRABS;
    CM_DIR * psDirRABX  = gsSvcWork.tsWorkers[dwWorker].fSXIdentical 
                          ? &gsSvcWork.tsWorkers[dwWorker].sDirRABS
                          : &gsSvcWork.tsWorkers[dwWorker].sDirRABX;
    CM_FILE * psFile;
    CM_FILE * psFound;
    char  szFile[CM_MAX_STR];
    char  szFilePath[CM_MAX_STR];

    NTSVCInfo( "CMThreadDispatch(), Démarrage du thread worker %s", psWorker->szName );

	while (NTSVCWaitForEnd(psWorker->dwPollingPeriod) == WAIT_TIMEOUT)
	{
		if (gsSvcWork.fPaused)
			continue;

		CMDirRefresh(psDirRAB);
		CMDirRefresh(psDirRABS);
		if (!psWorker->fSXIdentical)
			CMDirRefresh(psDirRABX);

		if ((psDirRAB->dwState == CM_STATE_ERROR) ||
			(psDirRABS->dwState == CM_STATE_ERROR))
			continue;

		if (!psWorker->fSXIdentical)
			if (psDirRABX->dwState == CM_STATE_ERROR)
				continue;

		dwInc = 1;

		for (dwIndex = 0; dwIndex < psDirRAB->dwFiles; dwIndex += dwInc)
		{
			dwInc = 1;

			psFile = &psDirRAB->psFiles[dwIndex];
			CM_STRNCPY(szFile, psFile->szName, sizeof(szFile));

			psFound = CMDirFind(psDirRABS, szFile, FALSE);
			if (psFound != NULL)
			{
				if ((psWorker->dwInAndAckDelete != 0) &&
					(psWorker->dwInAndAckDelete != 2))
				{
					NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ACK), il doit être effacé du répertoire de réception", psWorker->szName, szFile);
					if (!CMDirMove(psDirRAB, NULL, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s", psWorker->szName, szFile);
					}
					else
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s", psWorker->szName, szFile);
						dwInc = 0;
					}

					if (psWorker->dwEnsureOrder != 0)
						break;
					else
						continue;
				}
				else
				{
					if ((psFile->ullSize == psFound->ullSize) &&
						(psFile->ullLastWrite == psFound->ullLastWrite))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ACK), il doit être effacé", psWorker->szName, szFile);
						if (!CMDirMove(psDirRAB, NULL, szFile, FALSE))
						{
							NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s", psWorker->szName, szFile);
						}
						else
						{
							NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s", psWorker->szName, szFile);
							dwInc = 0;
						}
					}
					else
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ACK) dans une version différente, la version acquitée doit être effacée", psWorker->szName, szFile);
						if (!CMDirMove(psDirRABS, NULL, szFile, FALSE))
						{
							NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s (ACK)", psWorker->szName, szFile);
						}
						else
						{
							NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s (ACK)", psWorker->szName, szFile);
						}
					}
				}

				if (psWorker->dwEnsureOrder != 0)
					break;
				else
					continue;
			}

			psFound = CMDirFind(psDirRABX, szFile, FALSE);
			if ((!psWorker->fSXIdentical) && (psFound != NULL))
			{
				if ((psWorker->dwInAndErrDelete != 0) &&
					(psWorker->dwInAndErrDelete != 2))
				{
					NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ERR), il doit être effacé", psWorker->szName, szFile);
					if (!CMDirMove(psDirRAB, NULL, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s", psWorker->szName, szFile);
					}
					else
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s", psWorker->szName, szFile);
						dwInc = 0;
					}

					if (psWorker->dwEnsureOrder != 0)
						break;
					else
						continue;
				}
				else
				{
					if ((psFile->ullSize == psFound->ullSize) &&
						(psFile->ullLastWrite == psFound->ullLastWrite))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ERR), il doit être effacé", psWorker->szName, szFile);
						if (!CMDirMove(psDirRAB, NULL, szFile, FALSE))
						{
							NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s", psWorker->szName, szFile);
						}
						else
						{
							NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s", psWorker->szName, szFile);
							dwInc = 0;
						}
					}
					else
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s, mais celui-ci a déjà été acquité (ERR) dans une version différente, la version acquitée doit être effacée", psWorker->szName, szFile);
						if (!CMDirMove(psDirRABX, NULL, szFile, FALSE))
						{
							NTSVCInfo("CMThreadWorker(), le worker %s n'a pu effacer le fichier %s (ERR)", psWorker->szName, szFile);
						}
						else
						{
							NTSVCInfo("CMThreadWorker(), le worker %s a effacé le fichier %s (ERR)", psWorker->szName, szFile);
						}
					}
				}

				if (psWorker->dwEnsureOrder != 0)
					break;
				else
					continue;
			}

			NTSVCInfo("CMThreadWorker(), le worker %s a détecté le fichier %s", psWorker->szName, szFile);
			CMMakePath(psDirRAB->szPath, szFile, szFilePath, sizeof(szFilePath));

			if (psWorker->pfProcess != NULL)
			{
				if (psWorker->pfProcess(psWorker->hWrk, szFilePath))
				{
					NTSVCInfo("%s | %s | [%s]:%s",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_OK", "INF_PROCESS_FILE_OK"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath);

					if (!CMDirMove(psDirRAB, psDirRABS, szFile, FALSE))
					{
						NTSVCError("CMThreadWorker(), le worker %s a traité le fichier %s, mais n'a pas pu déplacer dans le répertoire d'acquittement", psWorker->szName, szFile);
						break;
					}
					dwInc = 0;
					NTSVCInfo("CMThreadWorker(), le worker %s a effectué le traitement du fichier %s et l'a déplacé dans le répertoire d'acquittement", psWorker->szName, szFile);
				}
				else
				{
					dwErr = GetLastError();
					NTSVCError("%s | #ERR%u# | %s | [%s]:%s",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_ERROR", "INF_PROCESS_FILE_ERROR"),
						dwErr,
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath);
				}
			}
			else
			{
				fRetry = FALSE;
				dwStatus = psWorker->pfProcessEx(psWorker->hWrk, szFilePath, NULL);

				dwErr = GetLastError();
				if (dwErr != NO_ERROR)
				{
					NTSVCInfo("CMThreadWorker(), le traitement a retourné l'erreur système %u", dwErr);
				}

				if ((dwStatus & CM_WORKER_ACK_MASK) == CM_WORKER_ACK_NONE)
				{
					NTSVCInfo("CMThreadWorker(), le worker %s a ignoré le fichier %s", psWorker->szName, szFile);
				}
				else if ((dwStatus & CM_WORKER_ACK_MASK) == CM_WORKER_ACK_OK)
				{
					NTSVCError("%s | %s | [%s]:%s",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_OK", "INF_PROCESS_FILE_OK"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath);
				}
				else if ((dwStatus & CM_WORKER_ACK_MASK) == CM_WORKER_ACK_RETRY_ERR)
				{
					NTSVCError("%s | %s | [%s]:%s",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_ERROR_RETRY", "INF_PROCESS_FILE_ERROR_RETRY"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath);
					fRetry = TRUE;
				}
				else if ((dwStatus & CM_WORKER_ACK_MASK) == CM_WORKER_ACK_RETRY_BUSY)
				{
					NTSVCInfo("CMThreadWorker(), le worker %s n'a pas pu effectuer le traitement du fichier %s pour cause d'indisponibilité", psWorker->szName, szFile);
					fRetry = TRUE;
				}
				else if ((dwStatus & CM_WORKER_ACK_MASK) == CM_WORKER_ACK_ABANDON)
				{
					NTSVCError("%s | %s | [%s]:%s",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_ERROR_ABANDON", "INF_PROCESS_FILE_ERROR_ABANDON"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath);
				}
				else
				{
					NTSVCError("%s | %s | [%s]:%s-0x%08X",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_UNKNOWN_ACK", "INF_PROCESS_FILE_UNKNOWN_ACK"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath,
						dwStatus);
				}

				if ((dwStatus & CM_WORKER_ACT_MASK) == CM_WORKER_ACT_NONE)
				{
					NTSVCInfo("CMThreadWorker(), le worker %s n'a aucune action pour le fichier %s", psWorker->szName, szFile);
				}
				else if ((dwStatus & CM_WORKER_ACT_MASK) == CM_WORKER_ACT_MOVE_ACK)
				{
					if (!CMDirMove(psDirRAB, psDirRABS, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a tenté de déplacer le fichier %s dans le répertoire d'acquittement mais n'a pas pu", psWorker->szName, szFile);
						break;
					}
					dwInc = 0;
					NTSVCInfo("CMThreadWorker(), le worker %s a déplacé le fichier %s dans le répertoire d'acquittement", psWorker->szName, szFile);
				}
				else if ((dwStatus & CM_WORKER_ACT_MASK) == CM_WORKER_ACT_COPY_ERR)
				{
					if (!psWorker->fSXIdentical)
					{
						if (!CMDirMove(psDirRAB, psDirRABX, szFile, TRUE))
						{
							NTSVCInfo("CMThreadWorker(), le worker %s a tenté de copier le fichier %s dans le répertoire des erreurs mais n'a pas pu", psWorker->szName, szFile);
							break;
						}
					}

					if (!CMDirMove(psDirRAB, psDirRABS, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a tenté de déplacer le fichier %s dans le répertoire des erreurs mais n'a pas pu", psWorker->szName, szFile);
						break;
					}
					dwInc = 0;

					NTSVCInfo("CMThreadWorker(), le worker %s a copié le fichier %s dans le répertoire des erreurs puis déplacé dans le répertoire d'acquittement", psWorker->szName, szFile);
				}
				else if ((dwStatus & CM_WORKER_ACT_MASK) == CM_WORKER_ACT_MOVE_ERR)
				{
					if (!CMDirMove(psDirRAB, psWorker->fSXIdentical ? psDirRABS : psDirRABX, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a tenté de déplacer le fichier %s dans le répertoire des erreurs mais n'a pas pu", psWorker->szName, szFile);
						break;
					}
					dwInc = 0;
					NTSVCInfo("CMThreadWorker(), le worker %s a déplacé le fichier %s dans le répertoire des erreurs", psWorker->szName, szFile);
				}
				else if ((dwStatus & CM_WORKER_ACT_MASK) == CM_WORKER_ACT_DELETE)
				{
					if (!CMDirMove(psDirRAB, NULL, szFile, FALSE))
					{
						NTSVCInfo("CMThreadWorker(), le worker %s a tenté de supprimer le fichier %s", psWorker->szName, szFile);
						break;
					}
					dwInc = 0;
					NTSVCInfo("CMThreadWorker(), le worker %s a supprimé le fichier %s", psWorker->szName, szFile);
				}
				else
				{
					NTSVCError("%s | %s | [%s]:%s-0x%08X",
						TextFind(gsSvcWork.hText, "INF_PROCESS_FILE_UNKNOWN_ACT", "INF_PROCESS_FILE_UNKNOWN_ACT"),
						szFile,
						psDirRAB->psHost->szName, psDirRAB->szPath,
						dwStatus);
				}

				if (fRetry && (psWorker->dwEnsureOrder != 0))
				{
					NTSVCInfo("CMThreadWorker(), le worker %s sort de boucle de traitement pour cause de RETRY", psWorker->szName);
					break;
				}
			}
		}

		PurgeDir(psDirRABS, psWorker->dwMaxAckFiles, psWorker->dwMaxAckKB);
		if (!psWorker->fSXIdentical)
			PurgeDir(psDirRABX, psWorker->dwMaxErrFiles, psWorker->dwMaxErrKB);
	}

    NTSVCInfo( "CMThreadDispatch(), Arrêt du thread worker %s", psWorker->szName );

    ExitThread( NO_ERROR );
    return NO_ERROR;
}




//
// CODE DES FONCTIONS PRIVEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void DoExport( CM_LINK * psLink )
 * --------------------------------------------------------------------
 * PARAMETERS: psLink : Pointe sur la structure de gestion d'un lien commoc
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Effectue la gestion d'un lien commoc sortant
 * --------------------------------------------------------------------
 */
PRIVATE void DoExport( CM_LINK * psLink )
{
    DWORD dwIndex;
    BOOL bContinue;
    BOOL bBreak;
    char  szFile[CM_MAX_STR];
    char  szFileDel[CM_MAX_STR];
    CM_DIR * psDirEAB  = &psLink->sDirEAB;
    CM_DIR * psDirEABS = &psLink->sDirEABS;
    CM_DIR * psDirRBA  = &psLink->sDirRBA;
    CM_DIR * psDirRBAS = &psLink->sDirRBAS;
    CM_DIR * psDirRBAT = &psLink->sDirRBAT;

    if ( ( ! psLink->bExport ) && ( ! psLink->bDispatch ) )
        return;

    //
    // Rafraichir le contenu des répertoires qu'on va utiliser
    //
    CMDirRefresh( psDirEAB );
    CMDirRefresh( psDirEABS );

    //
    // Dans le cas du dispatching, l'utilité de cette fonction
    // est uniquement le rafraichissement du contenu des répertoire
    // EAB et EABS pour qu'ils soient utilisables par le thread de
    // dispatching.
    //
    if ( psLink->bDispatch )
        return;

    CMDirRefresh( psDirRBA );
    CMDirRefresh( psDirRBAS );
    CMDirRefresh( psDirRBAT );

    //
    // Les étapes de traitement sont effectuées dans l'ordre
    // inverse de leur définition de manière à minimiser
    // l'accumulation de fichiers chez destinataire en cas
    // de rupture de communication pendant les traitements.
    //

    //
    // Etape E400 : ( E406 à E409 ) Nettoyage acquittement du transfert
    //              Pour tout fichier du répertoire RBAS, s'il n'existe
    //              pas dans EAB, l'éffacer de RBAS.
    // 
    CMDirLock( psDirEAB );
    CMDirLock( psDirRBAS );
    do
    {
        for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirRBAS->dwFiles ; dwIndex ++ )
        {
            if ( ( psDirEAB->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRBAS->dwState == CM_STATE_ERROR ) )
                 break;

            CM_STRNCPY( szFile, psDirRBAS->psFiles[dwIndex].szName, sizeof(szFile) );

            if ( CMDirExist( psDirEAB, szFile ) == CM_NOT_FOUND )
            {
                bContinue = CMDirMove( psDirRBAS, NULL, szFile, FALSE );
                break;
            }
        }
    }
    while ( bContinue );
    CMDirUnlock( psDirRBAS );
    CMDirUnlock( psDirEAB );

    //
    // Etape E400 : ( E401 à E405 ) acquittement du transfert
    //              Pour tout fichier du répertoire EAB existant
    //              egalement dans RBAS :
    //               - s'il n'existe pas dans EABS, le deplacer
    //                 de EAB vers EABS.
    //               - s'il existe dans EABS, le supprimer de EAB.
    //
    CMDirLock( psDirEAB );
    CMDirLock( psDirEABS );
    CMDirLock( psDirRBAS );
    do
    {
        for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEAB->dwFiles ; dwIndex ++ )
        {
            if ( ( psDirEAB->dwState  == CM_STATE_ERROR ) || 
                 ( psDirEABS->dwState == CM_STATE_ERROR ) || 
                 ( psDirRBAS->dwState == CM_STATE_ERROR ) )
                 break;

            CM_STRNCPY( szFile, psDirEAB->psFiles[dwIndex].szName, sizeof(szFile) );

            if ( CMDirExist( psDirRBAS, szFile ) == CM_EXISTS )
            {
                NTSVCInfo( "%s | %s | [%s]:%s",
                    TextFind( gsSvcWork.hText, "INF_PROCESSED", "INF_PROCESSED" ),
                    szFile,
                    psDirEAB->psHost->szName, psDirEAB->szPath );

                if ( CMDirExist( psDirEABS, szFile ) == CM_NOT_FOUND )
                    bContinue = CMDirMove( psDirEAB, psDirEABS, szFile, FALSE );
                else
                    bContinue = CMDirMove( psDirEAB, NULL     , szFile, FALSE );
                break;
            }
        }
    }
    while ( bContinue );
    CMDirUnlock( psDirRBAS );
    CMDirUnlock( psDirEABS );
    CMDirUnlock( psDirEAB );

    //
    // Etape E200 : Transfert de EAB vers RBA
    //              Pour tout fichier de EAB n'existant ni dans
    //              RBA ni dans RBAS, purger RBAT, transmettre
    //              le fichier de EAB vers RBAT, puis le
    //              déplacer de RBAT vers RBA.
    //
    CMDirLock( psDirEAB );
    CMDirLock( psDirRBA );
    CMDirLock( psDirRBAS );
    CMDirLock( psDirRBAT );
    do
    {
        for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEAB->dwFiles ; dwIndex ++ )
        {
            if ( ( psDirEAB->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRBA->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRBAS->dwState == CM_STATE_ERROR ) ||
                 ( psDirRBAT->dwState == CM_STATE_ERROR ) )
                 break;

            CM_STRNCPY( szFile, psDirEAB->psFiles[dwIndex].szName, sizeof(szFile) );

            if ( ( CMDirExist( psDirRBA , szFile ) == CM_NOT_FOUND ) &&
                 ( CMDirExist( psDirRBAS, szFile ) == CM_NOT_FOUND )
               )
            {
                //
                // Tant qu'il y a des fichier dans RBAT et qu'il n'y a pas
                // d'erreur effacer le premier fichier de la liste.
                //
                bBreak = FALSE;
                while ( ( psDirRBAT->dwFiles > 0 ) && ( psDirRBAT->dwState == CM_STATE_SYNCHRONIZED ) )
                {
                    CM_STRNCPY( szFileDel, psDirRBAT->psFiles[0].szName, sizeof(szFileDel) );
                    bBreak = ! CMDirMove( psDirRBAT, NULL, szFileDel, FALSE );
                }
                if ( bBreak || ( psDirRBAT->dwState == CM_STATE_ERROR ) )
                    break;

                if ( ! CMDirTransmit( psDirEAB, psDirRBAT, szFile ) )
                    break;
                if ( psDirRBAT->dwState == CM_STATE_ERROR )
                    break;

                bContinue = CMDirMove( psDirRBAT, psDirRBA, szFile, FALSE );
                break;
            }
        }
    }
    while ( bContinue );
    CMDirUnlock( psDirRBAT );
    CMDirUnlock( psDirRBAS );
    CMDirUnlock( psDirRBA );
    CMDirUnlock( psDirEAB );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void DoImport( CM_LINK * psLink )
 * --------------------------------------------------------------------
 * PARAMETERS: psLink : Pointe sur la structure de gestion d'un lien commoc
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Effectue la gestion d'un lien commoc entrant
 * --------------------------------------------------------------------
 */
PRIVATE void DoImport( CM_LINK * psLink )
{
    DWORD dwIndex;
    BOOL bContinue;
    BOOL bBreak;
    char  szFile[CM_MAX_STR];
    char  szFileDel[CM_MAX_STR];
    CM_DIR * psDirEBA  = &psLink->sDirEBA;
    CM_DIR * psDirEBAS = &psLink->sDirEBAS;
    CM_DIR * psDirRAB  = &psLink->sDirRAB;
    CM_DIR * psDirRABS = &psLink->sDirRABS;
    CM_DIR * psDirRABT = &psLink->sDirRABT;

    if ( ! psLink->bImport )
        return;

    //
    // Rafraichir le contenu des répertoires qu'on va utiliser
    //
    CMDirRefresh( psDirEBA );
    if ( psLink->bImportAck ) CMDirRefresh( psDirEBAS );
    CMDirRefresh( psDirRAB );
    CMDirRefresh( psDirRABS );
    CMDirRefresh( psDirRABT );

    //
    // Les étapes de traitement sont effectuées dans l'ordre
    // inverse de leur définition de manière à minimiser
    // l'accumulation de fichiers chez destinataire en cas
    // de rupture de communication pendant les traitements.
    //

    //
    // Etape R300.1 ou R300.2 :
    // ( R305.1 à R308.1 ou R306.2 à R309.2) Nettoyage acquittement du transfert
    //              Pour tout fichier de RABS n'existant pas dans EBA,
    //              effacer le fichier de RABS.
    //
    CMDirLock( psDirEBA );
    CMDirLock( psDirRABS );
    do
    {
        for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirRABS->dwFiles ; dwIndex ++ )
        {
            if ( ( psDirEBA->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRABS->dwState == CM_STATE_ERROR ) )
                 break;

            CM_STRNCPY( szFile, psDirRABS->psFiles[dwIndex].szName, sizeof(szFile) );

            if ( CMDirExist( psDirEBA, szFile ) == CM_NOT_FOUND )
            {
                bContinue = CMDirMove( psDirRABS, NULL, szFile, FALSE );
                break;
            }
        }
    }
    while ( bContinue );
    CMDirUnlock( psDirRABS );
    CMDirUnlock( psDirEBA );

    if ( ! psLink->bImportAck )
    {
        //
        // L'import est fait en mode non acquité
        //

        //
        // Etape R300.1 : ( R301.1 à R304.1 ) acquittement du transfert
        //              Pour tout fichier du répertoire EBA existant
        //              egalement dans RABS, le supprimer de EBA.
        //
        CMDirLock( psDirEBA );
        CMDirLock( psDirRABS );
        do
        {
            for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEBA->dwFiles ; dwIndex ++ )
            {
                if ( ( psDirEBA->dwState  == CM_STATE_ERROR ) || 
                     ( psDirRABS->dwState == CM_STATE_ERROR ) )
                     break;

                CM_STRNCPY( szFile, psDirEBA->psFiles[dwIndex].szName, sizeof(szFile) );

                if ( CMDirExist( psDirRABS, szFile ) == CM_EXISTS )
                {
                    bContinue = CMDirMove( psDirEBA, NULL, szFile, FALSE );
                    break;
                }
            }
        }
        while ( bContinue );
        CMDirUnlock( psDirRABS );
        CMDirUnlock( psDirEBA );
    }
    else
    {
        //
        // L'import est fait en mode acquité
        //

        //
        // Etape R300.2 : ( R301.2 à R305.1 ) acquittement du transfert
        //              Pour tout fichier du répertoire EBA existant
        //              egalement dans RABS :
        //               - s'il n'existe pas dans EBAS, le deplacer
        //                 de EBA vers EBAS.
        //               - s'il existe dans EBAS, le supprimer de EBA.
        //
        CMDirLock( psDirEBA );
        CMDirLock( psDirEBAS );
        CMDirLock( psDirRABS );
        do
        {
            for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEBA->dwFiles ; dwIndex ++ )
            {
                if ( ( psDirEBA->dwState  == CM_STATE_ERROR ) || 
                     ( psDirEBAS->dwState == CM_STATE_ERROR ) || 
                     ( psDirRABS->dwState == CM_STATE_ERROR ) )
                     break;

                CM_STRNCPY( szFile, psDirEBA->psFiles[dwIndex].szName, sizeof(szFile) );

                if ( CMDirExist( psDirRABS, szFile ) == CM_EXISTS )
                {
                    NTSVCInfo( "%s | %s | [%s]:%s",
                        TextFind( gsSvcWork.hText, "INF_PROCESSED", "INF_PROCESSED" ),
                        szFile,
                        psDirEBA->psHost->szName, psDirEBA->szPath );

                    if ( CMDirExist( psDirEBAS, szFile ) == CM_NOT_FOUND )
                        bContinue = CMDirMove( psDirEBA, psDirEBAS, szFile, FALSE );
                    else
                        bContinue = CMDirMove( psDirEBA, NULL     , szFile, FALSE );
                    break;
                }
            }
        }
        while ( bContinue );
        CMDirUnlock( psDirRABS );
        CMDirUnlock( psDirEBAS );
        CMDirUnlock( psDirEBA );
    }

    //
    // Etape R100 : Transfert de EBA vers RAB
    //              Pour tout fichier de EBA n'exitant ni
    //              dans RAB, ni dans RABS, purger RABT,
    //              transferer le fichier de EBA vers RABT
    //              puis le déplacer de RABT vers RAB.
    //
    CMDirLock( psDirEBA );
    CMDirLock( psDirRAB );
    CMDirLock( psDirRABS );
    CMDirLock( psDirRABT );
    do
    {
        for ( bContinue = FALSE, dwIndex = 0 ; dwIndex < psDirEBA->dwFiles ; dwIndex ++ )
        {
            if ( ( psDirEBA->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRAB->dwState  == CM_STATE_ERROR ) || 
                 ( psDirRABS->dwState == CM_STATE_ERROR ) ||
                 ( psDirRABT->dwState == CM_STATE_ERROR ) )
                 break;

            CM_STRNCPY( szFile, psDirEBA->psFiles[dwIndex].szName, sizeof(szFile) );

            if ( ( CMDirExist( psDirRAB , szFile ) == CM_NOT_FOUND ) &&
                 ( CMDirExist( psDirRABS, szFile ) == CM_NOT_FOUND )
               )
            {
                //
                // Tant qu'il y a des fichier dans RABT et qu'il n'y a pas
                // d'erreur effacer le premier fichier de la liste.
                //
                bBreak = FALSE;
                while ( ( psDirRABT->dwFiles > 0 ) && ( psDirRABT->dwState == CM_STATE_SYNCHRONIZED ) )
                {
                    CM_STRNCPY( szFileDel, psDirRABT->psFiles[0].szName, sizeof(szFileDel) );
                    bBreak = ! CMDirMove( psDirRABT, NULL, szFileDel, FALSE );
                }
                if ( bBreak || ( psDirRABT->dwState == CM_STATE_ERROR ) )
                    break;

                if ( ! CMDirTransmit( psDirEBA, psDirRABT, szFile ) )
                    break;
                if ( psDirRABT->dwState == CM_STATE_ERROR )
                    break;

                bContinue = CMDirMove( psDirRABT, psDirRAB, szFile, FALSE );
                break;
            }
        }
    }
    while ( bContinue );
    CMDirUnlock( psDirRABT );
    CMDirUnlock( psDirRABS );
    CMDirUnlock( psDirRAB );
    CMDirUnlock( psDirEBA );
}




PRIVATE void PurgeDir( CM_DIR * psDir, DWORD dwMaxFiles, DWORD dwMaxKB )
{
    ULONGLONG ullMaxBytes = (ULONGLONG)dwMaxKB * (ULONGLONG)1024; 
    ULONGLONG ullBytes;
    ULONGLONG ullDate;
    char      szFile[CM_MAX_STR];
    DWORD     dwSelectIndex;
    DWORD     dwIndex;

    __try
    {
        if ( ( dwMaxFiles == 0 ) && ( dwMaxKB == 0 ) )
            __leave;

        while ( TRUE )
        {
            dwSelectIndex = 0xFFFFFFFF;
            ullBytes = 0;
            ullDate = 0xFFFFFFFFFFFFFFFF;

            for ( dwIndex = 0 ; dwIndex < psDir->dwFiles ; dwIndex ++ )
            {
                ullBytes += psDir->psFiles[dwIndex].ullSize;

                if ( psDir->psFiles[dwIndex].ullLastWrite < ullDate )
                {
                    ullDate = psDir->psFiles[dwIndex].ullLastWrite;
                    dwSelectIndex = dwIndex;
                }
            }

            if ( dwSelectIndex == 0xFFFFFFFF )
                __leave;

            if ( ( ( dwMaxFiles  != 0 ) && ( psDir->dwFiles > dwMaxFiles ) ) ||
                 ( ( ullMaxBytes != 0 ) && ( ullBytes > ullMaxBytes      ) ) )
            {
				strcpy_s(szFile, sizeof(szFile), psDir->psFiles[dwSelectIndex].szName);
                if ( ! CMDirMove( psDir, NULL, szFile, FALSE ) )
                {
                    NTSVCError( "%s | %s | [%s]:%u",
                        TextFind( gsSvcWork.hText, "INF_FILE_PURGE_ERROR", "INF_FILE_PURGE_ERROR" ),
                        szFile,
                        psDir->szPath );
                    __leave;
                }
                else
                {
                    NTSVCInfo( "%s | %s | [%s]",
                        TextFind( gsSvcWork.hText, "INF_FILE_PURGED", "INF_FILE_PURGED" ),
                        szFile,
                        psDir->szPath );
                }
            }
            else
                __leave;            
        }
    }
    __finally
    {
    }
}