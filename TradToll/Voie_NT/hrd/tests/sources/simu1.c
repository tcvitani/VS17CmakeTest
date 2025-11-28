/****************************************************************************
*
*                                  CS ROUTE
* TITRE       : SIMU1.C
* AUTEUR      : DA
* CREATION    : 17/05/96
* DESCRIPTION :
*
*           Role du fichier : Fichier source de la tƒche SIMU
*           Test les fonctions synchrones de demande de reveil,
*           d'evolution du temps et de changement de date/heure
*
****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <string.h>

#include <noyau.h>

#include <horodate.h>

#include <simu1.h>
#include <init.h>

extern short int  etat_logiciel;
extern short int priorite_tache;
extern short int  acq_fin;

/* DEBUG */
char	chaine[50] ;

/*********************** DEFINITION DES VARIABLES **************************/

short int   fin;         /* flag pour sortir de la tache */


noyau_pool_id  simu_pool_id;             /* Nø de pool … utiliser */
noyau_tache_id simu_id;                  /* Nø de la tƒche SIMU   */

noyau_bal_id   simu_bl;                  /* Nø de la BAL par defaut de SIMU  */
noyau_bal_id   simu_bl_A;                /* Nø de la BAL A de SIMU  */
noyau_bal_id   simu_bl_B;                /* Nø de la BAL B de SIMU  */
noyau_bal_id   simu_bl_C;                /* Nø de la BAL C de SIMU  */
noyau_bal_id   simu_bl_D;                /* Nø de la BAL D de SIMU  */

noyau_bal_id   simu_hrd_bl;              /* Nø de la BAL de ZIP  */

short int      simu_sem_ecran;           /* Nø du s‚maphore de SIMU */

noyau_bal_id   boite ;                   /* boite aux lettres par defaut pour affichage */

struct_tache t_simu[SIMU_NB_TACHE+1] =        /* Parametres lies a             */
                                         /* l'initialisation des tƒches   */
                                         /* temps r‚elles                 */
{
   /*Activation, priorite,pointeur,taille pile,point d'entree */
   { 1, THREAD_PRIORITY_NORMAL, 2048, simu, NULL },
   { 0,0,0,NULL,NULL }

};

/*********************** DEFINITION DES FONCTIONS **************************/
/*****************************************************************************
* Function  : syst_ecrire_trace
*-----------------------------------------------------------------------------
* Parameters: input : char far *p_trace : Pointeur on message to display
*-----------------------------------------------------------------------------
*  Action   : Display the message on the screen
*-----------------------------------------------------------------------------
* Return    : None
*****************************************************************************/

void syst_ecrire_trace (char *p_trace)
{

  DebutRegion();
  printf (p_trace);
  FinRegion();

}


/****************************************************************************
* Fonction  : simu_num_bal_defaut
*----------------------------------------------------------------------------
* Parametres: Aucun
*----------------------------------------------------------------------------
*  Action   : Envoi des messages applicatifs voie => tcpip
*----------------------------------------------------------------------------
* Retour    : Aucun
****************************************************************************/


void simu_num_bal_defaut(void)
{
  short int        choix;
  short int        the_end ;

  /* clear screen */
  syst_ecrire_trace ("\x1B[2J");
  syst_ecrire_trace ("\nD‚finition du num‚ro de BAL par d‚faut en retour\n\n");
  syst_ecrire_trace ("\t a : boite aux lettres A\n") ;
  syst_ecrire_trace ("\t b : boite aux lettres B\n") ;
  syst_ecrire_trace ("\t c : boite aux lettres C\n") ;
  syst_ecrire_trace ("\t d : boite aux lettres D\n") ;

  the_end = 0 ;

  while ( the_end == 0 )
  {
    if (_kbhit() != 0)
    {
      syst_ecrire_trace ("   Choix => ");
      /* R‚cup‚ration de la touche clavier */
      choix = _getch();
      sprintf_s (chaine, sizeof(chaine) ,"%c\n", choix);
      syst_ecrire_trace (chaine) ;

      switch (choix)
      {
         case 'a'  :
            the_end = 1 ;
            simu_bl = simu_bl_A ;
            boite ='A' ;
         break ;

         case 'b'  :
            the_end = 1 ;
            simu_bl = simu_bl_B ;
            boite ='B' ;
         break ;

         case 'c'  :
            the_end = 1 ;
            simu_bl = simu_bl_C ;
            boite ='C' ;
         break ;

         case 'd'  :
            the_end = 1 ;
            simu_bl = simu_bl_D ;
            boite ='D' ;
         break ;

         default :
            the_end = 1 ;
            syst_ecrire_trace ("TOUCHE NON VALIDE\n") ;
         break ;
      } /* switch */
    DelaiTache(20);
    } /* touche !=0 */
    else
    {
      DelaiTache (2);
    }
  } /* while the_end */
}



