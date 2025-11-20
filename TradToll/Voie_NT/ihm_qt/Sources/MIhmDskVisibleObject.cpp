

#include "MIhmDskVisibleObject.h"
#include "MHelpFuncs.h"
#include "MHMenuView.h"
#include "MIhmConfigGeneral.h"
#include "MRVideoView.h"
#include "MButtonLabel.h"
#include "MTabView.h"
//#include "MIhmTaskBar.h"
#include "MListWidget.h"
#include "MTracer.h"
#include "MFormMain.h"
#include "MIhmVirtualObject.h"

#include <QMovie>
#include <QLabel>
#include <QPixmap>
#include <QBoxLayout>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QStringList>
#include <QListWidget>
#include <QCursor>
#include <QPixmapCache>


extern "C" {
	#include <run.H>
	#include <ihm.H>
	#include <reg.h>
};

#define IHM_REG_SEPARATOR ","


MIhmDskVisibleObject * MIhmDskVisibleObject::findVisibleObjectByWidget(QVector <MIhmDskVisibleObject*> *pObjVector, QWidget *pWdg)
{
	for (int i = enuIhmVisibleObjUnknown+1; i < pObjVector->size(); ++i) 
	{
		if(pObjVector->at(i)!=NULL)
			if(pObjVector->at(i)->getClickableWidget() == pWdg)
			{
				return pObjVector->at(i);
			}
	}

	return NULL;	
}



