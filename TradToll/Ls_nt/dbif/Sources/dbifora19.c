/***************** (v) 2015 SANEF ITS - All rights reserved ******************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* FILE:     DBIFORA.C                                                       */
/* LANGUAGE: C                                                               */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*             																 */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/


/*-------------------------------- INCLUDES:  -------------------------------*/

#include <windows.h>
#include <stdio.h>

#ifndef DBIF_MAP
	#include <oci.h>
#endif

#include <dbif.h>

/*-------------------------------- RESERVED:  -------------------------------*/

#include <memclass.h>

/*-------------------------------- EXTERNALS: -------------------------------*/


/*-------------------------------- DEFINES:   -------------------------------*/

#ifndef DBIF_MAP

#define DB_ENTER_CRITICAL_SECTION(cnx) {if((cnx)->bCriticalInit)EnterCriticalSection(&((cnx)->sCritical));}
#define DB_LEAVE_CRITICAL_SECTION(cnx) {if((cnx)->bCriticalInit)LeaveCriticalSection(&((cnx)->sCritical));}

// Taille maximum du texte d'une erreur
#define DB_MAX_ERR          1024

/*-------------------------------- TYPEDEFS:  -------------------------------*/

// Structure de gestion d'une connexion avec une base
// de données de type Oracle 8.0 par interface OCI 8.0
typedef struct DB_CNX
{
    // Section critique permettant les accès concurrents
    CRITICAL_SECTION     sCritical;
    BOOL                 bCriticalInit;

    // Contexte de callback. Si NULL, alors on est pas en callback
    OCIExtProcContext  * hExt;

    // Données liés à la session en cours
    // OCIEnv             * hEnv;          // Environment handle
    OCIError           * hErr;          // Error handle
    OCIServer          * hSrv;          // Server handle
    OCISvcCtx          * hCtx;          // Service context handle
    OCISession         * hSes;          // Session handle

    // Données liées à la requête en cours
    struct DB_STMT     * psStmts;

    // Données liées aux erreurs
    DWORD                dwErrClass;    // Classe de l'erreur (DB/OS)
    DWORD                dwErrCode;     // Code de l'erreur
    char                 szErrText[DB_MAX_ERR]; // Texte associé à l'erreur

}
DB_CNX;

// Données liées à une requête en cours
typedef struct DB_STMT
{
    // Chainage de la liste
    struct DB_STMT     * psNext;
    struct DB_STMT     * psPrev;

    // Liaison vers la connexion propriétaire
    struct DB_CNX      * psDb;

    // Données liées à une requête en cours
    OCIStmt            * hStm;          // Statement handle
    struct DB_VAR      * psVars;        // Variable chained list
}
DB_STMT;

// Structure de gestion d'une variable pour l'interface
// avec une base de données de type Oracle 8.0 par
// interface OCI 8.0.
// Cette structure fonctionne aussi bien pour les
// binds (placeholders) que pour les defines (résultat
// fetchés en fin de requète).
typedef struct DB_VAR
{
    // Chainage de la liste
    struct DB_VAR      * psNext;
    struct DB_VAR      * psPrev;

    // Liaison vers la connexion propriétaire
    struct DB_STMT     * psStmt;

    // Nom de la variable
    char                 szName[DB_VAR_NAME_SIZE+1];

    // Description de la structure de données
    ub2                  wDataType;     // Type de données ORACLE
    sb4                  dwMaxItemSize; // Taille maximum d'un item, pour une chaine, doit prévoir le terminateur
    ub4                  dwMaxItemCount;// Nombre d'items (0 pour un scalaire, 1 à n pour un tableau) alloués pour le tableau
    BOOL                 bIsTable;      // Indique qu'il s'agit d'un tableau

    // Liaison avec la requète
    union 
    {
    OCIBind            * hBind;         // S'il s'agit d'un bind
    OCIDefine          * hDefine;       // S'il s'agit d'un define
    };

    // Nombre courant d'items dans le tableau (peut etre inférieur à dwMaxItemCount
    ub4                  dwCurrentItemCount;

    // Données de la variable
    sb2                * pwItemsValid;  // Indicateurs de validité des item du tableau
    ub2                * pwItemsSize;   // Indicateurs de taille des item du tableau
    ub2                * pwItemsReturn; // Codes de retour individuels par item du tableau
    BYTE               * pbItemsData;   // Données des items : 1 élément tous les dwMaxItemSize caractères
}
DB_VAR;

/*-------------------------------- FUNCTIONS: -------------------------------*/

PRIVATE void DBCleanString( 
                char * szString );

PRIVATE void DBSetErrOs( 
                DB_CNX * psDb, 
                DWORD dwErrOs, 
                BOOL bOverWrite );

PRIVATE void DBSetErrDb( 
                DB_CNX * psDb, 
                void * pvErr, 
                DWORD dwType, 
                DWORD dwStatus, 
                BOOL bOverWrite );

PRIVATE DB_VAR * DBFindVariable( 
                DB_STMT * psStmt, 
                char * pcVarName );

PRIVATE DB_VAR * DBOpenVariable( 
                char * pcVarName,
                DWORD dwDataType, 
                DWORD dwItemSize, 
                DWORD dwItemCount,
                DWORD * pdwWinErr );

PRIVATE void DBCloseVariable( 
                DB_VAR * psVar );

PRIVATE void * WINAPI DBGetVariableItemValueEx( 
                DB_VAR * psVar, 
                DWORD dwIndex, 
                void * pvBuffer, 
                DWORD * pdwLen,
                BOOL bTruncate );

/*-------------------------------- VARIABLES: -------------------------------*/

PRIVATE BOOL gbDBInitialized = FALSE;
PRIVATE CRITICAL_SECTION gsSerializer;
PRIVATE OCIEnv * hEnv = NULL;

#else

#define DB_FWD_FUNC(func,type,params) { static tp##func * pfFunc = NULL; \
                                        type tResult; \
                                        if ( pfFunc == NULL ) \
                                            pfFunc = (void*)GetProcAddress( DbLoadLibrary(), #func ); \
                                        tResult = pfFunc params; \
                                        return tResult; \
                                      }
#define DB_FWD_PROC(func,params)      { static tp##func * pfFunc = NULL; \
                                        if ( pfFunc == NULL ) \
                                            pfFunc = (void*)GetProcAddress( DbLoadLibrary(), #func ); \
                                        pfFunc params; \
                                      }

PRIVATE HMODULE WINAPI DbLoadLibrary();


#endif


/*-------------------------------- CODE:      -------------------------------*/

EXPORT BOOL WINAPI DllMain(
	IN HINSTANCE hInstDLL,
	IN DWORD dwReason,
	IN LPVOID pvReserved)
{
	BOOL bRes = TRUE; 

    switch (dwReason)
	{
	case DLL_PROCESS_ATTACH : 
#ifndef DBIF_MAP
        InitializeCriticalSection( &gsSerializer );
#else
#endif
		break;

	case DLL_THREAD_ATTACH :  
		break;

	case DLL_THREAD_DETACH :  
		break;

	case DLL_PROCESS_DETACH : 
#ifndef DBIF_MAP
		if (hEnv != NULL)
			OCIHandleFree( hEnv, OCI_HTYPE_ENV );

        DeleteCriticalSection( &gsSerializer );
#else
#endif
		break;

	default : 
		break;
    }

    return bRes;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT char * WINAPI DBGetLastError( DB_CNX * psDb, DWORD * pdwClass, DWORD * pdwCode, char * pcText, DWORD dwSize )
 * PARAMETRES: psDb      : Handle de la connexion avec la base de données
 *             pdwClass  : Récupère la classe de l'erreur (DB / OS) si non NULL
 *             pdwCode   : Récupère le code de l'erreur si non NULL
 *             pcText    : Récupère le texte de l'erreur si non NULL
 *             dwSize    : Taille du buffer pointé par pcText
 * RETOUR    : La valeur de pcText
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations consernant la dernière erreur survenue
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:DBGetLastError=_DBGetLastError@20" )
#endif
EXPORT char * WINAPI DBGetLastError( 
                DB_CNX * psDb, 
                DWORD * pdwClass, 
                DWORD * pdwCode, 
                char * pcText, 
                DWORD dwSize )
#ifndef DBIF_MAP
{
    DB_ENTER_CRITICAL_SECTION( psDb );

    // Ne mettre à jour que les valeurs non nulles
    if ( pdwClass != NULL ) *pdwClass = psDb->dwErrClass;
    if ( pdwCode != NULL ) *pdwCode = psDb->dwErrCode;
    if ( ( pcText != NULL ) && ( dwSize > 1 ) )
    {
		strncpy_s(pcText, dwSize, psDb->szErrText, _TRUNCATE);
        pcText[dwSize-1] = '\0';
    }

    DB_LEAVE_CRITICAL_SECTION( psDb );

    return pcText;
}
#else
    DB_FWD_FUNC( DBGetLastError, char*, ( psDb, pdwClass, pdwCode, pcText, dwSize ) )
#endif


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DB_CNX * WINAPI DBConnectCallback( void * pvCallbackContext )
 * PARAMETRES: pvCallbackContext : Context de callback
 * RETOUR    : Un handle de connexion ou NULL si erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue une connexion base de données avec la session appelante.
 *             ATTENTION : A n'utiliser qu'en cas d'appel externe depuis le
 *             serveur de base.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:DBConnectCallback=_DBConnectCallback@4" )
#endif
EXPORT DB_CNX * WINAPI DBConnectCallback( 
                void * pvCallbackContext )
#ifndef DBIF_MAP
{
    sword wOraErr;              // Status OCI
    DB_CNX * psDb = NULL;       // Pour la structure de connexion à la base
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        // Allocation de la structure de connexion
        psDb = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psDb ) );
        if ( psDb == NULL ) break;

        // Mémoriser le contexte de callback
        psDb->hExt = pvCallbackContext;

        // Obtenir les handles nécessaires
        wOraErr = OCIExtProcGetEnv( 
                psDb->hExt,
				&hEnv,
                &psDb->hCtx,
                &psDb->hErr );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Pas de section critique pour les connexions en callback
        psDb->bCriticalInit = FALSE;

        // Succés de la fonction
        bCompleted = TRUE;
    }
    while ( FALSE );

    // En cas d'erreur, on libère ce qu'il y a à libérer
    if ( ( ! bCompleted ) && ( psDb != NULL ) )
        DBDisconnect( psDb );

    return ( bCompleted ? psDb : NULL );
}
#else
    DB_FWD_FUNC( DBConnectCallback, DB_CNX*, ( pvCallbackContext ) )
