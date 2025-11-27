/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HORODATE.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: HORODATE est chargee de gerer les demandes de reveil et
*              les conversions de date et d'heure sous differents formats.
* --------------------------------------------------------------------
* HISTORIQUE:
 * Rev 1.1   02 Oct 1998 11:09  bph
 * Rev 1.18  03 Jun 1998 10:40  DPI
 * 1.0.6     25 Jan 2011 16:48  POG Multithread protection
* --------------------------------------------------------------------
* $F_HEAD


/* #include Borland C */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* #include modulotheque */
#include <noyau.h>

#include <horodate.h>

#define LOC_DEF
#include <hrd_serv.h>
#undef LOC_DEF

#include <memclass.h>

 
/*--------------- VARIABLES:---------------*/

//PRIVATE char  *what = DEFINIR_WHAT_VERSION("HORODATE", HRD_VERSION, HRD_COMMENT);

/*---------------- PUBLIC FUNCTIONS ----------------------------*/

 
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: short int HRDLance(struct_hrd_config config);
* PARAMETRES: void
* RETOUR: Compte-rendu de lancement.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Lance le module HORODATE.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_hrd_compte_rendu HRDLance(struct_hrd_config config, noyau_bal_id *bl_hrd)
{
   short int cr_lance;
   int i;

   DebutRegion(); // pour protéger HRD_NB_INSTANCES

   /* horodate ne peut être instancié qu'une fois */
   if (HRD_NB_INSTANCES <= 0)
   {
      HRD_NB_INSTANCES ++;
      _hrd_.main_instance = TRUE;
   }
   else
   {
      _hrd_.main_instance = FALSE;
   }
   FinRegion();

   if (_hrd_.main_instance == FALSE)
   {
      /* OpenSemaphore is a windows function. Noyau should be updated to
         include an implementation */
      _hrd_.HRD_sem = OpenSemaphore (SEMAPHORE_ALL_ACCESS , TRUE, "HRD");

      if (NULL == _hrd_.HRD_sem)
      {
         goto Error;
      }
   }
   else
   {
      if (NOYAU_OK != AlloueSemaphore (&(_hrd_.HRD_sem), 0, 1, "HRD"))
      {
         goto Error;
      }

      /*Initialisation des parametres de HRD avant le lancement*/
      _HRD_.pool = config.num_pool;
      _HRD_.priorite = config.priorite_max;

      for (i = 0; i < HRD_NB_TACHE; i++)
         _HRD_tache[i].noyau_priorite_tache = config.priorite_init_max;


      RendSemaphore(_hrd_.HRD_sem, 1);

      /*Le nombre d'alarmes que HRD sait gerer*/
      if (config.nb_alarme_max > MAX_ALARMES)
         goto Error;

      cr_lance = LanceTache(_HRD_tache);
      if (cr_lance == NOYAU_OK)
      {
         *bl_hrd = AttendBAL ("HRD_0");
      }
      else
      {
         goto Error;
      }
   }
   return HRD_LANCE_OK;

Error:
   if (HRD_NB_INSTANCES > 0 && _hrd_.main_instance == TRUE)
      HRD_NB_INSTANCES = 0;
   return HRD_LANCE_NOK;

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: short int HRDArret(void);
* PARAMETRES: void
* RETOUR: Compte-rendu d'arret.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Arrete le module HORODATE.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_hrd_compte_rendu HRDArret(void)
{
   short int cr_arret;
   
   if (_hrd_.main_instance == TRUE)
   {
      SupprimeBAL ("HRD_0");
   
      DebutRegion(); // pour protéger HRD_NB_INSTANCES
      if (HRD_NB_INSTANCES > 0)
         HRD_NB_INSTANCES --;
      FinRegion();

      cr_arret = ArretTaches(_HRD_tache);
      if (cr_arret != NOYAU_ARRET_TACHE_OK)
         goto Error;
   }

   if (_hrd_.HRD_sem != NULL)
   {
      LibereSemaphore (&(_hrd_.HRD_sem));
   }
   return HRD_ARRET_OK;

Error:
   return HRD_ARRET_NOK;
}
