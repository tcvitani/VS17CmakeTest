/* --------------------------------------------------------------------
 * (C) 2000 CS Route - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : Commoc
 * FILE       : cm_callback.c
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : 
 * --------------------------------------------------------------------
 * DESCRIPTION: Fonctions callback pour la gestion de la communication
 *              dans le service.
 * --------------------------------------------------------------------
 * HISTORY    : 
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#include <windows.h>
#include <stdio.h>
#include <acom.h>
#include <ntsvc.h>
#include <col.h>

#include <cmhost.h>
#include <cmwork.h>

#include <cm_glob.h>
#include <cm_text.h>
#include <cm_cmd.h>


#define LOC_DEF
#include <cm_callback.h>
#undef LOC_DEF

#include <memclass.h>




//
// CODE DES FONCTIONS PROTEGEES
//

/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackShutdown(
 *                     DWORD64 dwWksUsrKey,
 *                     DWORD dwError )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Appelé lorsque la com va mal. Cela provoque un arrêt du service.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackShutdown(
		DWORD64 dwWksUsrKey,
        DWORD dwError )
{
    SVC_ERR( ERROR_INVALID_DATA, "ERR_COMM_SHUTDOWN" );
    RaiseException( 0xE0000001, EXCEPTION_NONCONTINUABLE, 0, NULL );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED DWORD64 CALLBACK CallbackConnection(
 *                     DWORD dwInstUsrKey,
 *                     ACOM_CNX_HANDLE hCnxHandle )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Cf. doc ACOM
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se connecter. dwInstUsrKey désigne
 *             le type de la connexion ( LS/LC/CMD).
 * --------------------------------------------------------------------
 */
PROTECTED DWORD64 CALLBACK CallbackConnection(
		DWORD64 dwInstUsrKey,
        ACOM_CNX_HANDLE hCnxHandle )
{
    DWORD dwErr = NO_ERROR;

    NTSVCInfo( "CallbackConnection(), connexion d'une interface de commande" );

    return hCnxHandle;
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackDisconnection(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Un client vient de se déconnecter.
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackDisconnection(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey)
{
    NTSVCInfo( "CallbackDisconnection(), déconnexion d'une interface de commande" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void CALLBACK CallbackReceived(
 *                     DWORD64 dwInstUsrKey,
 *                     DWORD64 dwCnxUsrKey,
 *                     DWORD dwMsgSize,
 *                     BYTE * pbMsg )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : On vient de recevoir un message d'un client
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackReceived(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
        DWORD dwMsgSize,
        BYTE * pbMsg )
{

    NTSVCInfo( "CallbackReceived(), réception d'un message d'une interface de commande" );
    
    // Le message est une chaine de caractère sans marqueur de fin.
    // Dans la mesure ou le traitement de la commande prévoit qu'il n'y
    // ai pas de marqueur de fin, inutile d'en rajouter un ...
    // Allouer un bloc suffisament grand pour pouvoir placer un marqueur de fin de chaine
    CmdReceive( (ACOM_CNX_HANDLE)dwCnxUsrKey, dwMsgSize, pbMsg );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    :PROTECTED void CALLBACK CallbackSent(
 *                    DWORD64 dwInstUsrKey,
 *                    DWORD64 dwCnxUsrKey,
 *                    DWORD64 dwMsgUsrKey,
 *                    DWORD dwError )
 * --------------------------------------------------------------------
 * PARAMETERS: Cf. doc ACOM
 * --------------------------------------------------------------------
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : On vient d'envoyer un message
 * --------------------------------------------------------------------
 */
PROTECTED void CALLBACK CallbackSent(
		DWORD64 dwInstUsrKey,
		DWORD64 dwCnxUsrKey,
		DWORD64 dwMsgUsrKey,
        DWORD dwError )
{
    NTSVCInfo( "CallbackSent(), acquittement, code=%u", dwError );
}

