/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur [AFF] (ou Afficheur/Clavier)
* FICHIER: AFF.H
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
*
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef AFF_H
#define AFF_H

/*--------------- INCLUDES: ---------------*/

#include <windows.h>
#include <csrlc32.h>
#include <run.h>
#include <module.h>

/* Macros sur types declaratifs */ 
#ifdef AFF_DEF
#	include <public.h>
#else
#	include <export.h>
#endif



/*--------------- DEFINES: ---------------*/

// Definition de la longueur max. des messages pour le dop
#define AFF_MAX_DONNEES_AFFICHAGE 300
#define AFF_MAX_LABEL 40
// <-- NBL
#define AFF_MAX_LIGNES 5
// NBL -->

/* Afficheur Allum, / ,teind */
#define AFF_ON   TRUE
#define AFF_OFF  (!AFF_ON)

#	define	AFF_REG_KEYn_ModAFF		"AFF\\"


/*--------------- TYPEDEFS: ---------------*/

/* Liste des  	 geres par le service AFFICHAGE */
typedef enum
{
	AFF_LED_F1,
	AFF_LED_F2,
	AFF_LED_F3,
	AFF_LED_F4,
	AFF_LED_CROSS_ARROW, /* F5 */
	AFF_LED_MNT,         /* F6 */
	AFF_LED_TEST,        /* F7 */
	AFF_LED_ALARM        /* F8 */
}
enum_aff_voyant;


/* Etats des voyants geres par le service AFFICHAGE */
typedef enum
{
	AFF_LED_ON,
	AFF_LED_OFF
}
enum_aff_cmd_voyant;


/* Liste des services disponibles pour l'afficheur exterieur */
typedef enum
{
	M_AFF_AFFICHAGE = M_SRV_USER,
	M_AFF_VISU,

	BUTEE_AFF_SERVICE,
	DERNIER_AFF_SERVICE = BUTEE_AFF_SERVICE - 1,
	NB_AFF_SERVICE = BUTEE_AFF_SERVICE
}
enum_aff_service;


/* liste des types de message de services */
typedef enum
{
	AFF_TYP_ALLUME = SRV_TYP_USER,
	AFF_TYP_ALLUME_ACQ,
	AFF_TYP_ALLUME_NACQ,

	AFF_TYP_VOYANT,
	AFF_TYP_VOYANT_ACQ,
	AFF_TYP_VOYANT_NACQ,

	AFF_TYP_RESET,
	AFF_TYP_RESET_ACQ,
	AFF_TYP_RESET_NACQ,

	AFF_TYP_AFFICHAGE_COURANT,

	AFF_TYP_TOUCHE,

   /******* MSU**********/
	AFF_TYP_NEW_FILE,
	AFF_TYP_NEW_FILE_ACQ,
	AFF_TYP_NEW_FILE_NACQ,
   /*********************/

   AFF_TYP_RESET_HW,
   AFF_TYP_RESET_HW_ACQ,
   AFF_TYP_RESET_HW_NACQ
}
enum_aff_type;


/*******************************************/
/* STRUCTURE DES MESSAGES DU SERVICE ETAT  */
/*******************************************/

/* Definition des status en champs de bits */
typedef struct
{
	unsigned int  aff_hs     : 1; // AFF hs
	unsigned int  liaison_hs : 1; // Liaison AFF hs
	unsigned int  specifique : 6; // non detaille car depend du hard utilise
}
struct_b_etat_aff;

/* Definition du message proprement dit */
typedef struct
{
	union
	{
		struct_srv_etat		srv_etat;
		struct_b_etat_aff	b_status;
	} u;
} struct_aff_etat;


/***********************************************/
/* STRUCTURE DES MESSAGES DU SERVICE AFFICHAGE */
/***********************************************/

typedef struct
{
    boolean bString;    // VOIE NT : si TRUE, utiliser szData, sinon utiliser lData
    union                 
    {
        char szData[AFF_MAX_DONNEES_AFFICHAGE];
        unsigned long lData;
    }
    my_data;   // TO DO : remplace my_data par data
}
struct_aff_donnee_affichage;

