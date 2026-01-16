/*------   (v) 1999 CSEE-Peage   ---------   Droits reserves   ------*/
/*
* $D_HEAD
* --------------------------------------------------------------------
* MODULE:  BOUCLE PRINCIPALE DE RECEPTION DES MESSAGE
* FICHIER: srvpv_wm_dispatch.c
* LANGAGE: C
* --------------------------------------------------------------------
* MOT-CLE:
* --------------------------------------------------------------------
* RESUME: Ce module se charge d'interpréter les window messages
* --------------------------------------------------------------------
* HISTORIQUE:
* $Log:   T:/PRODUITS/LS/TCO/commun/PV/Serveur/srvpvsvc/sources/route_wm.c_v  $
*
*    Rev 1.6   26 Apr 2002 11:44:58   dsilberm
* Version issu des modif apportees sur
* dartford concernant les classes en
* temps reelles
*
*    Rev 1.4   19 Dec 2000 15:10:12   DSI
* J-1 + filtre des classes et des paiements et
* macro du strncpy
*
*    Rev 1.3   13 Dec 2000 16:34:50   DSI
* Ajout de la possibilite de crypter les mots
* de passe
*
*    Rev 1.0   Nov 22 1999 11:00:38   PGG
* Checked in from initial workfile by PVCS Version Manager Project Assistant.
* --------------------------------------------------------------------
* $F_HEAD
*/

#include <windows.h>
#include <ntsvc.h>
#include <route_wm.h>
#include <route_aut.h>
#include <plaza.h>
#include <alarm.h>
#include <srvpv_main.h>
#include <route_msg.h>
#include <pv_wm.h>
#include <msg_sv_con_rep.h>
#include <msg_pv_ack_req.h>
#include <msg_pv_com_req.h>
#include <msg_lc_header.h>
#include <msg_lc_command.h>
#include <msg_lc_comp_inf_tr.h>
#include <csr_des.h>
//#include <openssl/conf.h>
//#include <openssl/evp.h>
//#include <openssl/err.h>
//#include <openssl/sha.h>


#include <pvhook.h>

/*--------------- EXTERNALS: ---------------*/
#include <memclass.h>

/*--------------- DEFINES: -----------------*/

// Nom texte du window message utilisé pour signaler les événements
#define ROUTE_WM_EVENT_NAME		"WM_ROUTE_EVENT"
#define MAX_MSG_SIZE			32000
#define _PARAM					"_Param"
#define _PARAM_DEFAULT			3
#define TMP1_LEN				20
#define TMP2_LEN				24
#define DATA_LEN				512
#define REF1_LEN				32
#define REF2_LEN				16

/*--------------- TYPEDEFS: ----------------*/

/*--------------- VARIABLES INITIALISEES (1): ---------------*/

PRIVATE struct
{
	aut_route_id	aut[ROUTE_ACOM_MAX_CONNECTIONS];

	HLIST			hlRouteMsgList;

	UINT			uiCleanTrsStatTimerId;

	HANDLE			hThread;
	DWORD			ThreadId;
	DWORD			WM_ROUTE;
	DWORD			WM_ACOM;
}
ROUTE = { 0 };

