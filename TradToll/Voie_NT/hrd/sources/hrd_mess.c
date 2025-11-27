/*------   (v) 1998 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: DLL HORODATE
* FICHIER: MESSAGE.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: Gestion des messages de type M_HRD_GESTION_ALARME.
* --------------------------------------------------------------------
* HISTORIQUE:
 * Rev 1.1   02 Oct 1998 11:09  bph
 * Rev 1.13  03 Jun 1998 10:40  DPI
 * 1.0.6     25 Jan 2011 16:48  POG Multithread protection
 *                              Non reuse of the same alarm ids
 *                              Factorisation
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

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EnvoiAcq(unsigned char bal_dest,
                         short int service_id,
                         short int type_message)
* PARAMETRES:
*            entree : boite a lettre destinatrice
*                     identificateur de service
*                     type du message a envoyer
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Envoyer un message d'acquittement ou de non acquittement
*      vers l'application pour les services M_HRD_EVOL_HEURE et M_HRD_ARRET.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EnvoiAcq(noyau_bal_id bal_dest,
                        short int service_id,
                        short int type_message)
{
   struct_hrd_message *p_message_emis = (struct_hrd_message *) (0);

   /*Allocation memoire pour le message a emettre*/
   ExitAlloue((struct_neutre **)(&p_message_emis),
                     sizeof(struct_hrd_message),
                     _HRD_.pool);

   p_message_emis->entete.service = service_id;
   p_message_emis->entete.type_message = type_message;

   /*Envoi du message*/
   ExitEnvoie(bal_dest,_HRD_.bal, (struct_neutre *)(p_message_emis));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EnvoiAcqGestionAlarme(unsigned char bal_dest,
                         short int service_id,
                         short int type_message,
                         struct_hrd_gestion_alarme msg_gestion_alarme)
