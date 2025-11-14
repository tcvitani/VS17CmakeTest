

#include "MIhmWebVisibleObject.h"
#include "MIhmLaneTypeSettings.h"

#include <QList>
#include <QDateTime>
#include <QDir>
#include <QSettings>
#include <QDomDocument>
#include <QDomElement>
#include <QDomNodeList>
#include <QUrl>


#include "MIhmLanguages.h"
#include "MIhmConfigWeb.h"
#include "MHelpFuncs.h"
#include "MIhmAction.h"
#include "MTracer.h"
#include "MHMenuFileLoader.h"
#include "MIhmHttpRequest.h"
#include "MTableModelRow.h"
#include "MParamList.h"


#define HTML_CLASS_HIDDEN	" hidden"
#define HTML_CLASS_CLICKABLE " clickable"
#define HTML_CLASS_DISABLED " disabled"

#define HTML_TAB_CLASS_FIRST "first"
#define HTML_TAB_CLASS_ACTIVE "active"



#define HTML_CLASS_ALT_ROW "alt-row" //for altenate row colors of web table view

#define HTML_ATTRIBUTE_READONLY "readonly=\"readonly\""
#define HTML_ATTRIBUTE_DISABLED "disabled"
#define HTML_ATTRIBUTE_CHECKED "checked"


#define HTML_TEMPLATE_TEXT_LABEL_ID "%id%"
#define HTML_TEMPLATE_TEXT_LABEL_CLASS "%class%"
#define HTML_TEMPLATE_TEXT_LABEL_STYLE "%style%"
#define HTML_TEMPLATE_TEXT_LABEL_TEXT "%text%"

#define HTML_TEMPLATE_IMAGE_ID "%id%"
#define HTML_TEMPLATE_IMAGE_CLASS "%class%"
#define HTML_TEMPLATE_IMAGE_STYLE "%style%"
#define HTML_TEMPLATE_IMAGE_SRC "%images-root%/%image-name%"

//-----------------------------------------------------------------

// MIhmWebVisibleObject * MIhmWebVisibleObject::findVisibleObjectByName(QList <MIhmWebVisibleObject*> *plst, QString sName)
// {
// 	MIhmWebVisibleObject *pRetObj = NULL;
// 
// 	for (int i = 0; i < plst->size(); ++i) 
// 	{
// 		if(plst->at(i)->getName()==sName)
// 		{
// 			pRetObj = plst->at(i);
// 			break;
// 		}
// 	}
// 
// 	return pRetObj;	
// }

QString MIhmWebVisibleObject::getVisibleObjNameForID(enumWebVisibleObjectId eId)
{
	switch(eId)
	{
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS:				return LABEL_REG_VAL_WEB_CLASS;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS_TITLE:		return LABEL_REG_VAL_WEB_CLASS_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE:		return LABEL_REG_VAL_WEB_ADD_REVENUE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE_TITLE:	return LABEL_REG_VAL_WEB_ADD_REVENUE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE:				return LABEL_REG_VAL_WEB_FARE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TITLE:			return LABEL_REG_VAL_WEB_FARE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES:				return LABEL_REG_VAL_WEB_AXLES;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES_TITLE:		return LABEL_REG_VAL_WEB_AXLES_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE:				return LABEL_REG_VAL_WEB_TYPE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE_TITLE:			return LABEL_REG_VAL_WEB_TYPE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE:				return LABEL_REG_VAL_WEB_SALE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE_TITLE:			return LABEL_REG_VAL_WEB_SALE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE:			return LABEL_REG_VAL_WEB_SALEDUE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_TITLE:		return LABEL_REG_VAL_WEB_SALEDUE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_PAID:		return LABEL_REG_VAL_WEB_SALEDUE_PAID;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE:			return LABEL_REG_VAL_WEB_BALANCE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE_TITLE:		return LABEL_REG_VAL_WEB_BALANCE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY:				return LABEL_REG_VAL_WEB_ENTRY;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY_TITLE:		return LABEL_REG_VAL_WEB_ENTRY_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR:		return LABEL_REG_VAL_WEB_TRANSACTIONNBR;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR_TITLE:return LABEL_REG_VAL_WEB_TRANSACTIONNBR_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR:			return LABEL_REG_VAL_WEB_RECEIPTNBR;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR_TITLE:	return LABEL_REG_VAL_WEB_RECEIPTNBR_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_ID:				return LABEL_REG_VAL_WEB_COL_ID;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_NAME:			return LABEL_REG_VAL_WEB_COL_NAME;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME:			return LABEL_REG_VAL_WEB_PLAZA_NAME;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME1:			return LABEL_REG_VAL_WEB_PLAZA_NAME1;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME2:			return LABEL_REG_VAL_WEB_PLAZA_NAME2;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME3:			return LABEL_REG_VAL_WEB_PLAZA_NAME3;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME_TITLE:	return LABEL_REG_VAL_WEB_PLAZA_NAME_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME:			return LABEL_REG_VAL_WEB_LANE_NAME;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME1:			return LABEL_REG_VAL_WEB_LANE_NAME1;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME2:			return LABEL_REG_VAL_WEB_LANE_NAME2;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME_TITLE:	return LABEL_REG_VAL_WEB_LANE_NAME_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER_TITLE:	return LABEL_REG_VAL_WEB_PLAZA_NUMBER_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER:		return LABEL_REG_VAL_WEB_PLAZA_NUMBER;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER_TITLE:	return LABEL_REG_VAL_WEB_LANE_NUMBER_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER:		return LABEL_REG_VAL_WEB_LANE_NUMBER;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE1:return LABEL_REG_VAL_WEB_SPARE1;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE2:return LABEL_REG_VAL_WEB_SPARE2;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE3:return LABEL_REG_VAL_WEB_SPARE3;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE4:return LABEL_REG_VAL_WEB_SPARE4;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE5:return LABEL_REG_VAL_WEB_SPARE5;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE6:return LABEL_REG_VAL_WEB_SPARE6;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE7:return LABEL_REG_VAL_WEB_SPARE7;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE8:return LABEL_REG_VAL_WEB_SPARE8;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE9:return LABEL_REG_VAL_WEB_SPARE9;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE10:return LABEL_REG_VAL_WEB_SPARE10;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE11:return LABEL_REG_VAL_WEB_SPARE11;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE12:return LABEL_REG_VAL_WEB_SPARE12;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE13:return LABEL_REG_VAL_WEB_SPARE13;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE14:return LABEL_REG_VAL_WEB_SPARE14;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE15:return LABEL_REG_VAL_WEB_SPARE15;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE16:return LABEL_REG_VAL_WEB_SPARE16;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE17:return LABEL_REG_VAL_WEB_SPARE17;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE18:return LABEL_REG_VAL_WEB_SPARE18;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE19:return LABEL_REG_VAL_WEB_SPARE19;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE20:return LABEL_REG_VAL_WEB_SPARE20;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE21:return LABEL_REG_VAL_WEB_SPARE21;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE22:return LABEL_REG_VAL_WEB_SPARE22;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE23:return LABEL_REG_VAL_WEB_SPARE23;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE24:return LABEL_REG_VAL_WEB_SPARE24;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE25:return LABEL_REG_VAL_WEB_SPARE25;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE26:return LABEL_REG_VAL_WEB_SPARE26;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE27:return LABEL_REG_VAL_WEB_SPARE27;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE28:return LABEL_REG_VAL_WEB_SPARE28;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE29:return LABEL_REG_VAL_WEB_SPARE29;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE30:return LABEL_REG_VAL_WEB_SPARE30;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE31:return LABEL_REG_VAL_WEB_SPARE31;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE32:return LABEL_REG_VAL_WEB_SPARE32;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE33:return LABEL_REG_VAL_WEB_SPARE33;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE34:return LABEL_REG_VAL_WEB_SPARE34;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE35:return LABEL_REG_VAL_WEB_SPARE35;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE36:return LABEL_REG_VAL_WEB_SPARE36;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE37:return LABEL_REG_VAL_WEB_SPARE37;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE38:return LABEL_REG_VAL_WEB_SPARE38;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE39:return LABEL_REG_VAL_WEB_SPARE39;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE40:return LABEL_REG_VAL_WEB_SPARE40;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE41:return LABEL_REG_VAL_WEB_SPARE41;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE42:return LABEL_REG_VAL_WEB_SPARE42;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE43:return LABEL_REG_VAL_WEB_SPARE43;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE44:return LABEL_REG_VAL_WEB_SPARE44;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE45:return LABEL_REG_VAL_WEB_SPARE45;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE46:return LABEL_REG_VAL_WEB_SPARE46;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE47:return LABEL_REG_VAL_WEB_SPARE47;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE48:return LABEL_REG_VAL_WEB_SPARE48;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE49:return LABEL_REG_VAL_WEB_SPARE49;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE50:return LABEL_REG_VAL_WEB_SPARE50;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE51:return LABEL_REG_VAL_WEB_SPARE51;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE52:return LABEL_REG_VAL_WEB_SPARE52;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE53:return LABEL_REG_VAL_WEB_SPARE53;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE54:return LABEL_REG_VAL_WEB_SPARE54;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE55:return LABEL_REG_VAL_WEB_SPARE55;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE56:return LABEL_REG_VAL_WEB_SPARE56;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE57:return LABEL_REG_VAL_WEB_SPARE57;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE58:return LABEL_REG_VAL_WEB_SPARE58;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE59:return LABEL_REG_VAL_WEB_SPARE59;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE60:return LABEL_REG_VAL_WEB_SPARE60;



		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR:		return LABEL_REG_VAL_WEB_INSTR;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR_2:	return LABEL_REG_VAL_WEB_INSTR_2;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PAYMENT:	return LABEL_REG_VAL_WEB_PAYMENT;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ERR_PAYMENT:return LABEL_REG_VAL_WEB_ERR_PAYMENT;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_REMARK:		return LABEL_REG_VAL_WEB_REMARK;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_NBCAR:		return LABEL_REG_VAL_WEB_NBCAR;

		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE:				return LABEL_REG_VAL_WEB_FARE_TYPE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE_TITLE:		return LABEL_REG_VAL_WEB_FARE_TYPE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE_TITLE:		return LABEL_REG_VAL_WEB_TXT_MODE_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE:				return LABEL_REG_VAL_WEB_TXT_MODE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_COLLECTOR_TITLE:	return LABEL_REG_VAL_WEB_TXT_COLLECTOR_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER_TITLE:	return LABEL_REG_VAL_WEB_TXT_IN_CTRL_USER_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER:		return LABEL_REG_VAL_WEB_TXT_IN_CTRL_USER;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER1:		return LABEL_REG_VAL_WEB_TXT_IN_CTRL_USER1;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_CONTAINER_TITLE:return LABEL_REG_VAL_WEB_TXT_TRS_CONTAINER_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALARMS_TITLE:		return LABEL_REG_VAL_WEB_TXT_ALARMS_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_HISTORY_TITLE:	return LABEL_REG_VAL_WEB_TXT_TRS_HISTORY_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALERTES_TITLE:		return LABEL_REG_VAL_WEB_TXT_ALERTES_TITLE;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CAM_NUM_PIC:			return LABEL_REG_VAL_WEB_CAM_NUM_PIC;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SCAN_NUM_PIC:			return LABEL_REG_VAL_WEB_SCAN_NUM_PIC;
		case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_NB_CARS_SAS:		return LABEL_REG_VAL_WEB_TXT_NB_CARS_SAS;


		// Date is not updated by the model, but is updated on MClock signal
		case MIhmWebVisibleObject::enuCLK_WEB_DATE:return CLK_REG_VAL_WEB_DATE;
// 		//  !!! This one is updated by jscript on the web client side using default response content
// 		case MIhmWebVisibleObject::enuCLK_REG_VAL_WEB_TIME:return CLK_REG_VAL_WEB_TIME;

		// //------------------------------------
		// //Lists
		case MIhmWebVisibleObject::enuLIST_WEB_LIST_ALARMS:return LIST_REG_VAL_WEB_LIST_ALARMS;
		case MIhmWebVisibleObject::enuLIST_WEB_LIST_WARNINGS:return LIST_REG_VAL_WEB_LIST_WARNINGS;

		// //Icons
		case MIhmWebVisibleObject::enuICO_WEB_CAR:			return ICO_REG_VAL_WEB_CAR;
		case MIhmWebVisibleObject::enuICO_WEB_COLLECTOR:	return ICO_REG_VAL_WEB_COLLECTOR;
		case MIhmWebVisibleObject::enuICO_WEB_ENTRYGATE:	return ICO_REG_VAL_WEB_ENTRYGATE;
		case MIhmWebVisibleObject::enuICO_WEB_ENTRYLOOP:	return ICO_REG_VAL_WEB_ENTRYLOOP;
		case MIhmWebVisibleObject::enuICO_WEB_EXITGATE:		return ICO_REG_VAL_WEB_EXITGATE;
		case MIhmWebVisibleObject::enuICO_WEB_EXITLOOP:		return ICO_REG_VAL_WEB_EXITLOOP;
		case MIhmWebVisibleObject::enuICO_WEB_MODE:			return ICO_REG_VAL_WEB_MODE;
		case MIhmWebVisibleObject::enuICO_WEB_STATUS:		return ICO_REG_VAL_WEB_STATUS;
		case MIhmWebVisibleObject::enuICO_WEB_TRAFFIC:		return ICO_REG_VAL_WEB_TRAFFIC;
		case MIhmWebVisibleObject::enuICO_WEB_ENT_OPTICAL_BARIER:return ICO_REG_VAL_WEB_ENT_OPTICAL_BARIER;
		case MIhmWebVisibleObject::enuICO_WEB_VIOLATION:	return ICO_REG_VAL_WEB_VIOLATION;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE1:		return ICO_REG_VAL_WEB_SPARE1;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE2:		return ICO_REG_VAL_WEB_SPARE2;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE3:		return ICO_REG_VAL_WEB_SPARE3;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE4:		return ICO_REG_VAL_WEB_SPARE4;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE5:		return ICO_REG_VAL_WEB_SPARE5;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE6:		return ICO_REG_VAL_WEB_SPARE6;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE7:		return ICO_REG_VAL_WEB_SPARE7;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE8:		return ICO_REG_VAL_WEB_SPARE8;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE9:		return ICO_REG_VAL_WEB_SPARE9;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE10:		return ICO_REG_VAL_WEB_SPARE10;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE11:		return ICO_REG_VAL_WEB_SPARE11;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE12:		return ICO_REG_VAL_WEB_SPARE12;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE13:		return ICO_REG_VAL_WEB_SPARE13;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE14:		return ICO_REG_VAL_WEB_SPARE14;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE15:		return ICO_REG_VAL_WEB_SPARE15;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE16:		return ICO_REG_VAL_WEB_SPARE16;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE17:		return ICO_REG_VAL_WEB_SPARE17;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE18:		return ICO_REG_VAL_WEB_SPARE18;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE19:		return ICO_REG_VAL_WEB_SPARE19;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE20:		return ICO_REG_VAL_WEB_SPARE20;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE21:		return ICO_REG_VAL_WEB_SPARE21;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE22:		return ICO_REG_VAL_WEB_SPARE22;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE23:		return ICO_REG_VAL_WEB_SPARE23;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE24:		return ICO_REG_VAL_WEB_SPARE24;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE25:		return ICO_REG_VAL_WEB_SPARE25;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE26:		return ICO_REG_VAL_WEB_SPARE26;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE27:		return ICO_REG_VAL_WEB_SPARE27;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE28:		return ICO_REG_VAL_WEB_SPARE28;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE29:		return ICO_REG_VAL_WEB_SPARE29;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE30:		return ICO_REG_VAL_WEB_SPARE30;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE31:		return ICO_REG_VAL_WEB_SPARE31;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE32:		return ICO_REG_VAL_WEB_SPARE32;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE33:		return ICO_REG_VAL_WEB_SPARE33;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE34:		return ICO_REG_VAL_WEB_SPARE34;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE35:		return ICO_REG_VAL_WEB_SPARE35;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE36:		return ICO_REG_VAL_WEB_SPARE36;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE37:		return ICO_REG_VAL_WEB_SPARE37;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE38:		return ICO_REG_VAL_WEB_SPARE38;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE39:		return ICO_REG_VAL_WEB_SPARE39;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE40:		return ICO_REG_VAL_WEB_SPARE40;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE41:		return ICO_REG_VAL_WEB_SPARE41;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE42:		return ICO_REG_VAL_WEB_SPARE42;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE43:		return ICO_REG_VAL_WEB_SPARE43;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE44:		return ICO_REG_VAL_WEB_SPARE44;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE45:		return ICO_REG_VAL_WEB_SPARE45;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE46:		return ICO_REG_VAL_WEB_SPARE46;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE47:		return ICO_REG_VAL_WEB_SPARE47;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE48:		return ICO_REG_VAL_WEB_SPARE48;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE49:		return ICO_REG_VAL_WEB_SPARE49;
		case MIhmWebVisibleObject::enuICO_WEB_SPARE50:		return ICO_REG_VAL_WEB_SPARE50;

		case MIhmWebVisibleObject::enuICO_WEB_CAMERA1:		return ICO_REG_VAL_WEB_CAMERA1;
		case MIhmWebVisibleObject::enuICO_WEB_CAMERA2:		return ICO_REG_VAL_WEB_CAMERA2;
		case MIhmWebVisibleObject::enuICO_WEB_BEACON:		return ICO_REG_VAL_WEB_BEACON;
		case MIhmWebVisibleObject::enuICO_WEB_BEACON2:		return ICO_REG_VAL_WEB_BEACON2;
		case MIhmWebVisibleObject::enuICO_WEB_SIGNAL_LIGHT:return ICO_REG_VAL_WEB_SIGNAL_LIGHT;
		case MIhmWebVisibleObject::enuICO_WEB_EXT_OPT_BARR:return ICO_REG_VAL_WEB_EXT_OPT_BARR;
		case MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE:	return CNT_REG_VAL_WEB_DYN_IMAGE;
		case MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE2:	return CNT_REG_VAL_WEB_DYN_IMAGE2;

		//------------------------------------
		//Containers
		case MIhmWebVisibleObject::enuCNT_WEB_MAIN_DIALOG:	return CNT_REG_VAL_WEB_MAIN_DIALOG;
		case MIhmWebVisibleObject::enuCNT_WEB_PAYMENT_GROUP:return CNT_REG_VAL_WEB_PAYMENT_GROUP;
		case MIhmWebVisibleObject::enuCNT_WEB_INSTR_GROUP:return CNT_REG_VAL_WEB_INSTR_GROUP;
		case MIhmWebVisibleObject::enuCNT_WEB_TRS_GROUP:return CNT_REG_VAL_WEB_TRS_GROUP;
		case MIhmWebVisibleObject::enuCNT_WEB_MODE_GROUP:return CNT_REG_VAL_WEB_MODE_GROUP;
		case MIhmWebVisibleObject::enuCNT_WEB_HEADER_GROUP:return CNT_REG_VAL_WEB_HEADER_GROUP;
		case MIhmWebVisibleObject::enuCNT_WEB_COMMENTS:return CNT_REG_VAL_WEB_COMMENTS;
		case MIhmWebVisibleObject::enuCNT_WEB_CAMERA:return CNT_REG_VAL_WEB_CAMERA;
		case MIhmWebVisibleObject::enuCNT_WEB_SCANNER:return CNT_REG_VAL_WEB_SCANNER;

		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE1:return CNT_REG_VAL_WEB_CNT_SPARE1;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE2:return CNT_REG_VAL_WEB_CNT_SPARE2;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE3:return CNT_REG_VAL_WEB_CNT_SPARE3;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE4:return CNT_REG_VAL_WEB_CNT_SPARE4;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE5:return CNT_REG_VAL_WEB_CNT_SPARE5;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE6:return CNT_REG_VAL_WEB_CNT_SPARE6;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE7:return CNT_REG_VAL_WEB_CNT_SPARE7;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE8:return CNT_REG_VAL_WEB_CNT_SPARE8;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE9:return CNT_REG_VAL_WEB_CNT_SPARE9;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE10:return CNT_REG_VAL_WEB_CNT_SPARE10;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE11:return CNT_REG_VAL_WEB_CNT_SPARE11;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE12:return CNT_REG_VAL_WEB_CNT_SPARE12;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE13:return CNT_REG_VAL_WEB_CNT_SPARE13;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE14:return CNT_REG_VAL_WEB_CNT_SPARE14;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE15:return CNT_REG_VAL_WEB_CNT_SPARE15;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE16:return CNT_REG_VAL_WEB_CNT_SPARE16;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE17:return CNT_REG_VAL_WEB_CNT_SPARE17;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE18:return CNT_REG_VAL_WEB_CNT_SPARE18;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE19:return CNT_REG_VAL_WEB_CNT_SPARE19;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE20:return CNT_REG_VAL_WEB_CNT_SPARE20;

		case MIhmWebVisibleObject::enuCNT_WEB_CNT_MINI_WEB:return CNT_REG_VAL_WEB_CNT_MINI_WEB;
		case MIhmWebVisibleObject::enuCNT_WEB_CNT_ALARMS_GROUP:return CNT_REG_VAL_WEB_CNT_ALARMS_GROUP;
			
		//------------------------------------------------------------
		// Buttons
		//------------------------------------------------------------
		
		case MIhmWebVisibleObject::enuBTN_WEB_CLASS:return BTN_VAL_WEB_CLASS;
		case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT:return BTN_VAL_WEB_ENTRY_POINT;
		case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT_NBR:return BTN_VAL_WEB_ENTRY_POINT_NBR;
		case MIhmWebVisibleObject::enuBTN_WEB_MODE:return BTN_VAL_WEB_MODE;
		case MIhmWebVisibleObject::enuBTN_WEB_ARROW_ON:return BTN_VAL_WEB_ARROW_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_ARROW_OFF:return BTN_VAL_WEB_ARROW_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_CROSS_ON:return BTN_VAL_WEB_CROSS_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_CROSS_OFF:return BTN_VAL_WEB_CROSS_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_DSRC_ON:return BTN_VAL_WEB_DSRC_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_DSRC_OFF:return BTN_VAL_WEB_DSRC_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_MAGNET_ON:return BTN_VAL_WEB_MAGNET_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_MAGNET_OFF:return BTN_VAL_WEB_MAGNET_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_ON:return BTN_VAL_WEB_COLLECTOR_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_OFF:return BTN_VAL_WEB_COLLECTOR_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_CAMERA:return BTN_VAL_WEB_CAMERA;
		case MIhmWebVisibleObject::enuBTN_WEB_SCANNER:return BTN_VAL_WEB_SCANNER;
		case MIhmWebVisibleObject::enuBTN_WEB_COMMENT:return BTN_VAL_WEB_COMMENT;
		case MIhmWebVisibleObject::enuBTN_WEB_GABARITE:return BTN_VAL_WEB_GABARITE;
		case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_GATE:return BTN_VAL_WEB_ENTRY_GATE;
		case MIhmWebVisibleObject::enuBTN_WEB_EXIT_GATE:return BTN_VAL_WEB_EXIT_GATE;
		case MIhmWebVisibleObject::enuBTN_WEB_CAR3:return BTN_VAL_WEB_CAR3;
		case MIhmWebVisibleObject::enuBTN_WEB_CAR2:return BTN_VAL_WEB_CAR2;
		case MIhmWebVisibleObject::enuBTN_WEB_CAR:return BTN_VAL_WEB_CAR;
		case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_LOOP:return BTN_VAL_WEB_ENTRY_LOOP;
		case MIhmWebVisibleObject::enuBTN_WEB_EXIT_LOOP:return BTN_VAL_WEB_EXIT_LOOP;
		case MIhmWebVisibleObject::enuBTN_WEB_TBL_BEACON:return BTN_VAL_WEB_TBL_BEACON;
		case MIhmWebVisibleObject::enuBTN_WEB_CAM_GRAB:return BTN_VAL_WEB_CAM_GRAB;
		case MIhmWebVisibleObject::enuBTN_WEB_CAM_HIDE:return BTN_VAL_WEB_CAM_HIDE;
		case MIhmWebVisibleObject::enuBTN_WEB_SCAN_GRAB:return BTN_VAL_WEB_SCAN_GRAB;
		case MIhmWebVisibleObject::enuBTN_WEB_SCAN_HIDE:return BTN_VAL_WEB_SCAN_HIDE;
		case MIhmWebVisibleObject::enuBTN_WEB_SCAN_SEND:return BTN_VAL_WEB_SCAN_SEND;
		case MIhmWebVisibleObject::enuBTN_WEB_SCAN_SAVE:return BTN_VAL_WEB_SCAN_SAVE;
		case MIhmWebVisibleObject::enuBTN_WEB_COMM_HIDE:return BTN_VAL_WEB_COMM_HIDE;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY:return BTN_VAL_WEB_BTN_EXTERNAL_DISPLAY;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY2:return BTN_VAL_WEB_BTN_EXTERNAL_DISPLAY2;
		case MIhmWebVisibleObject::enuBTN_WEB_BUTTON1:return BTN_VAL_WEB_BUTTON1;
		case MIhmWebVisibleObject::enuBTN_WEB_BUTTON2:return BTN_VAL_WEB_BUTTON2;
		case MIhmWebVisibleObject::enuBTN_WEB_BUTTON3:return BTN_VAL_WEB_BUTTON3;
		case MIhmWebVisibleObject::enuBTN_WEB_BUTTON4:return BTN_VAL_WEB_BUTTON4;
		case MIhmWebVisibleObject::enuBTN_WEB_BUTTON5:return BTN_VAL_WEB_BUTTON5;
		case MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_ON:return BTN_VAL_WEB_SPEED_LIMIT_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_OFF:return BTN_VAL_WEB_SPEED_LIMIT_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_WARNING_ON:return BTN_VAL_WEB_WARNING_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_WARNING_OFF:return BTN_VAL_WEB_WARNING_OFF;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_ON:return BTN_VAL_WEB_BTN_DSRC_TSA_ON;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_OFF:return BTN_VAL_WEB_BTN_DSRC_TSA_OFF;

		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_1:return BTN_VAL_WEB_BTN_SPARE_1;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_2:return BTN_VAL_WEB_BTN_SPARE_2;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_3:return BTN_VAL_WEB_BTN_SPARE_3;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_4:return BTN_VAL_WEB_BTN_SPARE_4;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_5:return BTN_VAL_WEB_BTN_SPARE_5;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_6:return BTN_VAL_WEB_BTN_SPARE_6;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_7:return BTN_VAL_WEB_BTN_SPARE_7;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_8:return BTN_VAL_WEB_BTN_SPARE_8;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_9:return BTN_VAL_WEB_BTN_SPARE_9;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_10:return BTN_VAL_WEB_BTN_SPARE_10;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_11:return BTN_VAL_WEB_BTN_SPARE_11;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_12:return BTN_VAL_WEB_BTN_SPARE_12;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_13:return BTN_VAL_WEB_BTN_SPARE_13;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_14:return BTN_VAL_WEB_BTN_SPARE_14;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_15:return BTN_VAL_WEB_BTN_SPARE_15;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_16:return BTN_VAL_WEB_BTN_SPARE_16;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_17:return BTN_VAL_WEB_BTN_SPARE_17;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_18:return BTN_VAL_WEB_BTN_SPARE_18;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_19:return BTN_VAL_WEB_BTN_SPARE_19;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_20:return BTN_VAL_WEB_BTN_SPARE_20;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_21:return BTN_VAL_WEB_BTN_SPARE_21;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_22:return BTN_VAL_WEB_BTN_SPARE_22;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_23:return BTN_VAL_WEB_BTN_SPARE_23;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_24:return BTN_VAL_WEB_BTN_SPARE_24;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_25:return BTN_VAL_WEB_BTN_SPARE_25;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_26:return BTN_VAL_WEB_BTN_SPARE_26;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_27:return BTN_VAL_WEB_BTN_SPARE_27;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_28:return BTN_VAL_WEB_BTN_SPARE_28;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_29:return BTN_VAL_WEB_BTN_SPARE_29;
		case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_30:return BTN_VAL_WEB_BTN_SPARE_30;

		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuDLG_VAL_WEB_ASYNC_INPUT:return DLG_VAL_WEB_ASYNC_INPUT;
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW1:return CNT_REG_VAL_WEB_FILE_VIEW1;
		case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW2:return CNT_REG_VAL_WEB_FILE_VIEW2;
		case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW3:return CNT_REG_VAL_WEB_FILE_VIEW3;
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW1:return CNT_REG_VAL_WEB_HMENU_VIEW1;
		case MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW2:return CNT_REG_VAL_WEB_HMENU_VIEW2;
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW1:return CNT_REG_VAL_WEB_TAB_VIEW1;
		case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW2:return CNT_REG_VAL_WEB_TAB_VIEW2;
		case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW3:return CNT_REG_VAL_WEB_TAB_VIEW3;
		case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW4:return CNT_REG_VAL_WEB_TAB_VIEW4;

		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW1:return CNT_REG_VAL_WEB_RVIDEO_VIEW1;
		case MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW2:return CNT_REG_VAL_WEB_RVIDEO_VIEW2;

		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCMD_WEB_CLIENT_DIRECT_VIDEO:return CMD_REG_VAL_WEB_CLIENT_DIRECT_VIDEO;	
		
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCMD_WEB_WINDOW_TITLE:return CMD_REG_VAL_WEB_WINDOW_TITLE;	
		//------------------------------------------------------------			
		//------------------------------------------------------------
		//to be able to update captions and enable/disable on these specific buttons
		case MIhmWebVisibleObject::enuBTN_WEB_TAKE_CTRL:return BTN_VAL_WEB_TAKE_CTRL;	

		case MIhmWebVisibleObject::enuBTN_WEB_RETURN_CTRL:return BTN_VAL_WEB_RETURN_CTRL;	
		//------------------------------------------------------------
		//--------- Modal input dialog -------------------------------
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG:return DLG_REG_VAL_WEB_INPUT_DIALOG;	
		case MIhmWebVisibleObject::enuDLG_WEB_INPUT_DIALOG_EX:return DLG_REG_VAL_WEB_INPUT_DIALOG_EX;
		case MIhmWebVisibleObject::enuDLG_VAL_WEB_HPRIO_INPUT_DIALOG:return DLG_VAL_WEB_HPRIO_INPUT_DIALOG;	
		//------------------------------------------------------------
		//----- For specific take over mode handling -----------------
		//------------------------------------------------------------
		//a command to the client to update the look of the form when  in control status change
		case MIhmWebVisibleObject::enuCMD_WEB_IN_CONTROL_MODE:return CMD_REG_VAL_WEB_IN_CONTROL_MODE;
		//------------------------------------------------------------
		//----- For specific OpenLink command handling -----------------
		//------------------------------------------------------------
		case MIhmWebVisibleObject::enuCMD_WEB_OPEN_LINK:return CMD_REG_VAL_WEB_OPEN_LINK;	
		//------------------------------------------------------------			
		//----- For specific OpenLink command handling -----------------
	default:
		TRACE_WEB_W(QString("MIhmWebVisibleObject::getVisibleObjNameForID: Error unknown visible object ID:%1").arg((int)eId));
		Q_ASSERT(false);
	    return QString("");
	}

}

