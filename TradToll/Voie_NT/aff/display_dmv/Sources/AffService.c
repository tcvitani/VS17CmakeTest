#include <noyau.h>
#include <debug.h>
#include <reg.h>
#include <InstTracer.h>
#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <DmvGlobal.h>
#include <DmvAni.h>
#include <AffService.h>
#include <AffMessage.h>

#include <memclass.h>

PROTECTED void AFFProcessServiceMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu)
{
	enum_aff_validity   ret;
	struct_b_etat_aff	*pStatus;
	long				line;
	aff_infos			sDataInfo;


	pStatus = (struct_b_etat_aff *)&gsDMV[inst_id].sStatus;

	// depending on the type of service requested 
	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		// redirection trace 
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Start service DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		// search if service available and acknowledgment
		ret = AFFStartService(inst_id,
							  M_AFF_AFFICHAGE,
							  p_msg_recu->entete.neutre.bl_retour);

		// if the service is free, send a message to IOS
		if (ret == AFF_VALID)
		{
			// for faster pooling on the AFF state
			gsDMV[inst_id].timeout_mbox = AFF_TIMEOUT_FAST;
		}
		break;

	case SRV_TYP_FIN:
		// redirection trace 
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', End service DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		ret = AFFEndService(inst_id,
							M_AFF_AFFICHAGE,
							p_msg_recu->entete.neutre.bl_retour);

		// if the service is disabled
		if (ret == AFF_VALID)
		{
			// for slower pooling on the AFF state
			gsDMV[inst_id].timeout_mbox = AFF_TIMEOUT_SLOW;
		}
		break;

	case SRV_TYP_DEMANDE:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Demande DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);


		// Data calculated with price precision
		if (AFF_CHAMP_DONNEE_LONG(p_msg_recu->u.srv_affichage.demande.precision_tarif) == 0)
		{
			MInstTracer_FichierTrace(inst_id, "AFF_SERV *** Service DISPLAY => price precision must not be zero ***");
			ExitBad();
		}

		AFF_DataConversion(&sDataInfo, &p_msg_recu->u.srv_affichage.demande);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// sending an acknowledgment to the application
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 SRV_TYP_DEMANDE_ACQ);

			// If the display is ready, you can send
			if (pStatus->aff_hs == FALSE)
			{
				AffManagement(inst_id, p_msg_recu, &sDataInfo);
				AFFSendAcknowledgement(inst_id,
									 p_msg_recu->entete.neutre.bl_retour,
									 M_AFF_AFFICHAGE,
									 SRV_TYP_EFFECTUE);
			}
			else
			{
				// Send to IOS only to remember what the last display was
				AffManagement(inst_id, p_msg_recu, &sDataInfo);
			}

		}
		else
		{
			// sending a non-acknowledgment
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 SRV_TYP_DEMANDE_NACQ);
		}
		break;

	case SRV_TYP_TEST:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Test DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// sending an acknowledgment to the application
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 SRV_TYP_TEST_ACQ);

			// If the display is ready, you can send
			if (pStatus->aff_hs == FALSE)
			{
				AffManagement(inst_id, p_msg_recu, &sDataInfo);
				AFFSendAcknowledgement(inst_id,
									 p_msg_recu->entete.neutre.bl_retour,
									 M_AFF_AFFICHAGE,
									 SRV_TYP_EFFECTUE);
			}

		}
		else
		{
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 SRV_TYP_TEST_NACQ);
		}

		break;

	case AFF_TYP_RESET:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Reset DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 AFF_TYP_RESET_ACQ);

			if (pStatus->aff_hs == FALSE)
			{
				ResetManagement(inst_id, p_msg_recu);
				AFFSendAcknowledgement(inst_id,
									 p_msg_recu->entete.neutre.bl_retour,
									 M_AFF_AFFICHAGE,
									 SRV_TYP_EFFECTUE);
			}

		}
		else
		{
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 AFF_TYP_RESET_NACQ);
		}
		break;

	case AFF_TYP_RESET_HW:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Reset HW DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_AFF_AFFICHAGE,
								 AFF_TYP_RESET_HW_ACQ);

			// Si l'afficheur est pret, on peut envoyer
			if (pStatus->aff_hs == FALSE)
			{
				// This device doesn't support hardware reset so message won't be sent to IOS and
				// SRV_TYP_DEMANDE_INCONNUE instead SRV_TYP_EFFECTUE will be sent to main application.

				AFFSendAcknowledgement(inst_id,
									 p_msg_recu->entete.neutre.bl_retour,
									 M_AFF_AFFICHAGE,
									 SRV_TYP_DEMANDE_INCONNUE /*SRV_TYP_EFFECTUE*/);

				MInstTrtacer_FichierError(inst_id, "AFF_SERV: AFFReceptionAffichage() => Hardware reset not supported by this device!");
			}

		}
		else
		{
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_RESET_HW_NACQ);
		}
		break;

	case AFF_TYP_ALLUME:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Eclairage DISPLAY ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// envoi d'un acquittement a l'application 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_ALLUME_ACQ);

			// Si l'afficheur est pret, on peut envoyer
			if (pStatus->aff_hs == FALSE)
			{
				// This device doesn't support dimming so SRV_TYP_DEMANDE_INCONNUE instead of SRV_TYP_EFFECTUE
				// will be sent to main application. Message will be sent to IOS but IOS doesn't do anything.

				DimmingManagement(inst_id, p_msg_recu);
				AFFSendAcknowledgement(inst_id,
									   p_msg_recu->entete.neutre.bl_retour,
									   M_AFF_AFFICHAGE,
									   SRV_TYP_DEMANDE_INCONNUE /*SRV_TYP_EFFECTUE*/);
			}

		}
		else
		{
			// envoi d'un non acquittement 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_ALLUME_NACQ);
		}
		break;

	case AFF_TYP_NEW_FILE:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', DISPLAY - AFF_TYP_NEW_FILE => %s",
								 p_msg_recu->entete.neutre.bl_retour,
								 p_msg_recu->u.srv_affichage.newFile.FilePath);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// envoi d'un acquittement a l'application 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_NEW_FILE_ACQ);
			gsDMV[inst_id].position = 1;

			if (AFF_ReadFile(p_msg_recu->u.srv_affichage.newFile.FilePath,
				gsDMV[inst_id].LabelTable,
				NB_MAX_LABEL,
				&line,
				&gsDMV[inst_id].position) == FALSE)
			{
				FreeTabLabel(gsDMV[inst_id].LabelTable, NB_MAX_LABEL);
				MInstTrtacer_FichierError(inst_id, "AFF_INIT ***** AffLance() => Error in the file %s, line %ld *****",
										  p_msg_recu->u.srv_affichage.newFile.FilePath,
										  line);
				//return( INST_INIT_ERR_FICHIER_PARAM );
				gsDMV[inst_id].position = 1;
				if (AFF_ReadFile(gsDMV[inst_id].sInitStructure.szFormatFile,
					gsDMV[inst_id].LabelTable,
					NB_MAX_LABEL,
					&line,
					&gsDMV[inst_id].position) == FALSE)
				{
					FreeTabLabel(gsDMV[inst_id].LabelTable, NB_MAX_LABEL);

					MInstTracer_FichierTrace(inst_id, "AFF_INIT ***** AffLance() => Error in the file %s, line %ld *****",
											 gsDMV[inst_id].sInitStructure.szFormatFile,
											 line);
				}
			}
			else
			{
				MInstTracer_FichierTrace(inst_id, "AFFReceptionAffichage() - New Ped File: %s => %s",
										 p_msg_recu->u.srv_affichage.newFile.FilePath,
										 gsDMV[inst_id].pcKey);

				FinRegionLocale();
				DelaiTache(2);
				DebutRegionLocale();

				REG_Ecrire_Chaine(CSR_REG_KEYi_ROOT, gsDMV[inst_id].pcKey, AFF_REG_KEYv_FORMAT_FILE, p_msg_recu->u.srv_affichage.newFile.FilePath);
			}
		}
		else
		{
			// envoi d'un non acquittement 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_NEW_FILE_NACQ);
		}
		break;

	case AFF_TYP_VOYANT:
		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_AFFICHAGE], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// envoi d'un acquittement a l'application 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_VOYANT_ACQ);

			if (pStatus->aff_hs == FALSE)
			{
				// This device doesn't support dimming so SRV_TYP_DEMANDE_INCONNUE instead of SRV_TYP_EFFECTUE
				// will be sent to main application. Message will be sent to IOS but IOS doesn't do anything.

				AFFSendToIos(inst_id, M_VOYANT, &p_msg_recu->u.srv_affichage.voyant);
				AFFSendAcknowledgement(inst_id,
									   p_msg_recu->entete.neutre.bl_retour,
									   M_AFF_AFFICHAGE,
									   SRV_TYP_DEMANDE_INCONNUE /*SRV_TYP_EFFECTUE*/);
			}
		}
		else
		{
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   AFF_TYP_VOYANT_NACQ);
		}
		break;

	default:
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service DISPLAY => type '%d' inconnu ***",
								  p_msg_recu->entete.type_message);
		break;
	}
}

