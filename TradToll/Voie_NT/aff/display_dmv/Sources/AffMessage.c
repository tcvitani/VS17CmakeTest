#include <stdio.h>
#include <string.h>

#include <noyau.h>
#include <debug.h>
#include <InstTracer.h>

#include <aff_ext.h>

#include <DmvGlobal.h>
#include <AffService.h>
#include <AffMessage.h>

#include <memclass.h>

PRIVATE BOOL WINAPI SendSpyRequest(PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur);
PRIVATE BOOL WINAPI SendRequest(PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur);


PROTECTED enum_aff_validity AFFStartService(dmv_inst_id      inst_id,
											enum_aff_service service_id,
											noyau_bal_id     bal_dest)
{
	enum_aff_type	ret;

	ret = SRV_TYP_DEBUT_NACQ;

	// Verify that the service is not already opened
	if ((SrvEstDemandeur(gsDMV[inst_id].service[service_id], bal_dest) == FALSE) && (gsDMV[inst_id].sStatus.liaison_hs == FALSE))
		// Is service counter maximum reached?	
		if (SrvAjouteDemandeur(gsDMV[inst_id].service[service_id], bal_dest, 0L) != NULL)
		{
			ret = SRV_TYP_DEBUT_ACQ;
		}

	AFFSendAcknowledgement(inst_id, bal_dest, service_id, ret);

	if (ret == SRV_TYP_DEBUT_NACQ)
	{
		MInstTracer_FichierTrace(inst_id, "AFF_MESS: Debut Service: Pb Ajout Demandeur: %d", SRV_TYP_DEBUT_NACQ);
		return AFF_NON_VALID;
	}

	return AFF_VALID;
}

PROTECTED enum_aff_validity AFFStartSpyService(dmv_inst_id        inst_id,
											   noyau_bal_id      BalDemandeur,
											   enum_espion_nature      nature,
											   enum_espion_sens		 sens)
{
	struct_srv_espion		*pSrvData;
	enum_aff_type				ret = SRV_TYP_DEBUT_NACQ;

	// le demandeur possède deja le service ?
	if (SrvEstDemandeur(gsDMV[inst_id].service[M_SRV_ESPION], BalDemandeur) == FALSE)
	{
		// le nombre de jetons max est atteind ?
		pSrvData = SrvAjouteDemandeur(gsDMV[inst_id].service[M_SRV_ESPION],
									  BalDemandeur,
									  sizeof(struct_srv_espion));
		if (pSrvData != NULL)
		{
			pSrvData->sens = sens;
			pSrvData->nature = nature;
			ret = SRV_TYP_DEBUT_ACQ;
		}
	}

	AFFSendAcknowledgement(inst_id, BalDemandeur, M_SRV_ESPION, ret);

	if (ret == SRV_TYP_DEBUT_NACQ)
	{
		MInstTracer_FichierTrace(inst_id, "AFF_MESS: Debut Service Espion: Pb Ajout Demandeur: %d", SRV_TYP_DEBUT_NACQ);
		return AFF_NON_VALID;
	}


	return AFF_VALID;
}


PROTECTED enum_aff_validity AFFEndService(dmv_inst_id       inst_id,
										  enum_aff_service  service_id,
										  noyau_bal_id      bal_dest)
{

	PVOID pSrvData;

	// recherche le demandeur de ce service
	pSrvData = SrvRechercheDemandeur(gsDMV[inst_id].service[service_id],
									 bal_dest,
									 NULL,
									 NULL);
	if (pSrvData == NULL)
	{
		AFFSendAcknowledgement(inst_id, bal_dest, service_id, SRV_TYP_FIN_NACQ);
		return AFF_NON_VALID;
	}

	SrvEnleveDemandeur(gsDMV[inst_id].service[service_id], &pSrvData);

	AFFSendAcknowledgement(inst_id, bal_dest, service_id, SRV_TYP_FIN_ACQ);

	return AFF_VALID;

}

