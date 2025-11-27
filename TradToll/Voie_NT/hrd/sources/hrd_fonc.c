/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HRD_FONC.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Gestion des messages de type M_HRD_GESTION_ALARME.
* --------------------------------------------------------------------
* HISTORIQUE:
 * Rev 1.1  02 Oct 1998 11:09  bph
 * Rev 1.1  03 Jun 1998 10:40  DPI
 * 1.0.6     25 Jan 2011 16:48  POG Multithread protection
 *                              Non reuse of the same alarm ids
*
* --------------------------------------------------------------------
* $F_HEAD
*/

/* #include Borland C */
#include <stdio.h>

/* #include Modulotheque */
#include <noyau.h>
#include <horodate.h>

/* #include interne*/
#include <hrd_serv.h>

#include <memclass.h>


PROTECTED void DumpHrd()
{
   unsigned short i = 0;
   SYSTEMTIME sNow = {0};

   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      GetLocalTime(&sNow);

      for ( i = 0; i < MAX_ALARMES ; i++)
      {
         fprintf(_hrd_.fic_trace,
                 "HRD : %d/%d/%d %dh%dm%ds%dms : tab_alarmes[%d] : id : %d - %us since 80 - periode : %d - type : %d - dest : %d\n",
                 sNow.wYear,
                 sNow.wMonth,
                 sNow.wDay,
                 sNow.wHour,
                 sNow.wMinute,
                 sNow.wSecond,
                 sNow.wMilliseconds,
                 i,
                 _HRD_.tab_alarme[i].id,
                 _HRD_.tab_alarme[i].secondes_depuis_80,
                 _HRD_.tab_alarme[i].periode,
                 _HRD_.tab_alarme[i].type_alarme,
                 _HRD_.tab_alarme[i].bl_retour);
         fflush(_hrd_.fic_trace);
      }
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED t_hrd_num_reveil GenerateId
*                                        (struct_reservation_alarme* alarm)
* PARAMETRES: pointer to the alarm reservation structure
* RETOUR: the generated alarm identifier. 0 if error
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Generate an alarm identifier
* THREAD: This function is NOT THREAD SAFE. The caller must get exclusive
*         access to _hrd_.HRD_sem_ before a call.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED t_hrd_num_reveil GenerateId(struct_reservation_alarme* alarm)
{
   static last_id = 0;
   t_hrd_num_reveil id = 0;
   unsigned short increment;

   if (alarm == NULL)
   {
      return 0;
   }

   increment = (alarm->id - 1) / MAX_ALARMES;

   if (alarm->id == 0 ||
       increment >= MAX_ID_INCREMENT)
   {
	   id = (t_hrd_num_reveil)(alarm - _HRD_.tab_alarme + 1);
   }
   else
   {
      id = alarm->id + MAX_ALARMES;
   }
   return id;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void FindAlarm(t_hrd_num_reveil id)
* PARAMETRES: alarm id
* RETOUR: pointer to the alarm reservation structure. NULL if not found.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Find the alarm reservation structure
* THREAD: This function is NOT THREAD SAFE. The caller must get exclusive
*         access to _hrd_.HRD_sem_ before a call.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED struct_reservation_alarme* FindAlarm(t_hrd_num_reveil id)
{
   unsigned short index = 0;
   struct_reservation_alarme* alarm = NULL;

   if (id == 0)
   {
      return NULL;
   }

   index = (id - 1) % MAX_ALARMES;
   alarm = &(_HRD_.tab_alarme[index]);
   
   if (alarm->id == id)
   {
      return alarm;
   }
   else
   {
      return NULL;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: struct_reservation_alarme* AllocateAlarm(
*                                        struct_hrd_date_entree_hex date,
*                                        struct_hrd_heure_hex heure,
*                                        short int periode,
*                                        unsigned char bl_retour)
* PARAMETRES: La date, l'heure, la periode de reveil et la BAL de retour.
* RETOUR: The alarm structure in _HRD_.tab_alarme.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Memorise les informations concernant le reveil.
*       Retourne le numero d'alarme le plus petit non attribue,
*       retourne 0 s'il n'y a plus de numero disponible.
* THREAD: This function is NOT THREAD SAFE. The caller must get exclusive
*         access to _hrd_.HRD_sem_ before a call.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED struct_reservation_alarme* AllocateAlarm (struct_hrd_date_entree_hex date,
                                                    struct_hrd_heure_hex heure,
                                                    long int periode,
                                                    noyau_bal_id bl_retour,
                                                    unsigned long int secondes_depuis_80)
{
   unsigned short int i = 0;

   /*Recherche d'un numero non attribue*/
   while ( (i < MAX_ALARMES) && 
           (_HRD_.tab_alarme[i].type_alarme != ALARME_INIT))
   {
      i++;
   }

   if ( i >= MAX_ALARMES)
   {
      /*Tous les numeros d'alarme sont attribues*/
      return(NULL);
   }
   else
   {
      /*Memorisation des donnees du reveil et de la BAL de retour*/
      _HRD_.tab_alarme[i].date = date;
      _HRD_.tab_alarme[i].heure = heure;
      _HRD_.tab_alarme[i].periode = periode;
      _HRD_.tab_alarme[i].bl_retour = bl_retour;
      _HRD_.tab_alarme[i].secondes_depuis_80 = secondes_depuis_80;
      _HRD_.tab_alarme[i].id = GenerateId(&(_HRD_.tab_alarme[i]));
      
      return &(_HRD_.tab_alarme[i]);
   }
}

PUBLIC enum_hrd_reveil HRDArmeReveilPonctuel (struct_hrd_date_entree_hex date,
                                              struct_hrd_heure_hex heure,
                                              noyau_bal_id bal_reponse,
                                              t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil status = HRD_REVEIL_OK;
   unsigned long int secondes_depuis_80;
   struct_reservation_alarme* alarm = NULL;

   DumpHrd();
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Initialisation of a ponctual alarm\n");
   }

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }

      status = HRD_UNAVAILABLE;
      goto Error;
   }

   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Obtained semaphore\n");
      fflush(_hrd_.fic_trace);
   }

   /*calcul du nombre de secondes du reveil depuis 01/01/1980*/
   secondes_depuis_80 = HRDSecondesDepuis80(date, heure);

   if (secondes_depuis_80 < HRDSecondesDepuis80(_HRD_.date,_HRD_.heure))
   {
      /* l'heure est depassee, le reveil n'est pas acquitte */
      status = HRD_DATE_HEURE_DEPASSEE;
      goto Error;
   }

   /*Attribution d'un numero d'alarme */
   alarm = AllocateAlarm(date,
                         heure,
                         0,  /* periode = 0 */
                         bal_reponse,
                         secondes_depuis_80);

   if (alarm == NULL)
   {
      /*Il n'y a plus de numero d'alarme disponible */
      status = HRD_PAS_DE_REVEIL_DISPONIBLE;
      goto Error;
   }

   /*Reservation du numero d'alarme*/
   alarm->type_alarme = ALARME_PONCT;
   *num_reveil = alarm->id;

   RendSemaphore(_hrd_.HRD_sem, 1);
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Released semaphore\n");
      fflush(_hrd_.fic_trace);
   }


   return HRD_REVEIL_OK;