QString MIhmDskVisibleObject::getVisibleObjNameForID(enumVisibleObjectId eId)
{

	switch(eId)
	{
		case enuTEXT_LABEL_CLASS:
				return LABEL_REG_VAL_CLASS;
			break;

		case enuTEXT_LABEL_CLASS_TITLE:
					return LABEL_REG_VAL_CLASS_TITLE;
				break;
		case enuTEXT_LABEL_FARE:
					return LABEL_REG_VAL_FARE;
				break;	
		case enuTEXT_LABEL_FARE_TITLE:
					return LABEL_REG_VAL_FARE_TITLE;
				break;

		case enuTEXT_LABEL_ADD_REVENUE:
					return LABEL_REG_VAL_ADD_REVENUE;
				break;
		case enuTEXT_LABEL_ADD_REVENUE_TITLE:
					return LABEL_REG_VAL_ADD_REVENUE_TITLE;
				break;

		case enuTEXT_LABEL_AXLES:
					return LABEL_REG_VAL_AXLES;
				break;
		case enuTEXT_LABEL_AXLES_TITLE:
					return LABEL_REG_VAL_AXLES_TITLE;
				break;
		
		case enuTEXT_LABEL_TYPE:
					return LABEL_REG_VAL_TYPE;
				break;
		case enuTEXT_LABEL_TYPE_TITLE:
					return LABEL_REG_VAL_TYPE_TITLE;
				break;

		case enuTEXT_LABEL_SALE:
					return LABEL_REG_VAL_SALE;
				break;

		case enuTEXT_LABEL_SALE_TITLE:
					return LABEL_REG_VAL_SALE_TITLE;
				break;

		case enuTEXT_LABEL_SALEDUE:
					return LABEL_REG_VAL_SALEDUE;
				break;

		case enuTEXT_LABEL_SALEDUE_TITLE:
					return LABEL_REG_VAL_SALEDUE_TITLE;
				break;
		
		case enuTEXT_LABEL_SALEDUE_PAID:
					return LABEL_REG_VAL_SALEDUE_PAID;
				break;

		case enuTEXT_LABEL_BALANCE:
					return LABEL_REG_VAL_BALANCE;
				break;
		case enuTEXT_LABEL_BALANCE_TITLE:
					return LABEL_REG_VAL_BALANCE_TITLE;
				break;
		case enuTEXT_LABEL_ENTRY:
					return LABEL_REG_VAL_ENTRY;
				break;

		case enuTEXT_LABEL_ENTRY_TITLE:
					return LABEL_REG_VAL_ENTRY_TITLE;
				break;

		case enuTEXT_LABEL_TRANSACTIONNBR:
					return LABEL_REG_VAL_TRANSACTIONNBR;
				break;

		case enuTEXT_LABEL_TRANSACTIONNBR_TITLE:
					return LABEL_REG_VAL_TRANSACTIONNBR_TITLE;
				break;

		case enuTEXT_LABEL_RECEIPTNBR:
					return LABEL_REG_VAL_RECEIPTNBR;
				break;

		case enuTEXT_LABEL_RECEIPTNBR_TITLE:
					return LABEL_REG_VAL_RECEIPTNBR_TITLE;
				break;

		case enuTEXT_LABEL_COL_ID:
					return LABEL_REG_VAL_COL_ID;
				break;

		case enuTEXT_LABEL_COL_NAME:
					return LABEL_REG_VAL_COL_NAME;
				break;

		case enuTEXT_LABEL_PLAZA_NAME:
					return LABEL_REG_VAL_PLAZA_NAME;
				break;
		case enuTEXT_LABEL_PLAZA_NAME1:
					return LABEL_REG_VAL_PLAZA_NAME1;
				break;
		case enuTEXT_LABEL_PLAZA_NAME2:
					return LABEL_REG_VAL_PLAZA_NAME2;
				break;
		case enuTEXT_LABEL_PLAZA_NAME3:
					return LABEL_REG_VAL_PLAZA_NAME3;
				break;
		case enuTEXT_LABEL_PLAZA_NAME_TITLE:
					return LABEL_REG_VAL_PLAZA_NAME_TITLE;
				break;

		case enuTEXT_LABEL_LANE_NAME_TITLE:
					return LABEL_REG_VAL_LANE_NAME_TITLE;
				break;

		case enuTEXT_LABEL_LANE_NAME:
					return LABEL_REG_VAL_LANE_NAME;
				break;
		case enuTEXT_LABEL_LANE_NAME1:
					return LABEL_REG_VAL_LANE_NAME1;
				break;
		case enuTEXT_LABEL_LANE_NAME2:
					return LABEL_REG_VAL_LANE_NAME2;
				break;
		case enuTEXT_LABEL_PLAZA_NUMBER_TITLE:
					return LABEL_REG_VAL_PLAZA_NUMBER_TITLE;
				break;

		case enuTEXT_LABEL_PLAZA_NUMBER:
					return LABEL_REG_VAL_PLAZA_NUMBER;
				break;
		case enuTEXT_LABEL_LANE_NUMBER_TITLE:
					return LABEL_REG_VAL_LANE_NUMBER_TITLE;
				break;

		case enuTEXT_LABEL_LANE_NUMBER:
					return LABEL_REG_VAL_LANE_NUMBER;
				break;

		case enuTEXT_LABEL_SPARE1:
					return LABEL_REG_VAL_SPARE1;
				break;
		case enuTEXT_LABEL_SPARE2:
					return LABEL_REG_VAL_SPARE2;
				break;
		case enuTEXT_LABEL_SPARE3:
					return LABEL_REG_VAL_SPARE3;
				break;
		case enuTEXT_LABEL_SPARE4:
					return LABEL_REG_VAL_SPARE4;
				break;
		case enuTEXT_LABEL_SPARE5:
					return LABEL_REG_VAL_SPARE5;
				break;
		case enuTEXT_LABEL_SPARE6:
					return LABEL_REG_VAL_SPARE6;
				break;	
		case enuTEXT_LABEL_SPARE7:
					return LABEL_REG_VAL_SPARE7;
				break;
		case enuTEXT_LABEL_SPARE8:
					return LABEL_REG_VAL_SPARE8;
				break;
		case enuTEXT_LABEL_SPARE9:
					return LABEL_REG_VAL_SPARE9;
				break;
		case enuTEXT_LABEL_SPARE10:
					return LABEL_REG_VAL_SPARE10;
				break;

		case enuTEXT_LABEL_SPARE11:
					return LABEL_REG_VAL_SPARE11;
				break;

		case enuTEXT_LABEL_SPARE12:
					return LABEL_REG_VAL_SPARE12;
				break;
		case enuTEXT_LABEL_SPARE13:
					return LABEL_REG_VAL_SPARE13;
				break;
		case enuTEXT_LABEL_SPARE14:
					return LABEL_REG_VAL_SPARE14;
				break;
		case enuTEXT_LABEL_SPARE15:
					return LABEL_REG_VAL_SPARE15;
				break;
		case enuTEXT_LABEL_SPARE16:
					return LABEL_REG_VAL_SPARE16;
				break;
		case enuTEXT_LABEL_SPARE17:
					return LABEL_REG_VAL_SPARE17;
				break;
		case enuTEXT_LABEL_SPARE18:
					return LABEL_REG_VAL_SPARE18;
				break;
		case enuTEXT_LABEL_SPARE19:
					return LABEL_REG_VAL_SPARE19;
				break;
		case enuTEXT_LABEL_SPARE20:
					return LABEL_REG_VAL_SPARE20;
				break;

		case enuTEXT_LABEL_SPARE21:
					return LABEL_REG_VAL_SPARE21;
				break;
		case enuTEXT_LABEL_SPARE22:
					return LABEL_REG_VAL_SPARE22;
				break;
		case enuTEXT_LABEL_SPARE23:
					return LABEL_REG_VAL_SPARE23;
				break;
		case enuTEXT_LABEL_SPARE24:
					return LABEL_REG_VAL_SPARE24;
				break;
		case enuTEXT_LABEL_SPARE25:
					return LABEL_REG_VAL_SPARE25;
				break;
		case enuTEXT_LABEL_SPARE26:
					return LABEL_REG_VAL_SPARE26;
				break;	
		case enuTEXT_LABEL_SPARE27:
					return LABEL_REG_VAL_SPARE27;
				break;
		case enuTEXT_LABEL_SPARE28:
					return LABEL_REG_VAL_SPARE28;
				break;
		case enuTEXT_LABEL_SPARE29:
					return LABEL_REG_VAL_SPARE29;
				break;
		case enuTEXT_LABEL_SPARE30:
					return LABEL_REG_VAL_SPARE30;
				break;
		case enuTEXT_LABEL_SPARE31:
					return LABEL_REG_VAL_SPARE31;
				break;
		case enuTEXT_LABEL_SPARE32:
					return LABEL_REG_VAL_SPARE32;
				break;
		case enuTEXT_LABEL_SPARE33:
					return LABEL_REG_VAL_SPARE33;
				break;
		case enuTEXT_LABEL_SPARE34:
					return LABEL_REG_VAL_SPARE34;
				break;
		case enuTEXT_LABEL_SPARE35:
					return LABEL_REG_VAL_SPARE35;
				break;
		case enuTEXT_LABEL_SPARE36:
					return LABEL_REG_VAL_SPARE36;
				break;	
		case enuTEXT_LABEL_SPARE37:
					return LABEL_REG_VAL_SPARE37;
				break;
		case enuTEXT_LABEL_SPARE38:
					return LABEL_REG_VAL_SPARE38;
				break;
		case enuTEXT_LABEL_SPARE39:
					return LABEL_REG_VAL_SPARE39;
				break;
		case enuTEXT_LABEL_SPARE40:
					return LABEL_REG_VAL_SPARE40;
				break;
		case enuTEXT_LABEL_SPARE41:
					return LABEL_REG_VAL_SPARE41;
				break;
		case enuTEXT_LABEL_SPARE42:
					return LABEL_REG_VAL_SPARE42;
				break;
		case enuTEXT_LABEL_SPARE43:
					return LABEL_REG_VAL_SPARE43;
				break;
		case enuTEXT_LABEL_SPARE44:
					return LABEL_REG_VAL_SPARE44;
				break;
		case enuTEXT_LABEL_SPARE45:
					return LABEL_REG_VAL_SPARE45;
				break;
		case enuTEXT_LABEL_SPARE46:
					return LABEL_REG_VAL_SPARE46;
				break;	
		case enuTEXT_LABEL_SPARE47:
					return LABEL_REG_VAL_SPARE47;
				break;
		case enuTEXT_LABEL_SPARE48:
					return LABEL_REG_VAL_SPARE48;
				break;
		case enuTEXT_LABEL_SPARE49:
					return LABEL_REG_VAL_SPARE49;
				break;
		case enuTEXT_LABEL_SPARE50:
					return LABEL_REG_VAL_SPARE50;
				break;
		case enuTEXT_LABEL_SPARE51:
					return LABEL_REG_VAL_SPARE51;
				break;
		case enuTEXT_LABEL_SPARE52:
					return LABEL_REG_VAL_SPARE52;
				break;
		case enuTEXT_LABEL_SPARE53:
					return LABEL_REG_VAL_SPARE53;
				break;
		case enuTEXT_LABEL_SPARE54:
					return LABEL_REG_VAL_SPARE54;
				break;
		case enuTEXT_LABEL_SPARE55:
					return LABEL_REG_VAL_SPARE55;
				break;
		case enuTEXT_LABEL_SPARE56:
					return LABEL_REG_VAL_SPARE56;
				break;	
		case enuTEXT_LABEL_SPARE57:
					return LABEL_REG_VAL_SPARE57;
				break;
		case enuTEXT_LABEL_SPARE58:
					return LABEL_REG_VAL_SPARE58;
				break;
		case enuTEXT_LABEL_SPARE59:
					return LABEL_REG_VAL_SPARE59;
				break;
		case enuTEXT_LABEL_SPARE60:
					return LABEL_REG_VAL_SPARE60;
				break;


		case enuTEXT_LABEL_FARE_TYPE:
					return LABEL_REG_VAL_FARE_TYPE;
				break;

		case enuTEXT_LABEL_FARE_TYPE_TITLE:
					return LABEL_REG_VAL_FARE_TYPE_TITLE;
				break;

		case enuTEXT_LABEL_TXT_MODE_TITLE:
					return LABEL_REG_VAL_TXT_MODE_TITLE;
				break;
		case enuTEXT_LABEL_TXT_MODE:
					return LABEL_REG_VAL_TXT_MODE;
				break;

		case enuTEXT_LABEL_TXT_COLLECTOR_TITLE:
					return LABEL_REG_VAL_TXT_COLLECTOR_TITLE;
				break;

		case enuTEXT_LABEL_TXT_IN_CTRL_USER_TITLE:
					return LABEL_REG_VAL_TXT_IN_CTRL_USER_TITLE;
				break;

		case enuTEXT_LABEL_TXT_IN_CTRL_USER:
					return LABEL_REG_VAL_TXT_IN_CTRL_USER;
				break;

		case enuTEXT_LABEL_TXT_IN_CTRL_USER1:
					return LABEL_REG_VAL_TXT_IN_CTRL_USER1;
				break;

		case enuTEXT_LABEL_TXT_TRS_CONTAINER_TITLE:
					return LABEL_REG_VAL_TXT_TRS_CONTAINER_TITLE;
				break;

		case enuTEXT_LABEL_TXT_ALARMS_TITLE:
					return LABEL_REG_VAL_TXT_ALARMS_TITLE;
				break;

		case enuTEXT_LABEL_TXT_TRS_HISTORY_TITLE:
					return LABEL_REG_VAL_TXT_TRS_HISTORY_TITLE;
				break;

		case enuTEXT_LABEL_TXT_ALERTES_TITLE:
					return LABEL_REG_VAL_TXT_ALERTES_TITLE;
				break;

		case enuTEXT_LABEL_TXT_CAM_NUM_PIC:
					return LABEL_REG_VAL_TXT_CAM_NUM_PIC;
				break;
		case enuTEXT_LABEL_TXT_SCAN_NUM_PIC:
					return LABEL_REG_VAL_TXT_SCAN_NUM_PIC;
				break;
		case enuTEXT_LABEL_TXT_NB_CARS_SAS:
					return LABEL_REG_VAL_TXT_NB_CARS_SAS;
				break;

	//Instructions
		case enuTEXT_LABEL_INSTR:
					return LABEL_REG_VAL_INSTR;
				break;	

		case enuTEXT_LABEL_INSTR_2:
					return LABEL_REG_VAL_INSTR_2;
				break;

		case enuTEXT_LABEL_PAYMENT:
					return LABEL_REG_VAL_PAYMENT;
				break;

		case enuTEXT_LABEL_ERR_PAYMENT:
					return LABEL_REG_VAL_ERR_PAYMENT;
				break;

		case enuTEXT_LABEL_REMARK:
					return LABEL_REG_VAL_REMARK;
				break;
	//------------------------------------
		//separate object not updated by virtual objects, 
		// but depending on "in control" status of the interface
		case enuTEXT_LABEL_IN_CONTROL_STATUS:
					return LABEL_REG_VAL_IN_CONTROL_STATUS;
			break;
	//Lists
		case enuLIST_LIST_ALARMS:
					return LIST_REG_VAL_LIST_ALARMS;
				break;

		case enuLIST_LIST_WARNINGS:
					return LIST_REG_VAL_LIST_WARNINGS;
				break;

	//------------------------------------

		case enuTEXT_LABEL_NBCAR:
					return LABEL_REG_VAL_NBCAR;
				break;


	//Icons
		case enuICO_CAR:
					return ICO_REG_VAL_CAR;
				break;

		case enuICO_COLLECTOR:
					return ICO_REG_VAL_COLLECTOR;
				break;
		case enuICO_ENTRYGATE:
					return ICO_REG_VAL_ENTRYGATE;
				break;

		case enuICO_ENTRYLOOP:
					return ICO_REG_VAL_ENTRYLOOP;
				break;

		case enuICO_EXITGATE:
					return ICO_REG_VAL_EXITGATE;
				break;

		case enuICO_EXITLOOP:
					return ICO_REG_VAL_EXITLOOP;
				break;

		case enuICO_MODE:
					return LABEL_REG_VAL_NBCAR;
				break;

		case enuICO_STATUS:
					return ICO_REG_VAL_STATUS;
				break;
		case enuICO_TRAFFIC:
					return ICO_REG_VAL_TRAFFIC;
				break;

		case enuICO_VIOLATION:
					return ICO_REG_VAL_VIOLATION;
				break;
		case enuICO_ICO_BEACON:
					return ICO_REG_VAL_ICO_BEACON;
				break;
		case enuICO_ICO_OPT_BARR:
					return ICO_REG_VAL_ICO_OPT_BARR;
				break;

		case enuICO_SPARE1:	return ICO_REG_VAL_SPARE1;
		case enuICO_SPARE2:	return ICO_REG_VAL_SPARE2;
		case enuICO_SPARE3: return ICO_REG_VAL_SPARE3;
		case enuICO_SPARE4: return ICO_REG_VAL_SPARE4;
		case enuICO_SPARE5: return ICO_REG_VAL_SPARE5;
		case enuICO_SPARE6:	return ICO_REG_VAL_SPARE6;
		case enuICO_SPARE7:	return ICO_REG_VAL_SPARE7;
		case enuICO_SPARE8: return ICO_REG_VAL_SPARE8;
		case enuICO_SPARE9: return ICO_REG_VAL_SPARE9;
		case enuICO_SPARE10: return ICO_REG_VAL_SPARE10;
		case enuICO_SPARE11: return ICO_REG_VAL_SPARE11;
		case enuICO_SPARE12: return ICO_REG_VAL_SPARE12;
		case enuICO_SPARE13: return ICO_REG_VAL_SPARE13;
		case enuICO_SPARE14: return ICO_REG_VAL_SPARE14;
		case enuICO_SPARE15: return ICO_REG_VAL_SPARE15;
		case enuICO_SPARE16: return ICO_REG_VAL_SPARE16;
		case enuICO_SPARE17: return ICO_REG_VAL_SPARE17;
		case enuICO_SPARE18: return ICO_REG_VAL_SPARE18;
		case enuICO_SPARE19: return ICO_REG_VAL_SPARE19;
		case enuICO_SPARE20: return ICO_REG_VAL_SPARE20;
		case enuICO_SPARE21: return ICO_REG_VAL_SPARE21;
		case enuICO_SPARE22: return ICO_REG_VAL_SPARE22;
		case enuICO_SPARE23: return ICO_REG_VAL_SPARE23;
		case enuICO_SPARE24: return ICO_REG_VAL_SPARE24;
		case enuICO_SPARE25: return ICO_REG_VAL_SPARE25;
		case enuICO_SPARE26: return ICO_REG_VAL_SPARE26;
		case enuICO_SPARE27: return ICO_REG_VAL_SPARE27;
		case enuICO_SPARE28: return ICO_REG_VAL_SPARE28;
		case enuICO_SPARE29: return ICO_REG_VAL_SPARE29;
		case enuICO_SPARE30: return ICO_REG_VAL_SPARE30;
		case enuICO_SPARE31: return ICO_REG_VAL_SPARE31;
		case enuICO_SPARE32: return ICO_REG_VAL_SPARE32;
		case enuICO_SPARE33: return ICO_REG_VAL_SPARE33;
		case enuICO_SPARE34: return ICO_REG_VAL_SPARE34;
		case enuICO_SPARE35: return ICO_REG_VAL_SPARE35;
		case enuICO_SPARE36: return ICO_REG_VAL_SPARE36;
		case enuICO_SPARE37: return ICO_REG_VAL_SPARE37;
		case enuICO_SPARE38: return ICO_REG_VAL_SPARE38;
		case enuICO_SPARE39: return ICO_REG_VAL_SPARE39;
		case enuICO_SPARE40: return ICO_REG_VAL_SPARE40;
		case enuICO_SPARE41: return ICO_REG_VAL_SPARE41;
		case enuICO_SPARE42: return ICO_REG_VAL_SPARE42;
		case enuICO_SPARE43: return ICO_REG_VAL_SPARE43;
		case enuICO_SPARE44: return ICO_REG_VAL_SPARE44;
		case enuICO_SPARE45: return ICO_REG_VAL_SPARE45;
		case enuICO_SPARE46: return ICO_REG_VAL_SPARE46;
		case enuICO_SPARE47: return ICO_REG_VAL_SPARE47;
		case enuICO_SPARE48: return ICO_REG_VAL_SPARE48;
		case enuICO_SPARE49: return ICO_REG_VAL_SPARE49;
		case enuICO_SPARE50: return ICO_REG_VAL_SPARE50;

		case enuICO_CAMERA1:
					return ICO_REG_VAL_CAMERA1;
				break;

		case enuICO_CAMERA2:
					return ICO_REG_VAL_CAMERA2;
				break;

		case enuICO_BEACON2:
					return ICO_REG_VAL_BEACON2;
				break;
		case enuICO_SIGNAL_LIGHT:
					return ICO_REG_VAL_SIGNAL_LIGHT;
				break;

		case enuICO_EXT_OPT_BARR:
					return ICO_REG_VAL_EXT_OPT_BARR;
				break;

	//------------------------------------


	//Containers
		case enuCNT_PAYMENT_GROUP:
					return CNT_REG_VAL_PAYMENT_GROUP;
				break;

		case enuCNT_INSTR_GROUP:
					return CNT_REG_VAL_INSTR_GROUP;
				break;

		case enuCNT_TRS_GROUP:
					return CNT_REG_VAL_TRS_GROUP;
				break;

		case enuCNT_MODE_GROUP:
					return CNT_REG_VAL_MODE_GROUP;
				break;
		case enuCNT_HEADER_GROUP:
					return CNT_REG_VAL_HEADER_GROUP;
				break;


		case enuCNT_CNT_DYN_PICTURE:
					return CNT_REG_VAL_CNT_DYN_PICTURE;
				break;

		case enuCNT_LBL_DYN_IMAGE:
					return CNT_REG_VAL_LBL_DYN_IMAGE;
				break;

		case enuCNT_CNT_DYN_PICTURE2:
					return CNT_REG_VAL_CNT_DYN_PICTURE2;
				break;	

		case enuCNT_LBL_DYN_IMAGE2:
					return CNT_REG_VAL_LBL_DYN_IMAGE2;
				break;
		case enuCNT_CNT_COMMENTS:
					return CNT_REG_VAL_CNT_COMMENTS;
				break;

		case enuCNT_CNT_CAMERA:
					return CNT_REG_VAL_CNT_CAMERA;
				break;

		case enuCNT_CNT_SCANNER:
					return CNT_REG_VAL_CNT_SCANNER;
				break;

		case enuCNT_CNT_SPARE1:return CNT_REG_VAL_CNT_SPARE1;
		case enuCNT_CNT_SPARE2:return CNT_REG_VAL_CNT_SPARE2;
		case enuCNT_CNT_SPARE3:return CNT_REG_VAL_CNT_SPARE3;
		case enuCNT_CNT_SPARE4:return CNT_REG_VAL_CNT_SPARE4;
		case enuCNT_CNT_SPARE5:return CNT_REG_VAL_CNT_SPARE5;
		case enuCNT_CNT_SPARE6:return CNT_REG_VAL_CNT_SPARE6;
		case enuCNT_CNT_SPARE7:return CNT_REG_VAL_CNT_SPARE7;
		case enuCNT_CNT_SPARE8:return CNT_REG_VAL_CNT_SPARE8;
		case enuCNT_CNT_SPARE9:return CNT_REG_VAL_CNT_SPARE9;
		case enuCNT_CNT_SPARE10:return CNT_REG_VAL_CNT_SPARE10;
		case enuCNT_CNT_SPARE11:return CNT_REG_VAL_CNT_SPARE11;
		case enuCNT_CNT_SPARE12:return CNT_REG_VAL_CNT_SPARE12;
		case enuCNT_CNT_SPARE13:return CNT_REG_VAL_CNT_SPARE13;
		case enuCNT_CNT_SPARE14:return CNT_REG_VAL_CNT_SPARE14;
		case enuCNT_CNT_SPARE15:return CNT_REG_VAL_CNT_SPARE15;
		case enuCNT_CNT_SPARE16:return CNT_REG_VAL_CNT_SPARE16;
		case enuCNT_CNT_SPARE17:return CNT_REG_VAL_CNT_SPARE17;
		case enuCNT_CNT_SPARE18:return CNT_REG_VAL_CNT_SPARE18;
		case enuCNT_CNT_SPARE19:return CNT_REG_VAL_CNT_SPARE19;
		case enuCNT_CNT_SPARE20:return CNT_REG_VAL_CNT_SPARE20;

		case enuCNT_CNT_MINI_WEB:		return CNT_REG_VAL_CNT_MINI_WEB;
		case enuCNT_CNT_ALARMS_GROUP:	return CNT_REG_VAL_CNT_ALARMS_GROUP;

			//Buttons
		case enuCNT_BTN_TAKE_CTRL:
					return CNT_REG_VAL_BTN_TAKE_CTRL;
				break;

		case enuCNT_BTN_RETURN_CTRL:
					return CNT_REG_VAL_BTN_RETURN_CTRL;
				break;

		case enuCNT_BTN_CLASS:
					return CNT_REG_VAL_BTN_CLASS;
				break;

		case enuCNT_BTN_ENTRY_POINT:
					return CNT_REG_VAL_BTN_ENTRY_POINT;
				break;

		case enuCNT_BTN_ENTRY_POINT_NBR:
					return CNT_REG_VAL_BTN_ENTRY_POINT_NBR;
				break;

		case enuCNT_BTN_EXTERNAL_DISPLAY:
					return CNT_REG_VAL_BTN_EXTERNAL_DISPLAY;
				break;
		case enuCNT_BTN_EXTERNAL_DISPLAY2:
					return CNT_REG_VAL_BTN_EXTERNAL_DISPLAY2;
				break;
		case enuCNT_BTN_MODE:
					return CNT_REG_VAL_BTN_MODE;
				break;

		case enuCNT_BTN_ARROW_ON:
					return CNT_REG_VAL_BTN_ARROW_ON;
				break;

		case enuCNT_BTN_ARROW_OFF:
					return CNT_REG_VAL_BTN_ARROW_OFF;
				break;

		case enuCNT_BTN_CROSS_ON:
					return CNT_REG_VAL_BTN_CROSS_ON;
				break;
		case enuCNT_BTN_CROSS_OFF:
					return CNT_REG_VAL_BTN_CROSS_OFF;
				break;

		case enuCNT_BTN_DSRC_ON:
					return CNT_REG_VAL_BTN_DSRC_ON;
				break;

		case enuCNT_BTN_DSRC_OFF:
					return CNT_REG_VAL_BTN_DSRC_OFF;
				break;
		case enuCNT_BTN_MAGNET_ON:
					return CNT_REG_VAL_BTN_MAGNET_ON;
				break;

		case enuCNT_BTN_MAGNET_OFF:
					return CNT_REG_VAL_BTN_MAGNET_OFF;
				break;

		case enuCNT_BTN_COLLECTOR_ON:
					return CNT_REG_VAL_BTN_COLLECTOR_ON;
				break;

		case enuCNT_BTN_COLLECTOR_OFF:
					return CNT_REG_VAL_BTN_COLLECTOR_OFF;
				break;

		case enuCNT_BTN_CAMERA:
					return CNT_REG_VAL_BTN_CAMERA;
				break;
		case enuCNT_BTN_SCANNER:
					return CNT_REG_VAL_BTN_SCANNER;
				break;	
		
		case enuCNT_BTN_COMMENT:
					return CNT_REG_VAL_BTN_COMMENT;
				break;

		case enuCNT_PRODUCT_BUTTON1:
					return CNT_REG_VAL_PRODUCT_BUTTON1;
				break;

		case enuCNT_PRODUCT_BUTTON2:
					return CNT_REG_VAL_PRODUCT_BUTTON2;
				break;
		case enuCNT_PRODUCT_BUTTON3:
					return CNT_REG_VAL_PRODUCT_BUTTON3;
				break;

		case enuCNT_PRODUCT_BUTTON4:
					return CNT_REG_VAL_PRODUCT_BUTTON4;
				break;

		case enuCNT_PRODUCT_BUTTON5:
					return CNT_REG_VAL_PRODUCT_BUTTON5;
				break;

		case enuCNT_BTN_SPEED_LIMIT_ON:
					return CNT_REG_VAL_BTN_SPEED_LIMIT_ON;
				break;
		case enuCNT_BTN_SPEED_LIMIT_OFF:
					return CNT_REG_VAL_BTN_SPEED_LIMIT_OFF;
				break;

		case enuCNT_BTN_WARNING_ON:
					return CNT_REG_VAL_BTN_WARNING_ON;
				break;

		case enuCNT_BTN_WARNING_OFF:
					return CNT_REG_VAL_BTN_WARNING_OFF;
				break;

		case enuCNT_BTN_DSRC_TSA_ON:
					return CNT_REG_VAL_BTN_DSRC_TSA_ON;
				break;
		case enuCNT_BTN_DSRC_TSA_OFF:
					return CNT_REG_VAL_BTN_DSRC_TSA_OFF;
				break;

		case enuCNT_BTN_SPARE_1:return CNT_REG_VAL_BTN_SPARE_1;
		case enuCNT_BTN_SPARE_2:return CNT_REG_VAL_BTN_SPARE_2;
		case enuCNT_BTN_SPARE_3:return CNT_REG_VAL_BTN_SPARE_3;
		case enuCNT_BTN_SPARE_4:return CNT_REG_VAL_BTN_SPARE_4;
		case enuCNT_BTN_SPARE_5:return CNT_REG_VAL_BTN_SPARE_5;
		case enuCNT_BTN_SPARE_6:return CNT_REG_VAL_BTN_SPARE_6;
		case enuCNT_BTN_SPARE_7:return CNT_REG_VAL_BTN_SPARE_7;
		case enuCNT_BTN_SPARE_8:return CNT_REG_VAL_BTN_SPARE_8;
		case enuCNT_BTN_SPARE_9:return CNT_REG_VAL_BTN_SPARE_9;
		case enuCNT_BTN_SPARE_10:return CNT_REG_VAL_BTN_SPARE_10;

		case enuCNT_BTN_SPARE_11:return CNT_REG_VAL_BTN_SPARE_11;
		case enuCNT_BTN_SPARE_12:return CNT_REG_VAL_BTN_SPARE_12;
		case enuCNT_BTN_SPARE_13:return CNT_REG_VAL_BTN_SPARE_13;
		case enuCNT_BTN_SPARE_14:return CNT_REG_VAL_BTN_SPARE_14;
		case enuCNT_BTN_SPARE_15:return CNT_REG_VAL_BTN_SPARE_15;
		case enuCNT_BTN_SPARE_16:return CNT_REG_VAL_BTN_SPARE_16;
		case enuCNT_BTN_SPARE_17:return CNT_REG_VAL_BTN_SPARE_17;
		case enuCNT_BTN_SPARE_18:return CNT_REG_VAL_BTN_SPARE_18;
		case enuCNT_BTN_SPARE_19:return CNT_REG_VAL_BTN_SPARE_19;
		case enuCNT_BTN_SPARE_20:return CNT_REG_VAL_BTN_SPARE_20;

		case enuCNT_BTN_SPARE_21:return CNT_REG_VAL_BTN_SPARE_21;
		case enuCNT_BTN_SPARE_22:return CNT_REG_VAL_BTN_SPARE_22;
		case enuCNT_BTN_SPARE_23:return CNT_REG_VAL_BTN_SPARE_23;
		case enuCNT_BTN_SPARE_24:return CNT_REG_VAL_BTN_SPARE_24;
		case enuCNT_BTN_SPARE_25:return CNT_REG_VAL_BTN_SPARE_25;
		case enuCNT_BTN_SPARE_26:return CNT_REG_VAL_BTN_SPARE_26;
		case enuCNT_BTN_SPARE_27:return CNT_REG_VAL_BTN_SPARE_27;
		case enuCNT_BTN_SPARE_28:return CNT_REG_VAL_BTN_SPARE_28;
		case enuCNT_BTN_SPARE_29:return CNT_REG_VAL_BTN_SPARE_29;
		case enuCNT_BTN_SPARE_30:return CNT_REG_VAL_BTN_SPARE_30;


		case enuCNT_BTN_GABARITE:
					return CNT_REG_VAL_BTN_GABARITE;
				break;
		case enuCNT_BTN_ENTRY_GATE:
					return CNT_REG_VAL_BTN_ENTRY_GATE;
				break;

		case enuCNT_BTN_EXIT_GATE:
					return CNT_REG_VAL_BTN_EXIT_GATE;
				break;

		case enuCNT_BTN_CAR3:
					return CNT_REG_VAL_BTN_CAR3;
				break;
		case enuCNT_BTN_CAR2:
					return CNT_REG_VAL_BTN_CAR2;
				break;

		case enuCNT_BTN_CAR:
					return CNT_REG_VAL_BTN_CAR;
				break;

		case enuCNT_BTN_ENTRY_LOOP:
					return CNT_REG_VAL_BTN_ENTRY_LOOP;
				break;

		case enuCNT_BTN_EXIT_LOOP:
					return CNT_REG_VAL_BTN_EXIT_LOOP;
				break;

		case enuCNT_BTN_TBL_BEACON:
					return CNT_REG_VAL_BTN_TBL_BEACON;
				break;


		case enuCNT_BTN_CAM_GRAB:
					return CNT_REG_VAL_BTN_CAM_GRAB;
				break;
		case enuCNT_BTN_CAM_HIDE:
					return CNT_REG_VAL_BTN_CAM_HIDE;
				break;	
		case enuCNT_BTN_SCAN_GRAB:
					return CNT_REG_VAL_BTN_SCAN_GRAB;
				break;	
		case enuCNT_BTN_SCAN_HIDE:
					return CNT_REG_VAL_BTN_SCAN_HIDE;
				break;
		case enuCNT_BTN_SCAN_SEND:
					return CNT_REG_VAL_BTN_SCAN_SEND;
				break;
		case enuCNT_BTN_SCAN_SAVE:
					return CNT_REG_VAL_BTN_SCAN_SAVE;
				break;
		case enuCNT_BTN_COMM_HIDE:
					return CNT_REG_VAL_BTN_COMM_HIDE;
				break;


	//------------------------------------------------------------

		case enuCLK_TIME:
					return CLK_REG_VAL_TIME;
				break;

		case enuCLK_DATE:
					return CLK_REG_VAL_DATE;
				break;

		
	//------------------------------------------------------------

		case enuCNT_FILE_VIEW1:
					return CNT_REG_VAL_FILE_VIEW1;
				break;

		case enuCNT_FILE_VIEW2:
					return CNT_REG_VAL_FILE_VIEW2;
				break;

		case enuCNT_FILE_VIEW3:
					return CNT_REG_VAL_FILE_VIEW3;
				break;
	//------------------------------------------------------------
		
		case enuCNT_HMENU_VIEW1:
					return CNT_REG_VAL_HMENU_VIEW1;
				break;
		
		case enuCNT_HMENU_VIEW2:
					return CNT_REG_VAL_HMENU_VIEW2;
				break;
		
	//------------------------------------------------------------
		
		case enuCNT_RVIDEO_VIEW1:
					return CNT_REG_VAL_RVIDEO_VIEW1;
				break;

		case enuCNT_RVIDEO_VIEW2:
					return CNT_REG_VAL_RVIDEO_VIEW2;
				break;

		case enuCNT_TAB_VIEW1:return CNT_REG_VAL_TAB_VIEW1;
		case enuCNT_TAB_VIEW2:return CNT_REG_VAL_TAB_VIEW2;
		case enuCNT_TAB_VIEW3:return CNT_REG_VAL_TAB_VIEW3;
		case enuCNT_TAB_VIEW4:return CNT_REG_VAL_TAB_VIEW4;

		case enuCNT_CNT_TASK_BAR:
					return CNT_REG_VAL_CNT_TASK_BAR;
				break;
			
	default:
		TRACE_W(QString("MIhmDskVisibleObject::getVisibleObjNameForID: Error unknown visible object ID:%1").arg((int)eId));
		Q_ASSERT(false);
	    return QString("");
	}
}


