/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  AUTOMATE DECISIONNEL DE COMPTAGES LAN
* FICHIER: atm_lan.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/includes/Atm_lan.h_v  $
 * 
 *    Rev 1.0   Nov 22 1999 14:55:14   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#ifndef ATM_LAN_H
#define ATM_LAN_H

/*--------------- INCLUDES: ----------------*/
#include "noyau.h"
#include "csr_aut.h"

#ifdef ATM_LAN_DEF
   #include "public.h"
#else
   #include "export.h"
#endif

/*--------------- DEFINES: -----------------*/
/*--------------- TYPEDEFS: ----------------*/

typedef enum
{
    PROT_APPLICATIF,
    PROT_DEMANDE_VIE,
    PROT_REPONSE_VIE
}
enum_prot_type;

#pragma pack(push)
#pragma pack(1)
typedef struct
{
    enum_prot_type MsgType;
    LONGLONG MsgDate;
    DWORD MsgSize;
} 
struct_prot;
#pragma pack(pop)

/*--------------- FUNCTIONS: ---------------*/

PUBLIC aut_enum_retour ATM_LAN_Lance (HANDLE *atm_lan, short int ident);
PUBLIC aut_enum_retour ATM_LAN_Arret (HANDLE *atm_lan);
PUBLIC BOOL ATM_LAN_Reception_Message_BAL (HANDLE id, struct_neutre *p_neutre);
PUBLIC BOOL ATM_LAN_Reception_Message_Reseau (HANDLE id, struct_prot *p_prot);
PUBLIC void ATM_LAN_Connexion(HANDLE id);
PUBLIC void ATM_LAN_Shutdown(HANDLE id);
PUBLIC void ATM_LAN_Break(HANDLE id);
PUBLIC void ATM_LAN_Synchronisation (HANDLE id);

#endif