PROTECTED void AFFProcessViewMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu)
{
	enum_aff_validity   ret;
	struct_b_etat_aff	*p_etat;

	p_etat = (struct_b_etat_aff *)&gsDMV[inst_id].sStatus;

	// suivant le type de service demande 
	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		// redirection trace 
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Debut service VISU ",
								 p_msg_recu->entete.neutre.bl_retour);

		// recherche si service disponible et acquittement 
		ret = AFFStartService(inst_id,
							  M_AFF_VISU,
							  p_msg_recu->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN:
		// redirection trace 
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Fin service VISU ",
								 p_msg_recu->entete.neutre.bl_retour);

		ret = AFFEndService(inst_id,
							M_AFF_VISU,
							p_msg_recu->entete.neutre.bl_retour);
		break;

	case SRV_TYP_DEMANDE:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Demande VISU ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_AFF_VISU], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// envoi d'un acquittement a l'application 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_VISU,
								   SRV_TYP_DEMANDE_ACQ);

			// Si l'afficheur est pret, on peut envoyer
			if (p_etat->aff_hs == FALSE)
			{
				AFFSendView(inst_id,
							p_msg_recu->entete.neutre.bl_retour,
							SRV_TYP_DEMANDE);
			}

		}
		else
		{
			// envoi d'un non acquittement 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_AFF_AFFICHAGE,
								   SRV_TYP_DEMANDE_NACQ);
		}
		break;

	default:
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service DISPLAY => type '%d' inconnu ***",
								  p_msg_recu->entete.type_message);
		break;
	}
}