#   define AFF_LIRE_DONNEE_AS_LONG(x) ( x.bString ? atol(x.my_data.szData) : x.my_data.lData )
#   define AFF_CHAMP_DONNEE_LONG(x) ( x.my_data.lData )
#   define AFF_CHAMP_DONNEE_STRING(x) ( x.my_data.szData )
#   define AFF_ECRIRE_DONNEE_LONG(x,l) ( x.my_data.lData = l , x.bString = FALSE )
#   define AFF_ECRIRE_DONNEE_STRING(x,str,pool) \
        if ( strlen(str) > sizeof(x.my_data.szData) ) ExitBad();\
		else ( strcpy_s( x.my_data.szData,sizeof(x.my_data.szData), str ) , x.bString = TRUE )


/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/
/*      Ne pas changer l'ordre de ces structures    */
/*  Il correspond a l'ordre des mots cles pour      */
/*  les variables a afficher (cf sgm_ani.h )        */
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/

typedef struct
{
   struct_aff_donnee_affichage  payment_type;
   struct_aff_donnee_affichage  veh_class;
   struct_aff_donnee_affichage  fare;
   struct_aff_donnee_affichage  paid;
   struct_aff_donnee_affichage  sale_fare;
   struct_aff_donnee_affichage  sale_paid;
   struct_aff_donnee_affichage  libelle1;
   struct_aff_donnee_affichage  libelle2;
   struct_aff_donnee_affichage  libelle3;
   struct_aff_donnee_affichage  amount_due;
   struct_aff_donnee_affichage  overpay;
} struct_aff_vehicule;

typedef struct
{
   unsigned char                label[AFF_MAX_LABEL];
   unsigned char                priorite;
   struct_aff_donnee_affichage  soft_revision;
   struct_aff_donnee_affichage  precision_tarif;
   struct_aff_vehicule          vehicule;
} struct_aff_demande;

typedef struct
{
   unsigned char   priorite;
} struct_aff_reset;

typedef struct
{
   boolean   etat;
} struct_aff_allume;

typedef struct
{
   enum_aff_voyant      type;
   enum_aff_cmd_voyant  cmd;
} struct_aff_voyant;

typedef struct
{
// <-- NBL
    // unsigned char  label[AFF_MAX_LABEL];
    unsigned char  label[AFF_MAX_LIGNES][AFF_MAX_DONNEES_AFFICHAGE];
// --> NBL
} struct_aff_visu;


   /******* MSU**********/
typedef struct
{
    unsigned char  FilePath[MAX_PATH];
} struct_aff_newfile;
   /*********************/

typedef struct
{
   struct_aff_demande  demande;
   struct_aff_reset    reset;
   struct_aff_allume   allume;
   struct_aff_voyant   voyant;
   /******* MSU**********/
	struct_aff_newfile newFile;
   /*********************/
} struct_aff_affichage;

/*********************************************/
/* STRUCTURE DES MESSAGES DU SERVICE TEST	 */
/*********************************************/
typedef struct
{
	unsigned char	priorite;
	char			ligne[AFF_MAX_DONNEES_AFFICHAGE];				//buffer a afficher
} struct_aff_test;

/******************************************************/
/* STRUCTURE DES MESSAGES ECHANGES AVEC L'APPLICATION */
/******************************************************/

typedef struct
{
   struct_srv_entete entete;
   union
   {
      struct_srv_arret				srv_arret;
      struct_srv_etat				srv_etat;
      struct_srv_espion				srv_espion;
      struct_aff_affichage			srv_affichage;
      struct_aff_visu				srv_visu;
	  struct_aff_test				srv_test;
	  struct_device_info_item_list	srv_device_info;
   } u;
} struct_aff_message;



/*--------------- FUNCTIONS: ---------------*/
#ifdef AFF_PROTOTYPES
EXPORT enum_instance_result WINAPI MODLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI AFFLance( char * pcKey, char * pcBalNam, noyau_bal_id * piBalId );
EXPORT enum_instance_result WINAPI MODArret( noyau_bal_id iBalId );
EXPORT enum_instance_result WINAPI AFFArret( noyau_bal_id iBalId );
#endif



#include <undef.h>
#endif /* AFF_F */



