/*------   (v) 1998 CS-Route   -----------   Droits reserves   ------*/

/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE: LAN
* FICHIER: LAN_COM.C
* LANGAGE: C
* --------------------------------------------------------------------
* RESUME: Code des fonctions propre a la communcation LAN_IP
* --------------------------------------------------------------------
* DESCRIPTION:
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/MODULO/VoieNt/Lan/Pipe/sources/lan_com.c_v  $
 * 
 *    Rev 1.1   Sep 08 2000 14:49:52   bph
 *  
 * 
 *    Rev 1.0   Nov 22 1999 14:55:16   PGG
 * Checked in from initial workfile by PVCS Version Manager Project Assistant.
* 
*    Rev 1.1   02 Oct 1998 11:45:50   bph
*  
* 
*    Rev 1.16   05 Feb 1998 10:37:56   BPH
* Correction Arret du chrono de vie
* 
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ---------------*/
#include <stdio.h>
#include <dos.h>
#include <conio.h>
#include <string.h>

/* module NOYAU */
#include <noyau.h>

#include <csr_lan.h>
#include <lan_glob.h>
#include <lan_mess.h>
#include <atm_lan.h>

//#include <console.h>

/*--------------- RESERVED: ---------------*/
#include <memclass.h>

/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/
/*--------------- FUNCTIONS: ---------------*/
PROTECTED void LanErreurRecep(short int ident);




