#ifndef CSR_EXCPT_H
#define CSR_EXCPT_H


#include <windows.h>


#ifdef CSR_EXCPT_DEF
#    include <public.h>
#else
#    include <export.h>
#endif



typedef void WINAPI EXCPT_CALLBACK( DWORD dwExcpt, char * szThreadName );

#define EXPT_MODE_DEFAULT      ((void*)0x00000000)
#define EXPT_MODE_EXITTHREAD   ((void*)0xFFFFFFFE)
#define EXPT_MODE_EXITPROCESS  ((void*)0xFFFFFFFF)
#define EXPT_MODE_EXECUTE(x)   ((void*)(x))      



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : char * WINAPI ExcptDump(
 *                      EXCEPTION_POINTERS        * psExcptPrt,
 *                      char                      * pcContext,
 *                      char                      * pcString,
 *                      DWORD                     * pdwStringSize )
 * PARAMETRES: psExcptPtr   : Pointeur sur la structure descriptive d'une exception
 *                            accessible dans __except() à l'aide de la fonction
 *                            intrinseque _exception_info().
 *             pcContext    : Chaine permettant de décrire le contexte de
 *                            l'exception dans la trace.
 *             pcString     : Pointe sur le buffer dans lequel stocker le texte
 *                            du dump de l'exception.
 *             pdwStringSize: En entrée, pointe sur la taille du buffer pointé par
 *                            pcString. En sortie, renvoie la taille restante dans
 *                            le buffer.
 * RETOUR    : Un pointeur sur le caractère '\0' en fin du dump si le buffer n'est
 *             pas totalement rempli. NULL si le buffer est plein.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le dump des données d'une exception dans un buffer.
 * Exemple d'utilisation :
 *  {  
 *      char  * pcContext = "DEPART";
 *      char    szDumpString[10000];
 *      DWORD   dwDumpStringSize = sizeof(szDumpString);
 *      __try
 *      {
 *          pcContext = "CONTEXT_1";
 *          // TO DO : Effectuer des appels pour lesquels le contrôle
 *          // TO DO : d'exception doit être fait.
 *    
 *          pcContext = "CONTEXT_2";
 *          // TO DO : Effectuer d'autres appels pour lesquels le contrôle
 *          // TO DO : d'exception doit être fait.
 *      }
 *      __except( ( ExcptDump( 
 *                        (EXCEPTION_POINTERS*)_exception_info(),
 *                        pcContext,
 *                        szDumpString,
 *                        &dwDumpStringSize )
 *                ),
 *                ( EXCEPTION_EXECUTE_HANDLER )
 *              )
 *      {
 *          // TO DO : Effectuer le traitement de l'exception
 *          printf( "Texte de l'exception:\n%s", szDumpString );
 *      }
 *  }              
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT char * WINAPI ExcptDump(
        EXCEPTION_POINTERS        * psExcptPrt,
        char                      * pcContext,
        char                      * pcString,
        DWORD                     * pdwStringSize );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : void WINAPI ExcptLog(
 *                      EXCEPTION_POINTERS        * psExcptPrt,
 *                      char                      * pcContext )
 * PARAMETRES: psExcptPtr : Pointeur sur la structure descriptive d'une exception
 *                          accessible dans __except() à l'aide de la fonction
 *                          intrinseque _exception_info().
 *             pcContext  : Chaine permettant de décrire le contexte de
 *                          l'exception dans la trace.
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Effectue la trace d'une exception dans un fichier texte.
 *             Le chemin et le nom du fichier sont définis dans le registre
 *             sous la valeur "ExceptionLogFile".
 *             La taille maximum du fichier est définie dans le registre
 *             sous la valeur "MaxExceptionLogSize".
 *             Ces deux valeurs se trouve dans :
 *               HKLM\SOFTWARE\CSRoute.
 *             Les valeurs par défaut sont :
 *                     ExceptionLogFile    (REG_SZ)    = C:\EXCPT.ERR
 *                     MaxExceptionLogSize (REG_DWORD) = 1400000
 * Exemple d'utilisation :
 *  {  
 *      char  * pcContext = "DEPART";
 *      __try
 *      {
 *          pcContext = "CONTEXT_1";
 *          // TO DO : Effectuer des appels pour lesquels le contrôle
 *          // TO DO : d'exception doit être fait.
 *    
 *          pcContext = "CONTEXT_2";
 *          // TO DO : Effectuer d'autres appels pour lesquels le contrôle
 *          // TO DO : d'exception doit être fait.
 *      }
 *      __except( ( ExcptLog( 
 *                        (EXCEPTION_POINTERS*)_exception_info(), 
 *                        "CONTEXTE DE L'EXCEPTION" ) 
 *                ),
 *                ( EXCEPTION_EXECUTE_HANDLER )
 *              )
 *      {
 *          // TO DO : Effectuer le traitement de l'exception
 *      }
 *  }              
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI ExcptLog(
        EXCEPTION_POINTERS        * psExcptPrt,
        char                      * pcContext );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE WINAPI ExcptCreateThread( 
 *                      SECURITY_ATTRIBUTES       * psThreadAttributes,
 *                      DWORD                       dwStackSize,
 *                      LPTHREAD_START_ROUTINE      pfStartAddress,
 *                      void                      * pvParameter,
 *                      DWORD                       dwCreationFlags,
 *                      DWORD                     * pdwThreadId,
 *                      char                      * szThreadName )
 * PARAMETRES: psThreadAttributes : Cf. CreateThread / WIN32
 *             dwStackSize        : Cf. CreateThread / WIN32
 *             pfStartAddress     : Cf. CreateThread / WIN32
 *             pvParameter        : Cf. CreateThread / WIN32
 *             dwCreationFlags    : Cf. CreateThread / WIN32
 *             pdwThreadId        : Cf. CreateThread / WIN32
 *             szThreadName       : Utilisé pour identifié les traces.
 * RETOUR    : Cf. CreateThread / WIN32
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création d'un thread avec handler d'exception.
 *             Le handler d'exception effectue une trace telle que définie pour
 *             la fonction ExcptLog. Puis terminer le thread, le process ou
 *             exécute une fonction selon le mode des threads défini
 *             par ExcptSetDefaultThreadMode().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT HANDLE WINAPI ExcptCreateThread( 
        SECURITY_ATTRIBUTES   * psThreadAttributes,
        DWORD                   dwStackSize,
        LPTHREAD_START_ROUTINE  pfStartAddress,
        void                  * pvParameter,
        DWORD                   dwCreationFlags,
        DWORD                 * pdwThreadId,
        char                  * szThreadName );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : HANDLE WINAPI ExcptCreateThread( 
 *                      SECURITY_ATTRIBUTES       * psThreadAttributes,
 *                      DWORD                       dwStackSize,
 *                      LPTHREAD_START_ROUTINE      pfStartAddress,
 *                      void                      * pvParameter,
 *                      DWORD                       dwCreationFlags,
 *                      DWORD                     * pdwThreadId,
 *                      char                      * szThreadName,
 *                      void                      * pvHandler )
 * PARAMETRES: psThreadAttributes : Cf. CreateThread / WIN32
 *             dwStackSize        : Cf. CreateThread / WIN32
 *             pfStartAddress     : Cf. CreateThread / WIN32
 *             pvParameter        : Cf. CreateThread / WIN32
 *             dwCreationFlags    : Cf. CreateThread / WIN32
 *             pdwThreadId        : Cf. CreateThread / WIN32
 *             szThreadName       : Utilisé pour identifié les traces.
 *             pvHandler          : Mode de handling d'exception.
 *                                  EXPT_MODE_DEFAULT pour utiliser celui défini par défaut
 *                                  EXPT_MODE_EXITTHREAD pour terminer le thread fautif
 *                                  EXPT_MODE_EXITPROCESS pour terminer le process fautif
 *                                  EXPT_MODE_EXECUTE(x) pour excuter le handler x de
 *                                  type EXCPT_CALLBACK.
 * RETOUR    : Cf. CreateThread / WIN32
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création d'un thread avec handler d'exception.
 *             Le handler d'exception effectue une trace telle que définie pour
 *             la fonction ExcptLog.
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT HANDLE WINAPI ExcptCreateThreadWithHandler( 
        SECURITY_ATTRIBUTES   * psThreadAttributes,
        DWORD                   dwStackSize,
        LPTHREAD_START_ROUTINE  pfStartAddress,
        void                  * pvParameter,
        DWORD                   dwCreationFlags,
        DWORD                 * pdwThreadId,
        char                  * szThreadName,
        void                  * pvHandler );


