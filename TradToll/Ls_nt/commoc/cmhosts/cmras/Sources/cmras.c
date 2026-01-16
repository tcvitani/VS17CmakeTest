/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMRAS
 * FILE       : CMRAS.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, ras
 * --------------------------------------------------------------------
 * SUMMARY    : Extension RAS pour commoc.
 * --------------------------------------------------------------------
 * DESCRIPTION: Extension de communication pour commoc. Cette librairie
 *              fournie l'ensemble des fonctions requises par Commoc
 *              pour mettre en oeuvre des échanges de fichiers basés
 *              sur RAS.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

//
// ATTENTION, CETTE DEFINITION DE MACRO EST NECESSAIRE POUR
// BENEFICIER DU CALLBACK DE TYPE RASDIALFUNC2 AVEC RAS.
//
#define WINVER  0x401

#include <windows.h>
#include <stdio.h>
#include <ras.h>
#include <raserror.h>

#include <trc.h>
#include <reg.h>
#include <csr_excpt.h>


#define LOC_DEF
#include <cmras.h>
#undef LOC_DEF

//#include "commoc.h"

#include <memclass.h>



//
// PROTOTYPE DES FONCTIONS PRIVÉES
//

PRIVATE char * GetRasStatusText( RASCONNSTATE state, char * szString, DWORD dwStringSize );

PRIVATE void HostTrace( HOST_INST * psInst, char * szFormat, ... );

PRIVATE DWORD WINAPI DialNotification(
      DWORD dwCallbackId,
      DWORD dwSubEntry,
      HRASCONN hrasconn,
      UINT unMsg,
      RASCONNSTATE rascs,
      DWORD dwError,
      DWORD dwExtendedError );

PRIVATE BOOL BeginTransfer( HOST_INST * psInst );

PRIVATE void EndTransfer( HOST_INST * psInst );

PRIVATE DWORD ConnectThread( HOST_INST * psInst );




//
// CODE DES FONCTIONS EXPORTEES
//




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: szName     : Nom donnée à la connexion.
 *             hKeyConfig : Handle de clé de registre dans laquelle la
 *                          fonction va aller chercher les informations
 *                          dont elle à besoin pour configurer la connexion.
 * --------------------------------------------------------------------
 * RETURN    : Un handle de connexion en cas de succés d'initialisation.
 *             NULL en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une connexion RAS. Attention, un exécution
 *             réussie ne signifie pas une connexion établie, mais
 *             signifie uniquement que la connexion pourra avoir lieu
 *             en fonction du paramétrage. Un appel à la fonction
 *             HostQueryStatus permettra de connaitre l'état réel de la
 *             connexion RAS.
 *             Le paramétrage attendu par la fonction dans la clé du
 *             registre est le suivant (attention, la présence de
 *             toutes les valeurs est obligatoire) :
 *
 *        * TraceFile (REG_SZ) : Chemin d'accés du fichier de trace utilisé
 *         pour tracer le fonctionnement de la librairie. Si cette chaine
 *         est vide, aucune trace n'est effectuée. Sinon, les traces sont
 *         envoyées sur la console et dans le fichier (si et uniquement si
 *         celui-ci existe déjà).
 *
 *        * PhoneBook (REG_SZ) : Chemin d'accés au répertoire téléphonique
 *         (phonebook) contenant les informations sur les connexions RAS
 *         possibles. Si ce paramètre est vide, c'est le répertoir téléphonique
 *         par défaut qui sera utilisé (en principe, le fichier RASPHONE.PBK
 *         dans %SYSTEMROOT%\SYSTEM32\RAS).
 *
 *        * EntryName (REG_SZ) : Nom de l'entrée à utiliser dans le répertoire
 *         téléphonique spécifié avec le paramètre PhoneBook. Si ce paramètre
 *         est vide, la tentative de connexion aura lieu sur le premier modem
 *         disponible en utilisant le numéro de téléphone spécifié à l'aide du
 *         paramètre PhoneNumber.
 *
 *        * PhoneNumber (REG_SZ) : Numéro de téléphone à composer pour
 *         effectuer la connexion RAS en remplacement de celui de l'entrée du
 *         répertoire téléphonique défini par les paramètres PhoneBook et
 *         EntryName. Si cette chaine est vide, c'est le numéro de l'entrée
 *         du répertoire téléphonique qui sera utilisé.
 *
 *        * CallbackNumber (REG_SZ) : Numéro de téléphone utilisé en mode
 *         callback pour effectuer la connexion. Si ce paramètre n'est pas
 *         fourni ou s'il est vide, le mode callback n'est pas utilisé.
 *         Si ce paramètre vaut "*", c'est le paramètre défini dans le
 *         répertoire téléphonique qui sera utilisé.
 *
 *        * UserName / Password / Domain (REG_SZ) : Nom d'utilisateur, mot de
 *         passe et domaine transmis lors de la connexion RAS. Ces données seront
 *         utilisées par le serveur RAS pour autoriser ou non la connexion.
 *
 *        * DialTimes (REG_SZ) : Plages horaires pendant lesquelles les connexions
 *         RAS doivent avoir lieu. Il s'agit d'une liste d'intervalles exprimés
 *         sous la forme "HH:MM-HH:MM", séparés par un caractère "|". HH exprimant
 *         l'heure (de 0 à 23), MM exprimant les minutes (de 0 à 59). Lorsque la
 *         première heure est plus petite que la deuxième, il s'agit d'un intervale
 *         entièrement compris dans une journée. Lorsque la première heure est plus
 *         ou égale à la deuxième, il s'agit d'un intervale à cheval sur minuit.
 *
 *        * AbortTransfer (REG_DWORD) : Définit le comportement à suivre si la
 *         fin d'un créneau de connexion arrive alors qu'un transfert est en cours.
 *         Si la valeur est 0, la connexion RAS ne sera fermée que lorsqu'aucun
 *         transfert ne sera en cours. Sinon, la connexion RAS est fermée
 *         immédiatement.
 *
 *        * Simulated (REG_DWORD) : Définit si les appels RAS sont simulés ou non.
 *         Lorsque la valeur est différent de 0, les appels sont simulés et
 *         aucune action RAS n'a réellement lieu. Le système se comporte comme si
 *         les appels RAS se déroulaient toujours correctement.
 *
 *        * PollingPeriodNormal (REG_SZ) : Délai de scrutation du thread de gestion dans
 *         un context normal. Ce délai est forcé au 1000 ms s'il est inférieur.
 *
 *        * PollingPeriodShort (REG_SZ) : Délai de scrutation du thread de gestion dans
 *         un contexte ou l'exécution doit être rapide. Ce délai doit être inférieur ou
 *         égal à PollingPeriodNormal (peut être 0). Sa valeur est forcée à
 *         PollingPeriodNormal s'il est supérieur.
 *
 *        * PollingPeriodLong (REG_SZ) : Délai de scrutation du thread de gestion dans un
 *         contexte ou l'exécution doit être ralentie. Ce délai doit être à la fois
 *         supérieur à PollingPeriodNormal et à 3000 ms. Sa valeur est forcée au maximum
 *         des deux s'il est inférieur.
 *
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostOpen=_HostOpen@8" )
#endif
EXPORT HANDLE WINAPI HostOpen( char * szName, HKEY hKeyConfig )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur à retourner
    DWORD       dwId;               // Id du thread de gestion de la connexion
    DWORD       dwTime;             // Numéro de créneau horaire
    DWORD       dwH1, dwM1, dwS1;   // Heure, minute et seconde de début de créneau
    DWORD       dwH2, dwM2, dwS2;   // Heure, minute et seconde de fin de créneau
    DWORD       dwSize;             // Taille de buffer utilisée pour les accés registre
    char      * pcTimes;            // Utilisé pour l'analyse de la liste des créneaux horaires.
    char      * pcNext;             // Utilisé pour l'analyse de la liste des créneaux horaires.
    HOST_INST * psInst = NULL;      // Pointe sur le bloc de données de l'instance en cours de création.
    char        szTimes[1024];      // Copie de la liste des créneaux horaires.
    char        szTrace[MAX_PATH];  // Fichier de trace


    //
    // DEFINITION D'UN ENSEMBLE DE MACROS SIMPLIFIANT L'ACCES AU REGISTRE
    //

