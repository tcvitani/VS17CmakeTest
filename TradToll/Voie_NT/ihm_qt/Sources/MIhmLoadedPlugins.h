#ifndef MIHM_LOADED_PLUGINS_H
#define MIHM_LOADED_PLUGINS_H

#include <QString>
#include <QList>

class MIHMPluginINT;
class QPluginLoader;

class MPluginSettings
{
	public:
		MPluginSettings(QString sKey) :m_sName(sKey), pPluginInstance(NULL){};
		~MPluginSettings();

		const QString getName(){ return m_sName; };

		const QString m_sName;
		QString m_sDllName;
		MIHMPluginINT *pPluginInstance;
		QPluginLoader *m_pLoader;
};

class MIhmLoadedPlugins
{
	
	public:
		MIhmLoadedPlugins();
		~MIhmLoadedPlugins();

		static MIhmLoadedPlugins * getGlobal(){ return m_pLoadedPlugins; };
		static MIHMPluginINT * getLoadedPlugin(QString sPluginInstName);

		bool InitPlugins(QString sModuleConfigKey);

private:
	static MIhmLoadedPlugins * m_pLoadedPlugins;

	MIHMPluginINT * findLoadedPlugin(QString sPluginInstName);

	QList <MPluginSettings *> m_lstPlugins;
};






#endif