PROTECTED void AFFProcessStatusMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu)
{
	enum_aff_validity ret;

	// suivant le type de message recu 
	switch (p_msg_recu->entete.type_message)
	{
		// debut du service 
	case SRV_TYP_DEBUT:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Debut service ETAT ",
								 p_msg_recu->entete.neutre.bl_retour);
		// recherche si service disponible et acquittement 
		ret = AFFStartService(inst_id,
							  M_SRV_ETAT,
							  p_msg_recu->entete.neutre.bl_retour);
		if (ret == AFF_VALID)
		{
			// envoi d'un message d'etat 
			AFFSendStatus(inst_id,
						  p_msg_recu->entete.neutre.bl_retour,
						  SRV_TYP_DEMANDE);
		}
		break;

		// fin du service 
	case SRV_TYP_FIN:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Fin service ETAT ",
								 p_msg_recu->entete.neutre.bl_retour);
		// envoi d'un acquittement de fin de service 
		AFFEndService(inst_id,
					  M_SRV_ETAT,
					  p_msg_recu->entete.neutre.bl_retour);

		break;

	case SRV_TYP_DEMANDE:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Demande ETAT ",
								 p_msg_recu->entete.neutre.bl_retour);
		if (SrvEstDemandeur(gsDMV[inst_id].service[M_SRV_ETAT], p_msg_recu->entete.neutre.bl_retour) == TRUE)
		{
			// envoi d'un acquittement a l'application 
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_SRV_ETAT,
								 SRV_TYP_DEMANDE_ACQ);

			// envoi d'un message d'etat 
			AFFSendStatus(inst_id,
						  p_msg_recu->entete.neutre.bl_retour,
						  SRV_TYP_DEMANDE);
		}
		else
		{
			// envoi d'un non acquittement 
			AFFSendAcknowledgement(inst_id,
								   p_msg_recu->entete.neutre.bl_retour,
								   M_SRV_ETAT,
								   SRV_TYP_DEMANDE_NACQ);
		}
		break;

	default:
		// appel au fichier trace 
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service ETAT => type '%d' inconnu ***",
								  p_msg_recu->entete.type_message);
		break;
	}
}

PROTECTED void AFFProcessSpyMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu)
{
	enum_aff_validity ret;

	/* suivant le type de service recu */
	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Debut service ESPION ",
								 p_msg_recu->entete.neutre.bl_retour);

		/* test de la disponibilite du service et acquittement */
		ret = AFFStartSpyService(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 p_msg_recu->u.srv_espion.nature,
								 p_msg_recu->u.srv_espion.sens);

		break;

	case SRV_TYP_FIN:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Fin service ESPION ",
								 p_msg_recu->entete.neutre.bl_retour);

		/* acquittement de la fin de service */
		ret = AFFEndService(inst_id,
							M_SRV_ESPION,
							p_msg_recu->entete.neutre.bl_retour);
		break;

	default:
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service ESPION => type '%d' inconnu ***",
								  p_msg_recu->entete.type_message);
		break;
	}
}