/*--------------- VARIABLES: ---------------*/
/*--------------- CODE: ---------------*/
/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED unsigned short int LanConnect()
* PARAMETRES: aucun
* RETOUR:    ??????
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Assure l'ouverture d'un pipe et la connexion
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_erreur LanConnect(short int ident, struct_lan_parametre *
      lan_info)
{
   SYSTEMTIME today;
   DWORD mode_pipe;
   short int offset = lan_info->num_msg;
   
   /*******************************************/
   /* Initialisation du pipe de communication */
   /*******************************************/
   
   // création du pipe distant
   lan_info->pipe = CreateFile(LAN[ident].param.nom_distant[offset], 
         GENERIC_READ |GENERIC_WRITE, FILE_SHARE_WRITE |FILE_SHARE_READ, NULL, 
         OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL |FILE_FLAG_WRITE_THROUGH |
         FILE_FLAG_OVERLAPPED, NULL);
   
   if (lan_info->pipe == INVALID_HANDLE_VALUE)
   {
      LanFichierDebug(ident, 
            "LAN COM : Echec sur connection du pipe %s (Err = %lu)", LAN[ident].
            param.nom_distant[offset], GetLastError());
      
      return LAN_NOK;
   }
   
   // modification de ces attributs
   mode_pipe = PIPE_READMODE_MESSAGE | PIPE_WAIT;
   
   if (!SetNamedPipeHandleState(lan_info->pipe, &mode_pipe, NULL, NULL))
   {
      LanFichierDebug(ident, 
            "LAN COM : Echec sur modification du pipe %s en mode message (Err = %lu)"
            , LAN[ident].param.nom_distant[offset], GetLastError());
      
      CloseHandle(lan_info->pipe);
      lan_info->pipe = LAN_NO_PIPE_OPENED;
      
      return LAN_NOK;
   }
   
   /*** Récupération de la date et de l'heure */
   GetLocalTime(&today);
   LanFichierTrace(ident, "*** Connexion le %2d/%2d/%4u a %02d-%02d-%02d ***", 
         today.wDay, today.wMonth, today.wYear, today.wHour, today.wMinute, 
         today.wSecond);
   
   return LAN_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: PROTECTED unsigned short int LanDisconnect(int sock)
* PARAMETRES: sock : identificateur du Socket
* RETOUR:    ???
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Assure la deconnexion et la fermeture du pipe sock
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_erreur LanDisconnect(short int ident, struct_lan_parametre *
      lan_info)
{
   enum_lan_erreur retour;
   SYSTEMTIME today;
   
   if (lan_info->pipe == LAN_NO_PIPE_OPENED)
   {
      LanFichierTrace(ident, "LanDisconnect => pipe '%ld' déja fermée !", 
            lan_info->pipe);
      retour = LAN_OK;
   }
   else
   {
      
      /* deconnexion = fermeture du pipe */
      if (CloseHandle(lan_info->pipe) == TRUE)
      {
         lan_info->pipe = LAN_NO_PIPE_OPENED;
         
         /*** Récupération de la date et de l'heure */
         GetLocalTime(&today);
         LanFichierTrace(ident, 
               "*** Deconnexion le %2d/%2d/%4u a %02d-%02d-%02d ***", today.wDay
               , today.wMonth, today.wYear, today.wHour, today.wMinute, today.
               wSecond);
         retour = LAN_OK;
      }
      else
      {
         LanFichierDebug(ident, "*** echec fermeture pipe '%ld' ***", lan_info->
               pipe);
         retour = LAN_NOK;
      }
   }
   
   return(retour);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void LanBreak (short int ident)
* PARAMETRES: identificateur
* RETOUR:     aucun
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Coupe la connexion et lance un chrono pour un nouvelle connexion
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanBreak(short int ident)
{
   LanFichierTrace(ident, "LAN BREAK");
   
   ATM_LAN_Break(LAN[ident].atm_lan);
}




/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_lan_erreur LanShutDown()
* PARAMETRES: aucun
* RETOUR:
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Arret de toute communication reseau
* -------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_erreur LanShutDown(short int ident)
{
   LanFichierTrace(ident, "LAN SHUTDOWN");
   
   ATM_LAN_Shutdown(LAN[ident].atm_lan);
   
   return LAN_OK;
}


/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_lan_erreur LanTransmit(int sock,
*                                            char *buffer,
*                                            int longueur)
* PARAMETRES: sock : identicateur du Socket
*             buffer : adresse du message a emettre
*             longueur: taille du message a emettre
* RETOUR: transmission correcte ou non.
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Assure l'emission d'un message conformement au protocole LAN/IP
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_erreur LanTransmit(short int ident, HANDLE hPipe, 
      struct_lan_msg *buffer)
{
   enum_lan_erreur retour;
   DWORD ecrits;
   OVERLAPPED * pOver = &LAN[ident].param.wOver;
   short int offset = LAN[ident].param.num_msg;
   
   // RAZ de la struct overlapped
   memset(pOver, 0, sizeof(OVERLAPPED));
   pOver->hEvent = LAN[ident].param.wEvent;
   EffaceEvt(pOver->hEvent);
   
   
   // Ecriture du msg
   if (!WriteFile(hPipe, buffer, buffer->prot.MsgSize + sizeof(struct_prot), &
            ecrits, pOver))
      if (GetLastError() == ERROR_IO_PENDING)
         GetOverlappedResult(hPipe, pOver, &ecrits, TRUE);
   
   
   // si ecriture NOK
   if (ecrits != (buffer->prot.MsgSize + sizeof(struct_prot)))
   {
      LanFichierTrace(ident, "TRANSMIT : erreur %ld , pipe %s ", GetLastError(), 
            LAN[ident].param.nom_distant[offset]);
      retour = LAN_NOK;
   }
   else
   {
      retour = LAN_OK;
   }
   
   return(retour);
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE: enum_lan_erreur LanReceive(int sock,
*                                       char *buffer ,
*                                       int longueur)
* PARAMETRES: sock : identificateur du Socket
*             buffer : buffer de reception
*             longueur : longueur du message attendu
* RETOUR: Reception correcte ou non
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Assure la reception d'un message conformement au protocole LAN/IP
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED enum_lan_erreur LanReceive(short int ident, HANDLE hPipe, 
      struct_lan_msg *buffer, DWORD longueur, DWORD Timeout)
{
   DWORD lus;
   OVERLAPPED * pOver = &LAN[ident].param.rOver;
   short int offset = LAN[ident].param.num_msg;
   static flag_timedout = FALSE;
   
   lus = 0;
   
   if (flag_timedout == FALSE)
   {
      // RAZ de la struct overlapped
      memset(pOver, 0, sizeof(OVERLAPPED));
      pOver->hEvent = LAN[ident].param.rEvent;
      EffaceEvt(pOver->hEvent);
      
      
      // attente d'un message
      if (!ReadFile(hPipe, buffer, longueur, &lus, pOver))
      {
         if (GetLastError() != ERROR_IO_PENDING)
            return LAN_NOK;
      }
   }
   
   
   // on réalise une attente temporisée
   if (WaitForSingleObject(pOver->hEvent, Timeout) == WAIT_TIMEOUT)
   {
      flag_timedout = TRUE;
      return LAN_TIMEOUT;
   }
   
   flag_timedout = FALSE;
   
   
   // dans tous les cas (timeout ou ok), on récupère le résultat
   if (GetOverlappedResult(hPipe, pOver, &lus, FALSE) == FALSE)
      return LAN_NOK;
   
   if (lus < sizeof(struct_prot) || buffer->prot.MsgSize != (lus - sizeof(
         struct_prot)))
   {
      LanFichierTrace(ident, "RECEIVE => erreur %ld sur pipe %s ", GetLastError(
            ), LAN[ident].param.nom_distant[offset]);
      
      return LAN_NOK;
   }
   
   return LAN_OK;
}



/**/
/*
* $D_FCTN
* --------------------------------------------------------------------
* SYNTAXE:    PROTECTED void LanTraitementErreur (void);
* PARAMETERS: aucun
* RETOUR: rien
* --------------------------------------------------------------------
* VARIABLES:
* --------------------------------------------------------------------
* TYPE: fonction locale
* ROLE: Traitement lors d'un debordement de buffer LAN/IP  (IT3)
* --------------------------------------------------------------------
* $F_FCTN
*/
PROTECTED void LanTraitementErreur(void)
{
}