//DWORD dwParam = 0;
//
//BYTE data1[] = { 0x29, 0xD8, 0xC5, 0x42, 0x70, 0xA5, 0xBA, 0x8D, 0x52, 0xF4, 0x2B, 0x60, 0xA6, 0xD5, 0x39, 0x69, 0x2F, 0x88, 0x6E, 0x1C,
//0x66, 0x38, 0xA0, 0x02, 0x7A, 0xE6, 0x18, 0x54, 0x96, 0xA3, 0xF4, 0xC8, 0x62, 0x02, 0xCA, 0xAB, 0xCC, 0xCD, 0x8E, 0x30,
//0x08, 0xF7, 0xB6, 0x08, 0xD5, 0xD2, 0x9B, 0x32, 0x03, 0x82, 0x23, 0x15, 0x32, 0xB2, 0x9D, 0xCC, 0xDC, 0x85, 0xEC, 0xC1,
//0x96, 0xCF, 0xE8, 0x7D, 0x92, 0x86, 0x01, 0xBE, 0x69, 0x73, 0x26, 0xCA, 0x17, 0x2D, 0xE5, 0x04, 0x47, 0x48, 0xD3, 0xBD,
//0xD2, 0xCA, 0x2E, 0x4C, 0x82, 0xDE, 0xC1, 0x61, 0x1B, 0xCD, 0x08, 0x96, 0x9C, 0x29, 0x95, 0x47, 0xC2, 0x4C, 0xB2, 0xA4,
//0xCE, 0x01, 0x48, 0x04, 0x88, 0x80, 0xFF, 0x7D, 0x84, 0xA0, 0xA7, 0xB1, 0xA4, 0x48, 0xD7, 0xEA, 0xDE, 0x26, 0xD3, 0x6A,
//0x3F, 0xA4, 0xB3, 0xFC, 0x30, 0x18, 0xC2, 0xCF, 0xF9, 0x3B, 0x25, 0x84, 0x83, 0xDD, 0xBD, 0x86, 0x62, 0xD7, 0x4E, 0x9A,
//0x12, 0x40, 0xE0, 0xB7, 0x81, 0xE4, 0x65, 0x17, 0xCE, 0xF1, 0x9D, 0x8E, 0x51, 0xF4, 0x37, 0x1B, 0xDB, 0x04, 0x39, 0xBD,
//0x3F, 0x3D, 0xA1, 0x87, 0x11, 0x71, 0x4C, 0xF7, 0xAD, 0x19, 0x18, 0x84, 0x7B, 0x92, 0x46, 0x76, 0x72, 0x86, 0x22, 0xFA,
//0xDB, 0xA8, 0xE5, 0x76, 0x58, 0xA4, 0xDA, 0x1A, 0x24, 0x55, 0xC0, 0xB8, 0x9C, 0x39, 0x70, 0xD5, 0xFF, 0x23, 0xBF, 0x00,
//0x72, 0x30, 0xA6, 0x62, 0x47, 0xFC, 0xA0, 0x97, 0x7E, 0x16, 0x54, 0xBC, 0x8C, 0xB1, 0x74, 0xC9, 0x5C, 0x9C, 0xE5, 0x21,
//0x94, 0x74, 0x20, 0x68, 0x19, 0x18, 0xD8, 0x95, 0xD5, 0x5D, 0xDD, 0x48, 0xB8, 0x03, 0x40, 0x5D, 0xFC, 0xE8, 0xB8, 0xC1,
//0xAE, 0x7C, 0x44, 0x84, 0x6A, 0x75, 0x12, 0x30, 0x66, 0xC4, 0xA0, 0x5A, 0xB1, 0xC7, 0x21, 0x76, 0xBD, 0xC0, 0x22, 0xF0,
//0x1C, 0x86, 0x6D, 0x78, 0x87, 0x7A, 0x2D, 0x9D, 0x24, 0xBD, 0x54, 0x9D, 0x09, 0xA0, 0x3A, 0x7B, 0xFC, 0x5C, 0x86, 0x53,
//0x7C, 0x04, 0x52, 0xEB, 0x06, 0xB7, 0x8A, 0x8A, 0x91, 0x1A, 0x95, 0x0D, 0x5B, 0x05, 0x38, 0x37, 0xE7, 0x7D, 0xB7, 0x49,
//0x46, 0xE3, 0x3D, 0xD2, 0x97, 0x6A, 0x7D, 0xC9, 0xCA, 0xCF, 0x9A, 0xD9, 0xA9, 0x44, 0x44, 0xFC, 0x18, 0xAD, 0x25, 0x4B,
//0x9B, 0x0E, 0x7B, 0x15, 0x1A, 0x44, 0x06, 0x2E, 0xE1, 0xF4, 0x24, 0x89, 0xEB, 0xC2, 0x3B, 0x09, 0x60, 0xC3, 0x57, 0x95,
//0x5C, 0x34, 0x14, 0x6B, 0xF2, 0x6C, 0xBF, 0xB6, 0x71, 0x0F, 0xB8, 0x62, 0xE5, 0x85, 0x1A, 0x2F, 0xE1, 0xAA, 0x9E, 0x04,
//0x7C, 0xC7, 0xBD, 0x87, 0x9A, 0xC2, 0x15, 0xEA, 0x6D, 0x91, 0x0E, 0x07, 0x3B, 0xF4, 0xBD, 0xB0, 0x61, 0x64, 0x09, 0x40,
//0x98, 0x46, 0x0C, 0x76, 0x77, 0x65, 0xBC, 0x84, 0x39, 0x9F, 0xCA, 0x62, 0xC1, 0xDF, 0xCA, 0x68, 0xDB, 0x4B, 0x95, 0x1C,
//0xC5, 0xBE, 0xEB, 0x49, 0xEE, 0x78, 0x5D, 0x51, 0xFD, 0x14, 0x6A, 0xC3, 0x15, 0xBB, 0xE9, 0x30, 0x57, 0x98, 0xA7, 0x3B,
//0xA9, 0x8B, 0x50, 0xF0, 0xB6, 0x22, 0x93, 0x57, 0x36, 0xC5, 0x81, 0x48, 0x70, 0x30, 0x36, 0x80, 0xFD, 0x27, 0xBC, 0xF5,
//0xCE, 0x64, 0x2C, 0x67, 0x71, 0xD5, 0x16, 0x38, 0x8F, 0x04, 0x23, 0x7D, 0xBA, 0xD7, 0xF9, 0x55, 0x65, 0x79, 0x5F, 0x7A,
//0x34, 0x98, 0xA2, 0x15, 0x77, 0xD1, 0x37, 0xD7, 0xF1, 0x68, 0xA1, 0x45, 0xDD, 0x3D, 0x96, 0x53, 0x33, 0xFB, 0x5D, 0x35,
//0x27, 0x93, 0x7A, 0x72, 0xF2, 0xE9, 0x8D, 0x3A, 0xD6, 0xCC, 0x78, 0xF9, 0x5C, 0x2C, 0xC0, 0x2C, 0xE1, 0x87, 0x36, 0x74,
//0x50, 0xA5, 0xD6, 0xE8, 0x31, 0x85, 0xEF, 0xB0, 0xE3, 0x97, 0x84, 0xD0 };
//
//BYTE pack1[] = { 0x65, 0xE8, 0xCB, 0x0D, 0xF3, 0xED, 0xEF, 0x4A, 0x86, 0x97, 0x05, 0x0C, 0x74, 0x77, 0xAA, 0x8E, 0xD8, 0xC8, 0x7B, 0x09,
//0xDA, 0xA5, 0x7B, 0x86, 0x31, 0x7A, 0xB5, 0xF1, 0xE6, 0xB9, 0x22, 0x70, 0x5A, 0xCE, 0xCC, 0xF3, 0x8A, 0x54, 0x34, 0x0B,
//0x92, 0x89, 0xF1, 0xFF, 0x70, 0xFF, 0x9B, 0x1D, 0x0B, 0x95, 0xE7, 0x4E, 0x74, 0xA6, 0x13, 0xED, 0x6B, 0x80, 0x85, 0xD9,
//0x25, 0x18, 0xAF, 0xC9, 0x4C, 0xFC, 0x35, 0xE0, 0x48, 0x88, 0x52, 0x82, 0xBD, 0x5D, 0x78, 0x65, 0x54, 0x0F, 0x36, 0xEB,
//0xBF, 0x1E, 0x5F, 0xAF, 0xF7, 0x28, 0x69, 0x5D, 0xC8, 0x5C, 0x13, 0xC8, 0x90, 0x32, 0x4A, 0x36, 0x44, 0x59, 0x4E, 0xFE,
//0xB3, 0xF1, 0x11, 0x56, 0x0F, 0xFB, 0xE0, 0x66, 0xA9, 0x0E, 0x44, 0xA1, 0xFC, 0x4B, 0x2B, 0x54, 0xED, 0x93, 0x43, 0x7F,
//0x51, 0xF7, 0xA7, 0xE5, 0xB0, 0x6F, 0xBD, 0x5F };
//
//BYTE data2[] = { 0x9E, 0x0A, 0xAB, 0x88, 0x61, 0x75, 0x8C, 0xC2, 0x63, 0xA6, 0x98, 0x01, 0xB9, 0xAB, 0xB5, 0x68, 0x8B, 0x5C, 0x6B, 0x2D,
//0xC8, 0x3E, 0x3A, 0x9E, 0x18, 0xA7, 0x31, 0x62, 0x13, 0x23, 0xBC, 0x02, 0xB0, 0x17, 0xBE, 0xA8, 0x00, 0xA4, 0xDF, 0x26,
//0x1C, 0xCE, 0x28, 0x39, 0x5C, 0xDE, 0xCD, 0x0F, 0xC9, 0x04, 0x49, 0x08, 0xD5, 0x41, 0xB0, 0xBA, 0x4A, 0x84, 0x41, 0x37,
//0x81, 0x14, 0x3B, 0x92, 0xD4, 0x2D, 0xF3, 0x9F, 0x55, 0xEE, 0x22, 0xFB, 0xC1, 0x73, 0x83, 0x4C, 0xAF, 0x3E, 0x9E, 0xCA,
//0x0D, 0xDB, 0xDE, 0xFF, 0xCD, 0xC3, 0xEA, 0xCA, 0xDC, 0x82, 0x32, 0xAA, 0x54, 0x15, 0xC8, 0x39, 0xF2, 0x96, 0x72, 0xF7,
//0xCA, 0x1B, 0xFD, 0x49, 0x39, 0xDB, 0xD4, 0xC2, 0x1C, 0x92, 0xD1, 0xA0, 0x3F, 0x27, 0x52, 0x9A, 0x19, 0x8C, 0x90, 0xD9,
//0xB6, 0xE7, 0x2C, 0x44, 0xF4, 0xEF, 0x0B, 0x00, 0xF2, 0x98, 0x64, 0x37, 0xC6, 0xF2, 0x17, 0x19, 0x6E, 0x4E, 0x41, 0x21,
//0x09, 0xA4, 0x0D, 0x9A, 0x48, 0x31, 0xCD, 0x61, 0x1F, 0x64, 0x49, 0xEC, 0xC0, 0xFB, 0x56, 0x8B, 0xC9, 0x6A, 0xAB, 0xEE,
//0xBC, 0x7C, 0xF1, 0xE8, 0xB1, 0x3C, 0x19, 0x72, 0x47, 0x16, 0xF2, 0xF3, 0xE0, 0x3A, 0xFB, 0xBF, 0x0E, 0x41, 0x76, 0xDD,
//0x48, 0x13, 0xBD, 0x0D, 0xE2, 0x08, 0xD9, 0x09, 0x3C, 0x4D, 0x51, 0x0C, 0x39, 0x8A, 0x40, 0x91, 0xF2, 0xC1, 0xAA, 0x5D,
//0xAC, 0x7A, 0x10, 0xBF, 0x08, 0x1D, 0x42, 0x17, 0x7A, 0x9F, 0x75, 0x95, 0xFC, 0x60, 0x95, 0x43, 0x04, 0x54, 0xDD, 0x46,
//0xB3, 0x62, 0xA4, 0x61, 0x4F, 0x09, 0x7A, 0xBA, 0xF3, 0x49, 0x73, 0xE1, 0x85, 0xC1, 0xC3, 0x0E, 0xE9, 0x15, 0x8B, 0xAA,
//0x74, 0x92, 0xF3, 0x11, 0xA4, 0x15, 0x79, 0x97, 0x0E, 0x28, 0x89, 0xC9, 0x97, 0x6F, 0x4D, 0xF5, 0xE7, 0x43, 0xC6, 0xEC,
//0x1B, 0x9C, 0x1C, 0x06, 0xBA, 0x34, 0x32, 0x67, 0xEB, 0xE9, 0x7E, 0x81, 0xE5, 0x63, 0x18, 0xDA, 0xA6, 0xF8, 0x10, 0x0F,
//0xE8, 0xCC, 0x05, 0x1A, 0x51, 0x3D, 0xF3, 0xD0, 0xEA, 0x7E, 0x4D, 0xAD, 0xD2, 0x81, 0x4C, 0xD4, 0x34, 0x1B, 0x86, 0x52,
//0x76, 0x63, 0xC2, 0xA3, 0xB3, 0x5B, 0x0B, 0x77, 0x6C, 0xD2, 0x03, 0xB6, 0x7A, 0x88, 0x7A, 0xC1, 0x47, 0x94, 0x3F, 0xFB,
//0x3E, 0x03, 0x36, 0x88, 0x80, 0xC2, 0xAB, 0x54, 0xD9, 0xBE, 0xE6, 0x57, 0xF2, 0x49, 0x00, 0x18, 0xC8, 0xC0, 0xF5, 0x70,
//0x5C, 0x6A, 0xF9, 0x91, 0xAD, 0xA4, 0x10, 0x46, 0xE3, 0x3B, 0xD9, 0x7D, 0xCD, 0x1A, 0xAA, 0x02, 0x8D, 0x25, 0xE2, 0x8D,
//0x92, 0x64, 0x7B, 0xFF, 0xC7, 0x64, 0xDF, 0xE9, 0x0A, 0xDA, 0xFD, 0x4E, 0xE0, 0x8A, 0x9B, 0xAA, 0x67, 0x67, 0xEE, 0x32,
//0x8C, 0xFC, 0x68, 0x5E, 0x7D, 0x0A, 0xD1, 0xA5, 0x62, 0x73, 0x99, 0x87, 0x48, 0x56, 0x6D, 0xD1, 0x5D, 0x7C, 0x08, 0x1D,
//0x63, 0xF3, 0x4C, 0x98, 0x5F, 0xF8, 0x91, 0x08, 0x91, 0x1B, 0x3D, 0x0D, 0xA9, 0x9D, 0x94, 0xA6, 0x38, 0x1D, 0xD2, 0xF8,
//0x65, 0x73, 0x77, 0x4B, 0xE1, 0xDD, 0xE3, 0x56, 0x1D, 0x59, 0x22, 0xBC, 0xBA, 0x51, 0x09, 0xD6, 0x41, 0x7D, 0x82, 0xB3,
//0x14, 0x06, 0x24, 0x5D, 0xA4, 0xE7, 0x07, 0x5E, 0xE5, 0x99, 0xD6, 0x83, 0xFF, 0xEF, 0x25, 0xE0, 0x47, 0x3F, 0x05, 0x14,
//0x6D, 0xC2, 0xD6, 0xCF, 0xF6, 0x3D, 0x5D, 0x91, 0xED, 0xE1, 0x17, 0xB4, 0xD6, 0x70, 0xCC, 0xAC, 0xE4, 0xA2, 0x66, 0x8E,
//0x6C, 0xC7, 0x04, 0xCF, 0x98, 0xAD, 0x4B, 0x4E, 0x60, 0xC4, 0xFC, 0x95, 0xB6, 0x80, 0xCC, 0x5C, 0x01, 0xFF, 0x70, 0x4F,
//0xCD, 0xEC, 0xF7, 0x12, 0xC1, 0xA3, 0x5C, 0x81, 0xD1, 0x96, 0x5C, 0x3C };
//
//BYTE pack2[] = { 0x12, 0x29, 0x3C, 0xE9, 0x63, 0xE2, 0x4F, 0x40, 0xF1, 0xBF, 0x66, 0x2A, 0xE5, 0x12, 0x23, 0x15, 0x02, 0x61, 0x7F, 0x61,
//0x38, 0xAF, 0x05, 0xC4, 0x76, 0xFD, 0x3E, 0xED, 0xC1, 0x11, 0x6C, 0x67, 0x83, 0x76, 0x7A, 0xC1, 0x25, 0xC4, 0x34, 0xEC,
//0xDF, 0x22, 0xB4, 0xE4, 0xCF, 0x08, 0x1C, 0x92, 0x49, 0x4A, 0x8B, 0xF2, 0x6F, 0x52, 0x49, 0x71, 0x01, 0xAC, 0x45, 0x99,
//0x76, 0xC9, 0xC2, 0xEF, 0x4B, 0xFC, 0x4D, 0x1C, 0x9B, 0xC9, 0xF2, 0x4C, 0xF0, 0x5F, 0xAF, 0xD4, 0x59, 0x63, 0x29, 0x17,
//0xBC, 0xEF, 0x9E, 0xAA, 0x6E, 0xDB, 0x1B, 0xB6, 0x01, 0x3B, 0xEF, 0x9E, 0x5D, 0x28, 0x5F, 0xE2, 0x12, 0xC4, 0x9B, 0x44,
//0xED, 0x78, 0xF1, 0x29, 0xCA, 0x80, 0x41, 0x41, 0xA8, 0xAA, 0xB1, 0x6A, 0x03, 0x5A, 0xA1, 0xD2, 0x5E, 0xD2, 0xE2, 0x5C,
//0x1F, 0xB0, 0x22, 0x46, 0x9F, 0xB8, 0xAB, 0x08 };
//
//BYTE data3[] = { 0xFA, 0x71, 0x53, 0x09, 0x46, 0x22, 0xC7, 0x58, 0x97, 0xB5, 0xB6, 0xD5, 0x85, 0xF9, 0x03, 0xE3, 0xBC, 0x76, 0x66, 0x6D,
//0x45, 0xA3, 0xC9, 0xC2, 0x07, 0x5A, 0xF2, 0xA4, 0x8D, 0x88, 0x63, 0x9B, 0x09, 0x29, 0x7E, 0x90, 0x99, 0x74, 0xED, 0x96,
//0xCA, 0x52, 0x41, 0x0B, 0x5B, 0x15, 0xD9, 0xE0, 0x43, 0x67, 0x06, 0x35, 0xF4, 0x80, 0x6C, 0x29, 0xE2, 0x9D, 0xAC, 0xF0,
//0x3F, 0xAD, 0x40, 0x26, 0x09, 0x4D, 0xA6, 0x62, 0x47, 0xF6, 0xC0, 0x0F, 0x90, 0x75, 0x7B, 0x77, 0xCC, 0x9C, 0xAF, 0x21,
//0x3F, 0x38, 0x46, 0x5D, 0xC3, 0x43, 0xA1, 0x1D, 0x54, 0x5A, 0x37, 0x06, 0xD2, 0x71, 0x7F, 0xC7, 0x03, 0xC5, 0x11, 0xAA,
//0xD8, 0x12, 0x08, 0x68, 0xF1, 0x06, 0xAA, 0x42, 0x62, 0xF6, 0x4B, 0x4D, 0xB8, 0xD1, 0x6E, 0x0E, 0x37, 0x52, 0x2E, 0x46,
//0x66, 0x79, 0xDC, 0x0F, 0xCE, 0x3B, 0xEA, 0x28, 0xFE, 0x79, 0x07, 0x8E, 0xD2, 0x2F, 0x8C, 0x4C, 0xF5, 0xEE, 0x17, 0x93,
//0xA5, 0x8C, 0x75, 0x0D, 0xB6, 0x1A, 0xC6, 0x6F, 0xDA, 0x16, 0xDB, 0x4E, 0x1F, 0xE2, 0xE3, 0x33, 0xB9, 0x75, 0x47, 0xF1,
//0x02, 0x51, 0xD2, 0x36, 0xBF, 0xB4, 0x15, 0x61, 0xA5, 0x13, 0x0A, 0x93, 0x1C, 0xC4, 0xF7, 0x09, 0x42, 0xFB, 0x95, 0xA9,
//0x33, 0xF7, 0x72, 0xDA, 0x88, 0x7D, 0x94, 0x99, 0x1D, 0x86, 0x5F, 0xCA, 0x18, 0x35, 0xB9, 0xCB, 0x26, 0x0B, 0x6B, 0x4F,
//0x08, 0x5E, 0xCA, 0x6A, 0x4F, 0x07, 0x98, 0xE8, 0x63, 0x5C, 0x25, 0xE3, 0xCA, 0x5E, 0xB9, 0x95, 0xA7, 0x33, 0x36, 0x6A,
//0x81, 0xD6, 0xFA, 0x59, 0x46, 0x0D, 0x03, 0x75, 0x90, 0x9B, 0x56, 0xBF, 0x26, 0xB7, 0xA1, 0x3F, 0xC6, 0xBE, 0x1F, 0x53,
//0x22, 0x25, 0xBC, 0x46, 0x28, 0xB1, 0x7A, 0x24, 0x86, 0xEA, 0x13, 0xCD, 0x6E, 0xCD, 0xE0, 0x49, 0x96, 0xC1, 0xF9, 0x5B,
//0x84, 0xCC, 0x9E, 0x59, 0x08, 0x07, 0xD5, 0x3A, 0x0A, 0x4F, 0x55, 0xF1, 0x85, 0x3E, 0xA3, 0xFA, 0xD4, 0x59, 0x7B, 0x32,
//0x2D, 0x8B, 0x71, 0xEB, 0x6B, 0xD3, 0xD8, 0x40, 0x10, 0x39, 0xE3, 0xA6, 0x8A, 0x06, 0x0A, 0xC4, 0xB5, 0x34, 0xAF, 0x85,
//0xA2, 0x24, 0x02, 0x6B, 0x97, 0x8F, 0x22, 0x26, 0x56, 0xBF, 0x85, 0x65, 0xA2, 0xFF, 0x99, 0xEC, 0x00, 0x50, 0xAB, 0xE6,
//0xBB, 0x5D, 0x09, 0x7D, 0x2A, 0xB1, 0x63, 0xA9, 0x35, 0x28, 0x77, 0x45, 0x16, 0xA3, 0xEE, 0x68, 0x5D, 0x00, 0x80, 0xF2,
//0x37, 0x4A, 0x5F, 0x65, 0xEF, 0x2C, 0xD3, 0xF7, 0xB5, 0xAC, 0x21, 0xE5, 0x9F, 0x16, 0xB2, 0x09, 0xED, 0x35, 0x71, 0xAF,
//0x91, 0xC9, 0x75, 0xA5, 0xEF, 0x36, 0xE0, 0x91, 0xE1, 0x7A, 0x07, 0x87, 0xFE, 0x61, 0xD3, 0xDB, 0x1C, 0xFC, 0x67, 0x34,
//0x13, 0x4F, 0x02, 0xE2, 0xC4, 0x4A, 0x27, 0x72, 0x5A, 0xFB, 0xFC, 0x7F, 0xD0, 0x01, 0xF1, 0xCB, 0xB4, 0x44, 0xA1, 0x8E,
//0x2A, 0xE3, 0xFD, 0x0D, 0x33, 0x6E, 0xA7, 0x6F, 0x2F, 0x56, 0x9C, 0xCC, 0xA1, 0xA1, 0x16, 0x88, 0x33, 0xE2, 0xB1, 0xDF,
//0xFA, 0x69, 0xCC, 0xC0, 0xFA, 0xB7, 0x32, 0xDF, 0xED, 0x35, 0xFA, 0x06, 0x40, 0x25, 0xAB, 0xAB, 0x5F, 0xD8, 0xA4, 0xC8,
//0x32, 0x21, 0xBB, 0xE7, 0xE7, 0x0B, 0x25, 0x7C, 0xF3, 0xC7, 0x96, 0x78, 0x53, 0xE8, 0xA8, 0x1B, 0x18, 0xD3, 0x81, 0x0A,
//0x22, 0x70, 0xB0, 0xA1, 0x2C, 0x26, 0x59, 0x8A, 0x09, 0x07, 0x92, 0x8F, 0x2C, 0x47, 0x0C, 0xAC, 0x6E, 0xF3, 0xF6, 0x69,
//0x0C, 0xDF, 0x1E, 0x67, 0xEF, 0xE1, 0x5A, 0x38, 0x31, 0x3C, 0x26, 0x73, 0xDC, 0x5F, 0x8E, 0x08, 0xF5, 0xD0, 0x4F, 0xD2,
//0xB9, 0x63, 0x39, 0x6B, 0x26, 0xAF, 0xD7, 0x44, 0xBD, 0xC2, 0x52, 0xF5 };
//
//BYTE pack3[] = { 0xBC, 0x6F, 0xE5, 0xC5, 0x89, 0x80, 0x86, 0xA6, 0xDB, 0xEC, 0x5B, 0x1C, 0xDA, 0x0B, 0xEB, 0xE1, 0x6A, 0x01, 0x5D, 0xD4,
//0xFE, 0x40, 0xE8, 0x9B, 0xB1, 0x4E, 0x8A, 0x8C, 0x93, 0xC9, 0xFD, 0xCE, 0x40, 0xA8, 0xE7, 0xBB, 0xB8, 0xC3, 0x51, 0x89,
//0x6A, 0x96, 0x5A, 0x2A, 0x92, 0x41, 0x2C, 0x70, 0xC5, 0xD4, 0x30, 0x75, 0xCA, 0xCA, 0x7E, 0x3A, 0xAC, 0xD3, 0x98, 0x8E,
//0x0D, 0xD7, 0x57, 0x32, 0xBD, 0x37, 0x25, 0x3A, 0x8C, 0x56, 0x6D, 0xB6, 0x5C, 0x58, 0x50, 0x92, 0x7A, 0x29, 0x9A, 0xBB,
//0x2E, 0xC1, 0xF4, 0x83, 0x90, 0xA7, 0x5B, 0x47, 0x9E, 0xB4, 0xD0, 0x4F, 0x8C, 0x1A, 0x51, 0xF5, 0x5B, 0xA5, 0x09, 0x0A,
//0xAD, 0x40, 0xC4, 0x75, 0xD8, 0xBA, 0xA5, 0x1D, 0x39, 0xCA, 0x1B, 0x0A, 0xCB, 0x51, 0x11, 0xC7, 0xF7, 0xE0, 0x65, 0x0A,
//0xAE, 0x7A, 0x9C, 0x96, 0xB6, 0x9B, 0xD4, 0x61 };
//
//BYTE data4[] = { 0xC0, 0x7E, 0x03, 0xAA, 0xC3, 0x3A, 0xBE, 0xED, 0xDE, 0x80, 0x67, 0xC0, 0x3D, 0x1D, 0xAA, 0xED, 0x11, 0xC8, 0x6C, 0x96,
//0x88, 0x4E, 0x5E, 0x77, 0xFC, 0xEC, 0xD2, 0x6C, 0x6B, 0xFC, 0xCE, 0x33, 0x27, 0xD5, 0x5F, 0x72, 0xCE, 0xEC, 0x30, 0xFC,
//0x7D, 0x3F, 0x5B, 0x50, 0x41, 0xF3, 0x7F, 0x63, 0xCE, 0x3B, 0x13, 0x9F, 0x74, 0x7D, 0xFA, 0x2B, 0x4E, 0xB0, 0x57, 0x6D,
//0xBD, 0x09, 0xA2, 0xCD, 0x14, 0xD6, 0xFC, 0xBC, 0x95, 0xAA, 0xBF, 0xF5, 0x9D, 0xBA, 0x5A, 0x03, 0x1B, 0x87, 0x5C, 0x04,
//0x46, 0xBD, 0xCE, 0xAC, 0x90, 0x5B, 0xC6, 0x8D, 0xD5, 0x15, 0x39, 0x95, 0xD9, 0xF7, 0x11, 0xD8, 0xF1, 0x4A, 0x7F, 0x8F,
//0x00, 0xBB, 0x93, 0x61, 0xA5, 0xBA, 0x0A, 0xE2, 0xB3, 0x21, 0x67, 0x5A, 0xE2, 0x16, 0x45, 0xC7, 0xC4, 0x32, 0xEB, 0x25,
//0xFA, 0xE3, 0x21, 0xEC, 0xA7, 0x0E, 0x42, 0xDB, 0x25, 0xF9, 0x93, 0xF4, 0xF7, 0x69, 0xD3, 0xC8, 0xFF, 0x07, 0x13, 0xF4,
//0xF1, 0x08, 0xC8, 0xEC, 0x08, 0x0D, 0x1F, 0xF7, 0xF8, 0x48, 0x90, 0x7B, 0x23, 0xE1, 0xB6, 0x7B, 0x0F, 0x98, 0x5E, 0x5E,
//0x18, 0xAB, 0xDE, 0xF2, 0x62, 0xFA, 0xA8, 0xB0, 0x12, 0x69, 0x1E, 0x42, 0xBE, 0x86, 0xA8, 0x42, 0x21, 0x6C, 0x04, 0x45,
//0x14, 0xE7, 0xDE, 0x18, 0xBA, 0xD8, 0xA1, 0x49, 0xFA, 0xDB, 0x8C, 0xE9, 0x32, 0x28, 0x08, 0x91, 0x29, 0xB7, 0x39, 0xA3,
//0x46, 0xA8, 0xD3, 0xD2, 0x00, 0xE1, 0x2F, 0xE5, 0x96, 0xF9, 0x9E, 0xB7, 0x00, 0x55, 0xFD, 0x83, 0x25, 0x90, 0x89, 0x5E,
//0x4E, 0x20, 0xF5, 0x05, 0xDB, 0x37, 0xBA, 0xD9, 0x35, 0xE8, 0xAF, 0x28, 0x03, 0x8B, 0xD9, 0xAE, 0xA1, 0x62, 0x84, 0x5C,
//0xCD, 0x7D, 0x92, 0x58, 0xA4, 0xDA, 0x14, 0x44, 0xCE, 0xD1, 0x17, 0xC3, 0xF5, 0xAF, 0xC0, 0x39, 0x7B, 0xA2, 0x9C, 0xD3,
//0x6C, 0xD5, 0x31, 0xC8, 0xB1, 0xDB, 0xD9, 0xDE, 0x87, 0x51, 0xD0, 0x2F, 0x30, 0xBF, 0x8B, 0x2D, 0xE8, 0xC0, 0x4B, 0xE4,
//0x1B, 0x63, 0xF3, 0x7F, 0xD5, 0xD0, 0x16, 0x0E, 0x7A, 0x2F, 0x55, 0x86, 0xB7, 0xC5, 0xE8, 0x09, 0xB8, 0x60, 0x7A, 0x68,
//0x9C, 0xF6, 0x3A, 0xE7, 0x26, 0x89, 0x2F, 0x40, 0xB5, 0x4D, 0xCB, 0xEA, 0x76, 0x0D, 0xC4, 0x9A, 0xD8, 0xCB, 0x1F, 0x0B,
//0x40, 0xA7, 0x8E, 0xFB, 0xFF, 0x02, 0xFB, 0x76, 0x81, 0xE2, 0x65, 0x56, 0xC6, 0x98, 0xEB, 0x0B, 0x16, 0xA1, 0x26, 0x98,
//0xF1, 0xCB, 0xBC, 0xDE, 0x46, 0x98, 0x32, 0x1D, 0xE5, 0xE7, 0x02, 0xB8, 0x35, 0xD1, 0xEE, 0x38, 0x2A, 0xD2, 0x93, 0x94,
//0x72, 0x29, 0x41, 0xB0, 0xEF, 0x81, 0x0A, 0x23, 0x6A, 0xD0, 0x19, 0x3D, 0x86, 0x80, 0x46, 0x49, 0xF2, 0xCD, 0xE7, 0x0F,
//0xE3, 0x6E, 0xE8, 0x9D, 0xC2, 0x77, 0x1E, 0x47, 0x21, 0x7E, 0xD8, 0xED, 0xF4, 0xFC, 0xB7, 0x7C, 0xFE, 0xF7, 0xC0, 0xB7,
//0x89, 0xDE, 0xB0, 0x38, 0x5A, 0xB5, 0x8F, 0xB1, 0xE4, 0x71, 0x88, 0x78, 0xBC, 0xA6, 0xF8, 0x43, 0x4D, 0x5F, 0xC5, 0x30,
//0xCD, 0x4D, 0xF1, 0x08, 0x70, 0x25, 0x6F, 0xC4, 0xDB, 0x43, 0x35, 0x50, 0xDB, 0x93, 0x92, 0x90, 0x57, 0xB1, 0xC2, 0xA1,
//0x83, 0x4F, 0xC0, 0x61, 0x57, 0xD8, 0x5B, 0x31, 0x40, 0x52, 0x8C, 0xF9, 0x15, 0x89, 0x07, 0x6D, 0x4C, 0x6A, 0x10, 0x8F,
//0x6E, 0xAD, 0x90, 0x75, 0xC8, 0xB7, 0x68, 0x4D, 0x64, 0xBE, 0x05, 0x9E, 0x37, 0x2E, 0x34, 0xD1, 0x9C, 0x4B, 0x38, 0xEF,
//0x01, 0x18, 0x1B, 0xAC, 0xE0, 0x78, 0x41, 0xA5, 0xF2, 0x94, 0x43, 0xE6, 0x9F, 0x7C, 0xFA, 0xAF, 0xB8, 0x11, 0xD9, 0x78,
//0x68, 0x22, 0x80, 0x32, 0x68, 0xD5, 0x8F, 0xD0, 0x73, 0x49, 0xB8, 0x06 };
//
//BYTE pack4[] = { 0xD1, 0x3D, 0x5C, 0x8B, 0x4C, 0x4C, 0xF4, 0xB4, 0x39, 0x92, 0x3F, 0xBC, 0xA6, 0xF1, 0x69, 0xAB, 0xD3, 0xC1, 0x38, 0xC1,
//0x4C, 0x68, 0x4C, 0xAC, 0x56, 0x2A, 0xA5, 0xFE, 0x8C, 0x43, 0x65, 0xDD, 0xCA, 0x33, 0xD7, 0xFC, 0x4A, 0x5A, 0xB1, 0xCB,
//0x0D, 0xC4, 0x96, 0xEB, 0x7D, 0x46, 0xDF, 0xE5, 0x50, 0xCA, 0x9B, 0x1B, 0xAF, 0xE7, 0xD4, 0xFF, 0x98, 0xB5, 0x15, 0x82,
//0x81, 0xDF, 0x28, 0xD0, 0x3E, 0xF9, 0x24, 0x3A, 0x24, 0x14, 0xA8, 0x73, 0x6F, 0x90, 0x61, 0x00, 0xE1, 0x32, 0xD2, 0xEA,
//0xAC, 0x76, 0x54, 0xB6, 0x92, 0x27, 0x5B, 0x96, 0x4B, 0x2C, 0xFC, 0xE1, 0x27, 0xA5, 0xAC, 0xC1, 0xF4, 0x37, 0x4B, 0x2C,
//0x22, 0xA3, 0x61, 0x15, 0x25, 0x9E, 0xA7, 0xE3, 0x1B, 0xDD, 0xC9, 0x22, 0xAF, 0x6F, 0x6A, 0x78, 0x3D, 0x4F, 0x68, 0xDB,
//0x37, 0x3A, 0x62, 0x02, 0xC7, 0xBE, 0x78, 0x19 };
//
//int buffer1[] = { 0x075A, 0x04A0, 0x01C4, 0x0428, 0x0648, 0x0026, 0x05ED, 0x05CB, 0x005C, 0x0106, 0x0118, 0x0175, 0x0360, 0x0698, 0x041D, 0x073B,
//0x064A, 0x04EA, 0x06FD, 0x035D, 0x0163, 0x05AA, 0x05DA, 0x05C1, 0x0601, 0x000A, 0x00E8, 0x03E7, 0x02DB, 0x047A, 0x0220, 0x06B6 };
//
//int buffer2[] = { 0x0064, 0x0576, 0x061C, 0x01E9, 0x05D9, 0x0293, 0x06B4, 0x0792, 0x00B8, 0x07B8, 0x0246, 0x0756, 0x003D, 0x01F4, 0x01D8, 0x051A };

