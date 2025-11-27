/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HRD_DATE.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Donne la date courante et l'heure courante.
*              Diverses operations sur les dates et heures :
*                                   -Addition
*                                   -Soustraction
*                                   -Comparaison
* --------------------------------------------------------------------
* HISTORIQUE:
 * Rev 1.2   May 04 2000 10:15  bph
 * Rev 1.1   02 Oct 1998 11:09  bph
 * Rev 1.14  03 Jun 1998 10:40  DPI
 * 1.0.6     25 Jan 2011 16:48  POG Multithread protection
 *                              Non reuse of the same alarm ids
* --------------------------------------------------------------------
* $F_HEAD
*/

/* #include Borland C */
#include <stdio.h>

/* #include Modulotheque */
//#include <stdcsee.h>
#include <noyau.h>
#include <horodate.h>

/* #include interne*/
#include <hrd_serv.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void MAJReveils()
* PARAMETRES: void
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Met a jour les reveils apres un changement d'heure ou une
*       reprise d'evolution du temps.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void MAJReveils(void)
{
   t_hrd_num_reveil i;
   unsigned long int secondes_depuis_80 , nb_periode;
   struct_hrd_heure_hex temps;

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }
   }

   for (i = 0; i < MAX_ALARMES; i++)
   {
      switch (_HRD_.tab_alarme[i].type_alarme)
      {
         case (ALARME_INIT) :
         case (ALARME_PONCT) :
             /*Pas de traitement particulier*/
         break;

         case (ALARME_JOUR) :

            temps.minute = 0;
            temps.seconde = 0;
            temps.heure = 0;

           /*calcul du nombre de secondes du reveil depuis 01/01/1980*/
           secondes_depuis_80 =
                     HRDSecondesDepuis80(DatePC(),temps)
                     + 86400l;  /*nombre de secondes jusqu'a JJ+1 a 00:00:00*/

           /*MAJ du nombre de secondes pour le prochain reveil*/
           _HRD_.tab_alarme[i].secondes_depuis_80 = secondes_depuis_80;

         break;

         case (ALARME_HOR) :

            temps.heure = HeurePC().heure;
            temps.minute = 0;
            temps.seconde = 0;

            /*calcul du nombre de secondes du reveil depuis 01/01/1980*/
            secondes_depuis_80 =
               HRDSecondesDepuis80(DatePC(),temps)
               + 3600;  /*nombre de secondes jusqu'a HH+1:00:00*/

           /*MAJ du nombre de secondes pour le prochain reveil*/
           _HRD_.tab_alarme[i].secondes_depuis_80 = secondes_depuis_80;

         break;

         case (ALARME_REGUL) :

            temps.heure = _HRD_.tab_alarme[i].heure.heure;
            temps.minute = _HRD_.tab_alarme[i].heure.minute;
            temps.seconde = _HRD_.tab_alarme[i].heure.seconde;

           /*Calcul des secondes de 80 a maintenant*/
           secondes_depuis_80 = HRDSecondesDepuis80(DatePC(),HeurePC());

           /*calcul du nombre de secondes du reveil depuis 01/01/1980*/
           if (_HRD_.tab_alarme[i].secondes_depuis_80 < secondes_depuis_80 )
           {
            /*L'heure du reveil est depassee on calcul le nombre de periodes
            comprises entre date_heure avant changement
            et date_heure apres changement*/

            nb_periode = (secondes_depuis_80 - _HRD_.tab_alarme[i].secondes_depuis_80 )
                         / _HRD_.tab_alarme[i].periode;

             nb_periode = nb_periode + 1;

             secondes_depuis_80 = _HRD_.tab_alarme[i].secondes_depuis_80
                                + ( _HRD_.tab_alarme[i].periode * nb_periode);

            }
            else
            {
            /*on calcul le nombre de periodes comprises entre date_heure avant
            changement et date_heure apres changement*/
             nb_periode = (_HRD_.tab_alarme[i].secondes_depuis_80 - secondes_depuis_80 )
                         / _HRD_.tab_alarme[i].periode;

             secondes_depuis_80 = _HRD_.tab_alarme[i].secondes_depuis_80
                                - ( _HRD_.tab_alarme[i].periode * nb_periode);
            }
            _HRD_.tab_alarme[i].secondes_depuis_80 = secondes_depuis_80;


         break;

         default :
            if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
            {
             fprintf(_hrd_.fic_trace,"HRD : Pb lors de la mise a jour des reveils\n");
            }
         break;
      }
   }

   RendSemaphore(_hrd_.HRD_sem, 1);
}

