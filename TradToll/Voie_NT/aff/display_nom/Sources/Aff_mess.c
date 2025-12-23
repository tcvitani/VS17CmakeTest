/************** (v) 2016 EMOVIS - All rights reserved ***************/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: Afficheur ext‚rieur
* FICHIER: AFF_MESS.C
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Code des fonctions traitant les types de messages recus par
*         la tache AFF_ANI
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/

#include <noyau.h>
#include <debug.h>
#include <fic_gere.h>

#include <aff_ext.h>
#include <aff_util.h>
#include <aff_glob.h>
#include <aff_serv.h>
#include <aff_mess.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/

PRIVATE BOOL WINAPI EnvoiDemandeurEspion (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur);
PRIVATE BOOL WINAPI EnvoiDemandeur (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur);

/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_aff_valide AFFDebutService ( short int ident_aff,
*                                            enum_aff_ident service_id,
*                                            noyau_bal_id bal_dest)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : service demand‚
*           : BAL qui demande l'ouverture du service et vers laquelle repondre
*     retour: demande VALIDE ou NON
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache AFF_ANI
* ROLE: Verifie la demande de d‚but d'un service et acquitte ou
*       non cette demande.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_aff_valide AFFDebutService( short int         ident_aff,
                                           enum_aff_service service_id,
                                           noyau_bal_id     bal_dest)
{
	enum_aff_type	ret = SRV_TYP_DEBUT_NACQ;

	// Verify that the service is not already opened
	if (SrvEstDemandeur (AFF[ident_aff].service[service_id], bal_dest) == FALSE)
		// Is service counter maximum reached?
	    if (SrvAjouteDemandeur (AFF[ident_aff].service[service_id], bal_dest, 0L) != NULL)
		{
			ret = SRV_TYP_DEBUT_ACQ;
		}

	AFFEnvoiAcquittement(ident_aff,bal_dest,service_id,ret);
	
	if (ret == SRV_TYP_DEBUT_NACQ)
    {
		AffFichierTrace(ident_aff, "AFF_MESS: Debut Service: Pb Ajout Demandeur: %d",SRV_TYP_DEBUT_NACQ);
		return AFF_NON_VALIDE;
	}
	
    return AFF_VALIDE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_aff_valide AFFDebutServiceEspion ( short int ident_aff,
*                                               noyau_bal_id BalDemandeur,
*												enum_espion_nature  nature,
*												enum_espion_sens  sens) 
* PARAMETRES:
*     entree: instance id
*           : mailbox id of requestor
*           : type of data to be watched (input, output, protocol, data)
*     retour: demande VALIDE ou NON
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* ROLE: starts service dop
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_aff_valide AFFDebutServiceEspion( short int         ident_aff,
                                              noyau_bal_id      BalDemandeur,
											  enum_espion_nature      nature,
											  enum_espion_sens		 sens)
{
	struct_srv_espion		*pSrvData;
    enum_aff_type				ret = SRV_TYP_DEBUT_NACQ;

    // le demandeur possède deja le service ?
    if (SrvEstDemandeur (AFF[ident_aff].service[M_SRV_ESPION], BalDemandeur) == FALSE)
    {
        // le nombre de jetons max est atteind ?
        pSrvData = SrvAjouteDemandeur (AFF[ident_aff].service[M_SRV_ESPION], 
                                       BalDemandeur, 
                                       sizeof(struct_srv_espion));
        if (pSrvData != NULL)
        {
            pSrvData->sens = sens;
            pSrvData->nature = nature;
            ret = SRV_TYP_DEBUT_ACQ;
        }
    }

    AFFEnvoiAcquittement(ident_aff, BalDemandeur, M_SRV_ESPION, ret);

	if (ret == SRV_TYP_DEBUT_NACQ)
    {
		AffFichierTrace(ident_aff, "AFF_MESS: Debut Service Espion: Pb Ajout Demandeur: %d", SRV_TYP_DEBUT_NACQ);
		return AFF_NON_VALIDE;
	}
	
    return AFF_VALIDE;
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_aff_valide AFFFinService(short int ident_aff,
*                                        enum_aff_ident service_id,
*                                        noyau_bal_id bal_dest)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : service demand‚
*           : BAL qui demande la fermeture du service et vers laquelle repondre
*     retour: demande VALIDE ou NON
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale pour la tache AFF_ANI
* ROLE: Verifie la demande de fermeture d'un service et acquitte ou
*       non cette demande.
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_aff_valide AFFFinService( short int        ident_aff,
                                         enum_aff_service service_id,
                                         noyau_bal_id        bal_dest)
{

    PVOID pSrvData;

    // recherche le demandeur de ce service
    pSrvData = SrvRechercheDemandeur (AFF[ident_aff].service[service_id], 
                                      bal_dest,
                                      NULL,
                                      NULL);
    if (pSrvData == NULL)
    {
        AFFEnvoiAcquittement(ident_aff, bal_dest, service_id, SRV_TYP_FIN_NACQ);
        return AFF_NON_VALIDE;
    }
    
    SrvEnleveDemandeur (AFF[ident_aff].service[service_id], &pSrvData);
    
    AFFEnvoiAcquittement(ident_aff,bal_dest, service_id, SRV_TYP_FIN_ACQ);

    return AFF_VALIDE;

}

/**/
/*******************************************************************/
/*SYNTAX: void AFFEnvoiDeviceInfo(short int		ident_aff,		   */
/*                                enum_aff_type type_message,	   */
/*		   _AFF_ANI_DEV_INFO_MSG *device_info_msg *device_info_msg)*/
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            The function sends the device info data to the	   */
/*            destination mail box.                                */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      short int		ident_aff			- Instance ID.         */
/*      enum_aff_type	type_message		- Message type.		   */
/* _AFF_ANI_DEV_INFO_MSG *device_info_msg	- Device info msg.	   */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED void AFFEnvoiDeviceInfo(short int		ident_aff,
                                  enum_aff_type	type_message,
								  _AFF_ANI_DEV_INFO_MSG *device_info_msg)
{
	struct_aff_message *p_msg_emis = NULL;
	
	// Allocating the memory
	ExitAlloue((struct_neutre **)&(p_msg_emis),
		sizeof(struct_aff_message),
		AFF[ident_aff].pool);
	
	switch(type_message)
	{
		case SRV_TYP_EFFECTUE :		
		// Filling the message structure
		p_msg_emis->entete.neutre.bl_retour= AFF[ident_aff].ani_bal;
		p_msg_emis->entete.service = M_SRV_DEVICE_INFO;
		p_msg_emis->entete.type_message = type_message;
		memcpy( &p_msg_emis->u.srv_device_info,
				&device_info_msg->sDeviceInfoList,
				sizeof(struct_device_info_item_list));
		
		AffFichierTrace(ident_aff,
			"AFF_ANI: DCPEnvoiDeviceInfo() => vers BAL %d, msg_id %d, type %d ",
			device_info_msg->neutre.bl_retour,
			p_msg_emis->entete.service,
			p_msg_emis->entete.type_message);
		
		// Sending message
		if( Envoie(	device_info_msg->neutre.bl_retour,
					AFF[ident_aff].ani_bal,
					(struct_neutre *)p_msg_emis) != NOYAU_OK)
		{
			ExitLibere((struct_neutre **)&p_msg_emis);
		}

		default:
			break;
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void AFFEnvoiAcquittement( short int ident_aff,
*                                     noyau_bal_id bal_dest,
*                                     enum_aff_ident service_id,
*                                     enum_aff_type type_message)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : boite aux lettres destinataire
*           : identificateur du service
*           : type du message d'acquittement a renvoyer
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message d'acquittement vers l'application
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiAcquittement( short int        ident_aff,
                                     noyau_bal_id        bal_dest,
                                     enum_aff_service service_id,
                                     enum_aff_type    type_message)
{
   struct_aff_message  *p_msg_emis = (struct_aff_message *)(0);

   /* preparation du message pour la boite aux lettres */
   ExitAlloue( (struct_neutre **)&(p_msg_emis),
                sizeof(struct_aff_message),AFF[ident_aff].pool);
   #ifdef DEBUG
   AffFichierTrace(ident_aff,"AFF_MESS => Alloue() : %p ", p_msg_emis);
   #endif


   /* mise a jour de la structure de dialogue */
   p_msg_emis->entete.neutre.bl_retour= AFF[ident_aff].ani_bal;
   p_msg_emis->entete.service = service_id;
   p_msg_emis->entete.type_message = type_message;

   AffFichierTrace(ident_aff,"AFF_ANI: EnvoiAcquittement() => vers BAL %d, msg_id %d, type %d ",
                         bal_dest,p_msg_emis->entete.service,p_msg_emis->entete.type_message);

	/* envoie du message dans la boite aux lettres */
	if(Envoie(bal_dest,AFF[ident_aff].ani_bal,(struct_neutre *)p_msg_emis) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFEnvoiMessageEspion( short int ident_aff,
*                                    unsigned char *msg_espion,
*                                    short int longueur,
*                                    enum_espion_sens sens)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : buffer contenant le message dop
*           : longueur du message
*           : sens du message : entrant / sortant
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer les messages du ESPION vers la tache ayant ouvert le service
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiMessageEspion(short int     ident_aff,
                                  unsigned char *msg_espion,
                                  short int     longueur,
								  enum_espion_nature nature,
                                  enum_espion_sens  sens)
{
	struct_aff_message  p_msg_emis;
    struct_aff_envoi_service data = { ident_aff, &p_msg_emis };

    // construction du message espion 
	p_msg_emis.entete.neutre.bl_retour= AFF[ident_aff].ani_bal;
	p_msg_emis.entete.service = M_SRV_ESPION;
	p_msg_emis.entete.type_message = SRV_TYP_MESSAGE_ESPION;
	p_msg_emis.u.srv_espion.taille = longueur;	// si la demande concerne tout le protocole 
	
	p_msg_emis.u.srv_espion.nature = nature;

	p_msg_emis.u.srv_espion.sens = sens;

	memcpy( p_msg_emis.u.srv_espion.donnee, msg_espion, longueur);
   
	AffFichierTrace(ident_aff, "AFF_ANI: EnvoiMessageEspion() => msg_id %d, type %d ",
					p_msg_emis.entete.service,p_msg_emis.entete.type_message);
	
	// Envoie un message à chaque demandeur du service
	SrvPourChaqueDemandeur(AFF[ident_aff].service[M_SRV_ESPION], EnvoiDemandeurEspion, &data);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  void AFFEnvoiEtat(	short				ident_aff,
*								noyau_bal_id		bal_dest,
*								enum_aff_type		type,
*								struct_b_etat_aff	uiStatus)
* PARAMETRES:
*     entree: numero de l'afficheur
*           : Boite aux lettres destinataire du message (si demande)
*           : message d'etat
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message contenant l'etat de l'afficheur
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiEtat(short int			ident_aff,
							noyau_bal_id		bal_dest,
							enum_aff_type		type,
							unsigned int		uiStatus)
{
	struct_aff_message p_msg_emis;
	struct_aff_message *p_msg = NULL;
	struct_aff_envoi_service data = { ident_aff, &p_msg_emis };

   switch(type)
   {
      case SRV_TYP_DEMANDE :
         ExitAlloue((struct_neutre **)&(p_msg), 
		  sizeof(struct_aff_message),AFF[ident_aff].pool);

         /* mise a jour de la structure de dialogue */
         p_msg->entete.neutre.bl_retour= AFF[ident_aff].ani_bal;
         p_msg->entete.service = M_SRV_ETAT;
         p_msg->entete.type_message = SRV_TYP_NOUVEL_ETAT;
         /* message d'etat */
         memcpy( &p_msg->u.srv_etat.status, &AFF[ident_aff].gestion_peripherique.b_etat,
                 sizeof(struct_b_etat_aff));

         /* envoie du message dans la boite aux lettres */
		 if(Envoie(bal_dest,AFF[ident_aff].ani_bal,(struct_neutre *)p_msg) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg);
		 }
 		 break;

      case SRV_TYP_NOUVEL_ETAT : // envoie etat a tous les demandeurs du service
         // si la liste n'est pas vide
         if ( ! SrvEstVide (AFF[ident_aff].service[M_SRV_ETAT]))
		 {		  
               p_msg_emis.entete.neutre.bl_retour= AFF[ident_aff].ani_bal;
               p_msg_emis.entete.service = M_SRV_ETAT;
               p_msg_emis.entete.type_message = SRV_TYP_NOUVEL_ETAT;

               memcpy( &p_msg_emis.u.srv_etat.status, &uiStatus, sizeof(unsigned int));

   	   		   SrvPourChaqueDemandeur(AFF[ident_aff].service[M_SRV_ETAT], EnvoiDemandeur, &data);

			}
         else
            AffFichierTrace(ident_aff, "AFF_MESS: EnvoiEtat: Liste demandeurs vide Aff. No : %d",ident_aff);
         break;

      default :
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:void AffEnvoiVisu( short int     ident_aff,
*                            noyau_bal_id     bal_dest,
*                            enum_aff_type type)
* PARAMETRES:
*    entree : numero de l'afficheur
*           : type de message (DEN_DEMANDE ou VIDE)
*           : bal du demandeur de la visu
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: Fonction qui transmet les messages du dialogue operateur
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiVisu( short int     ident_aff,
                             noyau_bal_id     bal_dest,
                             enum_aff_type type)
{
   struct_aff_message  msg_emis;
   struct_aff_message  * p_msg = NULL;
   struct_aff_envoi_service data = { ident_aff, &msg_emis };
   unsigned int	i, n;
   DWORD dwSize;

   switch(type)
   {
      case SRV_TYP_DEMANDE : // si demande => envoyer msg au demandeur

         ExitAlloue((struct_neutre **)(&p_msg),
                    sizeof(struct_aff_message),AFF[ident_aff].pool);

         dwSize = sizeof( p_msg->u.srv_visu.label[0] );
		 for (i=0; i<AFF_MAX_LIGNES; i++)
		 {
			 strncpy_s(p_msg->u.srv_visu.label[i], sizeof(p_msg->u.srv_visu.label[i]), AFF[ident_aff].tab_lignes_courantes[i], dwSize);
			 p_msg->u.srv_visu.label[i][dwSize-1] = '\0';
		 }

         p_msg->entete.service = M_AFF_VISU;
         p_msg->entete.type_message = AFF_TYP_AFFICHAGE_COURANT;

         if(Envoie( bal_dest, AFF[ident_aff].ani_bal, (struct_neutre *)(p_msg)) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg);
		 }

         break;

      case AFF_TYP_AFFICHAGE_COURANT :
         // si la liste n'est pas vide
         if ( ! SrvEstVide (AFF[ident_aff].service[M_AFF_VISU]))
         {
               dwSize = sizeof( msg_emis.u.srv_visu.label[0] );

			   for (i=0; i<AFF_MAX_LIGNES; i++)
			   {
				   strncpy_s(msg_emis.u.srv_visu.label[i], sizeof(msg_emis.u.srv_visu.label[i]), AFF[ident_aff].tab_lignes_courantes[i], dwSize);
					msg_emis.u.srv_visu.label[i][dwSize-1] = '\0';

					// reverse charset conversion
					for( n = 0; n < strlen(msg_emis.u.srv_visu.label[i]); n++)
					{
					  msg_emis.u.srv_visu.label[i][n]=AFF[ident_aff].inv_char_conv_array[msg_emis.u.srv_visu.label[i][n]];
					}

			   }

               msg_emis.entete.service = M_AFF_VISU;
               msg_emis.entete.type_message = AFF_TYP_AFFICHAGE_COURANT;

			   SrvPourChaqueDemandeur(AFF[ident_aff].service[M_AFF_VISU], EnvoiDemandeur, &data);
         }
         else
            AffFichierTrace(ident_aff, "AFF_MESS: EnvoiVisu: Liste demandeurs vide Aff. ");
         break;

      default :
         AffFichierTrace(ident_aff, "AFF_MESS: EnvoiVisu: PB envoie message Visu Aff. No : %d",ident_aff);
         break;
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: void  AFFEnvoiIos( short int     ident_aff,
*                             enum_ani__ios msg_id,
*                             void          *parametres)
* PARAMETRES:
*     entree: identificateur de l'imprimante
*           : type de message destin‚ … la tache IOS
*           : paramŠtres selon le type du message
*     retour: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale, sp‚cifique  … l'imprimante affcale.
* ROLE: emission de messages … destination de la tache IOS
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void AFFEnvoiIos( short int     ident_aff,
                            enum_ani__ios msg_id,
                            void          *parametre)
{
   struct_ani__ios *p_ani = (struct_ani__ios *)(0);
   struct_aff_message *pAffMsg = (struct_aff_message *)parametre;

   /* allocation memoire pour le message */
   ExitAlloue ((struct_neutre **)(&p_ani),sizeof(struct_ani__ios),
                  AFF[ident_aff].pool);
   #ifdef DEBUG
   AffFichierTrace(ident_aff,"AFF_MESS => Alloue() : %p ", p_ani);
   #endif

   /* mise a jour de l'identificateur du message */
   p_ani->message_id = msg_id;

   /* commande en fonction du message */
   switch (msg_id)
   {
      case M_AFFICHAGE :
         p_ani->u.ani_ios.u.contenu.modif_priorite = (short)parametre;
         break;
      case M_ALLUME :
         p_ani->u.ani_ios.u.allume.etat = (boolean)parametre;
		 break;
	  case M_VOYANT:
		 memcpy(&p_ani->u.ani_ios.u.voyant, parametre, sizeof(struct_aff_voyant));
		 break;
                         /********************/
      case M_ETAT :      /* Pas de paramŠtre */
      case M_ARRET :     /********************/
	  case M_RESET_HW :
         break;

	  case SRV_TYP_GET:		 
		 memcpy(&p_ani->u.ani_ios.u.sDevInfoMsg.neutre,
				&pAffMsg->entete.neutre,
				sizeof(struct_neutre));
		 break;

      default :
         break;
   }

   /* redirection trace */
   AffFichierTrace(ident_aff, "AFF_ANI: EnvoiIOS() =>  msg_id '%d' ",
                   p_ani->message_id);

   /* postage dans la boite aux lettres */
   if(Envoie (AFF[ident_aff].ios_bal,AFF[ident_aff].ani_bal,(struct_neutre *)p_ani) != NOYAU_OK){
	ExitLibere((struct_neutre **)&p_ani);
   }
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  PROTECTED BOOL EnvoiDemandeurEspion (zip_ident_lecteur ident, 
*                                enum_zip_service  service,
*                                enum_zip_type     type,
*                                struct_zip_message *p_msg)
*
* PARAMETRES:
*     entree: numero du lecteur
*           : service du message
*           : type du message
*           : message
*     retour: TRUE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message contenant les donnees d'une carte
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE BOOL WINAPI EnvoiDemandeurEspion (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
    struct_aff_envoi_service *p_param = Param;
    struct_srv_espion * p_data = DataDuDemandeur;
    struct_aff_message *p_msg_emis; 


    // test si demandeur interesse ou non
   if (( p_data->nature == SRV_ESPION_MESSAGE_SERVICE ||
         p_param->p_msg->u.srv_espion.nature == SRV_ESPION_MESSAGE_DONNEES) &&
        (p_data->sens == SRV_ESPION_ENTRANT_SORTANT ||
         p_data->sens == p_param->p_msg->u.srv_espion.sens) )
    {
        
        ExitAlloue((struct_neutre **)(&p_msg_emis),
            sizeof(struct_aff_message),AFF[p_param->inst_id].pool);
        
        // recopie du message
        *p_msg_emis = *(p_param->p_msg);
        
		if(Envoie(BalDemandeur,AFF[p_param->inst_id].ani_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg_emis);
		}
    }

    // en renvoyant TRUE, on passe au demandeur suivant
    return TRUE;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:  PROTECTED BOOL EnvoiDemandeur (PVOID Param, 
*										   noyau_bal_id BalDemandeur,
*										   PVOID DataDuDemandeur)
*
* PARAMETRES:
*     entree: numero du lecteur
*           : service du message
*           : type du message
*           : message
*     retour: TRUE
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Fonction locale
* ROLE: envoyer un message contenant les donnees 
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE BOOL WINAPI EnvoiDemandeur (PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
    struct_aff_envoi_service *p_data = (PVOID) Param;
    struct_aff_message *p_msg_emis;
    
    ExitAlloue((struct_neutre **)(&p_msg_emis),
        sizeof(struct_aff_message),AFF[p_data->inst_id].pool);
    
    // recopie du message
    *p_msg_emis = *(p_data->p_msg);

    if(Envoie(BalDemandeur,AFF[p_data->inst_id].ani_bal,(struct_neutre *)(p_msg_emis)) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
    // en renvoyant TRUE, on passe au demandeur suivant
    return TRUE;
}

/**/
/*******************************************************************/
/*SYNTAX: void AFFLibereBAL(noyau_bal_id bal)	                   */
/*=================================================================*/
/*TYPE:   Public function.                                         */
/*=================================================================*/
/*DESCRIPTION:                                                     */
/*            This function deallocates memory in the mail box     */
/*            occupied by the messages.                            */
/*=================================================================*/
/*PARAMETERS:                                                      */
/*      noyau_bal_id bal    - Mail box ID.			               */
/*=================================================================*/
/*  Return                Description                              */
/*-----------------------------------------------------------------*/
/*  void                  This function does not return a value.   */
/*******************************************************************/
PROTECTED void AFFLibereBAL(noyau_bal_id bal)
{
	struct_neutre  *p_neutre;

	while((TestRecoit(bal, (struct_neutre **)(&p_neutre)) == NOYAU_BAL_MESS))
	{
		ExitLibere((struct_neutre **)(&p_neutre));
	}
}