MIhmDskVisibleObject::enumVisibleObjectId MIhmDskVisibleObject::getVisibleObjIDForName(const QString &sName)
{
	enumVisibleObjectId eRetVal = enuIhmVisibleObjUnknown;
		
	for (int i = enuIhmVisibleObjUnknown; i < enuIhmVisibleObjLast; i++)
	{
		if(getVisibleObjNameForID((enumVisibleObjectId)i)==sName)
			return (enumVisibleObjectId)i;
	}

	return eRetVal;
}

MIhmDskVisibleObject * MIhmDskVisibleObject::findVisibleObjectByID(QVector <MIhmDskVisibleObject*> *pObjVector, const enumVisibleObjectId eId)
{
	//do the check 
	if(pObjVector->at((int)eId)!=NULL)
		if(pObjVector->at((int)eId)->getId() != eId)	
		{
			TRACE_W(QString("MIhmDskVisibleObject::findVisibleObjectByID: Error invalid visible object ID:%1").arg((int)eId));
			ExitBad();
		}

	return pObjVector->at((int)eId);
}



void MIhmDskVisibleObject::setReadOnly(bool bReadOnly)
{
	m_bReadOnly = bReadOnly;
}


void MIhmDskVisibleObject::setCursor(const QCursor& cursor)
{
	m_WidgetCursor = cursor;

	if(m_pWidget!=NULL)
		m_pWidget->setCursor(cursor);

}