Error:
   *num_reveil = 0;
   if (status != HRD_UNAVAILABLE)
   {
      RendSemaphore(_hrd_.HRD_sem, 1);
         if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Released semaphore after error\n");
      fflush(_hrd_.fic_trace);
   }
   }
   return status;
}

PUBLIC enum_hrd_reveil HRDArmeReveilRegulier (struct_hrd_date_entree_hex date,
                                              struct_hrd_heure_hex heure,
                                              long int periode_en_secondes,
                                              noyau_bal_id bal_reponse,
                                              t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil status = HRD_REVEIL_OK;
   unsigned long int secondes_depuis_80;
   struct_reservation_alarme* alarm = NULL;

   DumpHrd();
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Initialisation of a regular alarm\n");
   }

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }

      status = HRD_UNAVAILABLE;
      goto Error;
   }


   /*calcul du nombre de secondes du reveil depuis 01/01/1980*/
   secondes_depuis_80 = HRDSecondesDepuis80(date, heure);

   if (secondes_depuis_80 < HRDSecondesDepuis80(_HRD_.date,_HRD_.heure))
   {
      /* l'heure est depassee, le reveil n'est pas acquitte */
      status = HRD_DATE_HEURE_DEPASSEE;
      goto Error;
   }

   /*Attribution d'un numero d'alarme */
   alarm = AllocateAlarm(date,
                         heure,
                         periode_en_secondes,
                         bal_reponse,
                         secondes_depuis_80);

   if (alarm == NULL)
   {
      /*Il n'y a plus de numero d'alarme disponible */
      status = HRD_PAS_DE_REVEIL_DISPONIBLE;
      goto Error;
   }

   /*Reservation du numero d'alarme*/
   alarm->type_alarme = ALARME_REGUL;
   *num_reveil = alarm->id;
   RendSemaphore(_hrd_.HRD_sem, 1);  

   return HRD_REVEIL_OK;
   
