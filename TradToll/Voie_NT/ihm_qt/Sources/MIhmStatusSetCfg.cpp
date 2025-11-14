
#include "MIhmStatusSetCfg.h"
#include "MHelpFuncs.h"
#include "MTracer.h"
#include <QStringList.h>
 

extern "C" {
	#include <reg.h>
};



MStatusSetGroup::~MStatusSetGroup()
{
	while(!lstStatusSetGroupIds.isEmpty())
		delete lstStatusSetGroupIds.takeLast();
}

bool MStatusSetGroup::appendObject(QString sCfgSetting)
{
	QStringList sListObjects = QString(sCfgSetting).split(IHM_STATUS_OBJ_ID_SEPARATOR_CHAR);
	
	if(sListObjects.size()>0)
	{
		QString sObjId = sListObjects.at(0).trimmed();
		enum_ihm_objets  objet = (enum_ihm_objets)MHelpFuncs::getObjectIdFromString(sObjId);
		
		//TO DO define special parsing for specific type of objects (e.g. menus, ..)
		if(objet>0) 
		{
			SStatusSetGroupObject * pSSObject = new SStatusSetGroupObject();
			pSSObject->objet = objet;
			lstStatusSetGroupIds.append(pSSObject);
			return true;
		}
		else
			return false;
	}

	return false;
}

//--------------------------------------------------------

//----------------------------------------------------------
StatusSetGroupList::StatusSetGroupList()
{
	m_plstOrderedStatusSetGroups = new QList<MStatusSetGroup*>();
}

StatusSetGroupList::~StatusSetGroupList()
{
	while(!m_plstOrderedStatusSetGroups->isEmpty())
		delete m_plstOrderedStatusSetGroups->takeLast();

	delete m_plstOrderedStatusSetGroups;

}



bool StatusSetGroupList::lessThan(const MStatusSetGroup* x1, const MStatusSetGroup* x2) 
{ 
	return x1->getID() < x2->getID(); 
}


void StatusSetGroupList::sort()
{
	//qSort(m_plstOrderedStatusSetGroups->begin(), m_plstOrderedStatusSetGroups->end(), StatusSetGroupList::lessThan);
	std::sort(m_plstOrderedStatusSetGroups->begin(), m_plstOrderedStatusSetGroups->end(), StatusSetGroupList::lessThan);
}


bool StatusSetGroupList::find(QString  &sGroupName, MStatusSetGroup ** pFound)
{
	MStatusSetGroup oWanted(sGroupName);
	
	QList<MStatusSetGroup*>::iterator it;
	
	//it = qBinaryFind(m_plstOrderedStatusSetGroups->begin(), m_plstOrderedStatusSetGroups->end(), &oWanted, StatusSetGroupList::lessThan);
	it = std::lower_bound(m_plstOrderedStatusSetGroups->begin(), m_plstOrderedStatusSetGroups->end(), &oWanted, StatusSetGroupList::lessThan);

	if( it !=  m_plstOrderedStatusSetGroups->end())
	{
		*pFound = *it;
		return true;
	}

	return false; //NULL
}

//--------------------------------------------------------


MIhmStatusSetCfg::MIhmStatusSetCfg()
{

}

MIhmStatusSetCfg::~MIhmStatusSetCfg()
{

}


void MIhmStatusSetCfg::Init(QString sConfigKey)
{
    char    szKey[IHM_LG_CHAINE];

    TRACE_D("MIhmStatusSetCfg::Init: ..." );

	QString sKey = QString("%1\\%2").arg(sConfigKey).arg(IHM_REG_STATUS_SET);
	strcpy(szKey, sKey.toLatin1().data());

    if ( !MIhmStatusSetLoadGroupsFromRegister( szKey ) )
    {
        TRACE_W("MIhmStatusSetCfg::Init: ERREUR MIhmStatusSetLoadGroupsFromRegister retourne false" );
        ExitBad ();
    }

    TRACE_D("MIhmStatusSetCfg::Init: Init OK!" );
}