MIhmWebVisibleObject::enumWebVisibleObjectId MIhmWebVisibleObject::getVisibleObjIDForName(const QString &sName)
{
	enumWebVisibleObjectId eRetVal = enuWebVisibleObjectUnknown;
		
	for (int i = enuWebVisibleObjectUnknown; i < enuWebVisibleObjectLast; i++)
	{
		if(getVisibleObjNameForID((enumWebVisibleObjectId)i)==sName)
			return (enumWebVisibleObjectId)i;
	}

	return eRetVal;
}

MIhmWebVisibleObject * MIhmWebVisibleObject::findVisibleObjectByID(QVector <MIhmWebVisibleObject*> *pObjVector, const enumWebVisibleObjectId eId)
{
	//do the check 
	if(pObjVector->at((int)eId)!=NULL)
		if(pObjVector->at((int)eId)->getId() != eId)	
		{
			TRACE_WEB_W(QString("MIhmWebVisibleObject::findVisibleObjectByID: Error invalid visible object ID:%1").arg((int)eId));
			ExitBad();
		}

	return pObjVector->at((int)eId);
}


MIhmWebVisibleObject::MIhmWebVisibleObject(enumWebVisibleObjectId eId): 
			m_eId(eId)
{
	m_sName = getVisibleObjNameForID(eId);
	m_sUpdTimeStamp = "";
	m_sAckTimeStamp = "0";
	m_bHtmlTemplAvailable = false;
	m_bReady = false;
	m_bReadOnly = true;
	m_bVisible = false;
	m_bNotUsed = false;
	m_bUsedAtMiniWeb = false;
	m_bHtmlPrepared = false;
	m_bReadOnlyHtmlPrepared = false;;

	m_iInternalItemIDCounter = 0;
	m_iTimestampCounter = 0; //used to avoid possibility that something was updated and acknowledged in the same millisecond
}



MIhmWebVisibleObject::~MIhmWebVisibleObject()
{
	resetCtrlIdActionMappping();
}

void MIhmWebVisibleObject::setStyle(QString sStyle)
{
	m_sStyle = sStyle;
	generateTimestamp();
}

void MIhmWebVisibleObject::setVisible(bool bVisible)
{
	m_bVisible = bVisible;
	generateTimestamp();
}

bool MIhmWebVisibleObject::isVisible()
{
	return m_bVisible;
}


void MIhmWebVisibleObject::setReadOnly(bool bReadOnly)
{
	generateTimestamp();
	m_bReadOnly = bReadOnly;
}


QString	MIhmWebVisibleObject::generateNewItemID()
{
	++m_iInternalItemIDCounter;
	QString sNewID = m_sName + m_sUpdTimeStamp + QString("_%1").arg(m_iInternalItemIDCounter);

	return sNewID;
}


bool MIhmWebVisibleObject::isUpdated()
{
	if(m_sUpdTimeStamp==m_sAckTimeStamp) 
		return false;
	else
		return true;
}

void MIhmWebVisibleObject::setBeingUpdated()
{
	m_sAckTimeStamp = m_sUpdTimeStamp;
}


void MIhmWebVisibleObject::setAckTimeStamp(QString sTS)
{
	m_sAckTimeStamp = sTS;
} 


void MIhmWebVisibleObject::resetHtmlPreparedFlags()
{
	m_bHtmlPrepared = false;
	m_bReadOnlyHtmlPrepared = false;
}


QString MIhmWebVisibleObject::getStyle(bool bReadOnly)
{
	return m_sStyle;
}




void MIhmWebVisibleObject::generateTimestamp()
{
	m_iInternalItemIDCounter = 0;
	m_iTimestampCounter++;
	if(m_iTimestampCounter>9)
			m_iTimestampCounter = 0;

	m_sUpdTimeStamp = QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz");
	m_sUpdTimeStamp.append(QChar(m_iTimestampCounter + 48));
	resetHtmlPreparedFlags();

}


QString MIhmWebVisibleObject::loadTemplateFileContent(QString sFilePath)
{
	QString sRes;
	QFile file(sFilePath);

	if (file.open(QIODevice::ReadOnly | QIODevice::Text))
	{
		QByteArray baFileContent;

		while (!file.atEnd()) {
			baFileContent += file.readLine();
		 }

		sRes = baFileContent;
	}
	else
	{
		sRes ="";
		TRACE_WEB_W(QString("MIhmWebVisibleObject::loadTemplateFileContent:ERROR Unable to open file:%1 for object:%2").
					arg(sFilePath.toLatin1().data()).
					arg(m_sName.toLatin1().data()));
	}

	return sRes;
}


bool MIhmWebVisibleObject::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	return false;
}

bool MIhmWebVisibleObject::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	bool bOK = false;

	if(sCtrlId==m_sName)
	{
		sActionID = m_sActionId;
		sParams = "";
		bOK = !m_sActionId.isEmpty();
	}

	return bOK;
}

void MIhmWebVisibleObject::setSingleAction(QString sActionID)
{
	m_sActionId = sActionID;
}



