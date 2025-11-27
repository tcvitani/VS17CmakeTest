/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: HRD_SERV.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Gestion des services recus par HORODATE.
* --------------------------------------------------------------------
* HISTORIQUE:
 * Rev 1.1   02 Oct 1998 11:09  bph
 * Rev 1.12  03 Jun 1998 10:41  DPI
 * 1.0.6     25 Jan 2011 16:48  POG Multithread protection
 *                              Non reuse of the same alarm ids
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

/*--------------- RESERVED: ---------------*/
#include <memclass.h>


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ReceptionArret(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES: pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reception de la demande d'arret.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void  ReceptionArret(struct_hrd_message *p_struct_hrd_message)
{

   switch (p_struct_hrd_message->entete.type_message)
      {
         case (HRD_DEMANDE) :

            Demande(p_struct_hrd_message);
            break;

         default :
            if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
            {
               fprintf(_hrd_.fic_trace,"HRD : Message (du service M_HRD_ARRET) inexistant\n");
            }
            break;
      }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void ReceptionGestionAlarme(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES: pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reception des messages d'alarme.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ReceptionGestionAlarme
                                 (struct_hrd_message *p_struct_hrd_message)
{
   switch (p_struct_hrd_message->entete.type_message)
      {
            case (HRD_ARME_PONCT) :

               ArmePonct(p_struct_hrd_message);
            break;

            case (HRD_ARME_HOR) :

               ArmeHor(p_struct_hrd_message);
            break;

            case (HRD_ARME_JOUR) :

               ArmeJour(p_struct_hrd_message);
            break;

            case (HRD_ARME_REGUL) :

               ArmeRegul(p_struct_hrd_message);
            break;

            case (HRD_DESARME_PONCT) :
            case (HRD_DESARME_HOR) :
            case (HRD_DESARME_JOUR) :
            case (HRD_DESARME_REGUL) :

               Desarme(p_struct_hrd_message);
            break;

           default :
              if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
              {
                 fprintf(_hrd_.fic_trace,
                    "HRD : Message (du service M_HRD_GESTION_ALARME) inexistant\n");
              }
           break;
      }


}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED void ReceptionEvolHeure(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reception des messages de gestion de l'_HRD_.heure.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void ReceptionEvolHeure(struct_hrd_message *p_struct_hrd_message)
{
   switch (p_struct_hrd_message->entete.type_message)
      {
         case (HRD_EVOL_ARRET) :

            ArretEvol(p_struct_hrd_message->entete.neutre.bl_retour);
            break;

         case (HRD_EVOL_DEPART) :

            DepartEvol(p_struct_hrd_message->entete.neutre.bl_retour);
            break;

         default :
            if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
            {
                fprintf(_hrd_.fic_trace,
                   "HRD : Message (du service M_HRD_EVOL_HEURE) inexistant\n");
            }
            break;
      }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED void ReceptionGestionDateHeure(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES: pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reception des messages de gestion d'_HRD_.heure et de date.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED void ReceptionGestionDateHeure
                                (struct_hrd_message *p_struct_hrd_message)
{
   switch (p_struct_hrd_message->entete.type_message)
      {
         case (HRD_NOUVELLE_DATE_HEURE) :

            NouvelleDateHeure(p_struct_hrd_message);
            break;

         default :
            if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
            {
               fprintf(_hrd_.fic_trace,
                 "HRD : Message (du service M_HRD_NOUVELLE_DATE_HEURE) inexistant\n");
            }
            break;
      }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED void Reveil();
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reveil des taches en attente.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void Reveil()
{
   short int i;
   struct_hrd_gestion_alarme msg_gestion_alarme;
   unsigned long int secondes_depuis_80;
   noyau_bal_id bl_retour;

   /*Parcours du tableau contenant les informations de reveil*/
   i = 0;

   secondes_depuis_80 = HRDSecondesDepuis80(_HRD_.date, _HRD_.heure);

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }
   }

   while ( i < MAX_ALARMES)
   {
      /* si la date du reveil est atteinte ou depassee */
      if ( (_HRD_.tab_alarme[i].secondes_depuis_80 <= secondes_depuis_80 ) &&
           (_HRD_.tab_alarme[i].type_alarme != ALARME_INIT) )
      {
         DumpHrd();
         if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
         {
            fprintf(_hrd_.fic_trace, 
                    "Waking alarm %d\n",
                    _HRD_.tab_alarme[i].id);
         }

         /* Recuperation des donnees du reveil */
         msg_gestion_alarme.date = _HRD_.tab_alarme[i].date;
         msg_gestion_alarme.temps = _HRD_.tab_alarme[i].heure;
         msg_gestion_alarme.periode = _HRD_.tab_alarme[i].periode;
         msg_gestion_alarme.num_alarme = _HRD_.tab_alarme[i].id;
         bl_retour = _HRD_.tab_alarme[i].bl_retour;

         /*Reveil de la tache*/
         EnvoiAcqGestionAlarme(bl_retour,
                               M_HRD_GESTION_ALARME,
                               HRD_ALARME,
                               msg_gestion_alarme);

         if ( _HRD_.tab_alarme[i].type_alarme == ALARME_PONCT)
         {
            /*c'est une alarme ponctuelle on libere le numero d'alarme */
            _HRD_.tab_alarme[i].bl_retour = _HRD_.bal;
            _HRD_.tab_alarme[i].type_alarme = ALARME_INIT;
            _HRD_.tab_alarme[i].date.annee = 2100;
            _HRD_.tab_alarme[i].date.mois = 1;
            _HRD_.tab_alarme[i].date.jour = 1;
            _HRD_.tab_alarme[i].heure.heure = 0;
            _HRD_.tab_alarme[i].heure.minute = 0;
            _HRD_.tab_alarme[i].heure.seconde = 0;
            _HRD_.tab_alarme[i].periode = 0;
            _HRD_.tab_alarme[i].secondes_depuis_80 =
                             HRDSecondesDepuis80(_HRD_.tab_alarme[i].date,
                                              _HRD_.tab_alarme[i].heure);
         }
         else
         {
            /*On ajoute au nombre de secondes depuis 80 la periode du reveil*/
            _HRD_.tab_alarme[i].secondes_depuis_80 =
                      _HRD_.tab_alarme[i].secondes_depuis_80 +
                      _HRD_.tab_alarme[i].periode;
         }
      }
      i++;
   }

   RendSemaphore(_hrd_.HRD_sem, 1);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:PROTECTED void Reveil_Figeant();
* PARAMETRES: aucun
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Reveil des taches en attente.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void Reveil_Figeant()
{
   struct_hrd_gestion_alarme msg_gestion_alarme;
   unsigned long int secondes_depuis_80;
   noyau_bal_id bl_retour;
   struct_reservation_alarme* alarm = NULL;

   secondes_depuis_80 = HRDSecondesDepuis80(DatePC(), HeurePC());

   /* si aucun reveil figeant n'est arme, on sort */
   if (_HRD_.num_reveil_figeant < 0)
      return;

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }
   }

   alarm = FindAlarm(_HRD_.num_reveil_figeant);

   /* si la date du reveil est atteinte ou depassee */
   if ( (alarm != NULL) &&
        (alarm->type_alarme != ALARME_INIT) &&
        (alarm->secondes_depuis_80 <= secondes_depuis_80 ) )
   {
         /* Recuperation des donnees du reveil */
         msg_gestion_alarme.date = alarm->date;
         msg_gestion_alarme.temps = alarm->heure;
         msg_gestion_alarme.periode = alarm->periode;
         msg_gestion_alarme.num_alarme = _HRD_.num_reveil_figeant;
         bl_retour = alarm->bl_retour;

         /*Reveil de la tache*/
         EnvoiAcqGestionAlarme (bl_retour,
                                M_HRD_GESTION_ALARME,
                                HRD_REVEIL_FIGEANT,
                                msg_gestion_alarme);

         /* on fige le temps */
         if (HRDSuspendEvolutionDuTemps (bl_retour) != HRD_TEMPS_SUSPENDU)
         {
            if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
            {
               fprintf(_hrd_.fic_trace,
                 "HRD : ProblŠme de suspension du temps pour le r‚veil figeant\n");
            }
         }

         if ( alarm->type_alarme == ALARME_PONCT)
         {
            /*c'est une alarme ponctuelle on libere le numero d'alarme*/
            alarm->bl_retour = _HRD_.bal;
            alarm->type_alarme = ALARME_INIT;
            alarm->date.annee = 2100;
            alarm->date.mois = 1;
            alarm->date.jour = 1;
            alarm->heure.heure = 0;
            alarm->heure.minute = 0;
            alarm->heure.seconde = 0;
            alarm->periode = 0;
            alarm->secondes_depuis_80 = HRDSecondesDepuis80(alarm->date,
                                                            alarm->heure);

            /* on libŠre le t‚moin de r‚veil figeant */
            _HRD_.num_reveil_figeant = -1;
         }
         else
         {
            /*On ajoute au nombre de secondes depuis 80 la periode du reveil*/
           alarm->secondes_depuis_80 = 
             alarm->secondes_depuis_80 + alarm->periode;
         }
   }
   RendSemaphore(_hrd_.HRD_sem, 1);
}