void MIhmDskVisibleObject::repaintWidget()
{
	if(m_pWidget!=NULL)
		m_pWidget->repaint();

}


//--------------------------------------------------------------------

MIhmContainer::MIhmContainer(enumVisibleObjectId eId)
{ 
	m_eId = eId; 
	m_sName = getVisibleObjNameForID(eId);
}



bool MIhmContainer::setIniSettings(LaneTypeVisObjParams * pVisParams)
{
	QString sVisible = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE);
    if (sVisible.compare("1",Qt::CaseInsensitive) == 0 || sVisible.compare("true",Qt::CaseInsensitive)==0)
    {
		this->setConfVisible(true);
    }
	else 
		this->setConfVisible(false);

	//init NOT_USED flag
	QString sNotUsed = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED);
	
	if(sNotUsed.compare("1",Qt::CaseInsensitive) == 0 || sNotUsed.compare("true",Qt::CaseInsensitive)==0)
		this->setNotUsed(true);
	else
		this->setNotUsed(false);



	QString sLeft = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_LEFT);
	QString sTop = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TOP);
	
	bool bOK1,bOK2;
	int iLeft = sLeft.toInt(&bOK1);
	int iTop = sTop.toInt(&bOK2);

	if(sLeft!="" && sTop!="" && bOK1 && bOK2)
	{
		QPoint pos = QPoint(iLeft,iTop);
		this->getFrame()->move(pos);
	}

	QString sWidth = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_WIDTH);
	QString sHeight = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_HEIGHT);

	int iWidth = sWidth.toInt(&bOK1);
	int iHeight = sHeight.toInt(&bOK2);

	if(sWidth!="" && sHeight!="" && bOK1 && bOK2)
	{
		QSize sz = QSize(iWidth,iHeight);
		this->getFrame()->resize(sz);
	}


	m_sActionID = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID);

	return true;
}