Error:
   *num_reveil = 0;
   if (status != HRD_UNAVAILABLE)
   {
      RendSemaphore(_hrd_.HRD_sem, 1);
   }
   return status;
}

PUBLIC enum_hrd_reveil HRDArmeReveilHoraire (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil status = HRD_REVEIL_OK;
   unsigned long int secondes_depuis_80;
   struct_hrd_heure_hex temps;
   struct_reservation_alarme* alarm = NULL;

   DumpHrd();
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Initialisation of a hourly alarm\n");
   }

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }

      status = HRD_UNAVAILABLE;
      goto Error;
   }

   
   temps.heure = _HRD_.heure.heure;
   temps.minute = 0;
   temps.seconde = 0;

   /* calcul du nombre de secondes du reveil depuis 01/01/1980
      jusqu'a HH+1:00:00 */
   secondes_depuis_80 = HRDSecondesDepuis80(_HRD_.date,temps) + 3600;

   /*Attribution d'un numero d'alarme */
   alarm = AllocateAlarm(_HRD_.date,
                         temps,
                         3600, /* une heure */
                         bal_reponse,
                         secondes_depuis_80);

   if (alarm == NULL)
   {
      /*Il n'y a plus de numero d'alarme disponible */
      status = HRD_PAS_DE_REVEIL_DISPONIBLE;
      goto Error;
   }

   /*Reservation du numero d'alarme*/
   alarm->type_alarme = ALARME_HOR;
   *num_reveil = alarm->id;
   
   RendSemaphore(_hrd_.HRD_sem, 1); 

   return HRD_REVEIL_OK;

Error:
   *num_reveil = 0;
   if (status != HRD_UNAVAILABLE)
   {
      RendSemaphore(_hrd_.HRD_sem, 1);
   }
   return status;
}

PUBLIC enum_hrd_reveil HRDArmeReveilJournalier (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil status = HRD_REVEIL_OK;
   unsigned long int secondes_depuis_80;
   struct_hrd_heure_hex temps;
   struct_reservation_alarme* alarm = NULL;

   DumpHrd();
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Initialisation of a daily alarm\n");
   }

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }

      status = HRD_UNAVAILABLE;
      goto Error;
   }

   temps.minute = 0;
   temps.seconde = 0;
   temps.heure = 0;

   /* calcul du nombre de secondes du reveil depuis 01/01/1980
      jusqu'a JJ+1 a 00:00:00 */
   secondes_depuis_80 = HRDSecondesDepuis80(_HRD_.date,temps) + 86400l;

   /*Attribution d'un numero d'alarme */
   alarm = AllocateAlarm(_HRD_.date,
                         temps,
                         86400l, /* un jour */
                         bal_reponse,
                         secondes_depuis_80);

   if (alarm == NULL)
   {
      /*Il n'y a plus de numero d'alarme disponible */
      status = HRD_PAS_DE_REVEIL_DISPONIBLE;
      goto Error;
   }

   /*Reservation du numero d'alarme*/
   alarm->type_alarme = ALARME_JOUR;
   *num_reveil = alarm->id;

   RendSemaphore(_hrd_.HRD_sem, 1);  

   return HRD_REVEIL_OK;

