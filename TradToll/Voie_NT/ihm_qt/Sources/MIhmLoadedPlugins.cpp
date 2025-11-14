
#include <QSettings>
#include <QStringList>
#include <QPluginLoader>
#include "MIhmLoadedPlugins.h"
#include "MIHMPluginINT.h"
#include "MTracer.h"

MIhmLoadedPlugins * MIhmLoadedPlugins::m_pLoadedPlugins = NULL;



MPluginSettings::~MPluginSettings()
{
	if (m_pLoader != NULL)
	{
		if (m_pLoader->isLoaded())
			m_pLoader->unload();
		delete m_pLoader;
	}
}

MIhmLoadedPlugins::MIhmLoadedPlugins()
{
	MIhmLoadedPlugins::m_pLoadedPlugins = this;
}

MIhmLoadedPlugins::~MIhmLoadedPlugins()
{
	while (!m_lstPlugins.isEmpty())
		delete m_lstPlugins.takeLast();

}

bool MIhmLoadedPlugins::InitPlugins(QString sModuleConfigKey)
{
	QString sPluginsConfigKey;
	bool bRetVal = true;

	sPluginsConfigKey = QString("HKEY_LOCAL_MACHINE\\%1\\PLUGINS").arg(sModuleConfigKey);
	
	TRACE_D(QString("MIhmLoadedPlugins::InitPlugins: RegKey:%1")
		.arg(sPluginsConfigKey));

	QSettings regBaseValuesIHM(sPluginsConfigKey, QSettings::NativeFormat);
	
	QStringList keys = regBaseValuesIHM.childGroups();

	if (keys.size() > 0)
	{
		foreach(QString sKey, keys)
		{
			QVariant varRet;

			TRACE_D(QString("MIhmLoadedPlugins::InitPlugins: RegKey:[%1] - sKey:[%2]")
				.arg(sPluginsConfigKey)
				.arg(sKey));

			regBaseValuesIHM.beginGroup(sKey);

			MPluginSettings * pNewSettings = new MPluginSettings(sKey);
			m_lstPlugins.append(pNewSettings);

			QString sPluginKey = QString("%1\\%2").arg(sPluginsConfigKey).arg(sKey);


			varRet = regBaseValuesIHM.value("DoNotLoad");
			if (!varRet.isValid())
			{
				TRACE_W(QString("MIhmLoadedPlugins::readConfig: Unable to find reg value:%1")
					.arg(sKey + "/DoNotLoad default enabled!"));
			}
			else
			{
				bool bDoNotLoad = varRet.toInt() == 0 ? false : true;
				if (bDoNotLoad)
				{
					TRACE_D(QString("MIhmLoadedPlugins::readConfig: Plugin:%1 is marked DoNotLoad!")
						.arg(sKey));

					regBaseValuesIHM.endGroup();
					continue;
				}
			}
			

			varRet.clear();

			varRet = regBaseValuesIHM.value("Dll_Name");
			if (!varRet.isValid())
			{
				TRACE_W(QString("MIhmLoadedPlugins::readConfig: Unable to find reg value:%1")
					.arg(sKey + "/Dll_Name"));
				bRetVal = false;
				break;
			}
			else
				pNewSettings->m_sDllName = varRet.toString();

			varRet.clear();

			pNewSettings->m_pLoader = new QPluginLoader(pNewSettings->m_sDllName);

			pNewSettings->m_pLoader->load();

			if (pNewSettings->m_pLoader->isLoaded())
			{
				QObject * pIHMGEP = pNewSettings->m_pLoader->instance();
				if (pIHMGEP)
				{
					pNewSettings->pPluginInstance = qobject_cast<MIHMPluginINT*>(pIHMGEP);

					if (pNewSettings->pPluginInstance->initialize(sPluginKey))
					{
						TRACE_D(QString("Initialization %1 OK!").arg(sPluginKey));
						bRetVal = true;
					}
					else
					{
						TRACE_W(QString("MIhmLoadedPlugins::readConfig:Error initializing from registry path:[%1]!").arg(sPluginKey));
						bRetVal = false;
					}
				}
				else
				{
					TRACE_W(QString("MIhmLoadedPlugins::readConfig:Error invalid instance:[%1]!").arg(pNewSettings->m_sDllName));
					bRetVal = false;
				}
			}
			else
			{
				TRACE_W(QString("MIhmLoadedPlugins::readConfig:Error loading:[%1]!").arg(pNewSettings->m_sDllName));
				bRetVal = false;
			}

			regBaseValuesIHM.endGroup();

			if (bRetVal == false)
				break;

		}
	}
	else
	{
		TRACE_D(QString("MIhmLoadedPlugins::readConfig:No plugins to initialize!"));
		bRetVal = true;
	}

	return bRetVal;
}




MIHMPluginINT * MIhmLoadedPlugins::getLoadedPlugin(QString sPluginInstName)
{
	MIHMPluginINT * pRetPluginInstance = NULL;
	
	MIhmLoadedPlugins * p = MIhmLoadedPlugins::getGlobal();
	pRetPluginInstance = p->findLoadedPlugin(sPluginInstName);

	return pRetPluginInstance;
}


MIHMPluginINT * MIhmLoadedPlugins::findLoadedPlugin(QString sPluginInstName)
{
	foreach(MPluginSettings *pSet, m_lstPlugins)
	{
		if (pSet->getName() == sPluginInstName)
		{
			return pSet->pPluginInstance;
		}
	}

	return NULL;
}