void MIhmContainer::setReadOnly(bool bReadOnly)
{
	if(m_bReadOnly!=bReadOnly)
	{
		m_bReadOnly = bReadOnly;

		if(m_pWidget!=NULL)
		{
			if (!bReadOnly && m_sActionID != "")
				m_pWidget->setCursor(m_WidgetCursor);
			else
			{
				if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
					m_pWidget->setCursor(Qt::BlankCursor);
				else
					m_pWidget->setCursor(Qt::ArrowCursor);
			}
		}
	}
}


void MIhmContainer::setVisible(bool bShow)
{
	if(m_pWidget !=NULL)
	{
		m_pWidget->setVisible(bShow);
	}
}

//--------------------------------------------------------------------



MIhmIco::MIhmIco(enumVisibleObjectId eId):
			MIhmContainer(eId)
{
	m_movie = NULL;
}



MIhmIco::~MIhmIco()
{
	if(m_movie!=NULL)
	{	
		m_movie->stop();

		QLabel * pLabel = getLabel();
		if(pLabel !=NULL)
			pLabel->setMovie(NULL);

		delete m_movie;
		m_movie = NULL;	
	}
}


void MIhmIco::resetIcon()
{
	QLabel * pLabel = getLabel();

	if(pLabel !=NULL)
	{
		pLabel->clear();

		if(m_movie!=NULL)
		{	
			if(m_movie->state() == QMovie::Running)
				m_movie->stop();
			delete m_movie;
			m_movie = NULL;	
		}

	}
}



void MIhmIco::setVisible(bool bShow)
{
	QLabel * pLabel = getLabel();

	if(pLabel !=NULL)
	{
		pLabel->setVisible(bShow);
	}
}

