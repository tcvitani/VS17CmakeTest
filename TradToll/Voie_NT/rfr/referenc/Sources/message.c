/*------   (v) 1995 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  Traitement des messages
* FICHIER: message.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME:
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* --------------------------------------------------------------------
* HISTORIQUE:
 *
 * $Log:   T:/MODULO/VoieNt/Referenc/Sources/message.c_v  $
 * 
 *    Rev 1.0   14 Dec 1999 15:17:50   afx
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
 * 
 *    Rev 1.3   03 Nov 1997 16:07:28   DPI
 * Version 4.00
 * 
 *    Rev 1.2   29 Oct 1997 19:13:52   DPI
 * Suppression de l'include "noyau2.h"
 * 
 *    Rev 1.1   Jun 04 1997 19:13:12   DPI
 *  
 * 
 *    Rev 1.0   Apr 24 1997 15:02:18   DPI
 *  
 *
* --------------------------------------------------------------------
* $F_HEAD
*/

#define  MESSAGE_DEF

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>

#include "err.h"
#include "fic.h"
#include "str.h"

#include "rc_def.h"
#include "fic_gere.h"
#include "referenc.h"

#include "fic_msg.h"
#include "fic_act.h"

#include "rc_glob.h"

#include "action.h"
#include "message.h"

/*--------------- RESERVED: ---------------*/

#include "memclass.h"

/*--------------- RESERVED: ---------------*/

/*--------------- PRIVATE: ---------------*/
PRIVATE void      InitialiserMessage( TReceptionMessage  *Message,
                                      noyau_bal_id       Bal,
                                      TTacheMessage      TacheMessage);


/*--------------- FUNCTION: ---------------*/

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   AttendreMessage
* PARAMETRES: - le contexte de l'action
*             - la date et heure de l'message voulue
*             - l'identifiant de l'message en retour
*             - la fonction a appeler sur declenchement du chrono
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Enregistre une nouvelle message si aucune n'est d‚j… attendue
* --------------------------------------------------------------------
* NOTA : L'identifiant de l'message en retour est en fait la position
*        de l'action.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT boolean WINAPI AttendreMessage( TpTIdContext     IdContext,
                                      noyau_bal_id     Bal,
                                      TTacheMessage    Message,
                                      TIdMessage       *IdMessage,
                                      TFcn_DoIt        Faire)
{
   FIC_enum_retour      retour;
   TpTTacheContext      tacheContext;
   TReceptionMessage     message;
   TReceptionMessage     *message_lue;

   tacheContext = ( TpTTacheContext) IdContext->TacheContext;

   /* existe-t'il une message pour cette date et heure */
   retour = LireIdMessage( tacheContext->GereMessages, Bal, Message,
                           &message_lue);
   if( retour != FIC_OK)
   {
     InitialiserMessage( &message, Bal, Message);
     retour = AjouterPosMessage( tacheContext->GereMessages, &message, IdMessage);
     ERR_EstVrai( retour == FIC_OK);

     /* il faut lire l'message pour valoriser son IdMessage */
     retour = LireIdMessage( tacheContext->GereMessages, Bal, Message,
                             &message_lue);
     ERR_EstVrai( retour == FIC_OK);
   }

   EnregistrerActionMessage( tacheContext, IdContext, Faire,
                             message_lue->Position,
                             &(message_lue->PremiereAction),
                             &(message_lue->DerniereAction));

   *IdMessage = message_lue->DerniereAction;

   retour = EcrirePosMessage( tacheContext->GereMessages,
                              message_lue->Position,
                              message_lue);
   ERR_EstVrai( retour == FIC_OK);

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   RecevoirMessage
* PARAMETRES: - le contexte de l'action
*             - l'identifiant de l'message externe
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PUBLIC boolean       RecevoirMessage( TpTTacheContext  Context,
                                      noyau_bal_id     Bal,
                                      TTacheMessage    Message)
{
   FIC_enum_retour      retour;
   TReceptionMessage     *message_lue;

   /* existe-t'il une message pour cette date et heure */
   retour = LireIdMessage( Context->GereMessages, Bal, Message,
                           &message_lue);
   if( retour != FIC_OK)
     return(FALSE);

   Context->MessageEnCours = message_lue->Position;

   return( TRUE);
}

/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   OublierMessage
* PARAMETRES: - le contexte de l'action
*             - l'identifiant de l'message
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE:
* ROLE: Permet de supprimer une message sans declencher l'action
* --------------------------------------------------------------------
* NOTA : L'identifiant de l'message en retour est en fait la position
*        de l'action.
* --------------------------------------------------------------------
* $F_FCTN
*/
EXPORT boolean WINAPI OublierMessage( TpTIdContext     IdContext,
                                    TIdMessage        *IdMessage)
{
   FIC_enum_retour      retour;
   TpTTacheContext      tacheContext;
   TReceptionAction     *action_lue;
   TReceptionMessage     *message_lue;
   long                 indiceMessage;
   boolean              encore;

   tacheContext = ( TpTTacheContext) IdContext->TacheContext;

   if ( *IdMessage == NO_ALARME)
      return( FALSE);

   /* lire l'action */
   retour = LirePosAction( tacheContext->GereActions, *IdMessage, &action_lue);
   ERR_EstVrai( retour == FIC_OK);

   /* verifier que c'est une message */
   if( action_lue->TypeAction != MESSAGE)
      return( FALSE);

   /* stocker l'indice de l'message pour savoir si elle n'est plus attendue
    */
   indiceMessage = action_lue->IndiceType;
   retour = LirePosMessage( tacheContext->GereMessages,
                           indiceMessage, &message_lue);

   retour = AbandonnerUneAction( tacheContext, *IdMessage, FALSE);
   *IdMessage = NO_ALARME;

   if( retour !=  FIC_OK)
      return( FALSE);

   /* reste-t'il une action pour cette date et heure ? */
   retour = LirePosMessage( tacheContext->GereMessages,
                           indiceMessage, &message_lue);
   encore = ( message_lue->PremiereAction != NO_ACTION);
   if( encore)
   {
      retour = EcrirePosMessage( tacheContext->GereMessages, message_lue->Position, message_lue);
      ERR_EstVrai( retour == FIC_OK);
   }
   else
   {
      retour = LibererPosMessage( tacheContext->GereMessages, message_lue->Position);
      ERR_EstVrai( retour == FIC_OK);
   }

   return( retour !=  FIC_NOK);
}




/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/
/*=======================================================================*/


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE :   InitialiserMessage
* PARAMETRES: - l'message
*             - la date et heure de l'message voulue
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: Private
* ROLE:
* --------------------------------------------------------------------
* $F_FCTN
*/
PRIVATE void      InitialiserMessage( TReceptionMessage  *Message,
                                      noyau_bal_id       Bal,
                                      TTacheMessage      TacheMessage)
{
   Message->Etat  = WAITING_MESSAGE;
   Message->Bal = Bal;
   Message->Message = TacheMessage;
   Message->PremiereAction = NO_ACTION;
   Message->DerniereAction = NO_ACTION;
}
