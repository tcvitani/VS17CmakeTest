/***************************************************************************
*
*                                  CS ROUTE
* TITRE       : SIMU0.H
* AUTEUR      : DA
* CREATION    : 17/05/96
* DESCRIPTION :
*
*           Role du fichier : Fichier INCLUDE la tƒche SIMU
*
*
*
***************************************************************************/

#ifndef SIMU_H
#define SIMU_H

/*********************** DEFINITION DES CONSTANTES*************************/

#define SIMU_NB_TACHE               1
#define SIMU_PRIORITE_MAX           7

#define SIMU_C_BAL_TIMEOUT     0
#define SIMU_C_BAL_TIMEOUT2   10


/*********************** DEFINITION DES FONCTIONS *************************/
void syst_ecrire_trace (char   *p_trace);

//void simu_num_bal_defaut(void) ;

//void simu_envoi_hrd(short int, short int);
/*void simu_envoi_date_heure(short int, short int, struct_hrd_gestion_date_heure);*/
//void simu_envoi_desarme(short int, short int,unsigned short);
/*void simu_envoi_hrd_alarme(short int, short int, struct_hrd_gestion_alarme);*/
//void messages_hrd(void) ; */

short int SimuLance(short int /* priorite_max*/) ;
short int SimuArret(void) ;


DWORD WINAPI simu( LPVOID param);


#endif