bool MIhmIco::loadIcon(QString sPath, bool bAnimate)
{
	QLabel * pLabel = getLabel();
	
	if(pLabel !=NULL)
	{	
		if(m_movie!=NULL)
		{	
			if(m_movie->state() == QMovie::Running)
				m_movie->stop();
			
			pLabel->setMovie(NULL);

			delete m_movie;
			m_movie = NULL;	
		}

		if(!bAnimate)
		{
			 QPixmap oPixmap;
			 bool bPixmapOk = false;

			 if (QPixmapCache::find(sPath, &oPixmap))
			 {
				 bPixmapOk = true;
			 } 
			 else 
			 {
				if(oPixmap.load(sPath))
				{
					QPixmapCache::insert(sPath, oPixmap);
					bPixmapOk = true;
				}
			 }

			if(bPixmapOk)
			{
				pLabel->setPixmap(oPixmap);
				return true;
			}

		}
		else
		{	
			
			TRACE_D(QString("MIhmIco::loadIcon: Loading movie...:%1").arg(sPath));
			
			m_movie = new QMovie(sPath);
			m_movie->setCacheMode(QMovie::CacheAll);

			if(m_movie->isValid())
			{
				pLabel->setMovie(m_movie);
		 		m_movie->start();
				TRACE_D(QString("MIhmIco::loadIcon: Movie loaded."));
				return true;
			}
			else
			{
				delete m_movie;
				m_movie = NULL;	
				
				TRACE_W(QString("MIhmIco::loadIcon: unable to load movie file:%1").arg(sPath));
			}

		}
	}

	return false;
}

//--------------------------------------------------------------------


void MIhmLabel::setVisible(bool bShow)
{
	QLabel * pLabel = getLabel();

	if(pLabel !=NULL)
	{
		pLabel->setVisible(bShow);
	}
}



//--------------------------------------------------------------------


MIhmButton::MIhmButton(enumVisibleObjectId eId)
			:MIhmContainer(eId)
{
	m_pButtonLabel = NULL;
	m_bInitialized = false;
	m_bReadOnly = true;
	m_bEnabled = false;
	m_bReadOnlyInvariant = false;
}

MIhmButton::~MIhmButton()
{
	if(m_pButtonLabel!=NULL)	
		delete m_pButtonLabel;
}
		
bool MIhmButton::initialize(MFormMain * pMainForm, bool bReadOnlyInvariant)
{
	QFrame * frFrame = (QFrame *)m_pWidget;
	m_bReadOnlyInvariant = bReadOnlyInvariant;
	
	if(!m_bInitialized && frFrame!=NULL)
	{
		frFrame->setFrameShape(QFrame::NoFrame);
		m_hlayout = new QHBoxLayout(frFrame);
		m_hlayout->setSpacing(0);
		m_hlayout->setContentsMargins(0,0,0,0);

		m_pButtonLabel = new MButtonLabel(frFrame);
		m_pButtonLabel->initialize(this, pMainForm);
		m_pButtonLabel->setObjectName(m_sName);
		
        QSizePolicy sizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(m_pButtonLabel->sizePolicy().hasHeightForWidth());

		m_pButtonLabel->setSizePolicy(sizePolicy);
        m_pButtonLabel->setFocusPolicy(Qt::NoFocus);


		m_hlayout->addWidget(m_pButtonLabel);

		m_bInitialized = true;
		updateReadOnlyState();
	}
	else
	{
		TRACE_W(QString("MIhmButton::initialize: Error in object: %1 - frFrame == %2 ")
			.arg(m_sName)
			.arg((frFrame == NULL) ? "frFrame==NULL" : "frFrame != NULL"));
	}

	return m_bInitialized;
}

void MIhmButton::setVisible(bool bShow)
{
	if(m_pWidget!=NULL)
	{
		m_pWidget->setVisible(bShow);
		
		if(m_pButtonLabel!=NULL)
			m_pButtonLabel->setVisible(bShow);
	}
}


void MIhmButton::updateButton(QString sTranslatedText, QString sToolTip, QString sImagePath, bool bEnabled)
{
	if(m_pButtonLabel!=NULL)
	{
		m_pButtonLabel->updateButton(sTranslatedText, sImagePath);
		m_pButtonLabel->setToolTip(sToolTip);
	}

	setEnabled(bEnabled);
}


void MIhmButton::setText(QString sTranslatedText)
{
	if(m_pButtonLabel!=NULL)
		m_pButtonLabel->setText(sTranslatedText);
}


void MIhmButton::updateReadOnlyState()
{
	if(m_pButtonLabel!=NULL)
	{
		bool bActualEnabled;
		
		//if not dependent on out of control state of the session
		//do not use the m_bReadOnly flag to generate the result
		//this way we can use the same class for take control and return control buttons
		if(m_bReadOnlyInvariant)
			bActualEnabled = m_bEnabled;
		else
			bActualEnabled = !m_bReadOnly && m_bEnabled;
		
		m_pButtonLabel->setEnabled(bActualEnabled);

		if (bActualEnabled && m_sActionID != "")
			m_pButtonLabel->setCursor(m_WidgetCursor);
		else
		{
			if (MIhmConfigGeneral::getCfg()->m_bHideCursor)
				m_pButtonLabel->setCursor(Qt::BlankCursor);
			else
				m_pButtonLabel->setCursor(Qt::ArrowCursor);
		}
		
	}
}

void MIhmButton::setEnabled(bool bEnabled)
{
	if(m_bEnabled!=bEnabled)
	{
		m_bEnabled = bEnabled;
		updateReadOnlyState();
	}
}


void MIhmButton::setReadOnly(bool bReadOnly)
{
	if(m_bReadOnly!=bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		updateReadOnlyState();
	}
}


void MIhmButton::setCursor(const QCursor& cursor)
{
	m_WidgetCursor = cursor;

	if(m_pButtonLabel!=NULL)
		m_pButtonLabel->setCursor(cursor);

}

//--------------------------------------------------------------------
//-------- MIhmTabView -----------------------------------------------
//--------------------------------------------------------------------


MIhmTabView::MIhmTabView(enumVisibleObjectId eId)
			:MIhmContainer(eId)
{
	m_pTabView = NULL;
	m_bInitialized = false;
}

MIhmTabView::~MIhmTabView()
{
	if(m_pTabView!=NULL)	
		delete m_pTabView;
}
		
bool MIhmTabView::initialize(QFrame * pFrame, QFrame * pContentFrame)
{
	
	if(!m_bInitialized && pFrame!=NULL)
	{
		m_pTabView = new MTabView(pFrame);
		m_pTabView->setObjectName(m_sName);
		
		QSize sz = pFrame->size(); 
		QPoint pt = pFrame->pos(); 

		TRACE_D(QString("MIhmTabView::initialize:Size before Layout: size: width:%1, height:%2; pos: x:%3, y:%4")
									.arg(sz.width()).arg(sz.height())					
									.arg(pt.x()).arg(pt.y()));

		m_layout = new QVBoxLayout (pFrame);
		m_layout->setSpacing(0);
		m_layout->setContentsMargins(0,0,0,0);
		m_layout->addWidget(m_pTabView);

		pFrame->move(pt);
		pFrame->resize(sz);
	
		m_pTabView->resize(sz);
		m_pTabView->move(pt);
		m_pTabView->setFocusPolicy(Qt::NoFocus);

		m_layout->update();

		m_bInitialized = true;

		m_pContentFrame = pContentFrame;
		m_pContentFrame->setVisible(true);
	}

	return m_bInitialized;
}

void MIhmTabView::setVisible(bool bShow)
{
	if(m_pWidget!=NULL)
	{
		m_pWidget->setVisible(bShow);
	}

}


void MIhmTabView::updateTab(QStringList& sLstTabs, int iCurrent)
{
	int iTabCount;
	MIhmLanguages *pLang = MIhmConfigGeneral::getCfg()->getLanguages();

	m_pTabView->setUpdatesEnabled(false);

	for(int i=0;i<sLstTabs.size();i++)
	{		
		iTabCount = m_pTabView->count();
		QString sTranslated;
		
		pLang->getLabelTranslation(sLstTabs.at(i), MIhmLanguages::enuTranslTargetDesktop, sTranslated);

		if(iTabCount>i)
		{
			if(sTranslated != m_pTabView->tabText(i))
			{
				m_pTabView->setTabText(i,sTranslated);
			}
		}
		else //if(iTabCount<=i)
		{
			QFrame * pNewFrame = new QFrame(m_pTabView);
			int iNew = m_pTabView->addTab(pNewFrame, sTranslated);
			
		}
	}

	iTabCount = m_pTabView->count();


	if(iTabCount>sLstTabs.size())
	{
		for(int i=iTabCount-1;i>=sLstTabs.size();i--)
		{
			QFrame * pFrame = (QFrame *)m_pTabView->widget(i);
			m_pTabView->removeTab(i);
			if(pFrame != NULL)
				delete pFrame;
		}
	}

	m_pTabView->setCurrentIndex(iCurrent);
	m_pTabView->setUpdatesEnabled(true);
	
}

