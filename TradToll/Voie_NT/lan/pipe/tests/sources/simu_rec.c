/*------   (v) 1997 CS-Route  -----------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: SIMULATEUR
* FICHIER: SIMU0.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
 * $Log:   T:/MODULO/VoieNt/Lan/Pipe/test/simu_rec.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:50:20   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:18   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.1   02 Oct 1998 11:46:12   bph
 *  
 * 
 *    Rev 1.0   Aug 04 1997 17:35:22   ANA
 * Transfert de fichiers avec renommage
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <conio.h>

/* module noyau */
#include <noyau.h>
//#include <console.h>

#include <csr_lan.h>

#include <simu.h>
#include <simu_glo.h>

#include <memclass.h>
/*------------------DEFINES:--------------------------*/


/*------------------TYPEDEF:--------------------------*/


/*--------------------FONCTIONS: ---------------*/

/*------------------VARIABLES:------------------------*/
PRIVATE boolean temoin_arret;
PROTECTED enum_etat_liaison etat_connexion;
PROTECTED long num_msg;
PROTECTED char emission_possible;
PROTECTED char fichier_possible;

/*-------------------CODE:----------------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void simu_recoit()
* PARAMETRES:
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/

PROTECTED DWORD WINAPI SimuRecoit (void *param)
{
   struct_neutre   *p_neutre;
   noyau_enum_retour code_rtc;
   noyau_bal_id     cpt_bal ;

   DebutRegion();

   temoin_arret = FALSE;
   /* Changement de la priorite de la tache  */
   ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;

   /* Boite aux lettres par defaut*/
   SIMU.boite = 'A' ;

   FinRegion();
   
   cpt_bal = AttendBAL ("SIMU_LAN_0");
   
   /* traitement */
   while ( TRUE )
   {
     code_rtc = Recoit(cpt_bal, &p_neutre, 5);
     
     DebutRegion();

     if( code_rtc == NOYAU_BAL_MESS )
     {
        /* fenetre de reception des messages */
        //cadre( 5, 15, 75, 24);
		// system("cls");
        //textcolor(YELLOW);
        gotoxy(2,2);
        cprintf(" RECEPTION de bal %d\n\r",p_neutre->bl_id ) ;
        AfficheReception(p_neutre);

        /* On libŠre la m‚moire */
        if( Libere (&p_neutre) != NOYAU_OK )
           cprintf("SIMU : PB Libere() \n");

     }
     
     FinRegion();
     
     if( SIMU.flag_fin == TRUE )
     {
         /* Terminaison de la tƒche */
         etat_logiciel = LOGICIEL_FINI;

         Termine();
     }
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AfficheReceptiont(struct_neutre *p_neutre)
* PARAMETRES: Pointeur sur la structure neutre du message recu
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche les messages recus du module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AfficheReception( struct_neutre *p_neutre)
{
   int i;
   struct_lan_message *p_simu_message = (struct_lan_message *)(0);
   struct_lan_gestion_fic *p_param;
   struct_lan_message_externe msg_lan;

   p_simu_message = (struct_lan_message *)p_neutre;

   switch( p_simu_message->entete.service)
   {
      case M_LAN_DOP :
      /* messages du DOP */
           if( p_simu_message->entete.type_message == LAN_MESSAGE_DOP)
           {
            if( p_simu_message->u.msg_dop.sens == SRV_ESPION_ENTRANT )
            {
               gotoxy(2,7);
               cprintf("DOP < : ");
            }
            else
            {
               gotoxy(2,6);
               cprintf("DOP > : ");
            }
            switch (p_simu_message->u.msg_dop.type )
            {
               case LAN_DOP_MSG:
                    for( i=0; i<p_simu_message->u.msg_dop.taille;i++)
                         cprintf("%c",p_simu_message->u.msg_dop.msg[i]);
               break;
               
               case LAN_DOP_VIE:
                   cprintf ("VIE");
               break;

               case LAN_DOP_FIC:
                    p_param = (struct_lan_gestion_fic *)p_simu_message->u.msg_dop.msg;
                    cprintf("%s",p_param->fichier);
                    cprintf("  %s",p_param->path);
                    cprintf("  %s",p_param->hostname);
               break;

               case LAN_DOP_TPS:
                    for( i=0; i<p_simu_message->u.msg_dop.taille;i++)
                    cprintf("%02X",p_simu_message->u.msg_dop.msg[i]);
               break;

               default:
                    gotoxy(4,4);
                    AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
               break;
            }
           }
      break;
      case M_LAN_MESSAGE:
      /* message recu par le reseau */
         if( p_simu_message->entete.type_message == LAN_RECEPTION)
         {
            gotoxy(4,4);
            cprintf("< %s\n",p_simu_message->u.message.contenu);
            //      gotoxy(3,5);
            //      for( i=0; i<p_simu_message->u.message.longueur; i++)
            //         cprintf("%x",p_simu_message->u.message.p_contenu[i]);
            EnvoiMessage (SIMU.bal_module);
         }
         else
         {
            gotoxy(4,4);
            AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
         }
      break;

      case M_LAN_HORAIRE :
         if( p_simu_message->entete.type_message == LAN_RECEPTION)
         {
            gotoxy(4,4);
            cprintf("Difference de temps recue : %ld secondes\n",p_simu_message->u.msg_tps.difference_tps );
         }
         else
         {
            gotoxy(4,4);
            AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
         }
      break;

      case M_LAN_ETAT:
         if( p_simu_message->entete.type_message == LAN_NOUVEL_ETAT)
         {
            etat_connexion = p_simu_message->u.msg_etat.serveur_msg;
//            if( etat_connexion == LAN_DEGRADE )
//                SimuEnvoiLan(SIMU.bal_module,M_LAN_VIDER_BAL,LAN_EMISSION,NULL);
            gotoxy(4,4);
            cprintf("ETAT TPS = %s",etat_serveur[p_simu_message->u.msg_etat.serveur_tps]);
            gotoxy(4,5);
            cprintf("ETAT FIC = %s",etat_serveur[p_simu_message->u.msg_etat.serveur_fic]);
            gotoxy(4,6);
            cprintf("ETAT LAN = %s\n",etat_serveur[p_simu_message->u.msg_etat.serveur_msg]);
         }
         else
         {
            gotoxy(4,4);
            AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
         }
      break;

      case M_LAN_FICHIER:
           switch( p_simu_message->entete.type_message)
           {
            case LAN_RECEPTION_EFFECTUEE :
                 gotoxy(4,4);
                 cprintf("Reception fichier : %s OK",p_simu_message->u.param_fic.fichier);

                 msg_lan.entete.service = M_LAN_FICHIER;
                 msg_lan.entete.type_message = LAN_RECEPTION;
                 msg_lan.u.param_fic.fichier = p_simu_message->u.param_fic.fichier;
                 msg_lan.u.param_fic.path = "C:\\TEMP";
                 msg_lan.u.param_fic.hostname = p_simu_message->u.param_fic.hostname;
                 
//                 EnvoiLan(SIMU.bal_module,SIMU.bal_id, POOL_SIMU, &msg_lan);
            break;

            case LAN_RECEPTION_NON_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Erreur Reception fichier : %s",p_simu_message->u.param_fic.fichier);
            break;

            case LAN_EMISSION_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Emission du fichier : %s OK    ",p_simu_message->u.param_fic.fichier);

                 msg_lan.entete.service = M_LAN_FICHIER;
                 msg_lan.entete.type_message = LAN_EMISSION;
                 msg_lan.u.param_fic.fichier = p_simu_message->u.param_fic.fichier;
                 msg_lan.u.param_fic.path = "C:\\TEMP\\SEND";
                 msg_lan.u.param_fic.hostname = p_simu_message->u.param_fic.hostname;

//                 EnvoiLan(SIMU.bal_module,SIMU.bal_id, POOL_SIMU, &msg_lan);
            break;

            case LAN_EMISSION_NON_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Erreur Emission du fichier : %s",p_simu_message->u.param_fic.fichier);
            break;

            case LAN_RECEPTION_RENOMME_EFFECTUEE :
                 gotoxy(4,4);
                 cprintf("Reception du fichier: %s OK    ",p_simu_message->u.param_fic.fichier);
                 gotoxy(4,5);
                 cprintf("Nouveau nom: %s ",p_simu_message->u.param_fic.path);
            break;

            case LAN_RECEPTION_RENOMME_NON_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Erreur Reception fichier renomme : %s",p_simu_message->u.param_fic.fichier);
            break;

            case LAN_EMISSION_RENOMME_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Emission fichier : %s OK",p_simu_message->u.param_fic.fichier);
                 gotoxy(4,5);
                 cprintf("Nouveau nom: %s ",p_simu_message->u.param_fic.path);
            break;

            case LAN_EMISSION_RENOMME_NON_EFFECTUEE:
                 gotoxy(4,4);
                 cprintf("Erreur Emission fichier renomme %s",p_simu_message->u.param_fic.fichier);
            break;

            default:
                 gotoxy(4,4);
                 AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
            break;
           }
      break;

      case M_LAN_ARRET:
            SIMU.flag_fin = TRUE;
      break;
                

      default:
              gotoxy(4,4);
              AfficheMessageRecu(p_simu_message->entete.service,
                           p_simu_message->entete.type_message);
      break;
   }

   //textbackground(BLACK);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void affiche_message_recu(enum_lan_service service,
*                                    enum_lan_type type_service)
* PARAMETRES: Service concern‚
*             Type du message recu
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche les messages emis par le module pour les demandes
*       sp‚cifiques aux services.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AfficheMessageRecu(enum_lan_service service,enum_lan_type type)
{
   switch(service)
   {
      case M_LAN_ARRET:
         switch(type)
         {
            case LAN_DEMANDE_ACQ:
               cprintf("acquittement ARRET tache\r");
               SIMU.flag_fin = TRUE;
               emission_possible = 0;
            break;

            default :
               cprintf("NON acquittement ARRET\r");
            break;
         }
      break;

      case M_LAN_CONNEXION:
         AfficheAcquittement("CONNEXION",type);
      break;

      case M_LAN_MESSAGE:
         switch(type)
         {
            case LAN_EMISSION_ACQ:
            //   cprintf("demande EMISSION MESSAGE acquitt‚e \r\n");
            break;

            case LAN_EMISSION_NACQ:
               /* decrementer le nbre de message car il n'a pas ete emis */
               num_msg --;
            //   cprintf("demande EMISSION MESSAGE non acquitt‚e\r\n");
            break;

            default:
            //   AfficheAcquittement("MESSAGE",type);
               if( type == LAN_DEBUT_ACQ )
                  emission_possible = 1;
               else if (type == LAN_FIN_ACQ )
                  emission_possible = 0;
            break;
         }
      break;

      case M_LAN_FICHIER:
         switch(type)
         {
            case LAN_EMISSION_ACQ:
               cprintf("EMISSION FICHIER acquitt‚e\r");
            break;

            case LAN_EMISSION_NACQ:
               cprintf("EMISSION FICHIER non acquitt‚e\r");
            break;

            case LAN_RECEPTION_ACQ:
               cprintf("RECEPTION FICHIER acquitt‚e\r");
            break;

            case LAN_RECEPTION_NACQ:
               cprintf("RECEPTION FICHIER non acquitt‚e\r");
            break;

            case LAN_EMISSION_RENOMME_ACQ:
               cprintf("EMISSION FICHIER avec RENOMMAGE acquitt‚e\r");
            break;

            case LAN_EMISSION_RENOMME_NACQ:
               cprintf("EMISSION FICHIER avec RENOMMAGE non acquitt‚e\r");
            break;

            case LAN_RECEPTION_RENOMME_ACQ:
               cprintf("RECEPTION FICHIER avec RENOMMAGE acquitt‚e\r");
            break;

            case LAN_RECEPTION_RENOMME_NACQ:
               cprintf("RECEPTION FICHIER avec RENOMMAGE non acquitt‚e\r");
            break;


            default:
               AfficheAcquittement("FICHIER",type);
               if( type == LAN_DEBUT_ACQ )
                  fichier_possible = 1;
               else if (type == LAN_FIN_ACQ )
                  fichier_possible = 0;
            break;
         }
      break;

      case M_LAN_HORAIRE:
         AfficheAcquittement("HORAIRE",type);
      break;

      case M_LAN_ETAT:
         AfficheAcquittement("ETAT",type);
      break;

      case M_LAN_DOP:
         AfficheAcquittement("DOP",type);
      break;

      default:
         cprintf("message inconnu\r");
      break;
   }

}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AfficheAcquittement(char *texte,enum_lan_type type_service)
* PARAMETRES: Message a afficher
*             Service concern‚
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Affiche le message d'acquittement emis par le module
*       pour les demandes de debut et fin de service uniquement
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AfficheAcquittement(char *texte, enum_lan_type type_service)
{
   switch( type_service)
   {
      case LAN_DEBUT_ACQ:
         cprintf("acquittement DEBUT %s",texte);
      break;
      case LAN_DEBUT_NACQ:
         cprintf("NON acquittement DEBUT %s",texte);
      break;

      case LAN_DEMANDE_ACQ:
         cprintf("acquittement DEMANDE %s",texte);
      break;
      case LAN_DEMANDE_NACQ:
         cprintf("NON acquittement DEMANDE %s",texte);
      break;


      case LAN_FIN_ACQ:
         cprintf("acquittement FIN %s",texte);
      break;
      case LAN_FIN_NACQ:
         cprintf("NON acquittement FIN %s",texte);
      break;
      default :
         cprintf("type message %s inconnu",texte);
      break;
   }
}