Error:
   *num_reveil = 0;
   if (status != HRD_UNAVAILABLE)
   {
      RendSemaphore(_hrd_.HRD_sem, 1);
   }
   return status;
}


PUBLIC enum_hrd_reveil HRDArmeReveilPonctuelFigeant (struct_hrd_date_entree_hex date,
                                                     struct_hrd_heure_hex heure,
                                                     noyau_bal_id bal_reponse,
                                                     t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil retour;

   /* si existe deja un r‚veil figeant */
   if (_HRD_.num_reveil_figeant != -1)
      return HRD_PAS_DE_REVEIL_DISPONIBLE;

   /* sinon arme un nouveau r‚veil */
   retour = HRDArmeReveilPonctuel (date, heure, bal_reponse, num_reveil);

   /* si OK alors le d‚clare comme figeant */
   if (retour == HRD_REVEIL_OK);
   {
      _HRD_.num_reveil_figeant = *num_reveil;
   }

   return retour;
}

PUBLIC enum_hrd_reveil HRDArmeReveilRegulierFigeant (struct_hrd_date_entree_hex date,
                                                     struct_hrd_heure_hex heure,
                                                     long int periode_en_secondes,
                                                     noyau_bal_id bal_reponse,
                                                     t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil retour;

   /* si existe deja un r‚veil figeant */
   if (_HRD_.num_reveil_figeant != -1)
      return HRD_PAS_DE_REVEIL_DISPONIBLE;

   /* sinon arme un nouveau r‚veil */
   retour = HRDArmeReveilRegulier (date, heure, periode_en_secondes, bal_reponse, num_reveil);

   /* si OK alors le d‚clare comme figeant */
   if (retour == HRD_REVEIL_OK);
   {
      _HRD_.num_reveil_figeant = *num_reveil;
   }

   return retour;
}

PUBLIC enum_hrd_reveil HRDArmeReveilHoraireFigeant (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil retour;

   /* si existe deja un r‚veil figeant */
   if (_HRD_.num_reveil_figeant != -1)
      return HRD_PAS_DE_REVEIL_DISPONIBLE;

   /* sinon arme un nouveau r‚veil */
   retour = HRDArmeReveilHoraire (bal_reponse, num_reveil);

   /* si OK alors le d‚clare comme figeant */
   if (retour == HRD_REVEIL_OK);
   {
      _HRD_.num_reveil_figeant = *num_reveil;
   }

   return retour;
}

PUBLIC enum_hrd_reveil HRDArmeReveilJournalierFigeant (noyau_bal_id bal_reponse, t_hrd_num_reveil * num_reveil)
{
   enum_hrd_reveil retour;

   /* si existe deja un r‚veil figeant */
   if (_HRD_.num_reveil_figeant != -1)
      return HRD_PAS_DE_REVEIL_DISPONIBLE;

   /* sinon arme un nouveau r‚veil */
   retour = HRDArmeReveilJournalier (bal_reponse, num_reveil);

   /* si OK alors le d‚clare comme figeant */
   if (retour == HRD_REVEIL_OK);
   {
      _HRD_.num_reveil_figeant = *num_reveil;
   }

   return retour;
}