#endif



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DB_CNX * WINAPI DBConnect( char * pcInst, char * pcUsr, char * pcPwd )
 * PARAMETRES: pcInst : Nom d'instance / d'alias / de service de la base
 *             pcUsr  : Nom d'utilisateur
 *             pcPwd  : Mot de passe
 * RETOUR    : Un handle de connexion ou NULL si erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue une nouvelle connexion base de données
 *             ATTENTION : A ne pas utiliser en cas d'appel externe depuis le
 *             serveur de base.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT)
	#pragma comment( linker, "/EXPORT:DBConnect=_DBConnect@12" )
#endif
EXPORT DB_CNX * WINAPI DBConnect( 
                char * pcInst, 
                char * pcUsr, 
                char * pcPwd )
#ifndef DBIF_MAP
{
    sword wOraErr;              // Status OCI
    DB_CNX * psDb = NULL;       // Pour la structure de connexion à la base
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie

    EnterCriticalSection( &gsSerializer );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        // Initialiser la library si pas déjà fait
 /*       if ( ! gbDBInitialized )
        {
            wOraErr = OCIInitialize(
                    OCI_THREADED,            // OCI mode (threaded)
                    NULL,                   // User defined malloc / free context pointer
                    NULL,                   // User defined malloc callback (NULL = use default)
                    NULL,                   // User defined realloc callback (NULL = use default)
                    NULL                    // User defined free callback (NULL = use default)
                    );
            if ( wOraErr != OCI_SUCCESS ) break;
            gbDBInitialized = TRUE;
        }*/

        // Allocation de la structure de connexion
        psDb = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psDb ) );
        if ( psDb == NULL ) break;

        // Création d'un nouvel environnement