void MIhmWebVisibleObject::resetCtrlIdActionMappping()
{
	//delete the list content
	 while (!m_lstCtrlIdActionMapping.isEmpty())
		delete m_lstCtrlIdActionMapping.takeFirst();
}

void MIhmWebVisibleObject::addCtrlIdActionMapping(QString sCtrlId, QString sActionID, QString sParams)
{
	MCtrlIdAction* pNew = new MCtrlIdAction();

	pNew->sCtrlId = sCtrlId;
	pNew->sAction = sActionID;
	pNew->sParams = sParams;

	m_lstCtrlIdActionMapping.append(pNew);
}


MCtrlIdAction* MIhmWebVisibleObject::findActionForCtrlId(QString sCtrlId)
{
	MCtrlIdAction* pCurrent;

	for(int i=0; i<m_lstCtrlIdActionMapping.size();i++)
	{
		pCurrent = m_lstCtrlIdActionMapping.at(i);

		if(pCurrent->sCtrlId == sCtrlId)
		{
			return pCurrent;
		}
	}

	return NULL;
}


//Translate and encode for web
QString MIhmWebVisibleObject::translateAndEncodeForWeb(QString sText)
{
	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
	pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetNoTransform, sText);
	sText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sText);
	sText = MHelpFuncs::replaceNewLinesForWeb(sText);

	return sText;
}



QString MIhmWebVisibleObject::updateInputExFileds(QList <MInputExField *> * plstFieldList, QString sTemplateHtml)
{
	QString sItemsList;
	QString sTextToChange,sReadOnlyTextToChange;
	QString sText, sReadOnly, sValue, sSecondaryValidator;
	QString sParamToChange, sParam;
	bool bOKButtonIsReadOnly = true;
	int iTemp;

	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();

	QString sRes = sTemplateHtml;

	MInputExField * pCurrentField;

	for (int i=0; i<plstFieldList->size();i++)
	{
		pCurrentField = plstFieldList->at(i);

		switch(pCurrentField->getType())
		{
		case MInputExField::enuDLG_EX_FIELD_CHECK_BOX:
		case MInputExField::enuDLG_EX_FIELD_RADIO_BUTTON:
			sTextToChange = QString("%%1-text%").arg(pCurrentField->getName());
			sText = pCurrentField->getText();
			sText = translateAndEncodeForWeb(sText);
			sRes.replace(sTextToChange, sText, Qt::CaseInsensitive);
			
			sTextToChange = QString("%%1-value%").arg(pCurrentField->getName());
			sValue = pCurrentField->getValue();
			sRes.replace(sTextToChange, sValue, Qt::CaseInsensitive);

			sTextToChange = QString("%%1-checked%").arg(pCurrentField->getName());
			if(QString::compare("true",pCurrentField->getValue(), Qt::CaseInsensitive)==0 || 
				QString::compare("1",pCurrentField->getValue(), Qt::CaseInsensitive)==0)
				sValue = HTML_ATTRIBUTE_CHECKED;
			else
				sValue = "";

			sRes.replace(sTextToChange, sValue, Qt::CaseInsensitive);

			sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
			sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_DISABLED:"";
			sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);
			
			break;

		case MInputExField::enuDLG_EX_FIELD_COMBO_BOX:
		case MInputExField::enuDLG_EX_FIELD_LIST_WIDGET:
			{
				QList <MOptionData*> *pList = pCurrentField->getOptions();

				QString sCurrentText, sCurrentValue, sCurrentRow, sCurrentSelected;
				
				QString sListRowTemplate = "<option value=\"%value%\" %selected%>%text%</option>";//ListRow
				
				sText = "";

				bool bCurrentValueExist = false;

				for (int i=0;i<pList->size();i++)
				{	
					if(pList->at(i)->m_sValue == pCurrentField->getValue() && pList->at(i)->m_sValue!="")
					{
						bCurrentValueExist = true;
						break;
					}
				}

				bool bSelectFirst = false;

				//if no value set for the list input or the value set does not exist in the list
				// select the first item 
				if(pCurrentField->getValue() =="" || !bCurrentValueExist)
					bSelectFirst = true;
				
				
				for (int i=0;i<pList->size();i++)
				{
					sCurrentText = pList->at(i)->m_sText;
					sCurrentText = translateAndEncodeForWeb(sCurrentText);
					sCurrentValue = pList->at(i)->m_sValue;
					sCurrentValue = translateAndEncodeForWeb(sCurrentValue);
					
					if(bSelectFirst && i==0)
						sCurrentSelected = "selected";
					else 
					{
						if(sCurrentValue == pCurrentField->getValue())
							sCurrentSelected = "selected";		
						else
							sCurrentSelected = "";
					}

					sCurrentRow = sListRowTemplate;

					sCurrentRow.replace("%value%", sCurrentValue, Qt::CaseInsensitive);
					sCurrentRow.replace("%text%", sCurrentText, Qt::CaseInsensitive);
					sCurrentRow.replace("%selected%", sCurrentSelected, Qt::CaseInsensitive);
					//add item
					sText += sCurrentRow;

				}

				sTextToChange = QString("%%1-data%").arg(pCurrentField->getName());
				sRes.replace(sTextToChange, sText, Qt::CaseInsensitive);

				sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
				sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_DISABLED:"";
				sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);
			}
			break;

			
		case MInputExField::enuDLG_EX_FIELD_LINE_EDIT_EXT:
		case MInputExField::enuDLG_EX_FIELD_LINE_EDIT:
		case MInputExField::enuDLG_EX_FIELD_HIDDEN_EDIT:
			//line edit text update
			sTextToChange = QString("%%1-text%").arg(pCurrentField->getName());
			sText = pCurrentField->getValue();
			//Note that the value content is not to be translated
			sText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sText);
			sRes.replace(sTextToChange, sText, Qt::CaseInsensitive);
			
			//secondary validator update %name-regex%
			sTextToChange = QString("%%1-regex%").arg(pCurrentField->getName());
			sSecondaryValidator = MHelpFuncs::escapeRegex(pCurrentField->getSecondaryValidator());
			sSecondaryValidator = MIhmConfigWeb::getCfg()->htmlEncodeStr(sSecondaryValidator);
			sRes.replace(sTextToChange, sSecondaryValidator, Qt::CaseInsensitive);
			

		
			
			
			//------------------------------------------------------------------------
			//numeric input settings
			//------------------------------------------------------------------------
			if(pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_NUMBER)
			{
				// preDecimals
				sParamToChange = QString("%%1-preDecimals%").arg(pCurrentField->getName());
				sParam = QString("%1").arg(pCurrentField->getNumericPreDecimals());					
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// decimals
				sParamToChange = QString("%%1-decimals%").arg(pCurrentField->getName());
				sParam = QString("%1").arg(pCurrentField->getNumericDecimals());					
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// allowNegative
				sParamToChange = QString("%%1-allowNegative%").arg(pCurrentField->getName());
				sParam = (pCurrentField->getNumericAllowNegative())?"true":"false";
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// separatorChar
				sParamToChange = QString("%%1-separatorChar%").arg(pCurrentField->getName());
				sParam = QString("%1").arg(pCurrentField->getNumericSeparatorChar());				
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
			}
			//------------------------------------------------------------------------
			
			if(pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY ||
				pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16 ||
				pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_LUHN_KEY_MOD16_M3 ||
				pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_TEXT)
			{
				// minlen
				sParamToChange = QString("%%1-minlen%").arg(pCurrentField->getName());
				iTemp = pCurrentField->getMinLen();
				sParam = (i>=0)?QString("%1").arg(iTemp):"";							
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// maxlen
				sParamToChange = QString("%%1-maxlen%").arg(pCurrentField->getName());
				iTemp = pCurrentField->getMaxLen();
				sParam = (i>=0)?QString("%1").arg(iTemp):"";							
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				//------------------------------------------------------------------------
			}	
			
			//------------------------------------------------------------------------
			//masked input settings
			//------------------------------------------------------------------------
			if(pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_MASK ||
				pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_RIGHTMASK)
			{
				// mask
				sParamToChange = QString("%%1-mask%").arg(pCurrentField->getName());
				sParam = pCurrentField->getInputMask();					
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// mask-placeholder
				sParamToChange = QString("%%1-mask-placeholder%").arg(pCurrentField->getName());
				sParam = pCurrentField->getMaskPlaceholder();						
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// backward-input
				sParamToChange = QString("%%1-backward-input%").arg(pCurrentField->getName());
				sParam = (pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_RIGHTMASK)?"true":"false";
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				//------------------------------------------------------------------------
			}

			//------------------------------------------------------------------------
			//formula input settings
			//------------------------------------------------------------------------
			if(pCurrentField->getFormatType() == MInputExField::enuFIELD_FORMAT_CALCULATION)
			{
				// formula
				sParamToChange = QString("%%1-formula%").arg(pCurrentField->getName());
				sParam = pCurrentField->getFormatDetail();
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
			
				// decimals
				sParamToChange = QString("%%1-resultPrecision%").arg(pCurrentField->getName());
				sParam = QString("%1").arg(pCurrentField->getResultPrecision());												
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// enable label switch-logic
				sParamToChange = QString("%%1-useSwitchLogic%").arg(pCurrentField->getName());
				sParam = (pCurrentField->isLblSwitchingEnabled())?"true":"false";										
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);

				// switch-logic formula
				sParamToChange = QString("%%1-switchFormula%").arg(pCurrentField->getName());
				sParam = pCurrentField->getLbLSwitchingFormula();	
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// message container that will be enabled when the switch formula result is true
				sParamToChange = QString("%%1-trueMessageContainer%").arg(pCurrentField->getName());
				sParam = pCurrentField->getLbLSwitchingLabelTrue();	
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				
				// message container that will be enabled when the switch formula result is false
				sParamToChange = QString("%%1-falseMessageContainer%").arg(pCurrentField->getName());
				sParam = pCurrentField->getLbLSwitchingLabelFalse();				
				sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
				//------------------------------------------------------------------------

				//control read-only status update
				sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
				sReadOnly = HTML_ATTRIBUTE_READONLY;
				//sReadOnly = HTML_ATTRIBUTE_DISABLED;
				sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);
			
			}
			else
			{
				//control read-only status update
				sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
				sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_READONLY:"";
				//sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_DISABLED:"";
				sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);

				// formula
				sParamToChange = QString("%%1-formula%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "", Qt::CaseInsensitive);
			
				// decimals
				sParamToChange = QString("%%1-resultPrecision%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "0", Qt::CaseInsensitive);
				
				// enable label switch-logic
				sParamToChange = QString("%%1-useSwitchLogic%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "false", Qt::CaseInsensitive);

				// switch-logic formula
				sParamToChange = QString("%%1-switchFormula%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "", Qt::CaseInsensitive);
				
				// message container that will be enabled when the switch formula result is true
				sParamToChange = QString("%%1-trueMessageContainer%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "", Qt::CaseInsensitive);
				
				// message container that will be enabled when the switch formula result is false
				sParamToChange = QString("%%1-falseMessageContainer%").arg(pCurrentField->getName());
				sRes.replace(sParamToChange, "", Qt::CaseInsensitive);
				//------------------------------------------------------------------------
			}

			break;
			

		case MInputExField::enuDLG_EX_FIELD_TEXT_EDIT:
			sTextToChange = QString("%%1-text%").arg(pCurrentField->getName());
			sText = pCurrentField->getValue();
			
			//Note the text box content is not to be translated but only encoded if it is not html to be shown
			//If text value is HTML the target HTML template have to be created without TEXTAREA tag			
			if(!pCurrentField->isValueHtml())
			{
				sText = MHelpFuncs::replaceNewLinesForDsk(sText);
				sText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sText);
			}

			sRes.replace(sTextToChange, sText, Qt::CaseInsensitive);
			
			sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
			sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_READONLY:"";
			sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);

			// minlen
			sParamToChange = QString("%%1-minlen%").arg(pCurrentField->getName());
			iTemp = pCurrentField->getMinLen();
			sParam = (i>=0)?QString("%1").arg(iTemp):"";							
			sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
			
			// maxlen
			sParamToChange = QString("%%1-maxlen%").arg(pCurrentField->getName());
			iTemp = pCurrentField->getMaxLen();
			sParam = (i>=0)?QString("%1").arg(iTemp):"";							
			sRes.replace(sParamToChange, sParam, Qt::CaseInsensitive);
			//------------------------------------------------------------------------

			break;



		case MInputExField::enuDLG_EX_FIELD_LABEL:
		case MInputExField::enuDLG_EX_FIELD_BUTTON_OK:
		case MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL:
		case MInputExField::enuDLG_EX_FIELD_BUTTON:
			sTextToChange = QString("%%1-text%").arg(pCurrentField->getName());
			sText = pCurrentField->getText();
			sText = translateAndEncodeForWeb(sText);
			sRes.replace(sTextToChange, sText, Qt::CaseInsensitive);

			//control read-only status update
			sReadOnlyTextToChange = QString("%%1-readonly%").arg(pCurrentField->getName());
			sReadOnly = (pCurrentField->isReadOnly())?HTML_ATTRIBUTE_DISABLED:"";
			sRes.replace(sReadOnlyTextToChange, sReadOnly, Qt::CaseInsensitive);
			if(pCurrentField->getType() == MInputExField::enuDLG_EX_FIELD_BUTTON_OK)
				bOKButtonIsReadOnly = pCurrentField->isReadOnly();
			break;
		case MInputExField::enuDLG_EX_FIELD_LABEL_LUHN_CHECK:
			break;

		default:
			TRACE_WEB_W(QString("MIhmWebVisibleObject::updateInputExFileds:Invalid MInputExField type for field [%1] type:%2")
					.arg(pCurrentField->getName()).arg((int)pCurrentField->getType()));

			break;
		}
	}
	

	bool bConfigUpdateOKOnValidation = !(MIhmConfigWeb::getCfg()->getInputDlgsValidAlwaysEnabled());
		
	QString sUpdateOKOnValidation = (!bOKButtonIsReadOnly && bConfigUpdateOKOnValidation)?"true":"false";
		
	sRes.replace("%dialog-update-ok-button-on-validation%", sUpdateOKOnValidation, Qt::CaseInsensitive);


	return sRes;

}



//-----------------------------------------------------------------
MIhmWebVisibleInControlMode::MIhmWebVisibleInControlMode(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;
	m_bReady = true;
	m_bReadOnly = true;

	generateTimestamp();
}

MIhmWebVisibleInControlMode::~MIhmWebVisibleInControlMode()
{

}

		
void MIhmWebVisibleInControlMode::initTemplateHtml(QString sHtml)
{

}

QString MIhmWebVisibleInControlMode::getHtml(bool bReadOnly)
{
	return QString("");
}

void MIhmWebVisibleInControlMode::setReadOnly(bool bReadOnly)
{
	m_bReadOnly = bReadOnly;
	generateTimestamp();
}


QString MIhmWebVisibleInControlMode::getMode(bool bReadOnly)
{
	if(!bReadOnly)
		return QString("incontrol");
	else
		return QString("readonly");
}

//-----------------------------------------------------------------



MIhmWebVisibleOpenLink::MIhmWebVisibleOpenLink(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;
	m_bReady = true;
	m_bReadOnly = true;

	generateTimestamp();
	setBeingUpdated();
}

MIhmWebVisibleOpenLink::~MIhmWebVisibleOpenLink()
{
	clearRequests();
}

void MIhmWebVisibleOpenLink::clearRequests()
{
	while (!m_lstRequests.isEmpty())
		delete m_lstRequests.takeFirst();

}	

void MIhmWebVisibleOpenLink::setReadOnly(bool bIsReadOnly)
{
	m_bReadOnly = bIsReadOnly;
	
	if(m_bReadOnly)
	{
		clearRequests();
	}
}


void MIhmWebVisibleOpenLink::appendOpenLinkReq(QString sUrl, int iTargetWin, QString sButtonText, bool bOpen)
{
	MOpenLinkReq * pNew = new MOpenLinkReq();

	pNew->m_iIndex = iTargetWin;
	pNew->m_sUrl = sUrl;
	pNew->m_sButtonText = sButtonText;
	pNew->m_bClose = !bOpen;

	generateTimestamp();
	pNew->m_sTimeStamp = m_sUpdTimeStamp;

	m_lstRequests.append(pNew);
}

//Only for OpenLink visible object the isUpdated is keept this way
// because the open link commands are executed at the client one at a time
bool MIhmWebVisibleOpenLink::isUpdated()
{
	return (!m_lstRequests.isEmpty()); 
}

void MIhmWebVisibleOpenLink::setAckTimeStamp(QString sTS)
{
	MOpenLinkReq * pCurrent = NULL;

	if(!m_lstRequests.isEmpty())
	{
		pCurrent = m_lstRequests.at(0);

		if(pCurrent->m_sTimeStamp == sTS)
		{
			delete m_lstRequests.takeFirst();
			
			if(m_lstRequests.isEmpty())
				setBeingUpdated();
		}
	}
}


QString MIhmWebVisibleOpenLink::getHtml(bool bReadOnly)
{
	return QString("");
}


MOpenLinkReq * MIhmWebVisibleOpenLink::getOpenLinkReq()
{	
	MOpenLinkReq *pRetVal = NULL;
	
	if(m_lstRequests.size()>0)
		pRetVal = m_lstRequests.at(0);

	return pRetVal;
}	


//-----------------------------------------------------------------


MIhmWebVisibleContainer::MIhmWebVisibleContainer(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bReady = true;
}

MIhmWebVisibleContainer::~MIhmWebVisibleContainer()
{

}

QString MIhmWebVisibleContainer::getStyle(bool bReadOnly)
{
	QString sVisibilityStyle = (m_bVisible)?"":"display:none;";
	return m_sStyle + sVisibilityStyle;
}

		
void MIhmWebVisibleContainer::initTemplateHtml(QString sHtml)
{
}

QString MIhmWebVisibleContainer::getHtml(bool bReadOnly)
{
	return QString();
}

//-----------------------------------------------------------------


MIhmWebVisibleCtrlDependContainer::MIhmWebVisibleCtrlDependContainer(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId) 
{ 
	m_bReady = true;
}

MIhmWebVisibleCtrlDependContainer::~MIhmWebVisibleCtrlDependContainer()
{

}