void simu_envoi_hrd(short int service, short int type_message)
{
   short int code_rtc;
   struct_hrd_message  *p_msg_emis = (struct_hrd_message *)(0);

   code_rtc = Alloue((struct_neutre **)(&p_msg_emis),
               sizeof(struct_hrd_message),simu_pool_id);
   if(code_rtc == NOYAU_OK)
   {
      p_msg_emis->entete.service = service;
      p_msg_emis->entete.type_message = type_message;

      Envoie(simu_hrd_bl,simu_bl,(struct_neutre *)(p_msg_emis));
   }
   else
   {
   printf("probleme alloue\n");
   };
}

void simu_envoi_date_heure(short int service,
                           short int type_message,
                           struct_hrd_gestion_date_heure date_heure)
{
   short int code_rtc;
   struct_hrd_message  *p_msg_emis = (struct_hrd_message *)(0);

   code_rtc = Alloue((struct_neutre **)(&p_msg_emis),
               sizeof(struct_hrd_message),simu_pool_id);
   if(code_rtc == NOYAU_OK)
   {
      p_msg_emis->entete.service = service;
      p_msg_emis->entete.type_message = type_message;
      p_msg_emis->u.msg_gestion_date_heure = date_heure;

      Envoie(simu_hrd_bl,simu_bl,(struct_neutre *)(p_msg_emis));
   }
   else
   {
   printf("probleme alloue\n");
   };
}

void simu_envoi_desarme(short int service,
                        short int type_message,
                        unsigned short num_alarme)
{
   short int code_rtc;
   struct_hrd_message  *p_msg_emis = (struct_hrd_message *)(0);

   code_rtc = Alloue((struct_neutre **)(&p_msg_emis),
               sizeof(struct_hrd_message),simu_pool_id);
   if(code_rtc == NOYAU_OK)
   {
      p_msg_emis->entete.service = service;
      p_msg_emis->entete.type_message = type_message;
      p_msg_emis->u.msg_gestion_alarme.num_alarme = num_alarme;

      Envoie(simu_hrd_bl,simu_bl,(struct_neutre *)(p_msg_emis));
   }
   else
   {
   printf("probleme alloue\n");
   };
}

void simu_envoi_hrd_alarme(short int service,
                           short int type_message,
                           struct_hrd_gestion_alarme message_alarme)
{
   short int code_rtc;
   struct_hrd_message  *p_msg_emis = (struct_hrd_message *)(0);


   code_rtc = Alloue((struct_neutre **)(&p_msg_emis),
               sizeof(struct_hrd_message),simu_pool_id);

   if(code_rtc == NOYAU_OK)
   {
      p_msg_emis->entete.service = service;
      p_msg_emis->entete.type_message = type_message;
      p_msg_emis->u.msg_gestion_alarme = message_alarme;

      Envoie(simu_hrd_bl,simu_bl,(struct_neutre *)(p_msg_emis));
   }
   else
   {
   printf("probleme alloue\n");
   };
}
/****************************************************************************
* Fonction  : messages_hrd
*----------------------------------------------------------------------------
* Parametres: Aucun
*----------------------------------------------------------------------------
*  Action   : Envoi des messages vers hrd
*----------------------------------------------------------------------------
* Retour    : Aucun
****************************************************************************/

