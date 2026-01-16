/* --------------------------------------------------------------------
* (C) 1999 CS Route - All rights reserved
* --------------------------------------------------------------------
* MODULE     :  
* FILE       : GEN.H
* LANGAGE    : C
* --------------------------------------------------------------------
* KEYWORDS   : LS
* --------------------------------------------------------------------
* SUMMARY    : Module contenant les structures communes 
*              à tous les modules du projet
* --------------------------------------------------------------------
* DESCRIPTION: 
* --------------------------------------------------------------------
* HISTORY    : 
*
*   * $Log : $
*
* --------------------------------------------------------------------
*/

/*** VALEURS DE CONSTANTES SUPPOSEES !!!!! */
#define MAX_CAR 1024
#define MAX_INFO  50
#define MAX_MATRICULE 10
#define MAX_PROFILE_ID 10
#define MAX_ENR 200
#define MAX_ENR_RESUL 500
#define MAX_ID_PLAZA_NUM 10  
#define MAX_ID_LANE 10  
#define MAX_ID_LANE_GROUP 10  
#define MAX_ID_MENU 10  
#define MAX_ID_COMMAND 10  
#define MAX_RTO_ZONE 10  
#define MAX_ALARM_TYPE 10
#define MAX_ALARM_SUB_TYPE 10
#define MAX_PAYMENT_TYPE 10

static DWORD Index = 0;
static DWORD Taille = 0;


 struct tab {
	 char info_0[MAX_INFO];
	 char info_1[MAX_INFO];
	 char info_2[MAX_INFO];
 };
 typedef struct tab st_TabInfo;

struct resul {
	 char info_0[MAX_INFO];
	 char info_1[MAX_INFO];
	 char info_2[MAX_INFO];
	 char info_3[MAX_INFO];
	 char info_4[MAX_INFO];};
 typedef struct resul st_resul;

 static st_resul Resultat[MAX_ENR] = {0};