QString MIhmWebVisibleCtrlDependContainer::getStyle(bool bReadOnly)
{
	bool bShow = (m_bVisible && !bReadOnly); //dependent on read only status of the session

	QString sVisibilityStyle = (bShow)?"":"display:none;";
	return m_sStyle + sVisibilityStyle;
}

		
void MIhmWebVisibleCtrlDependContainer::initTemplateHtml(QString sHtml)
{
}

QString MIhmWebVisibleCtrlDependContainer::getHtml(bool bReadOnly)
{
	return QString();
}

//-----------------------------------------------------------------

//-----------------------------------------------------------------


MIhmWebVisibleTextLabel::MIhmWebVisibleTextLabel(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 

}

MIhmWebVisibleTextLabel::~MIhmWebVisibleTextLabel()
{

}

		
void MIhmWebVisibleTextLabel::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleTextLabel::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	m_sTemplateHtml = sHtml;
	generateTimestamp();

	if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_ID,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_STYLE,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_TEXT,0,Qt::CaseInsensitive)!=-1)
			m_bHtmlTemplAvailable = true;

}

void MIhmWebVisibleTextLabel::setTextLabel(QString sTranslation)
{
	m_bReady = true;
	
	m_sText = sTranslation;

	generateTimestamp();
}


QString MIhmWebVisibleTextLabel::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			*psResult = m_sTemplateHtml;
				
			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_ID,m_sName,Qt::CaseInsensitive);
			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_STYLE,m_sStyle,Qt::CaseInsensitive);
			

			if(m_sText.isEmpty())
				psResult->replace(HTML_TEMPLATE_TEXT_LABEL_TEXT,"&nbsp;",Qt::CaseInsensitive);
			else
				psResult->replace(HTML_TEMPLATE_TEXT_LABEL_TEXT,m_sText,Qt::CaseInsensitive);

			QString sClass;

			if(!isVisible())
				sClass = HTML_CLASS_HIDDEN;
			else if(getHasAction() && !bReadOnly) 
				sClass += HTML_CLASS_CLICKABLE;
			
			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_CLASS,sClass,Qt::CaseInsensitive);
		}

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return QString();
}

//-----------------------------------------------------------------
//-----------------------------------------------------------------


#define SUB_TEMPLATE_NAME_BUTTON_ICON "ButtonImage"
#define SUB_TEMPLATE_NAME_BUTTON_TEXT "ButtonText"
#define HTML_TEMPLATE_TEXT_BUTTON_TEXT "%button-text%"
#define HTML_TEMPLATE_TEXT_BUTTON_IMAGE "%button-image%"


MIhmWebVisibleButton::MIhmWebVisibleButton(enumWebVisibleObjectId eId, bool bReadOnlyInvariant):
 	MIhmWebVisibleObject(eId)

{ 
	m_bEnabled = true;
	m_bReadOnlyInvariant = bReadOnlyInvariant;
	m_bReady = false;

}

MIhmWebVisibleButton::~MIhmWebVisibleButton()
{

}

		
void MIhmWebVisibleButton::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleButton::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	m_sTemplateHtml = sHtml;
	generateTimestamp();

	if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_ID,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_STYLE,0,Qt::CaseInsensitive)!=-1)
			m_bHtmlTemplAvailable = true;
		else
			TRACE_WEB_W(QString("MIhmWebVisibleButton::initTemplateHtml:Error in template file for object %1!")
					.arg(m_sName));
}

bool MIhmWebVisibleButton::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	m_sIconTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, SUB_TEMPLATE_NAME_BUTTON_ICON);
	m_sTextTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, SUB_TEMPLATE_NAME_BUTTON_TEXT);

	generateTimestamp();
	
	if (m_sIconTemplate == "" || m_sTextTemplate == "")
		m_bHtmlTemplAvailable = false;
	else
		if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_ID,0,Qt::CaseInsensitive)!=-1 &&
			m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_STYLE,0,Qt::CaseInsensitive)!=-1)
		{
			m_bHtmlTemplAvailable = true;
			m_bReady = true;
		}
		else
			TRACE_WEB_W(QString("MIhmWebVisibleButton::initSubTemplates:Error in sub template files for object %1!")
					.arg(sObjectId));

	return m_bHtmlTemplAvailable;
}

void MIhmWebVisibleButton::setTextLabel(QString sTranslation)
{
	m_sText = sTranslation;
	generateTimestamp();

}

void MIhmWebVisibleButton::setIcon(QString sIcon)
{
	m_sIcon = sIcon;
	generateTimestamp();
}


QString MIhmWebVisibleButton::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable && m_bReady)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			*psResult = m_sTemplateHtml;
			
			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_ID,m_sName,Qt::CaseInsensitive);
			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_STYLE,m_sStyle,Qt::CaseInsensitive);
			
			QString sClass;

			bool bActualEnabled;
			if(m_bReadOnlyInvariant)
				bActualEnabled = m_bEnabled;
			else
				bActualEnabled = !bReadOnly && m_bEnabled;

			if(!bActualEnabled)
			{
				sClass = HTML_CLASS_DISABLED;
			}
			else
			{
				sClass = HTML_CLASS_CLICKABLE;
			}

			if(!isVisible())
				sClass += QString(" %1").arg(HTML_CLASS_HIDDEN);

			psResult->replace(HTML_TEMPLATE_TEXT_LABEL_CLASS,sClass,Qt::CaseInsensitive);

			QString sSubIcon;

			if(m_sIcon!="")
			{
				sSubIcon = m_sIconTemplate;
				sSubIcon.replace(HTML_TEMPLATE_IMAGE_SRC,m_sIcon,Qt::CaseInsensitive);
			}
			
			psResult->replace(HTML_TEMPLATE_TEXT_BUTTON_IMAGE,sSubIcon,Qt::CaseInsensitive);

			QString sSubText;

			if(m_sText!="")
			{
				sSubText = m_sTextTemplate;
				sSubText.replace(HTML_TEMPLATE_TEXT_LABEL_TEXT,m_sText,Qt::CaseInsensitive);
			}

			psResult->replace(HTML_TEMPLATE_TEXT_BUTTON_TEXT,sSubText,Qt::CaseInsensitive);
		}

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return QString();
}

void MIhmWebVisibleButton::setEnabled(bool bEnabled)
{
	m_bReady = true;
	m_bEnabled = bEnabled;
	generateTimestamp();

}


//-----------------------------------------------------------------

MIhmWebVisibleImage::MIhmWebVisibleImage(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 

}

MIhmWebVisibleImage::~MIhmWebVisibleImage()
{

}

		
void MIhmWebVisibleImage::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleImage::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	
	m_sTemplateHtml = sHtml;
	generateTimestamp();

	if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_ID,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_STYLE,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_SRC,0,Qt::CaseInsensitive)!=-1)
			m_bHtmlTemplAvailable = true;
		else
			TRACE_WEB_W(QString("MIhmWebVisibleImage::initTemplateHtml:Error in template file for object %1!")
					.arg(m_sName));

}

void MIhmWebVisibleImage::setImage(QString sImage)
{
	m_bReady = true;

	m_sImage = sImage;
	generateTimestamp();

}

QString MIhmWebVisibleImage::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable && m_bReady)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			*psResult = m_sTemplateHtml;
		
			psResult->replace(HTML_TEMPLATE_IMAGE_ID,m_sName,Qt::CaseInsensitive);
			psResult->replace(HTML_TEMPLATE_IMAGE_STYLE,m_sStyle,Qt::CaseInsensitive);
			psResult->replace(HTML_TEMPLATE_IMAGE_SRC,m_sImage,Qt::CaseInsensitive);

			QString sClass;

			if(!isVisible())
				sClass += HTML_CLASS_HIDDEN;
			else if(getHasAction() && !bReadOnly)
				sClass += HTML_CLASS_CLICKABLE;

			psResult->replace(HTML_TEMPLATE_IMAGE_CLASS,sClass,Qt::CaseInsensitive);

		}

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}

	return QString();
}

//-----------------------------------------------------------------
// For dynamic images on the web ....	
//-----------------------------------------------------------------

MIhmWebVisibleDynImage::MIhmWebVisibleDynImage(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 

}

MIhmWebVisibleDynImage::~MIhmWebVisibleDynImage()
{

}

		
void MIhmWebVisibleDynImage::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleDynImage::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	
	m_sTemplateHtml = sHtml;
	generateTimestamp();

	if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_ID,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_STYLE,0,Qt::CaseInsensitive)!=-1 &&
		m_sTemplateHtml.indexOf(HTML_TEMPLATE_IMAGE_SRC,0,Qt::CaseInsensitive)!=-1)
			m_bHtmlTemplAvailable = true;
		else
			TRACE_WEB_W(QString("MIhmWebVisibleDynImage::initTemplateHtml:Error in template file for object %1!")
					.arg(m_sName));

}

void MIhmWebVisibleDynImage::setImagePath(QString sImagePath)
{
	m_bReady = true;

	m_sImagePath = sImagePath;
	generateTimestamp();

}

QString MIhmWebVisibleDynImage::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			*psResult = m_sTemplateHtml;
		
			psResult->replace(HTML_TEMPLATE_IMAGE_ID,m_sName,Qt::CaseInsensitive);
			psResult->replace(HTML_TEMPLATE_IMAGE_STYLE,m_sStyle,Qt::CaseInsensitive);

			QString sRandNum = generateNewItemID();
			m_sImageSrc = QString("proxy/get_image.php?image=%1&dummy=%2").arg(m_sName).arg(sRandNum);
			psResult->replace(HTML_TEMPLATE_IMAGE_SRC, m_sImageSrc,Qt::CaseInsensitive);

			QString sClass;

			if(!isVisible())
				sClass += HTML_CLASS_HIDDEN;
			else if(getHasAction() && !bReadOnly)
				sClass += HTML_CLASS_CLICKABLE;

			psResult->replace(HTML_TEMPLATE_IMAGE_CLASS,sClass,Qt::CaseInsensitive);

		}
		
		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}

	return QString();
}

//-----------------------------------------------------------------

#define REG_VAL_HTML_LIST_ITEM_ICON "LstItemIcon"
#define REG_VAL_HTML_LIST_ITEM_TEXT "LstItemText"
#define REG_VAL_HTML_LIST_IS_ICON_MODE "IsIconMode"


MIhmWebVisibleList::MIhmWebVisibleList(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bIsIconMode = false;
	m_bHtmlTemplAvailable = false;
}

MIhmWebVisibleList::~MIhmWebVisibleList()
{

}

		
void MIhmWebVisibleList::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleList::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	m_sTemplateHtml = sHtml;
	generateTimestamp();


}


bool MIhmWebVisibleList::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{

	m_sListItemIconTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_HTML_LIST_ITEM_ICON);
	m_sListItemTextTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_HTML_LIST_ITEM_TEXT);

	QString sParamValue = pLaneSettings->getVisObjParamValue(sObjectId,REG_VAL_HTML_LIST_IS_ICON_MODE);
	m_bIsIconMode = (sParamValue == "1"|| sParamValue == "true")?true:false;

	if(m_sListItemIconTemplate=="" || m_sListItemTextTemplate=="")
	{
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initSubTemplates:Sub template files for object %1 not defined!")
					.arg(sObjectId));
	}
	else
		if(m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_ID,0,Qt::CaseInsensitive)!=-1 &&
			m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_STYLE,0,Qt::CaseInsensitive)!=-1 &&
			m_sTemplateHtml.indexOf(HTML_TEMPLATE_TEXT_LABEL_TEXT,0,Qt::CaseInsensitive)!=-1)
				m_bHtmlTemplAvailable = true;
		else
			TRACE_WEB_W(QString("MIhmWebVisibleList::initSubTemplates:Error in sub template files for object %1!")
					.arg(sObjectId));

	return m_bHtmlTemplAvailable;
}



void MIhmWebVisibleList::setStringList(QStringList slst)
{
	m_bReady = true;
	
	m_slstText = slst;
	generateTimestamp();

}

#define HTML_TEMPLATE_TEXT_ITEM_ID "%id%"
#define HTML_TEMPLATE_TEXT_ITEM_CLASS "%class%"
#define HTML_TEMPLATE_TEXT_ITEM_TEXT "%text%"


QString MIhmWebVisibleList::generateListItemText(QString sItemText)
{
	QString sRes = m_sListItemTextTemplate;

	sRes.replace(HTML_TEMPLATE_TEXT_ITEM_CLASS,"",Qt::CaseInsensitive);
	sRes.replace(HTML_TEMPLATE_TEXT_ITEM_ID,"",Qt::CaseInsensitive);

	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
	QString sImageToolTip;
	pLang->getLabelTranslation(sItemText, MIhmLanguages::enuTranslTargetWeb, sImageToolTip);
	sRes.replace(HTML_TEMPLATE_TEXT_ITEM_TEXT,sImageToolTip,Qt::CaseInsensitive);

	return sRes;
}


#define HTML_TEMPLATE_ICON_ITEM_ID "%id%"
#define HTML_TEMPLATE_ICON_ITEM_CLASS "image alarm-item %class%"
#define HTML_TEMPLATE_ICON_ITEM_TEXT "%text%"
#define HTML_TEMPLATE_ICON_ITEM_SRC "%images-root%/%image-name%"


QString MIhmWebVisibleList::generateListItemIcon(QString sItemID)
{
	QString sRes = m_sListItemIconTemplate;

	sRes.replace(HTML_TEMPLATE_ICON_ITEM_ID,"",Qt::CaseInsensitive);
	sRes.replace(HTML_TEMPLATE_TEXT_ITEM_CLASS,"",Qt::CaseInsensitive);
	
	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
	QString sImageToolTip;
	pLang->getLabelTranslation(sItemID, MIhmLanguages::enuTranslTargetWeb, sImageToolTip);
	sRes.replace(HTML_TEMPLATE_ICON_ITEM_TEXT,sImageToolTip,Qt::CaseInsensitive);

	QString sImagePath = MIhmConfigWeb::getCfg()->getWebConfigImages()->getImageFullPath(sItemID);
	sRes.replace(HTML_TEMPLATE_ICON_ITEM_SRC,sImagePath,Qt::CaseInsensitive);
	
	return sRes;
}



QString MIhmWebVisibleList::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(!m_bHtmlPrepared)
		{
			m_bHtmlPrepared = true;

			m_sResHtml = m_sTemplateHtml;

			QString sItemsList;

			for (int i=m_slstText.size()-1;i>=0;i--)
			{
				if(!m_bIsIconMode)
				{
					sItemsList += generateListItemText(m_slstText.at(i));
				}
				else
					sItemsList += generateListItemIcon(m_slstText.at(i));
			}

			m_sResHtml.replace(HTML_TEMPLATE_TEXT_LABEL_TEXT,sItemsList,Qt::CaseInsensitive);
			m_sResHtml.replace(HTML_TEMPLATE_TEXT_LABEL_ID,m_sName,Qt::CaseInsensitive);
			m_sResHtml.replace(HTML_TEMPLATE_TEXT_LABEL_STYLE,m_sStyle,Qt::CaseInsensitive);
			m_sResHtml.replace(HTML_TEMPLATE_TEXT_LABEL_CLASS,"",Qt::CaseInsensitive);
		}

		return m_sResHtml;
	}
	
	return QString();
}


//-----------------------------------------------------------------
//---------- WEB INPUT DIALOG    ----------------------------------
//-----------------------------------------------------------------

#define REG_VAL_DIALOG_TEMPL_BUTTON_OK					"ButtonOK"
#define REG_VAL_DIALOG_TEMPL_BUTTON_CANCEL				"ButtonCANCEL"
#define REG_VAL_DIALOG_TEMPL_LINEALARM				"LineAlarm"
#define REG_VAL_DIALOG_TEMPL_LINEEDIT				"LineEdit"
#define REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM			"LineEdit-Item"
#define REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_DECIMAL	"LineEdit-Item-Script-Decimal"
#define REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_INTEGER	"LineEdit-Item-Script-Integer"
#define REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_MASK		"LineEdit-Item-Script-Mask"
#define REG_VAL_DIALOG_TEMPL_LIST					"List"
#define REG_VAL_DIALOG_TEMPL_LIST_ITEM				"List-Item"




MIhmWebVisibleDialog::MIhmWebVisibleDialog(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_bInputInProgress = false;
}

MIhmWebVisibleDialog::~MIhmWebVisibleDialog()
{

}

		
void MIhmWebVisibleDialog::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleDialog::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	m_sTemplateHtml = sHtml;
	generateTimestamp();


}


bool MIhmWebVisibleDialog::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{

	m_sDialogButtonOkTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_BUTTON_OK);
	m_sDialogButtonCancelTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_BUTTON_CANCEL);
	m_sDialogLineAlarmTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEALARM);
	m_sDialogLineEditTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEEDIT);
	m_sDialogLineEditItemTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM);

	//script templates ...
	//-----------------------------------------------------------
	m_sDialogLineEditScriptDecimal = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_DECIMAL);
	m_sDialogLineEditScriptInteger = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_INTEGER);
	m_sDialogLineEditScriptMask = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LINEEDIT_ITEM_MASK);

	m_sDialogListTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LIST);
	m_sDialogListItemTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_DIALOG_TEMPL_LIST_ITEM);


	if(m_sDialogButtonOkTemplate=="" || 
		m_sDialogButtonCancelTemplate=="" ||
			m_sDialogLineEditTemplate=="" || 
			m_sDialogLineEditItemTemplate=="" || 
			m_sDialogLineEditScriptDecimal=="" || 
			m_sDialogLineEditScriptInteger=="" || 
			m_sDialogLineEditScriptMask=="" || 
			m_sDialogListTemplate=="" || 
			m_sDialogListItemTemplate== "" ||
			m_sTemplateHtml == "")
	{
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebVisibleDialog::initSubTemplates:Error in sub template files for object %1!").arg(sObjectId));
	}
	else
		m_bHtmlTemplAvailable = true;

	return m_bHtmlTemplAvailable;
}



bool MIhmWebVisibleDialog::initialize(MInputDialogReq * pReq)
{
	m_bReady = true;
	
	m_oInputDiagReq = *pReq;
	generateTimestamp();

	return true;
}

