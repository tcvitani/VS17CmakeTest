/* --------------------------------------------------------------------
 * (C) Sanef ITS Croatia - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : VIRT_KYB application
 * FILE       : virt_kyb.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *
 * --------------------------------------------------------------------
 */
#ifndef VIRT_KYB_H
#define VIRT_KYB_H

/*--------------- INCLUDES: ---------------*/

#include <WINDOWS.H>

#include <CSRLC32.H>
#include <run.h>
#include "module.h"   

#ifdef VIRT_KYB_DEF
#include <public.h>
#else
#include <export.h>
#endif

/*---------------------------- MACRO -------------------------*/

// ACCES AU REGISTRE

// Clé du registre contenant les configurations des instances VIRT_KYB
#define VIRT_KYB_REG_KEYn_ModVIRT_KYB     "VIRT_KYB\\"

#define VIRT_KYB_REG_KEY_BASE        CSR_REG_KEYn_CSRBASE \
                                CSR_REG_KEYn_LANE_BASE \
                                CSR_REG_KEYn_CONFIG \
                                MOD_REG_KEYn_MODULES \
                                VIRT_KYB_REG_KEYn_ModVIRT_KYB

// VALEURS DES PARAMETRES DES MESSAGES DE SERVICES

#define VIRT_KYB_LG_CHAINES_SRV 256

/*---------------------------- TYPES -------------------------*/
// Liste des services disponibles pour VIRT_KYB
typedef enum
{
    PREMIER_VIRT_KYB_SERVICE = 0,
    M_VIRT_KYB_AFFICHAGE = M_SRV_USER,

    BUTEE_VIRT_KYB_SERVICE,
    DERNIER_VIRT_KYB_SERVICE = BUTEE_VIRT_KYB_SERVICE - 1,
    NB_VIRT_KYB_SERVICE = BUTEE_VIRT_KYB_SERVICE - PREMIER_VIRT_KYB_SERVICE
}
enum_VIRT_KYB_service;

// Liste des types de messages
typedef enum
{
    PREMIER_VIRT_KYB_TYPE = 0,
    VIRT_KYB_INIT_CONFIG = M_SRV_USER,  //  Privées
	VIRT_KYB_AFF_MOVE,
	VIRT_KYB_AFF_SHOW,

    BUTEE_VIRT_KYB_TYPE,
    DERNIER_VIRT_KYB_TYPE = BUTEE_VIRT_KYB_TYPE - 1,
    NB_VIRT_KYB_TYPE = BUTEE_VIRT_KYB_TYPE - PREMIER_VIRT_KYB_TYPE
}
enum_VIRT_KYB_type;


// STRUCTURES POUR PARAMETRES SERVICES VIRT_KYB

// Service Affichage
typedef struct
{
    BYTE    keyboard_id[VIRT_KYB_LG_CHAINES_SRV];
	BOOL	bShow; //if TRUE show the item 
	DWORD   dwPosX,dwPosY;
}
struct_VIRT_KYB_srv_affichage;


// DÉFINITION DU MESSAGE INTER-TÂCHE

// structure des messages concernant le dialogue avec l'application (services)
typedef struct
{
    struct_srv_entete               entete;

    union
    {
        struct_srv_arret					srv_arret;
        struct_VIRT_KYB_srv_affichage		srv_affichage;
    } u;
}
struct_VIRT_KYB_message;


/*---------------------------- PROTOTYPES -------------------------*/

// enum_instance_result WINAPI VIRT_KYBLance ( IN  char			* pcRegKey,
// 											  IN  char			* pcBalName,
// 											  OUT noyau_bal_id	* iBalId );
// 
// enum_instance_result WINAPI VIRT_KYBArret( IN noyau_bal_id iBalId );
// 

#include <undef.h>
#endif   // VIRT_KYB_H
/*---------------------------- END OF FILE -------------------------*/