#define LIRE_CHAINE(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
        __leave;\
    else

#define LIRE_CHAINE_LOG(var,nom) \
    dwSize = sizeof(var);\
    dwErr = REG_Lire_Chaine( hKeyConfig, NULL, (nom), (var), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        HostTrace( psInst, "%s / HostOpen / Echec lecture config %s", psInst->szName, (nom) );\
        __leave;\
    }\
    else

#define LIRE_ENTIER_LOG(var,nom) \
    dwErr = REG_Lire_Entier( hKeyConfig, NULL, (nom), &dwSize );\
    if ( dwErr != NO_ERROR )\
    {\
        HostTrace( psInst, "%s / HostOpen / Echec lecture config %s", psInst->szName, (nom) );\
        __leave;\
    }\
    (var) = dwSize

    __try
    {
        //
        // Allocation du bloc de données qui va contenir les informations
        // de l'instance de connexion RAS.
        //
        psInst = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof(*psInst) );
        if ( psInst == NULL )
        {
            dwErr = ERROR_NOT_ENOUGH_MEMORY;
            __leave;
        }

        //
        // Mémoriser le nom du host
        //
		strncpy_s(psInst->szName, sizeof(psInst->szName), szName, _TRUNCATE);
        psInst->szName[sizeof(psInst->szName)-1] = 0;

        //
        // Les données pourront être utilisées simultanément par le thread
        // de gestion de la connexion et par un ou plusieurs threads utilisant
        // cette connexion. On effectue donc une protection des données de
        // l'instance par section critique.
        //
        InitializeCriticalSection( &psInst->sProtect );

        //
        // Initialiser la trace
        //
        LIRE_CHAINE( szTrace, "TraceFile" );
        if ( szTrace[0] != 0 )
        {
            dwErr = TRC_Initialise_Trace( 
                szName, 
                szTrace, 
                TRC_OPT_FICHIER | TRC_OPT_CONSOLE | TRC_OPT_NUMEROTATION,
                &psInst->hTrc );
            if ( dwErr != NO_ERROR )
            {
                __leave;
            }
        }
        else
            psInst->hTrc = NULL;

        HostTrace( psInst, "%s / HostOpen / Début", psInst->szName );

        //
        // La demande d'arrét du thread de gestion de la connexion est
        // signalée par un événement.
        //
        psInst->hEndEvt = CreateEvent( NULL, TRUE, FALSE, NULL );
        if ( psInst->hEndEvt == NULL )
        {
            dwErr = GetLastError();
            HostTrace( psInst, "%s / HostOpen / Echec CreateEvent avec erreur %", psInst->szName, dwErr );
            __leave;
        }

        //
        // LIRE LES PARAMÈTRES DU HOST
        //
        LIRE_CHAINE_LOG( psInst->szPhoneBook            , "PhoneBook" );
        LIRE_CHAINE_LOG( psInst->sDial.szEntryName      , "EntryName" );
        LIRE_CHAINE_LOG( psInst->sDial.szPhoneNumber    , "PhoneNumber" );
        LIRE_CHAINE_LOG( psInst->sDial.szCallbackNumber , "CallbackNumber" );
        LIRE_CHAINE_LOG( psInst->sDial.szUserName       , "UserName" );
        LIRE_CHAINE_LOG( psInst->sDial.szPassword       , "Password" );
        LIRE_CHAINE_LOG( psInst->sDial.szDomain         , "Domain" );
        LIRE_CHAINE_LOG( szTimes                        , "DialTimes" );
        LIRE_ENTIER_LOG( psInst->bAbortTransfer         , "AbortTransfer" );
        LIRE_ENTIER_LOG( psInst->bSimulated             , "Simulated" );
        LIRE_ENTIER_LOG( psInst->dwPollingPeriodNormal  , "PollingPeriodNormal" );
        LIRE_ENTIER_LOG( psInst->dwPollingPeriodShort   , "PollingPeriodShort" );
        LIRE_ENTIER_LOG( psInst->dwPollingPeriodLong    , "PollingPeriodLong" );

        //
        // Effectuer l'analyse de la liste des délais de polling
        //
        if ( psInst->dwPollingPeriodNormal < 1000                          ) psInst->dwPollingPeriodNormal = 1000;
        if ( psInst->dwPollingPeriodShort  > psInst->dwPollingPeriodNormal ) psInst->dwPollingPeriodShort  = psInst->dwPollingPeriodNormal;
        if ( psInst->dwPollingPeriodLong   < psInst->dwPollingPeriodNormal ) psInst->dwPollingPeriodLong   = psInst->dwPollingPeriodNormal;
        if ( psInst->dwPollingPeriodLong   < 3000                          ) psInst->dwPollingPeriodLong   = 3000;

        //
        // Effectuer l'analyse de la liste des plages horaires
        //
        dwTime = 0;
        pcTimes = szTimes;
        while ( *pcTimes != 0 )
        {
            //
            // Rechercher le prochain "|" et le remplacer par une fin de chaine.
            //
            pcNext = strchr( pcTimes, '|' );
            if ( pcNext != NULL )
                pcNext[0] = 0;

            //
            // Lire le contenu de l'élément
            //
			if (sscanf_s(pcTimes, "%u:%u-%u:%u", &dwH1, &dwM1, &dwH2, &dwM2) != 4)
            {
                dwErr = ERROR_INVALID_PARAMETER;
                HostTrace( psInst, "%s / HostOpen / Echec analyse periodes de connexion avec erreur % (syntaxe)", psInst->szName, dwErr );
                __leave;
            }
            dwS1 = dwS2 = 0;

            //
            // S'assurer de la validité et de la cohérence des valeurs
            //
            if ( ( ! VALID_TIME( dwH1, dwM1, dwS1 ) ) || ( ! VALID_TIME( dwH2, dwM2, dwS2 ) ) )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                HostTrace( psInst, "%s / HostOpen / Echec analyse periodes de connexion avec erreur %u (heure non valide)", psInst->szName, dwErr );
                __leave;
            }

            //
            // S'assurer qu'on en encore suffisament d'espace pour
            // stocker un nouveau crénau.
            //
            if ( dwTime >= HOST_MAX_TIMES )
            {
                dwErr = ERROR_INVALID_PARAMETER;
                HostTrace( psInst, "%s / HostOpen / Echec analyse periodes de connexion avec erreur %u (trop de créneaux)", psInst->szName, dwErr );
                __leave;
            }

            //
            // Mettre à jour les données du crénaux courant dans le tableau et
            // passer au suivant.
            //
            psInst->tsTimes[dwTime].dwSecStart = TIME_SEC( dwH1, dwM1, dwS1 );
            psInst->tsTimes[dwTime].dwSecStop  = TIME_SEC( dwH2, dwM2, dwS2 );
            dwTime ++;
            psInst->dwTimes = dwTime;

            //
            // S'il s'agit du dernier élément de la chaine, on sort de la boucle.
            //
            if ( pcNext == NULL )
                break;

            //
            // Passer à l'élément suivant.
            //
            pcTimes = pcNext + 1;
        }

        //
        // Toutes les initialisations ont été effectuées, on peut lancer
        // le thread de gestion de la connexion.
        //
        psInst->hThread = ExcptCreateThread( 
            NULL, 
            0, 
            (LPTHREAD_START_ROUTINE)&ConnectThread, 
            psInst, 
            0, 
            &dwId,
            "ConnectionManager" );
        if ( psInst->hThread == NULL )
        {
            dwErr = GetLastError();
            HostTrace( psInst, "%s / HostOpen / Echec CreateThread avec erreur %u", psInst->szName, dwErr );
            __leave;
        }

        //
        // Tout s'est bien passé.
        //
        dwErr = NO_ERROR;

        HostTrace( psInst, "%s / HostOpen / Fin", psInst->szName );

    }
    __finally
    {
        //
        // En cas d'erreur, et uniquement si le bloc des données de l'instance
        // a été alloué, on libère les resources allouées.
        //
        if ( ( dwErr != NO_ERROR ) && ( psInst != NULL ) )
        {
            if ( psInst->hThread != NULL )
            {
                TerminateThread( psInst->hThread, 0xDEADBEEF );
                CloseHandle( psInst->hThread );
            }
                
            if ( psInst->hEndEvt != NULL )
                CloseHandle( psInst->hEndEvt );

            if ( psInst->hTrc != NULL )
                TRC_Termine_Trace( psInst->hTrc );

            DeleteCriticalSection( &psInst->sProtect );

            HeapFree( GetProcessHeap(), 0, psInst );
            psInst = NULL;
        }

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le handle obtenu (ou NULL en cas d'erreur).
        //
        SetLastError( dwErr );  
    }

#undef LIRE_CHAINE
#undef LIRE_CHAINE_LOG
#undef LIRE_ENTIER_LOG

	return psInst;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle retourné par HostOpen
 * --------------------------------------------------------------------
 * RETURN    : TRUE lorsque la connexion RAS est établie. FALSE en
 *             cas de non connexion.
 * --------------------------------------------------------------------
 * ROLE      : Donne l'état courant de la connexion RAS.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostQueryStatus=_HostQueryStatus@4" )
#endif
EXPORT BOOL   WINAPI HostQueryStatus( HANDLE hCnx )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur à retourner
    BOOL        bStatus;            // Status à retourner
    HOST_INST * psInst;             // Bloc de données de l'instance
    RASCONNSTATUS sStatus;          // Etat de la connexion RAS

    //
    // Pointer au bon endroit.
    //
    psInst =  (HOST_INST*)hCnx;

    HostTrace( psInst, "%s / HostQueryStatus / Début", psInst->szName );

    //
    // Les actions suivantes accédent aux propriétés auxquelles accéde
    // également le thread de gestion de la connexion et potentiellement
    // d'autres appels à la DLL. On effectue donc une protection par
    // section critique.
    //
    EnterCriticalSection( &psInst->sProtect );

    //
    // Si l'état de la connexion est ok, on verifie l'état RAS.
    //
    if ( psInst->dwState == HOST_STATE_CONNECTED  )
    {
        //
        // Interroger le handle de connexion RAS pour connaitre son état.
        //
        if ( psInst->bSimulated )
        {
            dwErr = NO_ERROR;
            sStatus.rasconnstate = RASCS_Connected;
        }
        else
        {
            sStatus.dwSize = sizeof(sStatus);
            dwErr = RasGetConnectStatus( psInst->hRAS, &sStatus );
        }

        //
        // Si un erreur RAS a eu lieu ou que la connexion a été rompue
        //
        if ( ( dwErr != NO_ERROR ) || ( sStatus.rasconnstate != RASCS_Connected ) )
        {
            //
            // On change l'état courant pour signaler qu'on doit se déconnecter.
            // Le handle sera détruit dans le thread de gestion de la connexion.
            //
            psInst->dwState = HOST_STATE_DISCONNECTED;

            //
            // Remise à zéro du flag d'annulation des transferts (utilisé
            // uniquement lors des fin de créneau).
            //
            psInst->bCancelNewTransfer = FALSE;
        }
    }

    //
    // Composer la valeur à retourner
    //
    bStatus = ( psInst->dwState == HOST_STATE_CONNECTED );

    LeaveCriticalSection( &psInst->sProtect );
    
    HostTrace( psInst, "%s / HostQueryStatus / Fin avec status %s et erreur %u", psInst->szName, bStatus ? "CONNECTED" : "DISCONNECTED", dwErr );

    //
    // Finalement, mettre à jour le code d'erreur courant et retourner
    // le status obtenu.
    //
    SetLastError( dwErr );
    return bStatus;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostClose( HANDLE hCnx )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx : Handle retourné par HostOpen
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libère les resources alloués pour la gestion de
 *             la connexion RAS. En particulier, si une connexion
 *             est établie, celle-ci est rompue immédiatement.
 *             Au delà de cet appel, le handle hCnx n'est plus
 *             valide.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostClose=_HostClose@4" )
#endif
EXPORT void   WINAPI HostClose( HANDLE hCnx )
{
    DWORD       dwErr = NO_ERROR;   // Code d'erreur à retourner
    HOST_INST * psInst;             // Bloc de données de l'instance

    //
    // Pointer au bon endroit.
    //
    psInst =  (HOST_INST*)hCnx;

    HostTrace( psInst, "%s / HostClose / Début", psInst->szName );

    //
    // Signaler au thread de gestion de la connexion 
    // qu'il doit s'arréter
    //
    SetEvent( psInst->hEndEvt );

    //
    // Attendre que le thread s'arréte avec un délai max
    //
    WaitForSingleObject( psInst->hThread, 10000 );

    //
    // Forcer la fin du thread si celui-ci n'est pas déjà termine
    //
    TerminateThread( psInst->hThread, 0xDEADBEEF );

    //
    // Libérer les ressources allouées pour la connexion
    //
    if ( psInst->hTrc != NULL ) TRC_Termine_Trace( psInst->hTrc );
    CloseHandle( psInst->hEndEvt );
    DeleteCriticalSection( &psInst->sProtect );
    HeapFree( GetProcessHeap(), 0, psInst );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcLocalPath : Chemin local du fichier source.
 *             pcRemotePath: Chemin distant du fichier destination.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un transfert ascendant (upload) d'un fichier.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostPutFile=_HostPutFile@12" )
#endif
EXPORT BOOL   WINAPI HostPutFile( HANDLE hCnx, char * pcLocalPath , char * pcRemotePath )
{
    DWORD   dwErr = NO_ERROR;   // Code d'erreur à retourner
    BOOL    bResult = TRUE;     // Résultat à retourner
    BOOL    bBegin;             // Resultat de tentative de début de transfert

    __try
    {
        //
        // Tester si on peut lancer un transfert
        //
        bBegin = BeginTransfer( (HOST_INST*)hCnx );
        if ( ! bBegin )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            bResult = FALSE;
            __leave;
        }

        //
        // Dans un premier temps, on s'assure que la connexion est active.
        //
        bResult = HostQueryStatus( hCnx );
        if ( ! bResult )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

        //
        // Ensuite, on effectue la copie
        //
        bResult = CopyFile( pcLocalPath, pcRemotePath, TRUE );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            __leave;
        }
    }
    __finally
    {
        if ( bBegin ) EndTransfer( (HOST_INST*)hCnx );

        HostTrace( (HOST_INST*)hCnx, "%s / HostPutFile / %s  [%s] => [%s]", ((HOST_INST*)hCnx)->szName, bResult ? "OK" : "ERREUR", pcLocalPath, pcRemotePath );

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le status obtenu.
        //
        SetLastError( dwErr ); 
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemotePath: Chemin distant du fichier source.
 *             pcLocalPath : Chemin local du fichier destination.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un transfert descendant (download) d'un fichier.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostGetFile=_HostGetFile@12" )
#endif
EXPORT BOOL   WINAPI HostGetFile( HANDLE hCnx, char * pcRemotePath, char * pcLocalPath )
{
    DWORD   dwErr = NO_ERROR;   // Code d'erreur à retourner
    BOOL    bResult = TRUE;     // Résultat à retourner
    BOOL    bBegin;             // Resultat de tentative de début de transfert

    __try
    {
        //
        // Tester si on peut lancer un transfert
        //
        bBegin = BeginTransfer( (HOST_INST*)hCnx );
        if ( ! bBegin )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            bResult = FALSE;
            __leave;
        }

        //
        // Dans un premier temps, on s'assure que la connexion est active.
        //
        bResult = HostQueryStatus( hCnx );
        if ( ! bResult )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

        //
        // Ensuite, on effectue la copie
        //
        bResult = CopyFile( pcRemotePath, pcLocalPath, TRUE );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            __leave;
        }
    }
    __finally
    {
        if ( bBegin ) EndTransfer( (HOST_INST*)hCnx );

        HostTrace( (HOST_INST*)hCnx, "%s / HostGetFile / %s  [%s] => [%s]", ((HOST_INST*)hCnx)->szName, bResult ? "OK" : "ERREUR", pcRemotePath, pcLocalPath );

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le status obtenu.
        //
        SetLastError( dwErr );
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemoteSrc : Chemin distant du fichier source.
 *             pcRemoteDst : Chemin local du fichier destination. Si NULL, le
 *                           fichier source sera effacé.
 *             bCopy       : TRUE pour effectuer une copie, FALSE pour
 *                           effectuer un déplacement.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue un déplacement ou une copie d'une localisation distante
 *             vers une autre localisation distante.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostMoveFile=_HostMoveFile@16" )
#endif
EXPORT BOOL   WINAPI HostMoveFile( HANDLE hCnx, char * pcRemoteSrc , char * pcRemoteDst, BOOL bCopy )
{
    DWORD   dwErr = NO_ERROR;       // Code d'erreur à retourner
    BOOL    bResult = TRUE;         // Résultat à retourner
    BOOL    bBegin;                 // Resultat de tentative de début de transfert

    __try
    {
        //
        // Tester si on peut lancer un transfert
        //
        bBegin = BeginTransfer( (HOST_INST*)hCnx );
        if ( ! bBegin )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            bResult = FALSE;
            __leave;
        }

        //
        // Dans un premier temps, on s'assure que la connexion est active.
        //
        bResult = HostQueryStatus( hCnx );
        if ( ! bResult )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

        //
        // Ensuite, on effectue la copie, l'effacement ou le déplacement
        //
        if ( pcRemoteDst == NULL )
            bResult = DeleteFile( pcRemoteSrc );

        else if ( bCopy )
            bResult = CopyFile( pcRemoteSrc, pcRemoteDst, TRUE );

        else
            bResult = MoveFile( pcRemoteSrc, pcRemoteDst );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            __leave;
        }
    }
    __finally
    {
        if ( bBegin ) EndTransfer( (HOST_INST*)hCnx );

        HostTrace( (HOST_INST*)hCnx, "%s / HostMoveFile / %s  [%s] => [%s]", ((HOST_INST*)hCnx)->szName, bResult ? "OK" : "ERREUR", pcRemoteSrc, pcRemoteDst == NULL ? "NULL" : pcRemoteDst );

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le status obtenu.
        //
        SetLastError( dwErr );
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
 * --------------------------------------------------------------------
 * PARAMETERS: hCnx        : Handle retourné par HostOpen
 *             pcRemoteMask: Chemin distant du masque à utiliser pour l'énumération.
 *             ppsFound    : En cas de succés, retourne un pointeur sur un tableau de
 *                           structures de type WIN32_FIND_DATA contenant les informations
 *                           sur les fichiers énumérés.
 *             pdwCount    : En cas de succés, retourne le nombre de fichiers énumérés.
 * --------------------------------------------------------------------
 * RETURN    : TRUE en cas de réussite. FALSE en cas d'échec.
 *             GetLastError() permettant d'avoir des détails sur
 *             l'erreur.
 *             En cas d'échec, ne pas tenir compte des valeurs retournés par
 *             ppsFound et pdwCount.
 * --------------------------------------------------------------------
 * ROLE      : Effectue une énumération des fichiers contenus dans un répetoire
 *             distant. Attention, cette fonction n'énumère pas les répertoires.
 *             Le tableau alloué et retourné par la fonction doit être désalloué
 *             à l'aide de la fonction HostFreeEnum.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostEnumFiles=_HostEnumFiles@16" )
#endif
EXPORT BOOL   WINAPI HostEnumFiles( HANDLE hCnx, char * pcRemoteMask, WIN32_FIND_DATA ** ppsFound, DWORD * pdwCount )
{
    DWORD               dwErr = NO_ERROR;   // Code d'erreur à retourner
    DWORD               dwIndex;            // Indice de parcour de tableu
    DWORD               dwAllocated = 0;    // Nombre d'octets alloués
    DWORD               dwFiles = 0;        // Nombre de fichiers
    BOOL                bResult = TRUE;     // Résultat à retourner
    BOOL                bBegin;             // Resultat de tentative de début de transfert
    HANDLE              hFind = INVALID_HANDLE_VALUE;   // Handle d'énumération win32
    WIN32_FIND_DATA     sFile;              // Fichier énuméré
    WIN32_FIND_DATA   * psFiles = NULL;     // Tableau des fichiers énuméré
    WIN32_FIND_DATA   * psNewFiles;         // Tableau des fichiers énuméré réalloué

    __try
    {
        //
        // Tester si on peut lancer un transfert
        //
        bBegin = BeginTransfer( (HOST_INST*)hCnx );
        if ( ! bBegin )
        {
            dwErr = ERROR_CAN_NOT_COMPLETE;
            bResult = FALSE;
            __leave;
        }

        //
        // Dans un premier temps, on s'assure que la connexion est active.
        //
        bResult = HostQueryStatus( hCnx );
        if ( ! bResult )
        {
            dwErr = ERROR_DEVICE_NOT_CONNECTED;
            __leave;
        }

        //
        // Initier la recherche des fichiers
        //
        dwIndex = 0;
        bResult = ( ( hFind = FindFirstFile( pcRemoteMask, &sFile ) ) != INVALID_HANDLE_VALUE );
        dwErr = GetLastError();

        //
        // Tant qu'un fichier a été trouvé
        //
        while ( bResult )
        {
            //
            // Ne pas traiter les répertoires
            //
            if ( ( sFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ) == 0 )
            {
                //
                // S'assurer que la taille allouée est suffisament grande
                //
                while ( dwIndex >= dwAllocated )
                {
                    dwAllocated += HOST_LIST_INCREMENT;
                    if ( psFiles == NULL )
                        psNewFiles = HeapAlloc( GetProcessHeap(), 0, sizeof(psFiles[0]) * dwAllocated );
                    else
                        psNewFiles = HeapReAlloc( GetProcessHeap(), 0, psFiles, sizeof(psFiles[0]) * dwAllocated );
                    if ( psNewFiles == NULL )
                    {
                        bResult = FALSE;
                        dwErr = ERROR_NOT_ENOUGH_MEMORY;
                        __leave;
                    }
                    psFiles = psNewFiles;
                }

                //
                // Recopier les informations du fichier et incrémenter
                // l'index dans le tableau.
                //
                psFiles[dwIndex] = sFile;
                dwIndex++;
            }

            //
            // Aller chercher le fichier suivant
            //
            bResult = FindNextFile( hFind, &sFile );
            dwErr = GetLastError();
        }

        //
        // Si l'erreur est autre chose qu'une fin de recherche
        //
        if ( ( dwErr != ERROR_NO_MORE_FILES  ) && 
             ( dwErr != ERROR_FILE_NOT_FOUND ) &&
             ( dwErr != NO_ERROR             )
           )
        {
            bResult = FALSE;
            __leave;
        }

        //
        // Mettre à jour les paramètres en output
        //
        (*ppsFound) = psFiles;
        (*pdwCount) = dwIndex;
        psFiles = NULL;

        //
        // Tout s'est bien passé
        //
        bResult = TRUE;
        dwErr = NO_ERROR;
    }
    __finally
    {
        if ( psFiles != NULL )
            HeapFree( GetProcessHeap(), 0, psFiles );
        if ( hFind != INVALID_HANDLE_VALUE )
            FindClose( hFind );

        if ( bBegin ) EndTransfer( (HOST_INST*)hCnx );

        HostTrace( (HOST_INST*)hCnx, "%s / HostEnumFiles / %s  [%s]", ((HOST_INST*)hCnx)->szName, bResult ? "OK" : "ERREUR", pcRemoteMask );

        //
        // Finalement, mettre à jour le code d'erreur courant et retourner
        // le status obtenu.
        //
        SetLastError( dwErr );
    }

	return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind )
 * --------------------------------------------------------------------
 * PARAMETERS: psFind : Pointe sur un tableau d'énumération de fichiers
 *                      retourné par HostEnumFiles.
 * --------------------------------------------------------------------
 * RETURN    : Rien.
 * --------------------------------------------------------------------
 * ROLE      : Libère les resources alloués par un énumération de fichier.
 * --------------------------------------------------------------------
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/export:HostFreeEnum=_HostFreeEnum@4" )
#endif
EXPORT void   WINAPI HostFreeEnum( WIN32_FIND_DATA * psFind )
{
    //
    // Libérer le bloc alloué.
    //
    HeapFree( GetProcessHeap(), 0, psFind );
}




//
// CODE DES FONCTIONS PRIVEES
//




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE char * GetRasStatusText( RASCONNSTATE state, char * szString, DWORD dwStringSize )
 * --------------------------------------------------------------------
 * PARAMETERS: state        : Valeur d'état RAS
 *             szString     : Pointe sur un buffer destiné à recevoir le texte clair
 *                         correspondant à l'état RAS.
 *             dwStringSize : Taille max du buffer pointé par szString.
 * --------------------------------------------------------------------
 * RETURN    : szString.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir sous forme de texte clair une description d'un état RAS.
 * --------------------------------------------------------------------
 */
PRIVATE char * GetRasStatusText( RASCONNSTATE state, char * szString, DWORD dwStringSize )
{

    switch( state )
    {
#define STATUS_TEXT(x)  case RASCS_##x : strncpy_s( szString, dwStringSize, #x, _TRUNCATE ); szString[dwStringSize-1] = 0; break;
        STATUS_TEXT( OpenPort )
        STATUS_TEXT( PortOpened )
        STATUS_TEXT( ConnectDevice ) 
        STATUS_TEXT( DeviceConnected )     
        STATUS_TEXT( AllDevicesConnected ) 
        STATUS_TEXT( Authenticate )     
        STATUS_TEXT( AuthNotify )     
        STATUS_TEXT( AuthRetry ) 
        STATUS_TEXT( AuthCallback )     
        STATUS_TEXT( AuthChangePassword )     
        STATUS_TEXT( AuthProject ) 
        STATUS_TEXT( AuthLinkSpeed )     
        STATUS_TEXT( AuthAck )     
        STATUS_TEXT( ReAuthenticate ) 
        STATUS_TEXT( Authenticated )     
        STATUS_TEXT( PrepareForCallback ) 
        STATUS_TEXT( WaitForModemReset )     
        STATUS_TEXT( WaitForCallback )    
        STATUS_TEXT( Projected )  
#if (WINVER >= 0x400)     
        STATUS_TEXT( StartAuthentication )
        STATUS_TEXT( CallbackComplete )
        STATUS_TEXT( LogonNetwork )
#endif 
        STATUS_TEXT( SubEntryConnected )
        STATUS_TEXT( SubEntryDisconnected ) 
        STATUS_TEXT( Interactive )
        STATUS_TEXT( RetryAuthentication ) 
        STATUS_TEXT( CallbackSetByCaller )
        STATUS_TEXT( PasswordExpired )
#if (WINVER >= 0x500)
        STATUS_TEXT( InvokeEapUI )
#endif
        STATUS_TEXT( Connected ) 
        STATUS_TEXT( Disconnected )
#undef  STATUS_TEXT
default: strncpy_s(szString, dwStringSize, "Unknown", _TRUNCATE); szString[dwStringSize - 1] = 0; break;
    }

    return szString;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void HostTrace( HOST_INST * psInst, char * szFormat, ... )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointe sur la structure d'instance de la connexion.
 *             szFormat, ... ; Comme printf
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Effectue une trace
 * --------------------------------------------------------------------
 */
PRIVATE void HostTrace( HOST_INST * psInst, char * szFormat, ... )
{
    va_list pMark;

    if ( psInst->hTrc != NULL )
    {
        va_start( pMark, szFormat );
        TRC_Trace_Texte_V( psInst->hTrc, TRC_OPT_MASK, szFormat, pMark );
    }
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD WINAPI DialNotification(
 *                              DWORD dwCallbackId,
 *                              DWORD dwSubEntry,
 *                              HRASCONN hrasconn,
 *                              UINT unMsg,
 *                              RASCONNSTATE rascs,
 *                              DWORD dwError,
 *                              DWORD dwExtendedError )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc Win32/RAS
 * --------------------------------------------------------------------
 * RETURN    : Cf. doc Win32/RAS
 * --------------------------------------------------------------------
 * ROLE      : Fonction callback RAS de type RasDialFunc2. Pour plus
 *             d'informations, cf. doc Win32/RAS
 * --------------------------------------------------------------------
 */
PRIVATE DWORD WINAPI DialNotification(
                        DWORD dwCallbackId,
                        DWORD dwSubEntry,
                        HRASCONN hrasconn,
                        UINT unMsg,
                        RASCONNSTATE rascs,
                        DWORD dwError,
                        DWORD dwExtendedError )
{
    BOOL            bContineCallback = TRUE;    // Valeur retourné en fin de callback
    HOST_INST     * psInst;                     // Pointera sur l'instance concernée par le callback
    char            szState[32];                // Pour récupérer le texte associé à l'état de connexion

    //
    // Pointer sur l'instance concernée dont le pointeur est
    // transmis à l'aide du callback id fournis lors de l'appel
    // à la fonction RasDial.
    //
    psInst = (HOST_INST*)dwCallbackId;

    HostTrace( psInst, "%s / DialNotification / Etat = %s , Err = %u, ExtErr = %u", psInst->szName, GetRasStatusText( rascs, szState, sizeof(szState) ), dwError, dwExtendedError );

    if ( dwError != NO_ERROR )
    {
        //
        // Un erreur a eu lieu pendant la connexion, on signale
        // donc à la boucle de gestion qu'il faut déconnecter.
        // De plus, le callback n'est plus nécessaire.
        //
        psInst->bRasDisconnected = TRUE;
        bContineCallback = FALSE;
    }
    else if ( rascs == RASCS_Connected )
    {
        //
        // La connexion a réussie, on le signale donc à
        // la boucle de gestion.
        // A partir de ce moment, le callback n'est plus
        // nécessaire puisqu'il va y avoir un polling de
        // demandes d'état.
        //
        psInst->bRasConnected = TRUE;
        bContineCallback = FALSE;
    }
    else if ( rascs == RASCS_Disconnected )
    {
        //
        // La connexion a été fermée, on le signale donc à
        // la boucle de gestion.
        // A partir de ce moment, le callback n'est plus
        // nécessaire puisqu'il va y avoir fermeture de la
        // connexion.
        //
        psInst->bRasDisconnected = TRUE;
        bContineCallback = FALSE;
    }

    return (DWORD)bContineCallback;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE BOOL BeginTransfer( HOST_INST * psInst )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointe sur la structure d'instance de la connexion.
 * --------------------------------------------------------------------
 * RETURN    : TRUE si le transfert peut commencé. FALSE si le transfert
 *             ne doit pas être lancé.
 * --------------------------------------------------------------------
 * ROLE      : Indique le début d'un nouveau transfert. Lorsque le transfert
 *             a été accepté (la fonction retourne TRUE), il faut obligatoirement
 *             appeler la fonction EndTransfer() lorsqu'il est terminé.
 * --------------------------------------------------------------------
 */
PRIVATE BOOL BeginTransfer( HOST_INST * psInst )
{
    BOOL    bResult;    // Résultat à retourner

    EnterCriticalSection( &psInst->sProtect );
    bResult = ( ! psInst->bCancelNewTransfer );
    if ( bResult )
        psInst->dwTransfers ++;
    LeaveCriticalSection( &psInst->sProtect );

    return bResult;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE void EndTransfer( HOST_INST * psInst )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointe sur la structure d'instance de la connexion.
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Indique la fin d'un transfert.
 * --------------------------------------------------------------------
 */
PRIVATE void EndTransfer( HOST_INST * psInst )
{
    EnterCriticalSection( &psInst->sProtect );
    psInst->dwTransfers --;
    LeaveCriticalSection( &psInst->sProtect );
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE DWORD ConnectThread( HOST_INST * psInst )
 * --------------------------------------------------------------------
 * PARAMETERS: psInst        : Pointe sur la structure d'instance de la connexion.
 * --------------------------------------------------------------------
 * RETURN    : Non significatif
 * --------------------------------------------------------------------
 * ROLE      : Point d'entré du thread de gestion de la connexion RAS.
 *             Le thread s'arréte sur signalement de l'événemement
 *             dont un handle est fourni dans psInst->hEndEvt.
 * --------------------------------------------------------------------
 */
PRIVATE DWORD ConnectThread( HOST_INST * psInst )
{
    DWORD       dwIndex;        // Indice de boucle.
    DWORD       dwErr;          // Code d'erreur lors des appels API
    DWORD       dwSeconds;      // Nombre de secondes écoulées depuis minuit
    DWORD       dwWait;         // Délai d'attente de polling
    DWORD       dwTick;         // Valeur de tick utilisé pour les attentes actives
    BOOL        bConnect;       // Etat dans lequel doit se trouver la connexion RAS pour l'heure courante
    SYSTEMTIME  sLocalTime;     // Heure local courante
    HOST_TIME * psTime;         // Utiliser pour parcourir la liste des créneaux horaires
    RASCONNSTATUS sStatus;      // Etat de la connexion RAS

    HostTrace( psInst, "%s / ConnectThread / Demarrage thread de gestion des connexions", psInst->szName );

    //
    // Initialiser avec le délai de polling par défaut pour la première boucle
    //
    dwWait = psInst->dwPollingPeriodLong;

    while ( WaitForSingleObject( psInst->hEndEvt, dwWait ) == WAIT_TIMEOUT )
    {

        //
        // Initialiser avec le délai de polling par défaut pour les boucle suivantes
        //
        dwWait = psInst->dwPollingPeriodNormal;

        //
        // Obtenir l'heure local courante et en calculer le nombre correspondant
        // de secondes écoulées depuis minuit.
        //
        GetLocalTime( &sLocalTime );
        dwSeconds = TIME_SEC( sLocalTime.wHour, sLocalTime.wMinute, sLocalTime.wSecond );

        //
        // Scanner les périodes de connexion jusqu'à touver une période dans laquelle on
        // doit être connecté. Si en fin de scan, aucune période n'a été trouvée, on
        // doit être déconnecté.
        // Pour chaque créneau, les conditions de connexion sont :
        //   * Si la borne de début est plus petite que la borne de fin, il s'agit
        //     d'une période comprise entre minuit et minuit. L'heure courante doit
        //     alors être comprise entre les deux bornes.
        //   * Si la borne de début est plus grande que la borne de fin,
        //     il s'agit d'une période à cheval sur minuit. L'heure courante doit
        //     alors être soit supérieure à la borne de début, soit inférieure à la
        //     borne de fin.
        //   * Enfin, si la borne de début est égale à la borne de fin, il s'agit
        //     d'une période de 24 heures.
        //
        for ( bConnect = FALSE, dwIndex = 0 ; ( ! bConnect ) && ( dwIndex < psInst->dwTimes ) ; dwIndex ++ )
        {
            psTime = &psInst->tsTimes[dwIndex];

            if ( psTime->dwSecStart < psTime->dwSecStop )
                bConnect = ( ( dwSeconds >= psTime->dwSecStart ) && ( dwSeconds < psTime->dwSecStop ) );
            else if ( psTime->dwSecStart > psTime->dwSecStop )
                bConnect = ( ( dwSeconds >= psTime->dwSecStart ) || ( dwSeconds < psTime->dwSecStop ) );
            else 
                bConnect = ( psTime->dwSecStart == psTime->dwSecStop );
        }

        HostTrace( psInst, "%s / ConnectThread / L'heure actuelle est %s", psInst->szName, bConnect ? "CONNECTEE" : "NON CONNECTEE" );

        //
        // Les actions suivantes accédent aux propriétés auxquelles accédent
        // également les appels DLL. On effectue donc une protection par
        // section critique.
        //
        EnterCriticalSection( &psInst->sProtect );

        //
        // Mise à jour de l'état de la connexion d'aprés le callback RAS
        //
        if ( psInst->dwState == HOST_STATE_CONNECTING )
        {
            if  ( psInst->bRasConnected )
                psInst->dwState = HOST_STATE_CONNECTED;

            if  ( psInst->bRasDisconnected )
                psInst->dwState = HOST_STATE_DISCONNECTED;
        }

        //
        // Si la rupture de connexion RAS ne peut avoir lieu quand un
        // transfer est en cours.
        //
        if ( ( psInst->dwState == HOST_STATE_CONNECTED ) &&
             ( ! psInst->bAbortTransfer                ) &&
             ( ! bConnect                              ) &&
             ( psInst->dwTransfers > 0                 )
           )
        {
            HostTrace( psInst, "%s / ConnectThread / Un transfert est en cours, on maintient la connexion", psInst->szName );

            //
            // On tente de maintenir la connexion en place pour
            // que les transferts en cours puissent être terminé.
            //
            bConnect = TRUE;

            //
            // Par contre, toute nouvelle demande de transfert
            // doit échouer.
            //
            psInst->bCancelNewTransfer = TRUE;
        }

        //
        // Si l'état de la connexion est ok, on verifie l'état RAS.
        //
        if ( psInst->dwState == HOST_STATE_CONNECTED  )
        {
            //
            // Interroger le handle de connexion RAS pour connaitre son état.
            //
            if ( psInst->bSimulated )
            {
                dwErr = NO_ERROR;
                sStatus.rasconnstate = RASCS_Connected;
            }
            else
            {
                sStatus.dwSize = sizeof(sStatus);
                dwErr = RasGetConnectStatus( psInst->hRAS, &sStatus );
            }

            //
            // Si un erreur RAS a eu lieu ou que la connexion a été rompue
            //
            if ( ( dwErr != NO_ERROR ) || ( sStatus.rasconnstate != RASCS_Connected ) )
            {
                HostTrace( psInst, "%s / ConnectThread / La connexion qui devrait être active a été rompue", psInst->szName );

                //
                // On change l'état courant pour signaler qu'on doit se déconnecter.
                // Dans le code qui suit, le handle sera détruit.
                //
                psInst->dwState = HOST_STATE_DISCONNECTED;

                //
                // Remise à zéro du flag d'annulation des transferts (utilisé
                // uniquement lors des fin de créneau).
                //
                psInst->bCancelNewTransfer = FALSE;
            }
        }

        //
        // En fontion de l'état dans lequel on doit être, et de l'état dans
        // lequel on est, effectuer les actions nécessaires pour être
        // en concordance.
        //

        //
        // Si le handle RAS est tourjours valide alors qu'on est
        // à l'état déconnecté.
        //
        if ( psInst->dwState == HOST_STATE_DISCONNECTED && psInst->hRAS != NULL )
        {
            HostTrace( psInst, "%s / ConnectThread / Raccrochage RAS", psInst->szName );

            //
            // L'état courant est déconnecté alors qu'un handle de connexion
            // est toujour en cours d'usage. On en effectue donc la fermeture.
            // 
            if ( ! psInst->bSimulated )
                RasHangUp( psInst->hRAS );

            //
            // Conformément à la documentation Microsoft, on attend un délai
            // de 3 secondes de sorte que l'automate RAS ait le temps de
            // se remettre à zéro. Plutot que de faire un Sleep, on laisse
            // boucler en spécifiant une attente de polling plus grande.
            // Il est préférable, ici d'utiliser cette méthode, plutot que
            // la scrutation sur l'état du handle dans la mesure où d'autres
            // connexions peuvent avoir lieu et donc réattribuer ce handle.
            //
            dwWait =  psInst->dwPollingPeriodLong;

            psInst->hRAS = NULL;
        }

        //
        // Si la connexion RAS est en place, ou en cours de mise en place
        // alors qu'on devrait être déconnecté...
        //
        else if ( ( psInst->dwState != HOST_STATE_DISCONNECTED ) && ( ! bConnect ) )
        {
            HostTrace( psInst, "%s / ConnectThread / La connexion RAS est active mais doit être rompue", psInst->szName );

            //
            // On change l'état courant pour signaler qu'on doit se déconnecter.
            // Au prochain cycle, le handle sera détruit.
            //
            psInst->dwState = HOST_STATE_DISCONNECTED;

            //
            // Remise à zéro du flag d'annulation des transferts (utilisé
            // uniquement lors des fin de créneau).
            //
            psInst->bCancelNewTransfer = FALSE;

            //
            // On fait passer l'attente du polling à zéro de manière à reboucler
            // et donc détruire le handle le plus rapidement possible.
            //
            dwWait = psInst->dwPollingPeriodShort;
        }

        //
        // Si la connexion RAS n'est pas en place alors qu'on devrait être connecté...
        //
        else if ( ( psInst->dwState == HOST_STATE_DISCONNECTED ) && bConnect )
        {
            HostTrace( psInst, "%s / ConnectThread / La connexion RAS n'est pas active mais doit être établie", psInst->szName );

            psInst->dwState = HOST_STATE_CONNECTING;
            psInst->bRasConnected = FALSE;
            psInst->bRasDisconnected = FALSE;

            //
            // Lancer la demande de connexion RAS en utilisant les
            // paramètres de la connexion spécifié par l'appel à
            // la fonction HostOpen de la DLL.
            //
            if ( psInst->bSimulated )
            {
                dwErr = NO_ERROR;
                psInst->hRAS = (HRASCONN)0xDEADBEEF;
            }
            else
            {
                psInst->sDial.dwSize = sizeof(psInst->sDial);
                psInst->sDial.dwCallbackId = (DWORD)psInst;
                dwErr = RasDial(
                    NULL, 
                    NULL_IF_EMPTY( psInst->szPhoneBook ), 
                    &psInst->sDial, 
                    2, 
                    &DialNotification, 
                    &psInst->hRAS );
            }

            if ( dwErr != NO_ERROR ) 
            {
                HostTrace( psInst, "%s / ConnectThread / Echec de La connexion RAS avec l'erreur %u", psInst->szName, dwErr );

                //
                // Le handle peut être potentiellement non nul. Lorsque c'est le cas,
                // il doit être désalloué normallement. Ce sera le cas au prochain
                // cycle.
                //
                psInst->dwState = HOST_STATE_DISCONNECTED;

                //
                // Remise à zéro du flag d'annulation des transferts (utilisé
                // uniquement lors des fin de créneau).
                //
                psInst->bCancelNewTransfer = FALSE; 
            }
            else
            {
                HostTrace( psInst, "%s / ConnectThread / La connexion RAS est active", psInst->szName );
                if ( psInst->bSimulated )
                    psInst->bRasConnected = TRUE;
            }
        }

        //
        // S'assurer de l'état réel de la connexion RAS lorsqu'on
        // est censé être connecté.
        //
        if ( psInst->dwState == HOST_STATE_CONNECTED )
        {
            if ( ! HostQueryStatus( psInst ) )
                psInst->dwState = HOST_STATE_DISCONNECTED;
        }

        //
        // Fin de protection des données.
        //
        LeaveCriticalSection( &psInst->sProtect );
    }

    HostTrace( psInst, "%s / ConnectThread / Thread en cours d'arret", psInst->szName );

    //
    // Le thread doit être arrêté. On effectue donc une fermeture
    // de la connexion RAS si celle-ci est encore active.
    //
    if ( psInst->hRAS != NULL )
    {
        if ( ! psInst->bSimulated )
        {
            //
            // Effectuer la fermeture de la connexion.
            //
            RasHangUp( psInst->hRAS );

            //
            // Conformément à la documentation Microsoft, on boucle jusqu'à ce que le handle
            // RAS ne soit plus valide, c'est à dire jusqu'à ce que le système ait totalement
            // terminé la libération de cette connexion.
            // Il est préférable d'utiliser cette méthode plutot que l'attente forcée pour
            // pouvoir arréter le thread le plus vite possible. Dans tous les cas, le
            // délai d'attente est limité.
            // 
            dwTick = GetTickCount();
            
            do
            {
                if ( ( GetTickCount() - dwTick ) > 3000 )
                    break;
                Sleep( 1 );
                sStatus.dwSize = sizeof(sStatus);
            }
            while ( RasGetConnectStatus( psInst->hRAS, &sStatus ) == NO_ERROR );
        }
    }

    HostTrace( psInst, "%s / ConnectThread / Thread arrété", psInst->szName );

    ExitThread( 0 );
    return 0;
}