bool MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister(IN char * pcKey)
{
    bool bRes = true;      
    HKEY hKey;
    DWORD dwRes;
    DWORD dwIdxTab = 0;
    DWORD dwIdxEnum = 0;
    DWORD dwTailleNom;
    DWORD dwTailleValeur;
    DWORD dwType;
    char  szValeur[IHM_LG_CHAINE];
    char  szGroupName[IHM_LG_CHAINE];
	

     TRACE_D(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: Cle de registre %1").arg(pcKey));

    if ( ( dwRes = REG_Ouvrir( NULL, CSR_REG_KEYi_ROOT, pcKey, &hKey ) ) != ERROR_SUCCESS )
    {
        TRACE_W(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: ERREUR REG_Ouvrir(%1) retourne %2").arg(pcKey).arg(dwRes));
        return false;
    }

    dwTailleNom = sizeof(szGroupName);
    dwTailleValeur = sizeof(szValeur);

    // Tant qu'il y a des valeurs e enumerer
    while ( ( dwRes = REG_Enum_Valeurs(
                            hKey,
                            NULL,
                            dwIdxEnum,
                            szGroupName,
                            &dwTailleNom,
                            &dwType,
                            szValeur,
                            &dwTailleValeur ) ) == ERROR_SUCCESS )
    {   
        // Traitement des valeurs de type chaine uniquement
        // Pas de traitement de la valeur Default
        if ( ( dwType == REG_SZ ) && ( szGroupName[0] != '\0' ) )
        {   
			TRACE_D(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: Creating group %1 = %2").arg(szGroupName).arg(szValeur));
            
			if ( !MIhmAnalyseStatusSetGroup( szGroupName, szValeur ) )
            {
                bRes = false;
                TRACE_W(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: ERREUR MIhmAnalyseStatusSetGroup(%1,%2) ").arg(szGroupName).arg(szValeur));
            }

            dwIdxTab ++;   // on avance dans le tableau
        }
        else if( szGroupName[0] != '\0' )
        { 
            // Valeur non chaine non traitee
            TRACE_W(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: ERREUR REG_Enum_Valeurs a trouve la valeur [%1] which is not a string!")
									.arg(szGroupName));
        }
        
        if ( ! bRes )
        {
            // Pb d'analyse
            // Trace deje effectuee
            break; // On arrete
        }

        // Init pour enum suivante
        dwIdxEnum ++;
		dwTailleNom = sizeof(szGroupName);
		dwTailleValeur = sizeof(szValeur);
    }

    if ( bRes && ( dwIdxTab == 0 ) )
    {   
        // Aucune valeur trouvee
        bRes = false;
        TRACE_W("MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister:ERREUR: No group is present!" );
    }
	else
	{
		m_lstStatusSetGroups.sort();
	}

    REG_Fermer( hKey );

    TRACE_D(QString( "MIhmStatusSetCfg::MIhmStatusSetLoadGroupsFromRegister: returned %1").arg(bRes ? "true" : "false" ));
    return bRes;
}


bool MIhmStatusSetCfg::MIhmAnalyseStatusSetGroup ( IN char* szGroupName, IN char * szDesc)
{
    bool   bRes = true;
	QString sGroupName;

	sGroupName = szGroupName;
	sGroupName = sGroupName.trimmed();

    TRACE_D(QString( "MIhmStatusSetCfg::MIhmAnalyseStatusSetGroup: sGroupName =[%1] / value = [%2]").arg(sGroupName).arg(szDesc) );

	if(QString(szDesc).trimmed().size()==0)
		return true;

	QStringList sListObjects = QString(szDesc).split(IHM_STATUS_OBJ_SEPARATOR_CHAR);

	if(sListObjects.size()>0)
	{
		MStatusSetGroup *pNewGroup = new MStatusSetGroup(sGroupName);

		for(int i=0;i<sListObjects.size();i++)
		{
			QString sCurrObjSettings = sListObjects.at(i).trimmed();

			if(!pNewGroup->appendObject(sCurrObjSettings))
			{
				bRes = false;
				break;
			}	
		}	

		if(bRes)
		{
			m_lstStatusSetGroups.append(pNewGroup);
		}	
		else
		{
			delete pNewGroup;
			TRACE_W(QString( "MIhmStatusSetCfg::MIhmAnalyseStatusSetGroup: Error creating sGroupName =[%1] / value = [%2]").arg(sGroupName).arg(szDesc));
		}	
	}
 
    TRACE_D(QString( "MIhmStatusSetCfg::MIhmAnalyseStatusSetGroup: retourne %1").arg(bRes ? "true" : "false"));
    return bRes;
}


MStatusSetGroup * MIhmStatusSetCfg::getStatusSetGroup(QString sGroupName)
{
	MStatusSetGroup * pFound = NULL;

	if(m_lstStatusSetGroups.find(sGroupName, &pFound))
		return pFound;
	else
		return NULL;
}