* PARAMETRES:
*            entree : boite a lettre destinatrice
*                     identificateur de service
*                     type du message a envoyer
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Envoyer un message d'acquittement ou de non acquittement
*      vers l'application pour le service M_HRD_GESTION_ALARME.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EnvoiAcqGestionAlarme(noyau_bal_id bal_dest,
                                    short int service_id,
                                    short int type_message,
                                    struct_hrd_gestion_alarme msg_gestion_alarme)
{
   struct_hrd_message *p_message_emis = (struct_hrd_message *) (0);

   /*Allocation memoire pour le message a emettre*/
   ExitAlloue((struct_neutre **)(&p_message_emis),
                     sizeof(struct_hrd_message),
                     _HRD_.pool);

   p_message_emis->entete.service = service_id;
   p_message_emis->entete.type_message = type_message;
   p_message_emis->u.msg_gestion_alarme = msg_gestion_alarme;

   /*Envoi du message*/
   ExitEnvoie(bal_dest,_HRD_.bal,(struct_neutre *)(p_message_emis));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void EnvoiAcqGestionDateHeure(unsigned char bal_dest,
*                                        short int service_id,
*                                        short int type_message)
* PARAMETRES:
*            entree : boite a lettre destinatrice
*                     identificateur de service
*                     type du message a envoyer
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Envoyer un message d'acquittement ou de non acquittement
*      vers l'application pour le service M_HRD_GESTION_DATE_HEURE.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void EnvoiAcqGestionDateHeure
                    (noyau_bal_id bal_dest,
                     short int service_id,
                     short int type_message)
{
   struct_hrd_message *p_message_emis = (struct_hrd_message *) (0);

   /*Allocation memoire pour le message a emettre*/
   ExitAlloue((struct_neutre **)(&p_message_emis),
                     sizeof(struct_hrd_message),
                     _HRD_.pool);

   p_message_emis->entete.service = service_id;
   p_message_emis->entete.type_message = type_message;

   /*Envoi du message*/
   ExitEnvoie(bal_dest,_HRD_.bal,(struct_neutre *)(p_message_emis));
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: unsigned long int SecondesDe80ADateCourante()
* PARAMETRES:Void.
* RETOUR: Le nombre de secondes ecoulees depuis 01/01/1980 .
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Retourne le nombre de secondes ecoulees depuis 01/01/1980.
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED unsigned long int SecondesDe80ADateCourante()

{
   struct_hrd_heure_hex heure_dos;
   struct_hrd_date_entree_hex date_dos;

   date_dos = HRDDonneDate();
   heure_dos = HRDDonneHeure();

   return ( HRDSecondesDepuis80 ( date_dos,heure_dos) );
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void Demande(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Arrete HRD.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void Demande(struct_hrd_message *p_struct_hrd_message)
{
   EnvoiAcq(p_struct_hrd_message->entete.neutre.bl_retour,
            M_HRD_ARRET,
            HRD_ARRET_ACQ);
   Termine();
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ArmePonct(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Armer une alarme ponctuelle.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ArmePonct(struct_hrd_message * p_struct_hrd_message)
{
   struct_hrd_gestion_alarme msg_gestion_alarme = {0};
   t_hrd_num_reveil num_alarme = 0;

   if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
   {
      fprintf(_hrd_.fic_trace, 
              "Initialisation of a ponctual alarm requested via a message\n");
   }

   /*memoriation des donnees du reveil*/
   msg_gestion_alarme.date = p_struct_hrd_message->u.msg_gestion_alarme.date;
   msg_gestion_alarme.temps = p_struct_hrd_message->u.msg_gestion_alarme.temps;
   msg_gestion_alarme.periode = 0;

   if ( HRD_REVEIL_OK != HRDArmeReveilPonctuel(
                            p_struct_hrd_message->u.msg_gestion_alarme.date,
                            p_struct_hrd_message->u.msg_gestion_alarme.temps,
                            p_struct_hrd_message->entete.neutre.bl_retour,
                            &num_alarme))
   {
      msg_gestion_alarme.num_alarme = 0;

      /*Il n'y a plus de numero d'alarme disponible ou bien l'heure est depassee,
      le reveil n'est pas acquitte*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_PONCT_NACQ,
                            msg_gestion_alarme);
   }
   else
   {
      msg_gestion_alarme.num_alarme = num_alarme;

      /*Acquitttement de la demande de reveil*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_PONCT_ACQ,
                            msg_gestion_alarme);
   }
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ArmeHor(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Armer une alarme horaire.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ArmeHor(struct_hrd_message *p_struct_hrd_message)
{
   struct_hrd_gestion_alarme msg_gestion_alarme = {0};
   t_hrd_num_reveil num_alarme = 0;

   /*memorisation des donnees du reveil*/
   msg_gestion_alarme.date = p_struct_hrd_message->u.msg_gestion_alarme.date;
   msg_gestion_alarme.temps = p_struct_hrd_message->u.msg_gestion_alarme.temps;
   msg_gestion_alarme.periode = 3600;  /*periode = une heure*/

   if ( HRD_REVEIL_OK != HRDArmeReveilHoraire(
                            p_struct_hrd_message->entete.neutre.bl_retour,
                            &num_alarme))
   {
      msg_gestion_alarme.num_alarme = 0;

      /*Il n'y a plus de numero d'alarme disponible,le reveil n'est pas aquitte*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_HOR_NACQ,
                            msg_gestion_alarme);
   }
   else
   {
      msg_gestion_alarme.num_alarme = num_alarme;

      /*Acquitttement de la demande de reveil*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_HOR_ACQ,
                            msg_gestion_alarme);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ArmeJour(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Armer une alarme journaliere.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ArmeJour(struct_hrd_message *p_struct_hrd_message)
{
   struct_hrd_gestion_alarme msg_gestion_alarme = {0};
   t_hrd_num_reveil num_alarme = 0;

   /*memorisation des donnees du reveil*/
   msg_gestion_alarme.date = p_struct_hrd_message->u.msg_gestion_alarme.date;
   msg_gestion_alarme.temps = p_struct_hrd_message->u.msg_gestion_alarme.temps;
   msg_gestion_alarme.periode = 86400l;  /*periode = un jour*/

   if (HRD_REVEIL_OK != HRDArmeReveilJournalier(
                          p_struct_hrd_message->entete.neutre.bl_retour,
                          &num_alarme) )
   {
      msg_gestion_alarme.num_alarme = 0;

      /*Il n'y a plus de numero d'alarme disponible,le reveil n'est pas aquitte*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_JOUR_NACQ,
                            msg_gestion_alarme);
   }
   else
   {
      msg_gestion_alarme.num_alarme = num_alarme;

      /*Acquitttement de la demande de reveil*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_JOUR_ACQ,
                            msg_gestion_alarme);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ArmeRegul(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Armer une alarme reguliere.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void ArmeRegul(struct_hrd_message *p_struct_hrd_message)
{
   struct_hrd_gestion_alarme msg_gestion_alarme = {0};
   t_hrd_num_reveil num_alarme = 0;

   /*memoriation des donnees du reveil*/
   msg_gestion_alarme.date = p_struct_hrd_message->u.msg_gestion_alarme.date;
   msg_gestion_alarme.temps = p_struct_hrd_message->u.msg_gestion_alarme.temps;
   msg_gestion_alarme.periode = p_struct_hrd_message->u.msg_gestion_alarme.periode;

   if (HRD_REVEIL_OK != HRDArmeReveilRegulier(
                           p_struct_hrd_message->u.msg_gestion_alarme.date,
                           p_struct_hrd_message->u.msg_gestion_alarme.temps,
                           p_struct_hrd_message->u.msg_gestion_alarme.periode,
                           p_struct_hrd_message->entete.neutre.bl_retour,
                           &num_alarme))
   {
      msg_gestion_alarme.num_alarme = 0;

      /*Il n'y a plus de numero d'alarme disponible ou bien l'heure de reveil
      est depassee, le reveil n'est pas acquitte*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_REGUL_NACQ,
                            msg_gestion_alarme);
   }
   else
   {
      msg_gestion_alarme.num_alarme = num_alarme;

      /*Acquitttement de la demande de reveil*/
      EnvoiAcqGestionAlarme(p_struct_hrd_message->entete.neutre.bl_retour,
                            M_HRD_GESTION_ALARME,
                            HRD_ARME_REGUL_ACQ,
                            msg_gestion_alarme);
   }
}
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void Desarme(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Desarmer une alarme .
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void Desarme(struct_hrd_message *p_struct_hrd_message)
{
   short int acq;
   short int nacq;
   t_hrd_num_reveil num_alarme;
   struct_hrd_gestion_alarme msg_gestion_alarme;
   noyau_bal_id bl_retour;
   enum_type_alarme type_alarme;

   switch (p_struct_hrd_message->entete.type_message)
   {
      case (HRD_DESARME_PONCT):

         acq = HRD_DESARME_PONCT_ACQ;
         nacq = HRD_DESARME_PONCT_NACQ;
         type_alarme = ALARME_PONCT;

      break;

      case (HRD_DESARME_JOUR):

         acq = HRD_DESARME_JOUR_ACQ;
         nacq = HRD_DESARME_JOUR_NACQ;
         type_alarme = ALARME_JOUR;

      break;

      case (HRD_DESARME_HOR):

         acq = HRD_DESARME_HOR_ACQ;
         nacq = HRD_DESARME_HOR_NACQ;
         type_alarme = ALARME_HOR;

      break;

      case (HRD_DESARME_REGUL):

         acq = HRD_DESARME_REGUL_ACQ;
         nacq = HRD_DESARME_REGUL_NACQ;
         type_alarme = ALARME_REGUL;

      break;

      default :
         if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
         {
            fprintf(_hrd_.fic_trace,"HRD : Message du service (M_HRD_GESTION_ALARME) inexistant\n");
         }
         break;
   }

   num_alarme = p_struct_hrd_message->u.msg_gestion_alarme.num_alarme;
   bl_retour = p_struct_hrd_message->entete.neutre.bl_retour;
   msg_gestion_alarme.num_alarme = num_alarme;
   /*Si  la tache qui demande le desarmement
   n'est pas celle qui a demande le reveil on n'acquitte pas le desarmement
   ou si l'alarme n'est pas active ou si elle n'est pas du meme type*/
   if ( HRD_OK != HRDAnnuleReveil(num_alarme))
   {
       EnvoiAcqGestionAlarme(bl_retour,
                             M_HRD_GESTION_ALARME,
                             nacq,
                             msg_gestion_alarme);
   }
   else
   {
       /*On acquitte le desarmement*/
       EnvoiAcqGestionAlarme(bl_retour,
                             M_HRD_GESTION_ALARME,
                             acq,
                             msg_gestion_alarme);
   }

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void ArretEvol(unsigned char bl_retour)
* PARAMETRES:BAL de retour
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Arrete l'evolution du temps.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED  void ArretEvol(noyau_bal_id bl_retour)
{

   if (HRDDonneEtatDuTemps() == HRD_TEMPS_SUSPENDU)
   {
      /*l'ecoulement du temps est suspendu, la demande n'est pas acquittee*/
      EnvoiAcq(bl_retour,M_HRD_EVOL_HEURE,HRD_EVOL_ARRET_NACQ);
   }
   else
   {
      /* on arrˆte le temps */
      HRDSuspendEvolutionDuTemps (bl_retour);

      /*On acquitte la demande*/
      EnvoiAcq(bl_retour,M_HRD_EVOL_HEURE,HRD_EVOL_ARRET_ACQ);
   }

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void DepartEvol(unsigned char bl_retour)
* PARAMETRES:BAL de retour
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Reprend l'evolution du temps.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void DepartEvol(noyau_bal_id bl_retour)
{
   if ( (HRDDonneEtatDuTemps () == HRD_TEMPS_EN_EVOLUTION) ||  (_HRD_.evol_bl != bl_retour) )
   {
      /*L'ecoulement du temps est actif ou bien la tache qui demande la
      reprise du temps n'est pas celle qui a demande la
      suspension. On n'acquitte pas la demande*/
      EnvoiAcq(bl_retour,M_HRD_EVOL_HEURE,HRD_EVOL_DEPART_NACQ);
   }
   else
   {
      /*la tache qui demande la reprise du temps est celle qui a demande la
      suspension. On acquitte la demande et on reprend l'evolution*/
      HRDReprendEvolutionDuTemps (bl_retour);
      EnvoiAcq(bl_retour,M_HRD_EVOL_HEURE,HRD_EVOL_DEPART_ACQ);
   }


}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void NouvelleDateHeure(struct_hrd_message *p_struct_hrd_message)
* PARAMETRES:pointeur sur structure message
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale.
* ROLE: Change la date et l'heure.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void NouvelleDateHeure(struct_hrd_message *p_struct_hrd_message)
{
  SYSTEMTIME date;
  SYSTEMTIME date_avant_changement;

  GetLocalTime(&date_avant_changement);
  GetLocalTime(&date);

  date.wYear = p_struct_hrd_message->u.msg_gestion_date_heure.date.annee;
  date.wMonth = p_struct_hrd_message->u.msg_gestion_date_heure.date.mois;
  date.wDay = p_struct_hrd_message->u.msg_gestion_date_heure.date.jour;
  /*date.dayofweek= 0;*/

  date.wHour = p_struct_hrd_message->u.msg_gestion_date_heure.temps.heure ;
  date.wMinute = p_struct_hrd_message->u.msg_gestion_date_heure.temps.minute;
  date.wSecond = p_struct_hrd_message->u.msg_gestion_date_heure.temps.seconde;
  date.wMilliseconds = 0;

  if (SetLocalTime(&date))
  {
     /*Le changement de date et d'heure du PC s'est bien effectue,
        on acquitte le changement*/
      EnvoiAcqGestionDateHeure
                    (p_struct_hrd_message->entete.neutre.bl_retour,
                     M_HRD_GESTION_DATE_HEURE,
                     HRD_NOUVELLE_DATE_HEURE_ACQ);

      if (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION)
      {
         /*l'evolution du temps n'est pas suspendue*/
         _HRD_.date = DatePC();
         _HRD_.heure = HeurePC();
      }

      /*MAJ des reveils*/
      MAJReveils();
  }
  else
  {
     /*Le changement d'heure du PC s'est mal effectue,
      on n'acquitte pas le changement.*/
     /*La date du PC reprend son ancienne valeur*/
     if (SetLocalTime(&date_avant_changement))
     {

         EnvoiAcqGestionDateHeure
                    (p_struct_hrd_message->entete.neutre.bl_retour,
                     M_HRD_GESTION_DATE_HEURE,
                     HRD_NOUVELLE_DATE_HEURE_NACQ);
     }
     else
     {  /*Il est impossible de revenir a la date precedente*/
        EnvoiAcqGestionDateHeure
                    (p_struct_hrd_message->entete.neutre.bl_retour,
                     M_HRD_GESTION_DATE_HEURE,
                     HRD_NOUVELLE_DATE_HEURE_PB);

        if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
        {
           fprintf(_hrd_.fic_trace,"HRD : Pb lors du changement de date_heure\n");
        }
     }
  }
}

