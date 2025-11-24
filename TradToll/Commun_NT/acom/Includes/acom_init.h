/* --------------------------------------------------------------------
 * (C) 1999 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : acom
 * FILE       : acom_init.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions internes d'initialisation et de terminaison
 *              des working-set, des instances et des connexions.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef ACOM_INIT_H
#define ACOM_INIT_H

#include <protect.h>


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_WORKING_SET * AComInitOpenWorkingSet ( 
 *                      DWORD64 dwWksUsrKey,
 *                      DWORD dwMaxConnections, 
 *                      DWORD dwMaxInstances, 
 *                      DWORD dwPriority, 
 *                      DWORD dwConnectLoopDelay, 
 *                      DWORD dwWorkers, 
 *                      DWORD dwProtocol,
 *                      DWORD dwLifeTime,
 *                      DWORD dwMaxLife,
 *                      DWORD dwAllowedTimeShift,
 *                      DWORD dwUseMode,
 *                      ACOM_CALLBACK_SHUTDOWN * pfShut,
 *                      HANDLE hObjEvent )
 * PARAMETERS: DWORD64 dwWksUsrKey               : Clé utilisateur associé au working set (pour les
 *                                               callback)
 *             DWORD dwMaxConnections          : Nombre total maximum de connexions pour le working set
 *             DWORD dwMaxInstances            : Nombre total maximum d'instances pour le working set
 *             DWORD dwPriority                : Priorité de traitement des thread workers
 *             DWORD dwConnectLoopDelay        : Fréquence de test de l'état des connexions
 *             DWORD dwWorkers                 : Nombre de thread worker
 *             DWORD dwProtocol                : Type du protocol à mettre en place (ACOM_PROTOCOL_DEFAULT
 *                                               ou ACOM_PROTOCOL_LIFE).
 *                                               ATTENTION : Ce type de protocole ne fonctionne qu'avec
 *                                               les connexions bidirectionnelles (ne marche pas avec
 *                                               les mailslots).
 *             DWORD dwLifeTime                : Délai de vie pour le protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwMaxLife                 : Nombre max de demande de vies infructueuse pour le
 *                                               protocole ACOM_PROTOCOL_LIFE.
 *                                               Non utilisé si un autre protocole
 *             DWORD dwAllowedTimeShift        : Décalage d'heure GMT permis entre les deux extrémités.
 *                                               Sur dépassement de ce décallage, provoque un callback.
 *                                               Si 0, aucun callback. Valeur en ms.
 *                                               N'est utilisé qu'avec le protocole ACOM_PROTOCOL_LIFE.
 *             DWORD dwUseMode                 : Mode d'utilisation du working set (ACOM_MODE_XXXX)
 *             ACOM_CALLBACK_SHUTDOWN * pfShut : Callback d'erreur critique sur le working set si non null
 *             HANDLE hObjEvent                : En fonction de dwUseMode, handle de la fenêtre ou id du
 *                                               thread recevant les window messages de shutdown.
 * RETURN    : Un pointeur sur une structure de working set ou NULL si erreur.
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création et l'initialisation d'un working set.
 * --------------------------------------------------------------------
 */
PROTECTED ACOM_WORKING_SET * AComInitOpenWorkingSet(
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
                DWORD dwUseMode,
                ACOM_CALLBACK_SHUTDOWN * pfShut,
                HANDLE hObjEvent );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED ACOM_INSTANCE * AComInitOpenInstance ( 
 *                      ACOM_WORKING_SET * psWks, 
 *                      DWORD              dwInstType, 
 *                      DWORD64              dwInstUsrKey, 
 *                      DWORD              dwTimeToReconnect, 
 *                      ACOM_PARAM       * psParams, 
 *                      char             * pcFileName )
 * PARAMETERS: ACOM_WORKING_SET * psWks             : Structure de working set ouvert
 *             DWORD              dwInstType        : Type de l'instance (ACOM_INSTANCE_XXXX)
 *             DWORD64              dwInstUsrKey      : Clé utilisateur d'instance (pour callback)
 *             DWORD              dwTimeToReconnect : Délai de reconnexion en cas de déconnexion.
 *             ACOM_PARAM       * psParams          : Structure des paramètres d'instance
 *             char             * pcFileName        : Nom de "fichier" de l'instance dans le
 *                                                    format : \\Machine\Type\Nom
 * RETURN    : Un pointeur sur une structure d'instance, NULL si erreur
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la création et l'initialisation d'une instance dans un working set.
 * --------------------------------------------------------------------
 */
ACOM_INSTANCE * AComInitOpenInstance( 
                ACOM_WORKING_SET * psWks,
                DWORD              dwInstType,
				DWORD64              dwInstUsrKey,
                DWORD              dwTimeToReconnect,
                ACOM_PARAM       * psParams,
                char             * pcFileName );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComInitCloseInstance ( ACOM_INSTANCE * psInst )
 * PARAMETERS: ACOM_INSTANCE * psInst : Structure d'instance
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la fermetue d'une instance ouverte ou partiellement ouverte
 * --------------------------------------------------------------------
 */
void AComInitCloseInstance( ACOM_INSTANCE * psInst );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void AComInitCloseWorkingSet ( ACOM_WORKING_SET * psWks, DWORD dwTimeout )
 * PARAMETERS: ACOM_WORKING_SET * psWks : Structure de working set
 *             DWORD dwTimeout          : Délai imparti à la fermeture
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la fermeture d'un working set ouvert ou ouvert
 *             partiellement ainsi que les instances qu'il contient.
 *             Si au bout du temp imparti, les thread workers ne sont
 *             pas arrétés, ceux-ci sont tués.
 * --------------------------------------------------------------------
 */
PROTECTED void AComInitCloseWorkingSet( ACOM_WORKING_SET * psWks, DWORD dwTimeout );


#endif


/* -------------  FIN DU FICHIER : acom_init.h ------------- */ 
