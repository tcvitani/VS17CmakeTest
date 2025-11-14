#ifndef MIHM_CONFIG_WEB_H
#define MIHM_CONFIG_WEB_H

#include <QObject>
#include <QList>
#include <QString>
#include <QFont>
#include <QMap>



#include "MIhmLaneTypeSettings.h"
#include "MIhmConfigColor.h"
#include "MIhmLanguages.h"
#include "MIhmConfigImages.h"
#include "MIhmConfigActions.h"
#include "MIhmConfigDynamic.h"

#include "MHtmlCodec.h"

class MParamList;

//Default data read from registry
class MIhmConfigWeb
{
	
	public:
		MIhmConfigWeb();
		~MIhmConfigWeb();

		MIhmConfigColor * getConfigColors(){return &m_oConfigColors;};
		bool loadConfigFromRegistry(QString sRegKey); 

		bool reloadDynamicConfig();
		
		int getWebPort(){return m_iWebServerPort;};
	
		QString getModuleConfigKey(){return m_sModuleConfigKey;};
		
			
		MIhmLaneTypeSettings * getLaneTypeSetting(MIhmLaneTypeSettings::enumSettingsType eType,
												  QString sLaneType);

		QString getDefaultLanguage(){return m_sDefaultLanguage;};

		bool parsePoliceValueForWeb(QString sPoliceValue, QString& sStyle);

		QString getInitWebPage(){return m_sInitWebPage;};

		QString getLoginFailPage(){return m_sLoginFailPage;};
		QString getLogoutPage(){return m_sLogoutPage;};
		int getWebSessionTimeout(){return m_iWebSessionTimeout;};
		int	getWebMiniWebSessionTimeout() {return m_iMiniWebSessionTimeout;}

		QString getCommFileFullPath(QString sDataFile);
		
		MIhmLanguages * getLanguages(){return &m_oLanguages;};
		MIhmConfigImages * getWebConfigImages() {return &m_oWebImages;};
		MIhmConfigActions * getActions() {return &m_oConfigActions;};
		MIhmConfigDynamic * getCfgDynamic() {return &m_oConfigDynamic;};
		
		bool getGenericScriptParameters(QString sScriptVisibleObjectName,  MParamList *pLstParams);

		static MIhmConfigWeb * getCfg(){return m_pWebCfg;};
		
		QString htmlEncodeStr(QString str);

		QString getDefaultTimeFormat(){return m_sDefaultTimeFormat;};
		QString getDefaultDateFormat(){return m_sDefaultDateFormat;};

		bool getInputDlgsValidAlwaysEnabled() { return m_bInputDlgsValidAlwaysEnabled;};
		bool getShowCancelOnConnectingDlg(){return m_bShowCancelOnConnectingDlg;};

	private:
		QString getHtmlEncMapFile(){return m_sHtmlEncMapFile;};
		bool loadAvailableLaneTypesWeb(QString sLaneTypesCfgFilePath);

		QList <MIhmLaneTypeSettings *> m_lstLaneTypeSettings;

		QString m_sDefaultDateFormat;		
		QString m_sDefaultTimeFormat;
		
		QString m_sImagesCfgFilePath;
		MIhmConfigImages m_oWebImages;

		MIhmLanguages	m_oLanguages;

		QString				m_sDynamicColorsFile;
		MIhmConfigColor		m_oConfigColors;
		
		QString m_sActionsCfgFilePath;
		MIhmConfigActions	m_oConfigActions;
		
		QString m_sDynamicCfgFilePath;
		MIhmConfigDynamic	m_oConfigDynamic;
		
		static MIhmConfigWeb * m_pWebCfg;
		QString m_sModuleConfigKey;	
		
		QString m_sInitWebPage;
		QString m_sLoginFailPage;
		QString m_sLogoutPage;
		QString m_sDefaultLanguage;
		QString m_sLanguagesCfgFilePath;

		QString m_sHtmlEncMapFile;
		MHtmlCodec m_oHtmlCodec;


		int		m_iWebServerPort;  
		int		m_iWebSessionTimeout;
		int		m_iMiniWebSessionTimeout;

		QString	m_sCommFilesRoot;

		QString m_sLaneTypesCfgFile;
		bool m_bInputDlgsValidAlwaysEnabled;
		bool m_bShowCancelOnConnectingDlg;

};






#endif


