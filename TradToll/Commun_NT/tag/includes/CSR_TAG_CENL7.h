/* --------------------------------------------------------------------
 * (C) 2001 CS SI - UORO - All rights reserved
 * --------------------------------------------------------------------
 * MODULE     : CENL7
 * FILE       : CENL7.H
 * LANGAGE    : C/C++
 * --------------------------------------------------------------------
 * KEYWORDS   : 
 * --------------------------------------------------------------------
 * SUMMARY    : Définitions publiques couche 7 DSRC (norme CEN)
 * --------------------------------------------------------------------
 * DESCRIPTION: 
 * --------------------------------------------------------------------
 * HISTORY    :
 *
 *   * $Log : $
 *
 * --------------------------------------------------------------------
 */

#ifndef CSR_TAG_CENL7_H
#define CSR_TAG_CENL7_H


#ifdef TAG_EXPORTS
#include <public.h>
#else
#include <export.h>
#endif

#pragma pack( push, TAG_CENL7_PACK, 8 )


//
// Nombre maximum d'application gérées dans une BST ou un VST
//
#define CENL7_MAX_APPS                      5

//
// Nombre maximum de profiles gérées dans une BST ou un VST
//
#define CENL7_MAX_PROFS                     5

//
// Nombre maximum d'octets possible pour un bloc access credentials
//
#define CENL7_MAX_CREDENTIAL_BYTES          8

//
// Nombre maximum d'octets pour les données de securité
//
#define CENL7_MAX_SECURITY_DATA_BYTES       256

//
// Nombre maximum d'attributs dans une request GET/SET et dans
// les réponses équivalentes.
//
#define CENL7_MAX_ATTRS                     16

//
// Taille maximum d'un attribut (container compris)
//
#define CENL7_MAX_ATTR_BYTES                112

//
// Taille maximum des paramètres d'un event report
//
#define CENL7_MAX_EVENT_BYTES               4

//
// Taille maximum des paramètre d'une application
//
#define CENL7_MAX_APP_PARAM_BYTES           112

//
// Taille max des paramètres d'une action
//
#define CENL7_MAX_ACTION_BYTES              112

//
// Taille max d'un operand pour une action SUB ou ADD
//
#define CENL7_MAX_OPERAND_BYTES             4

//
// Taille d'une chaine a envoyer en echo
//
#define CENL7_MAX_ECHO_BYTES                128

//
// Taille max du nonce
//
#define CENL7_MAX_RND_BYTES                 4

//
// Taille max du MAC (authentificateur)
//
#define CENL7_MAX_MAC_BYTES                 4

//
// Taille max des données d'un transfer_channel
//
#define CENL7_MAX_CHANNEL_BYTES             128

//
// T/G specific : Nombre max de clés dans dans une requête TG de mise à jour
//
#define CENL7_TG_MAX_KEYS                   8

//
// T/G specific : Nombre max d'octets pour une clé dans dans une requête TG de mise à jour
//
#define CENL7_TG_KEY_BYTES                  8

//
// T/G specific : Taille max d'un bloc descriptif d'une application
//
#define CENL7_TG_MAX_APPBLOCK_BYTES         112

//
// QFree specific : Taille max d'un bloc RndOBE dans un GetNonce
//
#define CENL7_QF_MAX_RND_BYTES              6


//
// Définition des codes de requètes / réponses
//
#define CENL7_APDU_INI_REQ                  ((BYTE)0x08)
#define CENL7_APDU_INI_RSP                  ((BYTE)0x09)
#define CENL7_APDU_GET_REQ                  ((BYTE)0x06)
#define CENL7_APDU_GET_RSP                  ((BYTE)0x07)
#define CENL7_APDU_SET_REQ                  ((BYTE)0x04)
#define CENL7_APDU_SET_RSP                  ((BYTE)0x05)
#define CENL7_APDU_ACT_REQ                  ((BYTE)0x00)
#define CENL7_APDU_ACT_RSP                  ((BYTE)0x01)
#define CENL7_APDU_EVT_REQ                  ((BYTE)0x02)
#define CENL7_APDU_EVT_RSP                  ((BYTE)0x03)



//
// Définition des codes d'actions
//
#define CENL7_ACTCODE_GET_STAMPED           0
#define CENL7_ACTCODE_SET_STAMPED           1   //not used
#define CENL7_ACTCODE_GET_SECURE            2
#define CENL7_ACTCODE_SET_SECURE            3
#define CENL7_ACTCODE_GET_INSTANCE          4	//not used
#define CENL7_ACTCODE_SET_INSTANCE          5	//not used
#define CENL7_ACTCODE_GET_NONCE             6
#define CENL7_ACTCODE_SET_NONCE             7	//not used
#define CENL7_ACTCODE_TRANSFER_CHANNEL      8	//used in case of QFREE tag
#define CENL7_ACTCODE_COPY                  9	//not used
#define CENL7_ACTCODE_SET_MMI               10	//not used
#define CENL7_ACTCODE_SUBTRACT              11	//not used
#define CENL7_ACTCODE_ADD                   12	//not used
#define CENL7_ACTCODE_DEBIT                 13	//not used
#define CENL7_ACTCODE_CREDIT                14	//not used
#define CENL7_ACTCODE_ECHO                  15	//not used
#define CENL7_ACTCODE_TG_PERSO_APP          98	//used in case of Thales/GEA tag
#define CENL7_ACTCODE_PRIVATE_TRF_CHN       119	//used in case of CS tag