PROTECTED void AFFProcessStopMsg(dmv_inst_id inst_id, struct_aff_message *p_msg_recu)
{
	/* suivant le type de service recu */
	switch (p_msg_recu->entete.type_message)
	{
	case SRV_TYP_DEMANDE:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', demande ARRET ",
								 p_msg_recu->entete.neutre.bl_retour);

		if (gsDMV[inst_id].stop_mbox == -1)
		{
			gsDMV[inst_id].stop_mbox = p_msg_recu->entete.neutre.bl_retour;

			// La demande d'arret est prise en compte
			AFFSendAcknowledgement(inst_id,
								 p_msg_recu->entete.neutre.bl_retour,
								 M_SRV_ARRET,
								 SRV_TYP_ARRET_ACQ);

			/* demande d'arret de la tache ios */
			AFFSendToIos(inst_id, M_ARRET, NULL);
		}
		else
			AFFSendAcknowledgement(inst_id,
			p_msg_recu->entete.neutre.bl_retour,
			M_SRV_ARRET,
			SRV_TYP_ARRET_NACQ);

		/* la tache ANI attend le compte-rendu d'arret de IOS */
		/* pour s'arreter aussi */
		break;

	default:
		/* appel du fichier trace */
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service ARRET => type '%d' inconnu, bal emettrice '%d', Bal '%d' ***",
								  p_msg_recu->entete.type_message,
								  p_msg_recu->entete.neutre.bl_retour,
								  p_msg_recu->entete.neutre.bl_retour);
		break;
	}
}

PROTECTED void AffManagement(dmv_inst_id inst_id, struct_aff_message *p_message, aff_infos *buffer)
{
	int index;
	int priorite = p_message->u.srv_affichage.demande.priorite;

	MInstTracer_FichierTrace(inst_id, "AFF_SERV: affichage du label : '%s', priorite '%d' ",
							 p_message->u.srv_affichage.demande.label,
							 priorite);

	if (priorite < 0 || priorite >= NB_MAX_AFF_PRIO)
	{
		/* La priorite n'est pas possible */
		MInstTrtacer_FichierError(inst_id, "AFF_SERV *** Service DISPLAY => la priorite '%d' est impossible ***",
								  priorite);
		return;
	}

	/* Si le message est autoris‚ */
	if (gsDMV[inst_id].sScreen.tab_request[priorite].bal == NO_BAL ||
		gsDMV[inst_id].sScreen.tab_request[priorite].bal == p_message->entete.neutre.bl_retour)
	{
		/* Recherche du label demand‚ */
		for (index = 0; index < NB_MAX_LABEL; index++)
		{
			if (gsDMV[inst_id].LabelTable[index].type == MSG_LABEL &&
				strcmp(gsDMV[inst_id].LabelTable[index].label,
				p_message->u.srv_affichage.demande.label) == 0)
				break;
		}

		/* Le label a ‚t‚ trouv‚ */
		if (gsDMV[inst_id].LabelTable[index].type == MSG_LABEL)
		{
			gsDMV[inst_id].sScreen.tab_request[priorite].bal = p_message->entete.neutre.bl_retour;

			/* Construire les nouvelles lignes */
			AFF_CreateLine(gsDMV[inst_id].char_conv_array,
						   &gsDMV[inst_id].sScreen.tab_request[priorite],
						   &gsDMV[inst_id].LabelTable[index],
						   buffer);

			gsDMV[inst_id].tab_credit[priorite] = buffer->infos.sale_fare_int.my_data.lData;
			gsDMV[inst_id].tab_paid[priorite] = buffer->infos.fare_int.my_data.lData;
			MInstTracer_FichierTrace(inst_id, "paid %lu credit_serv  %lu ", gsDMV[inst_id].tab_paid[priorite], gsDMV[inst_id].tab_credit[priorite]);

			/* Envoyer les nouvelles lignes … IOS */
			AFFSendToIos(inst_id, M_AFFICHAGE, (void *)priorite);
		}
		else
		{
			/* Le label n'a pas ‚t‚ trouv‚ */
			MInstTracer_FichierTrace(inst_id, "AFF_SERV *** Service DISPLAY => le label '%s' n'existe pas ***",
									 p_message->u.srv_affichage.demande.label);
		}
	}
	else
	{
		/* Le message n'est pas autoris‚ */
		MInstTracer_FichierTrace(inst_id, "AFF_SERV *** Service DISPLAY => la priorite '%d' est deja utilisee par la Bal '%d' ***",
								 priorite,
								 gsDMV[inst_id].sScreen.tab_request[priorite].bal);
	}
}