bool MIhmTabView::setIniSettings(LaneTypeVisObjParams * pVisParams)
{
	bool bOK;
	
	bOK = MIhmContainer::setIniSettings(pVisParams);

	if(bOK)
	{
		//NOTE the action id for the visible object is unknown at the moment of the object initialize
		//so the data is initialized to the widget (child) object after initialized with LaneType settings 
		m_pTabView->init(getActionID());
	}

	return bOK;
}

QWidget * MIhmTabView::getClickableWidget()
{ 
	if (m_pTabView != NULL) 
		return(QWidget*)m_pTabView->getTabBar(); 
	else 
		return NULL; 
}



//--------------------------------------------------------------------
MIhmFileView::MIhmFileView(enumVisibleObjectId eId):MIhmContainer(eId)
{
	m_bValid = false;
	m_pTableModelView = NULL;
	m_uiLastCommandTimestamp = 0;
	m_iForceFocusKey = 0;
	m_iMaxRowsAllowed = CFG_DSK_TABLE_FILE_VIEW_DEF_MAX_ROWS_ALLOWED;
}

MIhmFileView::~MIhmFileView()
{
	if(m_pTableModelView!=NULL)
		delete m_pTableModelView;
}

bool MIhmFileView::setIniSettings(LaneTypeVisObjParams * pVisParams)
{
	bool bRetVal;

	bRetVal = MIhmContainer::setIniSettings(pVisParams);

	if(bRetVal)
	{
		m_sFormatFile = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_TEMPLATE);
		QString sMaxRows = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_MAX_ROWS);
		
		bool bOK;
		int iRows = sMaxRows.toInt(&bOK);
		
		if(bOK)
			m_iMaxRowsAllowed = iRows;

		QString sSetFocusKey = pVisParams->getParam(CFG_VISIBLE_OBJ_ATTRIBUTE_SET_FOCUS_KEY);
		m_iForceFocusKey = sSetFocusKey.toInt();


		if(m_sFormatFile == "")
		{
			TRACE_W(QString("MIhmFileView::setIniSettings: No format file defined for table view object %1").arg(m_sName));
			return false;
		}
		else
		{	
			QFrame *pFrame = getFrame();

			if(pFrame!=NULL)
			{
				m_hlayout = new QHBoxLayout(pFrame);
				m_hlayout->setSpacing(0);
				m_hlayout->setContentsMargins(0,0,0,0);

				MTableView *pCurrentTableView = new MTableView(pFrame);
				m_hlayout->addWidget(pCurrentTableView);

				m_pTableModelView = new MTableModelView();
				QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(m_sFormatFile);
				
				if(m_pTableModelView->loadFormatFile(sFullPath))
				{
					m_pTableModelView->setMaxRowsAllowed(m_iMaxRowsAllowed);
					m_pTableModelView->setTableView(pCurrentTableView);
					pCurrentTableView->setVisible(true);
					m_bValid = true;
					bRetVal = true;
				}
				else
				{
					TRACE_W(QString("MIhmFileView::setIniSettings: Unable to load format file %1 for object %2").arg(sFullPath).arg(m_sName));
					return false;
				}
			}
			else
			{
				TRACE_W(QString("MIhmFileView::setIniSettings: No frame widget defined for object %1").arg(m_sName));
				return false;	
			}
		}
	}

	return bRetVal;
}


void MIhmFileView::setReadOnly(bool bReadOnly)
{
	if(m_bReadOnly!=bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		m_pTableModelView->setReadOnly(bReadOnly);
	}
}

QWidget * MIhmFileView::getClickableWidget()
{
	QWidget * pRetVal = NULL;

	if(m_pTableModelView!=NULL && m_bValid)
		pRetVal = m_pTableModelView->getTableView();

	return pRetVal;
}

//by default return that key is not processed 
bool MIhmFileView::onKeyDetected(int iAskedKey)
{	
	bool bKeyProcessed = false;

	bKeyProcessed = m_pTableModelView->onKeyDetected(iAskedKey);

	return bKeyProcessed;
}

void MIhmFileView::forceFocus()
{
	if(m_bValid && m_pTableModelView!=NULL && !m_bReadOnly)
		m_pTableModelView->forceFocus();

}

bool MIhmFileView::executeRowCommand(QList <MTableRow*> *plstNewRows, enum_aff_table_command eCmd, quint64 uiCommandTimestamp)		
{
	bool bRetVal = false;

	if(m_pTableModelView!=NULL)
	{
		m_uiLastCommandTimestamp = uiCommandTimestamp;
		
		if(eCmd == enuIHM_AFF_TABLE_CMD_RESET_ROWS)
		{
			m_pTableModelView->clearAllRows();
			bRetVal = true;
		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_PREPEND_ROWS)
		{
			m_pTableModelView->prependRows(plstNewRows);
			bRetVal = true;

		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_APPEND_ROWS)
		{
			m_pTableModelView->appendRows(plstNewRows);
			bRetVal = true;

		}
		else if(eCmd == enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS)
		{
			m_pTableModelView->clearAllRows();
			m_pTableModelView->appendRows(plstNewRows);
			bRetVal = true;
		}
	}
	
	return bRetVal;
}

void MIhmFileView::setVisible(bool bVisible)
{
	if(!m_bValid)
			return;
	
	m_pTableModelView->getTableView()->setVisible(bVisible);
}



//--------------------------------------------------------------------
//MMM
MIhmHMenuView::MIhmHMenuView(enumVisibleObjectId eId):MIhmContainer(eId)
{
	m_bValid = false;
	m_menuFileView = NULL;
	m_pMenuFileLoader = new MHMenuFileLoader();
	m_bVisible = true;
	m_bReadOnly = true;
}

MIhmHMenuView::~MIhmHMenuView()
{
	delete m_pMenuFileLoader;
}


bool MIhmHMenuView::initialize()
{
	QFrame *pFrame = getFrame();
	if(pFrame!=NULL)
	{
		m_hlayout = new QHBoxLayout(pFrame);
		m_hlayout->setSpacing(0);
		m_hlayout->setContentsMargins(0, 0, 0, 0);

		m_menuFileView = new MHMenuView(pFrame);
		m_menuFileView->setObjectName(m_sName);
		m_menuFileView->setVisible(true);

		m_hlayout->addWidget(m_menuFileView);

		pFrame->setVisible(false);
		m_bValid = true;
	}

	return m_bValid;
}


void MIhmHMenuView::setReadOnly(bool bReadOnly)
{
	if(m_bReadOnly!=bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		updateReadOnlyState();
	}
}

bool MIhmHMenuView::updateMenu(QString sMenuContent, bool bUseExternalFile, bool bDoNotReloadMenu, bool bVisible)
{
	bool bOK = false;
	bool bReload = !bDoNotReloadMenu;


	TRACE_D(QString( "MFormMain::updateMenu: ExternalFile:%1; Visible:%2; bDoNotReloadMenu:%3;")
						.arg(bUseExternalFile).arg(bVisible).arg(bDoNotReloadMenu));
	
	m_bVisible = bVisible;

	if(m_menuFileView!=NULL && m_bValid)
	{
		if(bUseExternalFile)
		{
			QString sFullPath = MIhmConfigGeneral::getCfg()->getCommFileFullPath(sMenuContent);
			bOK = m_pMenuFileLoader->loadDataFromFile(sFullPath, bReload);
		}
		else
			bOK = m_pMenuFileLoader->loadDataFromBuffer(sMenuContent, bReload);

		if(bOK)
		{
			QList <MMenuModelItem*> * pModel = m_pMenuFileLoader->getLoadedModel();
 			m_menuFileView->updateMenu(pModel, bReload);
		}
		else
		{
			TRACE_W(QString("MIhmHMenuView::updateMenu: Failed for: Menu:%1, Reload:%2")
						.arg(sMenuContent).arg(bReload));
			TRACE_W(QString("\t %1").arg(m_pMenuFileLoader->getLastErrorMessage()));
		}
	}
	else
	{
		TRACE_W(QString( "MFormMain::updateMenu: Object not initialized!"));
		
	}

	if(!bOK)
	{
		m_bVisible = false;			
	}

	updateReadOnlyState();

	return bOK;
}