//
// Type de securité implémentée
//
#define CENL7_SECURITY_NONE                 0
#define CENL7_SECURITY_DES                  1
#define CENL7_SECURITY_ALGO2                2


//
// Définition du paramètre d'un message event report pour
// une fin de transaction.
//
#define CENL7_EVENT_CLOSE_LINK              0


//
// Définition des codes de container
//
#define CENL7_CHOICE_INTEGER                0
#define CENL7_CHOICE_OCTET_STRING           2
#define CENL7_CHOICE_ATTRIBUTE_ID_LIST      8
#define CENL7_CHOICE_ATTRIBUTE_LIST         9
#define CENL7_CHOICE_GET_STAMPED_RQ         17
#define CENL7_CHOICE_GET_STAMPED_RS         18
#define CENL7_CHOICE_CHANNEL_RQ             23
#define CENL7_CHOICE_CHANNEL_RS             24
#define CENL7_CHOICE_TG_PERSO_APP_RQ        100
#define CENL7_CHOICE_TG_PERSO_APP_RS        101
#define CENL7_CHOICE_VLPN                   47



//
// Pour signaler un mauvais code de retour
//
#define CENL7_INVALID                       0xFFFFFFFF
#define CENL7_DEFAULT                       0xFFFFFFFF


//
// Flags de configuration d'instance CENL7.
//
#define CENL7_FLAG_MASK                     0x000000FF

#define CENL7_FLAG_INTEGER_SINGLE           0x00000000
#define CENL7_FLAG_STD_CHANNEL              0x00000000
#define CENL7_FLAG_VARIABLE_SIZE_AC         0x00000000

#define CENL7_FLAG_INTEGER_ITERATION        0x00000001
#define CENL7_FLAG_STRING_CHANNEL           0x00000002
#define CENL7_FLAG_SUB_STRING_CHANNEL       0x00000004
#define CENL7_FLAG_FIXED_SIZE_AC            0x00000008
#define CENL7_FLAG_LONG_OBE_STATUS          0x00000010
#define CENL7_FLAG_TG_PRIVATE               0x00000020
#define CENL7_FLAG_QF_PRIVATE               0x00000040
#define CENL7_FLAG_CHANNEL_PSW              0x00000080





//
// Définition d'un bloc descriptif d'application selon le
// contenu d'une VST.
//
typedef struct 
{
    BOOL                fEIDPresent;
    BOOL                fParmPresent;
    BYTE                bAppId;
    BYTE                bEID;
	BYTE                bParmsLen;
    BYTE                tbParmsData[ CENL7_MAX_APP_PARAM_BYTES ];
}
    CENL7_APPLICATION;



//
// Définition complète d'un attribut (container+données). Si bFullLen vaut 0,
// la taille occupée dans tbRaw est déduite de ses premiers octets (container)
//
typedef struct 
{
    BYTE                bAttrId;
    BYTE                bAttrLen;
    BYTE                tbAttrData[ CENL7_MAX_ATTR_BYTES ];
}
    CENL7_ATTRIBUTE;
    


//
// Définition d'une requete d'initialisation (BST)
//
typedef struct 
{
    BOOL                fOptAppPresent;
    WORD                wManufacturer;
    DWORD               dwBeaconId;
    DWORD               dwTime;
    BYTE                bProf;
    
    BYTE                bAppCount;
    CENL7_APPLICATION   tsApps[ CENL7_MAX_APPS ];

    BYTE                bOptAppCount;
    CENL7_APPLICATION   tsOptApps[ CENL7_MAX_APPS ];

    BYTE                bProfCount;
    BYTE                tbProfs[ CENL7_MAX_PROFS ];
}
    CENL7_INI_REQ;




//
// Définition d'une réponse d'initialisation (VST)
//
typedef struct 
{
    BYTE                bProf;

    BYTE                bAppCount;
    CENL7_APPLICATION   tsApps[ CENL7_MAX_APPS ];

    BOOL                fOBEStatusPresent;

    WORD                wEqtClass;
    WORD                wManufacturerId;

    BYTE                bOBEStatus;

    BYTE                bPrivate;
}
    CENL7_INI_RSP;
    



//
// Définition d'une requete GET
//
typedef struct 
{
    BOOL                fAccCredPresent;
    BOOL                fIIDPresent;
    BOOL                fAttrIdListPresent;

    BYTE                bEID;

    BYTE                bAccCredLen;
    BYTE                tbAccCred[ CENL7_MAX_CREDENTIAL_BYTES ];

    BYTE                bIID;

    BYTE                bAttrIdCount;
    BYTE                tbAttrIds[ CENL7_MAX_ATTRS ];
}
    CENL7_GET_REQ;




//
// Définition d'une réponse GET
//
typedef struct 
{
    BOOL                fIIDPresent;
    BOOL                fAttrListPresent;
    BOOL                fStatusPresent;

    BYTE                bEID;
    
    BYTE                bIID;

    BYTE                bAttrCount;
    CENL7_ATTRIBUTE     tsAttrs[ CENL7_MAX_ATTRS ];

    BYTE                bStatus;
}
    CENL7_GET_RSP;