void MIhmWebVisibleDialog::setDialogInProgress(bool bInProgress)
{
	m_bInputInProgress = bInProgress;
	generateTimestamp();
}

void MIhmWebVisibleDialog::setReadOnly(bool bIsReadOnly)
{
	//Update only if changed 
	// to avoid continuously send ShowDlgCommand 
	// that would reset the dialog data
	if(m_bReadOnly!=bIsReadOnly)
	{
		m_bReadOnly = bIsReadOnly;	
		generateTimestamp();
	}

}

#define HTML_TEMPLATE_DIALOG_STYLE			"%dialog-style%"
#define HTML_TEMPLATE_DIALOG_TITLE			"%dialog-title%"
#define HTML_TEMPLATE_DIALOG_TITLE_STYLE	"%dialog-title-style%"
#define HTML_TEMPLATE_DIALOG_BODY			"%dialog-body%"
#define HTML_TEMPLATE_DIALOG_BODY_STYLE		"%dialog-body-style%"
#define HTML_TEMPLATE_DIALOG_BUTTONS		"%dialog-buttons%"
#define HTML_TEMPLATE_DIALOG_BUTTONS_STYLE	"%dialog-buttons-style%"
#define HTML_TEMPLATE_DIALOG_ID				"%dialog-id%"

#define HTML_TEMPLATE_DIALOG_BUTTON_TEXT		"%text%"
#define HTML_TEMPLATE_DIALOG_BUTTON_DISABLED	"%disabled%"


QString MIhmWebVisibleDialog::getHtml(bool bReadOnly)
{
	QString sRes;
	QString sItemsList;
	

	if(m_bHtmlTemplAvailable && m_bInputInProgress)
	{	
		sRes = m_sTemplateHtml;
		
		//parameters that are not used
		sRes.replace(HTML_TEMPLATE_DIALOG_STYLE,"",Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_DIALOG_TITLE_STYLE,"",Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_DIALOG_BODY_STYLE,"",Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_DIALOG_BUTTONS_STYLE,"",Qt::CaseInsensitive);

		//dialog title
		QString sDlgTitle = m_oInputDiagReq.getDialogTitle();
		MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sDlgTitle, MIhmLanguages::enuTranslTargetWeb, sDlgTitle);
		sRes.replace(HTML_TEMPLATE_DIALOG_TITLE, sDlgTitle, Qt::CaseInsensitive);
		
		//generate body html...
		QString sBodyHtml;	
		
		if(m_oInputDiagReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_FIELD || 
				m_oInputDiagReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM)
		{
			sBodyHtml = generateBodyHtmlFields();
		}
		else if(m_oInputDiagReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_CHOICE ||
			m_oInputDiagReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_BIGCHOICE)

		{
			sBodyHtml = generateBodyHtmlChoice();
		}

		sRes.replace(HTML_TEMPLATE_DIALOG_BODY,sBodyHtml,Qt::CaseInsensitive);

		QString sButtons;	
		sButtons = generateButtonsHtml();
		
		sRes.replace(HTML_TEMPLATE_DIALOG_BUTTONS,sButtons,Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_DIALOG_ID, m_sName, Qt::CaseInsensitive);

		if(m_bReadOnly)
			sRes.replace(HTML_TEMPLATE_DIALOG_BUTTON_DISABLED,"disabled=\"disabled\"",Qt::CaseInsensitive);
		else
			sRes.replace(HTML_TEMPLATE_DIALOG_BUTTON_DISABLED,"",Qt::CaseInsensitive);

	}
	
	if(sRes!="")
	{
		if(MTracer::getWebTracer()->isTrcEnabled())
			TRACE_WEB_D(QString("MIhmWebVisibleDialog::getHtml:Name:%1, HTML:%2")
						.arg(getName()).arg(sRes));
	}

	return sRes;
}

QString MIhmWebVisibleDialog::generateButtonsHtml()
{
	QString sButtonsHtml;
	
	QString sButton;

	if(m_oInputDiagReq.hasOKButton())
	{
		sButton = m_sDialogButtonOkTemplate;
		QString sOKText = MIhmConfigWeb::getCfg()->getLanguages()->getOKButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
		sButton.replace(HTML_TEMPLATE_DIALOG_BUTTON_TEXT,sOKText,Qt::CaseInsensitive);
		
		sButtonsHtml += sButton;
	}

	if(m_oInputDiagReq.hasCancelButton())
	{
		sButton = m_sDialogButtonCancelTemplate;

		QString sCancelText = MIhmConfigWeb::getCfg()->getLanguages()->getCancelButtonTranslation(MIhmLanguages::enuTranslTargetWeb);
		sButton.replace(HTML_TEMPLATE_DIALOG_BUTTON_TEXT,sCancelText,Qt::CaseInsensitive);
		
		sButtonsHtml += sButton;
	}

	return sButtonsHtml;
}

bool MIhmWebVisibleDialog::generateLineEditScript(	QString sFormat, 
													QString sCtrlId, 
													QString sValue,
													bool *pbIsPassword, 
													QString &sEditScriptHtml)
{
	char cType;

	if(sFormat.size()<1) 
			return false;

	cType = sFormat.at(0).toUpper().toLatin1();	
		
	if(cType == MInputDialogReq::cFORMAT_CHAR_PASSWORD)
	{
		if(sFormat.size()<2) 
				return false;
		
		sFormat = sFormat.mid(1);
		cType = sFormat.at(0).toUpper().toLatin1();	
		*pbIsPassword = true;

	}
	else
		*pbIsPassword = false;

	if(sFormat.size()<1) 
			return false;

	if(cType == MInputDialogReq::cFORMAT_CHAR_STRING)
	{
		return true; 
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_MASK)
	{
		sFormat = sFormat.mid(1);
		QStringList sLst = sFormat.split(";");
		QString	sMask = sLst.at(0);

		QString sPlaceHolder = " ";
		
		if(sLst.size()>1)
			sPlaceHolder = sLst.at(1);

		sEditScriptHtml = m_sDialogLineEditScriptMask;
		sEditScriptHtml.replace("%id%",sCtrlId,Qt::CaseInsensitive);
		sEditScriptHtml.replace("%mask%",sMask,Qt::CaseInsensitive);
		sEditScriptHtml.replace("%place-holder%",sPlaceHolder,Qt::CaseInsensitive);
		sEditScriptHtml.replace("%value%",sValue,Qt::CaseInsensitive);

		return true; 
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_RIGHTMASK)
	{
		sEditScriptHtml = "";
		return true; 
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_NUMBER)
	{
		int iSize = sFormat.size();
		if(iSize<=1) 
			return false;

		QString sMask, sPart;
		bool bSigned = false;
		bool bLeftPadZeroes = false;
		int iPos = 1;
		QChar chCurrent = sFormat.at(iPos);

		if(chCurrent == IHM_FIELD_FORMAT_CHAR_SIGNED)
		{
			sMask = "-?";
			iPos++;
			bSigned = true;
			if(iSize<=iPos) return false;
			chCurrent = sFormat.at(iPos);
		}
		
		if(chCurrent == IHM_FIELD_FORMAT_CHAR_ZERO)
		{
			bLeftPadZeroes = true;
			iPos++;
			
			if(iSize<=iPos) 
				return false;
			chCurrent = sFormat.at(iPos);
		}

		sPart = "";
		
		while(chCurrent.isDigit())
		{
			sPart += chCurrent;
			iPos++;
			if(iSize<=iPos) 
				break;
			chCurrent = sFormat.at(iPos);
		}

		bool bOK;

		int iNumDigits = sPart.toInt(&bOK);

		if(iSize<=iPos) 
		{
// 			bNumber = true;	bDecimal = false;
			sEditScriptHtml = m_sDialogLineEditScriptInteger;
			sEditScriptHtml.replace("%id%",sCtrlId,Qt::CaseInsensitive);
			sEditScriptHtml.replace("%pre-decimals%",QString("%1").arg(iNumDigits),Qt::CaseInsensitive);
			
			QString sAllowNegative = bSigned?"true":"false";
			sEditScriptHtml.replace("%allow-negative%", sAllowNegative,Qt::CaseInsensitive);
			sEditScriptHtml.replace("%value%",sValue,Qt::CaseInsensitive);

			return true;  
		}

		QString sDecimalSeparator = sFormat.at(iPos);

		iPos++;
		
		if(iSize<=iPos) 
				return false;
		
		chCurrent = sFormat.at(iPos);

		sPart = "";
		while(chCurrent.isDigit())
		{
			sPart += chCurrent;
			
			iPos++;
			if(iSize<=iPos) 
				break;
			chCurrent = sFormat.at(iPos);
		}

		int iNumDecimals = sPart.toInt(&bOK);
		
		//m_bNumber = true;	m_bDecimal = true;
		sEditScriptHtml = m_sDialogLineEditScriptDecimal;
		sEditScriptHtml.replace("%id%",sCtrlId,Qt::CaseInsensitive);
		sEditScriptHtml.replace("%pre-decimals%",QString("%1").arg(iNumDigits),Qt::CaseInsensitive);
		sEditScriptHtml.replace("%decimals%",QString("%1").arg(iNumDecimals),Qt::CaseInsensitive);
		
		QString sAllowNegative = bSigned?"true":"false";
		sEditScriptHtml.replace("%allow-negative%", sAllowNegative,Qt::CaseInsensitive);
		sEditScriptHtml.replace("%separator-codes%", "44, 46",Qt::CaseInsensitive);
		sEditScriptHtml.replace("%separator-char%", sDecimalSeparator, Qt::CaseInsensitive);
		sEditScriptHtml.replace("%value%",sValue,Qt::CaseInsensitive);

		return true;  
	}	
	else if(cType == MInputDialogReq::cFORMAT_CHAR_FUNCTION)
	{
		sEditScriptHtml = "";
		return true; 
	}	
	else 
	{
		//default:
		TRACE_WEB_W(QString("MIhmWebVisibleDialog::generateLineEditScript: Unknown format:%1!")
						.arg(sFormat.toLatin1().data()));
	}

	return false;
}




QString MIhmWebVisibleDialog::generateFieldHtml(QString sDefinition, 
												QString sDescription, 
												int iLineNo, 
												OUT int *iLabelCharSize, 
												OUT int *iInputCharSize)
{
	QString sFieldHtml;
	int iNumItems;
	int iItemsShift;
	QString sLabelText;
	bool bLocked;

//     Extraire le nombre d'item ou le libelle
	QString sTemp = MHelpFuncs::getStringItem(sDefinition,0, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	if(MHelpFuncs::isInt(sTemp))
	{
		iNumItems = sTemp.toInt();
		iItemsShift = 1;
		
		sTemp = MHelpFuncs::getStringItem(sDefinition,1, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	}
	else
	{
		iNumItems = 1;
		iItemsShift = 0;
	}

	if(iNumItems==0)
	{
		TRACE_WEB_D( "MIhmWebVisibleDialog::generateFieldHtml: Error: m_iNumItems==0!");
		return "";
	}

	//extract the label text
	if(sTemp == "")
	{
		sLabelText = "";
		bLocked = false;
	}
	else
	{
		if(sTemp.at(0) == MInputDialogReq::cSAISIE_VEROU)  // is '!' (is locked)
		{
			bLocked = true;
			sTemp = sTemp.mid(1);
		}
		else
			bLocked = false;

		MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sTemp, MIhmLanguages::enuTranslTargetWeb, sLabelText);
	}

	*iLabelCharSize = sLabelText.size();
	
	//extract the format
	QString sFormat = MHelpFuncs::getStringItem(sDefinition,1 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	//  Extraire la taille min 
	sTemp = MHelpFuncs::getStringItem(sDefinition,2 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	bool bOK;
	int iMinLen = sTemp.toInt(&bOK,10);

	if(!bOK)
	{
		TRACE_WEB_D( "MIhmWebVisibleDialog::generateFieldHtml: Error decoding field m_iMinLen!");
		return "";
	}

	//extract the max len in format:" ~9^0.00|"
	sTemp = MHelpFuncs::getStringItem(sDefinition, 3 + iItemsShift , QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);
	
	QString sMaxLen = MHelpFuncs::getStringItem(sTemp,0 , QString(MInputDialogReq::cSAISIE_RAZ_ZONE_SAISIE), true);
	int iMaxLen = sMaxLen.toInt(&bOK,10);
	
	if(!bOK)
	{
		TRACE_WEB_D( "MIhmWebVisibleDialog::generateFieldHtml: Error decoding field m_iMaxLen!");
		return "";
	}
	
	//now parse the description
	//------------------------------------------------------------
	
	QString sUserInfo = MHelpFuncs::getStringItem(sDescription,0, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), true);

	if(sUserInfo =="")
	{
		TRACE_WEB_D( "MIhmWebVisibleDialog::generateFieldHtml: Error decoding sUserInfo!");
		return "";
	}
	
//     Initial values
	QString sInitialValues = MHelpFuncs::getStringItem(sDescription,1, QString(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM), false);
	bool bIsAlarm = (m_oInputDiagReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM);
	
	
	QString sLineEditItems;
	QString sLineEditScripts;

	if(!bIsAlarm)
	{
		for(int i=0;i<iNumItems;i++)
		{
			QString sValue = MHelpFuncs::getStringItem(sInitialValues,i, QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), false);

			if(sValue!="")
			{
				//if first char is '?' translate the text 
				if(sValue.at(0) == MInputDialogReq::cSAISIE_DECODE_TEXTE)
				{
					sValue = sValue.mid(1);
					MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sValue, MIhmLanguages::enuTranslTargetWeb, sValue);
					
				}
			}
			
			*iInputCharSize = sValue.size();

			QString sLineEditItem;
			sLineEditItem = m_sDialogLineEditItemTemplate; 	
			QString sId = QString("Item_%1_%2").arg(iLineNo).arg(i);
			sLineEditItem.replace("%id%",sId,Qt::CaseInsensitive);
			sLineEditItem.replace("%value%",sValue,Qt::CaseInsensitive);
			sLineEditItem.replace("%max-length%",QString("%1").arg(iMaxLen),Qt::CaseInsensitive);
			//sLineEditItem.replace("%char-size%","",Qt::CaseInsensitive);
			
			if(bLocked)
				sLineEditItem.replace("%readonly%","readonly=\"readonly\"",Qt::CaseInsensitive);
			else
				sLineEditItem.replace("%readonly%","",Qt::CaseInsensitive);


			QString sLineEditScript;
			bool bIsPassword;
				
			if(!generateLineEditScript(sFormat, sId, sValue, &bIsPassword, sLineEditScript))
			{
				TRACE_WEB_D(QString("MIhmWebVisibleDialog::generateFieldHtml: Error setting format %1!")
							.arg(sFormat));
				return "";
			}
			else 
			{
				if(bIsPassword)
					sLineEditItem.replace("%type%","password",Qt::CaseInsensitive);
				else
					sLineEditItem.replace("%type%","text",Qt::CaseInsensitive);

				sLineEditItems += sLineEditItem;
				sLineEditScripts += sLineEditScript;
			}

		}

		sFieldHtml = m_sDialogLineEditTemplate;
		
		sFieldHtml.replace("%line-edit-label-value%", sLabelText ,Qt::CaseInsensitive);
		sFieldHtml.replace("%line-edit-label-style%", "" ,Qt::CaseInsensitive);
		//sFieldHtml.replace("%line-edit-label-char-size%", "" ,Qt::CaseInsensitive);
		sFieldHtml.replace("%line-edit-item-desc%", sUserInfo ,Qt::CaseInsensitive);

		sFieldHtml.replace("%line-edit-items%", sLineEditItems ,Qt::CaseInsensitive);
		sFieldHtml.replace("%line-edit-scripts%", sLineEditScripts ,Qt::CaseInsensitive);

	}
	else //if it is an alarm dialog use the Caption label to show all the text
	{
		QString sMessage = sLabelText;

		for(int i=0;i<iNumItems;i++)
		{
			sTemp = MHelpFuncs::getStringItem(sInitialValues,i, QString(MInputDialogReq::cSAISIE_SEPARATEUR_ITEM), false);
			
			if(sTemp!="")
			{
				//if first char is '?' translate the text 
				if(sTemp.at(0) == MInputDialogReq::cSAISIE_DECODE_TEXTE)
				{
					sTemp = sTemp.mid(1);
					MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sTemp, MIhmLanguages::enuTranslTargetWeb, sTemp);
				}
			}
		
			if(sMessage!="")
				sMessage += QString("<br/>");

			sMessage += sTemp;
		}
		
		sLabelText = sMessage;

		sFieldHtml = m_sDialogLineAlarmTemplate;

		sFieldHtml.replace("%text-value%", sLabelText ,Qt::CaseInsensitive);

	}


	

// 	pNewInput->setFont(fntTitle);
// 	pNewInput->setFontSize(iNormalFontSize);
// 
// 	//Init control colors if default template form is used
// 	if(pReq->getUITemplate()=="")
// 	{
// 		if(m_oReq.getDialogType()==MInputDialogReq::enuSAISIE_CAT_ALARM)
// 			pNewInput->setColors(m_sCol3,m_sCol3,m_sCol3,m_sCol3,m_sCol4,m_sCol4);
// 		else
// 			pNewInput->setColors(m_sCol3,"","",m_sCol3,m_sCol4,"");
// 	}
	
	return sFieldHtml;

}

#define MIN_LABEL_SIZE 20
#define MIN_INPUT_SIZE 20


QString MIhmWebVisibleDialog::generateBodyHtmlFields()
{
	QString sBodyHtml;

	int iNumFields = m_oInputDiagReq.getNumInputFileds();
	int iMaxLabelCharSize = 0;
	int iMaxInputCharSize = 0;

	//init edit controls

	for(int i = 0; i<iNumFields; i++)
	{
		QString sFieldDef = m_oInputDiagReq.getFieldDefinition(i);
		QString sFieldDesc = m_oInputDiagReq.getFieldDescription(i);
		
		int iLabelCharSize = 0;
		int iInputCharSize = 0;
		QString sFieldHtml = generateFieldHtml(sFieldDef, sFieldDesc, i, &iLabelCharSize, &iInputCharSize);

		iMaxLabelCharSize = qMax(iMaxLabelCharSize, iLabelCharSize);
		iMaxInputCharSize = qMax(iMaxInputCharSize, iInputCharSize);

		sBodyHtml += sFieldHtml;
	}	
	
	if(iMaxLabelCharSize<MIN_LABEL_SIZE)
		iMaxLabelCharSize = MIN_LABEL_SIZE;
	if(iMaxInputCharSize<MIN_INPUT_SIZE)
		iMaxInputCharSize = MIN_INPUT_SIZE;
	
	sBodyHtml.replace("%line-edit-label-char-size%", QString("%1").arg(iMaxLabelCharSize) ,Qt::CaseInsensitive);
	sBodyHtml.replace("%char-size%", QString("%1").arg(iMaxInputCharSize) ,Qt::CaseInsensitive);

	return sBodyHtml;
}