/*        wOraErr = OCIEnvInit( 
                &psDb->hEnv,            // Get the new environment handle
				OCI_THREADED,            // Environment mode (with mutex)
                0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS ) break;*/

		if (!gbDBInitialized)
		{
			wOraErr = OCIEnvCreate(
				&hEnv,             // Output: environment handle
				OCI_THREADED,      // Mode
				NULL, NULL, NULL, NULL,
				0,                 // Extra user memory
				NULL
				);

			if (wOraErr != OCI_SUCCESS) break;

			gbDBInitialized = TRUE;
		}

        // Creation d'un handle d'erreur
        wOraErr = OCIHandleAlloc( 
                hEnv,                   // Environment for the error handle
                &psDb->hErr,            // Get the new error handle
                OCI_HTYPE_ERROR,        // This is a error handle
                0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Creation d'un handle de serveur
        wOraErr = OCIHandleAlloc( 
                hEnv,                   // Environment for the server handle
                &psDb->hSrv,            // Get the new server handle
                OCI_HTYPE_SERVER,       // This is a server handle
                0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Creation d'un handle de contexte de service
        wOraErr = OCIHandleAlloc( 
                hEnv,                   // Environment for the service context handle
                &psDb->hCtx,            // Get the new service context handle
                OCI_HTYPE_SVCCTX,       // This is a service context handle
                0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Attacher tout ensemble afin de créer la nouvelle connexion
        wOraErr = OCIServerAttach(
                psDb->hSrv,             // Server handle for the connection
                psDb->hErr,             // Error handle for the connection
                pcInst,                 // Database instance or alias name
                (int)strlen( pcInst),        // Size of the database instance or alias name
                OCI_DEFAULT             // Attachment mode (nothing special)
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Affecter l'attribut "server" au contexte de service
        wOraErr = OCIAttrSet(
                psDb->hCtx,             // Handle of the object we want to set an attribute (service context)
                OCI_HTYPE_SVCCTX,       // The object we want to update is a service context
                psDb->hSrv,             // This is the attribute value
                0,                      // Use the default attribut size (depending on its type)
                OCI_ATTR_SERVER,        // The attribute we want to set is the server handle
                psDb->hErr              // Error handle to get the error
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Création d'un handle de session
        wOraErr = OCIHandleAlloc(
                hEnv,                   // Environment for the session handle
                &psDb->hSes,            // Get the new session handle
                OCI_HTYPE_SESSION,      // This is a session handle
		        0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Définir l'attribut "username" de la session
        wOraErr = OCIAttrSet(
                psDb->hSes,             // Handle of the object we want to set an attribute (session)
                OCI_HTYPE_SESSION,      // The object we want to update is a session
                pcUsr,                  // This is the attribute value
                (int)strlen( pcUsr ),        // Size of the attribute
                OCI_ATTR_USERNAME,      // The attribute we want to set is the user name
                psDb->hErr              // Error handle to get the error
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Définir l'attribut "password" de la session
        wOraErr = OCIAttrSet(
                psDb->hSes,             // Handle of the object we want to set an attribute (session)
                OCI_HTYPE_SESSION,      // The object we want to update is a session
                pcPwd,                  // This is the attribute value
                (int)strlen( pcPwd ),        // Size of the attribute
                OCI_ATTR_PASSWORD,      // The attribute we want to set is the password
                psDb->hErr              // Error handle to get the error
                );
        if ( wOraErr != OCI_SUCCESS ) break;
 
        // Effectuer l'ouverture de la session
        wOraErr = OCISessionBegin(
                psDb->hCtx,             // Service contexte handle for the session
                psDb->hErr,             // Error handle
                psDb->hSes,             // Session handle
                OCI_CRED_RDBMS,         // Use RDBMS credentials for authentication
		        OCI_DEFAULT             // Nothing special
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        // Placer la session dans le contexte de service
        wOraErr = OCIAttrSet(
                psDb->hCtx,             // Handle of the object we want to set an attribute (service context)
                OCI_HTYPE_SVCCTX,       // The object we want to update is a service context
                psDb->hSes,             // This is the attribute value
                0,                      // Use the default attribut size (depending on its type)
                OCI_ATTR_SESSION,       // The attribute we want to set is the session
                psDb->hErr              // Error handle to get the error
                );
        if ( wOraErr != OCI_SUCCESS ) break;

        InitializeCriticalSection( &psDb->sCritical );
        psDb->bCriticalInit = TRUE;

        // Tout s'est bien passé
        bCompleted = TRUE;
    }
    while ( FALSE );

    if ( ( ! bCompleted ) && ( psDb != NULL ) )
        DBDisconnect( psDb );

    LeaveCriticalSection( &gsSerializer );

    return ( bCompleted ? psDb : NULL );
}
#else
    DB_FWD_FUNC( DBConnect, DB_CNX*, ( pcInst, pcUsr, pcPwd ) )
#endif



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI DBDisconnect( DB_CNX * psDb )
 * PARAMETRES: psDb : Handle de la connexion avec la base de données
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Ferme un connexion avec une base de données ouverte avec
 *             DBConnect() ou DBConnectCallback().
 *             Cette fonction détruit tous ce qui a été créé dans le
 *             cadre de cette connexion (les requètes, ainsi que les
 *             variables).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBDisconnect=_DBDisconnect@4" )
#endif
EXPORT void WINAPI DBDisconnect( 
                DB_CNX * psDb )
#ifndef DBIF_MAP
{
    DB_STMT * psNext;

    DB_ENTER_CRITICAL_SECTION( psDb );

    // Si des requêtes sont en cours d'utilisation, on les ferme
    while ( psDb->psStmts != NULL )
    {
        psNext = psDb->psStmts->psNext;
        DBCloseStatement( psDb->psStmts );
        psDb->psStmts = psNext;
    }


    // S'il s'agit d'une connexion normale (non callback)
    if ( psDb->hExt == NULL )
    {

        // Terminer la session
        if ( ( psDb->hSrv != NULL ) && ( psDb->hErr != NULL ) && ( psDb->hSes != NULL ) )
            OCISessionEnd(
                    psDb->hCtx,             // Service contexte handle for the session
                    psDb->hErr,             // Error handle
                    psDb->hSes,             // Session handle
		            OCI_DEFAULT             // Nothing special
                    );

        // Detacher le handle d'erreur du serveur
        if ( ( psDb->hSrv != NULL ) && ( psDb->hErr != NULL ) )
            OCIServerDetach( psDb->hSrv, psDb->hErr, OCI_DEFAULT );

		// Free session handle
		if (psDb->hSes != NULL)
			OCIHandleFree(psDb->hSes, OCI_HTYPE_SESSION);

        // Fermer le handle de contexte de service
        // Dans la mesure où la seesion est un attribut du contexte de service, elle
        // est automatiquement fermée en même temps
        if ( psDb->hCtx != NULL )
            OCIHandleFree( psDb->hCtx, OCI_HTYPE_SVCCTX );

		// Fermer le handle de serveur
		if (psDb->hSrv != NULL)
			OCIHandleFree(psDb->hSrv, OCI_HTYPE_SERVER);

		// Fermer le handle d'erreur
        if ( psDb->hErr != NULL )
            OCIHandleFree( psDb->hErr, OCI_HTYPE_ERROR );

        // Enfin, fermer le handle d'environnement
        // if ( psDb->hEnv != NULL )
        //    OCIHandleFree( psDb->hEnv, OCI_HTYPE_ENV );
    }

	DB_LEAVE_CRITICAL_SECTION(psDb);

	// Si la section critique a été initialisée, on la détruit.
	if (psDb->bCriticalInit)
		DeleteCriticalSection(&psDb->sCritical);

    // Libérer la mémoire allouée pour la structure
    HeapFree( GetProcessHeap(), 0, psDb );
}
#else
    DB_FWD_PROC( DBDisconnect, ( psDb ) )
#endif




/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI DBReinitVariable( DB_VAR * psVar )
 * PARAMETRES: psVar : Pointeur de variable
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Lorsqu'une variable est créée, elle est rempli de
 *             NULL (au sens Oracle). Lorsqu'elle est utilisée, le nombre
 *             d'éléments ainsi que leurs valeurs peut changer.
 *             Cette fonction restaure la variable initiale (remplie de
 *             NULL et nombre d'éléments maximum).
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBReinitVariable=_DBReinitVariable@4" )
#endif
EXPORT void WINAPI DBReinitVariable( 
                DB_VAR * psVar )
#ifndef DBIF_MAP
{
    DWORD dwCount;  // Nombre d'items à réinitialiser
    DWORD dwIndex;  // Pour scanner les éléments du tableau

    DB_ENTER_CRITICAL_SECTION( psVar->psStmt->psDb );

    // Si scalaire, nb élément = 1, sinon, prendre la taille dans la
    // structure de la variable
    dwCount = ( psVar->bIsTable ? psVar->dwMaxItemCount : 1 );

    // Tout mettre à zéro
    ZeroMemory( psVar->pbItemsData, dwCount * psVar->dwMaxItemSize );
    ZeroMemory( psVar->pwItemsReturn, dwCount * sizeof(*psVar->pwItemsReturn) );
    ZeroMemory( psVar->pwItemsSize, dwCount * sizeof(*psVar->pwItemsReturn) );
    ZeroMemory( psVar->pwItemsValid, dwCount * sizeof(*psVar->pwItemsReturn) );

    // Sauf les indicateurs de validité qui doivent valoir OCI_IND_NULL pour indiquer la
    // valeur NULL (au sens Oracle).
    for ( dwIndex = 0 ; dwIndex < dwCount ; dwIndex ++ )
        psVar->pwItemsValid[dwIndex] = OCI_IND_NULL; // ORA-NULL value

    DB_LEAVE_CRITICAL_SECTION( psVar->psStmt->psDb );
}
#else
    DB_FWD_PROC( DBReinitVariable, ( psVar ) )
#endif


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DWORD WINAPI DBGetCurrentItemCount( DB_VAR * psVar )
 * PARAMETRES: psVar : Pointeur de variable
 * RETOUR    : Nombre d'élément dans la variable
 * --------------------------------------------------------------------
 * ROLE      : Détermine le nombre d'éléments courant dans une variable.
 *             Dans le cas d'un scalaire, retourn 0
 * --------------------------------------------------------------------
 * $F_FCTN
 */
 #if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBGetCurrentItemCount=_DBGetCurrentItemCount@4" )
#endif
EXPORT DWORD WINAPI DBGetCurrentItemCount(
                DB_VAR * psVar )
#ifndef DBIF_MAP
{
    DWORD dwItemCount;

    DB_ENTER_CRITICAL_SECTION( psVar->psStmt->psDb );
    dwItemCount = ( psVar->bIsTable ? psVar->dwCurrentItemCount : 0);
    DB_LEAVE_CRITICAL_SECTION( psVar->psStmt->psDb );

    return dwItemCount;
}
#else
    DB_FWD_FUNC( DBGetCurrentItemCount, DWORD, ( psVar ) )
#endif







/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void * WINAPI DBGetVariableItemValue( DB_VAR * psVar, DWORD dwIndex, void * pvBuffer, DWORD * pdwLen )
 * PARAMETRES: psVar   : Pointeur de variable
 *             dwIndex : Indexe de l'élément à récupérer (0 pour le premier)
 *                       Dans le cas d'un scalaire, la valeur est ignorée
 *             pvBuffer: Buffer où récupérer la valeur
 *             pdwLen  : En entrée, taille du buffer, en sortie, taille des données lues
 *                       Si pdwLen = NULL ou *pdwLen = 0, il n'y a pas de vérification
 *                       effectuée sur la compatibilité des tailles des données et du buffer.
 * RETOUR    : La valeur de pvBuffer si pas d'erreur, DB_VALUE_NULL si c'est une valeur
 *             nulle au sens d'Oracle, DB_VALUE_TOO_SMALL en cas de problème de taille de
 *             buffer, DB_VALUE_OUT_OF_RANGE si l'index est en dehors du tableau, DB_VALUE_ERROR 
 *             en cas d'erreur.
 *             Attention, dans le cas des chaines de caractère, la valeur nulle et la chaine
 *             vide sont équivalents. DBGetVariableItemValue retournera alors une chaine vide.
 * --------------------------------------------------------------------
 * ROLE      : Récupère le contenu d'un élement d'une variable. Attention, cet élement doit
 *             être accessible dans l'état courant de la variable : si c'est un tableau,
 *             l'index doit entrer dans les limites courantes.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBGetVariableItemValue=_DBGetVariableItemValue@16" )
#endif
EXPORT void * WINAPI DBGetVariableItemValue( 
                DB_VAR * psVar, 
                DWORD dwIndex, 
                void * pvBuffer, 
                DWORD * pdwLen )
#ifndef DBIF_MAP
{
    return DBGetVariableItemValueEx( psVar, dwIndex, pvBuffer, pdwLen, FALSE );
}
#else
    DB_FWD_FUNC( DBGetVariableItemValue, void*, ( psVar, dwIndex, pvBuffer, pdwLen ) )
#endif


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void * WINAPI DBGetVariableItemValueTruncate( DB_VAR * psVar, DWORD dwIndex, void * pvBuffer, DWORD * pdwLen )
 * PARAMETRES: psVar   : Pointeur de variable
 *             dwIndex : Indexe de l'élément à récupérer (0 pour le premier)
 *                       Dans le cas d'un scalaire, la valeur est ignorée
 *             pvBuffer: Buffer où récupérer la valeur
 *             pdwLen  : En entrée, taille du buffer, en sortie, taille des données lues
 *                       Si pdwLen = NULL ou *pdwLen = 0, il n'y a pas de vérification
 *                       effectuée sur la compatibilité des tailles des données et du buffer.
 * RETOUR    : La valeur de pvBuffer si pas d'erreur, DB_VALUE_NULL si c'est une valeur
 *             nulle au sens d'Oracle, DB_VALUE_OUT_OF_RANGE si l'index est en dehors du 
 *             tableau, DB_VALUE_ERROR en cas d'erreur.
 *             Si le buffer est trop petit, la valeur lue est récupérée mais tronquée à la taille
 *             du buffer. Dans le cas d'une chaine, le caractère nul terminateur est placé en
 *             fin de buffer. En sortie, *pdwLen donne la taille réelle nécessaire pour 
 *             stocker les données complètement.
 *             Attention, dans le cas des chaines de caractère, la valeur nulle et la chaine
 *             vide sont équivalents. DBGetVariableItemValue retournera alors une chaine vide.
 * --------------------------------------------------------------------
 * ROLE      : Récupère le contenu d'un élement d'une variable. Attention, cet élement doit
 *             être accessible dans l'état courant de la variable : si c'est un tableau,
 *             l'index doit entrer dans les limites courantes.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBGetVariableItemValueTruncate=_DBGetVariableItemValueTruncate@16" )
#endif
EXPORT void * WINAPI DBGetVariableItemValueTruncate( 
                DB_VAR * psVar, 
                DWORD dwIndex, 
                void * pvBuffer, 
                DWORD * pdwLen )
#ifndef DBIF_MAP
{
    return DBGetVariableItemValueEx( psVar, dwIndex, pvBuffer, pdwLen, TRUE );
}
#else
    DB_FWD_FUNC( DBGetVariableItemValueTruncate, void*, ( psVar, dwIndex, pvBuffer, pdwLen ) )
#endif


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void * WINAPI DBSetVariableItemValue( DB_VAR * psVar, DWORD dwIndex, void * pvBuffer, DWORD dwLen )
 * PARAMETRES: psVar   : Pointeur de variable
 *             dwIndex : Indexe de l'élément à mettre à jour (0 pour le premier)
 *                       Dans le cas d'un scalaire, la valeur est ignorée
 *             pvBuffer: Buffer où récupérer la valeur, ou DB_VALUE_NULL.
 *             dwLen   : Taille des données. 0 pour prendre la taille max définie pour la
 *                       variable (utilisable pour les variables de type numérique).
 *                       Attention, dans le cas d'une chaine, prendre en compte le
 *                       marqueur de fin dans l'évaluation de la taille.
 * RETOUR    : La valeur de pvBuffer si pas d'erreur, DB_VALUE_TOO_BIG si problème de taille
 *             de buffer.
 * --------------------------------------------------------------------
 * ROLE      : Met à jour le contenu d'un élement d'une variable. Attention, cet élement doit
 *             être accessible dans l'état courant de la variable : si c'est un tableau,
 *             l'index doit entrer dans les limites courantes.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBSetVariableItemValue=_DBSetVariableItemValue@16" )
#endif
EXPORT void * WINAPI DBSetVariableItemValue( 
                DB_VAR * psVar, 
                DWORD dwIndex, 
                void * pvBuffer, 
                DWORD dwLen )
#ifndef DBIF_MAP
{
    BOOL bOk = FALSE;   // Si TRUE à la fin, tout s'est bien passé.

    DB_ENTER_CRITICAL_SECTION( psVar->psStmt->psDb );

    // Si c'est un scalaire
    if ( ! psVar->bIsTable )
    {
        // on force l'index à zéro
        dwIndex = 0;
    }

    // La valeur à affectuer est la valeur NULL
    if ( pvBuffer == DB_VALUE_NULL )
    {
        psVar->pwItemsReturn[dwIndex] = OCI_SUCCESS;
        psVar->pwItemsSize[dwIndex] = 0;
        psVar->pwItemsValid[dwIndex] = OCI_IND_NULL;
        bOk = TRUE;
    }

    // La valeur à affecter est une valeur standard
    else
    {
        // Si utilisation de la taille par défaut
        if ( dwLen == 0 )
            dwLen = psVar->dwMaxItemSize;

        // S'assurer que les données peuvent rentrer
        if ( (sb4)dwLen <= psVar->dwMaxItemSize )
        {
            // Recopier les données, mettre à jour les infos sur l'élément
            CopyMemory( &psVar->pbItemsData[ psVar->dwMaxItemSize * dwIndex ], pvBuffer, dwLen );
            psVar->pwItemsReturn[dwIndex] = OCI_SUCCESS;
            psVar->pwItemsSize[dwIndex] = (ub2)dwLen;
            psVar->pwItemsValid[dwIndex] = OCI_IND_NOTNULL;
            bOk = TRUE;
        }
    }

    DB_LEAVE_CRITICAL_SECTION( psVar->psStmt->psDb );

    return ( bOk ? pvBuffer : DB_VALUE_TOO_BIG );
}
#else
    DB_FWD_FUNC( DBSetVariableItemValue, void*, ( psVar, dwIndex, pvBuffer, dwLen ) )
#endif


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DB_VAR * WINAPI DBBindPlaceHolderVariable( DB_STMT * psStmt, char * pcVarName, DWORD dwDataType, DWORD dwItemSize, DWORD dwItemCount )
 * PARAMETRES: psStmt     : Handle de la requète
 *             pcVarName  : Nom de la variable à créer
 *             dwDataType : Type de données de la variable (DB_TYPE_INT, DB_TYPE_STR)
 *             dwItemSize : Taille d'un élément
 *             dwItemCount: Nombre d'éléments. Si 0, c'est un scalaire.
 * RETOUR    : Un pointeur de variable ou NULL si erreur.
 * --------------------------------------------------------------------
 * ROLE      : Créer une variable de type placeholder dans un requète de
 *             la connexion psDb. Ce type de variable peut être indiféremment un
 *             tableau ou un scalaire. Il peut être utilisé en entrée comme en sortie.
 *             La variable est détruite sur fermeture de la connexion,
 *             sur changement ou sur destruction de la requète courante.
 *             Toute variable créée pour une requète est détruite lors le la destruction
 *             de la requête.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBBindPlaceHolderVariable=_DBBindPlaceHolderVariable@20" )
#endif
EXPORT DB_VAR * WINAPI DBBindPlaceHolderVariable( 
                DB_STMT * psStmt,
                char * pcVarName,
                DWORD dwDataType, 
                DWORD dwItemSize, 
                DWORD dwItemCount )
#ifndef DBIF_MAP
{
    DB_VAR * psVar = NULL;      // Pointera sur la future structure
    sword wOraErr;              // Status OCI
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie
    BOOL bIsTable;              // Indique si la variable est un tableau ou un scalaire
    DWORD dwWinErr;             // Code d'erreur Win32

    DB_ENTER_CRITICAL_SECTION( psStmt->psDb );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        // Vérifier la validité des paramètres

        // Vérifier qu'il y a bien une requête en cours
        if ( psStmt->hStm == NULL )
        {
            DBSetErrOs( psStmt->psDb, ERROR_INVALID_DATA, TRUE );
            break;
        }

        // Vérifier que la variable n'existe pas déjà
        if ( DBFindVariable( psStmt, pcVarName ) != NULL )
        {
            DBSetErrOs( psStmt->psDb, ERROR_INVALID_DATA, TRUE );
            break;
        }

        // Flag indiquant s'il s'agit d'un tableau ou d'un scalaire
        bIsTable = (dwItemCount != 0 );
        
        // Allouer la structure
        psVar = DBOpenVariable( pcVarName, dwDataType, dwItemSize, dwItemCount, &dwWinErr );
        if ( psVar == NULL )
        {
            DBSetErrOs( psStmt->psDb, dwWinErr, TRUE );
            break;
        }

        // Faire pointer sur le proprio
        psVar->psStmt = psStmt;

        // Associer la variable à la requète
        wOraErr = OCIBindByName(
                psStmt->hStm,               // Handle of the statement
                &psVar->hBind,              // Get the handle of the binded variable
                psStmt->psDb->hErr,         // Error handle
                psVar->szName,              // Placeholder name (variable name)
                (int)strlen( psVar->szName ),    // Size of the placeholder name (variable name)
                psVar->pbItemsData,         // Pointer to the variable buffer
                psVar->dwMaxItemSize,       // Maximum size of a data item in the array
                psVar->wDataType,           // Data type
                psVar->pwItemsValid,     // NULL / value validity indicator
                psVar->pwItemsSize,         // Actual value sizes
                psVar->pwItemsReturn,       // Individual items return codes
                dwItemCount,                // Maximum items in the table (0 if scalare)
                bIsTable ? &psVar->dwCurrentItemCount : NULL, // Current items in the table
                OCI_DEFAULT                 // Nothing special
                );
        if ( wOraErr != OCI_SUCCESS )
        {
            DBSetErrDb( psStmt->psDb, psStmt->psDb->hErr, OCI_HTYPE_ERROR, wOraErr, TRUE );
            break;
        }

        // Chainer la variable dans la liste des variables
        if ( psStmt->psVars != NULL )
            psStmt->psVars->psPrev = psVar;
        psVar->psNext = psStmt->psVars;
        psVar->psPrev = NULL;
        psStmt->psVars = psVar;

        // C'est OK
        bCompleted = TRUE;
    }
    while ( FALSE );

    // En cas d'erreur, on libère tout
    if ( ( ! bCompleted ) && ( psVar != NULL ) )
        DBCloseVariable( psVar );

    DB_LEAVE_CRITICAL_SECTION( psStmt->psDb );

    return ( bCompleted ? psVar : NULL );
}
#else
    DB_FWD_FUNC( DBBindPlaceHolderVariable, DB_VAR*, ( psStmt, pcVarName, dwDataType, dwItemSize, dwItemCount ) )
#endif



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT DB_VAR * WINAPI DBBindRowItemVariable( DB_STMT * psStmt, DWORD dwPosition, DWORD dwDataType, DWORD dwItemSize )
 * PARAMETRES: psStmt     : Handle de la requète
 *             dwPosition : Position de la colonne dans le résultat : commence à 1
 *             dwDataType : Type de données de la variable (DB_TYPE_INT, DB_TYPE_STR)
 *             dwItemSize : Taille d'un élément
 * RETOUR    : Un pointeur de variable ou NULL si erreur.
 * --------------------------------------------------------------------
 * ROLE      : Créer une variable de type define (résultat de requêtes en lignes)
 *             pour une requète. Ce type de variable
 *             est un scalaire. Il peut être utilisé en entrée comme en sortie.
 *             La variable est détruite sur fermeture de la connexion, sur
 *             changement ou sur destruction de la requète courante.
 *             Toute variable créée pour une requète est détruite lors le la destruction
 *             de la requête.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBBindRowItemVariable=_DBBindRowItemVariable@16" )
#endif
EXPORT DB_VAR * WINAPI DBBindRowItemVariable( 
                DB_STMT * psStmt,
                DWORD dwPosition,
                DWORD dwDataType, 
                DWORD dwItemSize )
#ifndef DBIF_MAP
{
    DB_VAR * psVar = NULL;      // Pointera sur la future structure
    sword wOraErr;              // Status OCI
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie
    DWORD dwWinErr;             // Code d'erreur WIN32
    char szPosition[20];        // Pour composer le nom de la variable

    DB_ENTER_CRITICAL_SECTION( psStmt->psDb );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        // Le nom de la variable est en fait sa position
        _ltoa_s( dwPosition, szPosition, 20, 10 );

        // Vérifier la validité des paramètres

        // Vérifier qu'il y a bien une requête en cours
        if ( psStmt->hStm == NULL )
        {
            DBSetErrOs( psStmt->psDb, ERROR_INVALID_DATA, TRUE );
            break;
        }

        // Vérifier que la variable n'existe pas déjà
        if ( DBFindVariable( psStmt, szPosition ) != NULL )
        {
            DBSetErrOs( psStmt->psDb, ERROR_INVALID_DATA, TRUE );
            break;
        }

        // Allouer la structure
        psVar = DBOpenVariable( szPosition, dwDataType, dwItemSize, 0, &dwWinErr );
        if ( psVar == NULL )
        {
            DBSetErrOs( psStmt->psDb, dwWinErr, TRUE );
            break;
        }

        // Faire pointer sur le proprio
        psVar->psStmt = psStmt;

        // Associer la variable à la requète
        wOraErr = OCIDefineByPos(
                psStmt->hStm,               // Handle of the statement
                &psVar->hDefine,            // Get the handle of the define
                psStmt->psDb->hErr,         // Error handle
                (ub4)dwPosition,            // Binded position
                psVar->pbItemsData,         // Pointer to the variable buffer
                psVar->dwMaxItemSize,       // Maximum size of a data item in the array
                psVar->wDataType,           // Data type
                psVar->pwItemsValid,        // NULL / value validity indicator
                psVar->pwItemsSize,         // Actual value sizes
                psVar->pwItemsReturn,       // Individual items return codes
                OCI_DEFAULT                 // Nothing special
                );
        if ( wOraErr != OCI_SUCCESS )
        {
            DBSetErrDb( psStmt->psDb, psStmt->psDb->hErr, OCI_HTYPE_ERROR, wOraErr, TRUE );
            break;
        }

        // Chainer la variable dans la liste des variables
        psStmt->psVars->psPrev = psVar;
        psVar->psNext = psStmt->psVars;
        psVar->psPrev = NULL;
        psStmt->psVars = psVar;

        // C'est OK
        bCompleted = TRUE;
    }
    while ( FALSE );

    // En cas d'erreur, on libère tout
    if ( ( ! bCompleted ) && ( psVar != NULL ) )
        DBCloseVariable( psVar );

    DB_LEAVE_CRITICAL_SECTION( psStmt->psDb );

    return ( bCompleted ? psVar : NULL );
}
#else
    DB_FWD_FUNC( DBBindRowItemVariable, DB_VAR*, ( psStmt, dwPosition, dwDataType, dwItemSize ) )
#endif


/*
 * $D_FCTN
 * -------------------------------------------------------------------- 
 * SYNTAXE   : EXPORT void WINAPI DBClearVariables( DB_STMT * psStmt )
 * PARAMETRES: psStmt      : Handle de la requète
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Réinitialiser les variables d'une requète.
 *             Les tableaux reprennent leur taille maxi. Les valeurs de tous les
 *             éléments sont mises à NULL.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBClearVariables=_DBClearVariables@4" )
#endif
EXPORT void WINAPI DBClearVariables( 
                DB_STMT * psStmt )
#ifndef DBIF_MAP
{
    DB_VAR * psScan = psStmt->psVars;     // Pour scanner la liste chainée

    DB_ENTER_CRITICAL_SECTION( psStmt->psDb );

    // Scan the chained list 
    while ( psScan != NULL )
    {
        DBReinitVariable( psScan );

        // variable suivante
        psScan = psScan->psNext;
    }

    DB_LEAVE_CRITICAL_SECTION( psStmt->psDb );
}
#else
    DB_FWD_PROC( DBClearVariables, ( psStmt ) )
#endif



/*
 * $D_FCTN
 * -------------------------------------------------------------------- 
 * SYNTAXE   : EXPORT DB_STMT * WINAPI DBOpenStatement( DB_CNX * psDb, char * pcStatement )
 * PARAMETRES: psDb       : Handle de la connexion avec la base de données
 *             pcStatement: Texte de la requête
 * RETOUR    : Handle de statement, NULL si erreur
 * --------------------------------------------------------------------
 * ROLE      : Définir une requète dans une connexion à la base
 *             Sur fermeture de la connexion, les requêtes sont automatiquement
 *             détruites, ainsi que les variables qui leurs sont associées
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBOpenStatement=_DBOpenStatement@8" )
#endif
EXPORT DB_STMT * WINAPI DBOpenStatement( 
                DB_CNX * psDb, 
                char * pcStatement )
#ifndef DBIF_MAP
{
    sword wOraErr;              // Status OCI
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie
    DB_STMT * psStmt;

    DB_ENTER_CRITICAL_SECTION( psDb );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        psStmt = HeapAlloc( GetProcessHeap(), HEAP_ZERO_MEMORY, sizeof( *psStmt ) );
        if ( psStmt == NULL )
            break;

        // Créer un nouveau handle de requête
        wOraErr = OCIHandleAlloc(
                hEnv,             // Environment for the statement handle
                &psStmt->hStm,          // Get the new statement handle
                OCI_HTYPE_STMT,         // This is a statement handle
		        0,                      // Extra user memory size
                NULL                    // Buffer allocated by OCI for extra user memory
                );
        if ( wOraErr != OCI_SUCCESS )
        {
            DBSetErrDb( psDb, hEnv, OCI_HTYPE_ENV, wOraErr, TRUE );
            break;
        }

        // Associer le texte de la requête au handle
        wOraErr = OCIStmtPrepare(
                psStmt->hStm,           // Statement handle
                psDb->hErr,             // Error handle for the statement
                pcStatement,            // Statement text
                (int)strlen( pcStatement ),  // Lenght of the statement text
                OCI_NTV_SYNTAX          // The syntax depends on the server version
                , OCI_DEFAULT
                );
        if ( wOraErr != OCI_SUCCESS )
        {
            DBSetErrDb( psDb, psDb->hErr, OCI_HTYPE_ERROR, wOraErr, TRUE );
            break;
        }

        psStmt->psDb = psDb;
        if ( psDb->psStmts != NULL )
            psDb->psStmts->psPrev = psStmt;
        psStmt->psNext = psDb->psStmts;
        psStmt->psPrev = NULL;
        psDb->psStmts = psStmt;
        
        // C'est tout bon
        bCompleted = TRUE;
    }
    while ( FALSE );

    if ( ( ! bCompleted ) && ( psStmt != NULL ) )
    {
        HeapFree( GetProcessHeap(), 0, psStmt );
        psStmt = NULL;
    }

    DB_LEAVE_CRITICAL_SECTION( psDb );

    return psStmt;
}
#else
    DB_FWD_FUNC( DBOpenStatement, DB_STMT*, ( psDb, pcStatement ) )
#endif


/*
 * $D_FCTN
 * -------------------------------------------------------------------- 
 * SYNTAXE   : EXPORT void WINAPI DBCloseStatement( DB_STMT * psStmt )
 * PARAMETRES: psStmt      : Handle de la requète
 * RETOUR    : TRUE si OK, FALSE si erreur
 * --------------------------------------------------------------------
 * ROLE      : Détruit une requète d'une connexion à la base.
 *             Si des variables on été définies, elles sont détruites.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBCloseStatement=_DBCloseStatement@4" )
#endif
EXPORT void WINAPI DBCloseStatement( 
                DB_STMT * psStmt )
#ifndef DBIF_MAP
{
    DB_VAR * psNext;                  // Pour mémoriser des données contenues dans un buffer désalloué
    DB_CNX * psDb = psStmt->psDb;     // Memoriser le handle de connexion pour pouvoir sortir de section critique

    DB_ENTER_CRITICAL_SECTION( psDb );

    // Libération du handle de statement, qui a pour effet de
    // libérer les handles des variables qui y étaient associées.
    // Il ne restera plus qu'a libérer les structures allouées
    // pour la liste chainée.
    OCIHandleFree( psStmt->hStm, OCI_HTYPE_STMT );

    // Scanner la liste chainée et fermer tour à tour toutes les variables
    while ( psStmt->psVars != NULL )
    {
        // Stocker le pointeur sur le suivant puisqu'on va libérer la structure
        // courante qui contient la valeur. psScan->psNext ne sera plus valide
        psNext = psStmt->psVars->psNext;

        // Libérer la structure de la variable
        DBCloseVariable( psStmt->psVars );

        // Passer à la variable suivante
        psStmt->psVars = psNext;
    }

    if ( psStmt->psPrev == NULL ) psStmt->psDb->psStmts = psStmt->psNext;
    if ( psStmt->psPrev != NULL ) psStmt->psPrev->psNext = psStmt->psNext;
    if ( psStmt->psNext != NULL ) psStmt->psNext->psPrev = psStmt->psPrev;

    HeapFree( GetProcessHeap(), 0, psStmt );

    DB_LEAVE_CRITICAL_SECTION( psDb );
}
#else
    DB_FWD_PROC( DBCloseStatement, ( psStmt ) )
#endif


/*
 * $D_FCTN
 * -------------------------------------------------------------------- 
 * SYNTAXE   : EXPORT BOOL WINAPI DBExecuteStatement( DB_STMT * psStmt )
 * PARAMETRES: psStmt      : Handle de la requète
 * RETOUR    : TRUE si OK, FALSE si erreur
 * --------------------------------------------------------------------
 * ROLE      : Demande l'exécution de la requète.
 *             Les variables placeholder définies pour cette requète sont
 *             potentiellement modifiées.
 *             La récupération des résultats en ligne s'effectue ensuite
 *             à l'aide de la fonction DBGetNextRow().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBExecuteStatement=_DBExecuteStatement@4" )
#endif
EXPORT BOOL WINAPI DBExecuteStatement( 
                DB_STMT * psStmt )
#ifndef DBIF_MAP
{
    sword wOraErr;              // Status OCI
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie

    DB_ENTER_CRITICAL_SECTION( psStmt->psDb );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        wOraErr = OCIStmtExecute(
            psStmt->psDb->hCtx,         // Service contexte handle for the statement execution
            psStmt->hStm,               // Handle of the statement to execute
            psStmt->psDb->hErr,         // Error handle
            1,                          // Interation count
            0,                          // Array start index
            NULL,                       // No initial snapshot for the execution
            NULL,                       // Do not retreive "after execution" snapshot
            OCI_DEFAULT                 // Nothing special
            );
        if ( wOraErr != OCI_SUCCESS )
        {
            DBSetErrDb( psStmt->psDb, psStmt->psDb->hErr, OCI_HTYPE_ERROR, wOraErr, TRUE );
            break;
        }

        bCompleted = TRUE;
    }
    while ( FALSE );

    DB_LEAVE_CRITICAL_SECTION( psStmt->psDb );

    return bCompleted;
}
#else
    DB_FWD_FUNC( DBExecuteStatement, BOOL, ( psStmt ) )
#endif


/*
 * $D_FCTN
 * -------------------------------------------------------------------- 
 * SYNTAXE   : EXPORT BOOL WINAPI DBGetNextRow( DB_STMT * psStmt, BOOL * pbEnd )
 * PARAMETRES: psStmt     : Handle de la requète
 *             pbEnd      : Récupère l'indication de fin. Si *pbEnd = TRUE,
 *                          il n'y a plus de données à récupérer.
 * RETOUR    : TRUE si OK, FALSE si erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le rappatriement d'une ligne des données résultant d'une
 *             requète. Ces données sont interrogeable par l'intermédiaire
 *             des variable de type define.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
#if defined(SUPPORT_32_BIT) 
	#pragma comment( linker, "/EXPORT:DBGetNextRow=_DBGetNextRow@8" )
#endif
EXPORT BOOL WINAPI DBGetNextRow( 
                DB_STMT * psStmt, 
                BOOL * pbEnd )
#ifndef DBIF_MAP
{
    sword wOraErr;              // Status OCI
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie
    
    DB_ENTER_CRITICAL_SECTION( psStmt->psDb );

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        wOraErr = OCIStmtFetch(
            psStmt->hStm,               // Handle of the statement to fetch
            psStmt->psDb->hErr,         // Error handle
            1,                          // Just fetch one row
            OCI_FETCH_NEXT,             // Get the next row
            OCI_DEFAULT                 // Nothing special
            );
        if ( ( wOraErr != OCI_SUCCESS_WITH_INFO ) && ( wOraErr != OCI_SUCCESS ) )
        {
            DBSetErrDb( psStmt->psDb, psStmt->psDb->hErr, OCI_HTYPE_ERROR, wOraErr, TRUE );
            break;
        }
        *pbEnd = ( wOraErr == OCI_SUCCESS_WITH_INFO );
        bCompleted = TRUE;
    }
    while ( FALSE );

    DB_LEAVE_CRITICAL_SECTION( psStmt->psDb );

    return bCompleted;
}
#else
    DB_FWD_FUNC( DBGetNextRow, BOOL, ( psStmt, pbEnd ) )
#endif



#ifndef DBIF_MAP


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE void DBCleanString( char * szString )
 * PARAMETRES: szString : Chaine à nettoyer
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Elimine les retour chariot, sauts de ligne et autres
 *             caractères non imprimables.
 *             CR et LF sont remplacés par '|'
 *             Les autres caractères spéciaux sont remplacés par des '.'
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE void DBCleanString( 
                char * szString )
{
    // On utilise directement le paramètre szString pour effectuer le scanning 
    // de la chaine de caracète.
    while ( *szString != '\0' )
    {
        // Retour chariot ou saut de ligne
        if ( ( *szString < '\r' ) || ( *szString < '\n' ) )
            *szString = '|';
        // Caractère non imprimable
        else if ( *szString < ' ' )
            *szString = '.';

        // Passer au caractère suivant
        szString ++;
    }
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE void DBSetErrOs( DB_CNX * psDb, DWORD dwErrOs, BOOL bOverWrite )
 * PARAMETRES: psDb      : Handle de la connexion avec la base de données
 *             dwErrOs   : Code d'erreur lié à l'OS
 *             bOverWrite: Si TRUE, le contexte d'erreur courant est écrasé, sinon, il
 *                         n'est mis a jour que si aucune erreur n'est enregistrée.
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Enregistre une erreur OS dans le contexte d'erreur
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE void DBSetErrOs( 
                DB_CNX * psDb, 
                DWORD dwErrOs, 
                BOOL bOverWrite )
{
    DWORD dwRes;  // Valeur de retour du FormatMessage

    DB_ENTER_CRITICAL_SECTION( psDb );
    
    // Vérifier qu'on peut mettre à jour l'erreur
    if ( ( ! bOverWrite ) && ( psDb->dwErrClass != DB_ERRCLASS_NONE ) )
    {
        DB_LEAVE_CRITICAL_SECTION( psDb );
        return;
    }

    // La classe d'erreur est OS
    psDb->dwErrClass = DB_ERRCLASS_OS;

    // Récupérer le texte associé à l'erreur
    dwRes = FormatMessage( 
        FORMAT_MESSAGE_FROM_SYSTEM, 
        NULL,
        dwErrOs,
        0,
        psDb->szErrText,
        sizeof(psDb->szErrText),
        NULL );
    
    // Si la récupération n'a pas marché
    if ( dwRes == 0 )
    {  
        // On place un texte par défaut contenant le code de l'erreur
        sprintf_s( psDb->szErrText, DB_MAX_ERR, "WIN-ERROR-%u", dwErrOs );
    }

    // Si la récupération a réussie
    else
    {
        // On s'assure tout d'abord qu'il y a bien un terminateur
        psDb->szErrText[sizeof(psDb->szErrText)-1] = '\0';

        // On effecture le nettoyage de la chaine de caractères
        // en éliminant les caractère non imprimables
        DBCleanString( psDb->szErrText );
    }

    // Mettre à jour le code de l'erreur
    psDb->dwErrCode = dwErrOs;

    DB_LEAVE_CRITICAL_SECTION( psDb );
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE void DBSetErrDb( DB_CNX * psDb, void * pvErr, DWORD dwType, DWORD dwStatus, BOOL bOverWrite )
 * PARAMETRES: psDb      : Handle de la connexion avec la base de données
 *             pvErr     : Context / code d'erreur base de données
 *             dwType    : Type du contexte d'erreur base de données
 *             wStatus   : Status associé à l'erreur base de données
 *             bOverWrite: Si TRUE, le contexte d'erreur courant est écrasé, sinon, il
 *                         n'est mis a jour que si aucune erreur n'est enregistrée.
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Enregistre une erreur base de données dans le contexte d'erreur
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE void DBSetErrDb( 
                DB_CNX * psDb, 
                void * pvErr, 
                DWORD dwType, 
                DWORD dwStatus, 
                BOOL bOverWrite )
{
    sword wOraErr;   // Status OCI

    DB_ENTER_CRITICAL_SECTION( psDb );

    // Vérifier qu'on peut mettre à jour l'erreur
    if ( ( ! bOverWrite ) && ( psDb->dwErrClass != DB_ERRCLASS_NONE ) )
    {
        DB_LEAVE_CRITICAL_SECTION( psDb );
        return;
    }

    // La classe d'erreur est DB
    psDb->dwErrClass = DB_ERRCLASS_DB;

    // Le code d'erreur par défaut est : SUCCESS
    psDb->dwErrCode = 0;

    // En fonction du status d'erreur récupéré, la
    // récupération de l'erreur n'est pas la même

    // Dans le cas de status : succès avec information
    if ( dwStatus == OCI_SUCCESS_WITH_INFO )
    {
        // Mettre à jour la chaine d'erreur avec le texte par défaut pour ce status
        strcpy_s( psDb->szErrText, DB_MAX_ERR, "OCI-SUCCESS-WITH-INFO" );
    }

    // Dans le cas de status : données manquantes
    else if ( dwStatus == OCI_NEED_DATA )
    {
        // Mettre à jour la chaine d'erreur avec le texte par défaut pour ce status
        strcpy_s( psDb->szErrText, DB_MAX_ERR, "OCI-NEED-DATA" );
    }

    // Dans le cas de status : données absentes
    else if ( dwStatus == OCI_NO_DATA )
    {
        // Mettre à jour la chaine d'erreur avec le texte par défaut pour ce status
        strcpy_s( psDb->szErrText, DB_MAX_ERR, "OCI-NO-DATA" );
    }

    // Dans le cas de status : erreur
    else if ( dwStatus == OCI_ERROR )
    {
        // Le handle d'erreur est il fourni ?
        if ( pvErr != NULL )
        {
            // Demander les informations nécessaires sur l'erreur
            wOraErr = OCIErrorGet( 
                        pvErr,
                        1,
                        NULL,
                        &psDb->dwErrCode,
                        psDb->szErrText,
                        sizeof(psDb->szErrText),
                        dwType
                        );
        }
        else
        {
            // Faire comme si la demande d'info sur l'erreur avait échouée
            wOraErr = OCI_ERROR;
        }

        // En cas d'échech sur la demande d'informations à propos de l'erreur OCI
        if ( wOraErr != OCI_SUCCESS )
        {
            // On restaure le code d'erreur potentiellement modifié par l'appel à OCIGetError()
            psDb->dwErrCode = 0;

            // Mettre à jour la chaine d'erreur avec le texte par défaut pour ce status
            strcpy_s( psDb->szErrText, DB_MAX_ERR, "OCI-ERROR" );
        }
        else
        {
            // On s'assure tout d'abord qu'il y a bien un terminateur
            psDb->szErrText[sizeof(psDb->szErrText)-1] = '\0';
            // On effecture le nettoyage de la chaine de caractères
            // en éliminant les caractère non imprimables
            DBCleanString( psDb->szErrText );
        }
    }

    else
    {
        // Mettre à jour la chaine d'erreur avec le texte par défaut pour ce status
        sprintf_s( psDb->szErrText, DB_MAX_ERR, "OCI-UNKNOWN-STATUS-%u", dwStatus );
    }

    DB_LEAVE_CRITICAL_SECTION( psDb );
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE DB_VAR * DBFindVariable( DB_STMT * psStmt, char * pcVarName )
 * PARAMETRES: psStmt    : Handle de la requète
 *             pcVarName : Nom de la variable à rechercher
 * RETOUR    : Un pointeur sur une variable
 * --------------------------------------------------------------------
 * ROLE      : Recherche une variable d'après sont nom
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE DB_VAR * DBFindVariable( 
                DB_STMT * psStmt, 
                char * pcVarName )
{
    DB_VAR * psFound = psStmt->psVars; // Pour scanner la liste chainée

    // Scanner la liste chainée jusqu'à trouver la variable avec le nom qu'on recherche
    while ( psFound != NULL )
    {
        // Attention, on n'est pas "case sensitive" sur les noms, on utilise stricmp
        if ( _stricmp( pcVarName, psFound->szName ) == 0 )
            break;

        // Passer à l'élément suivant
        psFound = psFound->psNext;
    }

    return psFound;
}


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE DB_VAR * DBOpenVariable( char * pcVarName,DWORD dwDataType, DWORD dwItemSize, DWORD dwItemCount,DWORD * pdwWinErr )
 * PARAMETRES: pcVarName  : Nom de la variable à créer
 *             dwDataType : Type de la variable (DB_TYPE_INT, DB_TYPE_STR)
 *             dwItemSize : Taille d'un élément
 *             dwItemCount: Nombre d'éléments (0 pour un scalaire, 1 à n pour un tableau)
 *             pdwWinErr  : Permet de récupérer un code d'erreur Win32 si ca se passe mal.
 * RETOUR    : Un pointeur sur la nouvelle variable
 * --------------------------------------------------------------------
 * ROLE      : Créer une variable, hors contexte de connexion
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE DB_VAR * DBOpenVariable( 
                char * pcVarName,
                DWORD dwDataType, 
                DWORD dwItemSize, 
                DWORD dwItemCount,
                DWORD * pdwWinErr )
{
    DB_VAR * psVar = NULL;      // Pointera sur la future structure
    ub2 wDataType;              // Type de données OCI interne
    DWORD dwIndex;              // Indexe pour scanner les tableaux
    BOOL bCompleted = FALSE;    // A la fin de la fonction, si TRUE, la fonction a réussie
    BOOL bIsTable;              // Indique si la variable est un tableau ou un scalaire
    DWORD dwWinErr = NO_ERROR;  // Code d'erreur Win32

    // La boucle do{}while() n'est pas utilisée en tant que boucle, mais comme
    // facilité pour le break sur erreur
    do
    {
        // Vérifier la validité des paramètres

        // Taille minimum d'un item : 1
        if ( dwItemSize == 0 )
        {
            dwWinErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Vérifier la taille du nom de la variable
        if ( strlen( pcVarName ) >= sizeof( psVar->szName ) )
        {
            dwWinErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Adapter les types de données de la librairie aux types OCI (chaine ou entier)
        if ( dwDataType == DB_TYPE_INT )
            wDataType = SQLT_INT;
        else if ( dwDataType == DB_TYPE_STR )
            wDataType = SQLT_STR;
        else
        {
            dwWinErr = ERROR_INVALID_PARAMETER;
            break;
        }

        // Si TRUE : c'est un tableau
        // Si FALSE: C'est un scalaire
        bIsTable = ( dwItemCount != 0 );

        // Initialiser les données

        // Allouer les buffers

        // Structure principale pour la variable
        psVar = HeapAlloc( 
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    sizeof( *psVar ) );
        if ( psVar == NULL )
        {
            dwWinErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        // Nécessaire pour OCI : codes de retour individuels pour chaque items
        psVar->pwItemsReturn = HeapAlloc( 
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    ( bIsTable ? dwItemCount : 1 ) * sizeof( *psVar->pwItemsReturn ) );
        // Nécessaire pour OCI : taille individuelle courante de chaque item
        psVar->pwItemsSize = HeapAlloc( 
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    ( bIsTable ? dwItemCount : 1 ) * sizeof( *psVar->pwItemsSize ) );
        // Nécessaire pour OCI : validité individuelle courante de chaque item
        psVar->pwItemsValid = HeapAlloc( 
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    ( bIsTable ? dwItemCount : 1 ) * sizeof( *psVar->pwItemsValid ) );
        // Données des items
        psVar->pbItemsData = HeapAlloc( 
                    GetProcessHeap(), 
                    HEAP_ZERO_MEMORY, 
                    ( bIsTable ? dwItemCount : 1 ) * dwItemSize );

        // Si une des allocation a échouées, on annule tout
        if ( ( psVar->pwItemsReturn == NULL ) ||
             ( psVar->pwItemsSize == NULL ) ||
             ( psVar->pwItemsValid == NULL ) ||
             ( psVar->pbItemsData == NULL ) )
        {
            dwWinErr = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        // Initialiser le contenu des structures

        // Mémorisier nom de variable,  type de données, tailles etc.
        strcpy_s( psVar->szName, DB_VAR_NAME_SIZE+1, pcVarName );
        psVar->wDataType = wDataType;
        psVar->dwMaxItemSize = dwItemSize;
        psVar->dwMaxItemCount = dwItemCount;
        psVar->dwCurrentItemCount = dwItemCount;
        psVar->bIsTable = bIsTable;

        // Par défaut, toutes les données de la table on la valeur NULL (au sens Oracle du terme)
        for ( dwIndex = 0 ; dwIndex < ( bIsTable ? dwItemCount : 1 ) ; dwIndex ++ )
            psVar->pwItemsValid[dwIndex] = OCI_IND_NULL; // ORA-NULL value

        // A part le tableau des indicateurs de validité qui a été initialisé à OCI_IND_NULL, tous
        // les autres tableau ont déjà été initialisés à 0 lors de leur allocation

        // Tout s'est bien passé
        bCompleted = TRUE;
    }
    while ( FALSE );

    // En cas d'erreur, on libère ce qui a été alloué
    if ( ( ! bCompleted ) && ( psVar != NULL ) )
            DBCloseVariable( psVar );

    if ( pdwWinErr != NULL )
        *pdwWinErr = dwWinErr;

    return ( bCompleted ? psVar : NULL );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE void DBCloseVariable( DB_VAR * psVar )
 * PARAMETRES: psVar : Pointeur de variable
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Désalloue ce qui a été alloué pour une variable
 *             ATTENTION : Si cette variable est chainée, elle
 *             doit être retirée de la chaine avant l'appel à
 *             cette fonction
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE void DBCloseVariable( 
                DB_VAR * psVar )
{
    if ( psVar->pbItemsData != NULL )
        HeapFree( GetProcessHeap(), 0, psVar->pbItemsData );
    if ( psVar->pwItemsReturn != NULL )
        HeapFree( GetProcessHeap(), 0, psVar->pwItemsReturn );
    if ( psVar->pwItemsValid != NULL )
        HeapFree( GetProcessHeap(), 0, psVar->pwItemsValid );
    if ( psVar->pwItemsSize != NULL )
        HeapFree( GetProcessHeap(), 0, psVar->pwItemsSize );
    HeapFree( GetProcessHeap(), 0, psVar );
}



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : PRIVATE void * WINAPI DBGetVariableItemValueEx( DB_VAR * psVar, DWORD dwIndex, void * pvBuffer, DWORD * pdwLen, BOOL bTruncate )
 * PARAMETRES: psVar   : Pointeur de variable
 *             dwIndex : Indexe de l'élément à récupérer (0 pour le premier)
 *                       Dans le cas d'un scalaire, la valeur est ignorée
 *             pvBuffer: Buffer où récupérer la valeur
 *             pdwLen  : En entrée, taille du buffer, en sortie, taille des données lues
 *                       Si pdwLen = NULL ou *pdwLen = 0, il n'y a pas de vérification
 *                       effectuée sur la compatibilité des tailles des données et du buffer.
 *             bTruncate:Si le buffer est trop petit, la valeur est tronquée et la fonction ne
 *                       renvoie par d'erreur. Dans ce cas, *pdwLen donne en sortie la taille
 *                       nécessaire à un buffer pour tout récupérer.
 * RETOUR    : La valeur de pvBuffer si pas d'erreur, DB_VALUE_NULL si c'est une valeur
 *             nulle au sens d'Oracle, DB_VALUE_TOO_SMALL en cas de problème de taille de
 *             buffer, DB_VALUE_OUT_OF_RANGE si l'index est en dehors du tableau, DB_VALUE_ERROR 
 *             en cas d'erreur.
 *             Attention, dans le cas des chaines de caractère, la valeur nulle et la chaine
 *             vide sont équivalents. DBGetVariableItemValue retournera alors une chaine vide.
 * --------------------------------------------------------------------
 * ROLE      : Récupère le contenu d'un élement d'une variable. Attention, cet élement doit
 *             être accessible dans l'état courant de la variable : si c'est un tableau,
 *             l'index doit entrer dans les limites courantes.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
PRIVATE void * WINAPI DBGetVariableItemValueEx( 
                DB_VAR * psVar, 
                DWORD dwIndex, 
                void * pvBuffer, 
                DWORD * pdwLen,
                BOOL bTruncate )
{
    DWORD dwNeeded;     // Taille de buffer nécessaire
    DWORD dwSize;
    DWORD dwLen;        // Taille effective du buffer
    BOOL  bOk = TRUE;   // Si FALSE, pas la peine de continuer

    DB_ENTER_CRITICAL_SECTION( psVar->psStmt->psDb );

    // Si c'est un scalaire
    if ( ! psVar->bIsTable )
    {
        // on force l'index à zéro
        dwIndex = 0;
    }
    // C'est un tableau
    else
    {
        // Vérifier la limite de l'index
        if ( dwIndex >= psVar->dwCurrentItemCount )
        {
            // On va taper en dehors
            pvBuffer = DB_VALUE_OUT_OF_RANGE;
            bOk = FALSE;
        }
    }

    if ( bOk )
    {
        // Est-ce que la valeur est valide et non nulle
        if ( psVar->pwItemsValid[dwIndex] == OCI_IND_NOTNULL )
        {
            // Si l'item n'est pas en erreur
            if ( psVar->pwItemsReturn[dwIndex] == OCI_SUCCESS )
            {
                // La taille nécessaire dépend du type, dans le cas d'une chaine, on
                // prévoit la place pour le terminateur
                dwSize = psVar->pwItemsSize[dwIndex];
                dwNeeded = dwSize + ( psVar->wDataType == SQLT_STR ? 1 : 0 );
                if ( pdwLen != NULL )
                    dwLen = *pdwLen;
                else
                    dwLen = 0;

                if ( ( ( dwLen != 0 ) && ( dwLen < dwNeeded ) ) && ! bTruncate )
                    pvBuffer = DB_VALUE_TOO_SMALL;
                else
                {
                    if ( ( dwLen != 0 ) && ( dwLen < dwNeeded ) )
                    {
                        dwNeeded = dwLen;
                        dwSize = dwLen - ( psVar->wDataType == SQLT_STR ? 1 : 0 );
                    }

                    // Copier les données dans le buffer utilisateur
                    CopyMemory( pvBuffer, &psVar->pbItemsData[ psVar->dwMaxItemSize * dwIndex ], dwSize );

                    // Si c'est une chaine, on met le terminateur (on a déjà verifié au dessus
                    // qu'on aurait la place de le mettre
                    if ( psVar->wDataType == SQLT_STR )
                        ((char*)pvBuffer)[dwSize] = '\0';

                    // Enfin, si le pointeur est valide, on met à jour la taille des données utilisateur
                    if ( pdwLen != NULL )
                        *pdwLen = dwNeeded;
                }
            }

            // Il y a un erreur
            else
                pvBuffer = DB_VALUE_ERROR;
        }

        // C'est une valeur NULL (au sens Oracle)
        else if ( psVar->pwItemsValid[dwIndex] == OCI_IND_NULL )
        {
            // Dans le cas d'une chaine, NULL = chaine vide
            if ( psVar->wDataType == SQLT_STR )
            {
                ((char*)pvBuffer)[0] = '\0';
                if ( pdwLen != NULL )
                    *pdwLen = 0;
            }
            else
                pvBuffer = DB_VALUE_NULL;
        }

        // La valeur n'a pas pu être récupérée en totalité par OCI
        else
            pvBuffer = DB_VALUE_TOO_SMALL;
    }

    DB_LEAVE_CRITICAL_SECTION( psVar->psStmt->psDb );

    return pvBuffer;
}


#else


PRIVATE HMODULE WINAPI DbLoadLibrary()
{
    HKEY hReg;
    LONG lResult;
    DWORD dwType;
    DWORD dwBytes;
    char  szDbLib[2048];
    static HMODULE hDbLib = NULL;


    if ( hDbLib == NULL )
    {
        lResult = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                                "SOFTWARE\\CSRoute\\DBIF",
                                0,
                                KEY_QUERY_VALUE,
                                &hReg );
        if ( lResult == ERROR_SUCCESS )
        {
            dwBytes = sizeof( szDbLib ) - 1;
            lResult = RegQueryValueEx( hReg,
                                       "MappedDll",
                                       NULL,
                                       &dwType,
                                       (LPBYTE)szDbLib,
                                       &dwBytes );
            if ( ( lResult == ERROR_SUCCESS ) && ( dwType == REG_SZ ) )
                szDbLib[dwBytes] = 0;
            else
                strcpy_s( szDbLib, 2048, "csr_dbifora8.dll" );
        }
        else
            strcpy_s( szDbLib, 2048, "csr_dbifora8.dll" );

        hDbLib = LoadLibrary( szDbLib );
    }

    return hDbLib;
}


#endif

/*-------------------------------- END OF FILE ------------------------------*/