//
// Définition d'une requête SET
//
typedef struct 
{
    BOOL                fAccCredPresent;
    BOOL                fIIDPresent;
    BOOL                fConfirmed;

    BYTE                bEID;
    
    BYTE                bAccCredLen;
    BYTE                tbAccCred[ CENL7_MAX_CREDENTIAL_BYTES ];
    
    BYTE                bIID;

    BYTE                bAttrCount;
    CENL7_ATTRIBUTE     tsAttrs[ CENL7_MAX_ATTRS ];
}
    CENL7_SET_REQ;



//
// Définition d'une réponse SET
//
typedef struct 
{
    BOOL                fIIDPresent;
    BOOL                fStatusPresent;

    BYTE                bEID;

    BYTE                bIID;

    BYTE                bStatus;
}
    CENL7_SET_RSP;




//
// Définition d'une requête ACTION
//
typedef struct 
{
    BOOL                fAccCredPresent;
    BOOL                fParmsPresent;
    BOOL                fIIDPresent;
    BOOL                fConfirmed;

    BYTE                bEID;

    BYTE                bAccCredLen;
    BYTE                tbAccCred[ CENL7_MAX_CREDENTIAL_BYTES ];

    BYTE                bActionType;
	BYTE                bParmsLen;
    BYTE                tbParmsData[ CENL7_MAX_ACTION_BYTES ];

    BYTE                bIID;
}
    CENL7_ACT_REQ;




//
// Définition d'une réponse ACTION
//
typedef struct 
{
    BOOL                fIIDPresent;
    BOOL                fParmsPresent;
    BOOL                fStatusPresent;

    BYTE                bEID;

    BYTE                bIID;

	BYTE                bParmsLen;
    BYTE                tbParmsData[ CENL7_MAX_ACTION_BYTES ];
 
    BYTE                bStatus;
}
    CENL7_ACT_RSP;




//
// Définition d'une requête event report
//
typedef struct 
{
    BOOL                fAccCredPresent;
    BOOL                fParmsPresent;
    BOOL                fIIDPresent;
    BOOL                fConfirmed;

    BYTE                bEID;

    BYTE                bAccCredLen;
    BYTE                tbAccCred[ CENL7_MAX_CREDENTIAL_BYTES ];

    BYTE                bEventType;

	BYTE                bParmsLen;
    BYTE                tbParmsData[ CENL7_MAX_EVENT_BYTES ];

    BYTE                bIID;
}
    CENL7_EVT_REQ;




//
// Définition d'une réponse event report
//
typedef struct 
{
    BOOL                fIIDPresent;
    BOOL                fStatusPresent;

    BYTE                bEID;

    BYTE                bIID;
    BYTE                bStatus;
}
    CENL7_EVT_RSP;




//
// Définition de l'api niveau 7 CEN
//
typedef struct 
{
    BYTE                bPDU;
    BYTE                bFrag;
    BYTE                bBlockType;

    union
    {
        CENL7_INI_REQ   sIniReq;
        CENL7_INI_RSP   sIniRsp;
        CENL7_GET_REQ   sGetReq;       
        CENL7_GET_RSP   sGetRsp;
        CENL7_SET_REQ   sSetReq;
        CENL7_SET_RSP   sSetRsp;
        CENL7_ACT_REQ   sActReq;
        CENL7_ACT_RSP   sActRsp;
        CENL7_EVT_REQ   sEvtReq;
        CENL7_EVT_RSP   sEvtRsp;
    };
}
    CENL7_API;



//
// Définition de la requête d'action SET_MMI
//
typedef struct 
{
    BYTE                bMMISignal;
}
    CENL7_ACT_MMI_REQ;


//
// Définition de la requête et de la réponse d'action TRANSFER_CHANNEL
//
typedef struct 
{
    BYTE                bChannelId;
    BYTE                bChannelLen;
    BYTE                tbChannelData[CENL7_MAX_CHANNEL_BYTES];
}
    CENL7_ACT_CHN_REQ,
    CENL7_ACT_CHN_RSP;


//
// Définition de la requête et de la réponse d'action ECHO
//
typedef struct 
{
    BYTE                bEchoLen;
    BYTE                tbEcho[CENL7_MAX_ECHO_BYTES];
}
    CENL7_ACT_ECH_REQ,
    CENL7_ACT_ECH_RSP;


//
// Définition de la réponse d'action GET_NONCE
//
typedef struct 
{
    BYTE                bRndLen;
    BYTE                tbRnd[CENL7_MAX_RND_BYTES];
}
    CENL7_ACT_GNO_RSP;


//
// Définition de la réponse d'action GET_NONCE version QFree
//
typedef struct 
{
    BYTE                bRndLen;
    BYTE                tbRnd[CENL7_QF_MAX_RND_BYTES];
}
    CENL7_ACT_QFGNO_RSP;