QString MIhmWebVisibleDialog::generateBodyHtmlChoice()
{
	QString sBodyHtml;

	int iNumFields = m_oInputDiagReq.getNumInputFileds();
	//init list control
	QString sCaption = m_oInputDiagReq.getFieldDefinition(0);
	MIhmConfigWeb::getCfg()->getLanguages()->getLabelTranslation(sCaption, MIhmLanguages::enuTranslTargetWeb, sCaption);
	
	QString sListItems;
	
	for(int i = 0; i<iNumFields; i++)
	{
		QString sFieldDesc = m_oInputDiagReq.getFieldDescription(i);
		
		QStringList sLst = sFieldDesc.split(MInputDialogReq::cSAISIE_SEPARATEUR_PARAM);

		if(sLst.size()>=2)
		{
			QString sListItem = m_sDialogListItemTemplate;
			sListItem.replace("%list-item-value%", sLst.at(0) ,Qt::CaseInsensitive);
			sListItem.replace("%list-item-text%", sLst.at(1) ,Qt::CaseInsensitive);
			sListItem.replace("%list-item-selected%", "" ,Qt::CaseInsensitive);

			sListItems +=sListItem;
		}
	}

	sBodyHtml = m_sDialogListTemplate;
	sBodyHtml.replace("%list-title%", sCaption, Qt::CaseInsensitive);
	sBodyHtml.replace("%control-id%", "", Qt::CaseInsensitive);
	sBodyHtml.replace("%list-item-count%", QString("%1").arg(iNumFields), Qt::CaseInsensitive);
	sBodyHtml.replace("%list-items%", sListItems, Qt::CaseInsensitive);

	return sBodyHtml;
}


bool MIhmWebVisibleDialog::getCurrentReq(MInputDialogReq * pDest)
{ 
	*pDest = m_oInputDiagReq;
	return m_bInputInProgress;
}

//-----------------------------------------------------------------
//---------- END WEB DIALOG  ---------------------------------
//-----------------------------------------------------------------


MIhmWebVisibleDialogEx::MIhmWebVisibleDialogEx(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_bInputInProgress = false;

}

MIhmWebVisibleDialogEx::~MIhmWebVisibleDialogEx()
{

}

		
void MIhmWebVisibleDialogEx::initTemplateHtml(QString sHtml)
{
	//For DialogEx template is initialized at the initialize function because each dialog uses different html template
}

bool MIhmWebVisibleDialogEx::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	//For DialogEx template is initialized at the initialize function because each dialog uses different html template
	return true;
}



bool MIhmWebVisibleDialogEx::initialize(MInputDialogExReq * pReq, QString sDialogTemplateContent)
{
	m_bReady = true;
	
	m_oInputDiagExReq = *pReq;

	if(sDialogTemplateContent!="")
	{
		m_sTemplateHtml = sDialogTemplateContent;
		m_bHtmlTemplAvailable = true;
		m_bReady = true;
	}
	
	generateTimestamp();

	return m_bHtmlTemplAvailable;
}

void MIhmWebVisibleDialogEx::setDialogInProgress(bool bInProgress)
{
	m_bInputInProgress = bInProgress;
	generateTimestamp();
}

void MIhmWebVisibleDialogEx::setReadOnly(bool bIsReadOnly)
{
	//Update only if changed 
	// to avoid continuously send ShowDlgCommand 
	// that would reset the dialog data
	if(m_bReadOnly!=bIsReadOnly)
	{
		m_bReadOnly = bIsReadOnly;	
		generateTimestamp();

		if(m_bReadOnly)
			m_bInputInProgress = false;
	}

}


bool MIhmWebVisibleDialogEx::getCurrentExReq(MInputDialogExReq * pDest)
{ 
	*pDest = m_oInputDiagExReq;
	return m_bInputInProgress;
}






QString MIhmWebVisibleDialogEx::getHtml(bool bReadOnly)
{
	QString sRes;

	if(m_bHtmlTemplAvailable && m_bReady && m_bInputInProgress)
	{	
		QList <MInputExField *> * plstFieldList = m_oInputDiagExReq.getInputFields();
		sRes = updateInputExFileds(plstFieldList, m_sTemplateHtml);
		sRes.replace(HTML_TEMPLATE_DIALOG_ID, m_sName, Qt::CaseInsensitive);
	}
	
	if(sRes!="")
	{
		if(MTracer::getWebTracer()->isTrcEnabled())
			TRACE_WEB_D(QString("MIhmWebVisibleDialogEx::getHtml:Name:%1, HTML:%2")
						.arg(getName()).arg(sRes));
	}

	return sRes;
}



//-----------------------------------------------------------------
//---------- WEB HIGH PRIORITY DIALOG  ----------------------------
//-----------------------------------------------------------------

//Predefined labels that will be used to dynamically update the html template

#define HTML_TEMPLATE_HPRIO_DIALOG_ID			"%dialog-id%"

#define HTML_TEMPLATE_HPRIO_DIALOG_TITLE		"%lblTitle-text%"
#define HTML_TEMPLATE_HPRIO_DIALOG_MESSAGE		"%lblMessage-text%"


#define HTML_TEMPLATE_HPRIO_BTN_CANCEL_TEXT		"%btnCancel-text%"
#define HTML_TEMPLATE_HPRIO_BTN_CANCEL_STYLE	"%btnCancel-style%"

#define HTML_TEMPLATE_HPRIO_BTN_OK_TEXT			"%btnOK-text%"

#define HTML_TEMPLATE_HPRIO_LABEL_LOGIN_VALUE	"%lblLoginName-text%"
#define HTML_TEMPLATE_HPRIO_TEXT_LOGIN_VALUE	"%txtUserName-text%"
#define HTML_TEMPLATE_HPRIO_LABEL_PWD_VALUE		"%lblPassword-text%"


MIhmWebVisibleHPrioDialog::MIhmWebVisibleHPrioDialog(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_bReady = false;
	m_bInputInProgress = false;
}

MIhmWebVisibleHPrioDialog::~MIhmWebVisibleHPrioDialog()
{

}

		
void MIhmWebVisibleHPrioDialog::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));

	m_sTemplateHtml = sHtml;
	m_sLoginDlgTemplate = sHtml; // this one is the default one and loaded as TEMPLATE attribute of DlgHPrioInputDialog
	generateTimestamp();
	m_bHtmlTemplAvailable = true;
	m_bReady = false;
	m_bInputInProgress = false;

}


bool MIhmWebVisibleHPrioDialog::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	m_sQuestionDlgTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, HTML_HPRIO_SUBTEMPLATE_NAME_QUESTION);
	m_sInfoDlgTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, HTML_HPRIO_SUBTEMPLATE_NAME_INFO);
	m_sConnectingDlgTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, HTML_HPRIO_SUBTEMPLATE_NAME_CONNECTING);

	if(m_sQuestionDlgTemplate=="")
	{	
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initSubTemplates:Sub template [%1] for object %2 not defined or the file does not exist!")
					.arg(HTML_HPRIO_SUBTEMPLATE_NAME_QUESTION).arg(sObjectId));
	}
	else if(m_sInfoDlgTemplate=="")
	{	
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initSubTemplates:Sub template [%1] for object %2 not defined or the file does not exist!")
					.arg(HTML_HPRIO_SUBTEMPLATE_NAME_INFO).arg(sObjectId));
	}
	else if(m_sConnectingDlgTemplate=="")
	{	
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initSubTemplates:Sub template [%1] for object %2 not defined or the file does not exist!")
					.arg(HTML_HPRIO_SUBTEMPLATE_NAME_CONNECTING).arg(sObjectId));
	}
	else if(m_sLoginDlgTemplate == "") // this one is the default one and loaded as TEMPLATE attribute of DlgHPrioInputDialog
	{
			m_bHtmlTemplAvailable = false;

			TRACE_WEB_W(QString("MIhmWebVisibleHPrioDialog::initSubTemplates:Error in template file for object %1!")
					.arg(sObjectId));
	}else
		m_bHtmlTemplAvailable = true;

	return m_bHtmlTemplAvailable;
}



bool MIhmWebVisibleHPrioDialog::initialize(enumDialogType eType, 
												QString sTitle, 
												QString sMsg, 
												QString sLogin, 
												QString sBtnOK, 
												QString sBtnCancel)
{
	m_eDlgType = eType;
	

	m_sTitle = sTitle;
	m_sMsg = sMsg;
	m_sLogin =sLogin;
	m_sBtnOK = sBtnOK;
	m_sBtnCancel = sBtnCancel;
	
	m_bReady = true;
	generateTimestamp();

	return m_bReady;
}

void MIhmWebVisibleHPrioDialog::setDialogInProgress(bool bInProgress)
{
	//Since it is possible that the m_bInputInProgress is not in the correct status 
	//for this type of dialog we always do the update to web if this function was used
	m_bInputInProgress = bInProgress;
	generateTimestamp();
}

QString MIhmWebVisibleHPrioDialog::getHtml(bool bReadOnly)
{
	QString sRes;
	QString sItemsList;
	
	if(m_bHtmlTemplAvailable && m_bReady && m_bInputInProgress)
	{	
		//hide the specific part which is not needed 
		//for the required type  of dialog.
		if(m_eDlgType == enuClientLoginDlg)
		{
			sRes = m_sLoginDlgTemplate;

			QString sLoginNameLabel = MIhmConfigWeb::getCfg()->getLanguages()->getLoginNameTranslation(MIhmLanguages::enuTranslTargetWeb);
			sRes.replace(HTML_TEMPLATE_HPRIO_LABEL_LOGIN_VALUE, sLoginNameLabel, Qt::CaseInsensitive);
			
			QString sLoginPwdLabel = MIhmConfigWeb::getCfg()->getLanguages()->getLoginPwdTranslation(MIhmLanguages::enuTranslTargetWeb);
			sRes.replace(HTML_TEMPLATE_HPRIO_LABEL_PWD_VALUE, sLoginPwdLabel, Qt::CaseInsensitive);
			sRes.replace(HTML_TEMPLATE_HPRIO_TEXT_LOGIN_VALUE, m_sLogin, Qt::CaseInsensitive);
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_OK_TEXT, m_sBtnOK, Qt::CaseInsensitive);
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_CANCEL_TEXT, m_sBtnCancel, Qt::CaseInsensitive);
		}
		else if(m_eDlgType == enuTakeOverMsg || m_eDlgType == enuTakeOverInfoMsg )
		{
			sRes = m_sInfoDlgTemplate;
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_OK_TEXT, m_sBtnOK, Qt::CaseInsensitive);
		}
		else if(m_eDlgType == enuClientConnectingMsg)
		{
			sRes = m_sConnectingDlgTemplate;
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_CANCEL_TEXT, m_sBtnCancel, Qt::CaseInsensitive);

			if(MIhmConfigWeb::getCfg()->getShowCancelOnConnectingDlg())
				sRes.replace(HTML_TEMPLATE_HPRIO_BTN_CANCEL_STYLE,"");
			else
				sRes.replace(HTML_TEMPLATE_HPRIO_BTN_CANCEL_STYLE,"visibility: hidden;");
		}
		else if(m_eDlgType == enuTakeOverQuestion)
		{
			sRes = m_sQuestionDlgTemplate;
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_OK_TEXT, m_sBtnOK, Qt::CaseInsensitive);
			sRes.replace(HTML_TEMPLATE_HPRIO_BTN_CANCEL_TEXT, m_sBtnCancel, Qt::CaseInsensitive);
		}
		else
		{
			TRACE_WEB_D(QString("MIhmWebVisibleHPrioDialog::Invalid dialog type %1").arg(m_eDlgType));
			return sRes;
		}

			
		sRes.replace(HTML_TEMPLATE_HPRIO_DIALOG_MESSAGE, m_sMsg, Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_HPRIO_DIALOG_TITLE, m_sTitle, Qt::CaseInsensitive);
		sRes.replace(HTML_TEMPLATE_HPRIO_DIALOG_ID, m_sName, Qt::CaseInsensitive);

	}
	
	if(sRes!="")
	{
		if(MTracer::getWebTracer()->isTrcEnabled())
			TRACE_WEB_D(QString("MIhmWebVisibleHPrioDialog::getHtml:Name:%1, HTML:%2")
							.arg(getName()).arg(sRes));
	}

	return sRes;
}


//-----------------------------------------------------------------
//---------- END WEB HIGH PRIORITY DIALOG  ---------------------------------
//-----------------------------------------------------------------


//--------------------------------------------------------------------
//----- WEB ASYNC INPUT
//---------------------------------------------------------------------
MIhmWebAsyncInput::MIhmWebAsyncInput(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_bReady = false;
}

MIhmWebAsyncInput::~MIhmWebAsyncInput()
{

}


void MIhmWebAsyncInput::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
	{
		m_bHtmlTemplAvailable = false;
		TRACE_WEB_W(QString("MIhmWebAsyncInput::initTemplateHtml:No HTML template for object:%1").arg(m_sName.toLatin1().data()));
	}
	else
		m_bHtmlTemplAvailable = true;

	m_sTemplateHtml = sHtml;
	generateTimestamp();
}

bool MIhmWebAsyncInput::initialize(MInputAsyncExReq * pReq)
{
	m_bReady = true;
	
	m_oAsyncInputExReq = *pReq;
	generateTimestamp();

	return m_bHtmlTemplAvailable;
}	
	

QString MIhmWebAsyncInput::getHtml(bool bReadOnly)
{
	if(m_bHtmlTemplAvailable && m_bReady)
	{	
		if(!m_bHtmlPrepared)
		{
			m_bHtmlPrepared = true;

			QList <MInputExField *> * plstFieldList = m_oAsyncInputExReq.getInputFields();
			m_sResHtml = updateInputExFileds(plstFieldList, m_sTemplateHtml);
		}

		TRACE_WEB_D(QString("MIhmWebAsyncInput::getHtml:Name:%1, HTML:...")
						.arg(getName()));

		return m_sResHtml;
	}
	
	return QString();
}




//--------------------------------------------------------------------
//--- END --- WEB ASYNC INPUT
//---------------------------------------------------------------------




//-----------------------------------------------------------------
//---------- WEB TABLE FILE VIEW  ---------------------------------
//-----------------------------------------------------------------

#define CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELL			 "TableCell"
#define CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELLDATA_IMAGE "TableCellDataImage"
#define CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELLDATA_TEXT	 "TableCellDataText"
#define CFG_VAL_TABLE_FILE_VIEW_TEMPL_ROW			 "TableRow"
#define CFG_VAL_TABLE_FILE_VIEW_TEMPL_TITLE			 "TableTitle"
#define CFG_VAL_TABLE_FILE_VIEW_FORMAT				 "TableFormat"
#define CFG_VAL_TABLE_FILE_VIEW_DEF_MAX_ROWS_ALLOWED	 10


MIhmWebVisibleFileView::MIhmWebVisibleFileView(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_uiLastCommandTimestamp = 0;
	m_iMaxRowsAllowed = CFG_VAL_TABLE_FILE_VIEW_DEF_MAX_ROWS_ALLOWED;
}

MIhmWebVisibleFileView::~MIhmWebVisibleFileView()
{

}

		
void MIhmWebVisibleFileView::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleFileView::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));

	m_sTemplateHtml = sHtml;
	generateTimestamp();


}

void MIhmWebVisibleFileView::setReadOnly(bool bIsReadOnly)
{
	if(m_bReadOnly!=bIsReadOnly)
	{
		m_bReadOnly = bIsReadOnly;
		
		recreateTableHtml();
		generateTimestamp();
	}

}




bool MIhmWebVisibleFileView::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{

	m_sTableCellTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELL);
	m_sTableCellDataImageTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELLDATA_IMAGE);
	m_sTableCellDataTextTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, CFG_VAL_TABLE_FILE_VIEW_TEMPL_CELLDATA_TEXT);

	m_sTableRowTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, CFG_VAL_TABLE_FILE_VIEW_TEMPL_ROW);
	m_sTableTitleTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, CFG_VAL_TABLE_FILE_VIEW_TEMPL_TITLE);
	
	m_sFormatFile = pLaneSettings->getVisObjParamValue(sObjectId, CFG_VISIBLE_OBJ_ATTRIBUTE_FORMAT_FILE);
	
	QString sMaxRows = pLaneSettings->getVisObjParamValue(sObjectId, CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS);
	
	bool bOK;
	int iRows = sMaxRows.toInt(&bOK);
	
	if(bOK)
		m_iMaxRowsAllowed = iRows;

	
	QString sFullPath = MIhmConfigWeb::getCfg()->getCommFileFullPath(m_sFormatFile);
	
	if(loadFormatFile(sFullPath))
	{
		if(m_sTableCellTemplate=="" || 
			m_sTableCellDataImageTemplate=="" ||
				m_sTableCellDataTextTemplate=="" || 
				m_sTableRowTemplate=="" || 
				m_sTemplateHtml == "")
		{
			m_bHtmlTemplAvailable = false;
		}
		else
		{
			m_bHtmlTemplAvailable = true;
			m_bReady = true; //can show as soon the format file is loaded 

		}
	}

	generateTimestamp();

	return m_bHtmlTemplAvailable;
}