PROTECTED void AFFSendAcknowledgement(dmv_inst_id       inst_id,
									  noyau_bal_id        bal_dest,
									  enum_aff_service service_id,
									  enum_aff_type    type_message)
{
	struct_aff_message  *p_msg_emis = (struct_aff_message *)(0);

	/* preparation du message pour la boite aux lettres */
	ExitAlloue((struct_neutre **)&(p_msg_emis),
			   sizeof(struct_aff_message), gsDMV[inst_id].pool);


	/* mise a jour de la structure de dialogue */
	p_msg_emis->entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
	p_msg_emis->entete.service = service_id;
	p_msg_emis->entete.type_message = type_message;

	MInstTracer_FichierTrace(inst_id, "AFF_ANI: EnvoiAcquittement() => vers BAL %d, msg_id %d, type %d ",
							 bal_dest, p_msg_emis->entete.service, p_msg_emis->entete.type_message);

	/* envoie du message dans la boite aux lettres */
	if (Envoie(bal_dest, gsDMV[inst_id].ani_mbox, (struct_neutre *)p_msg_emis) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
}

PROTECTED void AFFSendSpyMessage(dmv_inst_id    inst_id,
							     unsigned char *msg_espion,
							     dmv_inst_id    longueur,
							     enum_espion_nature nature,
							     enum_espion_sens  sens)
{
	struct_aff_message  p_msg_emis;
	struct_aff_send_service data = { inst_id, &p_msg_emis };

	// construction du message espion 
	p_msg_emis.entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
	p_msg_emis.entete.service = M_SRV_ESPION;
	p_msg_emis.entete.type_message = SRV_TYP_MESSAGE_ESPION;
	p_msg_emis.u.srv_espion.taille = longueur;	// si la demande concerne tout le protocole 

	p_msg_emis.u.srv_espion.nature = nature;

	p_msg_emis.u.srv_espion.sens = sens;

	memcpy(p_msg_emis.u.srv_espion.donnee, msg_espion, longueur);

	MInstTracer_FichierTrace(inst_id, "AFF_ANI: EnvoiMessageEspion() => msg_id %d, type %d ",
							 p_msg_emis.entete.service, p_msg_emis.entete.type_message);

	// Envoie un message à chaque demandeur du service
	SrvPourChaqueDemandeur(gsDMV[inst_id].service[M_SRV_ESPION], SendSpyRequest, &data);
}

PROTECTED void AFFSendStatus(dmv_inst_id    inst_id,
							 noyau_bal_id     bal_dest,
							 enum_aff_type type)
{
	struct_aff_message p_msg_emis;
	struct_aff_message *p_msg = NULL;
	struct_aff_send_service data = { inst_id, &p_msg_emis };

	switch (type)
	{
	case SRV_TYP_DEMANDE:
		ExitAlloue((struct_neutre **)&(p_msg),
				   sizeof(struct_aff_message), gsDMV[inst_id].pool);

		/* mise a jour de la structure de dialogue */
		p_msg->entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
		p_msg->entete.service = M_SRV_ETAT;
		p_msg->entete.type_message = SRV_TYP_NOUVEL_ETAT;
		/* message d'etat */
		memcpy(&p_msg->u.srv_etat.status, &gsDMV[inst_id].sStatus,
			   sizeof(struct_b_etat_aff));

		/* envoie du message dans la boite aux lettres */
		if (Envoie(bal_dest, gsDMV[inst_id].ani_mbox, (struct_neutre *)p_msg) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg);
		}
		break;

	case SRV_TYP_NOUVEL_ETAT: // envoie etat a tous les demandeurs du service
		// si la liste n'est pas vide
		// NBL : 17/11/99
		if (!SrvEstVide(gsDMV[inst_id].service[M_SRV_ETAT]))
		{
			p_msg_emis.entete.neutre.bl_retour = gsDMV[inst_id].ani_mbox;
			p_msg_emis.entete.service = M_SRV_ETAT;
			p_msg_emis.entete.type_message = SRV_TYP_NOUVEL_ETAT;

			memcpy(&p_msg_emis.u.srv_etat.status, &gsDMV[inst_id].sStatus, sizeof(struct_b_etat_aff));

			SrvPourChaqueDemandeur(gsDMV[inst_id].service[M_SRV_ETAT], SendRequest, &data);

		}
		else
			MInstTracer_FichierTrace(inst_id, "AFF_MESS: EnvoiEtat: Liste demandeurs vide Aff. No : %d", inst_id);
		break;

	default:
		break;
	}
}