PUBLIC enum_hrd_retour HRDAnnuleReveil (t_hrd_num_reveil num_reveil)
{
   enum_hrd_retour status = HRD_OK;
   struct_reservation_alarme* alarm = NULL;

   DumpHrd();
   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, "Removal of alarm %d\n", num_reveil);
   }

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }

      status = HRD_NOK;
      goto Error;
   }

   alarm = FindAlarm(num_reveil);
   
   /* si le r‚veil n'existe pas */
   if (alarm == NULL ||
       alarm->type_alarme == ALARME_INIT)
   {
      status = HRD_NOK;
      goto Error;
   }

   /*Liberation du numero d'alarme*/
   alarm->bl_retour = _HRD_.bal;
   alarm->type_alarme = ALARME_INIT;
   alarm->date.annee = 2100;
   alarm->date.mois = 1;
   alarm->date.jour = 1;
   alarm->heure.heure = 0;
   alarm->heure.minute = 0;
   alarm->heure.seconde = 0;
   alarm->periode = 0;
   alarm->secondes_depuis_80 =
              HRDSecondesDepuis80(alarm->date,
                                  alarm->heure);

   RendSemaphore(_hrd_.HRD_sem, 1);

   /* s'il s'agit du reveil figeant, on doit le lib‚rer */
   if (num_reveil == _HRD_.num_reveil_figeant)
      _HRD_.num_reveil_figeant = -1;

   return HRD_OK;
Error:
   RendSemaphore(_hrd_.HRD_sem, 1);
   return status;
}

PUBLIC enum_hrd_evolution_temps HRDSuspendEvolutionDuTemps (noyau_bal_id bal_demandeur)
{
   /* si le temps est d‚j… suspendu, ne rien faire */
   if (HRDDonneEtatDuTemps() == HRD_TEMPS_SUSPENDU)
      return HRD_TEMPS_DEJA_SUSPENDU;

   /* sinon, on arrˆte le temps */
   _HRD_.evol = HRD_TEMPS_SUSPENDU;

   /* et on memorise la tache qui a demande l'arret*/
   _HRD_.evol_bl = bal_demandeur;

   return _HRD_.evol;
}

PUBLIC enum_hrd_evolution_temps HRDReprendEvolutionDuTemps (noyau_bal_id bal_demandeur)
{
   /* L'ecoulement du temps est d‚j… actif */
   if (HRDDonneEtatDuTemps () == HRD_TEMPS_EN_EVOLUTION)
      return HRD_TEMPS_DEJA_EN_EVOLUTION;

   /* la tache qui demande la reprise du temps n'est pas celle
      qui a demandee la suspension. */
   if (_HRD_.evol_bl != bal_demandeur)
      return HRD_TEMPS_REPRISE_NOK;

   /*la tache qui demande la reprise du temps est celle qui a demande la
     suspension. On acquitte la demande et on reprend l'evolution*/
   _HRD_.evol = HRD_TEMPS_EN_EVOLUTION;
   _HRD_.evol_bl = -1;

   return _HRD_.evol;
}

PUBLIC enum_hrd_evolution_temps HRDDonneEtatDuTemps (void)
{
   return _HRD_.evol;
}

