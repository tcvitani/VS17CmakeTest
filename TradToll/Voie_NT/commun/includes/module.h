/* --------------------------------------------------------------------
 * --------------------------------------------------------------------
 * MODULE     : CSR
 * FILE       : module.h
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : common
 * --------------------------------------------------------------------
 * SUMMARY    : Modules Common definitions
 * --------------------------------------------------------------------
 * DESCRIPTION: This file contains common definitions for CS Route
 *              modules.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */
 
#ifndef MODULE_H
#define MODULE_H

// Standard Microsoft definitions
#include <WINDOWS.H>



// ***********************************************
// * Standard Basic Services Related Definitions *
// ***********************************************

// List of basic SERVICES
typedef enum
{
	PREMIER_SRV_SERVICE = 1,

	M_SRV_ARRET  = PREMIER_SRV_SERVICE,
	M_SRV_ESPION,
	M_SRV_ETAT,
	M_SRV_ETAT_CHAINE,
	M_SRV_DEVICE_INFO,

	BUTEE_SRV_SERVICE,
	DERNIER_SRV_SERVICE = BUTEE_SRV_SERVICE - 1,
	NB_SRV_SERVICE = BUTEE_SRV_SERVICE - PREMIER_SRV_SERVICE
}
enum_srv_service ;

// First user service available
//	If a module needs other services than the generic ones, the local enum must be define as shown below :
//		typedef enum
//		{
//			M_SRV_MySrv1 = M_SRV_USER,
//			M_SRV_MySrv2,
//			M_SRV_MySrv3,
//			...
//		}
//		enum_MOD_service ;
//	And use the generic enum_srv_service definition for later used variables
#define	M_SRV_USER	50


// List of basic service TYPES
typedef enum
{
	PREMIER_SRV_TYPE = 1,

	SRV_TYP_SERVICE_NON_GERE = PREMIER_SRV_TYPE,
	SRV_TYP_SERVICE_INCONNU,

	SRV_TYP_DEMANDE_NON_GEREE,
	SRV_TYP_DEMANDE_INCONNUE,

	SRV_TYP_ARRET_ACQ,
	SRV_TYP_ARRET_NACQ,
	SRV_TYP_ARRET_EFFECTUE,

	SRV_TYP_DEBUT,
	SRV_TYP_DEBUT_ACQ,
	SRV_TYP_DEBUT_NACQ,

	SRV_TYP_FIN,
	SRV_TYP_FIN_ACQ,
	SRV_TYP_FIN_NACQ,

	SRV_TYP_DEMANDE,
	SRV_TYP_DEMANDE_ACQ,
	SRV_TYP_DEMANDE_NACQ,
	SRV_TYP_DEMANDE_ANNULATION,

	SRV_TYP_EFFECTUE,

	SRV_TYP_NOUVEL_ETAT,

	SRV_TYP_MESSAGE_ESPION,

	SRV_TYP_SET,
	SRV_TYP_SET_ACQ,
	SRV_TYP_SET_NACQ,

	SRV_TYP_GET,
	SRV_TYP_GET_ACQ,
	SRV_TYP_GET_NACQ,

	SRV_TYP_TEST,
	SRV_TYP_TEST_ACQ,
	SRV_TYP_TEST_NACQ,

	// Fin
	BUTEE_SRV_TYPE,
	DERNIER_SRV_TYPE = BUTEE_SRV_TYPE - 1,
	NB_SRV_TYPE = BUTEE_SRV_TYPE - PREMIER_SRV_TYPE
}
enum_srv_type ;

// First user type available
//	If a module needs other types than the generic ones, the local enum must be define as shown below :
//		typedef enum
//		{
//			SRV_TYP_MyTyp1 = SRV_TYP_USER,
//			SRV_TYP_MyTyp2,
//			SRV_TYP_MyTyp3,
//			...
//		}
//		enum_MOD_type ;
//	And use the generic enum_srv_type definition for later used variables
#define	SRV_TYP_USER	100


// ESPION Service Related Definitions
// ----------------------------------

// type des messages du ESPION
typedef enum
{
    SRV_ESPION_MESSAGE_DONNEES = 1,
    SRV_ESPION_MESSAGE_SERVICE
}
enum_espion_nature ;


// sens des messages a transmettre
typedef enum
{
    SRV_ESPION_ENTRANT = 1,
    SRV_ESPION_SORTANT,
    SRV_ESPION_ENTRANT_SORTANT
}
enum_espion_sens ;
 

//structure des donnees des messages concernant le service ESPION
// Taille maximum d'un message ESPION
#define LG_TRAME_MAX              255

// Defines for device info structures
#define LG_DATA_MAX				  255
#define LG_NAME_MAX				   30
#define LG_ITEM_MAX				   15

typedef enum
{
	DEV_INFO_OK = 0,
	DEV_INFO_NOK
}
enum_device_info_status ;

typedef struct
{
     enum_espion_nature   nature;  	// donnees ou protocole
     enum_espion_sens     sens;    	// entrant et/ou sortant
     unsigned short int   taille;
     unsigned char        donnee[LG_TRAME_MAX];

	 SYSTEMTIME			  sSysDhm;	// time stamp
}
struct_srv_espion ;


// ARRET Service Related Definitions
// ---------------------------------

typedef struct
{
	unsigned char	dummy ;			// Pas de donnees utiles
}
struct_srv_arret ;


// ETAT Service Related Definitions
// ---------------------------------

typedef struct
{
    unsigned int	status;
    unsigned char	info[LG_TRAME_MAX];
}
struct_srv_etat ;


// ETAT_CHAINE Service Related Definitions
// ---------------------------------------

typedef struct
{
    unsigned char          etat_name[LG_TRAME_MAX];
    unsigned int	   etat;	
    unsigned char          info[LG_TRAME_MAX];
}
struct_srv_etat_chaine ;


// Service Messages Header Structure
// ---------------------------------

typedef struct
{
    struct_neutre       neutre;          // Partie standard noyau
    enum_srv_service    service;         // Identification du service (impression, espion, etat, arret)
    enum_srv_type       type_message;    // Identification du type de message (demande, acq, nacq etc)
}
struct_srv_entete ;


// Device Info Item Structure
// ---------------------------------
typedef struct _struct_device_info_item
{
    // Name of the item : if the name has less than LG_NAME_MAX
    // significant characters, a NULL character must be placed
    // at the end of the string. 
    unsigned char               name [ LG_NAME_MAX ] ;
		
    // Associated data block
    unsigned char               data[ LG_DATA_MAX ] ;

	// Item status
    enum_device_info_status		status ;
}
struct_device_info_item ;

// Device Info Data Structure
// ---------------------------------
// List of items
typedef struct _struct_device_info_item_list
{
    unsigned int				nb_item ;
    struct_device_info_item		items[ LG_ITEM_MAX ] ;
}
struct_device_info_item_list ;

#endif			// MODULE_H