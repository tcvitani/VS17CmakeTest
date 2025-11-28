/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD 
* --------------------------------------------------------------------
* MODULE: DLL SIMU
* FICHIER: SIMU_SRV.C
* LANGAGE: C
* --------------------------------------------------------------------
* DESCRIPTION: La tache SIMU_Srv est la tache serveur de communication
*              par BAL distante.
* --------------------------------------------------------------------
* $F_HEAD
*/

/*--------------- INCLUDES: ------------*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <noyau.h>

#include <debug.h>

#include <csr_lan.h>

#include <simu.h>
#include <simu_glo.h>

/*--------------- RESERVED: ---------------*/
#include "memclass.h"
/*--------------- EXTERNALS: ---------------*/
/*--------------- DEFINES: ---------------*/
/*--------------- TYPEDEFS: ---------------*/

typedef enum
{
    PROT_APPLICATIF,
    PROT_DEMANDE_VIE,
    PROT_REPONSE_VIE
}
enum_prot_type;

typedef struct
{
    DWORD MsgSize;
    enum_prot_type MsgType;
    LONGLONG MsgDate;
} 
struct_prot;

typedef struct
{
    struct_prot prot;
    BYTE data[LAN_LG_TRAME];
} struct_lan_msg;

/*--------------- FUNCTIONS: ---------------*/

PRIVATE void SimuReception (HANDLE hPipe);
PRIVATE BOOL SimuTransmit(HANDLE hPipe, DWORD MsgSize, enum_prot_type MsgType, BYTE *data);

/*--------------- CODE: ---------------*/

//
// !!!!!!!!!!!!!!!!!  ATTENTION   !!!!!!!!!!!!!!!
//
// Le serveur ne doit pas avoir de user déclaré (et pas forcément loggé)
// du même nom que le user du client mais avec un mot de passe différent
// sinon la connection est refusée coté client avec l'erreur 1326.
// (attention également au mot de passe identique mais expiré !)
// En revanche, si le serveur ne posséde pas du tout de user déclaré
// correspondant à celui du client, la connection est acceptée à condition d'avoir
// un compte INVITE SANS MOT DE PASSE.
// 
// En résumé, la connection \\serveur\ipc$ (visible avec un "net use") tente de
// s'établir vers le serveur avec le compte utilisateur loggé sur le client. 
// Si un même nom de compte existe sur le serveur mais avec un mot de passe différent 
// ou expiré, la connection échoue. Si le compte n'existe pas, la connection est tentée
// en tant qu'invité sans mot de passe. Simple, non ?
//

// serveur de connection par pipe
PROTECTED DWORD WINAPI SimuSrvPipe (PVOID param)
{
    HANDLE hPipe;
    BOOL bRet;
    SECURITY_ATTRIBUTES SA;
    PSECURITY_DESCRIPTOR pSD;
    OVERLAPPED over;
    HANDLE hEvent;
    DWORD Err, dummy;
    
    // Création d'un event pour l'overlapped 
    if (AlloueEvent(&hEvent, NULL) != NOYAU_OK)
        ExitBad();
    
    // changement de priorité du thread
    ChangePriorite( TacheCourante() , SIMU.priorite_tache) ;
    
    // Création d'un descripteur de securite autorisant l'acces au pipe à tout le monde
    if ( (pSD = (PSECURITY_DESCRIPTOR) malloc (SECURITY_DESCRIPTOR_MIN_LENGTH)) == NULL)
        ExitBad();
    
    if (!InitializeSecurityDescriptor (pSD, SECURITY_DESCRIPTOR_REVISION))
        ExitBad();
    
    if (!SetSecurityDescriptorDacl (pSD, TRUE, (PACL) NULL, FALSE))
        ExitBad();
    
    // initialisation de l'attribut de sécurité avec le pipe
    // donnant acces a tout le monde
    memset(&SA, 0, sizeof(SA));
    SA.nLength = sizeof(SA);
    SA.lpSecurityDescriptor = pSD;
    SA.bInheritHandle = TRUE;
    
    while (TRUE)
    {	
        // Création d'un pipe bidirectionnel asynchrone en mode message
        hPipe = CreateNamedPipe (
            "\\\\.\\pipe\\CSR_LC_Interface",
            PIPE_ACCESS_DUPLEX|FILE_FLAG_WRITE_THROUGH|FILE_FLAG_OVERLAPPED,
            PIPE_TYPE_MESSAGE|PIPE_READMODE_MESSAGE|PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES,
            2048,
            2048,
            INFINITE,
            &SA);
        
        if (hPipe == INVALID_HANDLE_VALUE)
            ExitBad();
        
        // RAZ de la struct overlapped
        memset(&over, 0, sizeof(over));
        over.hEvent = hEvent;
        
        // Attente bloquante d'une connection
        bRet = TRUE;
        if (!ConnectNamedPipe(hPipe, &over))
        {
            Err = GetLastError();
            if (Err == ERROR_IO_PENDING)
            {
                if (!GetOverlappedResult (hPipe, &over, &dummy, TRUE))
                    bRet = FALSE;
            }
            else if (Err != ERROR_PIPE_CONNECTED)
                bRet = FALSE;
        }
        
        if (!bRet) 
            ExitBad();
        
        SimuReception (hPipe);
    }

    if (LibereEvent (&hEvent) != NOYAU_OK)
        ExitBad();
    
    return 0;
}