//
// Définition de la requête GET_STAMPED
//
typedef struct 
{
    BYTE                bAttrIdCount;
    BYTE                tbAttrIds[ CENL7_MAX_ATTRS ];

    BYTE                bRndLen;
    BYTE                tbRnd[CENL7_MAX_RND_BYTES];

    BYTE                bKeyId;
}
    CENL7_ACT_GST_REQ;


//
// Définition de la réponse GET_STAMPED
//
typedef struct 
{
    BYTE                bAttrCount;
    CENL7_ATTRIBUTE     tsAttrs[ CENL7_MAX_ATTRS ];

    BYTE                bMacLen;
    BYTE                tbMac[CENL7_MAX_MAC_BYTES];
}
    CENL7_ACT_GST_RSP;


//
// Définition des requêtes SUB, ADD et SET_SECURE et de la réponse GET_SECURE
//
typedef struct 
{
    BYTE                bAttrCount;
    CENL7_ATTRIBUTE     tsAttrs[ CENL7_MAX_ATTRS ];
}
    CENL7_ACT_SUB_REQ,
    CENL7_ACT_ADD_REQ,
    CENL7_ACT_SSC_REQ,
    CENL7_ACT_GSC_RSP;


//
// Définition de la requête GET_SECURE
//
typedef struct 
{
    BYTE                bAttrIdCount;
    BYTE                tbAttrIds[ CENL7_MAX_ATTRS ];
}
    CENL7_ACT_GSC_REQ;


//
// Définition des requètes et réponses d'action sans paramètre
// (SET_MMI.RES / GET_NONCE.REQ / SUB.RES / ADD.RES, SET_SECURE.RES )
//
typedef struct 
{
    BYTE                bFoo;
}
    CENL7_ACT_MMI_RSP,
    CENL7_ACT_GNO_REQ,
    CENL7_ACT_SUB_RSP,
    CENL7_ACT_ADD_RSP,
    CENL7_ACT_SSC_RSP;



//
// Définition de la requête SET_SECURE (version TG)
//
typedef struct 
{
    BYTE                bKeyCount;
    struct
    {
        BYTE            bKeyRef;
        BYTE            tbKeyAccess[CENL7_MAX_MAC_BYTES];
        BYTE            tbCryptedKey[CENL7_TG_MAX_KEYS];
    }
                        tsKeys[CENL7_TG_MAX_KEYS];

    BYTE                bRndLen;
    BYTE                tbRnd[CENL7_MAX_RND_BYTES];
}
    CENL7_ACT_TGSSC_REQ;


//
// Définition de la réponse SET_SECURE (version TG)
//
typedef struct 
{
    BYTE                bKeyCount;
    struct
    {
        BYTE            bKeyRef;
        BYTE            tbKvc[CENL7_MAX_MAC_BYTES];
    }
                        tsKvcs[CENL7_TG_MAX_KEYS];
}
    CENL7_ACT_TGSSC_RSP;




//
// Définition de la requête PERSO_APP (version TG)
//
typedef struct 
{
    BOOL                fAppend;
    BYTE                bBlocksTotal;
    BYTE                bBlockNumber;
    BYTE                bBlockLen;
    BYTE                tbBlock[CENL7_TG_MAX_APPBLOCK_BYTES];
}
    CENL7_ACT_TGPAP_REQ;


//
// Définition de la réponse PERSO_APP (version TG)
//
typedef struct 
{
    BYTE                bBlockNumber;
    BYTE                bBlockStatus;
}
    CENL7_ACT_TGPAP_RSP;



typedef union
{
    CENL7_ACT_MMI_REQ   sMmiReq;
    CENL7_ACT_MMI_RSP   sMmiRsp;
    CENL7_ACT_GNO_REQ   sGNoReq;
    CENL7_ACT_GNO_RSP   sGNoRsp;
    CENL7_ACT_QFGNO_RSP sQFGNoRsp;
    CENL7_ACT_GST_REQ   sGStReq;
    CENL7_ACT_GST_RSP   sGStRsp;
    CENL7_ACT_ECH_REQ   sEchReq;
    CENL7_ACT_ECH_RSP   sEchRsp;
    CENL7_ACT_CHN_REQ   sChnReq;
    CENL7_ACT_CHN_RSP   sChnRsp;
    CENL7_ACT_SUB_REQ   sSubReq;
    CENL7_ACT_SUB_RSP   sSubRsp;
    CENL7_ACT_ADD_REQ   sAddReq;
    CENL7_ACT_ADD_RSP   sAddRsp;
    CENL7_ACT_GSC_REQ   sGScReq;
    CENL7_ACT_GSC_RSP   sGScRsp;
    CENL7_ACT_SSC_REQ   sSScReq;
    CENL7_ACT_SSC_RSP   sSScRsp;
    CENL7_ACT_TGSSC_REQ sTGSScReq;
    CENL7_ACT_TGSSC_RSP sTGSScRsp;
    CENL7_ACT_TGPAP_REQ sTGPApReq;
    CENL7_ACT_TGPAP_RSP sTGPApRsp;
}
    CENL7_ACT_PARAMS;



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7_TYPE_DECODER(
 *                      IN      DWORD                dwUserData,
 *                      IN      BYTE               * pbBufWithContainer,
 *                      IN      DWORD                dwBufBytes,
 *                      OUT     DWORD              * pdwRecBytesWithoutContainer )
 * PARAMETERS: dwUserData : As passed when CENL7RegisterDecoder was called.
 *             pbBufWithContainer : pointer to the first byte the datablock (the
 *                                  byte where the container code is)
 *             dwBufBytes         : Total size of the buffer
 *             pdwRecBytesWithoutContainer : Return the size of
 *                the data, without the container header.
 * RETURN    : CENL7_INVALID if encoding is wrong or buffer too small.
 *             Otherwise, the total size of the data bloc according to its type
 *             (from the byte where the container code is to the last data byte).
 * --------------------------------------------------------------------
 * ROLE      : Callback function definition for decoding data structure with
 *             complex structure that cannot just be handled by a container code
 *             and a size.
 * --------------------------------------------------------------------
 */