void MIhmHMenuView::updateReadOnlyState()
{
	if(m_menuFileView!=NULL)
	{
		QFrame *pFrame = getFrame();
		
		if(pFrame!=NULL)
		{
			bool bCurrentVisiblity = pFrame->isVisible();
			bool bTargetVisibility = m_bVisible && !m_bReadOnly;

			if(bCurrentVisiblity!=bTargetVisibility)
			{
				pFrame->setVisible(bTargetVisibility);

				if(bTargetVisibility==false)
				{
					//assure that all menus are closed
					QList <MMenuModelItem*> * pModel = m_pMenuFileLoader->getLoadedModel();
					
					m_menuFileView->collapseAll(pModel);
				}
			}
		}
		else
		{
			TRACE_W(QString( "MFormMain::updateReadOnlyState: pFrame not initialized!"));
		}
	}
}



//--------------------------------------------------------------------
// Restream video
//--------------------------------------------------------------------

MIhmRestreamVideoView::MIhmRestreamVideoView(enumVisibleObjectId eId, int iMyVirtObjId):MIhmContainer(eId)
{
	m_bValid = false;
	m_pRVideoView = NULL;
	m_bShown = false;
	m_iMyVirtObjId = iMyVirtObjId;
}

MIhmRestreamVideoView::~MIhmRestreamVideoView()
{
	if(m_pRVideoView!=NULL)
		delete m_pRVideoView;
}

bool MIhmRestreamVideoView::setSource(QString sURL, QString sUser, QString sPwd)
{
	bool bRet = false;

	if(m_pRVideoView!=NULL)
	{
		m_sSourceUrl = sURL ;
		m_sUser = sUser;
		m_sPwd = sPwd;
		
		m_sSourceUrlWithSessionID = m_sSourceUrl;
		//m_sSourceUrlWithSessionID.append(QString("&RESSClientID=CLI%1")
		//	.arg(MIhmVirtualObject::getNameForId((enum MIhmVirtualObject::enumVirtualObjectId)m_iMyVirtObjId)));

		bRet = m_pRVideoView->SetSource(m_sSourceUrlWithSessionID, m_sUser, m_sPwd);
	}
	return bRet;
}

void MIhmRestreamVideoView::showVideo()
{
	if(m_pRVideoView!=NULL)
	{
		m_bShown = true;
		m_pRVideoView->ShowVideo();
	}
}

void MIhmRestreamVideoView::hideVideo()
{
	if(m_pRVideoView!=NULL)
	{
		m_bShown = false;
		m_pRVideoView->HideVideo();
	}
}		

void MIhmRestreamVideoView::setVisible(bool bVisible)
{
	if(!m_bValid)
		return;
	
	QFrame *pFrame = getFrame();
	pFrame->setVisible(bVisible);
}

void MIhmRestreamVideoView::setReadOnly(bool bReadOnly)
{
	if(m_bReadOnly != bReadOnly)
	{
		m_bReadOnly = bReadOnly;
		
		if(m_pRVideoView!=NULL)
		{
			if(bReadOnly)
			{
				getFrame()->setVisible(false);
				m_pRVideoView->HideVideo();
			}
			else
			{
				 getFrame()->setVisible(true);
				
				if(!m_sSourceUrl.isEmpty())
					m_pRVideoView->SetSource(m_sSourceUrlWithSessionID, m_sUser, m_sPwd);
				
				if(m_bShown)
					m_pRVideoView->ShowVideo();
				else
					m_pRVideoView->HideVideo();
			}
		}
	}
}


bool MIhmRestreamVideoView::setIniSettings(LaneTypeVisObjParams * pVisParams)
{
	bool bRetVal;
	
	bRetVal = MIhmContainer::setIniSettings(pVisParams);
	
	if(bRetVal)
	{
		bRetVal = setIniVideoSettings();
	}
	
	return bRetVal;
}

bool MIhmRestreamVideoView::setIniVideoSettings()
{
	bool bRetVal = false;
	TRACE_D(QString("MIhmRestreamVideoView::setIniSettings: ..."));

	QFrame *pFrame = getFrame();
	pFrame->setVisible(true);
	
	if(pFrame!=NULL)
	{
		m_pRVideoView = new MRVideoView(pFrame);
			
		if(m_pRVideoView != NULL && m_pRVideoView->init()) 
		{	
			m_hlayout = new QHBoxLayout(pFrame);
			m_hlayout->setSpacing(0);
			m_hlayout->setContentsMargins(0, 0, 0, 0);
			m_hlayout->addWidget(m_pRVideoView);
			m_pRVideoView->show();
			
			m_bValid = true;
			bRetVal = true;
		}
		else
		{
			TRACE_W(QString("MIhmRestreamVideoView::setIniSettings: Unable to create video view object %1").arg(m_sName));
			bRetVal = false;
		}
	}
	else
	{
		TRACE_W(QString("MIhmRestreamVideoView::setIniSettings: No frame widget defined for object %1").arg(m_sName));
		bRetVal = false;
	}
	return bRetVal;
}




//--------------------------------------------------------------------

//MIhmTaskBarView::MIhmTaskBarView()
//{ 
//	m_pTaskBar = NULL;
//}
//
//MIhmTaskBarView::MIhmTaskBarView(enumVisibleObjectId eId)
//:MIhmContainer(eId)
//{ 
//	m_pTaskBar = NULL;
//}
//
//MIhmTaskBarView::~MIhmTaskBarView()
//{
//
//}
//
//void MIhmTaskBarView::minimizeAll()
//{
//	m_pTaskBar->minimizeAll();
//}
//
//
//void MIhmTaskBarView::setReadOnly(bool bReadOnly)
//{
//	if(m_pTaskBar!=NULL)
//	{
//		if(bReadOnly)
//			m_pTaskBar->closeAll();
//			
//	}
//}
//
//
//bool MIhmTaskBarView::initialize(QFrame *pFrame, MFormMain *pMainWindow, QString sDlgUITemplatePath)
//{
//	if(pFrame!=NULL)
//	{
//		setWidget(pFrame);
//		m_pTaskBar = new MIhmTaskBar(pFrame);
//		m_pTaskBar->init(pMainWindow, sDlgUITemplatePath);
//		m_pTaskBar->setObjectName(m_sName);
//
//		return true;
//	}				
//	
//	return false;
//}
//
//void MIhmTaskBarView::initDialog(int nType, QString sName,  const QRect &rectGeometry )
//{
//	m_pTaskBar->initDialog(nType, sName, rectGeometry);
//}
//
//
//
//void MIhmTaskBarView::openLink(int iTargetDlg, QString sUrl)
//{
//	if(m_pTaskBar!=NULL)
//		m_pTaskBar->openBrowserDlg(iTargetDlg,sUrl);
//}
//
//void  MIhmTaskBarView::closeDlg(int iTargetDlg)
//{
//	if (m_pTaskBar != NULL)
//		m_pTaskBar->closeBrowserDlg(iTargetDlg);
//}
//

//--------------------------------------------------------------------
// MIhmList
//--------------------------------------------------------------------

MIhmList::MIhmList()
{
	m_pMListWidget  = NULL;
}

MIhmList::MIhmList(enumVisibleObjectId eId):MIhmContainer(eId)
{
	m_pMListWidget  = NULL;
}

MIhmList::~MIhmList()
{

}

void MIhmList::clear()
{
	m_sItemList.clear();

	if(m_pMListWidget!=NULL)
		m_pMListWidget->clear();
}

bool MIhmList::initialize(QFrame*  pListFrame, QLabel * pToolTipLabel, bool bIconMode, bool bShowText, bool bShowIcons, bool bTooltipShowLastItem)
{
	 
	if(pListFrame!=NULL)
	{
		m_pMListWidget = new MListWidget(pListFrame);

		m_pMListWidget->setObjectName(m_sName);

		QVBoxLayout *pLayout = new QVBoxLayout(pListFrame);
		pLayout->setSpacing(0);
		pLayout->setContentsMargins(0,0,0,0);
		pLayout->addWidget(m_pMListWidget);

		this->setWidget(m_pMListWidget);

		m_pMListWidget->setViewMode((bIconMode)?QListView::IconMode: QListView::ListMode);
		m_pMListWidget->setIconSize(QSize(50,50));

		if(!bShowText && !bShowIcons) //do not allow showing empty items
			 bShowText = true; 

		m_pMListWidget->initialize(pToolTipLabel, bShowText, bShowIcons, bTooltipShowLastItem);


		return true;
	}
	else
		return false;

}

void MIhmList::addItemsToListView(QStringList &lstToAdd)
{
	m_sItemList << lstToAdd;
	m_pMListWidget->addItemsToListView(lstToAdd);
}