PUBLIC enum_hrd_retour HRDChangeDate (struct_hrd_date_entree_hex nouvelle_date)
{
  SYSTEMTIME date;
  SYSTEMTIME date_avant_changement;

  GetLocalTime(&date_avant_changement);


  GetLocalTime(&date);
  date.wYear = nouvelle_date.annee;
  date.wMonth = nouvelle_date.mois;
  date.wDay = nouvelle_date.jour;
  /*date.dayofweek= 0;*/

  if  (SetLocalTime(&date))
  {
     /*Le changement de date du PC s'est bien effectue */
     if (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION)
     {
        /*l'evolution du temps n'est pas suspendue*/
        _HRD_.date = DatePC();
     }

     /*MAJ des reveils*/
     MAJReveils();

     return HRD_OK;
  }

  /*Le changement de date du PC s'est mal effectue,
    on n'acquitte pas le changement.*/

  /*La date du PC reprend son ancienne valeur*/
  if (!SetLocalTime(&date_avant_changement))
  {
     /*Il est impossible de revenir a la date precedente*/
     if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
     {
        fprintf(_hrd_.fic_trace,"HRD : Pb lors du changement de date du PC\n");
     }
  }

  return HRD_NOK;
}

PUBLIC enum_hrd_retour HRDChangeHeure (struct_hrd_heure_hex nouvelle_heure)
{
  SYSTEMTIME heure;
  SYSTEMTIME heure_avant_changement;

  GetLocalTime (&heure_avant_changement);

  GetLocalTime (&heure);
  heure.wHour = nouvelle_heure.heure ;
  heure.wMinute = nouvelle_heure.minute;
  heure.wSecond = nouvelle_heure.seconde;
  heure.wMilliseconds = 50;

  /* ATTENTION, si on indique hsecond = 0, alors
   * la fonction _dos_settime retire 1 seconde … l'heure demand‚e
   */

  if  (SetLocalTime(&heure))
  {
     /*Le changement de l'heure du PC s'est bien effectue */
     if (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION)
     {
        /*l'evolution du temps n'est pas suspendue*/
        _HRD_.heure = HeurePC();
     }

     /*MAJ des reveils*/
     MAJReveils();

     return HRD_OK;
  }

  /*Le changement d'heure du PC s'est mal effectue,
    on n'acquitte pas le changement.*/

  /*La date du PC reprend son ancienne valeur*/
  if (!SetLocalTime(&heure_avant_changement))
  {
     /*Il est impossible de revenir a la date precedente*/
     if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
     {
        fprintf(_hrd_.fic_trace,"HRD : Pb lors du changement de d'heure du PC\n");
     }
  }

  return HRD_NOK;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: struct_hrd_date_entree_hex HRDDonneDate()
* PARAMETRES:Void.
* RETOUR: La date courante de horodate.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retourne la date de horodate (prend en compte les arrets de temps).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC struct_hrd_date_entree_hex HRDDonneDate()
{
   if  ( (_HRD_.etat_init == 0) || (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION) )
   {
      /*HRD n'est pas encore initialis‚e ou bien n'est pas arretee,
       on retourne la date systeme*/
      return ( DatePC() );
   }
   return ( _HRD_.date );
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: struct_hrd_date_entree_hex DatePC()
* PARAMETRES:Void.
* RETOUR: La date du PC .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retourne la date du PC.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED struct_hrd_date_entree_hex DatePC()
{
   SYSTEMTIME date;
   struct_hrd_date_entree_hex date_dos;

   /*Recuperation de la date du PC*/
   GetLocalTime(&date);
   date_dos.annee = (unsigned short) date.wYear;
   date_dos.mois = (unsigned char) date.wMonth;
   date_dos.jour = (unsigned char) date.wDay;

   return(date_dos);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  HRDDonneJulien()
* PARAMETRES:Void.
* RETOUR: La date courante au format jour julien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retourne la date de horodate (prend en compte les arrets de temps).
*       au format jour julien
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC unsigned short HRDDonneJulien()
{
   struct_hrd_date_entree_hex date;

   date = HRDDonneDate();

   return( HRDConversionDateJulien( &date));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: struct_hrd_heure_hex HRDDonneHeure()
* PARAMETRES:Void.
* RETOUR: L'heure courante dans horodate .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retourne l'heure de horodate (prend en compte les arrets de temps).
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC struct_hrd_heure_hex HRDDonneHeure()
{
   if ( ( _HRD_.etat_init == 0) || (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION) )   {
      /*HRD n'est pas encore initialis‚e ou bien n'est pas arretee,
      on retourne l'heure systeme*/
      return ( HeurePC() );
   }
   return ( _HRD_.heure );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: struct_hrd_heure_hex HeurePC()
* PARAMETRES:Void.
* RETOUR: L'heure du PC .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retourne l'heure du PC.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED struct_hrd_heure_hex HeurePC()
{
   SYSTEMTIME temps;
   struct_hrd_heure_hex heure_dos;

   /*Recuperation de l'heure du PC*/
   GetLocalTime(&temps);
   heure_dos.heure = (unsigned char) temps.wHour;
   heure_dos.minute = (unsigned char) temps.wMinute;
   heure_dos.seconde = (unsigned char) temps.wSecond;

   return (heure_dos);
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: char HRDCompare DatesHeures
*                                         (struct_hrd_date_entree_hex date1,
*                                          struct_hrd_heure_hex heure1,
*                                          struct_hrd_date_entree_hex date2,
*                                          struct_hrd_heure_hex heure2)
* PARAMETRES:Void.
* RETOUR: Retourne -1 si date_heure1 < date_heure2
*                   0 si date_heure1 = date_heure2
*                   1 si date_heure1 > date_heure2.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Compare deux dates.Retourne -1 si date_heure1 < date_heure2
*                                    0 si date_heure1 = date_heure2
*                                    1 si date_heure1 > date_heure2.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC char HRDCompareDatesHeures ( struct_hrd_date_entree_hex date1,
                             struct_hrd_heure_hex heure1,
                             struct_hrd_date_entree_hex date2,
                             struct_hrd_heure_hex heure2)
{
   char resultat;

   if ( date1.annee < date2.annee)
   {
      resultat =  -1;
   }

   else if ( date1.annee > date2.annee)
   {
      resultat =  1;
   }

   /*Les annees sont identiques*/

   else if ( date1.mois < date2.mois)
   {
      resultat =  -1;
   }

   else if ( date1.mois > date2.mois)
   {
      resultat =  1;
   }

   /*Les annees et les mois sont identiques*/

   else if ( date1.jour < date2.jour)
   {
      resultat =  -1;
   }

   else if ( date1.jour > date2.jour)
   {
      resultat =  1;
   }

   /*Les annees,les mois et les jours sont identiques*/

   else if ( heure1.heure < heure2.heure)
   {
      resultat =  -1;
   }

   else if ( heure1.heure > heure2.heure)
   {
      resultat =  1;
   }

   /*Les annees,les mois,les jours et les heures sont identiques*/

   else if ( heure1.minute < heure2.minute)
   {
      resultat =  -1;
   }

   else if ( heure1.minute > heure2.minute)
   {
      resultat =  1;
   }
   /*Les annees,les mois,les jours,les heures et les minutes sont identiques*/

   else if ( heure1.seconde < heure2.seconde)
   {
      resultat =  -1;
   }

   else if ( heure1.seconde > heure2.seconde)
   {
      resultat =  1;
   }
   /*Les dates et les heures sont identiques*/
   else resultat =  0;

   return resultat;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void HRDAjouteDateHeure (struct_hrd_date_entree_hex date_entree,
*                         struct_hrd_heure_hex heure_entree,
*                         unsigned long int nb_secondes,
*                         struct_hrd_date_entree_hex * date_sortie,
*                         struct_hrd_heure_hex * heure_sortie);
* PARAMETRES:Void.
* RETOUR:Pointeurs sur struct date et heure .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Ajoute a la date_heure donnee en entree nb_secondes.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void HRDAjouteDateHeure (struct_hrd_date_entree_hex date_entree,
                         struct_hrd_heure_hex heure_entree,
                         unsigned long int nb_secondes,
                         struct_hrd_date_entree_hex * date_sortie,
                         struct_hrd_heure_hex * heure_sortie)
{
   nb_secondes = nb_secondes + HRDSecondesDepuis80 (date_entree , heure_entree);
   HRDConvSecondesEnDateHeure(nb_secondes , date_sortie , heure_sortie);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void HRDRetrancheDateHeure (struct_hrd_date_entree_hex date_entree,
*                         struct_hrd_heure_hex heure_entree,
*                         unsigned long int nb_secondes,
*                         struct_hrd_date_entree_hex * date_sortie,
*                         struct_hrd_heure_hex * heure_sortie);
* PARAMETRES:Void.
* RETOUR:Pointeurs sur struct date et heure .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Retranche a la date_heure donnee en entree nb_secondes.
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC void HRDRetrancheDateHeure (struct_hrd_date_entree_hex date_entree,
                            struct_hrd_heure_hex heure_entree,
                            unsigned long int nb_secondes,
                            struct_hrd_date_entree_hex * date_sortie,
                            struct_hrd_heure_hex * heure_sortie)
{
   nb_secondes = HRDSecondesDepuis80 (date_entree , heure_entree) - nb_secondes;
   HRDConvSecondesEnDateHeure(nb_secondes , date_sortie , heure_sortie);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
*        unsigned char HRDDateCoherence (struct_hrd_date_entree_hex date_entree)
* PARAMETRES:Date a tester.
* RETOUR:Retourne 1 si date coherente (1980 <= annee <= 2100)
*        0 si incoherente
* --------------------------------------------------------------------
* VARIABLES: Date a tester
* --------------------------------------------------------------------
* ROLE: Test la coherence d'une date en tenant compte des annees bissextile
* --------------------------------------------------------------------
* $F_FCTN
*/


PUBLIC enum_hrd_retour HRDDateCoherence (struct_hrd_date_entree_hex date_entree)
{
   struct_hrd_date_entree_hex date_sortie;
   struct_hrd_heure_hex temps,heure_sortie;
   unsigned long int secondes;
   char res_comp;

   if ( (date_entree.annee < HRD_ANNEE_BASE) || (date_entree.annee > 2100) )
   {
      return 0;  /*L'annee est incoherente pour HRD*/
   }

   if ( (date_entree.mois < 1) || ( date_entree.mois > 12) )
   {
      return 0;  /*Le mois est incoherent*/
   }

   if ( (date_entree.jour < 1) || (date_entree.jour > 31) )
   {
      return 0;   /*Le jour est incoherent*/
   }

   /* BPH 12/4/00 : cas particulier du mois de février
      si pas année bissextile, le jour ne doit pas dépasser 28 sinon 29 */
   if (date_entree.mois == 2)
   { 
       if (bissextile(date_entree.annee) == 0)
       {
           if (date_entree.jour > 28)
           {
              return 0;   /*Le jour est incoherent*/
           }
       }
       else
       {
           if (date_entree.jour > 29)
           {
              return 0;   /*Le jour est incoherent*/
           }
       }
   }

   /*Coherence par rapport au nombre de jours dans un mois
   et aux annees bissextile*/
   temps.heure = 0;
   temps.minute = 0;
   temps.seconde = 0;
   secondes = HRDSecondesDepuis80 (date_entree , temps);
   HRDConvSecondesEnDateHeure(secondes,&date_sortie,&heure_sortie);
   res_comp = HRDCompareDatesHeures(date_entree,
                                    temps,
                                    date_sortie,
                                    heure_sortie);

   if (res_comp == 0)
   {
       /*La date est coherente*/
       return 1;
   }
   else
   {
      /*La date est incoherente*/
      return 0;
   }

}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:
*        unsigned char HRDHeureCoherence (struct_hrd_heure_hex heure_entree)
* PARAMETRES:Heure a tester.
* RETOUR:Retourne 1 si heure coherente
*        0 si incoherente
* --------------------------------------------------------------------
* VARIABLES: Heure a tester
* --------------------------------------------------------------------
* ROLE: Test la coherence d'une heure
* --------------------------------------------------------------------
* $F_FCTN
*/
/*Test la coherence d'une heure.Retourne 1 si heure coherente
                                         0 si incoherente*/

PUBLIC enum_hrd_retour HRDHeureCoherence (struct_hrd_heure_hex heure_entree)
{
   if ( heure_entree.heure > 23)
   {
      return 0; /*Heure incoherente*/
   }

   if ( heure_entree.minute > 59)
   {
      return 0;  /*minutes incoherentes*/
   }

   if ( heure_entree.seconde > 59)
   {
      return 0;  /*secondes incoherentes*/
   }

   /*L'heure est coherente*/
   return 1;
}