typedef DWORD WINAPI CENL7_TYPE_DECODER(
        IN      DWORD                dwUserData,
        IN      BYTE               * pbBufWithContainer,
        IN      DWORD                dwBufBytes,
        OUT     DWORD              * pdwRecBytesWithoutContainer );



#ifdef TAG_EXPORTS


#define CONTAINER_CODED_SIZE    0x80
#define CONTAINER_SIZE_MASK     0x7F

typedef struct _CENL7_INSTANCE 
{
    DWORD                           dwFlags;
    
    BYTE                            tbContainers[128];
    BYTE                            tbAttributes[128];

	DWORD                           dwSecurityType;

    DWORD                           dwMasterSecurityLen;
    BYTE                            tbMasterSecurity[ CENL7_MAX_SECURITY_DATA_BYTES ];

    DWORD                           dwDerivedSecurityLen;
    BYTE                            tbDerivedSecurity[ CENL7_MAX_SECURITY_DATA_BYTES ];

    DWORD                           dwAccessCredentialsLen;
    BYTE                            tbAccessCredentials[ CENL7_MAX_CREDENTIAL_BYTES ];

    CENL7_TYPE_DECODER *            tpfDecoders[128];
    DWORD                           tdwDecoders[128];
}
    CENL7_INSTANCE;


#else

typedef struct _CENL7_INSTANCE CENL7_INSTANCE;

#endif

    




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT CENL7_INSTANCE * WINAPI CENL7Open()
 * PARAMETERS: Aucun
 * RETURN    : Un handle de la nouvelle instance créée ou NULL en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Mise en place d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un bagde.
 * --------------------------------------------------------------------
 */
EXPORT CENL7_INSTANCE * WINAPI CENL7Open();




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7SetMode( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwModeFlags )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             dwModeFlags : Options de l'instance
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Définit le mode de fonctionnement de l'instance.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7SetMode( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwModeFlags );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7RegisterDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId,
 *                      IN      CENL7_TYPE_DECODER    * pfDecoder,
 *                      IN      DWORD                   dwUserData )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 *             pfDecoder   : Pointeur sur la fonction de décodage.
 *             dwUserData  : User data à passer en parametre des callback à pfDecoder
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Ajoute un décodeur spécial pour un type de container
 *             particulier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7RegisterDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId,
        IN      CENL7_TYPE_DECODER    * pfDecoder,
        IN      DWORD                   dwUserData );
        



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7UnregisterDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Supprimer un décodeur spécial pour un type de container
 *             particulier.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7UnregisterDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7GetDecoder( 
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                    bContId )
 *                         OUT  CENL7_TYPE_DECODER   ** ppfDecoder,
 *                         OUT  DWORD                 * pdwUserData )
 * PARAMETERS: psInst      : Handle d'instance créée avec CENL7Open().
 *             bContId  : Code container du type à décoder
 *             ppfDecoder   : retourne un pointeur sur la fonction de décodage. Use NULL to unregister.
 *             pdwUserData  : retourne les user data associée
 * RETURN    : TRUE en cas de succés, FALSE en cas d'erreur.
 * --------------------------------------------------------------------
 * ROLE      : Obtenir les information sur un décodeur enregistré
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7GetDecoder( 
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                    bContId,
           OUT  CENL7_TYPE_DECODER   ** ppfDecoder,
           OUT  DWORD                 * pdwUserData );


