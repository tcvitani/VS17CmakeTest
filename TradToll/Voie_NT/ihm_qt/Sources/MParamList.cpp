
#include <MParamList.h>

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <ihm.H>
	#include <reg.h>
};

MParamList::MParamList()
{

}

MParamList::~MParamList()
{
	qDeleteAll(m_lstParams);

}
	
bool MParamList::loadListFromRegistry(QString sRegKey)
{
 	bool bRetValue = false;
	char pcRegKey[IHM_LG_LIGNE_MAX] = {0};

	strcpy_s(pcRegKey, sizeof(pcRegKey), sRegKey.toLatin1().data());

    DWORD dwIndex = 0;
	char pcNomValeur[MAX_PATH] = { 0 };
	DWORD dwTailleNom = MAX_PATH;
	char pcValeur[MAX_PATH] = { 0 };
	DWORD dwTailleValeur = MAX_PATH;

	
	DWORD dwRetVal = REG_Enum_Valeurs_Chaine(
                      CSR_REG_KEYi_ROOT,
                      pcRegKey,
                      dwIndex,
                      pcNomValeur,
                      &dwTailleNom,
                      pcValeur,
                      &dwTailleValeur);   
    

    
    while(dwRetVal == ERROR_SUCCESS)
    {
		bRetValue = true;
		
        if (dwTailleNom!=0)
        {
			QString sName = QString(pcNomValeur);
			QString sValue = QString(pcValeur);

			// add each to the m_lstParams list
			m_lstParams.append(newParam(sName,sValue));
        }
         
        // Enumerer la valeur suivante
        dwIndex = dwIndex + 1;
		dwTailleNom = MAX_PATH;
		dwTailleValeur = MAX_PATH;

        dwRetVal = REG_Enum_Valeurs_Chaine(CSR_REG_KEYi_ROOT,
                      pcRegKey,
                      dwIndex,
                      pcNomValeur,
                      &dwTailleNom,
                      pcValeur,
                      &dwTailleValeur);
    }

	return bRetValue;
}



QString MParamList::getParam(QString sParamName)
{
	QString sRetVal;

	for (int i=0;i<m_lstParams.size();++i)
	{
		if(QString::compare(m_lstParams.at(i)->param_name,sParamName))
		{
			sRetVal = m_lstParams.at(i)->param_value;
			break;
		}
	}

	return sRetVal;
}

MParam * MParamList::newParam(QString sParamName, QString sValue)
{
	MParam * pParam;

	pParam = new MParam();
	pParam->param_name = sParamName;
	pParam->param_value = sValue;

	return pParam;
}


