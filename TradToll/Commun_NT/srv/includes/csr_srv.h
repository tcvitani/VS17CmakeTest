/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Generic circular double linked list component
* FICHIER: csr_list.h
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef CSR_SRV_H
#define CSR_SRV_H

/*--------------- INCLUDES: ---------------*/
#include <windows.h>

/*--------------- RESERVED: ---------------*/

#ifdef SERVICE_EXPORTS
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- TYPEDEFS: ---------------*/

typedef DWORD srv_id_demandeur;

typedef BOOL (WINAPI * srv_func_action)(PVOID Param, srv_id_demandeur IdDemandeur, PVOID DataDuDemandeur);
typedef BOOL (WINAPI * srv_func_recherche)(PVOID Param, PVOID DataDuDemandeur);

#define SRV_ILLIMITE        ((DWORD) -1)

/*--------------- FUNCTIONS: ---------------*/
 
// NbJetonsMax :    -1 -> infini
EXPORT HANDLE WINAPI SrvLance (IN DWORD NbJetonsMax);

EXPORT BOOL WINAPI SrvArret (IN OUT HANDLE *phSrv);

// Data : valeur void utilisé par l'appelant comme bon lui semble
// Renvoie un pointeur sur une zone mémoire (jeton) de taille SizeOfData
EXPORT PVOID WINAPI SrvAjouteDemandeur (IN HANDLE hSrv, IN srv_id_demandeur IdDemandeur, IN DWORD SizeOfData);

EXPORT BOOL WINAPI SrvEnleveDemandeur (IN HANDLE hSrv, IN OUT PVOID *DataDuDemandeur);

EXPORT BOOL WINAPI SrvEstDemandeur (IN HANDLE hSrv, IN srv_id_demandeur IdDemandeur);

// Renvoie NULL si la recherche echoue
EXPORT PVOID WINAPI SrvRechercheDemandeur (IN HANDLE hSrv, IN srv_id_demandeur IdDemandeur, IN srv_func_recherche fRecherche, IN OUT PVOID Param);

// Appel une fonction pour chaque demandeur du service
// Si le retour est FALSE, le parcour s'arrete
EXPORT BOOL WINAPI SrvPourChaqueDemandeur (IN HANDLE hSrv, IN srv_func_action fAction, IN OUT PVOID Param);

EXPORT BOOL WINAPI SrvEstVide(IN HANDLE hSrv);

EXPORT BOOL WINAPI SrvEstDisponible(IN HANDLE hSrv);

EXPORT DWORD WINAPI SrvCompteDemandeurs(HANDLE hSrv);

#endif CSR_SRV_H