/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7Close(
 *                      IN OUT  CENL7_INSTANCE         * psInst )
 * PARAMETERS: psInst : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Fermeture d'un contexte d'utilisation d'une couche niveau 7
 *             de communication avec un badge.
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7Close(
        IN OUT  CENL7_INSTANCE         * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7GetRecordSize(
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE               * pbBuf,
 *                      IN      DWORD                dwBufBytes,
 *                      OUT     DWORD              * pdwRecBytesWithoutContainer
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             pbBuf      : Buffer contenant des données
 *             dwBufBytes : Taille totale des données
 *             pdwRecBytesWithoutContainer : Usefull data size without
 *                          container related data. NULL si non utilisé.
 * RETURN    : Taille du premier bloc de données dans le buffer.
 * --------------------------------------------------------------------
 * ROLE      : Calcul la taille du premier bloc de données dans le buffer,
 *             le premier octet étant utilisé comme un type ASN1.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI CENL7GetRecordSize(
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE               * pbBuf,
        IN      DWORD                dwBufBytes,
        OUT     DWORD              * pdwRecBytesWithoutContainer );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT DWORD WINAPI CENL7GetBitRecordSize(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN      BYTE              * pbBuf,
 *                      IN      DWORD               dwBitPos,
 *                      IN      DWORD               dwBufBits,
 *                      OUT     DWORD             * pdwRecBitsWithoutCountainer )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             pbBuf      : Buffer contenant des données
 *             dwBitPos   : Position du premier bit
 *             dwBufBytes : Taille du buffer en bits, à partir du premier bit.
 *             pdwRecBitsWithoutCountainer : Usefull data size without
 *                          container related data. NULL si non utilisé.
 * RETURN    : Taille du premier bloc de données dans le buffer, en bits
 * --------------------------------------------------------------------
 * ROLE      : Calcul la taille du premier bloc de données dans le buffer,
 *             le premier octet étant utilisé comme un type ASN1.
 * --------------------------------------------------------------------
 */
EXPORT DWORD WINAPI CENL7GetBitRecordSize(
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE               * pbBuf,
        IN      DWORD                dwBitPos,
        IN      DWORD                dwBufBits,
        OUT     DWORD              * pdwRecBitsWithoutCountainer );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7BuildApdu( 
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      CENL7_API         * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer destiné à recevoir les données
 *             pdwBufBytePos : En entrée, position du premier octet à écrire dans le buffer,
 *                             En sortie, position de l'octet immmédiatement après le dernier écrit
 *             pdwBufByteLen : En entrée, taille du buffer disponible, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             psDef         : Définition du bloc à générer
 * RETURN    : TRUE si le bloc a été généré. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Génération d'un bloc CEN dans buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7BuildApdu( 
        IN      CENL7_INSTANCE     * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      CENL7_API         * psDef );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7AnalyseApdu( 
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN OUT  BYTE              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      OUT     CENL7_API         * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer contenant des données
 *             pdwBufBytePos : En entrée, position du premier octet,
 *                             En sortie, position de l'octet immmédiatement après le dernier utilisé
 *             pdwBufByteLen : En entrée, taille du buffer en octets, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             psDef         : Récupère la définition du bloc analysé.
 * RETURN    : TRUE si le bloc a été analysé. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Analyse le premier bloc CEN d'un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7AnalyseApdu( 
        IN      CENL7_INSTANCE     * psInst,
        IN OUT  BYTE              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        OUT     CENL7_API         * psDef );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7BuildActionParams(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      OUT     char              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      BYTE                bActionType,
 *                      IN      CENL7_ACT_PARAMS  * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer destiné à recevoir les données
 *             pdwBufBytePos : En entrée, position du premier octet à écrire dans le buffer,
 *                             En sortie, position de l'octet immmédiatement après le dernier écrit
 *             pdwBufByteLen : En entrée, taille du buffer disponible, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             bActionType   : Type d'action à traiter.
 *             psDef         : Définition des paramètres de l'action.
 * RETURN    : TRUE si le bloc a été généré. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Génération d'un bloc de paramètre d'action dans un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7BuildActionParams(
        IN      CENL7_INSTANCE    * psInst,
        OUT     char              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      BYTE                bActionType,
        IN      CENL7_ACT_PARAMS  * psDef );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7AnalyseActionParams(
 *                      IN      CENL7_INSTANCE    * psInst,
 *                      IN      char              * pbBuf,
 *                      IN OUT  DWORD             * pdwBufBytePos,
 *                      IN OUT  DWORD             * pdwBufByteLen,
 *                      IN      BYTE                bActionType,
 *                      OUT     CENL7_ACT_PARAMS  * psDef )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             pbBuf         : Buffer contenant des données
 *             pdwBufBytePos : En entrée, position du premier octet,
 *                             En sortie, position de l'octet immmédiatement après le dernier utilisé
 *             pdwBufByteLen : En entrée, taille du buffer en octets, à partir de la position donnée en entrée,
 *                             En sortie, taille du buffer restant en octets, à partir de la position donnée en sortie,
 *             bActionType   : Type d'action à traiter.
 *             psDef         : Récupère la définition du bloc analysé.
 * RETURN    : TRUE si le bloc a été analysé. FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Analyse un bloc de paramètre d'action dans un buffer.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7AnalyseActionParams(
        IN      CENL7_INSTANCE    * psInst,
        IN      char              * pbBuf,
        IN OUT  DWORD             * pdwBufBytePos,
        IN OUT  DWORD             * pdwBufByteLen,
        IN      BYTE                bActionType,
        OUT     CENL7_ACT_PARAMS  * psDef );

        



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7SetMasterSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwSecurityType,
 *                      IN      BYTE                  * pbSecurityData,
 *                      IN      DWORD                   dwSecurityLen )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             dwSecurityType: Type de sécurité utilisé (CENL7_SECURITY_NONE, CENL7_SECURITY_DES ou CENL7_SECURITY_ALGO2)
 *             pbSecurityData: Pointe sur un bloc contenant les données maitresses de sécurité.
 *                              - Dans le cas de la sécurité DES, il s'agit d'une succession
 *                               de 9 clés sur 16 octets chacunes.La première clé est la clé
 *                               d'accés par défaut. Les clés non connues doivent être initialisée à 0.
 *                              - Dans le cas de la sécurité ALGO2, il s'agit d'une clé sur 6 octets.
 *             dwSecurityLen : Taille du bloc contenant les données de sécurité maitre (6, 144).
 * RETURN    : TRUE si les données sont acceptables, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les données maitresses de la sécurité.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7SetMasterSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwSecurityType,
        IN      BYTE                  * pbSecurityData,
        IN      DWORD                   dwSecurityLen );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7SetDerivedSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      DWORD                   dwSecurityType,
 *                      IN      BYTE                  * pbSecurityData,
 *                      IN      DWORD                   dwSecurityLen )
 * PARAMETERS: psInst        : Handle d'instance créée avec CENL7Open().
 *             dwSecurityType: Type de sécurité utilisé (CENL7_SECURITY_NONE, CENL7_SECURITY_DES ou CENL7_SECURITY_ALGO2)
 *             pbSecurityData: Pointe sur un bloc contenant les données dérivées de sécurité.
 *                              - Dans le cas de la sécurité DES, il s'agit d'une succession
 *                               de 9 clés sur 8 octets chacunes. La première clé est la clé
 *                               d'accés par défaut. Les clés non connues doivent être initialisée à 0.
 *                               Si la référence à la clé n'est pas fournie, la valeur utilisée est 0.
 *                              - Dans le cas de la sécurité ALGO2, il s'agit d'une clé sur 6 octets.
 *             dwSecurityLen : Taille du bloc contenant les données de sécurité dérivées (6, 72).
 * RETURN    : TRUE si les données sont acceptables, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les données dérivées de la sécurité. Permet de passer outre
 *             la phase de dérivation.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7SetDerivedSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      DWORD                   dwSecurityType,
        IN      BYTE                  * pbSecurityData,
        IN      DWORD                   dwSecurityLen );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7DeriveSecurity(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbDerivationData,
 *                      IN      DWORD                   dwDerivationLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbDerivationData : Pointe sur les données à dériver pour obtenir les données
 *                                de sécurité de session.
 *             dwDerivationLen  : Taille des données de derivation.
 * RETURN    : TRUE si la dérivation a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue une dérivation des données de sécurité initialisées
 *             par un appel à CENL7SetMasterSecurity().
 *             En mode DES A1, les données à dériver sont constituées soit d'un
 *             bloc constitué des deux octets OBEGroupId (fourni en paramètre d'application
 *             dans la VST), soit d'un bloc de 7 octets constitué de la concaténation
 *             des 4 octets du ContractSerialNumber, et des 3 octets du ContractProvider.
 *             Dans le premier cas, seule la clé d'accés est dérivée, dans le second cas,
 *             seules les clés d'authentification sont dérivées.
 *             En mode ALGO2, les données à dériver sont constituées d'un
 *             bloc de 6 octets dont les deux premiers valent 0, les 4 suivant
 *             sont le contenu du champ "RSETime" de la BST.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7DeriveSecurity(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbDerivationData,
        IN      DWORD                   dwDerivationLen );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7GetAccessCredentials(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbNonce,
 *                      IN      DWORD                   dwNonceLen,
 *                      IN      DWORD                   dwKeyRef,
 *                      OUT     BYTE                  * pbAC,
 *                      IN OUT  DWORD                 * pdwACLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbNonce          : Pointe sur les données d'entrée pour le calcul des
 *                                access credentials.
 *             dwNonceLen       : Taille des données d'entrée
 *             dwKeyRef         : Référence à la clé de calcul.
 *                                 - Avec la sécurité DES : de 0 à 8, ou CENL7_DEFAULT.
 *                                 - Avec la sécurité ALGO2 : Non utilisé
 *             pbAC             : Pointe sur le buffer qui récupéra le bloc des access
 *                                credentials.
 *             pdwACLen         : En entrée, taille max du buffer pointé par pbAC.
 *                                En sortie, taille effectivement utilisée par le bloc
 *                                des access credentials.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le calcul des access credentials basé sur les données
 *             de sécurité initialisées par un appel à CENL7SetMasterSecurity() et
 *             dérivées par un appel à CENL7DeriveSecurity().
 *             En mode DES A1, les données Nonce sont constituées d'un
 *             bloc de 4 octets constitué de la dernière valeur Nonce obtenue d'un OBE
 *             (par les paramètres d'une application dans la VST ou par un message
 *             GET_NONCE.RESPONSE).
 *             En mode ALGO2, les données à dériver sont constituées d'un
 *             bloc de 6 octets constitué de l'attribut ContextMark.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7GetAccessCredentials(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbNonce,
        IN      DWORD                   dwNonceLen,
        IN      DWORD                   dwKeyRef,
        OUT     BYTE                  * pbAC,
        IN OUT  DWORD                 * pdwACLen );






/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT  BOOL WINAPI CENL7GetAuthenticator(
 *                      IN OUT  CENL7_INSTANCE        * psInst,
 *                      IN      BYTE                  * pbData,
 *                      IN      DWORD                   dwDataLen,
 *                      IN      DWORD                   dwKeyRef,
 *                      OUT     BYTE                  * pbMAC,
 *                      IN OUT  DWORD                 * pdwMACLen )
 * PARAMETERS: psInst           : Handle d'instance créée avec CENL7Open().
 *             pbData           : Pointe sur les données pour lesquelles un authentificateur
 *                                doit être généré.
 *             dwDataLen        : Taille des données à authentifier.
 *             dwKeyRef         : Référence à la clé de calcul.
 *                                 - Avec la sécurité DES : de 0 à 8, ou CENL7_DEFAULT.
 *                                 - Avec la sécurité ALGO2 : Non utilisé
 *             pbMAC            : Pointe sur le buffer qui récupéra le bloc d'authentification.
 *                                credentials.
 *             pdwMACLen        : En entrée, taille max du buffer pointé par pbMAC.
 *                                En sortie, taille effectivement utilisée par le bloc
 *                                de l'authentificateur.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le calcul d'un authentificateur basé sur les données
 *             de sécurité initialisées par un appel à CENL7SetMasterSecurity() et
 *             dérivées par un appel à CENL7DeriveSecurity().
 *             ATTENTION : Utilisable uniquement en mode DES.
 * --------------------------------------------------------------------
 */
EXPORT  BOOL WINAPI CENL7GetAuthenticator(
        IN OUT  CENL7_INSTANCE        * psInst,
        IN      BYTE                  * pbData,
        IN      DWORD                   dwDataLen,
        IN      DWORD                   dwKeyRef,
        OUT     BYTE                  * pbMAC,
        IN OUT  DWORD                 * pdwMACLen );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7ClearAttributDefinitions(
 *                      IN      CENL7_INSTANCE     * psInst )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Remet à zéro la table de définition des attributs
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7ClearAttributDefinitions(
        IN      CENL7_INSTANCE     * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT void WINAPI CENL7ClearContainerDefinitions(
 *                      IN      CENL7_INSTANCE     * psInst )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * ROLE      : Remet à zéro la table de définition des container ASN1
 * --------------------------------------------------------------------
 */
EXPORT void WINAPI CENL7ClearContainerDefinitions(
        IN      CENL7_INSTANCE     * psInst );




/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7DefineAttribute( 
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE                 bAttrId,
 *                      IN      BYTE                 bContId )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             bAttrId    : Identifiant de l'attribut à définir (0 à 127)
 *             bContId    : Identifiant de container ASN1.
 * RETURN    : TRUE  : OK
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini le container ASN1 associé à un attribut.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7DefineAttribute( 
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE                 bAttrId,
        IN      BYTE                 bContId );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7DefineContainer( 
 *                      IN      CENL7_INSTANCE     * psInst,
 *                      IN      BYTE                 bContId,
 *                      IN      BOOL                 fCodedSize,
 *                      IN      BYTE                 bBytesLen )
 * PARAMETERS: psInst     : Handle d'instance créée avec CENL7Open().
 *             bContId    : Identifiant de container ASN1.
 *             bContId    : Identifiant de container ASN1.
 *             fCodedSize : TRUE : La taille des données est codée systématiquement
 *                                 avec les données elle-mêmes.
 *                          FALSE: La taille des données est implicite.
 *             bBytesLen  : Taille implicite des données (n'est exploité que si 
 *                          fCodedSize vaut FALSE.
 * RETURN    : TRUE  : OK
 *             FALSE : Erreur
 * --------------------------------------------------------------------
 * ROLE      : Défini les codage d'un container ASN1.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7DefineContainer( 
        IN      CENL7_INSTANCE     * psInst,
        IN      BYTE                 bContId,
        IN      BOOL                 fCodedSize,
        IN      BYTE                 bBytesLen );



/*
 * --------------------------------------------------------------------
 * SYNTAX    : EXPORT BOOL WINAPI CENL7Algo2Cipher(
 *                      IN      BYTE      * pbSrcBlock, 
 *                      IN      DWORD       dwSrcBlockBytes,
 *                      OUT     BYTE      * pbDstBlock,
 *                      IN OUT  DWORD     * pdwDstBlockBytes,
 *                      IN      BYTE      * pbKey )
 * PARAMETERS: pbSrcBlock      : Pointe sur le bloc de données source
 *             dwSrcBlockBytes : Taille du bloc de données source
 *             pbDstBlock      : Pointe sur le bloc de données récupérant le résultat
 *             pdwDstBlockBytes: En entrée, taille max du bloc de destination,
 *                               En sortie, taille effective des données calculées.
 *             pbKey           : Pointeur sur un bloc de 6 octets contenant la clé de chiffrement.
 * RETURN    : TRUE si le calcul a été effectuée, FALSE en cas d'erreur
 * --------------------------------------------------------------------
 * ROLE      : Effectue le chiffrement ALGO2 d'un bloc de données. Attention, ce n'est pas
 *             une encryption dans la mesure ou ce chiffrement n'est pas réversible.
 * --------------------------------------------------------------------
 */
EXPORT BOOL WINAPI CENL7Algo2Cipher(
        IN      BYTE      * pbSrcBlock, 
        IN      DWORD       dwSrcBlockBytes,
        OUT     BYTE      * pbDstBlock,
        IN OUT  DWORD     * pdwDstBlockBytes,
        IN      BYTE      * pbKey );


#pragma pack( pop, TAG_CENL7_PACK )

        
#endif

