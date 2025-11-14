#ifndef MIhm_Lane_Type_Settings_H
#define MIhm_Lane_Type_Settings_H

#include <QString>
#include <QList>
#include <QDomNode>



#define CFG_LANE_TYPE_PARAM_WEB_MAIN_PAGE	"MAIN_PAGE"
#define CFG_LANE_TYPE_PARAM_WEB_MINI_WEB_PAGE	"MINI_WEB_PAGE"
#define CFG_LANE_TYPE_PARAM_WEB_HTMLTemplateRoot "HTMLTemplateRoot"


#define CFG_LANE_TYPE_PARAM_MAIN_DIALOG	"MAIN_DIALOG"
#define CFG_LANE_TYPE_PARAM_TSKBAR_BROWSER_DLG_TEMPLATE		"TASK_BAR_BROWSER_DLG_TEMPLATE"
#define CFG_LANE_TYPE_PARAM_TSKBAR_DLG_TYPES				"TASK_BAR_DLG_TYPES"
#define CFG_LANE_TYPE_PARAM_TSKBAR_DLG_GEOMETRY				"TASK_BAR_DLG_GEOMETRY_%1"

#define CFG_LANE_TYPE_PARAM_SEPARATOR				"|"


#define CFG_LANE_TYPE_PARAM_CLIENT_CSS_FILE				"CLIENT_CSS_FILE"
#define CFG_LANE_TYPE_PARAM_LOGIN_DLG_TEMPLATE			"CLIENT_LOGIN_DLG"
#define CFG_LANE_TYPE_PARAM_CLIENT_DLG_EMBEDED_TO		"CLIENT_DLG_EMBEDED_TO"
#define CFG_LANE_TYPE_PARAM_TAKE_OVER_DLG_TEMPLATE		"CLIENT_TAKEOVER_DLG"



#define XML_ELEMENT_IHM_CONFIG_ROOT			"IHM_CONFIG"
#define XML_ELEMENT_IHM_CONFIG_LANE_TYPES	"LANE_TYPES"
#define XML_ELEMENT_IHM_CONFIG_LANE_TYPE	"LANE_TYPE"
#define XML_ATTRIBUTE_ID					"ID"
 
#define CFG_LANE_TYPE_ELEMENT_WEB "WEB"
#define CFG_LANE_TYPE_ELEMENT_DESKTOP "DESKTOP"
#define CFG_LANE_TYPE_ATTRIBUTE_ASYNC_INPUT_FILE		"ASYNC_INPUT_DEF_FILE"

#define CFG_LANE_TYPE_ELEMENT_PARAM "PARAM"
#define CFG_VISIBLE_OBJ_ELEMENT				"VISIBLE_OBJ"
#define CFG_VISIBLE_OBJ_ELEMENT_SUB_TEMPLATE "SUB_TEMPLATE"
#define CFG_VISIBLE_OBJ_ELEMENT_PARAM		"PARAM"

#define CFG_VISIBLE_OBJ_ATTRIBUTE_ID "ID"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_VALUE "VALUE"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE "INIT_VISIBLE"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED "NOT_USED"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_USED_AT_MINI_WEB "USED_AT_MINI_WEB"

#define CFG_VISIBLE_OBJ_ATTRIBUTE_LEFT "LEFT"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_TOP "TOP"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_WIDTH "WIDTH"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_HEIGHT "HEIGHT"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID "ACTION_ID"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE "TEMPLATE"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_FORMAT_FILE "FORMAT_FILE"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS "MAX_ROWS"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_SET_FOCUS_KEY "SET_FOCUS_KEY"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_IS_ICON_MODE	"IS_ICON_MODE"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_TEXT	"SHOW_TEXT"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_SHOW_ICON	"SHOW_ICON"
#define CFG_VISIBLE_OBJ_ATTRIBUTE_TOOLTIP_SHOW_LAST_ITEM	"TOOLTIP_SHOW_LAST_ITEM"




struct LaneTypeParam
{
	QString param_name;
	QString param_value;
};


class LaneTypeVisObjParams
{
public:
	LaneTypeVisObjParams();
	~LaneTypeVisObjParams();

	QString getParam(QString sParamName);
	QString getSubTemplateContent(QString sParamName);

	QString m_sVisObjID;

	QList <LaneTypeParam*> m_lstParams;
	QList <LaneTypeParam*> m_lstSubTemplates;

};


class MIhmLaneTypeSettings
{
	
public:
	enum enumSettingsType
	{
		enuDESKTOP,
		enuWEB
	};

	MIhmLaneTypeSettings(enumSettingsType eType, QString sLaneType);
	~MIhmLaneTypeSettings();
	
	//bool loadSettings(QString sLaneTypesRegKey);
	bool loadXMLSettings(QDomNode nodeLaneType);


	QString getLaneType(){ return m_sLaneType;}
	enumSettingsType getSettingType(){ return m_eType;}

	QString getParam(QString sParamName);

		
	LaneTypeVisObjParams * getVisObjParams(QString sObjID);


	QString getVisObjParamValue(QString sObjID, QString sParamName);
	QString getVisObjHtmlTemplate(QString sObjID);
	QString getVisObjHtmlSubTemplate(QString sObjID, QString sParamName);


private:

	bool verifyIfAllWebParametersExist();
	bool verifyIfAllDskParametersExist();

	LaneTypeParam * newParam(QString sParamName, QString sValue);
	bool loadHtmlTemplates();

	enumSettingsType m_eType;
	QString m_sLaneType;
	
	QList <LaneTypeParam*> m_lstLaneTypeParams;
	QList <LaneTypeVisObjParams*> m_lstLaneTypeVisObjParams;
};


#endif