void messages_hrd(void)
{

  short int        choix;
  short int        touche;
  short int        the_end ;
  noyau_bal_id     bal_defaut ;
  noyau_bal_id     lettre;
  unsigned char    mois,jour,heure,minute,seconde ;
  struct_hrd_gestion_alarme message_alarme;
  struct_hrd_gestion_date_heure message_date_heure;
  short int num_reveil;
  unsigned long int nb_secondes;
  struct_hrd_date_entree_hex date = {0};
  struct_hrd_date_entree_hex date_sortie = { 0 };
  struct_hrd_date_entree_hex date1 = { 0 };
  struct_hrd_date_entree_hex date2 = { 0 };

  struct_hrd_heure_hex temps = { 0 };
  struct_hrd_heure_hex temps_sortie = { 0 };
  struct_hrd_heure_hex temps1 = { 0 };
  struct_hrd_heure_hex temps2 = { 0 };
  
  int resultat;

  syst_ecrire_trace ("\nSimulation des messages M_HRD_XXX\n\n");
  syst_ecrire_trace ("\t x : Demande d'arret de HRD\n");
  syst_ecrire_trace ("\t b : Demande d'alarme ponctuelle\n");
  syst_ecrire_trace ("\t c : Demande d'alarme horaire\n");
  syst_ecrire_trace ("\t d : Demande d'alarme journaliere\n");
  syst_ecrire_trace ("\t e : Demande d'alarme reguliere\n");
  syst_ecrire_trace ("\t f : Demande d'alarme ponctuelle figeante\n");
  syst_ecrire_trace ("\t g : Demande d'alarme horaire figeante\n");
  syst_ecrire_trace ("\t h : Demande d'alarme journaliere figeante\n");
  syst_ecrire_trace ("\t i : Demande d'alarme reguliere figeante\n");
  syst_ecrire_trace ("\t r : Demande d'annulation d'un r‚veil\n");
  syst_ecrire_trace ("\t j : Demande d'arret de l'evolution du temps\n");
  syst_ecrire_trace ("\t k : Demande de depart de l'evolution du temps\n");
  syst_ecrire_trace ("\t l : Demande de changement de date et d'heure\n");
  syst_ecrire_trace ("\t m : Changement de BAL de reception temporaire.\n");
  syst_ecrire_trace ("\t n : Conversion de secondes en date.\n");
  syst_ecrire_trace ("\t o : Addition de dates.\n");
  syst_ecrire_trace ("\t p : Soustraction de dates.\n");
  syst_ecrire_trace ("\t q : Comparaison de dates.\n\n");
  the_end = 0 ;

  /* sauvegarde de la BAL par defaut */
  bal_defaut = simu_bl ;
  lettre = boite ;

  while ( the_end == 0 )
  {
    touche = _kbhit();
    if (touche != 0)
    {
      syst_ecrire_trace ("   Choix => ");
      /* R‚cup‚ration de la touche clavier */
      choix = _getch();
      sprintf_s (chaine, sizeof(chaine) ,"%c\n", choix);
      syst_ecrire_trace (chaine) ;


      switch (choix)
      {
        case 'x'  :
           fin = 1;
           the_end = 1 ;
           simu_envoi_hrd(M_HRD_ARRET,HRD_DEMANDE);
        break ;

        case 'b'  :
           printf("Donner la date du reveil.\n");
           printf("Annee : ");
           scanf_s("%hd",&message_alarme.date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&message_alarme.date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&message_alarme.date.jour);
           printf("\nDonner l'heure du reveil");
           printf("\nHeure :");
           scanf_s("%hhd",&message_alarme.temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&message_alarme.temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&message_alarme.temps.seconde);

           resultat = HRDArmeReveilPonctuel (message_alarme.date,
                                             message_alarme.temps,
                                             simu_bl,
                                             &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_DATE_HEURE_DEPASSEE:
                   printf("\nDate de reveil d‚pass‚e.");
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'c'  :
           resultat = HRDArmeReveilHoraire (simu_bl,
                                            &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'd'  :
           resultat = HRDArmeReveilJournalier (simu_bl,
                                               &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'e'  :
           message_alarme.periode = 0x0;
           printf("Donner la date du reveil.\n");
           printf("Annee : ");
           scanf_s("%hd",&message_alarme.date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&message_alarme.date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&message_alarme.date.jour);
           printf("\nDonner l'heure du reveil");
           printf("\nHeure :");
           scanf_s("%hhd",&message_alarme.temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&message_alarme.temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&message_alarme.temps.seconde);
           printf("\nDonner la periode du reveil : ");
           scanf_s("%d",&message_alarme.periode);


           resultat = HRDArmeReveilRegulier (message_alarme.date,
                                             message_alarme.temps,
                                             message_alarme.periode,
                                             simu_bl,
                                             &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_DATE_HEURE_DEPASSEE:
                   printf("\nDate de reveil d‚pass‚e.");
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }

        break ;

        case 'f'  :
           printf("Donner la date du reveil.\n");
           printf("Annee : ");
           scanf_s("%hd",&message_alarme.date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&message_alarme.date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&message_alarme.date.jour);
           printf("\nDonner l'heure du reveil");
           printf("\nHeure :");
           scanf_s("%hhd",&message_alarme.temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&message_alarme.temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&message_alarme.temps.seconde);

           resultat = HRDArmeReveilPonctuelFigeant (message_alarme.date,
                                                    message_alarme.temps,
                                                    simu_bl,
                                                    &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_DATE_HEURE_DEPASSEE:
                   printf("\nDate de reveil d‚pass‚e.");
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'g'  :
           resultat = HRDArmeReveilHoraireFigeant (simu_bl,
                                                   &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'h'  :
           resultat = HRDArmeReveilJournalierFigeant (simu_bl,
                                                      &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }
        break ;

        case 'i'  :
           message_alarme.periode = 0x0;
           printf("Donner la date du reveil.\n");
           printf("Annee : ");
           scanf_s("%hd",&message_alarme.date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&message_alarme.date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&message_alarme.date.jour);
           printf("\nDonner l'heure du reveil");
           printf("\nHeure :");
           scanf_s("%hhd",&message_alarme.temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&message_alarme.temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&message_alarme.temps.seconde);
           printf("\nDonner la periode du reveil : ");
           scanf_s("%ld",&message_alarme.periode);


           resultat = HRDArmeReveilRegulierFigeant (message_alarme.date,
                                                    message_alarme.temps,
                                                    message_alarme.periode,
                                                    simu_bl,
                                                    &num_reveil);
           switch (resultat)
           {
              case HRD_REVEIL_OK:
                   printf("\nDemande de reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_DATE_HEURE_DEPASSEE:
                   printf("\nDate de reveil d‚pass‚e.");
                   break;

              case HRD_PAS_DE_REVEIL_DISPONIBLE:
                   printf("\nPas de r‚veil disponible.");
                   break;

              default :
                   printf("\nDemande de reveil refus‚e (retour = %d).", resultat);
                   break;
           }

        break ;

        case 'r'  :
           printf("Donner le numero de l'alarme a desarmer : ");
           scanf_s("%hd",&num_reveil);

           resultat = HRDAnnuleReveil (num_reveil);

           switch (resultat)
           {
              case HRD_OK:
                   printf("\nAnnulation du reveil accept‚e (num = %d).", num_reveil);
                   break;

              case HRD_NOK:
                   printf("\nAnnulation refus‚e ou impossible.");
                   break;

              default :
                   printf("\nPb sur annulation de reveil (retour = %d).", resultat);
                   break;
           }

        break;

        case 'j'  :
           resultat = HRDSuspendEvolutionDuTemps (simu_bl);

           switch (resultat)
           {
              case HRD_TEMPS_SUSPENDU:
                   printf("\nDemande d'arret du temps accept‚e.");
                   break;

              case HRD_TEMPS_DEJA_SUSPENDU:
                   printf("\nTemps d‚j… arret‚.");
                   break;

              default :
                   printf("\nErreur (retour = %d).", resultat);
                   break;
           }

        break;

        case 'k'  :
           resultat = HRDReprendEvolutionDuTemps (simu_bl);

           switch (resultat)
           {
              case HRD_TEMPS_EN_EVOLUTION:
                   printf("\nDemande de reprise d'evolution du temps accept‚e.");
                   break;

              case HRD_TEMPS_DEJA_EN_EVOLUTION:
                   printf("\nEvolution du temps d‚j… actif.");
                   break;

              case HRD_TEMPS_REPRISE_NOK:
                   printf("\nLe demandeur n'est pas celui sui a suspendu le temps.");
                   break;

              default :
                   printf("\nErreur (retour = %d).", resultat);
                   break;
           }

        break;

        case 'l'  :
           the_end = 1;

           printf("Donner la nouvelle date.\n");
           printf("Annee : ");
           scanf_s("%hd",&message_date_heure.date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&message_date_heure.date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&message_date_heure.date.jour);
           printf("\nDonner la nouvelle heure ");
           printf("\nHeure :");
           scanf_s("%hhd",&message_date_heure.temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&message_date_heure.temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&message_date_heure.temps.seconde);


           simu_envoi_date_heure(M_HRD_GESTION_DATE_HEURE,
                                 HRD_NOUVELLE_DATE_HEURE,
                                 message_date_heure);
        break;

        case 'm' :
            /* Changement du Nø de BAL pour la reception */
            simu_num_bal_defaut() ;
            syst_ecrire_trace ("\x1B[2J");
            syst_ecrire_trace("\n") ;
            syst_ecrire_trace ("\nSimulation des messages M_HRD_XXX\n\n");
            syst_ecrire_trace ("\t a : Demande d'arret de HRD\n");
            syst_ecrire_trace ("\t b : Demande d'alarme ponctuelle\n");
            syst_ecrire_trace ("\t c : Demande d'alarme horaire\n");
            syst_ecrire_trace ("\t d : Demande d'alarme journaliere\n");
            syst_ecrire_trace ("\t e : Demande d'alarme reguliere\n");
            syst_ecrire_trace ("\t f : Demande de desarmement d'alarme ponctuelle\n");
            syst_ecrire_trace ("\t g : Demande de desarmement d'alarme horaire\n");
            syst_ecrire_trace ("\t h : Demande de desarmement d'alarme journaliere\n");
            syst_ecrire_trace ("\t i : Demande de desarmement d'alarme reguliere\n");
            syst_ecrire_trace ("\t j : Demande d'arret de l'evolution du temps\n");
            syst_ecrire_trace ("\t k : Demande de depart de l'evolution du temps\n");
            syst_ecrire_trace ("\t l : Demande de changement de date et d'heure\n");
            syst_ecrire_trace ("\t m : Changement de BAL de reception temporaire.\n");
            syst_ecrire_trace ("\t n : Conversion de secondes en date.\n");
            syst_ecrire_trace ("\t o : Addition de dates.\n");
            syst_ecrire_trace ("\t p : Soustraction de dates.\n");
            syst_ecrire_trace ("\t q : Comparaison de dates.\n\n");
        break ;

        case 'n' :
           nb_secondes = 0x0;
           printf("Donner le nombre de secondes : ");
           scanf_s("%lu",&nb_secondes);
           HRDConvSecondesEnDateHeure(nb_secondes,&date,&temps);
           printf("date : %02d / %02d / %04d\n",
                                      date.jour,date.mois,date.annee);
           printf("heure : %02d : %02d : %02d\n",
                                temps.heure,temps.minute,temps.seconde);
        break;

        case 'o' :
           printf("Donner la date.\n");
           printf("Annee : ");
           scanf_s("%hd",&date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&date.jour);
           printf("\nDonner l'heure ");
           printf("\nHeure :");
           scanf_s("%hhd",&temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&temps.seconde);
           printf("\nDonner le nombre de secondes a ajouter : ");
           scanf_s("%lu",&nb_secondes);
           HRDAjouteDateHeure(date,temps,nb_secondes,&date_sortie,&temps_sortie);
           printf("date : %02d / %02d / %04d\n",
                                      date_sortie.jour,date_sortie.mois,date_sortie.annee);
           printf("heure : %02d : %02d : %02d\n",
                                temps_sortie.heure,temps_sortie.minute,temps_sortie.seconde);
        break;

        case 'p' :
           printf("Donner la date.\n");
           printf("Annee : ");
           scanf_s("%hd",&date.annee);
           printf("\nMois : ");
           scanf_s("%hhd",&date.mois);
           printf("\nJour : ");
           scanf_s("%hhd",&date.jour);
           printf("\nDonner l'heure");
           printf("\nHeure :");
           scanf_s("%hhd",&temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&temps.seconde);
           printf("\nDonner le nombre de secondes a retrancher : ");
           scanf_s("%lu",&nb_secondes);
           HRDRetrancheDateHeure(date,temps,nb_secondes,&date_sortie,&temps_sortie);
           printf("date : %02d / %02d / %04d\n",
                                      date_sortie.jour,date_sortie.mois,date_sortie.annee);
           printf("heure : %02d : %02d : %02d\n",
                                temps_sortie.heure,temps_sortie.minute,temps_sortie.seconde);
        break;

        case 'q':
           printf("Donner la date1.\n");
           printf("Annee1 : ");
           scanf_s("%hd",&date1.annee);
           printf("\nMois1 : ");
           scanf_s("%hhd",&mois);
           date1.mois = (unsigned char) mois;
           printf("\nJour1 : ");
           scanf_s("%hhd",&jour);
           date1.jour = (unsigned char) jour;

           printf("\nDonner l'heure1");
           printf("\nHeure1 :");
           scanf_s("%hhd",&heure);
           temps1.heure = (unsigned char) heure;
           printf("\nMinute1 :");
           scanf_s("%hhd",&minute);
           temps1.minute = (unsigned char) minute;
           printf("\nSeconde1 :");
           scanf_s("%hhd",&seconde);
           temps1.seconde = (unsigned char) seconde;

           _flushall();

           printf("Donner la date2.\n");
           printf("Annee2 : ");
           scanf_s("%hd",&date2.annee);
           printf("\nMois2 : ");
           scanf_s("%hhd",&mois);
           date2.mois = (unsigned char) mois;
           fflush(stdin);
           printf("\nJour2 : ");
           scanf_s("%hhd",&jour);
           date2.jour = (unsigned char) jour;

           printf("\nDonner l'heure2");
           printf("\nHeure2 :");
           scanf_s("%hhd",&temps2.heure);
           printf("\nMinute2 :");
           scanf_s("%hhd",&temps2.minute);
           printf("\nSeconde2 :");
           scanf_s("%hhd",&temps2.seconde);

           resultat= HRDCompareDatesHeures(date1,temps1,date2,temps2);
           if (resultat == 0)
           {
              printf("\nLes dates sont identiques.");
           }
           else if (resultat == 1)
           {
              printf("\nDate1 est superieure a date2.");
           }
           else
           {
              printf("\nDate1 est inferieure a date2.");
           }

        break;

        default :
           the_end = 1 ;
           syst_ecrire_trace ("TOUCHE NON VALIDE\n") ;
        break ;
      } /* switch  */
    }/* touche !=0 */
    else
    {
       //DelaiTache(50);
    }
  }/* while the_end */

  /* restitution de la BAL par defaut */
  simu_bl = bal_defaut ;
  boite = lettre ;
}

void EcrireDateHeureAlarme (struct_hrd_message * p_simu_message)
{

   printf("annee : %u\n",p_simu_message->u.msg_gestion_alarme.date.annee);
   printf("mois : %u\n",p_simu_message->u.msg_gestion_alarme.date.mois);
   printf("jour : %u\n",p_simu_message->u.msg_gestion_alarme.date.jour);
   printf("heure : %u\n",p_simu_message->u.msg_gestion_alarme.temps.heure);
   printf("minute : %u\n",p_simu_message->u.msg_gestion_alarme.temps.minute);
   printf("seconde : %u\n",p_simu_message->u.msg_gestion_alarme.temps.seconde);
   printf("num_alarme : %u\n",p_simu_message->u.msg_gestion_alarme.num_alarme);
   _flushall();
}

void EcrireDateHeure (struct_hrd_message *p_simu_message)
{

   printf("annee : %x",p_simu_message->u.msg_gestion_alarme.date.annee);
   printf("mois : %x",p_simu_message->u.msg_gestion_alarme.date.mois);
   printf("jour : %x",p_simu_message->u.msg_gestion_alarme.date.jour);
   printf("heure : %x",p_simu_message->u.msg_gestion_alarme.temps.heure);
   printf("minute : %x",p_simu_message->u.msg_gestion_alarme.temps.minute);
   printf("seconde : %x",p_simu_message->u.msg_gestion_alarme.temps.seconde);
   _flushall();
}

void RecuArret(struct_hrd_message *p_simu_message)
{
   if(p_simu_message->entete.type_message == HRD_ARRET_ACQ)
   {
      syst_ecrire_trace("SIMU: recu acquittement arret \n") ;
      acq_fin = 1;
   }
   else
   {
      syst_ecrire_trace("SIMU: recu NON acquittement arret \n") ;
   }

}
void RecuGestionAlarme(struct_hrd_message *p_simu_message)
{
   if(p_simu_message->entete.type_message == HRD_ARME_PONCT_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement ARME PONCT\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   EcrireDateHeureAlarme(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ARME_PONCT_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement ARME PONCT\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   EcrireDateHeure(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ARME_HOR_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement ARME HOR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ARME_HOR_NACQ)
   syst_ecrire_trace("SIMU recu NON acquittement ARME HOR\n");

   else if(p_simu_message->entete.type_message == HRD_ARME_JOUR_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement ARME JOUR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ARME_JOUR_NACQ)
   syst_ecrire_trace("SIMU recu NON acquittement ARME JOUR\n");

   else if(p_simu_message->entete.type_message == HRD_ARME_REGUL_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement ARME REGUL\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   EcrireDateHeureAlarme(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ARME_REGUL_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement ARME REGUL\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   EcrireDateHeure(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_ALARME)
   {
   syst_ecrire_trace("SIMU recu REVEIL\n");
   syst_ecrire_trace("SIMU recu DONNEES :\n");

   EcrireDateHeureAlarme(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_REVEIL_FIGEANT)
   {
   syst_ecrire_trace("SIMU recu REVEIL FIGEANT\n");
   syst_ecrire_trace("SIMU recu DONNEES :\n");

   EcrireDateHeureAlarme(p_simu_message);
   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_PONCT_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement DESARME PONCT\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_PONCT_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement DESARME PONCT\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_HOR_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement DESARME HOR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_HOR_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement DESARME HOR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_JOUR_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement DESARME JOUR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_JOUR_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement DESARME JOUR\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_REGUL_ACQ)
   {
   syst_ecrire_trace("SIMU recu acquittement DESARME REGUL\n");
   syst_ecrire_trace("SIMU recu DONNEES :");

   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);

   syst_ecrire_trace("\n");
   }

   else if(p_simu_message->entete.type_message == HRD_DESARME_REGUL_NACQ)
   {
   syst_ecrire_trace("SIMU recu NON acquittement DESARME REGUL\n");
   syst_ecrire_trace("SIMU recu DONNEES :");
   printf("num_alarme : %x",p_simu_message->u.msg_gestion_alarme.num_alarme);
   syst_ecrire_trace("\n");
   }

   else
   syst_ecrire_trace("SIMU: type message GESTION ALARME inconnu\n");
   _flushall();
}
void RecuEvolHeure(struct_hrd_message *p_simu_message)
{
   if(p_simu_message->entete.type_message == HRD_EVOL_ARRET_ACQ)
   syst_ecrire_trace("SIMU recu acquittement EVOL ARRET\n");

   else if(p_simu_message->entete.type_message == HRD_EVOL_DEPART_ACQ)
   syst_ecrire_trace("SIMU recu acquittement EVOL DEPART\n");

   else if(p_simu_message->entete.type_message == HRD_EVOL_ARRET_NACQ)
   syst_ecrire_trace("SIMU recu NON acquittement EVOL ARRET\n");

   else if(p_simu_message->entete.type_message == HRD_EVOL_DEPART_NACQ)
   syst_ecrire_trace("SIMU recu NON acquittement EVOL DEPART\n");

   else
   syst_ecrire_trace("SIMU: type message EVOL HEURE inconnu\n");
   _flushall();
}

void RecuDateHeure(struct_hrd_message *p_simu_message)
{
   if(p_simu_message->entete.type_message == HRD_NOUVELLE_DATE_HEURE_ACQ)
   syst_ecrire_trace("SIMU recu acquittement  NOUVELLE DATE HEURE\n");

   else if(p_simu_message->entete.type_message == HRD_NOUVELLE_DATE_HEURE_NACQ)
   syst_ecrire_trace("SIMU recu NON acquittement  NOUVELLE DATE HEURE\n");

   else

   printf("SIMU: type message NOUVELLE DATE HEURE inconnu: %d\n",p_simu_message->entete.type_message);

   _flushall();
}
/****************************************************************************
* Fonction  : simu
*----------------------------------------------------------------------------
* Parametres: Aucun
*----------------------------------------------------------------------------
*  Action   : Point d'entr‚e de la tƒche SIMU
*----------------------------------------------------------------------------
* Retour    : Aucun
****************************************************************************/

DWORD WINAPI simu( LPVOID param)
{
  short int       choix;
  short int       touche;
  short int       code_rtc;
  struct_hrd_message  *p_simu_message = (struct_hrd_message *)(0);
  struct_neutre   *p_neutre;
//  short int       cpt_bal ;
  unsigned char   bal_recu ;
//  int             cpt ;
//  int             i;
  struct_hrd_date_entree_hex date;
  struct_hrd_heure_hex heure,temps;
  unsigned long int secondes_depuis_80;
  FILE * trace;
  struct_hrd_gestion_date_heure message_date_heure = {0}; // ZBR
  char res;
  char trace_path[MAX_PATH] = {0}; // ZBR
  errno_t err;

  fin = 0;
  acq_fin = 0;
  boite = 'A' ;

  /* Boite aux lettres par defaut*/
  simu_bl_A = PublieBAL ("SIMU_HRD_0", 0);
  simu_bl = simu_bl_A ;

  DebutRegion() ;

  /* Changement de la priorite de la tache  */
  ChangePriorite( TacheCourante() , priorite_tache) ;

  // ZBR
  /*Ouverture du fichier trace*/
  NoyGetTracesPath(trace_path);
  strcat_s(trace_path, sizeof(trace_path), "\\tracehrd.txt");
  //trace = fopen("c:\\temp\\tracehrd.txt","a");
  err = fopen_s(&trace, trace_path, "a");
  // end of ZBR
  HRDDebutTrace (trace);

  /*Affichage de la date et de l'heure de horodate*/
  heure = HRDDonneHeure();
  date = HRDDonneDate();
  printf("date : %02d / %02d / %04d\n", date.jour,date.mois,date.annee);
  printf("datehex : %02x / %02x / %04x\n", date.jour,date.mois,date.annee);
  printf("heurehex : %02x : %02x : %02x\n",heure.heure,heure.minute,heure.seconde);
  printf("heure : %02d : %02d : %02d\n",heure.heure,heure.minute,heure.seconde);

  secondes_depuis_80 = HRDSecondesDepuis80(date,heure);
  printf("secondes depuis 80 : %ld\n",secondes_depuis_80);

  syst_ecrire_trace ("\n") ;
  syst_ecrire_trace ("\tLA BOITE AUX LETTRES DE RECEPTION PAR DEFAUT EST LA BOITE 'A'\n") ;
  syst_ecrire_trace ("\t a : MESSAGES DE SERVICES VERS HRD\n");
  syst_ecrire_trace ("\t b : CHANGEMENT BOITE AUX LETTRES DE RECEPTION PAR DEFAUT\n");
  syst_ecrire_trace ("\t c : Coherence date\n");
  syst_ecrire_trace ("\t d : Coherence heure\n");
  syst_ecrire_trace ("\t e : Envoi de cas d'erreur\n");
  syst_ecrire_trace ("\t q : QUITTER\n\n");
  FinRegion();

  while (acq_fin != 1)
  {
     DelaiTache(2);
     DebutRegion();

     touche = _kbhit();

     if (touche != 0)
     {
        syst_ecrire_trace ("   Choix => ");
        /* R‚cup‚ration de la touche clavier */
        choix = _getch();
		sprintf_s(chaine, sizeof(chaine), "%c\n", choix);
        syst_ecrire_trace (chaine) ;

        switch (choix)
        {
          case 'a':
               /* clear screen */
               syst_ecrire_trace ("\x1B[2J");
               /* Messages de service */
               messages_hrd() ;
          break;
          case 'b':
               /* Choix du Nø de BAL pour la reception */
               simu_num_bal_defaut();
          break;

          case 'c' :
          printf("\nDonner la date");
           printf("\nAnnee :");
           scanf_s("%hd",&date.annee);
           printf("\nMois :");
           scanf_s("%hhd",&date.mois);
           printf("\nJour :");
           scanf_s("%hhd",&date.jour);
           res = HRDDateCoherence(date);
           if (res == 0)
           {
              printf("date non coherente\n");
           }
           else
           {
              printf ("date coherente\n");
           }


          break;

          case 'd' :
           printf("\nDonner l'heure");
           printf("\nHeure :");
           scanf_s("%hhd",&temps.heure);
           printf("\nMinute :");
           scanf_s("%hhd",&temps.minute);
           printf("\nSeconde :");
           scanf_s("%hhd",&temps.seconde);
           res = HRDHeureCoherence(temps);
           if (res == 0)
           {
              printf("heure non coherente\n");
           }
           else
           {
              printf ("heure coherente\n");
           }

          break;

          case 'e':
          simu_envoi_date_heure(M_HRD_GESTION_DATE_HEURE,
                                 100,
                                 message_date_heure);

          simu_envoi_hrd(400,5);

          break;

          case 'q':
               /* clear screen */
//               syst_ecrire_trace ("\x1B[2J");
               syst_ecrire_trace("envoi demande d'arret\n");
               fin = 1 ;
               simu_envoi_hrd(M_HRD_ARRET,HRD_DEMANDE) ;
          break ;

          default:
          break;

        } /* switch */

        if ( fin == 0 )
        {
          /*Affichage de la date et de l'heure de horodate*/
          heure = HRDDonneHeure();
          date = HRDDonneDate();
          printf("date : %02d / %02d / %04d\n", date.jour,date.mois,date.annee);
          printf("datehex : %02x / %02x / %04x\n", date.jour,date.mois,date.annee);
          printf("heurehex : %02x : %02x : %02x\n",heure.heure,heure.minute,heure.seconde);
          printf("heure : %02d : %02d : %02d\n",heure.heure,heure.minute,heure.seconde);

          secondes_depuis_80 = HRDSecondesDepuis80(date,heure);
          printf("secondes depuis 80 : %ld\n",secondes_depuis_80);

          printf ("\tLA BOITE AUX LETTRES DE RECEPTION PAR DEFAUT EST LA BOITE '%c'\n", boite ) ;
          syst_ecrire_trace ("\t a : MESSAGES DE SERVICES VERS HRD\n");
          syst_ecrire_trace ("\t b : CHANGEMENT BOITE AUX LETTRES DE RECEPTION PAR DEFAUT\n");
          syst_ecrire_trace ("\t c : Coherence date\n");
          syst_ecrire_trace ("\t d : Coherence heure\n");
          syst_ecrire_trace ("\t e : Envoi de cas d'erreur\n\n");
        }

     }/* touche !=0 */
     else
     {
        //DelaiTache(20);
//        for (cpt_bal = simu_bl_A ; cpt_bal <= simu_bl_D ; cpt_bal++ )
        {
         /* On attend un message dans 1 des 4 BAL */
         code_rtc = TestRecoit (simu_bl_A, (struct_neutre **)&p_neutre);


         if (code_rtc == NOYAU_BAL_MESS)
         {
           /* R‚cup‚ration du message */
           p_simu_message = (struct_hrd_message *)(p_neutre);

//           if ( cpt_bal == simu_bl_A )
              bal_recu = 'A' ;
/*
           else
           if ( cpt_bal == simu_bl_B )
              bal_recu = 'B' ;
           else
           if ( cpt_bal == simu_bl_C )
              bal_recu = 'C' ;
           else
           if ( cpt_bal == simu_bl_D )
              bal_recu = 'D' ;
           else
              bal_recu = '?' ;
*/

           printf("SIMU : RECEPTION D'UN MESSAGE DANS BAL '%c' depuis bal %d\n", bal_recu ,p_simu_message->entete.neutre.bl_id ) ;


           switch(p_simu_message->entete.service)
           {
              case M_HRD_ARRET:
                RecuArret(p_simu_message);
              break;

              case M_HRD_GESTION_ALARME:
                 RecuGestionAlarme(p_simu_message);
              break;

              case M_HRD_EVOL_HEURE:
                 RecuEvolHeure(p_simu_message);
              break;

              case M_HRD_GESTION_DATE_HEURE:
                 RecuDateHeure(p_simu_message);
              break;

              default:
                 syst_ecrire_trace("service inconnu\n");
              break;
           }
           /* On libŠre la m‚moire */
           Libere (&p_neutre);
         }/* code RTC */

       } /* for  */
     } /* else touche */
     FinRegion();
   } /*while*/

  syst_ecrire_trace("SIMU : Attente compte rendu Terminaison\n") ;

  etat_logiciel = LOGICIEL_FINI;
  syst_ecrire_trace("SIMU : FIN TACHE\n") ;

  HRDFinTrace();
  if (trace != NULL)
     fclose(trace);

  /* Terminaison de la tƒche */
  Termine();

  return 0;
}

/*****************************************************************************
* Fonction  : SimuLance
*-----------------------------------------------------------------------------
* Parametres: short int   pool_id : Nø du pool … utiliser
*-----------------------------------------------------------------------------
*  Action   : Lance la tƒche SIMU
*-----------------------------------------------------------------------------
* Retour    : G_OK
*****************************************************************************/

short int SimuLance(short int priorite_max )
{
    short int retour = 0;
    short int nb_tache ;

    /* Initialisation de la priorite des taches */
    for ( nb_tache=0 ; nb_tache<SIMU_NB_TACHE ; nb_tache++)
       t_simu[nb_tache].noyau_priorite_tache = THREAD_PRIORITY_HIGHEST;

    /* Lancement de la tƒche SIMU */
    retour = LanceTache (t_simu);
    
    simu_hrd_bl = AttendBAL ("HRD_0");
    simu_bl = AttendBAL ("SIMU_HRD_0");
    
    return( retour );
}

/*****************************************************************************
* Fonction  : SIMUArret
*-----------------------------------------------------------------------------
* Parametres: Aucun
*-----------------------------------------------------------------------------
*  Action   : Arrˆt la tƒche SIMU
*-----------------------------------------------------------------------------
* Retour    : G_OK si OK
*****************************************************************************/

short int SimuArret(void)
{
    short int retour = 0;

    SupprimeBAL ("SIMU_HRD_0");

    /* Arrˆt des taches du module TCPIP */
    ArretTaches (t_simu);

    return retour;
}