/*

typedef enum
{
   HRD_TEMPS_SUSPENSION_NOK,
   HRD_TEMPS_SUSPENDU,
   HRD_TEMPS_DEJA_SUSPENDU,
   HRD_TEMPS_EN_EVOLUTION,
   HRD_TEMPS_DEJA_EN_EVOLUTION,
   HRD_TEMPS_ENCORE_SUSPENDU
}
enum_hrd_evolution_temps;

PUBLIC enum_hrd_evolution_temps HRDSuspendEvolutionDuTemps (void)
{
   if (_HRD_.cpt_evol == 0xFFFF)
      return HRD_TEMPS_SUSPENSION_NOK;

   if (_HRD_.cpt_evol++ > 0)
      return HRD_TEMPS_DEJA_SUSPENDU;

   return HRD_TEMPS_SUSPENDU;
}

PUBLIC enum_hrd_evolution_temps HRDReprendEvolutionDuTemps (void)
{
   if (_HRD_.cpt_evol <= 0)
      return HRD_TEMPS_DEJA_EN_EVOLUTION;

   if (_HRD_.cpt_evol-- == 1)
      return HRD_TEMPS_EN_EVOLUTION;

   return HRD_TEMPS_ENCORE_SUSPENDU;
}

PUBLIC enum_hrd_evolution_temps HRDDonneEtatDuTemps (void)
{
   if (_HRD_.cpt_evol <= 0)
      return HRD_TEMPS_EN_EVOLUTION;

   return HRD_TEMPS_SUSPENDU;
}
*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC short int HRDDebutTrace (FILE *fic);
* PARAMETRES: - identificateur du fichier trace
* RETOUR:
*             - HRD_TRACE_INACTIVE si mode trace non Ok
*             - HRD_TRACE_ACTIVE si mode trace Ok
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui positionne le flag "_hrd_.flag_trace_bal"
*       … ACTIF pour signaler le debut du mode trace
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_hrd_type_trace HRDDebutTrace (FILE *fic)
{
  SYSTEMTIME date;
  enum_hrd_type_trace retour;

    /*** Forcer le retour … mode trace non actif ***/
    retour = HRD_NO_TRACE;

    /*** R‚cup‚ration de l'adresse du pointeur de fichier ***/
    _hrd_.fic_trace = fic;

    /*** Si l'ouverture s'est bien pass‚e ***/
    if ( _hrd_.fic_trace != NULL )
    {

        /* Si le mode trace n'est pas deja actif */
        if (_hrd_.flag_trace != HRD_TRACE_ACTIVE)
        {
            /*** Passage … ACTIF du flag de redirection ***/
            _hrd_.flag_trace = HRD_TRACE_ACTIVE;
        }

        /*** Positionner le retour … mode trace actif ***/
        retour = HRD_TRACE;

        /*** R‚cup‚ration de la date et de l'_HRD_.heure */
        GetLocalTime ( &date);        

        /*** Redirection dans le fichier trace ***/
        fprintf (_hrd_.fic_trace,
               "\nDebut de l'espionnage HRD le %2d/%2d/%4u a %02d-%02d-%02d\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond);
    }

    
    /*** Indiquer si le mode trace est actif ***/
    return (retour);
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PUBLIC short int HRDFinTrace (void);
* PARAMETRES:
* RETOUR: 
*             - HRD_TRACE_INACTIVE si mode trace n'est plus actif
*             - HRD_TRACE_ACTIVE si mode trace toujours actif
* --------------------------------------------------------------------
* VARIABLES: 
* --------------------------------------------------------------------
* TYPE: fonction globale
* ROLE: fonction qui ouvre le fichier trace et positionne le flag "_hrd_.flag_trace_bal"
*       … ACTIF
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC enum_hrd_type_trace HRDFinTrace (void)
{
  SYSTEMTIME date;
  enum_hrd_type_trace retour;

    /*** Forcer le retour … mode trace non actif ***/
    retour = HRD_TRACE;

    /*** Si l'ouverture s'est bien pass‚e ***/
    if ( _hrd_.flag_trace == HRD_TRACE_ACTIVE )
    {
        /*** Passage … ACTIF du flag de redirection ***/
        _hrd_.flag_trace = HRD_TRACE_INACTIVE;

        /*** Positionner le retour … mode trace actif ***/
        retour = HRD_NO_TRACE;

        /*** R‚cup‚ration de la date et de l'_HRD_.heure */
        GetLocalTime ( &date);        

        /*** Redirection dans le fichier trace ***/
        DebutRegionLocale();	// AHA 16-3-2011
        fprintf (_hrd_.fic_trace,
               "\nFin de l'espionnage HRD le %2d/%2d/%4u a %02d-%02d-%02d\n",
               date.wDay, date.wMonth, date.wYear,
               date.wHour, date.wMinute, date.wSecond);
        FinRegionLocale();	// AHA 16-3-2011
    }

    /*** Indiquer si le mode trace est actif ***/
    return (retour);
}

