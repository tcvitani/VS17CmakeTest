/*------   (v) 1997 CS-Route  ------------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: EMISSION DE FICHIER
* FICHIER: emi_back.h
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Fichier interne au module
* --------------------------------------------------------------------
* DESCRIPTION: Declarations de constantes et variables pour la tache
*              de BACKUP : restitution et purge
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Emi_Fic/Includes/emi_back.h_v  $
 * 
 *    Rev 1.0   14 Dec 1999 14:09:02   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   Jul 24 1997 14:31:08   HMO
 *  
 * 
 *    Rev 1.2   Jul 21 1997 10:29:56   HMO
 *  
 * 
 *    Rev 1.1   May 27 1997 09:31:48   ANA
 * Modifications pour la restitution
 * 
 *    Rev 1.0   May 13 1997 17:09:34   ANA
 * Gestion de la sauvegarde sur disque et de la purge
 *
* --------------------------------------------------------------------
* $F_HEAD
*/
#ifndef EMI_BACK_H
#define EMI_BACK_H

/*--------------- INCLUDES: ---------------*/

#include <protect.h>

/*--------------- RESERVED: ---------------*/

/*--------------- EXTERNALS: ---------------*/

/*--------------- DEFINES: ---------------*/

/*----------------TYPEDEF:----------------*/
typedef enum
{
   EMI_RESTIT_OK = CSR_OK,
   EMI_RESTIT_ERR_FICHIER,
   EMI_RESTIT_ERR_NUMERO,
   EMI_RESTIT_DATE_FIN,
   EMI_RESTIT_ERR_DATE

}enum_restit_erreur;

typedef enum
{
   FICHIER_DATE_ABSENT,
   FICHIER_DATE_PRESENT
}enum_back_fichier;


typedef struct
{
   boolean flag_date;
   enum_back_fichier etat;
   int nb_fichier_trouve;
   int nb_fichier_cherche;
   int iteration;
   long *ptr_courant;
   long *ptr_premier;
   long *ptr_dernier;
   SYSTEMTIME courante;
   SYSTEMTIME debut;
   SYSTEMTIME fin;
   short int action;   /* a effectuer apres la recherche */
}struct_backup;

/* -------------- FONCTIONS: --------------*/
PROTECTED void InitBackup (void);
PROTECTED void EmiGestionBackup(enum_emi_etat *p_etat_backup,
                                struct_backup *ptr_backup,
                                struct_emi_utile *p_util);

PROTECTED void EmiGestionPurge( struct_emi_gestion_etat *p_etat,
                                struct_emi_num_fichier *num_fichier);

PROTECTED void EmiRestitution(struct_emi_gestion_restit *,
			                      struct_emi_gestion_etat *p_etat,
			                      struct_emi_num_fichier *num_fichier);

PROTECTED void EmiTrouveNumero(struct_emi_gestion_restit *,
			                      struct_emi_gestion_etat *p_etat,
			                      struct_emi_num_fichier *num_fichier);

PROTECTED enum_restit_erreur DateFichierSuivant(struct_backup *ptr_backup);

PROTECTED void DatePurgeFin(int nb_jours, char *date_purge, size_t sDate_purgeSize);

PROTECTED void DatePurgeDebut(int nb_jours, char *date_purge, size_t sDate_purgeSize);

PROTECTED boolean VerifDate(short int start_year,
                            short int start_day,
                            short int start_month,
                            short int end_year,
                            short int end_day,
                            short int end_month);

PROTECTED int ComparerDatesJour( SYSTEMTIME *debut, SYSTEMTIME *fin);

PROTECTED boolean ConvertirDate(char *date_chaine, SYSTEMTIME *date);

/*------------------VARIABLES:-----------------*/


#undef PROTECTED
#undef I
#undef INIT
#endif