PROTECTED void AFFSendView(dmv_inst_id    inst_id,
						   noyau_bal_id   bal_dest,
						   enum_aff_type  type)
{
	struct_aff_message  msg_emis;
	struct_aff_message  * p_msg = NULL;
	struct_aff_send_service data = { inst_id, &msg_emis };
	unsigned int	i, n;
	DWORD dwSize;

	switch (type)
	{
	case SRV_TYP_DEMANDE: // si demande => envoyer msg au demandeur

		ExitAlloue((struct_neutre **)(&p_msg),
				   sizeof(struct_aff_message), gsDMV[inst_id].pool);

		dwSize = sizeof(p_msg->u.srv_visu.label[0]);
		for (i = 0; i < AFF_MAX_LIGNES; i++)
		{
			strncpy_s(p_msg->u.srv_visu.label[i], sizeof(p_msg->u.srv_visu.label[i]), gsDMV[inst_id].szCurrentLineTable[i], dwSize);
			p_msg->u.srv_visu.label[i][dwSize - 1] = '\0';
		}

		p_msg->entete.service = M_AFF_VISU;
		p_msg->entete.type_message = AFF_TYP_AFFICHAGE_COURANT;

		if (Envoie(bal_dest, gsDMV[inst_id].ani_mbox, (struct_neutre *)(p_msg)) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg);
		}

		break;

	case AFF_TYP_AFFICHAGE_COURANT:
		// si la liste n'est pas vide
		// NBL : 17/11/99
		if (!SrvEstVide(gsDMV[inst_id].service[M_AFF_VISU]))
		{
			dwSize = sizeof(msg_emis.u.srv_visu.label[0]);

			for (i = 0; i < AFF_MAX_LIGNES; i++)
			{
				strncpy_s(msg_emis.u.srv_visu.label[i], sizeof(msg_emis.u.srv_visu.label[i]), gsDMV[inst_id].szCurrentLineTable[i], dwSize);
				msg_emis.u.srv_visu.label[i][dwSize - 1] = '\0';

				// reverse charset conversion
				for (n = 0; n < strlen(msg_emis.u.srv_visu.label[i]); n++)
				{
					msg_emis.u.srv_visu.label[i][n] = gsDMV[inst_id].inv_char_conv_array[msg_emis.u.srv_visu.label[i][n]]; 
				}

			}

			msg_emis.entete.service = M_AFF_VISU;
			msg_emis.entete.type_message = AFF_TYP_AFFICHAGE_COURANT;

			SrvPourChaqueDemandeur(gsDMV[inst_id].service[M_AFF_VISU], SendRequest, &data);
		}
		else
			MInstTracer_FichierTrace(inst_id, "AFF_MESS: EnvoiVisu: Liste demandeurs vide Aff. ");
		break;

	default:
		MInstTracer_FichierTrace(inst_id, "AFF_MESS: EnvoiVisu: PB envoie message Visu Aff. No : %d", inst_id);
		break;
	}
}

PROTECTED void AFFSendToIos(dmv_inst_id   inst_id,
							enum_ani_ios  msg_id,
							void          *parametre)
{
	struct_ani_ios *p_ani = (struct_ani_ios *)(0);

	ExitAlloue((struct_neutre **)(&p_ani), sizeof(struct_ani_ios), gsDMV[inst_id].pool);

	p_ani->message_id = msg_id;

	switch (msg_id)
	{
	case M_AFFICHAGE:
		p_ani->u.ani_ios.u.contenu.modif_priorite = (short)parametre;
		break;
	case M_ALLUME:
		p_ani->u.ani_ios.u.allume.etat = (boolean)parametre;
		break;
	case M_VOYANT:
		memcpy(&p_ani->u.ani_ios.u.voyant, parametre, sizeof(struct_aff_voyant));
		break;

	case M_ETAT:
	case M_ARRET:
	case M_RESET_HW:
		break;

	default:
		break;
	}

	MInstTracer_FichierTrace(inst_id, "AFF_ANI: EnvoiIOS() =>  msg_id '%d' ", p_ani->message_id);

	if (Envoie(gsDMV[inst_id].ios_mbox, gsDMV[inst_id].ani_mbox, (struct_neutre *)p_ani) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_ani);
	}

}


PRIVATE BOOL WINAPI SendSpyRequest(PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
	struct_aff_send_service *p_param = Param;
	struct_srv_espion * p_data = DataDuDemandeur; // NBL:12/99
	struct_aff_message *p_msg_emis;


	// test si demandeur interesse ou non
	if ((p_data->nature == SRV_ESPION_MESSAGE_SERVICE ||
		p_param->p_msg->u.srv_espion.nature == SRV_ESPION_MESSAGE_DONNEES) &&
		(p_data->sens == SRV_ESPION_ENTRANT_SORTANT ||
		p_data->sens == p_param->p_msg->u.srv_espion.sens))
	{

		ExitAlloue((struct_neutre **)(&p_msg_emis),
				   sizeof(struct_aff_message), gsDMV[p_param->inst_id].pool);

		// recopie du message
		*p_msg_emis = *(p_param->p_msg);

		if (Envoie(BalDemandeur, gsDMV[p_param->inst_id].ani_mbox, (struct_neutre *)(p_msg_emis)) != NOYAU_OK){
			ExitLibere((struct_neutre **)&p_msg_emis);
		}
	}

	// en renvoyant TRUE, on passe au demandeur suivant
	return TRUE;
}

PRIVATE BOOL WINAPI SendRequest(PVOID Param, noyau_bal_id BalDemandeur, PVOID DataDuDemandeur)
{
	struct_aff_send_service *p_data = (PVOID)Param;
	struct_aff_message *p_msg_emis;

	ExitAlloue((struct_neutre **)(&p_msg_emis),
			   sizeof(struct_aff_message), gsDMV[p_data->inst_id].pool);

	// recopie du message
	*p_msg_emis = *(p_data->p_msg);

	if (Envoie(BalDemandeur, gsDMV[p_data->inst_id].ani_mbox, (struct_neutre *)(p_msg_emis)) != NOYAU_OK){
		ExitLibere((struct_neutre **)&p_msg_emis);
	}
	// en renvoyant TRUE, on passe au demandeur suivant
	return TRUE;
}