PRIVATE void SimuReception (HANDLE hPipe)
{
    struct_lan_msg msg;
    DWORD lus;
    OVERLAPPED over;
    HANDLE hEvent;
    DWORD Err;
    DWORD Count;

    // Création d'un event pour l'overlapped 
    if (AlloueEvent(&hEvent, NULL) != NOYAU_OK)
        ExitBad();
    
    // RAZ de la struct overlapped
    memset(&over, 0, sizeof(over));
    over.hEvent = hEvent;

    Count = 0;
    while( TRUE )
    {
        // attente d'un message
        if (!ReadFile (hPipe, &msg, sizeof(msg), &lus, &over))
        {
            Err = GetLastError();
            if (Err == ERROR_IO_PENDING)
            {
                // on réalise une attente temporisée
                Err = WaitForSingleObject(over.hEvent, 10000);
        
                // dans tous les cas (timeout ou ok), on récupère le résultat
                if (Err != WAIT_TIMEOUT && GetOverlappedResult (hPipe, &over, &lus, FALSE) == FALSE)
                    // sortie du while
                    break;
            }
            else if (Err != ERROR_SUCCESS)
                // sortie du while
                break;
        }
        

        if (Err == WAIT_TIMEOUT)
        {
            if (Count < 3)
            {
                Err = SimuTransmit (hPipe, 0, PROT_DEMANDE_VIE, NULL);
                Count ++;
            }
            else
                Err = FALSE;
        }
        else
        {
            Count = 0;
            switch (msg.prot.MsgType)
            {
            case PROT_DEMANDE_VIE:
                Err = SimuTransmit (hPipe, 0, PROT_REPONSE_VIE, NULL);
                break;

            case PROT_APPLICATIF:
                Err = SimuTransmit (hPipe, msg.prot.MsgSize, PROT_APPLICATIF, msg.data);
                break;

            case PROT_REPONSE_VIE:
                Err = TRUE;
                break;

            default:
                ExitBad();
                break;
            }
        }

        if (Err == FALSE)
            // sortie du while
            break;
    }   
    
    // libération des ressources
    CloseHandle(hPipe);
    hPipe = INVALID_HANDLE_VALUE;
    
    if (LibereEvent (&hEvent) != NOYAU_OK)
        ExitBad();
}


PRIVATE BOOL SimuTransmit(HANDLE hPipe, DWORD MsgSize, enum_prot_type MsgType, BYTE *data)
{
    struct_lan_msg message;
    BOOL retour;
    DWORD ecrits;
    OVERLAPPED Over;
    HANDLE hEvent;

    /* initialisation */
    memset(&message,0, LAN_LG_TRAME);

    message.prot.MsgSize = MsgSize;
    message.prot.MsgType = MsgType;
    GetSystemTimeAsFileTime ((FILETIME *) &message.prot.MsgDate);
    if (MsgSize > 0)
        memcpy(&message.data, data, MsgSize);
    
    // Création d'un event pour l'overlapped 
    if (AlloueEvent(&hEvent, NULL) != NOYAU_OK)
        ExitBad();
    
    // RAZ de la struct overlapped
    memset(&Over, 0, sizeof(OVERLAPPED));
    Over.hEvent = hEvent;

   // Ecriture du msg
   if (!WriteFile (hPipe, &message, MsgSize + sizeof(struct_prot), &ecrits, &Over) )
       if (GetLastError() == ERROR_IO_PENDING)
           GetOverlappedResult (hPipe, &Over, &ecrits, TRUE);
                
   // si ecriture NOK
   if (ecrits != (MsgSize + sizeof(struct_prot)) )
      retour = FALSE;
   else
      retour = TRUE;

   if (LibereEvent (&hEvent) != NOYAU_OK)
       ExitBad();

   return( retour );
}