PROTECTED void ResetManagement(dmv_inst_id inst_id, struct_aff_message *p_message)
{
	int priorite = p_message->u.srv_affichage.reset.priorite;

	MInstTracer_FichierTrace(inst_id,
							 "AFF_SERV: reset de la priorite : '%d' ",
							 priorite);

	if (priorite < 0 || priorite >= NB_MAX_AFF_PRIO)
	{
		/* La priorite n'est pas possible */
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV *** Service RESET => la priorite '%d' est impossible ***",
								 priorite);
		return;
	}

	/* Si le reset est autoris‚ */
	if (gsDMV[inst_id].sScreen.tab_request[priorite].bal == p_message->entete.neutre.bl_retour)
	{
		gsDMV[inst_id].sScreen.tab_request[priorite].bal = NO_BAL;

		/* Envoyer les nouvelles lignes … IOS */
		AFFSendToIos(inst_id, M_AFFICHAGE, (void *)priorite);
	}
	else
	{
		if (gsDMV[inst_id].sScreen.tab_request[priorite].bal == NO_BAL)
		{
			/* La priorite n'est pas disponible */
			MInstTracer_FichierTrace(inst_id,
									 "AFF_SERV *** Service RESET => la priorite '%d' n'est pas utilis‚e ***",
									 priorite);
		}
		else
		{
			/* La priorite est d‚ja utilis‚e */
			MInstTracer_FichierTrace(inst_id, "AFF_SERV *** Service RESET => la priorite '%d' est d‚ja utilis‚e par la BAL %d ***",
									 priorite,
									 gsDMV[inst_id].sScreen.tab_request[priorite].bal);
		}
	}
}

PROTECTED void DimmingManagement(dmv_inst_id inst_id, struct_aff_message *p_message)
{
	boolean etat = p_message->u.srv_affichage.allume.etat;

	if (etat == TRUE)
	{
		MInstTracer_FichierTrace(inst_id, "AFF_SERV: Display backlight is ON");
	}
	else
	{
		MInstTracer_FichierTrace(inst_id, "AFF_SERV: Display backlight is OFF");
	}

	/* Envoyer les nouvelles lignes … IOS */
	AFFSendToIos(inst_id, M_ALLUME, (void *)etat);
}

PROTECTED void AFFProcessDeviceInfoMsg(dmv_inst_id inst_id, struct_aff_message *p_message)
{
	// Message type
	switch (p_message->entete.type_message)
	{
	case SRV_TYP_DEBUT:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Debut service DEVICE INFO ",
								 p_message->entete.neutre.bl_retour);


		AFFStartService(inst_id,
						M_SRV_DEVICE_INFO,
						p_message->entete.neutre.bl_retour);
		break;

	case SRV_TYP_FIN:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', Fin service DEVICE INFO ",
								 p_message->entete.neutre.bl_retour);

		AFFEndService(inst_id, M_SRV_DEVICE_INFO, p_message->entete.neutre.bl_retour);
		break;

	case SRV_TYP_GET:
		MInstTracer_FichierTrace(inst_id,
								 "AFF_SERV: Bal '%d', demande DEVICE INFO - SRV_TYP_GET ",
								 p_message->entete.neutre.bl_retour);

		if (SrvEstDemandeur(gsDMV[inst_id].service[M_SRV_DEVICE_INFO],
			p_message->entete.neutre.bl_retour) == TRUE)
		{
			AFFSendAcknowledgement(inst_id,
								 p_message->entete.neutre.bl_retour,
								 M_SRV_DEVICE_INFO,
								 SRV_TYP_GET_ACQ);

			AFFSendToIos(inst_id, SRV_TYP_GET, p_message);
		}
		else
		{
			AFFSendAcknowledgement(inst_id,
								 p_message->entete.neutre.bl_retour,
								 M_SRV_DEVICE_INFO,
								 SRV_TYP_GET_NACQ);
		}
		break;
	}
}