bool MIhmWebVisibleFileView::loadFormatFile(QString sFormatFile)
{
	bool bRet = false;
	
	TRACE_WEB_D(QString("MIhmWebVisibleFileView::loadFormatFile: UI from file %1").arg(sFormatFile));
	
	if(sFormatFile.isEmpty())
		return false;
	
	QDomDocument xmlData;

	if(OpenXMLFile(sFormatFile, &xmlData))			
	{
		MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
		MIhmConfigImages * pImages = MIhmConfigWeb::getCfg()->getWebConfigImages();

		QDomElement root = xmlData.documentElement();
		QDomElement elBody = root.firstChildElement(IHM_CFG_VAL_TABLEVIEW_BODY_ELEMENT);

		QString sRowHeight = MHelpFuncs::getAttributeText(&elBody, IHM_CFG_VAL_TABLEVIEW_HEIGHT_ATTRIBUTE);
		bool bOK;
		m_iRowHeight = sRowHeight.toInt(&bOK);
		m_iRowHeight = (bOK)?m_iRowHeight:0;
		
		QString sColWidth,sHeaderHeight, sHeaderResize ,sAlign;
		QDomElement elHeader = root.firstChildElement(IHM_CFG_VAL_TABLEVIEW_HEADER_ELEMENT);
		
		sHeaderHeight = MHelpFuncs::getAttributeText(&elHeader, IHM_CFG_VAL_TABLEVIEW_HEIGHT_ATTRIBUTE);
		m_iHeaderHeight = sHeaderHeight.toInt(&bOK);
		m_iHeaderHeight = (bOK)?m_iHeaderHeight:0;
		
		QDomNodeList lstHeadColumns = elHeader.elementsByTagName(IHM_CFG_VAL_TABLEVIEW_COLUMN_ELEMENT);
		int iWidth;

		QString sHeaderHtmlData, sText, sHtmlEncodedText, sTextToShow;
		QDomNode currColumn;

		for(int i=0;i<lstHeadColumns.count();i++)
		{
			currColumn = lstHeadColumns.at(i);
			sText = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_TEXT_ATTRIBUTE);
			sColWidth = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_WIDTH_ATTRIBUTE);
			sAlign = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ALIGN_ATTRIBUTE);

			pLang->getLabelTranslation(sText, MIhmLanguages::enuTranslTargetNoTransform, sTextToShow);
			sHtmlEncodedText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sTextToShow);
			
			iWidth = sColWidth.toInt(&bOK);
			iWidth = (bOK)?iWidth:0;
			
			sHeaderHtmlData += createHeaderCellHtml(i, lstHeadColumns.count(), sHtmlEncodedText, "");
		}

		m_sFormatedHeaderHtmlData = m_sTableRowTemplate;
		m_sFormatedHeaderHtmlData = m_sFormatedHeaderHtmlData.replace("%data%",sHeaderHtmlData);
		m_sFormatedHeaderHtmlData = m_sFormatedHeaderHtmlData.replace("%class%","");
		m_sFormatedHeaderHtmlData = m_sFormatedHeaderHtmlData.replace("%style%","");

		bRet = true;
	}		
	else
	{
		TRACE_WEB_W( QString("MIhmWebVisibleFileView::loadFormatFile: Error loading %1")
														.arg(sFormatFile));
	}

	return bRet;
}


QString MIhmWebVisibleFileView::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			*psResult = m_sTemplateHtml;

			QString sBodyHtmlData;
			
			if(bReadOnly)
				sBodyHtmlData = m_slstHtmlReadOnlyRows.join(" ");
			else
				sBodyHtmlData = m_slstHtmlRows.join(" ");

			psResult->replace("%id%", getName());
			psResult->replace("%table-title%", "");
			psResult->replace("%style%", "");
			psResult->replace("%header-data%", m_sFormatedHeaderHtmlData);
			psResult->replace("%body-data%", sBodyHtmlData);
		}

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return QString();
}


bool MIhmWebVisibleFileView::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	MCtrlIdAction * p =	findActionForCtrlId(sCtrlId);

	if(p!=NULL)
	{
		sActionID = p->sAction;
		sParams = p->sParams;
		return true;
	}
	else
	{
		sActionID = "";
		sParams = "";
		return false;
	}
}


void MIhmWebVisibleFileView::recreateTableHtml()
{
	generateTimestamp();

}


void MIhmWebVisibleFileView::clearAllHtmlRows()
{
	m_slstHtmlRows.clear();
	m_slstHtmlReadOnlyRows.clear();
}

void MIhmWebVisibleFileView::prependHtmlRows(QStringList *psLstNewRowsToPrepend, QStringList *psLstNewReadOnlyRowsToPrepend)
{
	int iCountAll = m_slstHtmlRows.count()+psLstNewRowsToPrepend->count();
	
	if(iCountAll>m_iMaxRowsAllowed && m_iMaxRowsAllowed!=0)
	{
		if(psLstNewRowsToPrepend->count()>=m_iMaxRowsAllowed)
		{
			m_slstHtmlRows.clear();
			m_slstHtmlReadOnlyRows.clear();

			for(int i = psLstNewRowsToPrepend->count() - m_iMaxRowsAllowed;i<psLstNewRowsToPrepend->count();i++)
			{
				m_slstHtmlRows.prepend(psLstNewRowsToPrepend->at(i));
				m_slstHtmlReadOnlyRows.prepend(psLstNewReadOnlyRowsToPrepend->at(i));
			}
		}
		else
		{
			for(int i=0;i<iCountAll-m_iMaxRowsAllowed;i++)
			{
				m_slstHtmlRows.removeLast();
				m_slstHtmlReadOnlyRows.removeLast();
			}

			for(int i=0;i<psLstNewRowsToPrepend->count();i++)
			{
				m_slstHtmlRows.prepend(psLstNewRowsToPrepend->at(i));
				m_slstHtmlReadOnlyRows.prepend(psLstNewReadOnlyRowsToPrepend->at(i));
			}
		}
	}
	else
	{
			for(int i=0;i<psLstNewRowsToPrepend->count();i++)
			{
				m_slstHtmlRows.prepend(psLstNewRowsToPrepend->at(i));
				m_slstHtmlReadOnlyRows.prepend(psLstNewReadOnlyRowsToPrepend->at(i));
			}
	}
}

void MIhmWebVisibleFileView::appendHtmlRows(QStringList *psLstNewRowsToAppend, QStringList *psLstNewReadOnlyRowsToAppend)
{
	int iCountAll = m_slstHtmlRows.count()+psLstNewRowsToAppend->count();
	
	if(iCountAll>m_iMaxRowsAllowed  && m_iMaxRowsAllowed!=0)
	{
		if(psLstNewRowsToAppend->count()>=m_iMaxRowsAllowed)
		{
			m_slstHtmlRows.clear();
			m_slstHtmlReadOnlyRows.clear();

			for(int i=0;i<m_iMaxRowsAllowed;i++)
			{
				m_slstHtmlRows.append(psLstNewRowsToAppend->at(i));
				m_slstHtmlReadOnlyRows.append(psLstNewReadOnlyRowsToAppend->at(i));
			}
		}
		else
		{
			for(int i=0;i<iCountAll-m_iMaxRowsAllowed;i++)
			{
				psLstNewRowsToAppend->removeLast();
				psLstNewReadOnlyRowsToAppend->removeLast();
			}

			for(int i=0;i<psLstNewRowsToAppend->count();i++)
			{
				m_slstHtmlRows.append(psLstNewRowsToAppend->at(i));
				m_slstHtmlReadOnlyRows.append(psLstNewReadOnlyRowsToAppend->at(i));
			}
		}
	}
	else
	{
			for(int i=0;i<psLstNewRowsToAppend->count();i++)
			{
				m_slstHtmlRows.append(psLstNewRowsToAppend->at(i));
				m_slstHtmlReadOnlyRows.append(psLstNewReadOnlyRowsToAppend->at(i));
			}
	}	
}



bool MIhmWebVisibleFileView::executeRowCommand(QList <MTableRow*> *plstNewRows, enum_aff_table_command eCmd, quint64 uiCommandTimestamp)
{
	bool bRetVal = false;

	generateTimestamp();

	if(m_bHtmlTemplAvailable)
	{
		m_uiLastCommandTimestamp = uiCommandTimestamp;
		
		if(eCmd == enuIHM_AFF_TABLE_CMD_RESET_ROWS)
		{
			clearAllHtmlRows();
			resetCtrlIdActionMappping();

			bRetVal = true;
		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_PREPEND_ROWS)
		{
			QStringList sLstNewRowsToPrepend, sLstNewReadOnlyRowsToPrepend;
			bRetVal = generateHtmlDataForRows(plstNewRows , &sLstNewRowsToPrepend,  &sLstNewReadOnlyRowsToPrepend);
			prependHtmlRows(&sLstNewRowsToPrepend,  &sLstNewReadOnlyRowsToPrepend);
			
		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_APPEND_ROWS)
		{
			QStringList sLstNewRowsToAppend, sLstNewReadOnlyRowsToAppend;
			bRetVal = generateHtmlDataForRows(plstNewRows , &sLstNewRowsToAppend,  &sLstNewReadOnlyRowsToAppend);
			appendHtmlRows(&sLstNewRowsToAppend,  &sLstNewReadOnlyRowsToAppend);
		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS)
		{
			clearAllHtmlRows();
			resetCtrlIdActionMappping();

			QStringList sLstNewRowsToAppend, sLstNewReadOnlyRowsToAppend;
			bRetVal = generateHtmlDataForRows(plstNewRows , &sLstNewRowsToAppend,  &sLstNewReadOnlyRowsToAppend);
			appendHtmlRows(&sLstNewRowsToAppend,  &sLstNewReadOnlyRowsToAppend);
		}
	}
	
	return false;	
}

bool MIhmWebVisibleFileView::generateHtmlDataForRows(QList <MTableRow*> *plstNewRows, QStringList *psLstNewRows,  QStringList *psLstNewReadOnlyRows)
{
	bool bRet = false;
	
	if(plstNewRows==NULL)
				return false;
	
	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();

	QString sRowHeight, sTextAlign,sIconPath;
	QString sFontWeightBold,sToolTip,sHtmlEncodedToolTip, sTextToShow,sHtmlEncodedText, sTextDecoration, sStyle;
	MTableRow* pCurrentRow;
	MTableCell *pCurrentCell;

	for(int i=0;i<plstNewRows->count();i++)
	{
		pCurrentRow = plstNewRows->at(i);

		QString sBodyRowHtmlData = "";
		QString sBodyRowReadOnlyHtmlData = "";
		
		for(int j=0;j<pCurrentRow->m_lstColumns.count();j++)
		{
			pCurrentCell = pCurrentRow->m_lstColumns.at(j);

			//pCurrentCell->m_iTextAlignment; //TO DO

			if(pCurrentCell->m_bFontBold)
				sStyle = "font-weight:bold;";
			else
				sStyle="";	

			if(pCurrentCell->m_bFontStrikeOut)
				sStyle += QString("text-decoration:line-through;");

			pLang->getLabelTranslation(pCurrentCell->m_sText, MIhmLanguages::enuTranslTargetWeb, sTextToShow);
			sHtmlEncodedText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sTextToShow);
			
			QString sNewItemID = generateNewItemID();
			//append new click action (for the in control case)
			addCtrlIdActionMapping(sNewItemID, pCurrentCell->m_sAction, pCurrentCell->m_sActionParam);
			
			pLang->getLabelTranslation(pCurrentCell->m_sTooltip, MIhmLanguages::enuTranslTargetWeb, sToolTip);
			sHtmlEncodedToolTip = MIhmConfigWeb::getCfg()->htmlEncodeStr(sToolTip);
	

			sBodyRowHtmlData +=			 createCellHtml(j,pCurrentRow->m_lstColumns.count(),sHtmlEncodedText, sHtmlEncodedToolTip, sIconPath, true, sNewItemID, sStyle);
			sBodyRowReadOnlyHtmlData  += createCellHtml(j,pCurrentRow->m_lstColumns.count(),sHtmlEncodedText, sHtmlEncodedToolTip, sIconPath, false, sNewItemID, sStyle);
		}
		
		QString sBodyRowHtml = m_sTableRowTemplate;
		
		//implemented on client side
// 			QString sClass;
// 			sClass = (i%2==0)?"":HTML_CLASS_ALT_ROW;
		sBodyRowHtml = sBodyRowHtml.replace("%class%","");
		sBodyRowHtml = sBodyRowHtml.replace("%style%","");


		QString sBodyRowReadOnlyHtml = sBodyRowHtml;

		sBodyRowHtml = sBodyRowHtml.replace("%data%",sBodyRowHtmlData);
		sBodyRowReadOnlyHtml = sBodyRowReadOnlyHtml.replace("%data%",sBodyRowReadOnlyHtmlData);

		psLstNewRows->append(sBodyRowHtml);
		psLstNewReadOnlyRows->append(sBodyRowReadOnlyHtml);
	}

	return true;
}


QString MIhmWebVisibleFileView::createTitleHtml(QString sTitle)
{
	QString sRez;

	if(sTitle!="")
	{
		sRez = m_sTableTitleTemplate;
		sRez = sRez.replace("%text%",sTitle);
	}

	return sRez;
}

QString MIhmWebVisibleFileView::createHeaderCellHtml(int iCol, int iColNum, QString sText, QString sStyle)
{
	QString sRez;
	QString sCellHtmlData;

	if(sText!="")
	{
		QString sCellTextHtml = m_sTableCellDataTextTemplate;
		

		sCellTextHtml = sCellTextHtml.replace("%class%","");
		sCellTextHtml = sCellTextHtml.replace("%style%",sStyle);
		sCellTextHtml = sCellTextHtml.replace("%text%",sText);
		sCellTextHtml = sCellTextHtml.replace("%id%","");
		sCellHtmlData = sCellTextHtml;
	}

	QString sClass;

	if(iCol+1<iColNum)
		sClass = QString("column-%1").arg(iCol+1);
	else
		sClass = QString("last-column");

	sRez = m_sTableCellTemplate;
	sRez = sRez.replace("%tooltip%","");
	sRez = sRez.replace("%class%",sClass);
	sRez = sRez.replace("%data%",sCellHtmlData);
	sRez = sRez.replace("%style%","");
	return sRez;
}

QString MIhmWebVisibleFileView::createCellHtml(int iCol, 
											   int iColNum, 
											   QString sText,
											   QString sToolTip,
											   QString sIconPath,
											   bool bClickable,
											   QString sId,
											   QString sStyle)
{
	QString sRez;
	QString sCellHtmlData;
	QString sClass = bClickable?HTML_CLASS_CLICKABLE:"";
	
	if(!sIconPath.isEmpty())
	{
		QString sCellImageHtml = m_sTableCellDataImageTemplate;
		sCellImageHtml = sCellImageHtml.replace("%images-root%/%image-name%",sIconPath);
		sCellImageHtml = sCellImageHtml.replace("%class%",sClass);
		sCellImageHtml = sCellImageHtml.replace("%id%",sId);
		sCellImageHtml = sCellImageHtml.replace("%text%","");

		sCellHtmlData += sCellImageHtml;
	}

	if(!sText.isEmpty())
	{
		QString sCellTextHtml = m_sTableCellDataTextTemplate;
		sCellTextHtml = sCellTextHtml.replace("%class%",sClass);
		sCellTextHtml = sCellTextHtml.replace("%text%",sText);
		sCellTextHtml = sCellTextHtml.replace("%id%",sId);

		//todo prilipit font iz data filea
		sCellTextHtml = sCellTextHtml.replace("%style%",sStyle); 
		sCellHtmlData += sCellTextHtml;
	}


	if(iCol+1<iColNum)
		sClass = QString("column-%1").arg(iCol+1);
	else
		sClass = QString("last-column");

	sRez = m_sTableCellTemplate;
	sRez = sRez.replace("%tooltip%",sToolTip);
	sRez = sRez.replace("%class%",sClass);
	sRez = sRez.replace("%data%",sCellHtmlData);
	sRez = sRez.replace("%style%","");

	return sRez;
}

void MIhmWebVisibleFileView::setVisible(bool bVisible)
{
	if(m_bVisible!=bVisible)
	{
		m_bVisible = bVisible;	
		generateTimestamp();
	}
}


bool MIhmWebVisibleFileView::OpenXMLBuffer(QString sXmlData, QDomDocument *pXmlData)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	
	bRet = pXmlData->setContent(sXmlData, true, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{
		QDomElement root = pXmlData->documentElement();
		if(root.tagName() != IHM_CFG_VAL_TABLEVIEW_ROOT_ELEMENT)
		{
			TRACE_WEB_W(QString( "MIhmWebVisibleFileView::OpenXMLBuffer: Invalid root element!"));
			bRet = false;
		}
	}
	else
	{
		QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_WEB_W(QString("MIhmWebVisibleFileView::OpenXMLBuffer: %1").arg(sMsg.toLatin1().data()));
	}

	return bRet;
}

bool MIhmWebVisibleFileView::OpenXMLFile(QString sDataFile, QDomDocument *pXmlData)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;
	QFile file(sDataFile);
	
	if(file.open(QFile::ReadOnly))
	{

		bRet = pXmlData->setContent((QIODevice*)&file, true, &errorStr, &errorLine, &errorColumn);

		if(bRet)
		{
			QDomElement root = pXmlData->documentElement();
			if(root.tagName() != IHM_CFG_VAL_TABLEVIEW_ROOT_ELEMENT)
			{
				TRACE_WEB_W( "MIhmWebVisibleFileView::OpenXMLFile: Invalid root element!");
				bRet = false;
			}
		}
		else
		{
			QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
			TRACE_WEB_W( QString("MIhmWebVisibleFileView::OpenXMLFile: %1").arg(sMsg) );
		}
	}
	else
	{
		TRACE_WEB_W( QString("MIhmWebVisibleFileView::OpenXMLFile: Unable to open file %1").arg(sDataFile));
	}
	return bRet;
}



//-----------------------------------------------------------------
#define REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM	"MenuItem"
#define REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM_SUBMENU	"MenuItemSubmenu"


MIhmWebVisibleHMenuView::MIhmWebVisibleHMenuView(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{ 
	m_bHtmlTemplAvailable = false;
	m_pMenuFile = new MHMenuFileLoader();
	m_bInitialyLoaded = false;

}

MIhmWebVisibleHMenuView::~MIhmWebVisibleHMenuView()
{
	delete m_pMenuFile;
}

		
void MIhmWebVisibleHMenuView::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebVisibleHMenuView::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));

	m_sTemplateHtml = sHtml;
	generateTimestamp();


}