/*--------------- FUNCTIONS: ---------------*/

//PRIVATE BOOL WriteRead(IN BYTE *pTmp, IN OUT DWORD *pdwLen, OUT BYTE *pResult, BOOL bWrite);

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE void ConvertHEXToASCII(IN BYTE *tbHex, IN DWORD dwLen, OUT UCHAR *szAscii)
{
	DWORD	dwH, dwA;
	UCHAR	chCh;

	for (dwH = 0, dwA = 0; dwH < dwLen; dwH++, dwA += 2)
	{
		szAscii[dwA] = (chCh = (tbHex[dwH] & 0xF0) >> 4) < 10 ? (chCh | 0x30) : (chCh + 55);
		szAscii[dwA + 1] = (chCh = tbHex[dwH] & 0x0F) < 10 ? (chCh | 0x30) : (chCh + 55);
	}
	szAscii[dwA] = '\0';
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE void ConvertASCIIToHEX(IN UCHAR *szAscii, OUT BYTE *tbHex, OUT DWORD *dwLen)
{
	DWORD	dwH, dwA;
	UCHAR	chCh;

	for (dwH = 0, dwA = 0; dwH < strlen(szAscii) / 2; dwH++, dwA += 2)
	{
		tbHex[dwH] = ((((chCh = szAscii[dwA]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)) << 4) |
			((((chCh = szAscii[dwA + 1]) >= 'A') ? (chCh - 55) : (chCh & 0x0F)));
	}
	*dwLen = dwH;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Connexion(ACOM_EVT_HANDLE hEvent)
{
	DWORD64			dwInstUsrKey;
	ACOM_CNX_HANDLE	hCnxHandle;
	aut_route_id	*pRoute;
	DWORD			Cnx;

	NTSVCInfo("ROUTE_Connexion() => open connection");

	// on récupère le handle de connexion
	AComGetConnectionEvtParameters(hEvent, &dwInstUsrKey, &hCnxHandle);

	// calcul d'un index dans le tableau des ROUTESVC connectés 
	// a partir du handle de connexion
	if (hCnxHandle == 0)
	{
		NTSVCError("ROUTE_Connexion() => handle to connection is NULL");

		return FALSE;
	}

	Cnx = ACOM_GET_SLOT_NUMBER(hCnxHandle);
	pRoute = &ROUTE.aut[Cnx];

	// initialisation de l'automate du PV connecté
	if (ROUTE_AUT_New(pRoute, hCnxHandle) == FALSE)
		return FALSE;

	// Envoi de l'event de connexion
	ROUTE_AUT_Send(*pRoute, EVT_ROUTE_CONNECTION, &hCnxHandle);

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Disconnexion(ACOM_EVT_HANDLE hEvent)
{
	DWORD64			dwInstUsrKey;
	DWORD64			hCnxHandle;
	aut_route_id	*pRoute;
	DWORD			Cnx;

	NTSVCInfo("ROUTE_Disconnexion() => close connection");

	// on récupère le handle de connexion
	AComGetDisconnectionEvtParameters(hEvent, &dwInstUsrKey, &hCnxHandle);

	// calcul d'un index dans le tableau des PV connecté 
	// a partir du handle de connexion
	if (hCnxHandle == 0)
	{
		NTSVCError("ROUTE_Disconnexion() => handle to connection is NULL");

		return FALSE;
	}

	Cnx = ACOM_GET_SLOT_NUMBER(hCnxHandle);
	pRoute = &ROUTE.aut[Cnx];

	// Envoi de l'event de déconnexion
	ROUTE_AUT_Send(*pRoute, EVT_ROUTE_DISCONNECTION, &hCnxHandle);

	// suppression de l'automate
	if (ROUTE_AUT_Delete(pRoute) == FALSE)
		return FALSE;

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
//PRIVATE int RemoveMessageReference(UCHAR *msg, int msgLen, UCHAR *pRef1, UCHAR *pRef2, UCHAR *pResult)
//{
//	EVP_CIPHER_CTX *ctx;
//
//	int len;
//
//	int result_len;
//
//	if (!(ctx = EVP_CIPHER_CTX_new()))
//		NTSVCError("RemoveMessageReference() -> error 1");
//
//	if (1 != EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), NULL, pRef1, pRef2))
//		NTSVCError("RemoveMessageReference() -> error 2");
//
//	if (1 != EVP_DecryptUpdate(ctx, pResult, &len, msg, msgLen))
//		NTSVCError("RemoveMessageReference() -> error 3");
//	result_len = len;
//
//	if (1 != EVP_DecryptFinal_ex(ctx, pResult + len, &len))
//		NTSVCError("RemoveMessageReference() -> error 4");
//	result_len += len;
//
//	EVP_CIPHER_CTX_free(ctx);
//
//	return result_len;
//}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
//PRIVATE BOOL GetReferenceNumber(UCHAR *pNumber, int iLen, int *pBuff)
//{
//	int		i = 0;
//	int		j = 0;
//	int		cnt = 0;
//	BYTE	*ptr = NULL;
//
//	if ((pNumber == NULL) || (pBuff == NULL))
//		return FALSE;
//
//	for (cnt = 0; cnt < iLen; cnt++)
//	{
//		i = pBuff[cnt] / DATA_LEN;
//		j = pBuff[cnt] % DATA_LEN;
//
//		if (i == 0)
//			ptr = data1;
//		else if (i == 1)
//			ptr = data2;
//		else if (i == 2)
//			ptr = data3;
//		else if (i == 3)
//			ptr = data4;
//		else
//		{
//			NTSVCError("GetReferenceNumber() -> error");
//			return FALSE;
//		}
//
//		pNumber[cnt] = ptr[j];
//	}
//
//	return TRUE;
//}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
//PRIVATE BOOL GetMsgHeaderSize(void *pvDataBuffer, DWORD *pdwSize)
//{
//	struct MSG_HEADER *p_header = NULL;
//
//	p_header = MSG_HEADER_New();
//
//	if (p_header != NULL)
//	{
//		if (!MSG_HEADER_Read(p_header, pvDataBuffer, MAX_MSG_SIZE, pdwSize))
//		{
//			MSG_HEADER_Delete_All(p_header);
//			return FALSE;
//		}
//	}
//	else
//	{
//		return FALSE;
//	}
//
//	MSG_HEADER_Delete_All(p_header);
//	return TRUE;
//}

///*|*/
///*****************************************************************************/
///*SYNTAX: 						                                             */
///*===========================================================================*/
///*TYPE:		                                                                 */
///*===========================================================================*/
///*DESCRIPTION:																 */
///*===========================================================================*/
///*PARAMETERS:																 */
///*===========================================================================*/
///*  Return			Description												 */
///*---------------------------------------------------------------------------*/
///*  void            This function does not return a value.   				 */
///*****************************************************************************/
//PRIVATE DWORD GetParam(void)
//{
//	DWORD					dwErr = 0;
//	NTSVC_PARAMETER_DEF		*psParams;
//
//	psParams = NTSVCOpenParameters(
//		_PARAM, REG_DWORD, 4, _PARAM_DEFAULT, &dwParam,
//		NULL);
//
//	if (psParams == NULL)
//		return _PARAM_DEFAULT;
//
//	if (NTSVCLoadParameters(psParams, &dwErr) != ERROR_SUCCESS)
//		return _PARAM_DEFAULT;
//
//	NTSVCCloseParameters(psParams);
//
//	return dwParam;
//}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
//PRIVATE BOOL ReadMessage(char *pvDataBuffer, DWORD *pdwSize, DWORD dwParam)
//{
//	INT		iSize1 = 0;
//	INT		iSize2 = 0;
//	INT		iSize3 = 0;
//	INT		iSize4 = 0;
//	INT		iLen = 0;
//	DWORD	dwHeaderSize = 0;
//	DWORD	dwTmp2Len = 0;
//	DWORD	dwLen1 = 0;
//	DWORD	dwLen2 = 0;
//	DWORD	dwLen3 = 0;
//	BYTE	bTmp1[MAX_PATH] = { 0 };
//	BYTE	bTmp2[MAX_PATH] = { 0 };
//	BYTE	bTmp3[MAX_PATH] = { 0 };
//	BYTE	bTmp1Calc[MAX_PATH] = { 0 };
//	BYTE	bBase[MAX_MSG_SIZE] = { 0 };
//	UCHAR	szRef1[MAX_PATH] = { 0 };
//	UCHAR	szRef2[MAX_PATH] = { 0 };
//	UCHAR	szHex1[MAX_PATH] = { 0 };
//	UCHAR	szHex2[MAX_PATH] = { 0 };
//	BYTE	szHex3[MAX_MSG_SIZE] = { 0 };
//	BYTE	bTmp[MAX_MSG_SIZE] = { 0 };
//
//	NTSVCInfo("ReadMessage()");
//
//	char szTmp[MAX_PATH] = { 0 };
//
//	if (!GetMsgHeaderSize(pvDataBuffer, &dwHeaderSize))
//	{
//		NTSVCInfo("Error getting header size");
//		NTSVCError("Error getting header size");
//		return FALSE;
//	}
//
//	if (dwParam == 1)
//	{
//		iSize1 = *pdwSize - TMP1_LEN * 2;
//
//		if (iSize1 <= 0)
//		{
//			NTSVCInfo("***iSize1 <= 0");
//			NTSVCError("iSize1 <= 0");
//			return FALSE;
//		}
//
//		memcpy(szHex1, &pvDataBuffer[iSize1], TMP1_LEN * 2);
//		ConvertASCIIToHEX(szHex1, bTmp1, &dwLen1);
//
//		memcpy(bBase, pvDataBuffer, iSize1);
//
//		SHA1(bBase, iSize1, bTmp1Calc);
//
//		if (memcmp(bTmp1, bTmp1Calc, TMP1_LEN) == 0)
//		{
//			memset(pvDataBuffer, 0, *pdwSize);
//			memcpy(pvDataBuffer, bBase, iSize1);
//			*pdwSize = (DWORD)iSize1;
//		}
//		else
//		{
//			NTSVCInfo("***Error reading message - 1");
//			NTSVCError("Error reading message - 1");
//			return FALSE;
//		}
//	}
//
//	if (dwParam == 2)
//	{
//		iSize2 = *pdwSize - TMP2_LEN * 2;
//
//		if (iSize2 <= 0)
//		{
//			NTSVCInfo("***iSize2 <= 0...1");
//			NTSVCError("iSize2 <= 0...1");
//			return FALSE;
//		}
//
//		memcpy(szHex2, &pvDataBuffer[iSize2], TMP2_LEN * 2);
//		ConvertASCIIToHEX(szHex2, bTmp2, &dwLen2);
//
//		memcpy(bBase, pvDataBuffer, iSize2);
//		dwTmp2Len = TMP2_LEN;
//
//		if (!WriteRead(bTmp2, &dwTmp2Len, bTmp1, FALSE))
//			return FALSE;
//
//		SHA1(bBase, iSize2, bTmp1Calc);
//
//		if (memcmp(bTmp1, bTmp1Calc, TMP1_LEN) == 0)
//		{
//			memset(pvDataBuffer, 0, *pdwSize);
//			memcpy(pvDataBuffer, bBase, iSize2);
//			*pdwSize = (DWORD)iSize2;
//		}
//		else
//		{
//			NTSVCInfo("***Error reading message - 2");
//			NTSVCError("Error reading message - 2");
//			return FALSE;
//		}
//	}
//
//	if (dwParam == 3)
//	{
//		iSize3 = *pdwSize - dwHeaderSize - sizeof(DWORD) * 2;
//
//		if (iSize3 <= 0)
//		{
//			NTSVCInfo("***iSize3 <= 0");
//			NTSVCError("iSize3 <= 0");
//			return FALSE;
//		}
//
//		if (!GetReferenceNumber(szRef1, REF1_LEN, buffer1))
//		{
//			NTSVCError("error getting 1st instance number");
//			return FALSE;
//		}
//
//		if (!GetReferenceNumber(szRef2, REF2_LEN, buffer2))
//		{
//			NTSVCError("error getting 2nd instance number");
//			return FALSE;
//		}
//
//		memcpy(szHex3, &pvDataBuffer[dwHeaderSize + sizeof(DWORD) * 2], iSize3);
//		ConvertASCIIToHEX(szHex3, bTmp, &dwLen3);
//
//		iSize4 = RemoveMessageReference(bTmp, iSize3 / 2, szRef1, szRef2, bTmp3);
//
//		iSize2 = iSize4 - TMP2_LEN;
//
//		if (iSize2 <= 0)
//		{
//			NTSVCInfo("***iSize2 <= 0...2");
//			NTSVCError("iSize2 <= 0...2");
//			return FALSE;
//		}
//
//		memcpy(bTmp2, &bTmp3[iSize2], TMP2_LEN);
//		memcpy(bBase, pvDataBuffer, dwHeaderSize);
//		memcpy(&bBase[dwHeaderSize], bTmp3, iSize2);
//		iSize2 += dwHeaderSize;
//		dwTmp2Len = TMP2_LEN;
//
//		if (!WriteRead(bTmp2, &dwTmp2Len, bTmp1, FALSE))
//			return FALSE;
//
//		SHA1(bBase, iSize2, bTmp1Calc);
//
//		if (memcmp(bTmp1, bTmp1Calc, TMP1_LEN) == 0)
//		{
//			memset(pvDataBuffer, 0, *pdwSize);
//			memcpy(pvDataBuffer, bBase, iSize2);
//			*pdwSize = (DWORD)iSize2;
//		}
//		else
//		{
//			NTSVCInfo("***Error reading message - 3");
//			NTSVCError("Error reading message - 3");
//			return FALSE;
//		}
//	}
//
//	return TRUE;
//}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Translate_WM_To_Event_HMSG(void *pvDataBuffer,
											  DWORD dwDataSize,
											  aut_event_id *aut_event,
											  HMSG *hMsg)
{
	struct MSG_HEADER* p_header;
	CHAR string[SVC_MSG_BUFFER_SIZE];

	// TO DO : traces...
	strzcpy(string, sizeof(string), pvDataBuffer, dwDataSize);
	string[dwDataSize] = '\0';
	NTSVCInfo("ROUTE_WM Receive (%lu / %s)", dwDataSize, string);

	// Pour sauvegarder les messages recus dans des fichiers au format des voies...
	Pvhook_DataWrite(dwDataSize, string);

	// interprétation du message à l'aide du moteur MSG
	p_header = MSG_New_Read_If_Found(ROUTE.hlRouteMsgList, pvDataBuffer, dwDataSize);
	if (p_header == NULL)
		return FALSE;

	// on rend le header en argument 
	*hMsg = p_header;

	// on convertit ensuite le CD du message en event pour l'automate
	switch (p_header->cd)
	{
	case MSG_SV_CON_REP_CD:
		*aut_event = EVT_ROUTE_LANE_CONNECTION;
		break;

	case MSG_TRANSACTION_CD:
		*aut_event = EVT_ROUTE_MSG_TRANSACTION;
		break;

	case MSG_COMP_INF_TR_CD:
		*aut_event = EVT_ROUTE_MSG_INFORMATION;
		break;

	case MSG_STATUS_CD:
		*aut_event = EVT_ROUTE_MSG_STATUS;
		break;

	case MSG_EVENT_CD:
		*aut_event = EVT_ROUTE_MSG_EVENT;
		break;

	case MSG_BOWL_STAT_CD:
		*aut_event = EVT_ROUTE_MSG_BOWL;
		break;

	case MSG_VAULT_STAT_CD:
		*aut_event = EVT_ROUTE_MSG_VAULT;
		break;

	case MSG_PARTLY_CD:
		*aut_event = EVT_ROUTE_MSG_PARTLY;
		break;

	case MSG_COIN_LISTING_CD:
		*aut_event = EVT_ROUTE_MSG_DISK;
		break;

	case MSG_COUNTER_STATUS_CD:
		*aut_event = EVT_ROUTE_MSG_COUNTERS;
		break;

		// ZDU start 03/09/2013
	case MSG_TRACE_CD:
		*aut_event = EVT_ROUTE_MSG_TRACE;
		break;
		// ZDU end 03/09/2013

	default:
		// libération du Msg 
		MSG_Delete_All(hMsg);
		return FALSE;
	}

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Receive(ACOM_EVT_HANDLE hEvent)
{
	DWORD64			dwInstUsrKey	= 0;
	DWORD64			hCnxHandle		= 0;
	DWORD			dwDataSize		= 0;
	void			*pvDataBuffer	= NULL;
	aut_route_id	*pRoute			= NULL;
	aut_event_id	aut_event;
	HMSG			hMsg			= NULL;
	DWORD			Cnx				= 0;
	BOOL			bRet			= TRUE;

	NTSVCInfo("ROUTE_Receive() => message received");

	// get the connection handle and associated data
	AComGetReceivedEvtParameters(hEvent, &dwInstUsrKey, &hCnxHandle, &dwDataSize, &pvDataBuffer);

	// if handle to connection is NULL
	if (hCnxHandle == 0)
	{
		NTSVCError("ROUTE_Receive() => handle to connection is NULL");
		return FALSE;
	}

	Cnx = ACOM_GET_SLOT_NUMBER(hCnxHandle);
	pRoute = &ROUTE.aut[Cnx];

	// interpretation of the PV's msg to automaton event and associated data
	bRet &= ROUTE_Translate_WM_To_Event_HMSG(pvDataBuffer, dwDataSize, &aut_event, &hMsg);

	// sending the disconnect event msg
	if (bRet)
	{
		bRet &= ROUTE_AUT_Send(*pRoute, aut_event, hMsg);

		// free message
		MSG_Delete_All(&hMsg);
	}

	return bRet;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL ROUTE_Command(struct MSG_PV_COM_REQ *p_req)
{
	struct MSG_PV_COM_REQ_Lanenum * p_lane;
	struct LANE_CNF *p_lane_cnf;
	EVT_ROUTE_COMMAND_ARGS args;
	ACOM_CNX_HANDLE hCnxHandle;
	aut_route_id *pRoute;

	// pour chaque voie concernée par la commande
	p_lane = MSG_PV_COM_REQ_Get_First_Lanenum(p_req->list_lanenum);
	while (p_lane != NULL)
	{
		// rechercher la voie
		p_lane_cnf = PLAZA_Search_Lane(p_lane->lanenum);
		if (p_lane_cnf == NULL)
			return TRUE;

		// recupérer son handle de connexion au RouteSvc
		hCnxHandle = LANE_Get_Connection_Handle(p_lane_cnf);
		if (hCnxHandle != 0)
		{
			// handle de l'automate lié à cette connection
			pRoute = &ROUTE.aut[ACOM_GET_SLOT_NUMBER(hCnxHandle)];

			// envoie de l'event
			if (*pRoute != NULL)
			{
				args.p_req = p_req;
				args.p_lane = p_lane;
				ROUTE_AUT_Send(*pRoute, EVT_ROUTE_COMMAND, &args);
			}
			else
			{
				// On prévient les PV de la déconnection
				PV_PostMessage(PV_WM_SERVER_COM_FAILURE, NULL);
			}
		}

		// voie suivante
		p_lane = MSG_PV_COM_REQ_Get_Next_Lanenum(p_req->list_lanenum, p_lane);
	}

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Ack_Alarm(struct MSG_PV_ACK_REQ *p_req)
{
	struct LANE_CNF *p_lane_cnf;

	// rechercher la voie concernée par cette alarme
	p_lane_cnf = PLAZA_Search_Lane(p_req->body.lanenum);
	if (p_lane_cnf == NULL)
		return FALSE;

	// on teste si la voie est toujours connectée
	if (LANE_Get_Connection_State(p_lane_cnf) == FALSE)
	{
		// On prévient les PV de la déconnection
		PV_PostMessage(PV_WM_SERVER_COM_FAILURE, NULL);
	}

	if (TestAlarmAckType())
		return ALARM_Search_And_Ack_Test(p_lane_cnf, p_req);
	else
		return ALARM_Search_And_Ack(p_lane_cnf, p_req);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL ROUTE_Ack_Alarm_Test(struct MSG_PV_ACK_REQ_REP *p_req)
{
	struct LANE_CNF *p_lane_cnf;

	// rechercher la voie concernée par cette alarme
	p_lane_cnf = PLAZA_Search_Lane(p_req->ack_req.body.lanenum);
	if (p_lane_cnf == NULL)
		return FALSE;

	// on teste si la voie est toujours connectée
	if (LANE_Get_Connection_State(p_lane_cnf) == FALSE)
	{
		// On prévient les PV de la déconnection
		PV_PostMessage(PV_WM_SERVER_COM_FAILURE, NULL);
	}

	return ALARM_Ack_Test(p_lane_cnf, p_req);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PRIVATE BOOL Dispatch_Plaza_State(BOOL partial)
{
	struct MSG_PV_PLAZ_REP *p_msg_pv_plz_rep;
	struct MSG_PV_PLAZ_REP_Alarm *p_alarm;
	HLIST hAlarm = NULL;
	BOOL bRet = TRUE;
	DWORD i;

	// tester si tous les RouteSvc sont connectés
	// s'il en manque un, avertir les PV
	if (partial == FALSE)
	{
		for (i = 0; i < ROUTE_ACOM_Get_Nb_Com_Server(); i++)
		{
			if (ROUTE.aut[i] == NULL)
			{
				// On prévient les PV de la déconnection
				PV_PostMessage(PV_WM_SERVER_COM_FAILURE, NULL);
			}
		}
	}

	// construire le message complet ou partiel d'état de la gare 
	// si aucun changement dans l'état de la gare, pas de message en retour
	p_msg_pv_plz_rep = PLAZA_Build_msg_pv_plz_rep(partial);
	if (p_msg_pv_plz_rep != NULL)
	{
		// parce que le message complet d'état de la gare peut etre tres volumineux
		// (a cause des listes d'alarmes completes par voie), on le morcelle en 
		// envoyant séparemment les messages d'etat des alarmes
		// mais il faut que ces messages partent APRES l'état des voies pour
		// que le PV s'en sorte dans son affichage
		// don cmémorise la liste des alarmes et on la passe a NULL dans l'état de la gare
		hAlarm = p_msg_pv_plz_rep->list_alarm;
		p_msg_pv_plz_rep->list_alarm = NULL;

		// Envoie de l'état général de la gare à tous les PV connectés
		if (PV_PostMessage(PV_WM_PLAZA_STATE, p_msg_pv_plz_rep) == FALSE)
		{
			bRet &= MSG_PV_PLAZ_REP_Delete_All(p_msg_pv_plz_rep);
		}
	}

	// maintenant on envoie un par un les alarmes des voies...
	p_alarm = MSG_PV_PLAZ_REP_Get_First_Alarm(hAlarm);
	while (p_alarm != NULL)
	{
		// envoi du message d'alarme a tous les PV
		bRet &= PV_PostMessage(PV_WM_ALARM_STATE, p_alarm->msg_pv_alar_rep);

		// on passe le pointeur de message à NULL pour ne pas qu'il soit 
		// effacé avec la liste (mais sur réception, apres traitement)
		p_alarm->msg_pv_alar_rep = NULL;

		p_alarm = MSG_PV_PLAZ_REP_Get_Next_Alarm(hAlarm, p_alarm);
	}

	// suppression de la liste des alarmes du message d'etat gare
	MSG_Delete_All_List(&hAlarm);

	return bRet;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
DWORD WINAPI ROUTE_WM_Dispatch(PVOID param)
{
	BOOL	RealTimeUpdate = (BOOL)param;
	MSG		msg;
	INT		ret;

	// Création d'une file de window messages pour ce thread
	PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);

	// boucle de réception des messages
	do
	{
		// attente d'un message
		ret = GetMessage(&msg, NULL, 0, 0);

		// erreur de réception
		if (ret < 0)
			continue;

		// un message a traiter venant du serveur de COM : ROUTE ?
		if (msg.message == ROUTE.WM_ACOM)
		{
			switch (msg.wParam)
			{
			case ACOM_WM_SHUTDOWN:
				NTSVC_ERR("ROUTE_WM_Dispatch() => ACOM_WM_SHUTDOWN");
				ret = FALSE;
				break;

			case ACOM_WM_CONNECTION:
				ret = ROUTE_Connexion(msg.lParam);
				break;

			case ACOM_WM_DISCONNECTION:
				ret = ROUTE_Disconnexion(msg.lParam);
				break;

			case ACOM_WM_RECEIVED:
				ret = ROUTE_Receive(msg.lParam);
				break;

			case ACOM_WM_SENT:
				// tant mieux...
				break;

			case ACOM_WM_TIME_SHIFT:
				// TO DO : mise a l'heure ??
				break;

			default:
				NTSVC_ERR("ROUTE_WM_Dispatch() => WM_ACOM unknown!");
				break;
			}

			// on libère l'event associé au message de ACOM
			AComReleaseEvt(msg.lParam);
		}
		// un message a traiter pour SRV ROUTE ?
		else if (msg.message == ROUTE.WM_ROUTE)
		{
			switch (msg.wParam)
			{
			case ROUTE_WM_TOTAL_PLAZA_STATE:
				// envoie l'état complet de la gare
				ret = Dispatch_Plaza_State(FALSE);
				break;

			case ROUTE_WM_ACK_ALARM_TEST:
				// demande d'acquittement d'une alarme
				ret = ROUTE_Ack_Alarm_Test((struct MSG_PV_ACK_REQ_REP *) msg.lParam);
				break;

			case ROUTE_WM_ACK_ALARM:
				// demande d'acquittement d'une alarme
				ret = ROUTE_Ack_Alarm((struct MSG_PV_ACK_REQ *) msg.lParam);
				break;

			case ROUTE_WM_COMMAND:
				// demande d'envoi d'une commande
				ret = ROUTE_Command((struct MSG_PV_COM_REQ *) msg.lParam);
				break;

			default:
				NTSVC_ERR("ROUTE_WM_Dispatch() => WM_ROUTE unknown!");
				break;
			}

			if (msg.wParam == ROUTE_WM_ACK_ALARM_TEST)
				ACK_REQ_REP_Delete_All((PVOID)msg.lParam);
			else
				MSG_Delete_All((PVOID)&msg.lParam);
		}
		else if (msg.message == WM_QUIT)
		{
			ret = 0;
			break;
		}
		else
		{
			NTSVC_ERR("ROUTE_WM_Dispatch() => WM unknown!");
		}

		// sur réception d'un message on envoie les changements 
		// d'états partiels de la gare aux PV connectés si flag de mise
		// a jour en temps réel est activé
		if (RealTimeUpdate)
			ret = Dispatch_Plaza_State(TRUE);
		else
			ret = TRUE;

		// pour que le serveur soit plus tolérant et robuste
		ret = TRUE;
	}
	// ret == 0 sur réception de WM_QUIT
	while (ret > 0);

	NTSVCInfo("ROUTE_WM_Dispatch() => thread stop!");

	return 0;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL ROUTE_Start(BOOL RealTimeUpdate)
{
	// RAZ de la strucutre interne
	memset(&ROUTE, 0, sizeof(ROUTE));

	// Création de la liste des messages reconnu des VOIES
	MSG_SV_CON_REP_New_Record(&ROUTE.hlRouteMsgList);
	MSG_TRANSACTION_New_Record(&ROUTE.hlRouteMsgList);
	MSG_COMP_INF_TR_New_Record(&ROUTE.hlRouteMsgList);
	MSG_STATUS_New_Record(&ROUTE.hlRouteMsgList);
	MSG_EVENT_New_Record(&ROUTE.hlRouteMsgList);
	MSG_BOWL_STAT_New_Record(&ROUTE.hlRouteMsgList);
	MSG_VAULT_STAT_New_Record(&ROUTE.hlRouteMsgList);
	MSG_PARTLY_New_Record(&ROUTE.hlRouteMsgList);
	MSG_COIN_LISTING_New_Record(&ROUTE.hlRouteMsgList);
	MSG_COUNTER_STATUS_New_Record(&ROUTE.hlRouteMsgList);
	MSG_TRACE_New_Record(&ROUTE.hlRouteMsgList);

	// init des paramètres du registre utilisé par les automates de ROUTE
	if (ROUTE_AUT_Init() == FALSE)
		return FALSE;

	// Configuration de la gare depuis la base
	if (PLAZA_Build_Conf_From_DB() == FALSE)
		return FALSE;

	// recherche des WM du module de COM réseau
	ROUTE.WM_ACOM = RegisterWindowMessage(ACOM_WM_EVENT_NAME);
	if (ROUTE.WM_ACOM == 0)
		return FALSE;

	// déclaration des WM du module lui-meme
	ROUTE.WM_ROUTE = RegisterWindowMessage(ROUTE_WM_EVENT_NAME);
	if (ROUTE.WM_ACOM == 0)
		return FALSE;

	// création du thread du module
	ROUTE.hThread = NTSVCCreateThread(NULL, 0, ROUTE_WM_Dispatch, (PVOID)RealTimeUpdate, 0, &ROUTE.ThreadId, "SRVPVSVC ROUTE Thread");
	if (ROUTE.hThread == NULL)
		return FALSE;

	// Démarrage de la COM réseau
	if (ROUTE_ACOM_Start(ROUTE.ThreadId) == FALSE)
		return FALSE;

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL ROUTE_Stop(DWORD TimeOut)
{
	// Arret de la COM réseau
	if (ROUTE_ACOM_Terminate() == FALSE)
		return FALSE;

	// on envoie un message au thread lui demandant de se terminer
	if (PostThreadMessage(ROUTE.ThreadId, WM_QUIT, 0, 0) == FALSE)
		return FALSE;

	// on attend la disparition du thread avec délai
	if (WaitForSingleObject(ROUTE.hThread, TimeOut) != WAIT_OBJECT_0)
		return FALSE;

	// libération du handle de thread
	CloseHandle(ROUTE.hThread);

	// Destruction de la configuration de la gare en mémoire
	if (PLAZA_Delete_All() == FALSE)
		return FALSE;

	// Destruction de la liste des messages
	MSG_Delete_All_Records(&ROUTE.hlRouteMsgList);

	return TRUE;
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
PROTECTED BOOL ROUTE_PostMessage(enum ROUTE_WM_TYPE type, PVOID Param)
{
	return PostThreadMessage(ROUTE.ThreadId, ROUTE.WM_ROUTE, (WPARAM)type, (LPARAM)Param);
}

/*|*/
/*****************************************************************************/
/*SYNTAX: 						                                             */
/*===========================================================================*/
/*TYPE:		                                                                 */
/*===========================================================================*/
/*DESCRIPTION:																 */
/*===========================================================================*/
/*PARAMETERS:																 */
/*===========================================================================*/
/*  Return			Description												 */
/*---------------------------------------------------------------------------*/
/*  void            This function does not return a value.   				 */
/*****************************************************************************/
//PRIVATE BOOL WriteRead(IN BYTE *pTmp, IN OUT DWORD *pdwLen, OUT BYTE *pResult, BOOL bWrite)
//{
//	HDESKEY		hDesKey;
//	DWORD		dwDstBlockBytes = *pdwLen;
//	BYTE		abBuff[24] = { 0xD5, 0x18, 0xAC, 0x69, 0x77, 0xFC, 0xC8, 0xE2,
//		0x78, 0x17, 0xD5, 0x4A, 0x7A, 0xB4, 0xDD, 0x10,
//		0x58, 0x5E, 0x8F, 0x22, 0x4D, 0x7A, 0x29, 0x19 };
//
//
//	hDesKey = DESOpenTripleKey((BYTE(*)[8])abBuff, FALSE);
//	if (hDesKey == NULL)
//		return FALSE;
//
//	if (!DESEncryptCbc(TRUE,
//		bWrite,
//		pTmp,
//		*pdwLen,
//		pResult,
//		&dwDstBlockBytes,
//		hDesKey))
//	{
//		DESCloseKey(hDesKey);
//		return FALSE;
//	}
//
//	*pdwLen = dwDstBlockBytes;
//
//	return TRUE;
//}