/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void WINAPI ExcptSetDefaultThreadMode( void * pvMode )
 * PARAMETRES: pvMode : EXPT_MODE_EXITTHREAD pour terminer le thread
 *                      fautif, sans terminer le process (à moins que
 *                      le thread soit unique). Le thread retourne
 *                      alors le code de l'exception en code de retour.
 *                      EXPT_MODE_EXITPROCESS pour terminer le process
 *                      fautif. Le process retourne alors le code de
 *                      l'exception en code de retour.
 *                      EXPT_MODE_EXECUTE(x) pour executer un handler
 *                      de type EXCPT_CALLBACK. Si le handler ne termine
 *                      ni le process, ni le thread, le thread est terminé
 *                      automatiquement et retourne le code de l'exception.
 * RETOUR    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Défini le mode de traitement des exception pour les threads
 *             lancés avec ExcptCreateThread().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void WINAPI ExcptSetDefaultThreadMode( void * pvMode );



/*
 * $D_FCTN
 * --------------------------------------------------------------------
 * SYNTAXE   : EXPORT void * WINAPI ExcptGetDefaultThreadMode()
 * PARAMETRES: Aucun
 * RETOUR    : EXPT_MODE_EXITTHREAD : Terminaison du thread fautif par ExitThread
 *             EXPT_MODE_PROCESS    : Terminaison du process fautif par ExitProcess
 *             Autre                : Adresse du handler
 * --------------------------------------------------------------------
 * ROLE      : Détermine le mode de traitement des exception pour les threads
 *             lancés avec ExcptCreateThread().
 * --------------------------------------------------------------------
 * $F_FCTN
 */
EXPORT void * WINAPI ExcptGetDefaultThreadMode();



#endif