QString MIhmWebVisibleHMenuView::getHtml(bool bReadOnly)
{
	QString sRes;
	QString sItemsList;
	
	if(m_bReady)
	{	
		TRACE_WEB_D(QString("MIhmWebVisibleHMenuView::getHtml:Name:%1, HTML:...")
						.arg(getName()));
		
		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return sRes;
}

bool MIhmWebVisibleHMenuView::updateMenu(QString sMenuContent, bool bUseExternalFile, bool bDoNotReloadMenu)
{
	//call this before loading menus because the internal clickable object IDs are created using this timestamp
	generateTimestamp();

	m_bReady = false;

	if(m_bHtmlTemplAvailable)
	{
		bool bOK = false;
		bool bReload = !bDoNotReloadMenu;
		
		if(!m_bInitialyLoaded)
			bReload = true;

		if(bUseExternalFile)
		{
			QString sMenuFile = MIhmConfigWeb::getCfg()->getCommFileFullPath(sMenuContent);
			bOK = m_pMenuFile->loadDataFromFile(sMenuFile, bReload);	
		}
		else
		{
			bOK = m_pMenuFile->loadDataFromBuffer(sMenuContent, bReload);
		}

		if(bOK)
		{
			if(!m_bInitialyLoaded)
				m_bInitialyLoaded = true;

			m_bReady = recreateMenuHtml();
		}
		else
		{
			TRACE_WEB_W(QString("MIhmWebVisibleHMenuView::updateMenu: Failed for: Menu:%1, Reload:%2")
						.arg(sMenuContent).arg(bReload));
			TRACE_WEB_W(QString("\t %1").arg(m_pMenuFile->getLastErrorMessage()))
		}
	}

	return m_bReady;
}

void MIhmWebVisibleHMenuView::setReadOnly(bool bIsReadOnly)
{
	if(m_bReadOnly!=bIsReadOnly)
	{
		m_bReadOnly = bIsReadOnly;
		m_bReady = recreateMenuHtml();
		generateTimestamp();
	}
}

bool MIhmWebVisibleHMenuView::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	m_sMenuItemTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM);
	
	if(m_sMenuItemTemplate=="")
		TRACE_WEB_W(QString("MIhmWebVisibleHMenuView::initSubTemplates:No HTML template %1 for object:%2")
															.arg(REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM)									
															.arg(m_sName.toLatin1().data()));

	
	m_sMenuItemSubmenuTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM_SUBMENU);
	if(m_sMenuItemSubmenuTemplate=="")
		TRACE_WEB_W(QString("MIhmWebVisibleHMenuView::initSubTemplates:No HTML template %1 for object:%2")
															.arg(REG_VAL_H_MENU_VIEW_TEMPL_MENU_ITEM_SUBMENU)									
															.arg(m_sName.toLatin1().data()));


	if(m_sMenuItemTemplate=="" || 
		m_sMenuItemSubmenuTemplate=="" ||
			m_sTemplateHtml == "")
	{
		m_bHtmlTemplAvailable = false;
	}
	else
		m_bHtmlTemplAvailable = true;

	generateTimestamp();

	return m_bHtmlTemplAvailable;
}


void MIhmWebVisibleHMenuView::setVisible(bool bVisible)
{
	if(m_bVisible!=bVisible)
	{
		m_bVisible = bVisible;	
		generateTimestamp();
	}
}
		
bool MIhmWebVisibleHMenuView::recreateMenuHtml()
{
	bool bRet = false;
	
	TRACE_WEB_D(QString("MIhmWebVisibleHMenuView::recreateMenuHtml: ...(from model)..."));

	resetCtrlIdActionMappping();

	if(m_bHtmlTemplAvailable)
	{
		m_sResHtml = m_sTemplateHtml;
		m_sResHtml.replace("%id%", m_sName);
		
		m_sReadOnlyResHtml = m_sResHtml;

		if(m_bVisible)
			m_sMenuBody = createMenuBody(m_pMenuFile->getLoadedModel());
		else
			m_sMenuBody = "";

		m_sResHtml.replace("%menu-body%", m_sMenuBody);
		m_sReadOnlyResHtml.replace("%menu-body%", "");

		bRet = true;
	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebVisibleHMenuView::loadDataFile: Object not initialized!"));
		m_sResHtml = "";
		m_sReadOnlyResHtml = "";
	}

	return bRet;
}

bool MIhmWebVisibleHMenuView::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	MCtrlIdAction * p =	findActionForCtrlId(sCtrlId);

	if(p!=NULL)
	{
		sActionID = p->sAction;
		sParams = p->sParams;
		return true;
	}
	else
	{
		sActionID = "";
		sParams = "";
		return false;
	}
}


QString MIhmWebVisibleHMenuView::createMenuBody(QList <MMenuModelItem*> *pLstItems)
{
	QListIterator<MMenuModelItem*> it(*pLstItems);

	QString sCurrentMenuLevel;

	MMenuModelItem * pItem;
	it.toFront();
	while (it.hasNext())
	{
		pItem = it.next();
		
		MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
		
		if(pItem->m_eType == MMenuModelItem::enumMenuItemType::enuMenu)
		{
			QString sNewSubMenu = m_sMenuItemTemplate;

			QString sNewItemID = generateNewItemID();
			sNewSubMenu.replace("%item-id%", sNewItemID);

			//Translate text and encode for HTML
			QString sTextToShow;
			pLang->getLabelTranslation(pItem->m_sText, MIhmLanguages::enuTranslTargetNoTransform, sTextToShow);
			QString sHtmlEncoded = MIhmConfigWeb::getCfg()->htmlEncodeStr(sTextToShow);
			QString sClass = pItem->m_bEnabled?"":HTML_CLASS_DISABLED;

			sNewSubMenu.replace("%item-text%", sHtmlEncoded);
			sNewSubMenu.replace("%class%", sClass);

			QString sSubMenuContent = createMenuBody(pItem->getChildItemsList());
			QString sSubMenuBody = m_sMenuItemSubmenuTemplate;
			sSubMenuBody.replace("%submenu-body%",sSubMenuContent);

			sNewSubMenu.replace("%item-submenu%", sSubMenuBody);

			sCurrentMenuLevel += sNewSubMenu;
		}
		else
		{
			QString sNewMenuItem = m_sMenuItemTemplate;

			QString sNewItemID = generateNewItemID();
			//append new click action
			addCtrlIdActionMapping(sNewItemID, pItem->m_sActionType, pItem->m_sActionParams);

			sNewMenuItem.replace("%item-id%", sNewItemID);

			QString sTextToShow;
			pLang->getLabelTranslation(pItem->m_sText,MIhmLanguages::enuTranslTargetNoTransform, sTextToShow);
			QString sHtmlEncoded = MIhmConfigWeb::getCfg()->htmlEncodeStr(sTextToShow);

			QString sClass;

			if(pItem->m_bEnabled)
				sClass = HTML_CLASS_CLICKABLE;
			else
				sClass = HTML_CLASS_DISABLED;
		
			sNewMenuItem.replace("%item-text%", sHtmlEncoded);
			sNewMenuItem.replace("%class%", sClass);
			
			sNewMenuItem.replace("%item-submenu%", "");


			sCurrentMenuLevel += sNewMenuItem;
		}
	}

	return sCurrentMenuLevel;
}




//-----------------------------------------------------------------


MIhmWebTabView::MIhmWebTabView(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;

}

MIhmWebTabView::~MIhmWebTabView()
{

}

void MIhmWebTabView::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
		TRACE_WEB_W(QString("MIhmWebTabView::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));

	m_sTemplateHtml = sHtml;
	generateTimestamp();
}

QString MIhmWebTabView::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{
			QString sOutputTabs;

			if(!bReadOnly)
			{
				QString sCurrentTabHtml;
				QString sCurrentTabText;

				MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
				resetCtrlIdActionMappping();

				for(int i = 0;i< m_lstTabText.size(); i++)
				{
					sCurrentTabHtml = m_sTabTemplate;
					sCurrentTabText = m_lstTabText.at(i);

					//Translate text and encode for HTML
					QString sTextToShow;
					pLang->getLabelTranslation(sCurrentTabText, MIhmLanguages::enuTranslTargetWeb, sTextToShow);

					QString sNewItemID = generateNewItemID();
					//append new click action
 					addCtrlIdActionMapping(sNewItemID, m_sActionId, QString("%1").arg(i));

					sCurrentTabHtml.replace("%id%", sNewItemID);
					sCurrentTabHtml.replace("%text%", sTextToShow);
					sCurrentTabHtml.replace("%class%", HTML_CLASS_CLICKABLE);

					if(i==0)
						sCurrentTabHtml.replace("%first%", HTML_TAB_CLASS_FIRST);
					else
						sCurrentTabHtml.replace("%first%","");

					if(i == m_iCurrentIndex)
						sCurrentTabHtml.replace("%active%", HTML_TAB_CLASS_ACTIVE);
					else
						sCurrentTabHtml.replace("%active%","");

					
					sOutputTabs.append(sCurrentTabHtml);
				}	
			}
			else
			{
				sOutputTabs = "";
			}

			*psResult = m_sTemplateHtml;
			psResult->replace("%id%", m_sName);
			psResult->replace("%class%", "");
			psResult->replace("%style%", "");
			psResult->replace("%tabs%", sOutputTabs);
		}

		TRACE_WEB_D(QString("MIhmWebTabView::getHtml:Name:%1, HTML:...")
						.arg(getName()));

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return QString();

}


#define SUB_TEMPLATE_TAB_VIEW	"TAB"

bool MIhmWebTabView::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	m_sTabTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, SUB_TEMPLATE_TAB_VIEW);

	generateTimestamp();
	
	if(!m_sTabTemplate.isEmpty())
		m_bHtmlTemplAvailable = true;
	else
	{
		TRACE_WEB_W(QString("MIhmWebTabView::initSubTemplates:Error in sub template files for object %1!")
					.arg(sObjectId));
	}
	return m_bHtmlTemplAvailable;

}


void MIhmWebTabView::updateTab(QStringList& lstTabText, int iCurrentIndex)
{
	m_bReady = true;
	m_lstTabText = lstTabText;
	m_iCurrentIndex = iCurrentIndex;
	generateTimestamp();

}


bool MIhmWebTabView::getActionForCtrlId(QString sCtrlId, QString &sActionID, QString &sParams)
{
	MCtrlIdAction * p =	findActionForCtrlId(sCtrlId);
	bool bRetVal = false;

	sActionID = "";
	sParams = "";

	if(p!=NULL)
	{
		bool bOK;
		int iIndex = p->sParams.toInt(&bOK);

		//keep action empty if the current clicked index is the same as the current index
		//the empty action will not be emited
		if(bOK && iIndex!=m_iCurrentIndex)
		{
			sActionID = p->sAction;
			sParams = p->sParams;
		}

		bRetVal = true; 
	}

	
	return bRetVal;
}


//------------------------------------------------------------------


MIhmWebRVideoView::MIhmWebRVideoView(enumWebVisibleObjectId eId, MIhmVirtualObject::enumVirtualObjectId eMyVirtObjId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;
	m_eMyVirtObjId = eMyVirtObjId;
}


MIhmWebRVideoView::~MIhmWebRVideoView()
{

}

void MIhmWebRVideoView::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
	{
		TRACE_WEB_W(QString("MIhmWebRVideoView::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	}
	else
	{
		m_bHtmlTemplAvailable = true;
		m_sTemplateHtml = sHtml;
		generateTimestamp();
	}
}

QString MIhmWebRVideoView::getHtml(bool bReadOnly)
{
	QString *psResult = NULL;

	if(m_bHtmlTemplAvailable)
	{	
		if(bReadOnly)
		{
			if(!m_bReadOnlyHtmlPrepared)
			{
				psResult = &m_sReadOnlyResHtml;
				m_bReadOnlyHtmlPrepared = true;
			}
		}
		else
		{
			if(!m_bHtmlPrepared)
			{
				psResult = &m_sResHtml;
				m_bHtmlPrepared = true;
			}
		}
		
		if(psResult!=NULL)
		{	
			*psResult = m_sTemplateHtml;

			psResult->replace("%id%", m_sName);
			psResult->replace("%style%", "");
			
			if(!bReadOnly)
			{
				QString sClass;

				if(!isVisible())
					sClass = HTML_CLASS_HIDDEN;
				
				psResult->replace("%class%", sClass, Qt::CaseInsensitive);
				psResult->replace("%restream-video-src%", QUrl::toPercentEncoding(m_sStreamSourceUrl));	
				psResult->replace("%restream-video-base-url%", QUrl::toPercentEncoding(m_sBaseUrl));	
			}
			else
			{
				psResult->replace("%class%", "", Qt::CaseInsensitive);
				psResult->replace("%restream-video-src%", "");	
				psResult->replace("%restream-video-base-url%", "");	
			}
		}

		TRACE_WEB_D(QString("MIhmWebRVideoView::getHtml:Name:%1, HTML:...")
						.arg(getName()));

		if(bReadOnly)
			return m_sReadOnlyResHtml;
		else
			return m_sResHtml;
	}
	
	return QString();


}

void MIhmWebRVideoView::updateVideo(QString sStreamSourceUrl, QString sBaseUrl)
{
	if(m_sStreamSourceUrl!=sStreamSourceUrl)
	{
		m_bReady = true;
		m_sStreamSourceUrl = sStreamSourceUrl;
		//m_sSourceUrlWithSessionID.append("&RESSClientID=CLI%1").
		//			arg(MIhmVirtualObject::getNameForId(m_eMyVirtObjId));

		m_sBaseUrl = sBaseUrl;
		generateTimestamp();
	}
}


void MIhmWebRVideoView::setVisible(bool bVisible)
{
	if(m_bVisible!=bVisible)
	{
		m_bVisible = bVisible;

		if(!m_sStreamSourceUrl.isEmpty())
					m_bReady = true;
			
		generateTimestamp();
	}
}


//--------------------------------------------------------------------

MIhmWebGenericScript::MIhmWebGenericScript(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;
}


MIhmWebGenericScript::~MIhmWebGenericScript()
{

}

void MIhmWebGenericScript::initTemplateHtml(QString sHtml)
{
	if(sHtml=="")
	{
		TRACE_WEB_W(QString("MIhmWebGenericScript::initTemplateHtml:No HTML template for object:%1")
											.arg(m_sName.toLatin1().data()));
	}
	else
	{
		m_bHtmlTemplAvailable = true;
		m_sTemplateHtml = sHtml;
		m_sResHtml = m_sTemplateHtml;

		generateTimestamp();
	}
}


QString MIhmWebGenericScript::getHtml(bool bReadOnly)
{
	return m_sResHtml;
}

void MIhmWebGenericScript::updateParameter(QString sParamName, QString sParamValue)
{
	m_bReady = true;
	m_sResHtml.replace(sParamName, sParamValue, Qt::CaseInsensitive);
	generateTimestamp();
}

void MIhmWebGenericScript::resetParameters()
{
	m_bReady = true;
	m_sResHtml = m_sTemplateHtml;
	generateTimestamp();

}


//--------------------------------------------------------------------


MIhmWebGenericCtrlDependentScript::MIhmWebGenericCtrlDependentScript(enumWebVisibleObjectId eId):
	MIhmWebVisibleObject(eId)
{
	m_bHtmlTemplAvailable = false;
}


MIhmWebGenericCtrlDependentScript::~MIhmWebGenericCtrlDependentScript()
{

}

void MIhmWebGenericCtrlDependentScript::initTemplateHtml(QString sHtml)
{
	//object main template is not used for this type of object since the script 
	//execution depend on current mode
}

#define SUB_TEMPLATE_ON_GOT_CTRL_SCRIPT		"ON_GOT_CTRL_SCRIPT"
#define SUB_TEMPLATE_ON_LOST_CTRL_SCRIPT	"ON_LOST_CTRL_SCRIPT"


bool MIhmWebGenericCtrlDependentScript::initSubTemplates(MIhmLaneTypeSettings *pLaneSettings, QString sObjectId)
{
	m_sOnGotCtrlScriptTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, SUB_TEMPLATE_ON_GOT_CTRL_SCRIPT);
	m_sOnLostCtrlScriptTemplate = pLaneSettings->getVisObjHtmlSubTemplate(sObjectId, SUB_TEMPLATE_ON_LOST_CTRL_SCRIPT);

	generateTimestamp();
	
	if(m_sOnGotCtrlScriptTemplate.isEmpty() && m_sOnLostCtrlScriptTemplate.isEmpty())
	{
		TRACE_WEB_W(QString("MIhmWebGenericCtrlDependentScript::initSubTemplates:Error in sub template files for object %1!")
					.arg(sObjectId));
	}

	m_bHtmlTemplAvailable = true;

	resetParameters();
	return m_bHtmlTemplAvailable;
}



QString MIhmWebGenericCtrlDependentScript::getHtml(bool bReadOnly)
{
	if(bReadOnly)
		return m_sReadOnlyResHtml;
	else
		return m_sResHtml;
}

void MIhmWebGenericCtrlDependentScript::updateStaticParemeters(MParamList *pParamList)
{
	MParam * pCurrent = NULL;

	for(int i=0; i< pParamList->getCount(); i++)
	{
		pCurrent = pParamList->at(i);

		if(pCurrent!=NULL)
		{
			updateParameter(pCurrent->param_name, pCurrent->param_value);
		}
	}
}



void MIhmWebGenericCtrlDependentScript::updateParameter(QString sParamName, QString sParamValue)
{
	m_bReady = true;
	QString sNameToReplace = QString("%%1%").arg(sParamName);
	m_sResHtml.replace(sNameToReplace, sParamValue, Qt::CaseInsensitive);
	m_sReadOnlyResHtml.replace(sNameToReplace, sParamValue, Qt::CaseInsensitive);

	generateTimestamp();
}

void MIhmWebGenericCtrlDependentScript::resetParameters()
{
	m_bReady = true;
	m_sResHtml = m_sOnGotCtrlScriptTemplate;
	m_sReadOnlyResHtml = m_sOnLostCtrlScriptTemplate;

	generateTimestamp();

}
