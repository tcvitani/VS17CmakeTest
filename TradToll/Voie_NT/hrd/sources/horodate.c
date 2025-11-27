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
 *    Rev 1.2   Jan 20 1999 14:15:44   bph
 *    Rev 1.1   02 Oct 1998 11:09:00   bph
 *    Rev 1.11  03 Jun 1998 10:39:38   DPI
 *    1.0.6     25 Jan 2011 16:48      POG Multithread protection
*
* --------------------------------------------------------------------
* $F_HEAD


/* #include Borland C */
#include <stdio.h>



/* #include Modulotheque */
#include <noyau.h>
#include <horodate.h>

/*#include interne */
#include <hrd_serv.h>

#include <memclass.h>

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED enum_hrd_reveil RequestHrdSemaphore (noyau_delai timeout)
* PARAMETRES: timeout
* RETOUR: 
* --------------------------------------------------------------------
* VARIABLES: HRD_UNAVAILABLE
*            HRD_REVEIL_OK
*            HRD_UNITITIALIZED
* --------------------------------------------------------------------
* ROLE: Request exclusive access to HRD internal structure
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_hrd_reveil RequestHrdSemaphore (noyau_delai timeout)
{
   noyau_enum_retour status_attente;

   /* This function is a bit of a hack.
      Considering the design of the module,
      hrd can't know if the calling thread already has open a semaphore.
      To have a minimum impact, the HRD_sem is first checked outside a region.
      If it needs to be open, a region is started, the HRD_sem is checked again
      and created if necessary */
   if (_hrd_.HRD_sem == NULL)
   {
      DebutRegionLocale();	// AHA 16-3-2011
      if (_hrd_.HRD_sem == NULL)
      {
         /* OpenSemaphore is a windows function. Noyau should be updated to
            include an implementation */
         _hrd_.HRD_sem = OpenSemaphore (SEMAPHORE_ALL_ACCESS , TRUE, "HRD");

         if (NULL == _hrd_.HRD_sem)
         {
            return HRD_UNITITIALIZED;
         }
      }
      FinRegionLocale();	// AHA 16-3-2011
   }

   status_attente = AttendPriseSemaphoreTantQue(_hrd_.HRD_sem, timeout);

   switch (status_attente)
   {
   case NOYAU_SEM_DELAI:
      _hrd_.HRD_sem = 0;
      return HRD_UNAVAILABLE;
   case NOYAU_SEM_NOK:
      _hrd_.HRD_sem = 0;
      return HRD_UNITITIALIZED;
   default:
      return HRD_REVEIL_OK;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED void InitHorodate (void)
* PARAMETRES: void
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Initialisation.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void InitHorodate (void)
{
   short int i = 0;

   /* Get exclusive access to the _HRD_ shared global variable */
   if (RequestHrdSemaphore(HRD_SEM_TO) != HRD_REVEIL_OK )
   {
      if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
      {
         fprintf(_hrd_.fic_trace, "Error waiting for semaphore\n");
         fflush(_hrd_.fic_trace);
      }
   }
   _HRD_.etat_init = 1;

   /*Initialisation du tableau d'alarme. Aucun numero n'est attribue*/
   for (i=0 ; i < MAX_ALARMES ; i++)
   {
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
      _HRD_.tab_alarme[i].id = 0;
   }

   RendSemaphore(_hrd_.HRD_sem, 1); 

   /* pas de reveil figeant activ‚ */
   _HRD_.num_reveil_figeant = -1;

   /*L'evolution du temps est active*/
   _HRD_.evol = HRD_TEMPS_EN_EVOLUTION;
   _HRD_.evol_bl = -1;

   /*La date et l'_HRD_.heure de HRD sont initialisees*/
   _HRD_.date = DatePC();
   _HRD_.heure = HeurePC();

   // on crée la BAL de horodate
   _HRD_.bal = PublieBAL ("HRD_0", 0);

}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  unsigned int Hrd
* PARAMETRES: void
* RETOUR:.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: Arrete le module HORODATE.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED DWORD WINAPI Hrd ( LPVOID param)
{
   struct_hrd_message *p_message_recu = (struct_hrd_message *) (0);
   struct_neutre *p_neutre = (struct_neutre *) (0);
   short int code_rtc;

   InitHorodate ();

   ChangePriorite (TacheCourante(), _HRD_.priorite);

   while (1)
   {
      /*Attente d'un message venant de l'application*/

      code_rtc = Recoit (_HRD_.bal, (struct_neutre **) (&p_neutre), 1);
      p_message_recu = (struct_hrd_message *)p_neutre;

      DebutRegionLocale();	// AHA 16-3-2011

      /*Traitement des services venus de l'application*/
      if (code_rtc == NOYAU_BAL_MESS)
      {
         switch (p_message_recu->entete.service)
         {
            case (M_HRD_ARRET) :

               ReceptionArret(p_message_recu);
               break;

            case (M_HRD_GESTION_ALARME) :

               ReceptionGestionAlarme(p_message_recu);
               break;

            case (M_HRD_EVOL_HEURE) :

               ReceptionEvolHeure(p_message_recu) ;
               break;

            case (M_HRD_GESTION_DATE_HEURE) :

               ReceptionGestionDateHeure(p_message_recu);
               break;

            default :
              if (_hrd_.flag_trace == HRD_TRACE_ACTIVE)
              {
                 fprintf(_hrd_.fic_trace,"HRD : Service inexistant\n");
              }
               break;

         } /*end switch*/

         ExitLibere((struct_neutre **)(&p_neutre));
      } /*end if*/

      FinRegionLocale();	// AHA 16-3-2011

      /* test si ler‚veil figeant est … ‚ch‚ance */
      if (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION)
      {
         Reveil_Figeant ();
      }

      /*Traitements cyclique*/
      if (HRDDonneEtatDuTemps() == HRD_TEMPS_EN_EVOLUTION)
      {  /*L'evolution du temps n'est pas suspendue*/
         /*Mise a jour de la date et de l'heure de HRD*/
         _HRD_.date = DatePC();
         _HRD_.heure = HeurePC();

         /*Envoi d'une alarme aux taches dont la date de reveil est a echeance*/
         Reveil();
      }

   } /*end while*/

   return 0;
}