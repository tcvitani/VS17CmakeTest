/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : communication asynchrone mailslot named pipe
 * --------------------------------------------------------------------
 * SUMMARY    : Librairie de communication asynchrone par callback
 *              orientée client / server basée sur le principe des
 *              workers.
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <reg.h>

#include <acom_dmem.h>
#define LOC_DEF
#include <acom.h>
#include <acom_priv.h>
#undef LOC_DEF
#include <acom_block.h>
#include <acom_init.h>
#include <acom_dbg.h>
#include <acom_tcp.h>

#include <memclass.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSet ( 
 *                              DWORD64 dwWksUsrKey,
 *                              DWORD dwMaxConnections, 
 *                              DWORD dwMaxInstances, 
 *                              DWORD dwPriority, 
 *                              DWORD dwConnectLoopDelay, 
 *                              DWORD dwWorkers, 
 *                              DWORD dwProtocol,
 *                              DWORD dwLifeTime,
 *                              DWORD dwMaxLife,
 *                              DWORD dwAllowedTimeShift,
 *                              ACOM_CALLBACK_SHUTDOWN * pfShut )
 * PARAMETERS: DWORD64 dwWksUsrKey               : Identifiant utilisateur du working set (passé en
 *                                               paramètre lors du callback de shutdown).
 *             DWORD dwMaxConnections          : Nombre maximum de connexions gérées par le working
 *                                               set (compter une pour chaque instance de client de
 *                                               mailslot, de serveur de mailslot, de client de named pipe,
 *                                               mais n pour un serveur de named pipe, n étant le nombre
 *                                               max de clients connectés sur ce serveur).
 *             DWORD dwMaxInstances            : Nombre maximum d'instances
 *             DWORD dwPriority                : Priorité des threads workers
 *             DWORD dwConnectLoopDelay        : Période max de poling du worker gérant les connexions
 *                                               en plus des i/o
 *             DWORD dwWorkers                 : Nombre de threads workers
 *             DWORD dwProtocol                : Type du protocol à mettre en place (ACOM_PROTOCOL_DEFAULT
 *                                               ou ACOM_PROTOCOL_LIFE).
 *                                               ATTENTION : Ce type de protocole ne fonctionne qu'avec
 *                                               les connexions bidirectionnelles (ne marche pas avec
 *                                               les mailslots).
 *                                               En complément, les flags suivants peuvent être cumulés
 *                                               par ou bit à bit :
 *                                                - ACOM_FLAG_GET_PEER_NAME : Recherche automatique du nom
 *                                                    du noeud distant (par défaut) lors de la connexion.
 *                                                - ACOM_FLAG_NO_GET_PEER_NAME : Désaactivation de la
 *                                                    recherche automatique du nom du noeud distant lors de
 *                                                    la connexion.
 *                                                - ACOM_FLAG_FIXED_LIFETIME : Le paramètre dwLifeTime est
 *                                                    utilisé des la connexion pour le timer de polling si
 *                                                    le protocol "life" est utilisé (par défaut).
 *                                                - ACOM_FLAG_LIFETIME_FACTOR(x) : A la connexion, le paramètre
 *                                                    dwLifeTime n'est pas utilisé pour le timer de polling. Le
 *                                                    premier timer est armé avec un délai augménté de x fois 
 *                                                    le paramètre dwLifeTime. La valeur de x accepté est
 *                                                    de 1 (délai = 2 fois dwLifeTime) à 7 (8 fois).
 *             DWORD dwLifeTime                : Délai de vie pour le protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwMaxLife                 : Nombre max de demande de vies infructueuse pour le
 *                                               protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwAllowedTimeShift        : Décalage d'heure GMT permis entre les deux extrémités.
 *                                               Sur dépassement de ce décallage, provoque un callback.
 *                                               Si 0, aucun callback. Valeur en ms.
 *                                               N'est utilisé qu'avec le protocole ACOM_PROTOCOL_LIFE.
 *             ACOM_CALLBACK_SHUTDOWN * pfShut : Procédure de callback pour l'arrêt innopiné du working
 *                                               set. Lorsque cela ce produit, le working set n'est plus
 *                                               dans un état cohérent.
 * RETURN    : Un handle de working set, ou NULL si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue l'initialisation d'un working set pour qu'il puisse recevoir des instances
 *             de pipes, de mailslot etc.
 * --------------------------------------------------------------------
 */
EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSet(
                DWORD64 dwWksUsrKey,
                DWORD dwMaxConnections,
                DWORD dwMaxInstances,
                DWORD dwPriority,
                DWORD dwConnectLoopDelay,
                DWORD dwWorkers,
                DWORD dwProtocol,
                DWORD dwLifeTime,
                DWORD dwMaxLife,
                DWORD dwAllowedTimeShift,
                ACOM_CALLBACK_SHUTDOWN * pfShut )
{
    ACOM_WKS_HANDLE hWks;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSet()" );

    hWks = (ACOM_WKS_HANDLE)AComInitOpenWorkingSet(
                dwWksUsrKey,
                dwMaxConnections,
                dwMaxInstances,
                dwPriority,
                dwConnectLoopDelay,
                dwWorkers,
                dwProtocol,
                dwLifeTime,
                dwMaxLife,
                dwAllowedTimeShift,
                ACOM_MODE_CALLBACK,
                pfShut,
                NULL );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSet return 0x%016X", hWks );
    return hWks;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetWM ( 
 *                              DWORD64 dwWksUsrKey,
 *                              DWORD dwMaxConnections, 
 *                              DWORD dwMaxInstances, 
 *                              DWORD dwPriority, 
 *                              DWORD dwConnectLoopDelay, 
 *                              DWORD dwWorkers, 
 *                              DWORD dwProtocol,
 *                              DWORD dwLifeTime,
 *                              DWORD dwMaxLife,
 *                              DWORD dwAllowedTimeShift,
 *                              HANDLE hObjEvent )
 * PARAMETERS: DWORD64 dwWksUsrKey               : Identifiant utilisateur du working set (passé en
 *                                               paramètre lors du callback de shutdown).
 *             DWORD dwMaxConnections          : Nombre maximum de connexions gérées par le working
 *                                               set (compter une pour chaque instance de client de
 *                                               mailslot, de serveur de mailslot, de client de named pipe,
 *                                               mais n pour un serveur de named pipe, n étant le nombre
 *                                               max de clients connectés sur ce serveur).
 *             DWORD dwMaxInstances            : Nombre maximum d'instances
 *             DWORD dwPriority                : Priorité des threads workers
 *             DWORD dwConnectLoopDelay        : Période max de poling du worker gérant les connexions
 *                                               en plus des i/o
 *             DWORD dwWorkers                 : Nombre de threads workers
 *             DWORD dwProtocol                : Type du protocol à mettre en place (ACOM_PROTOCOL_DEFAULT
 *                                               ou ACOM_PROTOCOL_LIFE).
 *                                               ATTENTION : Ce type de protocole ne fonctionne qu'avec
 *                                               les connexions bidirectionnelles (ne marche pas avec
 *                                               les mailslots).
 *                                               En complément, les flags suivants peuvent être cumulés
 *                                               par ou bit à bit :
 *                                                - ACOM_FLAG_GET_PEER_NAME : Recherche automatique du nom
 *                                                    du noeud distant (par défaut) lors de la connexion.
 *                                                - ACOM_FLAG_NO_GET_PEER_NAME : Désaactivation de la
 *                                                    recherche automatique du nom du noeud distant lors de
 *                                                    la connexion.
 *                                                - ACOM_FLAG_FIXED_LIFETIME : Le paramètre dwLifeTime est
 *                                                    utilisé des la connexion pour le timer de polling si
 *                                                    le protocol "life" est utilisé (par défaut).
 *                                                - ACOM_FLAG_LIFETIME_FACTOR(x) : A la connexion, le paramètre
 *                                                    dwLifeTime n'est pas utilisé pour le timer de polling. Le
 *                                                    premier timer est armé avec un délai augménté de x fois 
 *                                                    le paramètre dwLifeTime. La valeur de x accepté est
 *                                                    de 1 (délai = 2 fois dwLifeTime) à 7 (8 fois).
 *             DWORD dwLifeTime                : Délai de vie pour le protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwMaxLife                 : Nombre max de demande de vies infructueuse pour le
 *                                               protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwAllowedTimeShift        : Décalage d'heure GMT permis entre les deux extrémités.
 *                                               Sur dépassement de ce décallage, provoque un callback.
 *                                               Si 0, aucun callback. Valeur en ms.
 *                                               N'est utilisé qu'avec le protocole ACOM_PROTOCOL_LIFE.
 *             HWND hWndEvents                 : Fenêtre à laquelle envoyer un wm d'arrêt innopiné du
 *                                               working set. Lorsque cela ce produit, le working set 
 *                                               n'est plus dans un état cohérent.
 * RETURN    : Un handle de working set, ou NULL si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue l'initialisation d'un working set pour qu'il puisse recevoir des instances
 *             de pipes, de mailslot etc.
 * --------------------------------------------------------------------
 */
EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetWM(
                DWORD64 dwWksUsrKey,
                DWORD dwMaxConnections,
                DWORD dwMaxInstances,
                DWORD dwPriority,
                DWORD dwConnectLoopDelay,
                DWORD dwWorkers,
                DWORD dwProtocol,
                DWORD dwLifeTime,
                DWORD dwMaxLife,
                DWORD dwAllowedTimeShift,
                HWND hWndEvents )
{
    ACOM_WKS_HANDLE hWks;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSetWM()" );

    hWks = (ACOM_WKS_HANDLE)AComInitOpenWorkingSet(
                dwWksUsrKey,
                dwMaxConnections,
                dwMaxInstances,
                dwPriority,
                dwConnectLoopDelay,
                dwWorkers,
                dwProtocol,
                dwLifeTime,
                dwMaxLife,
                dwAllowedTimeShift,
                ACOM_MODE_WM,
                NULL,
                (HANDLE)hWndEvents );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSetWM return 0x%016X", hWks );
    return hWks;
}

/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetTQ ( 
 *                              DWORD64 dwWksUsrKey,
 *                              DWORD dwMaxConnections, 
 *                              DWORD dwMaxInstances, 
 *                              DWORD dwPriority, 
 *                              DWORD dwConnectLoopDelay, 
 *                              DWORD dwWorkers, 
 *                              DWORD dwProtocol,
 *                              DWORD dwLifeTime,
 *                              DWORD dwMaxLife,
 *                              DWORD dwAllowedTimeShift,
 *                              DWORD dwThreadId )
 * PARAMETERS: DWORD64 dwWksUsrKey               : Identifiant utilisateur du working set (passé en
 *                                               paramètre lors du callback de shutdown).
 *             DWORD dwMaxConnections          : Nombre maximum de connexions gérées par le working
 *                                               set (compter une pour chaque instance de client de
 *                                               mailslot, de serveur de mailslot, de client de named pipe,
 *                                               mais n pour un serveur de named pipe, n étant le nombre
 *                                               max de clients connectés sur ce serveur).
 *             DWORD dwMaxInstances            : Nombre maximum d'instances
 *             DWORD dwPriority                : Priorité des threads workers
 *             DWORD dwConnectLoopDelay        : Période max de poling du worker gérant les connexions
 *                                               en plus des i/o
 *             DWORD dwWorkers                 : Nombre de threads workers
 *             DWORD dwProtocol                : Type du protocol à mettre en place (ACOM_PROTOCOL_DEFAULT
 *                                               ou ACOM_PROTOCOL_LIFE).
 *                                               ATTENTION : Ce type de protocole ne fonctionne qu'avec
 *                                               les connexions bidirectionnelles (ne marche pas avec
 *                                               les mailslots).
 *                                               En complément, les flags suivants peuvent être cumulés
 *                                               par ou bit à bit :
 *                                                - ACOM_FLAG_GET_PEER_NAME : Recherche automatique du nom
 *                                                    du noeud distant (par défaut) lors de la connexion.
 *                                                - ACOM_FLAG_NO_GET_PEER_NAME : Désaactivation de la
 *                                                    recherche automatique du nom du noeud distant lors de
 *                                                    la connexion.
 *                                                - ACOM_FLAG_FIXED_LIFETIME : Le paramètre dwLifeTime est
 *                                                    utilisé des la connexion pour le timer de polling si
 *                                                    le protocol "life" est utilisé (par défaut).
 *                                                - ACOM_FLAG_LIFETIME_FACTOR(x) : A la connexion, le paramètre
 *                                                    dwLifeTime n'est pas utilisé pour le timer de polling. Le
 *                                                    premier timer est armé avec un délai augménté de x fois 
 *                                                    le paramètre dwLifeTime. La valeur de x accepté est
 *                                                    de 1 (délai = 2 fois dwLifeTime) à 7 (8 fois).
 *             DWORD dwLifeTime                : Délai de vie pour le protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwMaxLife                 : Nombre max de demande de vies infructueuse pour le
 *                                               protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwAllowedTimeShift        : Décalage d'heure GMT permis entre les deux extrémités.
 *                                               Sur dépassement de ce décallage, provoque un callback.
 *                                               Si 0, aucun callback. Valeur en ms.
 *                                               N'est utilisé qu'avec le protocole ACOM_PROTOCOL_LIFE.
 *             DWORD dwThreadId                : Id du thread auquel poster les événements
 * RETURN    : Un handle de working set, ou NULL si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue l'initialisation d'un working set pour qu'il puisse recevoir des instances
 *             de pipes, de mailslot etc.
 * --------------------------------------------------------------------
 */
EXPORT ACOM_WKS_HANDLE WINAPI AComOpenWorkingSetTQ(
                DWORD64 dwWksUsrKey,
                DWORD dwMaxConnections,
                DWORD dwMaxInstances,
                DWORD dwPriority,
                DWORD dwConnectLoopDelay,
                DWORD dwWorkers,
                DWORD dwProtocol,
                DWORD dwLifeTime,
                DWORD dwMaxLife,
                DWORD dwAllowedTimeShift,
                DWORD dwThreadId )
{
    ACOM_WKS_HANDLE hWks;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSetTQ()" );

    hWks = (ACOM_WKS_HANDLE)AComInitOpenWorkingSet(
                dwWksUsrKey,
                dwMaxConnections,
                dwMaxInstances,
                dwPriority,
                dwConnectLoopDelay,
                dwWorkers,
                dwProtocol,
                dwLifeTime,
                dwMaxLife,
                dwAllowedTimeShift,
                ACOM_MODE_TQ,
                NULL,
                (HANDLE)dwThreadId );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenWorkingSetTQ return 0x%016X", hWks );
    return hWks;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstance ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcPipeName,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwMaxConnections, 
 *                              DWORD                           dwOutBufferSize, 
 *                              DWORD                           dwInBufferSize, 
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                              ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                              ACOM_CALLBACK_RECEIVED        * pfRecv, 
 *                              ACOM_CALLBACK_SENT            * pfSent,
 *                              ACOM_CALLBACK_TIME_SHIFT      * pfTime )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcPipeName        : Nom du pipe (commence par \\.\pipe\)
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwMaxConnections  : Nombre maximum de clients connectés
 *                                                                 simultanément.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la connexion
 *                                                                 d'un client
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la déconnexion
 *                                                                 d'un client
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 *             ACOM_CALLBACK_TIME_SHIFT      * pfTime            : Procédure de callback pour le décalage
 *                                                                 d'heure.
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcPipeName,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwMaxConnections,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv,
                ACOM_CALLBACK_SENT            * pfSent,
                ACOM_CALLBACK_TIME_SHIFT      * pfTime )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstance(%s)", pcPipeName );

    sParams.sPipeServer.dwMaxConnections = dwMaxConnections;
    sParams.sPipeServer.dwOutBufferSize = dwOutBufferSize;
    sParams.sPipeServer.dwInBufferSize = dwInBufferSize;
    sParams.sPipeServer.dwQueueSize = dwQueueSize;
    sParams.sPipeServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = pfCnx;
    sParams.pfDcnx = pfDcnx;
    sParams.pfRecv = pfRecv;
    sParams.pfSent = pfSent;
    sParams.pfTime = pfTime;
    sParams.dwUseMode = ACOM_MODE_CALLBACK;
    sParams.hObjEvent = NULL;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstance return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstance ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcPipeName, 
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                              ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                              ACOM_CALLBACK_RECEIVED        * pfRecv, 
 *                              ACOM_CALLBACK_SENT            * pfSent,
 *                              ACOM_CALLBACK_TIME_SHIFT      * pfTime )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcPipeName        : Nom du pipe (commence par \\xxx\pipe\)
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la connexion
 *                                                                 à un serveur.
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la déconnexion
 *                                                                 d'un serveur
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 *             ACOM_CALLBACK_TIME_SHIFT      * pfTime            : Procédure de callback pour le décalage
 *                                                                 d'heure.
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcPipeName,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv,
                ACOM_CALLBACK_SENT            * pfSent,
                ACOM_CALLBACK_TIME_SHIFT      * pfTime )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstance(%s)", pcPipeName );

    sParams.sPipeClient.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sPipeClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = pfCnx;
    sParams.pfDcnx = pfDcnx;
    sParams.pfRecv = pfRecv;
    sParams.pfSent = pfSent;
    sParams.pfTime = pfTime;
    sParams.dwUseMode = ACOM_MODE_CALLBACK;
    sParams.hObjEvent = NULL;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstance return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstance ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcMailslotName, 
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                              ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                              ACOM_CALLBACK_RECEIVED        * pfRecv )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcMailslotName    : Nom du mailslot (commence par \\.\mailslot\)
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la mise en
 *                                                                 écoute du mailslot.
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la fin
 *                                                                 d'écoute du mailslot
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcMailslotName,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstance(%s)", pcMailslotName );

    sParams.sMailServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = pfCnx;
    sParams.pfDcnx = pfDcnx;
    sParams.pfRecv = pfRecv;
    sParams.pfSent = NULL;
    sParams.pfTime = NULL;
    sParams.dwUseMode = ACOM_MODE_CALLBACK;
    sParams.hObjEvent = NULL;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstance return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstance ( 
 *                      ACOM_WKS_HANDLE                 hWks,
 *                      char                          * pcMailslotName,
 *                      DWORD64                           dwInstUsrKey, 
 *                      DWORD                           dwTimeToReconnect, 
 *                      DWORD                           dwQueueSize, 
 *                      DWORD                           dwMaxMessageSize, 
 *                      ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                      ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                      ACOM_CALLBACK_SENT            * pfSent )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcMailslotName    : Nom du mailslot (commence par \\xxx\mailslot\)
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour l'init
 *                                                                 du client de mailslot.
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la fin
 *                                                                 du client de mailslot
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcMailslotName,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_SENT            * pfSent )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstance(%s)", pcMailslotName );

    sParams.sMailClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = pfCnx;
    sParams.pfDcnx = pfDcnx;
    sParams.pfRecv = NULL;
    sParams.pfSent = pfSent;
    sParams.pfTime = NULL;
    sParams.dwUseMode = ACOM_MODE_CALLBACK;
    sParams.hObjEvent = NULL;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstance return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstance ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwMaxConnections, 
 *                              DWORD                           dwOutBufferSize, 
 *                              DWORD                           dwInBufferSize, 
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                              ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                              ACOM_CALLBACK_RECEIVED        * pfRecv, 
 *                              ACOM_CALLBACK_SENT            * pfSent,
 *                              ACOM_CALLBACK_TIME_SHIFT      * pfTime )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse d'écoute pour les connexion.
 *                                                                 Si chaine vide : toutes les adresses locales.
 *             DWORD                           dwPort            : Numéro du port d'écoute à utiliser
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwMaxConnections  : Nombre maximum de clients connectés
 *                                                                 simultanément.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la connexion
 *                                                                 d'un client
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la déconnexion
 *                                                                 d'un client
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 *             ACOM_CALLBACK_TIME_SHIFT      * pfTime            : Procédure de callback pour le décalage
 *                                                                 d'heure.
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwMaxConnections,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv,
                ACOM_CALLBACK_SENT            * pfSent,
                ACOM_CALLBACK_TIME_SHIFT      * pfTime )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstance( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpServer.dwMaxConnections = dwMaxConnections;
        sParams.sTcpServer.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpServer.dwInBufferSize = dwInBufferSize;
        sParams.sTcpServer.dwQueueSize = dwQueueSize;
        sParams.sTcpServer.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpServer.wPort = (WORD)dwPort;
        sParams.pfCnx = pfCnx;
        sParams.pfDcnx = pfDcnx;
        sParams.pfRecv = pfRecv;
        sParams.pfSent = pfSent;
        sParams.pfTime = pfTime;
        sParams.dwUseMode = ACOM_MODE_CALLBACK;
        sParams.hObjEvent = NULL;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_SERVER,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstance return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstance ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwOutBufferSize,
 *                              DWORD                           dwInBufferSize,
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              ACOM_CALLBACK_CONNECTION      * pfCnx, 
 *                              ACOM_CALLBACK_DISCONNECTION   * pfDcnx, 
 *                              ACOM_CALLBACK_RECEIVED        * pfRecv, 
 *                              ACOM_CALLBACK_SENT            * pfSent,
 *                              ACOM_CALLBACK_TIME_SHIFT      * pfTime )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse IP (texte ou numérique du serveur)
 *             DWORD                           dwPort            : Port d'écoute du serveur
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la connexion
 *                                                                 à un serveur.
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la déconnexion
 *                                                                 d'un serveur
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 *             ACOM_CALLBACK_TIME_SHIFT      * pfTime            : Procédure de callback pour le décalage
 *                                                                 d'heure.
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstance(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv,
                ACOM_CALLBACK_SENT            * pfSent,
                ACOM_CALLBACK_TIME_SHIFT      * pfTime )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstance( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpClient.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpClient.dwQueueSize = dwQueueSize;
        sParams.sTcpClient.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpClient.dwInBufferSize = dwInBufferSize;
        sParams.sTcpClient.wPort = (WORD)dwPort;
        sParams.pfCnx = pfCnx;
        sParams.pfDcnx = pfDcnx;
        sParams.pfRecv = pfRecv;
        sParams.pfSent = pfSent;
        sParams.pfTime = pfTime;
        sParams.dwUseMode = ACOM_MODE_CALLBACK;
        sParams.hObjEvent = NULL;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_CLIENT,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstance return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstance(
 *                          ACOM_WKS_HANDLE                 hWks,
 *                          DWORD                           dwPort,
 *                          char                          * pcParameters,
 *                          DWORD64                           dwInstUsrKey,
 *                          DWORD                           dwTimeToReconnect,
 *                          DWORD                           dwOutBufferSize,
 *                          DWORD                           dwInBufferSize,
 *                          DWORD                           dwQueueSize,
 *                          DWORD                           dwMaxMessageSize,
 *                          DWORD                           dwReadInterval,
 *                          ACOM_CALLBACK_CONNECTION      * pfCnx,
 *                          ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
 *                          ACOM_CALLBACK_RECEIVED        * pfRecv,
 *                          ACOM_CALLBACK_SENT            * pfSent,
 *                          ACOM_CALLBACK_TIME_SHIFT      * pfTime )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             DWORD                           dwPort            : Numéro du port série
 *             char                          * pcParameters      : Paramétrage du port série selon la syntaxe de
 *                                                                 la commande "MODE COM"
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du port
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du port
 *             DWORD                           dwQueueSize       : Nombre max de message en attente
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD                           dwReadInterval    : Délai maximum entre deux caractères
 *             ACOM_CALLBACK_CONNECTION      * pfCnx             : Procédure de callback pour la connexion
 *                                                                 du port
 *             ACOM_CALLBACK_DISCONNECTION   * pfDcnx            : Procédure de callback pour la déconnexion
 *                                                                 du port
 *             ACOM_CALLBACK_RECEIVED        * pfRecv            : Procédure de callback pour la réception
 *                                                                 de données
 *             ACOM_CALLBACK_SENT            * pfSent            : Procédure de callback pour la fin
 *                                                                 d'émission de données
 *             ACOM_CALLBACK_TIME_SHIFT      * pfTime            : Procédure de callback pour le décalage
 *                                                                 d'heure.
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de communication par port série dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callbacks
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstance(
                ACOM_WKS_HANDLE                 hWks,
                DWORD                           dwPort,
                char                          * pcParameters,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                DWORD                           dwReadInterval,
                ACOM_CALLBACK_CONNECTION      * pfCnx,
                ACOM_CALLBACK_DISCONNECTION   * pfDcnx,
                ACOM_CALLBACK_RECEIVED        * pfRecv,
                ACOM_CALLBACK_SENT            * pfSent,
                ACOM_CALLBACK_TIME_SHIFT      * pfTime )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstance(%d)", dwPort );

    sParams.sSerial.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sSerial.dwQueueSize = dwQueueSize;
    sParams.sSerial.dwInBufferSize = dwInBufferSize;
    sParams.sSerial.dwOutBufferSize = dwOutBufferSize;
    sParams.sSerial.dwPort = dwPort;
    sParams.sSerial.dwReadInterval = dwReadInterval;
    sParams.pfCnx = pfCnx;
    sParams.pfDcnx = pfDcnx;
    sParams.pfRecv = pfRecv;
    sParams.pfSent = pfSent;
    sParams.pfTime = pfTime;
    sParams.dwUseMode = ACOM_MODE_CALLBACK;
    sParams.hObjEvent = NULL;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_SERIAL,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcParameters );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstance return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceWM ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcPipeName,
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwMaxConnections, 
 *                              DWORD             dwOutBufferSize, 
 *                              DWORD             dwInBufferSize, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              HWND              hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcPipeName        : Nom du pipe (commence par \\.\pipe\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié àcette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwMaxConnections  : Nombre maximum de clients connectés
 *                                                   simultanément.
 *             DWORD             dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD             dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND              hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceWM(
                ACOM_WKS_HANDLE   hWks,
                char            * pcPipeName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwMaxConnections,
                DWORD             dwOutBufferSize,
                DWORD             dwInBufferSize,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                HWND              hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstanceWM(%s)", pcPipeName );

    sParams.sPipeServer.dwMaxConnections = dwMaxConnections;
    sParams.sPipeServer.dwOutBufferSize = dwOutBufferSize;
    sParams.sPipeServer.dwInBufferSize = dwInBufferSize;
    sParams.sPipeServer.dwQueueSize = dwQueueSize;
    sParams.sPipeServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_WM;
    sParams.hObjEvent = hWndEvents;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstanceWM return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceWM ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcPipeName, 
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              HWND              hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcPipeName        : Nom du pipe (commence par \\xxx\pipe\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND              hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceWM(
                ACOM_WKS_HANDLE   hWks,
                char            * pcPipeName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                HWND              hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstanceWM(%s)", pcPipeName );

    sParams.sPipeClient.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sPipeClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_WM;
    sParams.hObjEvent = hWndEvents;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstanceWM return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceWM ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcMailslotName, 
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwMaxMessageSize, 
 *                              HWND              hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcMailslotName    : Nom du mailslot (commence par \\.\mailslot\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND              hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceWM(
                ACOM_WKS_HANDLE   hWks,
                char            * pcMailslotName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwMaxMessageSize,
                HWND              hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstanceWM(%s)", pcMailslotName );

    sParams.sMailServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_WM;
    sParams.hObjEvent = (HANDLE)hWndEvents;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstanceWM return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceWM ( 
 *                              ACOM_WKS_HANDLE   hWks,
 *                              char            * pcMailslotName,
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              HWND              hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcMailslotName    : Nom du mailslot (commence par \\xxx\mailslot\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND              hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceWM(
                ACOM_WKS_HANDLE   hWks,
                char            * pcMailslotName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                HWND              hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstanceWM(%s)", pcMailslotName );

    sParams.sMailClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_WM;
    sParams.hObjEvent = (HANDLE)hWndEvents;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstanceWM return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceWM ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwMaxConnections, 
 *                              DWORD                           dwOutBufferSize, 
 *                              DWORD                           dwInBufferSize, 
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              HWND                            hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse d'écoute pour les connexion.
 *                                                                 Si chaine vide : toutes les adresses locales.
 *             DWORD                           dwPort            : Numéro du port d'écoute à utiliser
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwMaxConnections  : Nombre maximum de clients connectés
 *                                                                 simultanément.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND                            hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur TCP dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceWM(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwMaxConnections,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                HWND                            hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstanceWM( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpServer.dwMaxConnections = dwMaxConnections;
        sParams.sTcpServer.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpServer.dwInBufferSize = dwInBufferSize;
        sParams.sTcpServer.dwQueueSize = dwQueueSize;
        sParams.sTcpServer.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpServer.wPort = (WORD)dwPort;
        sParams.pfCnx = NULL;
        sParams.pfDcnx = NULL;
        sParams.pfRecv = NULL;
        sParams.pfSent = NULL;
        sParams.pfTime = NULL;
        sParams.dwUseMode = ACOM_MODE_WM;
        sParams.hObjEvent = (HANDLE)hWndEvents;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_SERVER,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstanceWM return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceWM ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwOutBufferSize,
 *                              DWORD                           dwInBufferSize,
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              HWND                            hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse IP (texte ou numérique du serveur)
 *             DWORD                           dwPort            : Port d'écoute du serveur
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             HWND                            hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceWM(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                HWND                            hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstanceWM( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpClient.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpClient.dwQueueSize = dwQueueSize;
        sParams.sTcpClient.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpClient.dwInBufferSize = dwInBufferSize;
        sParams.sTcpClient.wPort = (WORD)dwPort;
        sParams.pfCnx = NULL;
        sParams.pfDcnx = NULL;
        sParams.pfRecv = NULL;
        sParams.pfSent = NULL;
        sParams.pfTime = NULL;
        sParams.dwUseMode = ACOM_MODE_WM;
        sParams.hObjEvent = (HANDLE)hWndEvents;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_CLIENT,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstanceWM return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceWM(
 *                          ACOM_WKS_HANDLE                 hWks,
 *                          DWORD                           dwPort,
 *                          char                          * pcParameters,
 *                          DWORD64                           dwInstUsrKey,
 *                          DWORD                           dwTimeToReconnect,
 *                          DWORD                           dwOutBufferSize,
 *                          DWORD                           dwInBufferSize,
 *                          DWORD                           dwQueueSize,
 *                          DWORD                           dwMaxMessageSize,
 *                          DWORD                           dwReadInterval,
 *                          HWND                            hWndEvents )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             DWORD                           dwPort            : Numéro du port série
 *             char                          * pcParameters      : Paramétrage du port série selon la syntaxe de
 *                                                                 la commande "MODE COM"
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du port
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du port
 *             DWORD                           dwQueueSize       : Nombre max de message en attente
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD                           dwReadInterval    : Délai maximum entre deux caractères
 *             HWND                            hWndEvents        : Fenêtre à laquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de communication par port série dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callbacks
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceWM(
                ACOM_WKS_HANDLE                 hWks,
                DWORD                           dwPort,
                char                          * pcParameters,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                DWORD                           dwReadInterval,
                HWND                            hWndEvents )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstanceWM(%d)", dwPort );

    sParams.sSerial.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sSerial.dwQueueSize = dwQueueSize;
    sParams.sSerial.dwInBufferSize = dwInBufferSize;
    sParams.sSerial.dwOutBufferSize = dwOutBufferSize;
    sParams.sSerial.dwPort = dwPort;
    sParams.sSerial.dwReadInterval = dwReadInterval;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.pfTime = NULL;
    sParams.dwUseMode = ACOM_MODE_WM;
    sParams.hObjEvent = (HANDLE)hWndEvents;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_SERIAL,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcParameters );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstanceWM return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceTQ ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcPipeName,
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwMaxConnections, 
 *                              DWORD             dwOutBufferSize, 
 *                              DWORD             dwInBufferSize, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              DWORD             dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcPipeName        : Nom du pipe (commence par \\.\pipe\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié àcette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwMaxConnections  : Nombre maximum de clients connectés
 *                                                   simultanément.
 *             DWORD             dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD             dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD             dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeServerInstanceTQ(
                ACOM_WKS_HANDLE   hWks,
                char            * pcPipeName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwMaxConnections,
                DWORD             dwOutBufferSize,
                DWORD             dwInBufferSize,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                DWORD             dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstanceTQ(%s)", pcPipeName );

    sParams.sPipeServer.dwMaxConnections = dwMaxConnections;
    sParams.sPipeServer.dwOutBufferSize = dwOutBufferSize;
    sParams.sPipeServer.dwInBufferSize = dwInBufferSize;
    sParams.sPipeServer.dwQueueSize = dwQueueSize;
    sParams.sPipeServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_TQ;
    sParams.hObjEvent = (HANDLE)dwThreadId;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeServerInstanceTQ return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceTQ ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcPipeName, 
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              DWORD             dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcPipeName        : Nom du pipe (commence par \\xxx\pipe\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD             dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenPipeClientInstanceTQ(
                ACOM_WKS_HANDLE   hWks,
                char            * pcPipeName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                DWORD             dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstanceTQ(%s)", pcPipeName );

    sParams.sPipeClient.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sPipeClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_TQ;
    sParams.hObjEvent = (HANDLE)dwThreadId;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_PIPE_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcPipeName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenPipeClientInstanceTQ return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceTQ ( 
 *                              ACOM_WKS_HANDLE   hWks, 
 *                              char            * pcMailslotName, 
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwMaxMessageSize, 
 *                              DWORD             dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcMailslotName    : Nom du mailslot (commence par \\.\mailslot\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD             dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotServerInstanceTQ(
                ACOM_WKS_HANDLE   hWks,
                char            * pcMailslotName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwMaxMessageSize,
                DWORD             dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstanceTQ(%s)", pcMailslotName );

    sParams.sMailServer.dwMaxMessageSize = dwMaxMessageSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_TQ;
    sParams.hObjEvent = (HANDLE)dwThreadId;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_SERVER,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotServerInstanceTQ return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceTQ ( 
 *                              ACOM_WKS_HANDLE   hWks,
 *                              char            * pcMailslotName,
 *                              DWORD64             dwInstUsrKey, 
 *                              DWORD             dwTimeToReconnect, 
 *                              DWORD             dwQueueSize, 
 *                              DWORD             dwMaxMessageSize, 
 *                              DWORD             dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE   hWks              : Handle de working set retourné
 *                                                   par AComOpenWorkingSet
 *             char            * pcMailslotName    : Nom du mailslot (commence par \\xxx\mailslot\)
 *             DWORD64             dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                   fournie en paramètre à chaque callback
 *                                                   lié à cette instance.
 *             DWORD             dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                   déconnexion a lieu.
 *             DWORD             dwQueueSize       : Nombre max de message en attent
 *                                                   d'émission. Si la file est saturée, la
 *                                                   connexion est rompue.
 *             DWORD             dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD             dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de mailslot dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenMailslotClientInstanceTQ(
                ACOM_WKS_HANDLE   hWks,
                char            * pcMailslotName,
				DWORD64             dwInstUsrKey,
                DWORD             dwTimeToReconnect,
                DWORD             dwQueueSize,
                DWORD             dwMaxMessageSize,
                DWORD             dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstanceTQ(%s)", pcMailslotName );

    sParams.sMailClient.dwQueueSize = dwQueueSize;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.dwUseMode = ACOM_MODE_TQ;
    sParams.hObjEvent = (HANDLE)dwThreadId;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_MAIL_CLIENT,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcMailslotName );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenMailslotClientInstanceTQ return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceTQ ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwMaxConnections, 
 *                              DWORD                           dwOutBufferSize, 
 *                              DWORD                           dwInBufferSize, 
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              DWORD                           dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse d'écoute pour les connexion.
 *                                                                 Si chaine vide : toutes les adresses locales.
 *             DWORD                           dwPort            : Numéro du port d'écoute à utiliser
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwMaxConnections  : Nombre maximum de clients connectés
 *                                                                 simultanément.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD                           dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de serveur TCP dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpServerInstanceTQ(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwMaxConnections,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                DWORD                           dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstanceTQ( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpServer.dwMaxConnections = dwMaxConnections;
        sParams.sTcpServer.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpServer.dwInBufferSize = dwInBufferSize;
        sParams.sTcpServer.dwQueueSize = dwQueueSize;
        sParams.sTcpServer.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpServer.wPort = (WORD)dwPort;
        sParams.pfCnx = NULL;
        sParams.pfDcnx = NULL;
        sParams.pfRecv = NULL;
        sParams.pfSent = NULL;
        sParams.pfTime = NULL;
        sParams.dwUseMode = ACOM_MODE_TQ;
        sParams.hObjEvent = (HANDLE)dwThreadId;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_SERVER,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpServerInstanceTQ return 0x%016X", hACom );
    return hACom;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceTQ ( 
 *                              ACOM_WKS_HANDLE                 hWks, 
 *                              char                          * pcAddress,
 *                              DWORD                           dwPort,
 *                              DWORD64                           dwInstUsrKey, 
 *                              DWORD                           dwTimeToReconnect, 
 *                              DWORD                           dwOutBufferSize,
 *                              DWORD                           dwInBufferSize,
 *                              DWORD                           dwQueueSize, 
 *                              DWORD                           dwMaxMessageSize, 
 *                              DWORD                           dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             char                          * pcAddress         : Adresse IP (texte ou numérique du serveur)
 *             DWORD                           dwPort            : Port d'écoute du serveur
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié à cette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du pipe
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du pipe
 *             DWORD                           dwQueueSize       : Nombre max de message en attent
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD                           dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de client de pipe dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenTcpClientInstanceTQ(
                ACOM_WKS_HANDLE                 hWks,
                char                          * pcAddress,
                DWORD                           dwPort,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                DWORD                           dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstanceTQ( %s , %u )", pcAddress, dwPort );

    dwErr = AComTcpInit();
    if ( dwErr == NO_ERROR )
    {
        sParams.sTcpClient.dwMaxMessageSize = dwMaxMessageSize;
        sParams.sTcpClient.dwQueueSize = dwQueueSize;
        sParams.sTcpClient.dwOutBufferSize = dwOutBufferSize;
        sParams.sTcpClient.dwInBufferSize = dwInBufferSize;
        sParams.sTcpClient.wPort = (WORD)dwPort;
        sParams.pfCnx = NULL;
        sParams.pfDcnx = NULL;
        sParams.pfRecv = NULL;
        sParams.pfSent = NULL;
        sParams.pfTime = NULL;
        sParams.dwUseMode = ACOM_MODE_TQ;
        sParams.hObjEvent = (HANDLE)dwThreadId;

        hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                    (ACOM_WORKING_SET*)hWks,
                    ACOM_INSTANCE_TCP_CLIENT,
                    dwInstUsrKey,
                    dwTimeToReconnect,
                    &sParams,
                    pcAddress );
    }
    else
    {
        AComDbgError( dwErr, __FILE__, __LINE__, "Erreur initialisation socket" );
        hACom = NULL;
    }

    AComDbgInfo( __FILE__, __LINE__, "AComOpenTcpClientInstanceTQ return 0x%016X", hACom );
    return hACom;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceTQ(
 *                          ACOM_WKS_HANDLE                 hWks,
 *                          DWORD                           dwPort,
 *                          char                          * pcParameters,
 *                          DWORD64                           dwInstUsrKey,
 *                          DWORD                           dwTimeToReconnect,
 *                          DWORD                           dwOutBufferSize,
 *                          DWORD                           dwInBufferSize,
 *                          DWORD                           dwQueueSize,
 *                          DWORD                           dwMaxMessageSize,
 *                          DWORD                           dwReadInterval,
 *                          DWORD                           dwThreadId )
 * PARAMETERS: ACOM_WKS_HANDLE                 hWks              : Handle de working set retourné
 *                                                                 par AComOpenWorkingSet
 *             DWORD                           dwPort            : Numéro du port série
 *             char                          * pcParameters      : Paramétrage du port série selon la syntaxe de
 *                                                                 la commande "MODE COM"
 *             DWORD64                           dwInstUsrKey      : Clé d'instance utilisateur qui sera
 *                                                                 fournie en paramètre à chaque callback
 *                                                                 lié àcette instance.
 *             DWORD                           dwTimeToReconnect : Délai avant reconnexion lorsqu'une
 *                                                                 déconnexion a lieu.
 *             DWORD                           dwOutBufferSize   : Taille des buffers de sortie du port
 *             DWORD                           dwInBufferSize    : Taille des buffers d'entrée du port
 *             DWORD                           dwQueueSize       : Nombre max de message en attente
 *                                                                 d'émission. Si la file est saturée, la
 *                                                                 connexion est rompue.
 *             DWORD                           dwMaxMessageSize  : Taille maximum d'un message.
 *             DWORD                           dwReadInterval    : Délai maximum entre deux caractères
 *             DWORD                           dwThreadId        : Thread auquelle envoyer un wm d'évènement
 * RETURN    : Un handle d'instance, NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise une instance de communication par port série dans un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callbacks
 * --------------------------------------------------------------------
 */
EXPORT ACOM_INST_HANDLE WINAPI AComOpenSerialPortInstanceTQ(
                ACOM_WKS_HANDLE                 hWks,
                DWORD                           dwPort,
                char                          * pcParameters,
				DWORD64                           dwInstUsrKey,
                DWORD                           dwTimeToReconnect,
                DWORD                           dwOutBufferSize,
                DWORD                           dwInBufferSize,
                DWORD                           dwQueueSize,
                DWORD                           dwMaxMessageSize,
                DWORD                           dwReadInterval,
                DWORD                           dwThreadId )
{
    ACOM_PARAM sParams;
    ACOM_INST_HANDLE hACom;

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstanceTQ(%d)", dwPort );

    sParams.sSerial.dwMaxMessageSize = dwMaxMessageSize;
    sParams.sSerial.dwQueueSize = dwQueueSize;
    sParams.sSerial.dwInBufferSize = dwInBufferSize;
    sParams.sSerial.dwOutBufferSize = dwOutBufferSize;
    sParams.sSerial.dwPort = dwPort;
    sParams.sSerial.dwReadInterval = dwReadInterval;
    sParams.pfCnx = NULL;
    sParams.pfDcnx = NULL;
    sParams.pfRecv = NULL;
    sParams.pfSent = NULL;
    sParams.pfTime = NULL;
    sParams.dwUseMode = ACOM_MODE_TQ;
    sParams.hObjEvent = (HANDLE)dwThreadId;

    hACom = (ACOM_INST_HANDLE)AComInitOpenInstance(
                (ACOM_WORKING_SET*)hWks,
                ACOM_INSTANCE_SERIAL,
                dwInstUsrKey,
                dwTimeToReconnect,
                &sParams,
                pcParameters );

    AComDbgInfo( __FILE__, __LINE__, "AComOpenSerialPortInstanceTQ return 0x%016X", hACom );
    return hACom;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComCloseInstance ( ACOM_INST_HANDLE hInst )
 * PARAMETERS: ACOM_INST_HANDLE hInst : Handle d'instance ouverte avec  AComOpenXXXXInstance()
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme une instance (les connexions sont fermées, les messages
 *             en attente d'émission sont perdus).
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComCloseInstance( ACOM_INST_HANDLE hInst )
{
    AComDbgInfo( __FILE__, __LINE__, "AComCloseInstance(0x%016X)", hInst );
    AComInitCloseInstance( (ACOM_INSTANCE *)hInst );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComCloseWorkingSet ( ACOM_WKS_HANDLE hWks, DWORD dwTimeout )
 * PARAMETERS: ACOM_WKS_HANDLE hWks : Handle de working set retourné par AComOpenWorkingSet
 *             DWORD dwTimeout      : Temps maximum autorisé pour la fermeture du working set.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ferme un working set.
 *             ATTENTION : Cette fonction ne peut et ne doit pas être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComCloseWorkingSet( ACOM_WKS_HANDLE hWks, DWORD dwTimeout )
{
    AComDbgInfo( __FILE__, __LINE__, "AComCloseWorkingSet(0x%016X)", hWks );
    AComInitCloseWorkingSet( (ACOM_WORKING_SET *)hWks, dwTimeout);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComSendMessage ( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle, DWORD64 dwMsgUsrKey, DWORD dwDataSize, void * pvData )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             DWORD64 dwMsgUsrKey           : Clé utilisateur d'identification du block de données, passé
 *                                           en paramètre lors du callback de fin d'émission
 *             DWORD dwDataSize            : Taille des données à envoyer
 *             void * pvData               : Pointe sur le bloc de données
 * RETURN    : NO_ERROR si données placées en file d'attente, une erreur Win32 sinon.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Demande l'émission de données sur une connexion d'un working set.
 *             ATTENTION : le retour de NO_ERROR ne signifie pas que le message a été envoyé.
 *             Il aura été envoyé lorsque le callback de fin d'émission aura été appelé pour
 *             la clé utilisateur dwMsgUsrKey.
 *             REMARQUE  : Cette fonction peut être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComSendMessage(
                ACOM_WKS_HANDLE hWks,
                ACOM_CNX_HANDLE hCnxHandle,
				DWORD64 dwMsgUsrKey,
                DWORD dwDataSize,
                void * pvData )
{
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
	DWORD dwErr = NO_ERROR;
    DWORD dwHeaderSize = 0;
    ACOM_LIFE_HEADER sHeader;
    ACOM_LIFE_HEADER * psHeader = NULL;

    AComDbgInfo( __FILE__, __LINE__, "AComSendMessage(0x%016X,0x%016X,0x%016X)", hWks, hCnxHandle, dwMsgUsrKey );

    // Si protocole LIFE, on ajoute l'entête
    if ( ACOM_GET_PROTOCOL(psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
    {
        // L'heure système sera mise à jour au moment du WriteFile
        sHeader.dwType = ACOM_LIFE_DATA;
        sHeader.dwDataSize = dwDataSize;
        psHeader = &sHeader;
        dwHeaderSize = sizeof( sHeader );
    }

    if ( dwIndex < psWks->dwMaxConnections )
        dwErr = AComBlockPost(
            psWks->hCompletion,
            ACOM_COMMAND_SEND,
            &psWks->psCnx[dwIndex],
            hCnxHandle,
            0,
            dwMsgUsrKey,
            FALSE,
            dwHeaderSize,
            psHeader,
            dwDataSize,
            pvData );
    else
        dwErr = ERROR_INVALID_HANDLE;

    AComDbgInfo( __FILE__, __LINE__, "AComSendMessage return %d", dwErr );
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComDisconnectPeer ( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle, BOOL bDisable  )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             BOOL bDisable               : Si FALSE : la connexion sera rendue inutilisable, les workers
 *                                           ne tentent plus de la rétablir.
 *                                           Si TRUE  : Rien ne change
 * RETURN    : NO_ERROR si demande prise en compte, une erreur Win32 sinon.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Demande la déconnexion d'une connexion d'un working set.
 *             ATTENTION : le retour de NO_ERROR ne signifie pas que la déconnexion a eu lieu.
 *             Celle-ci aura été effectuée lorsque le callback de déconnexion aura été appelé.
 *             ATTENTION : Une fois déconnecté, si l'instance liée n'est pas fermée, la reconnexion 
 *             sera effectuée automatiquement.
 *             REMARQUE  : Cette fonction peut être appelée depuis un des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComDisconnectPeer(
                ACOM_WKS_HANDLE hWks,
                ACOM_CNX_HANDLE hCnxHandle,
                BOOL bDisable )
{
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
	DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComDisconnectPeer(0x%016X,0x%016X,%s)", hWks, hCnxHandle, bDisable ? "TRUE" : "FALSE" );

    if ( dwIndex < psWks->dwMaxConnections )
        dwErr = AComBlockPost(
            psWks->hCompletion,
            ACOM_COMMAND_DISCONNECT,
            &psWks->psCnx[dwIndex],
            hCnxHandle,
            0,
            0,
            bDisable,
            0,
            NULL,
            0,
            NULL );
    else
        dwErr = ERROR_INVALID_HANDLE;

    AComDbgInfo( __FILE__, __LINE__, "AComDisconnectPeer return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetConnectionStatistics ( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle, ULONGLONG * pullNow, ULONGLONG * pullCnxDate, ULONGLONG * pullInBytes, ULONGLONG * pullOutBytes, ULONGLONG * pullInMsg, ULONGLONG * pullOutMsg )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             ULONGLONG * pullNow         : Récupère la date système courante (x100ns)
 *             ULONGLONG * pullCnxDate     : Récupère la date de connexion (x100ns)
 *             ULONGLONG * pullInBytes     : Nombre d'octets reçus
 *             ULONGLONG * pullOutBytes    : Nombre d'octets envoyés 
 *             ULONGLONG * pullInMsg       : Nombre de messages reçus
 *             ULONGLONG * pullOutMsg      : Nombre de messages envoyés
 * RETURN    : NO_ERROR si ok, sinon, un code win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les statistiques liées à une connexion.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetConnectionStatistics(
                ACOM_WKS_HANDLE hWks,
                ACOM_CNX_HANDLE hCnxHandle,
                ULONGLONG * pullNow,
                ULONGLONG * pullCnxDate,
                ULONGLONG * pullInBytes,
                ULONGLONG * pullOutBytes,
                ULONGLONG * pullInMsg,
                ULONGLONG * pullOutMsg )
{
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    ACOM_CONNECTION * psCnx;
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComGetConnectionStatistics(0x%016X,0x%016X)", hWks, hCnxHandle );

    // Inutile de passer par la section critique du working
    // set. Les données accèdées ne sont mises à jour que lors
    // de son init.
    if ( dwIndex >= psWks->dwMaxConnections )
    {
        dwErr = ERROR_INVALID_HANDLE;
        AComDbgError( dwErr, __FILE__, __LINE__, "hCnxHandle" );
    }
    else
    {
        psCnx = &psWks->psCnx[dwIndex];
        EnterCriticalSection( &psCnx->sCnxCritical );
        if ( ( psCnx->hCnxHandle != hCnxHandle  ) ||
             ( psCnx->dwState != ACOM_CONNECTION_STATE_CONNECTED ) )
        {
            dwErr = ERROR_INVALID_HANDLE;
            AComDbgError( dwErr, __FILE__, __LINE__, "hCnxHandle" );
        }
        else
        {
            *pullCnxDate = psCnx->ullCnxDate;
            *pullInBytes = psCnx->ullInBytes;
            *pullOutBytes = psCnx->ullOutBytes;
            *pullInMsg = psCnx->ullInMsg;
            *pullOutMsg = psCnx->ullOutMsg;
        }
        LeaveCriticalSection( &psCnx->sCnxCritical );
    }

    GetSystemTimeAsFileTime( (LPFILETIME)pullNow );

    AComDbgInfo( __FILE__, __LINE__, "AComGetConnectionStatistics return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetWorkersStatistics ( ACOM_WKS_HANDLE hWks, DWORD * pdwMaxConcurrentThreads, ULONGLONG * pullIdleTime, ULONGLONG * pullUserTime, ULONGLONG * pullKernelTime )
 * PARAMETERS: ACOM_WKS_HANDLE hWks            : Handle de working set retourné par AComOpenWorkingSet
 *             DWORD * pdwMaxConcurrentThreads : Nombre maximum de threads ayant tourné simultanément.
 *             ULONGLONG * pullIdleTime        : Temps total des workers passé endormi (x100ns)
 *             ULONGLONG * pullUserTime        : Temps total des workers passé en mode user (x100ns)
 *             ULONGLONG * pullKernelTime      : Temps total des workers passé en mode kernel (x100ns)
 * RETURN    : NO_ERROR si ok, un code Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les stats liées aux threads workers d'un working set.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetWorkersStatistics(
                ACOM_WKS_HANDLE hWks,
                DWORD * pdwMaxConcurrentThreads,
                ULONGLONG * pullIdleTime,
                ULONGLONG * pullUserTime,
                ULONGLONG * pullKernelTime )
{
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    DWORD dwIndex;
    BOOL bResult;
    ULONGLONG ullNow;
    ULONGLONG ullStart;
    ULONGLONG ullEnd;
    ULONGLONG ullKernel;
    ULONGLONG ullUser;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComGetWorkersStatistics(0x%016X)", hWks );

    (*pullIdleTime) = 0;
    (*pullUserTime) = 0;
    (*pullKernelTime) = 0;

    GetSystemTimeAsFileTime( (LPFILETIME)&ullNow );


    for ( dwIndex = 0 ; dwIndex < psWks->dwWorkers ; dwIndex ++ )
    {
        bResult = GetThreadTimes(
                        psWks->phWorkerThreads[dwIndex],
                        (LPFILETIME)&ullStart,
                        (LPFILETIME)&ullEnd,
                        (LPFILETIME)&ullKernel,
                        (LPFILETIME)&ullUser );
        if ( ! bResult )
        {
            dwErr = GetLastError();
            AComDbgError( dwErr, __FILE__, __LINE__, "GetThreadTimes" );
            break;
        }
        (*pullUserTime) += ullUser;
        (*pullKernelTime) += ullKernel;
        (*pullIdleTime) += ( ullNow - ullStart );
    }

    if ( dwErr == NO_ERROR )
    {
        (*pullIdleTime) -= ( (*pullKernelTime) + (*pullUserTime) );

        EnterCriticalSection( &psWks->sWksCritical );
        (*pdwMaxConcurrentThreads) = psWks->dwMaxWorkerConcurrency;
        LeaveCriticalSection( &psWks->sWksCritical );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComGetWorkersStatistics return %d", dwErr );

    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetConnectionEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, ACOM_CNX_HANDLE * phCnxHandle )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent        : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey         : Récupère la clé utilisateur associée à l'instance
 *                                             concernée par l'événement
 *             ACOM_CNX_HANDLE * phCnxHandle : Récupère le handle de la nouvelle connexion.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_CONNECTION
 *             reçu par window-message
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetConnectionEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
                        ACOM_CNX_HANDLE * phCnxHandle )
{
    ACOM_EVT_CONNECTION * psEvt = (ACOM_EVT_CONNECTION*)hEvent;
	AComDbgInfo(__FILE__, __LINE__, "AComGetConnectionEvtParameters(0x%016X)-evt(0x%016X)", psEvt->hCnxHandle, hEvent);
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *phCnxHandle = psEvt->hCnxHandle;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetDisconnectionEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, DWORD64 * pdwCnxUsrKey )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey  : Récupère la clé utilisateur associée à l'instance
 *                                      concernée par l'événement
 *             DWORD64 * pdwCnxUsrKey   : Récupère la clé utilisateur associée à la connexion
 *                                      concernée par l'évènement
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_DISCONNECTION
 *             reçu par window-message
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetDisconnectionEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
						DWORD64 * pdwCnxUsrKey)
{
    ACOM_EVT_DISCONNECTION * psEvt = (ACOM_EVT_DISCONNECTION *)hEvent;
    AComDbgInfo( __FILE__, __LINE__, "AComGetDisconnectionEvtParameters(0x%016X)", hEvent );
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *pdwCnxUsrKey = psEvt->dwCnxUsrKey;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetReceivedEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, DWORD64 * pdwCnxUsrKey, DWORD * pdwDataSize, void * pvDataBuffer )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey  : Récupère la clé utilisateur associée à l'instance
 *                                      concernée par l'événement
 *             DWORD64 * pdwCnxUsrKey   : Récupère la clé utilisateur associée à la connexion
 *                                      concernée par l'évènement
 *             DWORD * pdwDataSize    : Récupère la taille des données du buffer
 *             void ** ppvDataBuffer  : Récupère un pointeur sur le buffer
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_RECEIVED
 *             reçu par window-message. Le buffer reste valide tant que l'évènement n'a pas
 *             été libéré
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetReceivedEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
						DWORD64 * pdwCnxUsrKey,
                        DWORD * pdwDataSize,
                        void ** ppvDataBuffer )
{
    ACOM_EVT_RECEIVED * psEvt = (ACOM_EVT_RECEIVED*)hEvent;
    DWORD dwBufSize = *pdwDataSize;
    AComDbgInfo( __FILE__, __LINE__, "AComGetReceiveEvtParameters(0x%016X)", hEvent );
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *pdwCnxUsrKey = psEvt->dwCnxUsrKey;
    *pdwDataSize = psEvt->tdwData[0];
    *ppvDataBuffer = (void*)(&psEvt->tdwData[1]);
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComCopyReceivedEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, DWORD64 * pdwCnxUsrKey, DWORD * pdwDataSize, void * pvDataBuffer )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey  : Récupère la clé utilisateur associée à l'instance
 *                                      concernée par l'événement
 *             DWORD64 * pdwCnxUsrKey   : Récupère la clé utilisateur associée à la connexion
 *                                      concernée par l'évènement
 *             DWORD * pdwDataSize    : En entrée, fourni la taille du buffer pointé par pvDataBuffer,
 *                                      en sortie, récupère la taille des données placées dans pvDataBuffer.
 *                                      Si pvDataBuffer est NULL ou trop petit, *pdwDataSize récupère
 *                                      la taille de buffer nécessaire
 *             void * pvDataBuffer    : Pointe sur le buffer destiné à recevoir les données. Si NULL,
 *                                      *pdwDataSize récupère la taille de buffer nécessaire
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_RECEIVED
 *             reçu par window-message en copiant le contenu du buffer.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComCopyReceivedEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
						DWORD64 * pdwCnxUsrKey,
                        DWORD * pdwDataSize,
                        void * pvDataBuffer )
{
    ACOM_EVT_RECEIVED * psEvt = (ACOM_EVT_RECEIVED*)hEvent;
    DWORD dwBufSize = *pdwDataSize;
    AComDbgInfo( __FILE__, __LINE__, "AComCopyReceiveEvtParameters(0x%016X)", hEvent );
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *pdwCnxUsrKey = psEvt->dwCnxUsrKey;
    *pdwDataSize = psEvt->tdwData[0];
    if ( ( psEvt->tdwData[0] <= dwBufSize ) && ( pvDataBuffer != NULL ) )
        CopyMemory( pvDataBuffer, &psEvt->tdwData[1], psEvt->tdwData[0] );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetSentEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, DWORD64 * pdwCnxUsrKey, DWORD64 * pdwMsgUsrKey, DWORD * pdwError )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey  : Récupère la clé utilisateur associée à l'instance
 *                                      concernée par l'événement
 *             DWORD64 * pdwCnxUsrKey   : Récupère la clé utilisateur associée à la connexion
 *                                      concernée par l'évènement
 *             DWORD64 * pdwMsgUsrKey   : Récupère la clé utilisateur associée au bloc de données
 *                                      effectivement envoyé ou qui n'a pu être envoyé
 *             DWORD * pdwError       : Récupère le code d'erreur de l'émission. Si NO_ERROR, le
 *                                      bloc de données a été envoyé, sinon, il n'a pu l'être.
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_SENT
 *             reçu par window-message
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetSentEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
						DWORD64 * pdwCnxUsrKey,
						DWORD64 * pdwMsgUsrKey,
                        DWORD * pdwError )
{
    ACOM_EVT_SENT * psEvt = (ACOM_EVT_SENT*)hEvent;
    AComDbgInfo( __FILE__, __LINE__, "AComGetSentEvtParameters(0x%016X)", hEvent );
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *pdwCnxUsrKey = psEvt->dwCnxUsrKey;
    *pdwMsgUsrKey = psEvt->dwMsgUsrKey;
    *pdwError = psEvt->dwError;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetTimeShiftEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwInstUsrKey, DWORD64 * pdwCnxUsrKey )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwInstUsrKey  : Récupère la clé utilisateur associée à l'instance
 *                                      concernée par l'événement
 *             DWORD64 * pdwCnxUsrKey   : Récupère la clé utilisateur associée à la connexion
 *                                      concernée par l'évènement
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_TIME_SHIFT
 *             reçu par window-message
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetTimeShiftEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwInstUsrKey,
						DWORD64 * pdwCnxUsrKey)
{
    ACOM_EVT_TIME_SHIFT * psEvt = (ACOM_EVT_TIME_SHIFT *)hEvent;
    AComDbgInfo( __FILE__, __LINE__, "AComGetTimeShiftEvtParameters(0x%016X)", hEvent );
    *pdwInstUsrKey = psEvt->dwInstUsrKey;
    *pdwCnxUsrKey = psEvt->dwCnxUsrKey;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetShutdownEvtParameters ( ACOM_EVT_HANDLE hEvent, DWORD64 * pdwWksUsrKey, DWORD * pdwError )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 *             DWORD64 * pdwWksUsrKey   : Récupère la clé utilisateur associée au working set concerné
 *             DWORD * pdwError       : Récupère le code Win32 de l'erreur à l'origine du shutdown
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère les informations associées à un événement de type ACOM_WM_SHUTDOWN
 *             reçu par window-message
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetShutdownEvtParameters( 
                        ACOM_EVT_HANDLE hEvent, 
						DWORD64 * pdwWksUsrKey,
                        DWORD * pdwError )
{
    ACOM_EVT_SHUTDOWN * psEvt = (ACOM_EVT_SHUTDOWN*)hEvent;
    AComDbgInfo( __FILE__, __LINE__, "AComGetShutdownEvtParameters(0x%016X)", hEvent );
    *pdwWksUsrKey = psEvt->dwWksUsrKey;
    *pdwError = psEvt->dwError;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComReleaseEvt ( ACOM_EVT_HANDLE hEvent )
 * PARAMETERS: ACOM_EVT_HANDLE hEvent : Handle de l'événement reçu par window-message
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Libère un évènement reçu par window message.
 *             ATTETION : Cette fonction doit être appelée systématiquement une fois
 *             l'évènement traité. Lorsqu'elle a été appelé, le handle de l'évènement
 *             n'est plus valide.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComReleaseEvt( ACOM_EVT_HANDLE hEvent )
{
    AComDbgInfo( __FILE__, __LINE__, "AComReleaseEvt(0x%016X)", hEvent );
    DMEM_FREE( (void*)hEvent );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetDefaultWmEvtId (  )
 * PARAMETERS: Aucun
 * RETURN    : Identifiant du window message que le module utilise.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Retourne le code de window message par défaut utilisé par
 *             les working set pour signaler les évènements.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetDefaultWmEvtId()
{
    AComDbgInfo( __FILE__, __LINE__, "AComGetWmDefaultEvtId()" );
    AComDbgInfo( __FILE__, __LINE__, "AComGetWmDefaultEvtId return %d", gdwWmAComEvent );
    return gdwWmAComEvent;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComSetDefaultWmEvtId( DWORD dwWmId )
 * PARAMETERS: dwWmId               : Identifiant du window message que les
 *                                    working set doivent utiliser par défaut.
 * RETURN    : NO_ERROR si ok, un code Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Défini le code de window message par défaut utilisé par les
 *             working set pour signaler les évènements (par défaut, au chargement
 *             de la librairie, le code est initialisé à l'aide d'un
 *             appel à RegisterWindowMessage() avec la chaine "WM_ACOM_EVENT".
 *             Remarque : la fonction retourne une erreur si le code
 *             fourni est inférieur à WM_USER.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComSetDefaultWmEvtId( DWORD dwWmId )
{
    DWORD   dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComSetDefaultWmEvtId( %u )", dwWmId );
    if ( dwWmId >= WM_USER )
        gdwWmAComEvent = dwWmId;
    else
        dwErr = ERROR_INVALID_PARAMETER;

    AComDbgInfo( __FILE__, __LINE__, "AComSetDefaultWmEvtId return 0x%016X", dwErr );

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetWmEvtId ( ACOM_WKS_HANDLE hWks )
 * PARAMETERS: ACOM_WKS_HANDLE hWks : Handle de working set retourné par AComOpenWorkingSet
 * RETURN    : Identifiant du window message que le working set utilise.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Retourne le code de window message utilisé par
 *             un working set pour signaler les évènements.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetWmEvtId( ACOM_WKS_HANDLE hWks )
{
    AComDbgInfo( __FILE__, __LINE__, "AComGetWmEvtId( 0x%016X )", hWks );
    AComDbgInfo( __FILE__, __LINE__, "AComGetWmEvtId return %d", ((ACOM_WORKING_SET*)hWks)->dwWmAComEvent );
    return ((ACOM_WORKING_SET*)hWks)->dwWmAComEvent;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComSetWmEvtId( ACOM_WKS_HANDLE hWks, DWORD dwWmId )
 * PARAMETERS: ACOM_WKS_HANDLE hWks : Handle de working set retourné par AComOpenWorkingSet
 *             dwWmId               : Identifiant du window message que le working set doit
 *                                    utiliser.
 * RETURN    : NO_ERROR si ok, un code Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Défini le code de window message utilisé par le working set
 *             pour signaler les évènements (par défaut, au chargement
 *             de la librairie, le code est initialisé à l'aide d'un
 *             appel à RegisterWindowMessage() avec la chaine "WM_ACOM_EVENT".
 *             Remarque 1: la fonction retourne une erreur si le code
 *               fourni est inférieur à WM_USER.
 *             Remarque 2: il est préférable d'appeler cette fonction avant
 *               de créer des instances dans le working set pour éviter un
 *               changement de window message en cours de communication.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComSetWmEvtId( 
                            ACOM_WKS_HANDLE hWks, 
                            DWORD           dwWmId )
{
    DWORD   dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComSetWmEvtId( 0x%016X, %u )", hWks, dwWmId );
    if ( dwWmId >= WM_USER )
        ((ACOM_WORKING_SET*)hWks)->dwWmAComEvent = dwWmId;
    else
        dwErr = ERROR_INVALID_PARAMETER;

    AComDbgInfo( __FILE__, __LINE__, "AComSetWmEvtId return 0x%016X", dwErr );

    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComStartExclusiveAccess ( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: ACOM_WKS_HANDLE hWks       : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion, retourné par le callback de connexion
 * RETURN    : NO_ERROR si on est entré en accès exclusif sur la connexion, une erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Commence un accès exclusif à une connexion. Tant qu'on ne sort pas de l'accès exclusif,
 *             aucun nouvel évènement n'est déclenché sur la connexion.
 *             ATTENTION : Seules les fonctions AComSendMessage() et AComDisconnectPeer() sont
 *             acceptées à l'intérieur d'un accès exclusif, si elles portent sur la même connexion
 *             que l'accès exclusif lui-même.
 *             ATTENTION : Il est possible d'imbriquer des accès exclusifs sur la même connexion, mais
 *             il est fortement déconseillé de les imbriqués sur des connexion différentes.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComStartExclusiveAccess( 
                ACOM_WKS_HANDLE hWks,
                ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    ACOM_CONNECTION * psCnx;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComTryExclusiveAccess(0x%016X,0x%016X)", hWks, hCnxHandle );

    if ( dwIndex < psWks->dwMaxConnections )
    {
        psCnx = &psWks->psCnx[dwIndex];
        EnterCriticalSection( &psCnx->sCnxCritical );
        if ( psCnx->hCnxHandle != hCnxHandle )
        {
            dwErr = ERROR_INVALID_HANDLE;
            LeaveCriticalSection( &psCnx->sCnxCritical );
        }
    }
    else
        dwErr = ERROR_INVALID_HANDLE;

    AComDbgInfo( __FILE__, __LINE__, "AComTryExclusiveAccess return %d", dwErr );
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComEndExclusiveAccess ( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle )
 * PARAMETERS: ACOM_WKS_HANDLE hWks       : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle : Handle de connexion, retourné par le callback de connexion
 * RETURN    : NO_ERROR si on est sorti d'accès exclusif sur la connexion, une erreur Win32 sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Terine l'accès exclusif à une connexion.
 *             ATTENTION : Cette fonction ne peut être appelée que lorsque l'accès exclusif
 *             a été effectivement obtenu par AComStartExclusiveAccess().
 *             ATTENTION : Seules les fonctions AComSendMessage() et AComDisconnectPeer() sont
 *             acceptées à l'intérieur d'un accès exclusif, si elles portent sur la même connexion
 *             que l'accès exclusif lui-même.
 *             ATTENTION : Il est possible d'imbriquer des accès exclusifs sur la même connexion, mais
 *             il est fortement déconseillé de les imbriqués sur des connexion différentes.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComEndExclusiveAccess( 
                ACOM_WKS_HANDLE hWks,
                ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    ACOM_CONNECTION * psCnx;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComEndExclusiveAccess(0x%016X,0x%016X)", hWks, hCnxHandle );

    if ( dwIndex < psWks->dwMaxConnections )
    {
        psCnx = &psWks->psCnx[dwIndex];
        LeaveCriticalSection( &psCnx->sCnxCritical );
    }
    else
        dwErr = ERROR_INVALID_HANDLE;

    AComDbgInfo( __FILE__, __LINE__, "AComEndExclusiveAccess return %d", dwErr );
    return dwErr;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComOpenTrace( BOOL bInfo, BOOL bError, char * pcFileName )
 * PARAMETERS: BOOL bInfo        : Etat d'activation des traces informatives élémentaires
 *             BOOL bError       : Etat d'activation des traces d'erreur
 *             char * pcFileName : Nom du fichier de trace
 * RETURN    : NO_ERROR si trace activée, ERROR_ALREADY_EXISTS si déjà activée, un autre
 *             code Win32 en cas d'erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Active la trace de toutes les instances de tous les workings set ACOM
 *             ouverts dans le process courant.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComOpenTrace(
                BOOL bInfo,
                BOOL bError,
                char * pcFileName )
{
    return AComDbgOpen( bInfo, bError, pcFileName );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComCloseTrace()
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Désactive la trace de toutes les instances de tous les workings set ACOM
 *             ouverts dans le process courant.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComCloseTrace()
{
    AComDbgClose();
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetGMTPeerTimeAsFileTime( ACOM_WKS_HANDLE   hWks, ACOM_CNX_HANDLE   hCnxHandle, LONGLONG        * pllDate )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             ULONGLONG * pullDate        : Récupère une estimation de la date à l'autre bout de la connexion
 * RETURN    : NO_ERROR si ok, sinon, un code win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère une estimation de la date GMT à l'autre bout de la connexion.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetGMTPeerTimeAsFileTime(
                ACOM_WKS_HANDLE   hWks,
                ACOM_CNX_HANDLE   hCnxHandle,
                LONGLONG        * pllDate )
{
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    ACOM_CONNECTION * psCnx;
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    DWORD dwErr = NO_ERROR;
    LONGLONG llNow;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTimeAsFileTime(0x%016X,0x%016X)", hWks, hCnxHandle );

    if ( ACOM_GET_PROTOCOL(psWks->dwProtocol) == ACOM_PROTOCOL_LIFE )
    {
        // Inutile de passer par la section critique du working
        // set. Les données accèdées ne sont mises à jour que lors
        // de son init.
        if ( dwIndex >= psWks->dwMaxConnections )
        {
            dwErr = ERROR_INVALID_HANDLE;
            AComDbgError( dwErr, __FILE__, __LINE__, "hCnxHandle" );
        }
        else
        {
            psCnx = &psWks->psCnx[dwIndex];
            EnterCriticalSection( &psCnx->sCnxCritical );
            if ( ( psCnx->hCnxHandle != hCnxHandle  ) ||
                 ( psCnx->dwState != ACOM_CONNECTION_STATE_CONNECTED ) )
            {
                dwErr = ERROR_INVALID_HANDLE;
                AComDbgError( dwErr, __FILE__, __LINE__, "hCnxHandle" );
            }
            else
            {
                GetSystemTimeAsFileTime( (LPFILETIME)&llNow );
                (*pllDate) = psCnx->llLastRemoteDate + ( llNow - psCnx->llLastLocalDate );
            }
            LeaveCriticalSection( &psCnx->sCnxCritical );
        }
    }
    else
        dwErr = ERROR_INVALID_DATA;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTimeAsFileTime return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetGMTPeerTime(
 *                       ACOM_WKS_HANDLE   hWks,
 *                       ACOM_CNX_HANDLE   hCnxHandle,
 *                       DWORD           * pdwYear,
 *                       DWORD           * pdwMonth,
 *                       DWORD           * pdwDay,
 *                       DWORD           * pdwHour,
 *                       DWORD           * pdwMinute,
 *                       DWORD           * pdwSecond,
 *                       DWORD           * pdwMilli )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             pdwXXXX                     : Récupère les données de la date estimatée à l'autre bout de
 *                                           la connexion
 * RETURN    : NO_ERROR si ok, sinon, un code win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère une estimation de la date GMT à l'autre bout de la connexion.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetGMTPeerTime(
                ACOM_WKS_HANDLE   hWks,
                ACOM_CNX_HANDLE   hCnxHandle,
                DWORD           * pdwYear,
                DWORD           * pdwMonth,
                DWORD           * pdwDay,
                DWORD           * pdwHour,
                DWORD           * pdwMinute,
                DWORD           * pdwSecond,
                DWORD           * pdwMilli )
{
    DWORD dwErr = NO_ERROR;
    LONGLONG llNow;
    SYSTEMTIME sTime;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTime(0x%016X,0x%016X)", hWks, hCnxHandle );
    dwErr = AComGetGMTPeerTimeAsFileTime( hWks, hCnxHandle, &llNow );
    if ( dwErr == NO_ERROR )
    {
        if ( ! FileTimeToSystemTime( (LPFILETIME)&llNow, &sTime ) )
            dwErr = GetLastError();
        else
        {
            *pdwYear = sTime.wYear;
            *pdwMonth = sTime.wMonth;
            *pdwDay = sTime.wDay;
            *pdwHour = sTime.wHour;
            *pdwMinute = sTime.wMinute;
            *pdwSecond = sTime.wSecond;
            *pdwMilli = sTime.wMilliseconds;
        }
    }
    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTime return %d", dwErr );
    return dwErr;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI AComGetGMTTime(
 *                       DWORD           * pdwYear,
 *                       DWORD           * pdwMonth,
 *                       DWORD           * pdwDay,
 *                       DWORD           * pdwHour,
 *                       DWORD           * pdwMinute,
 *                       DWORD           * pdwSecond,
 *                       DWORD           * pdwMilli )
 * PARAMETERS: 
 *             pdwXXXX                     : Récupère les données de la date GMT du système local
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère la date GMT du système local
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI AComGetGMTTime(
                DWORD           * pdwYear,
                DWORD           * pdwMonth,
                DWORD           * pdwDay,
                DWORD           * pdwHour,
                DWORD           * pdwMinute,
                DWORD           * pdwSecond,
                DWORD           * pdwMilli )
{
    SYSTEMTIME sTime;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTTime()" );
    GetSystemTime( &sTime );
    *pdwYear = sTime.wYear;
    *pdwMonth = sTime.wMonth;
    *pdwDay = sTime.wDay;
    *pdwHour = sTime.wHour;
    *pdwMinute = sTime.wMinute;
    *pdwSecond = sTime.wSecond;
    *pdwMilli = sTime.wMilliseconds;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTTime return" );
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComSynchronize( ACOM_WKS_HANDLE   hWks, ACOM_CNX_HANDLE   hCnxHandle )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 * RETURN    : NO_ERROR si ok, sinon, un code win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Synchronise le système local de manière à ce qu'il ait les memes dates et heures que le
 *             système distant.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComSynchronize(
                ACOM_WKS_HANDLE   hWks,
                ACOM_CNX_HANDLE   hCnxHandle )
{
    DWORD dwErr = NO_ERROR;
    LONGLONG llNow;
    SYSTEMTIME sTime;

    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTime(0x%016X,0x%016X)", hWks, hCnxHandle );
    dwErr = AComGetGMTPeerTimeAsFileTime( hWks, hCnxHandle, &llNow );
    if ( dwErr == NO_ERROR )
    {
        if ( ! FileTimeToSystemTime( (LPFILETIME)&llNow, &sTime ) )
            dwErr = GetLastError();
        else if ( ! SetSystemTime( &sTime ) )
                dwErr = GetLastError();
    }
    AComDbgInfo( __FILE__, __LINE__, "AComGetGMTPeerTime return %d", dwErr );
    return dwErr;
}




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComSetNullSessionPipe( char * pcPipeName, BOOL bActivate )
 * PARAMETERS: pcPipeName : Nom complet du pipe ( "\\\\xxxx\\pipe\\nomdupipe" )
 *             bActivate  : Si TRUE, le pipe est placé dans la liste null session, si
 *                          FALSE, il en est retiré.
 * RETURN    : NO_ERROR si ok, sinon, un code win32.
 *             Si bActivate est FALSE et que le pipe n'est pas dans la liste, aucune erreur n'est
 *             provoquée, de même que si bActivate est TRUE et que le pipe est déjà dans la liste.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un nom de pipe dans la liste des pipes pour lesquels une session nulle est
 *             autorisée. La liste est permanente, un fois le pipe ajouté, le nom y reste, même
 *             après un redémarrage.
 *             Attention, pour que le pipe soit pris en compte, il faut que le service
 *             Server (LanmanServer) soit redémarré (ou mieux, que la machine soit
 *             redémarrée).
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComSetNullSessionPipe( char * pcPipeName, BOOL bActivate )
{
    DWORD dwSize = (DWORD)strlen( pcPipeName );
    DWORD dwPos = (DWORD)strlen( pcPipeName );
    DWORD dwSrc;
    DWORD dwDst;
	DWORD dwErr = NO_ERROR;
    DWORD dwType;
    char szShort[MAX_PATH];
    char szPipeSrc[20000];
    char szPipeDst[20000+MAX_PATH];

    if ( dwSize == 0 )
        return ERROR_INVALID_PARAMETER;
    dwPos = 0;

    if ( _strnicmp( pcPipeName + dwPos, "\\\\", 2 ) != 0 )
        return ERROR_INVALID_PARAMETER;
    dwPos += 2;

    while ( ( pcPipeName[dwPos] != '\\' ) && ( pcPipeName[dwPos] != '\0' ) )
        dwPos ++;
    if ( pcPipeName[dwPos] == '\0' )
        return ERROR_INVALID_PARAMETER;
    dwPos ++;

    if ( _strnicmp( &pcPipeName[dwPos], "pipe\\", 5 ) != 0 )
        return ERROR_INVALID_PARAMETER;
    dwPos += 5;

    dwSize -= dwPos;
    if ( dwSize == 0 )
        return ERROR_INVALID_PARAMETER;
    
    if ( dwSize >= sizeof(szShort) )
        return ERROR_NOT_ENOUGH_MEMORY;

    strcpy_s( szShort, MAX_PATH, pcPipeName + dwPos );
    _strupr_s( szShort, MAX_PATH );

    dwSize = sizeof( szPipeSrc );
    dwErr = REG_Lire( 
            HKEY_LOCAL_MACHINE, 
            "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters", 
            "NullSessionPipes", 
            &dwType,
            szPipeSrc, 
            &dwSize );
    if ( dwErr == ERROR_FILE_NOT_FOUND )
    {
        strcpy_s( szPipeSrc, 20000, "\0\0" );
        dwSize = 2;
    }
    else if ( dwErr != NO_ERROR )
        return dwErr;

    if ( dwSize < 2 )
    {
        strcpy_s( szPipeSrc, 20000, "\0\0" );
        dwSize = 2;
    }

    if ( ( szPipeSrc[dwSize-1] != '\0' ) && ( szPipeSrc[dwSize-2] != '\0' ) )
        return ERROR_INVALID_DATA;

    dwSrc = 0;
    dwDst = 0;

    while ( ( dwSize = (DWORD)strlen( szPipeSrc + dwSrc ) ) != 0 )
    {
        if ( _stricmp( szPipeSrc + dwSrc, szShort ) == 0 )
        {
            if ( bActivate )
                return NO_ERROR;
            else
                dwSrc += ( dwSize + 1 );
        }
        else
        {
            strcpy_s( szPipeDst + dwDst, 20000, szPipeSrc + dwSrc );
            dwSrc += ( dwSize + 1 );
            dwDst += ( dwSize + 1 );
        }
    }
    if ( bActivate )
    {
        strcpy_s( szPipeDst + dwDst, 20000, szShort );
        dwDst += ( (DWORD)strlen( szShort ) + 1 );
    }
    szPipeDst[dwDst] = '\0';
    dwDst ++;

    return REG_Ecrire( 
            HKEY_LOCAL_MACHINE, 
            "SYSTEM\\CurrentControlSet\\Services\\LanmanServer\\Parameters", 
            "NullSessionPipes", 
            dwType,
            szPipeDst, 
            dwDst );
}        






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI AComGetPeer( ACOM_WKS_HANDLE hWks, ACOM_CNX_HANDLE hCnxHandle, char * pcPeer, DWORD dwPeerBytes )
 * PARAMETERS: ACOM_WKS_HANDLE hWks        : Handle de working set retourné par AComOpenWorkingSet
 *             ACOM_CNX_HANDLE hCnxHandle  : Handle de connexion, retourné par le callback de connexion
 *                                           de l'instance.
 *             char *          pcPeer      : Récupère l'identification du correspondant
 *             DWORD           dwPeerBytes : Taille max du buffer pointé par pcPeer
 * RETURN    : NO_ERROR si ok, sinon, un code win32
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Récupère une identification texte de la connexion.
 *             REMARQUE  : Cette fonction peut être appelée depuis une des callback
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI AComGetPeer( 
                ACOM_WKS_HANDLE hWks, 
                ACOM_CNX_HANDLE hCnxHandle, 
                char * pcPeer,
                DWORD dwPeerBytes )
{
    DWORD dwIndex = hCnxHandle%ACOM_MAX_WKS_CONNECTIONS;
    ACOM_WORKING_SET * psWks = (ACOM_WORKING_SET *)hWks;
    ACOM_CONNECTION * psCnx;
    DWORD dwErr = NO_ERROR;

    AComDbgInfo( __FILE__, __LINE__, "AComGetPeer(0x%016X,0x%016X)", hWks, hCnxHandle );

    if ( dwIndex >= psWks->dwMaxConnections )
    {
        dwErr = ERROR_INVALID_HANDLE;
        AComDbgError( dwErr, __FILE__, __LINE__, "hCnxHandle" );
    }
    else if ( dwPeerBytes == 0 )
    {
        dwErr = ERROR_INVALID_HANDLE;
        AComDbgError( dwErr, __FILE__, __LINE__, "dwPeerBytes" );
    }
    else
    {
        psCnx = &psWks->psCnx[dwIndex];
        if ( dwPeerBytes > sizeof(psCnx->szIdentity) )
            dwPeerBytes = sizeof(psCnx->szIdentity);
        
        EnterCriticalSection( &psCnx->sCnxCritical );

        if ( psCnx->dwState == ACOM_CONNECTION_STATE_CONNECTED )
        {
            strncpy_s( pcPeer, dwPeerBytes, psCnx->szIdentity, dwPeerBytes );
            pcPeer[dwPeerBytes-1] = 0;
        }
        else
            dwErr = ERROR_NOT_READY;

        LeaveCriticalSection( &psCnx->sCnxCritical );
    }

    AComDbgInfo( __FILE__, __LINE__, "AComGetPeer return %d", dwErr );

    return dwErr;
}





/* -------------  FIN DU FICHIER : acom.c ------------- */ 
