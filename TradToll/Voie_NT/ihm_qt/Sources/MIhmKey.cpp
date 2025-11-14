
#include "MIhmKey.h"
#include "MHelpFuncs.h"
#include "MTracer.h"
 

extern "C" {
	#include <reg.h>
};


MIhmKey::MIhmKey()
{

}

MIhmKey::~MIhmKey()
{

}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED void IhmKeyInit (  )
 * PARAMETERS: Aucun
 * RETURN    : Rien
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Initialise les tableau de traitement des donnees liees aux
 *             touches.
 * --------------------------------------------------------------------
 */
void MIhmKey::IhmKeyInit(QString sConfigKey)
{
    DWORD   dwRes;
    DWORD   dwIndex;
    DWORD   dwTailleNom;
    DWORD   dwTailleValeur;
    int     iIndex;
    int     iVal;
    char    szKey[IHM_LG_CHAINE];
    char    szNomValeur[IHM_LG_CHAINE];
    char    szValeur[IHM_LG_CHAINE];

	m_sModuleConfigKey = sConfigKey;

    TRACE_D("MIhmKey::IhmKeyInit: ..." );

    // TABLEAU DE GESTION DES TOUCHES

    // Mise a KB_DONT_CARE pour toutes les touches pour toutes les taches
    memset (&gttbKeyTasks, KB_DONT_CARE, sizeof (gttbKeyTasks));

    // TABLEAU DE MAPPING DES TOUCHES

    // Mise a 0 du tableau de mapping des touches
    memset (&gtiKeyMap, 0, sizeof (gtiKeyMap));



//Read	bool m_bInvalidKeyDisplay;
	strcpy(szKey, sConfigKey.toLatin1().data());

    DWORD   dwTemp;
	if ((dwRes = REG_Lire_Entier (
                        CSR_REG_KEYi_ROOT, 
                        szKey,
                        (char*)IHM_REG_KEY_INVALID_KEY_DISPLAY, 
                        &dwTemp)) != ERROR_SUCCESS)
	{
        TRACE_W(QString( "MIhmKey::IhmKeyInit: Error key %1[%2]").arg(szKey).arg(IHM_REG_KEY_INVALID_KEY_DISPLAY));
		m_bInvalidKeyDisplay = true;
	}

	m_bInvalidKeyDisplay = (dwTemp==0)?false:true;



    // Construction du chemin de la cle de config du mappnig de touches
	QString sKey = QString("%1\\%2").arg(sConfigKey).arg(IHM_REG_KEY_KEYMAP);
	
	strcpy(szKey, sKey.toLatin1().data());

    // Lecture des touches remappees
    for ( dwIndex = 0, dwRes = ERROR_SUCCESS;
          dwRes == ERROR_SUCCESS;
          dwIndex++
        )
    {
        dwTailleNom = sizeof( szNomValeur );
        dwTailleValeur = sizeof( szValeur );
        dwRes = REG_Enum_Valeurs_Chaine(
            CSR_REG_KEYi_ROOT,
            szKey,
            dwIndex,
            szNomValeur,
            &dwTailleNom,
            szValeur,
            &dwTailleValeur);

        if ( dwRes == ERROR_SUCCESS )
        {
            // La chaine obtenue est de la forme :
            // "x| comment", x devant etre stocke dans le tableau, code a renvoyer a l'appli
            // (nom de la valeur = code ascii de la touche en decimal)
            iIndex = atoi( szNomValeur );
            iVal = atoi( szValeur );
            if ( ( iIndex >= 0 ) && ( iIndex <= IHM_KB_NB_TOUCHES_MAX ) &&
                 ( iVal   >= 0 ) && ( iVal   <= IHM_KB_NB_TOUCHES_MAX ) )
            {
                TRACE_D(QString( "MIhmKey::IhmKeyInit: Key mapping %1 -> %2").arg(iIndex).arg(iVal) );
                gtiKeyMap[iIndex] = iVal;
            }
            else
            {
                TRACE_W(QString( "MIhmKey::IhmKeyInit: Valeur de registre nom valide pour le mapping %1 = %2").arg(szNomValeur).arg(szValeur));
            }
        }
        else if ( ( dwRes != ERROR_NO_MORE_ITEMS ) && ( dwRes != ERROR_FILE_NOT_FOUND ) )
        {
            // la cle n'existe pas dans le registre
            TRACE_W(QString( "MIhmKey::IhmKeyInit: ERREUR REG_Enum_Valeurs_Chaine retourne %1").arg(dwRes));
            ExitBad ();
        }
    }

    // TABLEAU DE DEFINITION DES GROUPES DE TOUCHES

    // Mise a 0 du tableau de definition des groupes de touches
    memset( &gtsKeyGroups, 0, sizeof (gtsKeyGroups) );

    // Construction du chemin de la cle de config des groupes de touches
	sKey = QString("%1\\%2").arg(sConfigKey).arg(IHM_REG_KEY_KEYSET);
	strcpy(szKey, sKey.toLatin1().data());

    // Lecture et stockage des groupes de touches definis dans le registre
    // dans la cle szConfigKey+IHM_REG_KEY_KEYSET. Chaque valeur de la cle represente un groupe
    // qui est code code dans une chaine de la facon suivante :
    //          NomGroupe = "Key1,Key2,Key3,...,Keyn"

    if ( !IhmKeyLoadGroupsFromRegister( szKey ) )
    {
        TRACE_W("MIhmKey::IhmKeyInit: ERREUR _IhmKeyLoadGroupsFromRegister retourne false" );
        ExitBad ();
    }

    TRACE_D("MIhmKey::IhmKeyInit: Chargement effectue" );
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED bool IhmKeyUpdateKeyTable (
 *                      IN noyau_bal_id   iBalId,
 *                      IN char         * szGroup,
 *                      IN BYTE           bKey,
 *                      IN BYTE           bState )
 * PARAMETERS: IN noyau_bal_id   iBalId  : Demandeur de la modification
 *             IN char         * szGroup : Nom du groupe de touches touche, traite seulement
 *                                         si non chaine vide ou non NULL
 *             IN int            iKey    : Touche supplementaire a desactive si non 0
 *             IN BYTE           bState  : Nouvel etat (KB_DONT_CARE / KB_WAIT / KB_NM_WAIT)
 * RETURN    : true si une modification a eu lieu
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : 
 *       Cette fonction modifie l'etat de demande des touches du clavier 
 *       peager par les differentes taches de l'application. Une touche
 *       peut etre desiree (non)masquable (KB_WAIT / KB_NM_WAIT), ou
 *       non desiree (KB_DONT_CARE) par chaque tache de l'application.
 *       La demande est faite par l'utilisation du service TOUCHES
 *       Lors de l'appui d'une touche, celle-ci est envoye par l'Ihm VB 
 *       au module Ihm.dll qui fournit ensuite cette touche a toutes les
 *       taches (iBalId) qui le desirent.
 *       Les touches peuvent etre regroupees par groupe (defini dans le 
 *       registre ...\Config\Modules\Ihm\BL_XXX\KeySet), permettant de 
 *       modifier plusieurs touches par la meme demande du service TOUCHES
 * --------------------------------------------------------------------
 */
bool MIhmKey::IhmKeyUpdateKeyTable(IN noyau_bal_id   iBalId,
									IN char         * szGroup,
									IN int            iKey,
									IN BYTE           bState )
{
    bool bRes = false;
    int iGroup;
    int iKeyIndex;
    int iKeyScan;

    TRACE_D(QString( "MIhmKey::IhmKeyUpdateKeyTable: Processing... BAL_ID:%1: szGroup = %2 / iKey = %3 / bEtat = %4")
								.arg((int)iBalId).arg(szGroup).arg(iKey).arg((int)bState));

    // traitement du groupe
    if ( szGroup != NULL ) if ( strcmp( szGroup, KB_GRP_NOTUSED ) != 0 )
    {
        // recherche du groupe dans le tableau de groupe
        for ( iGroup = 0 ; iGroup < IHM_KB_NB_GROUPES_MAX ; iGroup ++ )
            if ( strncmp( gtsKeyGroups[iGroup].szGroupName, szGroup, sizeof( gtsKeyGroups[iGroup].szGroupName ) ) == 0 )
                break;

        // Si on trouve le groupe dans le tableau, 
        if ( iGroup < IHM_KB_NB_GROUPES_MAX )
        {
            // Parcourir et modifier les etats des touches de ce groupe
            for ( iKeyIndex = 0 ; iKeyIndex < IHM_KB_NB_KEY_PER_GROUP ; iKeyIndex ++ )
                if ( ( iKeyScan = gtsKeyGroups[iGroup].tiKeys[iKeyIndex] ) != 0 )
                {
                    gttbKeyTasks[iKeyScan][iBalId] = bState;
                    TRACE_D(QString( "MIhmKey::IhmKeyUpdateKeyTable: Updated: BAL_ID:%1:szGroup = %2 / iKeyScan = %3 / bEtat = %4").
										arg((int)iBalId).arg(szGroup).arg(iKeyScan).arg((int)bState));
                    bRes = true;   // Modif
                }
                else
                    break;
        }
        else
        {
            // Groupe introuvable
            TRACE_W(QString( "MIhmKey::IhmKeyUpdateKeyTable:Groupe introuvable szGroup = %1").arg(szGroup));
        }
    }

    // traitement de la touche independante
    if ( iKey != KB_KEY_NOTUSED )
    {
        TRACE_D(QString( "MIhmKey::IhmKeyUpdateKeyTable: Updated: BAL_ID:%1: iKeys = %2 / bEtat = %3").arg((int)iBalId).arg((int)iKey).arg((int)bState));
        gttbKeyTasks[iKey][iBalId] = bState;
        bRes = true;   // Modif
    }
	else
		bRes = true;

    return bRes;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED bool IhmKeyDontCare ( IN int iKey, IN int iTask )
 * PARAMETERS: IN int iKey : Touche a tester
 *             IN int iTask : Tache concernee (numero de bal)
 * RETURN    : True si la touche ne conserne par la tache identifiee par iTask
 *             False sinon
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Test si une tache n'est pas consernee par une touche
 * --------------------------------------------------------------------
 */
bool MIhmKey::IhmKeyDontCare( IN int iKey, IN int iBalId )
{
    return ( gttbKeyTasks[iKey][iBalId] == KB_DONT_CARE );
}


bool MIhmKey::IhmCommandDontCare(IN int iTask)
{
	return IhmKeyDontCare( IHM_COMMANDS_ACTIVATE_KEY, iTask);
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PROTECTED BYTE IhmKeyMap ( IN int iKey )
 * PARAMETERS: IN int iKey : Touche a mapper
 * RETURN    : 
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Effectue la translation d'une touche en une autre selon
 *             le mapping defini dans gtiKeyMap
 * --------------------------------------------------------------------
 */
int MIhmKey::IhmKeyMap( IN int iKey )
{
    return gtiKeyMap[iKey];
}


/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE bool _IhmKeyLoadGroupsFromRegister ( IN char * pcKey )
 * PARAMETERS: IN char * pcKey : Cle du registre sous laquelle trouver les infos
 * RETURN    : true si les groupes ont ete definis
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Lecture/Stockage des groupes de touches a partir du registre
 *             Format de la ligne de definition dans le registre :
 *             NomValeur = KeyCode,KeyCode,...,KeyCode
 *             Attention, la valeur "Default" ne doit pas etre traitee
 *             (faire une enumeration de la cle)
 * --------------------------------------------------------------------
 */
bool MIhmKey::IhmKeyLoadGroupsFromRegister ( IN char * pcKey )
{
    bool bRes = true;      
    HKEY hKey;
    DWORD dwRes;
    DWORD dwIdxTab = 0;
    DWORD dwIdxEnum = 0;
    DWORD dwTailleNom;
    DWORD dwTailleValeur;
    DWORD dwType;
    char  szNom[ sizeof(gtsKeyGroups[dwIdxTab].szGroupName) ];
    char  szValeur[IHM_LG_CHAINE];

    TRACE_D(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: Cle de registre %1").arg(pcKey));

    if ( ( dwRes = REG_Ouvrir( NULL, CSR_REG_KEYi_ROOT, pcKey, &hKey ) ) != ERROR_SUCCESS )
    {
        TRACE_W(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: ERREUR REG_Ouvrir(%1) retourne %2").arg(pcKey).arg(dwRes));
        return false;
    }

    dwTailleNom = sizeof (gtsKeyGroups[dwIdxTab].szGroupName);
    dwTailleValeur = sizeof (szValeur);

    // Tant qu'il y a des valeurs a enumerer
    while ( ( dwRes = REG_Enum_Valeurs(
                            hKey,
                            NULL,
                            dwIdxEnum,
                            szNom,
                            &dwTailleNom,
                            &dwType,
                            szValeur,
                            &dwTailleValeur ) ) == ERROR_SUCCESS )
    {   
        // Traitement des valeurs de type chaine uniquement
        // Pas de traitement de la valeur Default
        if ( ( dwType == REG_SZ ) && ( szNom[0] != '\0' ) )
        {   
            IHM_STRNCPY ( gtsKeyGroups[dwIdxTab].szGroupName, szNom, sizeof( gtsKeyGroups[dwIdxTab].szGroupName ) );
            TRACE_D(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: Ajout du groupe %1 = %2").arg(szNom).arg(szValeur));
            if ( !IhmKeyAnalyseKeyGroup( dwIdxTab, szValeur ) )
            {
                bRes = false;
                TRACE_W(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: ERREUR _IhmKeyAnalyseKeyGroup(%1,%2) retourne false").arg(szNom).arg(szValeur));
            }
            dwIdxTab ++;   // on avance dans le tableau
        }
        else if( szNom[0] != '\0' )
        { 
            // Valeur non chaine non traitee
            TRACE_W(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: ERREUR REG_Enum_Valeurs a trouve la valeur %1 qui n'est pas un chaine")
									.arg(szNom[0] == '\0' ? "[DEFAULT]" : szNom ));
        }
        
        if ( ! bRes )
        {
            // Pb d'analyse
            // Trace deja effectuee
            break; // On arrete
        }

        // Init pour enum suivante
        dwIdxEnum ++;
        dwTailleNom = sizeof (gtsKeyGroups[dwIdxTab].szGroupName);
        dwTailleValeur = sizeof (szValeur);
    }

    if ( bRes && ( dwIdxTab == 0 ) )
    {   
        // Aucune valeur trouvee
        bRes = false;
        TRACE_W("MIhmKey::IhmKeyLoadGroupsFromRegister:ERREUR Aucun groupe present" );
    }

    REG_Fermer( hKey );

    TRACE_D(QString( "MIhmKey::IhmKeyLoadGroupsFromRegister: retourne %1").arg(bRes ? "true" : "false" ));
    return bRes;
}



/*
 * --------------------------------------------------------------------
 * SYNTAX    : PRIVATE bool _IhmKeyAnalyseKeyGroup ( IN DWORD dwIdx, IN char * szDesc )
 * PARAMETERS: IN DWORD dwIdx   : Index du groupe dans gtsKeyGroups 
 *             IN char * szDesc : Description des touches du groupe (liste de nombres
 *                                separes par IHM_KB_GRP_SEPARATOR_CHAR)
 * RETURN    : True si ok, False si definition non valide (nombre de touches trop grand=
 * --------------------------------------------------------------------
 * VARIABLES : 
 * --------------------------------------------------------------------
 * ROLE      : Stockage des groupes de touches a partir d'une chaine lue dans le registre
 *             Format de la ligne de definition dans le registre :
 *                     KeyCode,KeyCode,...,KeyCode
 * --------------------------------------------------------------------
 */
bool MIhmKey::IhmKeyAnalyseKeyGroup ( IN DWORD dwIdx, IN char * szDesc)
{
    bool   bRes = true;
    char * pcScan;    
    int    iValue;    
    UINT   iIdxKey = 0;
	UINT   iIdxKey2 = 0;

    TRACE_D(QString( "MIhmKey::IhmKeyAnalyseKeyGroup: szGroupName = %1 / index = %2").arg(gtsKeyGroups[dwIdx].szGroupName).arg(dwIdx) );

	// tmo 24/04/01 : on va verifei que le tableau est bien initialise a 0 
	memset(gtsKeyGroups[dwIdx].tiKeys,0,IHM_KB_NB_KEY_PER_GROUP*sizeof(int));

    for ( iIdxKey = 0, pcScan = szDesc ;
          iIdxKey < IHM_KB_NB_KEY_PER_GROUP ;
          iIdxKey ++
        )
    {
        if ( ( iValue = atoi( pcScan ) ) == 0 ) break;

        gtsKeyGroups[dwIdx].tiKeys[iIdxKey] = iValue;
        TRACE_D(QString( "MIhmKey::IhmKeyAnalyseKeyGroup: Ajout de la touche %1 au groupe %2").arg(iValue).arg(gtsKeyGroups[dwIdx].szGroupName));
        pcScan = strchr( pcScan, IHM_KB_GRP_SEPARATOR_CHAR );
		iIdxKey2 ++; 

        if ( pcScan == NULL ) break;
        pcScan ++;

    }

    //if (iIdxKey1 == 0 ) // (TMO 24/04/01) modif car si une seule touche, iIdxKey=0 
    if (iIdxKey2 == 0 )
    {
        // la description est vide
        TRACE_D(QString( "MIhmKey::IhmKeyAnalyseKeyGroup: Groupe %1 vide").arg(gtsKeyGroups[dwIdx].szGroupName));
    }
	//else if (iIdxKey >= IHM_KB_NB_KEY_PER_GROUP) // TMO 24/04/01 modif car si une seule touche, iIdxKey=0 
    else if (iIdxKey2 >= IHM_KB_NB_KEY_PER_GROUP+1)
    { 
        // Format incorrect
        TRACE_W(QString( "MIhmKey::IhmKeyAnalyseKeyGroup: Format du groupe %1 incorrect ou trop long").arg(gtsKeyGroups[dwIdx].szGroupName));
        bRes = false;
    }

    TRACE_D(QString( "MIhmKey::IhmKeyAnalyseKeyGroup: retourne %1").arg(bRes ? "true" : "false"));
    return bRes;
}

