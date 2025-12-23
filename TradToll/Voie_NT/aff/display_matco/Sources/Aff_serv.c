/*------   (v) 1997 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_SERV.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Aff/AFF/SOURCES/Aff_serv.c_v  $
 * 
 *    Rev 1.3   Jul 19 2001 16:56:02   sbatiot
 *  
 * 
 *    Rev 1.2   Oct 06 2000 17:33:14   CL
 * Correction service AFFICHAGE / TYP_TEST
 * Lignes d'affichage de test séparées par 0x0A, 
 * la dernière ligne est terminée par ce caractère
 * Prise en compte NOY 1.2.3
 * 
 *    Rev 1.1   Mar 31 2000 14:47:06   CL
 * Augmentation de la taille du buffer d'entree et du nombre de lignes gerees dans ANI
 * 
 *    Rev 1.3   Nov 17 1999 10:54:44   nbl
 *  
 * 
 *    Rev 1.2   Nov 15 1999 09:37:30   nbl
 *  
 * 
 *    Rev 1.1   Oct 29 1999 19:28:40   nbl
 *  
 * 
 *    Rev 1.1   Dec 21 1998 13:56:44   nbl
 *  
 * 
 *    Rev 1.3   09 Apr 1998 11:32:48   HMO
 * - Correction init des services
 * - Ajout du nettoyage des services a l'arret
 * 
 *    Rev 1.2   06 Apr 1998 10:44:48   HMO
 *  
 * 
 *    Rev 1.1   02 Apr 1998 10:52:28   HMO
 *  
 * 
 *    Rev 1.0   Mar 27 1998 11:13:20   pgg
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 *
 * Renamed from SGM to AFF 21.07.1999. sasa - ecsat
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <string.h>

#include <noyau.h>
#include <debug.h>
#include <horodate.h>
#include <reg.h>
#include <fic_gere.h>

#include <aff_ext.h>
#include <aff_util.h>
#include <aff_glob.h>
#include <aff_ani.h>
#include <aff_serv.h>
#include <aff_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
PRIVATE void AFFTestAffichage( short int             ident_aff,
                                 struct_aff_message *p_message,
                                 aff_infos           *buffer);
/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFReceptionAffichage( short int ident_aff,
*                                       struct_aff_message *p_msg_recu)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : pointeur sur le message recu de l'application
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant l'affichage d'un message
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionAffichage( short int ident_aff,
                                      struct_aff_message *p_msg_recu)
{
   enum_aff_valide   retour;
   struct_b_etat_aff *p_etat;
   long              ligne;
   aff_infos         data_info;

   
   p_etat = (struct_b_etat_aff *)&AFF[ident_aff].gestion_peripherique.b_etat;

   // suivant le type de service demande 
   switch(p_msg_recu->entete.type_message)
   {
      case SRV_TYP_DEBUT:
         // redirection trace 
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Debut service AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);

         // recherche si service disponible et acquittement 
          retour = AFFDebutService( ident_aff,
                                    M_AFF_AFFICHAGE,
                                    p_msg_recu->entete.neutre.bl_retour);
         // si le service est libre, envoie d'un message vers IOS 
         if (retour == AFF_VALIDE)
         {
            // pour un pooling plus rapide sur l'etat du AFF 
            AFF[ident_aff].timeout_bal = AFF_TIMEOUT_RAPIDE;
         }
         break;

      case SRV_TYP_FIN:
         // redirection trace 
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Fin service AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);

         retour = AFFFinService( ident_aff,
                                 M_AFF_AFFICHAGE,
                                 p_msg_recu->entete.neutre.bl_retour);
         // si le service est desactive 
         if ( retour == AFF_VALIDE)
         {
            // pour un pooling plus lent sur l'etat du AFF 
            AFF[ident_aff].timeout_bal = AFF_TIMEOUT_LENT;
         }
         break;

      case SRV_TYP_DEMANDE :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Demande AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);


         // Donn‚es calcul‚es avec la pr‚cision tarif 
         if( AFF_CHAMP_DONNEE_LONG(p_msg_recu->u.srv_affichage.demande.precision_tarif) == 0)
         {
            // Erreur de pr‚cision tarif 
            AffFichierTrace( ident_aff, "AFF_ANI *** Service AFFICHAGE => precision tarif ne doit pas etre nul ***");
            ExitBad();
         }

         // Convertion du message 
         AFF_DataConversion( &data_info, &p_msg_recu->u.srv_affichage.demande);

		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		 {
               // envoi d'un acquittement a l'application 
               AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_DEMANDE_ACQ);

               // Si l'afficheur est pret, on peut envoyer
               if( p_etat->aff_hs == FALSE)
               {
                  AFFGestionAffichage( ident_aff, p_msg_recu, &data_info);
				  AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_EFFECTUE);
               }

         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  SRV_TYP_DEMANDE_NACQ);
         }
         break;

	  case SRV_TYP_TEST :

			AffFichierTrace( ident_aff,
				  "AFF_ANI: Bal '%d', Test AFFICHAGE ",
				  p_msg_recu->entete.neutre.bl_retour);

			if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
			{
			   // envoi d'un acquittement a l'application 
			   AFFEnvoiAcquittement( ident_aff,
									 p_msg_recu->entete.neutre.bl_retour,
									 M_AFF_AFFICHAGE,
									 SRV_TYP_TEST_ACQ);

			   // Si l'afficheur est pret, on peut envoyer
			   if( p_etat->aff_hs == FALSE)
			   {
				  //AFFGestionAffichage( ident_aff, p_msg_recu, &data_info);
				   AFFTestAffichage(ident_aff, p_msg_recu, &data_info);
				   AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_EFFECTUE);
			   }

			}
			else
			{
			// envoi d'un non acquittement 
			AFFEnvoiAcquittement( ident_aff,
								  p_msg_recu->entete.neutre.bl_retour,
								  M_AFF_AFFICHAGE,
								  SRV_TYP_TEST_NACQ);
			}

			break;

      case AFF_TYP_RESET :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Reset AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);
         
		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		 {
               // envoi d'un acquittement a l'application 
               AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     AFF_TYP_RESET_ACQ);

               // Si l'afficheur est pret, on peut envoyer
               if( p_etat->aff_hs == FALSE)
               {
                  AFFGestionReset( ident_aff, p_msg_recu);
				  AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_EFFECTUE);
               }

         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_RESET_NACQ);
         }
         break;

	 case AFF_TYP_RESET_HW :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Reset HW AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);
         
		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		 {
               // envoi d'un acquittement a l'application 
               AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     AFF_TYP_RESET_HW_ACQ);

               // Si l'afficheur est pret, on peut envoyer
               if( p_etat->aff_hs == FALSE)
               {
                  AFFEnvoiIos( ident_aff, M_RESET_HW, NULL);
				  AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_EFFECTUE);
               }

         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_RESET_HW_NACQ);
         }
         break;

      case AFF_TYP_ALLUME :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Eclairage AFFICHAGE ",
                          p_msg_recu->entete.neutre.bl_retour);

		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
         {
               // envoi d'un acquittement a l'application 
               AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     AFF_TYP_ALLUME_ACQ);

               // Si l'afficheur est pret, on peut envoyer
               if( p_etat->aff_hs == FALSE)
               {
                  AFFGestionDimming( ident_aff, p_msg_recu);
				  AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_AFFICHAGE,
                                     SRV_TYP_EFFECTUE);
               }

         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_ALLUME_NACQ);
         }
         break;
	/******************MSU*****************/
      case AFF_TYP_NEW_FILE :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', AFFICHAGE - AFF_TYP_NEW_FILE => %s",
                          p_msg_recu->entete.neutre.bl_retour,
						  p_msg_recu->u.srv_affichage.newFile.FilePath);

		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
         {
           // envoi d'un acquittement a l'application 
           AFFEnvoiAcquittement( ident_aff,
                                 p_msg_recu->entete.neutre.bl_retour,
                                 M_AFF_AFFICHAGE,
                                 AFF_TYP_NEW_FILE_ACQ);
				AFF[ident_aff].position = 1;

			if( AFF_ReadFile( p_msg_recu->u.srv_affichage.newFile.FilePath,
				AFF[ident_aff].tab_label,
				NB_MAX_LABEL,
				&ligne,
				&AFF[ident_aff].position) == FALSE)
			{
				FreeTabLabel( AFF[ident_aff].tab_label, NB_MAX_LABEL);
				AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Error in the file %s, line %ld *****",
					p_msg_recu->u.srv_affichage.newFile.FilePath,
					ligne);
			    //return( INST_INIT_ERR_FICHIER_PARAM );
				AFF[ident_aff].position = 1;
				if( AFF_ReadFile( AFF[ident_aff].pcFichierFormat,
				AFF[ident_aff].tab_label,
				NB_MAX_LABEL,
				&ligne,
				&AFF[ident_aff].position) == FALSE)
				{
					FreeTabLabel( AFF[ident_aff].tab_label, NB_MAX_LABEL);
					AffFichierDebug(ident_aff,"AFF_INIT ***** AffLance() => Error in the file %s, line %ld *****",
						AFF[ident_aff].pcFichierFormat,
						ligne);
				  //return( INST_INIT_ERR_FICHIER_PARAM );
				}
			}
			else
			{
				AffFichierTrace(ident_aff,"AFFReceptionAffichage() - New Ped File: %s => %s",
					p_msg_recu->u.srv_affichage.newFile.FilePath,
					AFF[ident_aff].pcKey);

				FinRegionLocale();
				DelaiTache(2);
				DebutRegionLocale();

				REG_Ecrire_Chaine( CSR_REG_KEYi_ROOT, AFF[ident_aff].pcKey, AFF_REG_KEYv_FICHIER_FORMAT, p_msg_recu->u.srv_affichage.newFile.FilePath);
			}
		 }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_NEW_FILE_NACQ);
         }
         break;

	  case AFF_TYP_VOYANT:
         if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
         {
             // envoi d'un acquittement a l'application 
             AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_VOYANT_ACQ);

			 if( p_etat->aff_hs == FALSE)
             {
                  AFFEnvoiIos(ident_aff, M_VOYANT, &p_msg_recu->u.srv_affichage.voyant);
             }
		 }
		 else
		 {
			 AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  AFF_TYP_VOYANT_NACQ);
		 }
		 break;
	/**************************************/			
      default:
         AffFichierDebug( ident_aff, "AFF_ANI *** Service AFFICHAGE => type '%d' inconnu ***",
                          p_msg_recu->entete.type_message);
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFReceptionVisu( short int ident_aff,
*                                  struct_aff_message *p_msg_recu)
* PARAMETRES:
*     entree: identificateur de l'afficheur
*           : pointeur sur le message recu de l'application
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant l'affichage d'un message
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionVisu( short int ident_aff,
                                 struct_aff_message *p_msg_recu)
{
   enum_aff_valide   retour;
   struct_b_etat_aff *p_etat;

//J   aff_inf           data_info;

   p_etat = (struct_b_etat_aff *)&AFF[ident_aff].gestion_peripherique.b_etat;

   // suivant le type de service demande 
   switch(p_msg_recu->entete.type_message)
   {
      case SRV_TYP_DEBUT:
         // redirection trace 
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Debut service VISU ",
                          p_msg_recu->entete.neutre.bl_retour);

         // recherche si service disponible et acquittement 
          retour = AFFDebutService( ident_aff,
                                    M_AFF_VISU,
                                    p_msg_recu->entete.neutre.bl_retour);
         break;

      case SRV_TYP_FIN:
         // redirection trace 
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Fin service VISU ",
                          p_msg_recu->entete.neutre.bl_retour);

         retour = AFFFinService( ident_aff,
                                 M_AFF_VISU,
                                 p_msg_recu->entete.neutre.bl_retour);
         break;

      case SRV_TYP_DEMANDE :
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Demande VISU ",
                          p_msg_recu->entete.neutre.bl_retour);

		 if (SrvEstDemandeur (AFF[ident_aff].service[M_AFF_VISU], p_msg_recu->entete.neutre.bl_retour) == TRUE)
         {
               // envoi d'un acquittement a l'application 
               AFFEnvoiAcquittement( ident_aff,
                                     p_msg_recu->entete.neutre.bl_retour,
                                     M_AFF_VISU,
                                     SRV_TYP_DEMANDE_ACQ);

               // Si l'afficheur est pret, on peut envoyer
               if( p_etat->aff_hs == FALSE)
               {
                  AFFEnvoiVisu( ident_aff,
                                p_msg_recu->entete.neutre.bl_retour,
                                SRV_TYP_DEMANDE);
               }

         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_AFF_AFFICHAGE,
                                  SRV_TYP_DEMANDE_NACQ);
         }
         break;

      default:
         AffFichierDebug( ident_aff, "AFF_ANI *** Service AFFICHAGE => type '%d' inconnu ***",
                          p_msg_recu->entete.type_message);
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFReceptionEtat(short int ident_aff,
*                                 struct_aff_message *p_msg_recu)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : pointeur sur le message recu de l'application
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant les demandes d'etat
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionEtat(short int ident_aff,struct_aff_message *p_msg_recu)
{
   enum_aff_valide retour;

   // suivant le type de message recu 
   switch( p_msg_recu->entete.type_message)
   {
      // debut du service 
      case SRV_TYP_DEBUT:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Debut service ETAT ",
                          p_msg_recu->entete.neutre.bl_retour);
         // recherche si service disponible et acquittement 
         retour = AFFDebutService( ident_aff,
                                   M_SRV_ETAT,
                                   p_msg_recu->entete.neutre.bl_retour);
         if (retour == AFF_VALIDE)
         {
            // envoi d'un message d'etat 
            AFFEnvoiEtat( ident_aff,
                          p_msg_recu->entete.neutre.bl_retour,
                          SRV_TYP_DEMANDE);
         }
         break;

      // fin du service 
      case SRV_TYP_FIN:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Fin service ETAT ",
                          p_msg_recu->entete.neutre.bl_retour);
         // envoi d'un acquittement de fin de service 
         AFFFinService( ident_aff,
                        M_SRV_ETAT,
                        p_msg_recu->entete.neutre.bl_retour);

         break;

      case SRV_TYP_DEMANDE:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Demande ETAT ",
                          p_msg_recu->entete.neutre.bl_retour);
		 if (SrvEstDemandeur (AFF[ident_aff].service[M_SRV_ETAT], p_msg_recu->entete.neutre.bl_retour) == TRUE)
         {
            // envoi d'un acquittement a l'application 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_SRV_ETAT,
                                  SRV_TYP_DEMANDE_ACQ);

            // envoi d'un message d'etat 
            AFFEnvoiEtat( ident_aff,
                          p_msg_recu->entete.neutre.bl_retour,
                          SRV_TYP_DEMANDE);
         }
         else
         {
            // envoi d'un non acquittement 
            AFFEnvoiAcquittement( ident_aff,
                                  p_msg_recu->entete.neutre.bl_retour,
                                  M_SRV_ETAT,
                                  SRV_TYP_DEMANDE_NACQ);
         }
         break;

      default:
         // appel au fichier trace 
         AffFichierDebug( ident_aff, "AFF_ANI *** Service ETAT => type '%d' inconnu ***",
                          p_msg_recu->entete.type_message);
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFReceptionEspion( , short int ident_aff,
*                                 struct_aff_message *p_msg_recu) )
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : pointeur sur le message recu de l'application
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant le Dialogue Operateur
* --------------------------------------------------------------------
* $F_FCTN
(*/
PROTECTED void AFFReceptionEspion(short int ident_aff,struct_aff_message *p_msg_recu)
{
   enum_aff_valide retour;

   /* suivant le type de service recu */
   switch( p_msg_recu->entete.type_message)
   {
      case SRV_TYP_DEBUT:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Debut service ESPION ",
                          p_msg_recu->entete.neutre.bl_retour);
         /* test de la disponibilite du service et acquittement */
         retour = AFFDebutServiceEspion( ident_aff,
									  p_msg_recu->entete.neutre.bl_retour,
                                      p_msg_recu->u.srv_espion.nature,
									  p_msg_recu->u.srv_espion.sens);

         break;

      case SRV_TYP_FIN:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', Fin service ESPION ",
                          p_msg_recu->entete.neutre.bl_retour);
         /* acquittement de la fin de service */
         retour = AFFFinService( ident_aff,
                                 M_SRV_ESPION,
                                 p_msg_recu->entete.neutre.bl_retour);
         break;

      default:
         AffFichierDebug( ident_aff, "AFF_ANI *** Service ESPION => type '%d' inconnu ***",
                          p_msg_recu->entete.type_message);
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFReceptionArret( short int ident_aff,
*                                  struct_aff_message *p_message)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : pointeur sur le message recu de l'application
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Traiter les messages concernant l'arret du module
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFReceptionArret(short int ident_aff,
                                 struct_aff_message *p_msg_recu)
{
   /* suivant le type de service recu */
   switch( p_msg_recu->entete.type_message)
   {
      case SRV_TYP_DEMANDE:
         AffFichierTrace( ident_aff,
                          "AFF_ANI: Bal '%d', demande ARRET ",
                          p_msg_recu->entete.neutre.bl_retour);
		 
		if (AFF[ident_aff].arret_bal == -1) 
		{
			AFF[ident_aff].arret_bal=p_msg_recu->entete.neutre.bl_retour;

			// La demande d'arret est prise en compte
			AFFEnvoiAcquittement(ident_aff,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_SRV_ARRET,
								 SRV_TYP_ARRET_ACQ );

			/* demande d'arret de la tache ios */
			AFFEnvoiIos( ident_aff, M_ARRET, NULL);
		}
		else
			AFFEnvoiAcquittement(ident_aff,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_SRV_ARRET,
								 SRV_TYP_ARRET_NACQ );

         /* la tache ANI attend le compte-rendu d'arret de IOS */
         /* pour s'arreter aussi */
         break;

      default:
         /* appel du fichier trace */
         AffFichierDebug( ident_aff, "AFF_ANI *** Service ARRET => type '%d' inconnu, bal emettrice '%d', Bal '%d' ***",
                          p_msg_recu->entete.type_message,
                          p_msg_recu->entete.neutre.bl_retour,
                          p_msg_recu->entete.neutre.bl_retour );
         break;
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: AFFGestionAffichage( short int          ident_aff,
*                               struct_aff_message *p_message,
*                               aff_infos           *buffer)
* PARAMETRES: identificateur de l'imprimante
*             buffer contenant la derniŠre commande enregistr‚e
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale au fichier
* ROLE: Envoi de la derniŠre commande du buffer d'impression :
*       entete ou corps
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFGestionAffichage( short int          ident_aff,
                                    struct_aff_message *p_message,
                                    aff_infos           *buffer)
{
   int index;
   int priorite = p_message->u.srv_affichage.demande.priorite;

   AffFichierTrace( ident_aff, "AFF_ANI: affichage du label : '%s', priorite '%d' ",
                    p_message->u.srv_affichage.demande.label,
                    priorite);

   if( priorite < 0 || priorite >= NB_MAX_AFF_PRIO)
   {
      /* La priorite n'est pas possible */
      AffFichierDebug( ident_aff, "AFF_ANI *** Service AFFICHAGE => la priorite '%d' est impossible ***",
                       priorite);
      return;
   }

   /* Si le message est autoris‚ */
   if( AFF[ident_aff].sScreen.tab_request[priorite].bal == NO_BAL ||
       AFF[ident_aff].sScreen.tab_request[priorite].bal == p_message->entete.neutre.bl_retour)
   {
      /* Recherche du label demand‚ */
      for( index = 0; index < NB_MAX_LABEL; index++)
      {
         if( AFF[ident_aff].tab_label[ index].type == MSG_LABEL &&
             strcmp( AFF[ident_aff].tab_label[ index].label,
                     p_message->u.srv_affichage.demande.label) == 0)
            break;
      }

      /* Le label a ‚t‚ trouv‚ */
      if( AFF[ident_aff].tab_label[ index].type == MSG_LABEL)
      {
         AFF[ident_aff].sScreen.tab_request[priorite].bal = p_message->entete.neutre.bl_retour;

         /* Construire les nouvelles lignes */
         AFF_CreateLine(AFF[ident_aff].char_conv_array,
                         &AFF[ident_aff].sScreen.tab_request[priorite],
                         &AFF[ident_aff].tab_label[index],
                         buffer);

		 AFF[ident_aff].tab_credit[priorite]=buffer->infos.sale_fare_int.my_data.lData;
		 AFF[ident_aff].tab_paid[priorite]=buffer->infos.fare_int.my_data.lData;
		 AffFichierTrace( ident_aff, "paid %lu credit_serv  %lu ",AFF[ident_aff].tab_paid[priorite],AFF[ident_aff].tab_credit[priorite]);

         /* Envoyer les nouvelles lignes … IOS */
         AFFEnvoiIos( ident_aff, M_AFFICHAGE, (void *)priorite);
      }
      else
      {
         /* Le label n'a pas ‚t‚ trouv‚ */
         AffFichierTrace( ident_aff, "AFF_ANI *** Service AFFICHAGE => le label '%s' n'existe pas ***",
                          p_message->u.srv_affichage.demande.label);
      }
   }
   else
   {
      /* Le message n'est pas autoris‚ */
      AffFichierTrace( ident_aff, "AFF_ANI *** Service AFFICHAGE => la priorite '%d' est deja utilisee par la Bal '%d' ***",
                       priorite,
                       AFF[ident_aff].sScreen.tab_request[priorite].bal);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: AFFTestAffichage( short int          ident_aff,
*                            struct_aff_message *p_message,
*                            aff_infos           *buffer)
* PARAMETRES: identificateur de l'imprimante
*             buffer contenant la derniŠre commande enregistr‚e
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale au fichier
* ROLE: Envoi d'un message à afficher
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void AFFTestAffichage( short int          ident_aff,
                                 struct_aff_message *p_message,
                                 aff_infos           *buffer)
{
   char		*pSrch = NULL, *pligne1 = NULL, *pligne2 = NULL;
   int		priorite = p_message->u.srv_test.priorite;
   int		i;
   struct_list_text *pElmt = NULL;

   AffFichierTrace( ident_aff, "AFF_ANI: affichage d'un message de test : '%s', priorite '%d' ",
                    p_message->u.srv_test.ligne,
                    priorite);

   if( priorite < 0 || priorite >= NB_MAX_AFF_PRIO)
   {
      /* La priorite n'est pas possible */
      AffFichierDebug( ident_aff, "AFF_ANI *** Service AFFICHAGE => la priorite '%d' est impossible ***",
                       priorite);
      return;
   }

   /* Si le message est autoris‚ */
   if( AFF[ident_aff].sScreen.tab_request[priorite].bal == NO_BAL ||
       AFF[ident_aff].sScreen.tab_request[priorite].bal == p_message->entete.neutre.bl_retour)
   {
     AFF[ident_aff].sScreen.tab_request[priorite].bal = p_message->entete.neutre.bl_retour;

	 AFF[ident_aff].sScreen.tab_request[priorite].p_label = NULL;

	 AFF[ident_aff].sScreen.tab_request[priorite].texte = malloc(sizeof(struct_list_text));
	 memset(AFF[ident_aff].sScreen.tab_request[priorite].texte, 0, sizeof(struct_list_text));

	 // Allocation d'une liste chainee de lignes a afficher
	 pElmt = AFF[ident_aff].sScreen.tab_request[priorite].texte;
	 i = 1;
	 while(i<AFF_MAX_LIGNES){
		 pElmt->suiv_valide = TRUE;
		 pElmt->suiv = (struct_list_text *)malloc(sizeof(struct_list_text));
		 if (pElmt->suiv == NULL)
		 { /* défaut mémoire */
		     AffFichierDebug( ident_aff, "AFF_ANI *** malloc failed in line AFFTestAffichage ***") ;
		     return ;
		 }

		 memset(pElmt->suiv, 0, sizeof(struct_list_text));
		 pElmt = pElmt->suiv;
		 i++;
	 }

	 pElmt->suiv_valide = FALSE;
	 pElmt->suiv = NULL;

	 // A chaque nouvelle ligne détectée par le caractère 0x0A (Line Feed),
	 // on remplit une nouvelle structure.
	 pElmt = AFF[ident_aff].sScreen.tab_request[priorite].texte;
	 pligne1 = p_message->u.srv_test.ligne;
	 pSrch = strchr(p_message->u.srv_test.ligne, 0x0A);
	 while((pSrch != NULL) && (pElmt != NULL)){

		 *pSrch = '\0';
		 strcpy_s(pElmt->texte, sizeof(pElmt->texte), pligne1);
		 pElmt->police = 'A';
		 pligne1 = pSrch + 1;
		 pSrch = strchr(pSrch + 1, 0x0A);
		 pElmt = pElmt->suiv;
	 }

     /* Envoyer les nouvelles lignes … IOS */
     AFFEnvoiIos( ident_aff, M_AFFICHAGE, (void *)priorite);
   }
   else
   {
      /* Le message n'est pas autoris‚ */
      AffFichierTrace( ident_aff, "AFF_ANI *** Service AFFICHAGE => la priorite '%d' est deja utilisee par la Bal '%d' ***",
                       priorite,
                       AFF[ident_aff].sScreen.tab_request[priorite].bal);
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: AFFGestionReset( short int          ident_aff,
*                           struct_aff_message *p_message)
* PARAMETRES: identificateur de l'afficheur
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale au fichier
* ROLE: Envoi de la derniŠre commande du buffer d'impression :
*       entete ou corps
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFGestionReset( short int          ident_aff,
                                struct_aff_message *p_message)
{
   int priorite = p_message->u.srv_affichage.reset.priorite;

   AffFichierTrace( ident_aff,
                    "AFF_ANI: reset de la priorite : '%d' ",
                    priorite);

   if( priorite < 0 || priorite >= NB_MAX_AFF_PRIO)
   {
      /* La priorite n'est pas possible */
      AffFichierTrace( ident_aff,
                       "AFF_ANI *** Service RESET => la priorite '%d' est impossible ***",
                       priorite);
      return;
   }

   /* Si le reset est autoris‚ */
   if( AFF[ident_aff].sScreen.tab_request[priorite].bal == p_message->entete.neutre.bl_retour)
   {
      AFF[ident_aff].sScreen.tab_request[priorite].bal = NO_BAL;

      /* Envoyer les nouvelles lignes … IOS */
      AFFEnvoiIos( ident_aff, M_AFFICHAGE, (void *)priorite);
   }
   else
   {
      if( AFF[ident_aff].sScreen.tab_request[priorite].bal == NO_BAL)
      {
         /* La priorite n'est pas disponible */
         AffFichierTrace( ident_aff,
                          "AFF_ANI *** Service RESET => la priorite '%d' n'est pas utilis‚e ***",
                          priorite);
      }
      else
      {
         /* La priorite est d‚ja utilis‚e */
         AffFichierTrace( ident_aff, "AFF_ANI *** Service RESET => la priorite '%d' est d‚ja utilis‚e par la BAL %d ***",
                          priorite,
                          AFF[ident_aff].sScreen.tab_request[priorite].bal);
      }
   }
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: AFFGestionDimming( short int          ident_aff,
*                             struct_aff_message *p_message)
* PARAMETRES: identificateur de l'afficheur
* RETOUR: aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale au fichier
* ROLE: Envoi de la derniŠre commande du buffer d'impression :
*       entete ou corps
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFGestionDimming( short int          ident_aff,
                                  struct_aff_message *p_message)
{
   boolean etat = p_message->u.srv_affichage.allume.etat;

   if( etat == TRUE)
   {
      AffFichierTrace( ident_aff, "AFF_ANI: retroeclairage de l'afficheur allume ");
   }
   else
   {
      AffFichierTrace( ident_aff, "AFF_ANI: retroeclairage de l'afficheur eteind ");
   }

   /* Envoyer les nouvelles lignes … IOS */
   AFFEnvoiIos( ident_aff, M_ALLUME, (void *)etat);
}

/**/
/*******************************************************************/
/*SYNTAX: void AFFReceptionDeviceInfo(short int ident_aff,		   */
/*									 struct_aff_message *p_message)*/
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function processes the DEVICE INFO service	   */
/*			  messages.											   */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      IN dcp_inst_id iInstanceIdx         - Instance ID.         */
/*      IN struct_dcp_message *p_msg_rec    - Pointer to the       */
/*                                            message structure.   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED void AFFReceptionDeviceInfo(short int ident_aff,
									struct_aff_message *p_message)
{
	// Message type
	switch(p_message->entete.type_message)
	{
		case SRV_TYP_DEBUT:
			AffFichierTrace( ident_aff,
							"AFF_ANI: Bal '%d', Debut service DEVICE INFO ",
							p_message->entete.neutre.bl_retour);

		
			AFFDebutService(ident_aff,
							M_SRV_DEVICE_INFO,
							p_message->entete.neutre.bl_retour);
			break;

		case SRV_TYP_FIN:
			AffFichierTrace(ident_aff,
							"AFF_ANI: Bal '%d', Fin service DEVICE INFO ",
							p_message->entete.neutre.bl_retour);
		
			AFFFinService( ident_aff, M_SRV_DEVICE_INFO, p_message->entete.neutre.bl_retour);
			break;
		
		case SRV_TYP_GET:
			AffFichierTrace(ident_aff,
							"AFF_ANI: Bal '%d', demande DEVICE INFO - SRV_TYP_GET ",
							p_message->entete.neutre.bl_retour);
		
			if ( SrvEstDemandeur( AFF[ident_aff].service[M_SRV_DEVICE_INFO], 
				 p_message->entete.neutre.bl_retour) == TRUE )
			{				
				AFFEnvoiAcquittement(ident_aff,
									p_message->entete.neutre.bl_retour,
									M_SRV_DEVICE_INFO,
									SRV_TYP_GET_ACQ);

				AFFEnvoiIos(ident_aff, SRV_TYP_GET, p_message);
			}
			else
			{
				AFFEnvoiAcquittement( ident_aff,
					p_message->entete.neutre.bl_retour,
					M_SRV_DEVICE_INFO,
					SRV_TYP_GET_NACQ);
			}
			break;
	}
}
/*--------------------------- END FILE ----------------------------*/