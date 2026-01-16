/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMWORK
 * FILE       : CMWORK.C
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : Commoc, worker
 * --------------------------------------------------------------------
 * SUMMARY    : Extension traitement pour commoc.
 * --------------------------------------------------------------------
 * DESCRIPTION: Extension de traitement pour commoc. Cette librairie
 *              fournie l'ensemble des fonctions requises par Commoc
 *              pour mettre en oeuvre un traitement fichiers.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CMWORK_H
#define CMWORK_H

#if defined(CMWORK_EXPORTS) || defined(CMWORKEX_EXPORTS)
#    include <public.h>
#else
#    include <export.h>
#endif



#define CM_WORKER_ACK_MASK       0x000000FF
#define CM_WORKER_ACK_NONE       0x00000000
#define CM_WORKER_ACK_OK         0x00000001
#define CM_WORKER_ACK_RETRY_ERR  0x00000002
#define CM_WORKER_ACK_RETRY_BUSY 0x00000003
#define CM_WORKER_ACK_ABANDON    0x00000004

#define CM_WORKER_ACT_MASK       0x0000FF00
#define CM_WORKER_ACT_NONE       0x00000000
#define CM_WORKER_ACT_MOVE_ACK   0x00000100
#define CM_WORKER_ACT_COPY_ERR   0x00000200
#define CM_WORKER_ACT_MOVE_ERR   0x00000300
#define CM_WORKER_ACT_DELETE     0x00000400


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig )
 * --------------------------------------------------------------------
 * PARAMETERS: szName     : Nom donné à l'instance créée.
 *             hKeyConfig : Handle de clé de registre dans laquelle la
 *                          fonction va aller chercher les informations
 *                          dont elle à besoin pour configurer l'instance.
 * --------------------------------------------------------------------
 * RETURN    : Un handle de l'instance en cas de succés d'initialisation.
 *             NULL en cas d'échec.
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de traitement de fichier.
 * --------------------------------------------------------------------
 */
EXPORT HANDLE WINAPI WorkerOpen( char * szName, HKEY hKeyConfig );


typedef HANDLE WINAPI CM_WORKER_OPEN( char * szName, HKEY hKeyConfig );





/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI WorkerClose( HANDLE hWrk )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Libére les resources allouées par WorkerOpen.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI WorkerClose( HANDLE hWrk );


typedef void WINAPI CM_WORKER_CLOSE( HANDLE hWrk );



#ifdef CMWORK_EXPORTS
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 *             szFileName : Chemin complet du fichier à traiter
 * --------------------------------------------------------------------
 * RETURN    : TRUE lorsque le fichier a été traité. FALSE sinon.
 *             GetLastError() donnant un code d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le traitement d'un fichier
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath );






#endif

#ifdef CMWORKEX_EXPORTS
/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI WorkerProcessFile( HANDLE hWrk, char * szFilePath, void * pvReserved )
 * --------------------------------------------------------------------
 * PARAMETERS: hWrk       : Handle retourné par WorkerOpen
 *             szFileName : Chemin complet du fichier à traiter
 *             pvReserved : Reservé pour de futures évolutions
 * --------------------------------------------------------------------
 * RETURN    : Un (et un seul) flag parmi :
 *                     CM_WORKER_ACK_NONE
 *                     CM_WORKER_ACK_OK
 *                     CM_WORKER_ACK_RETRY_ERR
 *                     CM_WORKER_ACK_RETRY_BUSY
 *                     CM_WORKER_ACK_ABANDON
 *             indiquant le résultat du traitement, combiné avec
 *             un (et un seul) flag parmi :
 *                     CM_WORKER_ACT_NONE
 *                     CM_WORKER_ACT_MOVE_ACK
 *                     CM_WORKER_ACT_COPY_ERR
 *                     CM_WORKER_ACT_MOVE_ERR
 *                     CM_WORKER_ACT_DELETE
 *             indiquant l'action à mener en retour.
 *             GetLastError() donne un code d'erreur lorsque c'est
 *             applicable et NO_ERROR sinon.
 * --------------------------------------------------------------------
 * ROLE      : Effectue le traitement étendu d'un fichier
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI WorkerProcessFileEx( HANDLE hWrk, char * szFilePath, void * pvReserved );

#endif


typedef BOOL WINAPI CM_WORKER_PROCESS( HANDLE hWrk, char * szFilePath );

typedef DWORD WINAPI CM_WORKER_PROCESS_EX( HANDLE hWrk, char * szFilePath, void * pvReserved );




#endif
