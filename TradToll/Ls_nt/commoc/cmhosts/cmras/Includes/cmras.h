/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CMRAS
 * FILE       : CMRAS.H
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

#ifndef CMRAS_H
#define CMRAS_H


#ifdef LOC_DEF
#    include <public.h>
#else
#    include <export.h>
#endif

#include <cmhost.h>



//
// Nombre max de créneaux horaires définissable pour une connexion
//
#define HOST_MAX_TIMES      16




//
// Etats courants du thread de gestion de la connexion RAS.
//   HOST_STATE_DISCONNECTED : Si la connexion RAS est encore active, elle doit
//                             être rompue le plus rapidement possible.
//   HOST_STATE_CONNECTING   : La connexion RAS est en cours d'établissement.  
//   HOST_STATE_CONNECTED    : La connexion RAS est établie.  
//
#define HOST_STATE_DISCONNECTED   0
#define HOST_STATE_CONNECTING     1
#define HOST_STATE_CONNECTED      3




//
// Retourne NULL pour une chaine vide, et la chaine si celle-ci n'est pas vide
//
#define NULL_IF_EMPTY(x)    ( (x)[0] == 0 ? NULL : (x) )




//
// Test si une heure est valide
//
#define VALID_TIME(h,m,s)   ( ((h)>=0) && ((h)<=23) && ((m)>=0) && ((m)<=59) && ((s)>=0) && ((s)<=59) )



//
// Calcule une heure en secondes depuis minuit
//
#define TIME_SEC(h,m,s)     ( ( ((DWORD)(h)) * (DWORD)60 + ((DWORD)(m)) ) * (DWORD)60 + ((DWORD)(s)) )




//
// Définition d'un créneau horaire
//
typedef struct HOST_TIME
{
    DWORD dwSecStart;
    DWORD dwSecStop;
}
    HOST_TIME;




//
// Définition de données d'instance d'une connexion
//
typedef struct HOST_INST
{
    // Handle du thread de gestion de la connexion
    HANDLE          hThread;

    // Handle de l'événement signalant au thread de connexion qu'il doit s'arréter
    HANDLE          hEndEvt;

    // Handle de la trace
    TRC_EMETTEUR    hTrc;

    // Indique que les appels RAS sont simulés
    BOOL            bSimulated;

    // Indique le comportement à suivre si la fin d'un créneau arrive alors
    // qu'un transfert est en cours. Si la valeur est FALSE, la connexion RAS
    // ne sera fermée que lorsqu'aucun transfert ne sera en cours. Si non, la
    // connexion RAS est fermée immédiatement.
    BOOL            bAbortTransfer;

    // Indique que tout nouveau transfer ne peut avoir lieu
    BOOL            bCancelNewTransfer;

    // Periodes de scrutation
    DWORD           dwPollingPeriodNormal;
    DWORD           dwPollingPeriodShort;
    DWORD           dwPollingPeriodLong;

    // Nombre de transferts en cours. Cette valeur est incrémenté à chaque
    // début de transfert et décrémenté à la fin.
    DWORD           dwTransfers;

    // Nombre d'éléments définis dans la table des crénaux horaires
    DWORD           dwTimes;

    // Table des crénaux horaires
    HOST_TIME       tsTimes[HOST_MAX_TIMES];

    // Etat courant de la connexion
    // Lorsqu'il vaut HOST_STATE_DISCONNECTED, la connexion est rompu ou doit l'être
    // (le handle de connexion peut être NULL ou non NULL).
    // Lorsqu'il vaut HOST_STATE_CONNECTED, la connexion est active (le handle de
    // connexion est non NULL).
    // Lorsqu'il vaut HOST_STATE_CONNECTING, la connexion est en cours d'activation
    // (le handle de connexion est non NULL). Les flags bRasConnected et
    // bRasDisconnected vont déterminer l'évolution de cet état.
    DWORD           dwState;

    // Etat du lien RAS mis à jour par le callback RAS. Permet de faire passer
    // l'état de la connexion de HOST_STATE_CONNECTING à HOST_STATE_CONNECTED
    // ou de HOST_STATE_CONNECTING à HOST_STATE_DISCONNECTED.
    // On passe par ces flag à cause du contexte dans lequel se situe le callback
    // RAS. En effet, le callback est exécuté dans le contexte du thread de
    // gestion de la connexion. Il n'est donc pas possible de proteger les
    // données de la structure à l'aide de la section critique. Ces flag sont
    // le seul moyen d'échanger des données entre le callback et le reste du thread
    // de gestion de la connexion.
    BOOL            bRasConnected, bRasDisconnected;
    
    // Handle de la connexion RAS courante.
    HRASCONN        hRAS;

    // Fichier répertoire téléphonique à utiliser.
    char            szPhoneBook[MAX_PATH];

    // Nom du host
    char            szName[MAX_PATH];

    // Paramètres d'appel
    RASDIALPARAMS   sDial;

    // Section critique de protection contre les accés simultanés.
    CRITICAL_SECTION sProtect;
}
    HOST_INST;


#endif
