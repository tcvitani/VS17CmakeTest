

#include "MIhmWebCommonObjLogic.h"
#include "MIhmWebVisibleObject.h"
#include "MIhmVirtualObject.h"
#include "MIhmMsgVideo.h"

#include "MIhmWebInterface.h"
#include "MIhmHttpResponse.h"
#include "MInputDialogReq.h"
#include "MTracer.h"
#include "MIhmMsg.h"
#include "MHelpFuncs.h"
#include "MRowsCommand.h"
#include "MParamList.h"

#include <QUuid>
#include <QString>
#include <QList>
#include <QRegularExpression>
#include <QDir>


MIhmWebCommonObjLogic::MIhmWebCommonObjLogic(MIhmWebInterface *parent)
: m_pWebInterface(parent)
{
	initializeCommonVisibleObjects();
}


MIhmWebCommonObjLogic::~MIhmWebCommonObjLogic()
{
	//clear visible objects vector
	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCommonWebVisibleObjects.size();i++)
	{
		if(m_vectCommonWebVisibleObjects.at(i)!=NULL)
			delete m_vectCommonWebVisibleObjects.at(i);
	}
}



void MIhmWebCommonObjLogic::initializeCommonVisibleObjects()
{
	MIhmWebVisibleObject * pNewObject;
	
	m_vectCommonWebVisibleObjects.fill(NULL, (int)MIhmWebVisibleObject::enuWebVisibleObjectLastCommon + 1);

	for (int i = MIhmWebVisibleObject::enuWebVisibleObjectUnknown + 1; i<=MIhmWebVisibleObject::enuWebVisibleObjectLastCommon; i++)
	{
		switch(i)
		{
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_PAID:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_ID:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_NAME:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME1:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME2:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME3:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME1:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME2:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE1:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE2:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE3:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE4:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE5:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE6:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE7:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE8:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE9:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE10:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE11:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE12:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE13:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE14:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE15:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE16:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE17:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE18:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE19:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE20:

			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE21:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE22:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE23:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE24:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE25:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE26:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE27:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE28:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE29:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE30:

			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE31:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE32:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE33:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE34:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE35:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE36:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE37:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE38:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE39:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE40:

			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE41:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE42:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE43:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE44:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE45:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE46:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE47:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE48:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE49:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE50:

			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE51:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE52:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE53:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE54:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE55:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE56:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE57:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE58:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE59:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE60:


			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR_2:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PAYMENT:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ERR_PAYMENT:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_REMARK:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_NBCAR:

			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_COLLECTOR_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER1:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_CONTAINER_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALARMS_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_HISTORY_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALERTES_TITLE:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CAM_NUM_PIC:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SCAN_NUM_PIC:
			case MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_NB_CARS_SAS:
			case MIhmWebVisibleObject::enuCLK_WEB_DATE:
				pNewObject = new MIhmWebVisibleTextLabel((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

	
			case MIhmWebVisibleObject::enuLIST_WEB_LIST_ALARMS:
			case MIhmWebVisibleObject::enuLIST_WEB_LIST_WARNINGS:
				pNewObject = new MIhmWebVisibleList((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			// //------------------------------------
			// //Icons
			case MIhmWebVisibleObject::enuICO_WEB_CAR:
			case MIhmWebVisibleObject::enuICO_WEB_COLLECTOR:
			case MIhmWebVisibleObject::enuICO_WEB_ENTRYGATE:
			case MIhmWebVisibleObject::enuICO_WEB_ENTRYLOOP:
			case MIhmWebVisibleObject::enuICO_WEB_EXITGATE:
			case MIhmWebVisibleObject::enuICO_WEB_EXITLOOP:
			case MIhmWebVisibleObject::enuICO_WEB_MODE:
			case MIhmWebVisibleObject::enuICO_WEB_STATUS:
			case MIhmWebVisibleObject::enuICO_WEB_TRAFFIC:
			case MIhmWebVisibleObject::enuICO_WEB_ENT_OPTICAL_BARIER:
			case MIhmWebVisibleObject::enuICO_WEB_VIOLATION:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE1:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE2:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE3:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE4:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE5:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE6:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE7:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE8:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE9:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE10:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE11:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE12:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE13:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE14:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE15:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE16:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE17:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE18:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE19:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE20:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE21:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE22:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE23:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE24:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE25:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE26:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE27:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE28:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE29:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE30:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE31:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE32:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE33:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE34:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE35:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE36:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE37:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE38:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE39:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE40:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE41:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE42:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE43:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE44:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE45:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE46:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE47:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE48:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE49:
			case MIhmWebVisibleObject::enuICO_WEB_SPARE50:
			case MIhmWebVisibleObject::enuICO_WEB_CAMERA1:
			case MIhmWebVisibleObject::enuICO_WEB_CAMERA2:
			case MIhmWebVisibleObject::enuICO_WEB_BEACON:
			case MIhmWebVisibleObject::enuICO_WEB_BEACON2:
			case MIhmWebVisibleObject::enuICO_WEB_SIGNAL_LIGHT:
			case MIhmWebVisibleObject::enuICO_WEB_EXT_OPT_BARR:				
				pNewObject = new MIhmWebVisibleImage((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE:				
			case MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE2:				
				pNewObject = new MIhmWebVisibleDynImage((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			//------------------------------------
			//Containers
			case MIhmWebVisibleObject::enuCNT_WEB_MAIN_DIALOG:
			case MIhmWebVisibleObject::enuCNT_WEB_PAYMENT_GROUP:
			case MIhmWebVisibleObject::enuCNT_WEB_INSTR_GROUP:
			case MIhmWebVisibleObject::enuCNT_WEB_TRS_GROUP:
			case MIhmWebVisibleObject::enuCNT_WEB_MODE_GROUP:
			case MIhmWebVisibleObject::enuCNT_WEB_HEADER_GROUP:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE1:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE2:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE3:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE4:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE5:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE6:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE7:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE8:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE9:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE10:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE11:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE12:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE13:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE14:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE15:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE16:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE17:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE18:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE19:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE20:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_MINI_WEB:
			case MIhmWebVisibleObject::enuCNT_WEB_CNT_ALARMS_GROUP:
				pNewObject = new MIhmWebVisibleContainer((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			case MIhmWebVisibleObject::enuCNT_WEB_COMMENTS:
			case MIhmWebVisibleObject::enuCNT_WEB_CAMERA:
			case MIhmWebVisibleObject::enuCNT_WEB_SCANNER:				
				pNewObject = new MIhmWebVisibleCtrlDependContainer((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;
			//------------------------------------------------------------
			// Buttons
			case MIhmWebVisibleObject::enuBTN_WEB_CLASS:
			case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT:
			case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT_NBR:
			case MIhmWebVisibleObject::enuBTN_WEB_MODE:
			case MIhmWebVisibleObject::enuBTN_WEB_ARROW_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_ARROW_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_CROSS_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_CROSS_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_DSRC_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_DSRC_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_MAGNET_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_MAGNET_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_CAMERA:
			case MIhmWebVisibleObject::enuBTN_WEB_SCANNER:
			case MIhmWebVisibleObject::enuBTN_WEB_COMMENT:
			case MIhmWebVisibleObject::enuBTN_WEB_GABARITE:
			case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_GATE:
			case MIhmWebVisibleObject::enuBTN_WEB_EXIT_GATE:
			case MIhmWebVisibleObject::enuBTN_WEB_CAR3:
			case MIhmWebVisibleObject::enuBTN_WEB_CAR2:
			case MIhmWebVisibleObject::enuBTN_WEB_CAR:
			case MIhmWebVisibleObject::enuBTN_WEB_ENTRY_LOOP:
			case MIhmWebVisibleObject::enuBTN_WEB_EXIT_LOOP:
			case MIhmWebVisibleObject::enuBTN_WEB_TBL_BEACON:
			case MIhmWebVisibleObject::enuBTN_WEB_CAM_GRAB:
			case MIhmWebVisibleObject::enuBTN_WEB_CAM_HIDE:
			case MIhmWebVisibleObject::enuBTN_WEB_SCAN_GRAB:
			case MIhmWebVisibleObject::enuBTN_WEB_SCAN_HIDE:
			case MIhmWebVisibleObject::enuBTN_WEB_SCAN_SEND:
			case MIhmWebVisibleObject::enuBTN_WEB_SCAN_SAVE:
			case MIhmWebVisibleObject::enuBTN_WEB_COMM_HIDE:
			case MIhmWebVisibleObject::enuBTN_WEB_BUTTON1:
			case MIhmWebVisibleObject::enuBTN_WEB_BUTTON2:
			case MIhmWebVisibleObject::enuBTN_WEB_BUTTON3:
			case MIhmWebVisibleObject::enuBTN_WEB_BUTTON4:
			case MIhmWebVisibleObject::enuBTN_WEB_BUTTON5:
			case MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_WARNING_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_WARNING_OFF:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_ON:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_OFF:

			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_1:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_2:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_3:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_4:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_5:				
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_6:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_7:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_8:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_9:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_10:				
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_11:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_12:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_13:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_14:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_15:				
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_16:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_17:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_18:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_19:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_20:				
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_21:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_22:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_23:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_24:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_25:				
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_26:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_27:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_28:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_29:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_30:				
				pNewObject = new MIhmWebVisibleButton((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;
			
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY:
			case MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY2:
				pNewObject = new MIhmWebVisibleButton((MIhmWebVisibleObject::enumWebVisibleObjectId)i, true);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			//------------------------------------------------------------
			case MIhmWebVisibleObject::enuDLG_VAL_WEB_ASYNC_INPUT:				
				pNewObject = new MIhmWebAsyncInput((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			//------------------------------------------------------------
 			case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW1:				
 			case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW2:				
 			case MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW3:				
				pNewObject = new MIhmWebVisibleFileView((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			//------------------------------------------------------------
  			case MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW1:				
  			case MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW2:				
				pNewObject = new MIhmWebVisibleHMenuView((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;
			//------------------------------------------------------------
			case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW1:				
			case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW2:				
			case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW3:				
			case MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW4:				
				pNewObject = new MIhmWebTabView((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;
			//------------------------------------------------------------
  			case MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW1:				
				pNewObject = new MIhmWebRVideoView((MIhmWebVisibleObject::enumWebVisibleObjectId)i, MIhmVirtualObject::enuIhmVirtRestreamVideoView1);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;
  			case MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW2:				
				pNewObject = new MIhmWebRVideoView((MIhmWebVisibleObject::enumWebVisibleObjectId)i, MIhmVirtualObject::enuIhmVirtRestreamVideoView2);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;


  			case MIhmWebVisibleObject::enuCMD_WEB_CLIENT_DIRECT_VIDEO:				
				pNewObject = new MIhmWebGenericCtrlDependentScript((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

  			case MIhmWebVisibleObject::enuCMD_WEB_WINDOW_TITLE:				
				pNewObject = new MIhmWebGenericScript((MIhmWebVisibleObject::enumWebVisibleObjectId)i);
				m_vectCommonWebVisibleObjects.replace(i, pNewObject);	
			break;

			default:
				TRACE_W(QString("MIhmWebCommonObjLogic::initializeVisibleObjects: Error unknown visible object ID:%1").arg(i));
				continue;
		}

	}
}



void MIhmWebCommonObjLogic::processVirtualObject(MIhmVirtualObject* pVirtObj)
{	
	if(m_pWebInterface->getStatus() != MIhmWebInterface::enuInitialized)
	{
			pVirtObj->clearUpdated();
	}

	switch(pVirtObj->getType())
	{
		case MIhmVirtualObject::enuIhmContainer:
				processVirtContainer((MIhmVirtContainer*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmIco:
				processVirtIcon((MIhmVirtIco*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmText:
				processVirtText((MIhmVirtText*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmButton:
				processVirtButton((MIhmVirtButton*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmList:
				processVirtList((MIhmVirtList*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmDynConfig:
				processVirtConfig((MIhmVirtDynConfig*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmDynStrDetection:
				processVirtStrDetConfig((MIhmVirtStrDetectConfig*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmTableViewReq:
				processFileViewReq((MIhmVirtTableViewReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmTabView:
				processTabViewReq((MIhmVirtTabView*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmAsyncInput:		//M_IHM_ASYNC_INPUT service handling
				processAsyncInput((MIhmVirtAsyncInput*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmHMenuViewReq:		
				processHMenuViewReq((MIhmVirtHMenuViewReq*)pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVideoViewReq:		
				processVideoViewReq((MIhmVirtVideoViewReq*)pVirtObj);
			break;

		default:
			TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processVirtualObject: Unknown object type %1").
										arg(pVirtObj->getType()));
			break;
	}	
}



//---------------------------------------------------------------------------------


void MIhmWebCommonObjLogic::processFileViewReq(MIhmVirtTableViewReq* pVirtObj)
{
	TRACE_WEB_D(QString("MIhmWebCommonObjLogic::processFileViewReq: ..."));

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtTableView1:
			updateFileViewContainer(MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtTableView2:
			updateFileViewContainer(MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtTableView3:
			updateFileViewContainer(MIhmWebVisibleObject::enuCNT_WEB_FILE_VIEW3, pVirtObj);
			break;
		default:
			break;
	}
	

}


void MIhmWebCommonObjLogic::updateFileViewContainer(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtTableViewReq* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateFileViewContainer: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleFileView * pCurrent;
	pCurrent = (MIhmWebVisibleFileView *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateFileViewContainer: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlTableView)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateFileViewContainer: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateFileViewContainer: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}


	//verify if the visible object is up to date with the virtual object
	if(pVirtObj->getLastUpdate() != pCurrent->getLastCommandTimestamp())
	{
		//process the commands 
		QList <MRowsCommand *> * pLst = pVirtObj->getRowCommandList();
		MRowsCommand * pCurrentCommand;

		if(pLst->count() > 0)
		{
			int i=0;
			bool bFoundTheLastUpdatedRow = false;

			TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateFileViewContainer: New list count:%1").arg(pLst->count()));

			quint64 uiCurrentLastUpdate = pCurrent->getLastCommandTimestamp();
			
			for(i=0; i<pLst->count();i++ )
			{
				pCurrentCommand = pLst->at(i);

				if(pCurrentCommand->m_uiCommandTimestamp == uiCurrentLastUpdate)
				{
					bFoundTheLastUpdatedRow = true;
					break;
				}
			}
			
			if(bFoundTheLastUpdatedRow)
			{
				i++;
				for(; i<pLst->count();i++ )
				{
					pCurrentCommand = pLst->at(i);
					TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateFileViewContainer: updating command:%1;new rows:%2").arg(i).arg(pCurrentCommand->m_lstRows.count()));
					pCurrent->executeRowCommand(&(pCurrentCommand->m_lstRows), pCurrentCommand->m_eCmd, pCurrentCommand->m_uiCommandTimestamp);
				}
			}
			else
			{
				pCurrent->executeRowCommand(NULL, 	enuIHM_AFF_TABLE_CMD_RESET_ROWS, Q_UINT64_C(0));
				
				for(i=0; i<pLst->count();i++ )
				{
					pCurrentCommand = pLst->at(i);
					pCurrent->executeRowCommand(&(pCurrentCommand->m_lstRows), pCurrentCommand->m_eCmd, pCurrentCommand->m_uiCommandTimestamp);
				}
			}
		}
		else
		{
			pCurrent->executeRowCommand(NULL, 	enuIHM_AFF_TABLE_CMD_RESET_ROWS, pVirtObj->getLastUpdate());
		}
	}

	
	pCurrent->setVisible(pVirtObj->isVisible());

}



//-------------------------------------------------------------------------
//----------Processing of MIhmVirtAsyncInput				---------------
//-------------------------------------------------------------------------


void MIhmWebCommonObjLogic::processAsyncInput(MIhmVirtAsyncInput* pVirtObj)
{
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processAsyncInput: ...");

	MIhmWebAsyncInput * pWebAsyncInputs;
	pWebAsyncInputs = (MIhmWebAsyncInput *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, 
										MIhmWebVisibleObject::enuDLG_VAL_WEB_ASYNC_INPUT);
	
	if(pWebAsyncInputs!=NULL)
	{
		if(!pWebAsyncInputs->initialize(pVirtObj->m_pAsyncReq))
		{
			TRACE_WEB_W("MIhmWebCommonObjLogic::processAsyncInput: Error updateValues returned false!");
		}			

	}
	else
	{
		TRACE_WEB_W("MIhmWebCommonObjLogic::processAsyncInput: pWebAsyncInputs == NULL");
	}
}

//-------------------------------------------------------------------------
//----------END Processing of MIhmVirtAsyncInput				---------------
//-------------------------------------------------------------------------



void MIhmWebCommonObjLogic::updateTabView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, QStringList& lstTabText, int iCurrentIndex ,bool bVisible)
{
	TRACE_WEB_D(QString( "MIhmWebCommonObjLogic::updateTabView: ..%1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebTabView * pCurrent;
	pCurrent = (MIhmWebTabView *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString( "MIhmWebCommonObjLogic::updateTabView: findVisibleObjectByName return NULL for the visible object name %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlTabView)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateTabView: invalid visible object type:[%1]!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}
	
	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D(QString("MIhmWebCommonObjLogic::updateTabView: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	pCurrent->updateTab(lstTabText, iCurrentIndex);
	pCurrent->setVisible(bVisible);
	
}


void MIhmWebCommonObjLogic::processTabViewReq(MIhmVirtTabView* pVirtObj)
{
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processTabViewReq: ...");

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtTabControl1:
				updateTabView(MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW1, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl2:
				updateTabView(MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW2, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl3:
				updateTabView(MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW3, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		case MIhmVirtualObject::enuIhmVirtTabControl4:
				updateTabView(MIhmWebVisibleObject::enuCNT_WEB_TAB_VIEW4, pVirtObj->m_slstTabText, pVirtObj->m_iCurrentIndex , pVirtObj->isVisible());
			break;
		default:
			TRACE_WEB_W(QString( "MIhmWebCommonObjLogic::processTabViewReq: Unknown virtual object id:%1")
									.arg(pVirtObj->getId()));
			break;
	}
}

//--------------------------------------------------------------------------------				


void MIhmWebCommonObjLogic::processVirtContainer(MIhmVirtContainer* pVirtObj)
{
	switch(pVirtObj->getId())
	{			

		case MIhmVirtualObject::enuIhmVirtCntHeaderGroup:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_HEADER_GROUP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntInstructionGroup:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_INSTR_GROUP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntModeGroup:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_MODE_GROUP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntPaymentGroup:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_PAYMENT_GROUP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntTrsGroup:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_TRS_GROUP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntComments:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_COMMENTS, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntCamera:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CAMERA, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtCntScanner:
			updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_SCANNER, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtCntSpare1:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE1, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare2:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE2, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare3:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE3, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare4:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE4, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare5:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE5, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare6:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE6, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare7:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE7, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare8:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE8, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare9:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE9, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare10:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE10, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare11:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE11, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare12:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE12, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare13:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE13, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare14:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE14, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare15:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE15, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare16:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE16, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare17:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE17, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare18:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE18, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare19:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE19, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtCntSpare20:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_SPARE20, pVirtObj);	break;

		case MIhmVirtualObject::enuIhmVirtCntMiniWeb:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_MINI_WEB, pVirtObj);break;
		case MIhmVirtualObject::enuIhmVirtCntAlarmsGroup:updateContainerView(MIhmWebVisibleObject::enuCNT_WEB_CNT_ALARMS_GROUP, pVirtObj); break;
		default:
			break;
	}


}

void MIhmWebCommonObjLogic::updateContainerView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtContainer* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateContainerView: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleObject * pCurrent;
	pCurrent = (MIhmWebVisibleObject *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateContainerView: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlContainer && 
		pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlCtrlDependContainer)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateContainerView: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateContainerView: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}


	pCurrent->setVisible(pVirtObj->isVisible()); 

	if (!(pVirtObj->m_oPoliceValue.isEmpty()))
	{
		QString sStyle = pVirtObj->m_oPoliceValue.getColorStyle();

		if (sStyle != "")
		{
			TRACE_WEB_D(QString("MIhmWebCommonObjLogic::updateContainerView: style: %1")
				.arg(sStyle));
			pCurrent->setStyle(sStyle);
		}
	}
}

//----------------------------------------------------------------------

MIhmWebRVideoView * MIhmWebCommonObjLogic::getRVideoObject(MIhmWebVisibleObject::enumWebVisibleObjectId eId)
{
	MIhmWebRVideoView * pCurrent;
	pCurrent = (MIhmWebRVideoView *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::getRVideoObject: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return NULL;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlRVideoView)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::getRVideoObject: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return NULL;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::getRVideoObject: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return NULL;
	}

	return pCurrent;
}

void MIhmWebCommonObjLogic::updateRVideoView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtVideoViewReq* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateRVideoView: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebRVideoView * pCurrent = getRVideoObject(eId);
	
	if(pCurrent!=NULL)	
	{
		pCurrent->setVisible(pVirtObj->isVisible()); 
		
		if(pVirtObj->m_bVideoOn)
		{
			pCurrent->updateVideo(pVirtObj->m_sStreamSourceUrlRemote, pVirtObj->m_sBaseUrl);
		}
		else
		{
			pCurrent->updateVideo("", pVirtObj->m_sBaseUrl);
		}
	}
}



void MIhmWebCommonObjLogic::processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj)
{
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processVideoViewReq: ...");

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtRestreamVideoView1:
			updateRVideoView(MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtRestreamVideoView2:
			updateRVideoView(MIhmWebVisibleObject::enuCNT_WEB_RVIDEO_VIEW2, pVirtObj);
			break;
		default:
			break;
	}


}


//----------------------------------------------------------------------
void MIhmWebCommonObjLogic::processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj)
{
	TRACE_WEB_D(QString("MIhmWebCommonObjLogic::processHMenuViewReq..."));

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtHMenuView1:
			updateHMenuView(MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtHMenuView2:
			updateHMenuView(MIhmWebVisibleObject::enuCNT_WEB_HMENU_VIEW2, pVirtObj);
			break;
		default:
			break;
	}

}


void MIhmWebCommonObjLogic::updateHMenuView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtHMenuViewReq* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateHMenuView: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleHMenuView * pCurrent;
	pCurrent = (MIhmWebVisibleHMenuView *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateHMenuView: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlHMenuView)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateHMenuView: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateHMenuView: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}



	if(pVirtObj->isVisible())
	{
		TRACE_WEB_D(QString("MIhmWebCommonObjLogic::updateHMenuView:content: ..; m_bDoNotReloadMenu:%2").arg(pVirtObj->m_bDoNotReloadMenu));

		pCurrent->setVisible(true);

		if(!pCurrent->updateMenu(pVirtObj->m_sMenuContent, pVirtObj->m_bUseExternalFile, pVirtObj->m_bDoNotReloadMenu))
		{
			TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateHMenuView:Unable to load XML file %1").arg(pVirtObj->m_sMenuContent));
			pCurrent->setVisible(false);
		}
	}
	else
		pCurrent->setVisible(false);

}

//---------------------------------------------------------------------------------------

MIhmWebVisibleDynImage * MIhmWebCommonObjLogic::getVisibleDynImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId)
{
	MIhmWebVisibleDynImage * pCurrent = NULL;
	pCurrent = (MIhmWebVisibleDynImage*)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::getVisibleDynImage: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}
	else if(pCurrent->getType() != MIhmWebVisibleObject::enuHtmlDynImage )
	{	
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::getVisibleDynImage: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}
	else if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::getVisibleDynImage: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}

	return pCurrent;
}

MIhmWebVisibleDynImage * MIhmWebCommonObjLogic::getVisibleDynImageByName(QString sDynImageObjectName)
{
	MIhmWebVisibleDynImage * pCurrent = NULL;

	for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCommonWebVisibleObjects.size();i++)
	{
		if(m_vectCommonWebVisibleObjects.at(i)!=NULL)
			if(m_vectCommonWebVisibleObjects.at(i)->getType()==MIhmWebVisibleObject::enuHtmlDynImage)
				if(m_vectCommonWebVisibleObjects.at(i)->getName()==sDynImageObjectName)
				{
					pCurrent = (MIhmWebVisibleDynImage *)m_vectCommonWebVisibleObjects.at(i);
					break;
				}
	}


	if(pCurrent==NULL)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::getVisibleDynImageByName: Unable to find dynamic image name %1")
										.arg(sDynImageObjectName));
	}

	return pCurrent;
}

QString MIhmWebCommonObjLogic::getDynImageFullPath(QString sDynImageObjectName)
{
	MIhmWebVisibleDynImage * pVisibleImage;
	pVisibleImage = getVisibleDynImageByName(sDynImageObjectName);
	QString sResult;

	if(pVisibleImage!=NULL)
	{
		sResult = pVisibleImage->getImagePath();
	}

	return sResult;
}

void MIhmWebCommonObjLogic::updateDynImage(QString sImagePath, bool bShow, MIhmWebVisibleObject::enumWebVisibleObjectId eId)
{
	TRACE_WEB_D(QString("MIhmWebCommonObjLogic::updateDynImage: %1 to %2") 
												.arg(sImagePath) 
												.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleDynImage * pVisibleImage;
	pVisibleImage = getVisibleDynImage(eId);
	
	if(pVisibleImage!=NULL)
	{
		pVisibleImage->setVisible(bShow);
		pVisibleImage->setImagePath(sImagePath);
	}
}

//---------------------------------------------------------------------------------

MIhmWebVisibleImage * MIhmWebCommonObjLogic::getVisibleImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId)
{
	MIhmWebVisibleImage * pCurrent = NULL;
	pCurrent = (MIhmWebVisibleImage*)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateImage: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}
	else if(pCurrent->getType() != MIhmWebVisibleObject::enuHtmlImage )
	{	
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateImage: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}
	else if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateImage: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		pCurrent = NULL;
	}

	return pCurrent;
}

void MIhmWebCommonObjLogic::updateImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtIco* pVirtObj)
{
	TRACE_WEB_D(QString("MIhmWebCommonObjLogic::updateImage: %1 to %2") 
												.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)) 
												.arg(pVirtObj->m_sImageID));

	MIhmWebVisibleImage * pVisibleImage;
	pVisibleImage = getVisibleImage(eId);
	
	if(pVisibleImage!=NULL)
	{
		pVisibleImage->setVisible(pVirtObj->isVisible());

		if(!pVirtObj->m_sImageID.isEmpty())
		{
			QString sImagePath = MIhmConfigWeb::getCfg()->getWebConfigImages()->getImageFullPath(pVirtObj->m_sImageID);
			pVisibleImage->setImage(sImagePath);
			
			if(sImagePath.isEmpty())
			{
				//write warning to ease the configuration
				TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateImage: Visible object [%1]. Path not found for ImageID [%2] !")
											.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)).arg(pVirtObj->m_sImageID));
			}

		}
		else
			pVisibleImage->setImage("");

	}
}


void MIhmWebCommonObjLogic::processVirtIcon(MIhmVirtIco*pVirtObj)
{

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtAllObject:
				; //TODO update logo image
			break;

		case MIhmVirtualObject::enuIhmVirtIcoLaneStatus:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_STATUS, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoLaneMode:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_MODE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoCollector:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_COLLECTOR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoEntryGate:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_ENTRYGATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExitGate:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_EXITGATE, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoCar:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_CAR, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoTrafficLight:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_TRAFFIC, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoEntOpticalBarrier:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_ENT_OPTICAL_BARIER, pVirtObj);
			break;
			

		case MIhmVirtualObject::enuIhmVirtIcoViolation:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_VIOLATION, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoEntryLoop:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_ENTRYLOOP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExitLoop:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_EXITLOOP, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare1:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE1, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare2:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE2, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare3:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE3, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare4:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE4, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare5:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE5, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare6:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE6, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare7:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE7, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare8:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE8, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare9:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE9, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare10:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE10, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare11:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE11, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare12:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE12, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare13:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE13, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare14:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE14, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare15:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE15, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare16:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE16, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare17:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE17, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare18:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE18, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare19:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE19, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare20:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE20, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare21:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE21, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare22:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE22, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare23:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE23, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare24:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE24, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare25:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE25, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare26:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE26, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare27:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE27, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare28:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE28, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare29:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE29, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare30:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE30, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare31:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE31, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare32:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE32, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare33:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE33, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare34:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE34, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare35:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE35, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare36:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE36, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare37:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE37, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare38:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE38, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare39:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE39, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare40:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE40, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare41:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE41, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare42:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE42, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare43:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE43, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare44:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE44, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare45:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE45, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare46:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE46, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare47:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE47, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare48:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE48, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare49:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE49, pVirtObj);	break;
		case MIhmVirtualObject::enuIhmVirtIcoSpare50:updateImage(MIhmWebVisibleObject::enuICO_WEB_SPARE50, pVirtObj);	break;

		case MIhmVirtualObject::enuIhmVirtIcoCamera1:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_CAMERA1, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoCamera2:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_CAMERA2, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoBeacon:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_BEACON, pVirtObj);
			break;

		case MIhmVirtualObject::enuIhmVirtIcoBeacon2:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_BEACON2, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoSignalLight:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_SIGNAL_LIGHT, pVirtObj);
			break;
		case MIhmVirtualObject::enuIhmVirtIcoExtOpticalBarrier:
			updateImage(MIhmWebVisibleObject::enuICO_WEB_EXT_OPT_BARR, pVirtObj);
			break;


			//TODO - 
			//special image object to be loaded directly from file not by ID
			// and to take care of show hide of container and label itself 
			//we have to assure that the file is "visible" form the web 
			// and that the browser client will not use cash to show the image
		case MIhmVirtualObject::enuIhmVirtImage:
			updateDynImage(pVirtObj->m_sImageID, pVirtObj->isVisible(), MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE);
			break;
		case MIhmVirtualObject::enuIhmVirtImage2:
			updateDynImage(pVirtObj->m_sImageID, pVirtObj->isVisible(), MIhmWebVisibleObject::enuCNT_WEB_DYN_IMAGE2);
			break;
		default:
			TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processVirtIcon:Unknown virtual object: %1").arg(pVirtObj->getId()));
				

			break;
	}
}



void MIhmWebCommonObjLogic::updateTextLabel(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtText* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateTextLabel:[%1],visible:%2").
								arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)).arg(pVirtObj->isVisible()));

	MIhmWebVisibleTextLabel * pCurrent;
	pCurrent = (MIhmWebVisibleTextLabel *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateTextLabel: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlTextLabel)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateTextLabel: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateTextLabel: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}


	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
	QString sTranslation;
	pLang->getLabelTranslation(pVirtObj->m_sTextID, MIhmLanguages::enuTranslTargetWeb,sTranslation);

	pCurrent->setTextLabel(sTranslation);
	pCurrent->setVisible(pVirtObj->isVisible());

	if(pVirtObj->m_oPoliceValue.isEmpty()!=true)
	{
		QString sTextStyle = pVirtObj->m_oPoliceValue.getColorStyle();

		if(!sTextStyle.isEmpty())
		{
			pCurrent->setStyle(sTextStyle);
		}
	}

}

void MIhmWebCommonObjLogic::processVirtButton(MIhmVirtButton* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::processVirtButton ... "));
	
	switch(pVirtObj->getId())
	{

		case MIhmVirtualObject::enuIhmVirtBtnClass:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CLASS, pVirtObj);
				break;


		case MIhmVirtualObject::enuIhmVirtBtnEntryPoint:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnEntryPointNbr:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ENTRY_POINT_NBR, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnMode:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_MODE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnArrowOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ARROW_ON, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnArrowOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ARROW_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCrossOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CROSS_ON, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCrossOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CROSS_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnDsrcOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_DSRC_ON, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnDsrcOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_DSRC_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnMagneticOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_MAGNET_ON, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnMagneticOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_MAGNET_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCollectorOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_ON, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCollectorOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_COLLECTOR_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCamera:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAMERA, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnScanner:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SCANNER, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnComment:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_COMMENT, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnGabarite:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_GABARITE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnEntryGate:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ENTRY_GATE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnExitGate:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_EXIT_GATE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnCar3:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAR3, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCar2:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAR2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnCar:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAR, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnEntryLoop:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_ENTRY_LOOP, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnExitLoop:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_EXIT_LOOP, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnTblBeacon:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_TBL_BEACON, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnCamGrab:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAM_GRAB, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCamHide:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_CAM_HIDE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnScanGrab:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SCAN_GRAB, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnScanHide:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SCAN_HIDE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnScanSend:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SCAN_SEND, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnScanSave:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SCAN_SAVE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnCommHide:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_COMM_HIDE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnExternalDisplay:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnExternalDisplay2:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_EXTERNAL_DISPLAY2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnTakeCtrl:
			//DO not update the take control button depending on the model 
			// because it is updated in updateSessionInControlStatuses 
			break;
		case MIhmVirtualObject::enuIhmVirtBtnReturnCtrl:
			//DO not update the return control button depending on the model 
			// because it is updated in updateSessionInControlStatuses 
			break;

		case MIhmVirtualObject::enuIhmVirtButton1:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BUTTON1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtButton2:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BUTTON2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtButton3:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BUTTON3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtButton4:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BUTTON4, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtButton5:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BUTTON5, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_ON, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_SPEED_LIMIT_OFF, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnWarningOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_WARNING_ON, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnWarningOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_WARNING_OFF, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOn:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_ON, pVirtObj);
				break;
				
		case MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOff:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_DSRC_TSA_OFF, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare1:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare2:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare3:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare4:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_4, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare5:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_5, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare6:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_6, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare7:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_7, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare8:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_8, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare9:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_9, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare10:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_10, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare11:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_11, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare12:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_12, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare13:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_13, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare14:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_14, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare15:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_15, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare16:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_16, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare17:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_17, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare18:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_18, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare19:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_19, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare20:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_20, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare21:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_21, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare22:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_22, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare23:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_23, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare24:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_24, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare25:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_25, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare26:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_26, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare27:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_27, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare28:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_28, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare29:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_29, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBtnSpare30:
					updateHtmlButton(MIhmWebVisibleObject::enuBTN_WEB_BTN_SPARE_30, pVirtObj);
				break;

		default:
			TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processVirtButton: no action defined for the object ID: %1").arg((int)pVirtObj->getId()));

			break;
	}	


}


void MIhmWebCommonObjLogic::updateHtmlButton(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtButton* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateHtmlButton: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleButton * pCurrent;
	pCurrent = (MIhmWebVisibleButton *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateHtmlButton: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlButton)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateHtmlButton: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateHtmlButton: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	MIhmLanguages * pLang = MIhmConfigWeb::getCfg()->getLanguages();
	QString sTranslation;

	pLang->getLabelTranslation(pVirtObj->m_sTextID, MIhmLanguages::enuTranslTargetWeb, sTranslation);
	pCurrent->setTextLabel(sTranslation);

	QString sImagePath = MIhmConfigWeb::getCfg()->getWebConfigImages()->getImageFullPath(pVirtObj->m_sImageID);
	pCurrent->setIcon(sImagePath);
	
	pCurrent->setVisible(pVirtObj->isVisible());

	pCurrent->setEnabled(pVirtObj->isEnabled());
}


//--------------------------------------------------------------------------------
void MIhmWebCommonObjLogic::processVirtText(MIhmVirtText*pVirtObj)
{
	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtPlazaName:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName1:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName2:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaName3:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaNameTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NAME_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtPlazaNbr:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPlazaNbrTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PLAZA_NUMBER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNbr:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNbrTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NUMBER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName1:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneName2:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtLaneNameTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_LANE_NAME_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtCollectorID:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_ID, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtCollectorName:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_COL_NAME, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtSpare1:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare2:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare3:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE3, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare4:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE4, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare5:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE5, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare6:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE6, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare7:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE7, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare8:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE8, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare9:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE9, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare10:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE10, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare11:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE11, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare12:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE12, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare13:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE13, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare14:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE14, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare15:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE15, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare16:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE16, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare17:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE17, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare18:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE18, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare19:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE19, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare20:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE20, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtSpare21:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE21, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare22:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE22, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare23:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE23, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare24:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE24, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare25:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE25, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare26:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE26, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare27:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE27, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare28:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE28, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare29:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE29, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare30:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE30, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare31:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE31, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare32:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE32, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare33:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE33, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare34:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE34, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare35:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE35, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare36:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE36, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare37:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE37, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare38:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE38, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare39:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE39, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare40:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE40, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare41:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE41, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare42:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE42, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare43:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE43, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare44:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE44, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare45:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE45, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare46:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE46, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare47:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE47, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare48:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE48, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare49:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE49, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare50:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE50, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare51:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE51, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare52:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE52, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare53:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE53, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare54:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE54, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare55:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE55, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare56:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE56, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare57:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE57, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare58:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE58, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare59:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE59, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtSpare60:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SPARE60, pVirtObj);
				break;


		case MIhmVirtualObject::enuIhmVirtTransactionNbr:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTransactionNbrTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TRANSACTIONNBR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtReceiptNbr:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtReceiptNbrTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_RECEIPTNBR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAxles:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAxlesTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_AXLES_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBalance:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtBalanceTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_BALANCE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtClass:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtClassTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CLASS_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtAddRevenue:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtAddRevenueTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ADD_REVENUE_TITLE, pVirtObj);
				break;



		case MIhmVirtualObject::enuIhmVirtNbCar:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_NBCAR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtEntry:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtEntryTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ENTRY_TITLE, pVirtObj);
				break;

//NOT USED
// 		case MIhmVirtualObject::enuIhmVirtCurrency:
// 					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CURRENCY, pVirtObj);
// 				break;
// 		case MIhmVirtualObject::enuIhmVirtCurrencyTitle:
// 					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CURRENCY_TITLE, pVirtObj);
// 				break;

		case MIhmVirtualObject::enuIhmVirtFare:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtFareTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtSale:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDue:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDueTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtSaleDuePaid:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SALEDUE_PAID, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTransactionType:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTransactionTypeTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TYPE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtFareType:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtFareTypeTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_FARE_TYPE_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtModeTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtMode:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_MODE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtCollectorTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_COLLECTOR_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUserTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUser:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtInCtrlUser1:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_IN_CTRL_USER1, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtTrsContainerTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_CONTAINER_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtAlarmsTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALARMS_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtTrsHistoryTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_TRS_HISTORY_TITLE, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtAlertesTitle:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_ALERTES_TITLE, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtCamNumPic:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_CAM_NUM_PIC, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtTxtScanNumPic:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_SCAN_NUM_PIC, pVirtObj);
				break;

		case MIhmVirtualObject::enuIhmVirtTxtNbCarSAS:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_TXT_NB_CARS_SAS, pVirtObj);
				break;

		//INSTRUCTIONS
		case MIhmVirtualObject::enuIhmVirtErrPayment:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_ERR_PAYMENT, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtPayment:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_PAYMENT, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtRemark:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_REMARK, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtInstruction:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtInstruction2:
					updateTextLabel(MIhmWebVisibleObject::enuTEXT_LABEL_WEB_INSTR_2, pVirtObj);
				break;
		case MIhmVirtualObject::enuIhmVirtCurrentDate:
					; //updated by the clock
				break;
		default:
			TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processVirtText: no action defined for the object ID: %1").arg((int)pVirtObj->getId()));

			break;
	}	

}

void MIhmWebCommonObjLogic::processVirtList(MIhmVirtList*pVirtObj)
{
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processVirtList: ...");

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtLstAlarms:
					updateListContainer(MIhmWebVisibleObject::enuLIST_WEB_LIST_ALARMS, pVirtObj);
				break;
	
		case MIhmVirtualObject::enuIhmVirtLstWarnings:
					updateListContainer(MIhmWebVisibleObject::enuLIST_WEB_LIST_WARNINGS, pVirtObj);
				break;
		default:
			TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processVirtList: no action defined for the object ID: %1").arg((int)pVirtObj->getId()));

			break;
	
	}

}


void MIhmWebCommonObjLogic::updateListContainer(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtList* pVirtObj)
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateListContainer: %1").arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));

	MIhmWebVisibleObject * pCurrent;
	pCurrent = (MIhmWebVisibleObject *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateListContainer: findVisibleObjectByName return NULL for the visible object name %1")
										.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlList)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateListContainer: invalid visible object type: %1")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	if(pCurrent->isNotUsed()) 
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateListContainer: Visible object [%1] is flagged NOT_USED in configuration!")
									.arg(MIhmWebVisibleObject::getVisibleObjNameForID(eId)));
		return;
	}

	MIhmWebVisibleList * pList = (MIhmWebVisibleList *)pCurrent;
	pList->setStringList(pVirtObj->m_slistIDs);

	if(!(pVirtObj->m_oPoliceValue.isEmpty()))
	{
		QString sStyle = pVirtObj->m_oPoliceValue.getColorStyle();

		if(sStyle!="")
		{
			TRACE_WEB_D(QString( "MIhmWebCommonObjLogic::updateListContainer: style: %1")
								.arg(sStyle));
			pList->setStyle(sStyle);
		}
	}

}



void MIhmWebCommonObjLogic::processVirtStrDetConfig(MIhmVirtStrDetectConfig*pVirtObj)
{
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processVirtStrDetConfig: String detection is not used at the web interface!");
	
}

void MIhmWebCommonObjLogic::processVirtConfig(MIhmVirtDynConfig* pVirtObj)
{

	switch(pVirtObj->getId())
	{
		case MIhmVirtualObject::enuIhmVirtConfProjectVersion:
				processConfigPrjVersion(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfProjectName:
				//project name and/or version updated
				processConfigPrjName(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfLaneType:
				processConfigLaneType(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfShiftType:
				processConfigShiftType(pVirtObj);
			break;		
		case MIhmVirtualObject::enuIhmVirtConfLanguage:
				//already processed at higher level in function:
				// MIhmWebInterface::processVirtConfigLanguage

			break;
		case MIhmVirtualObject::enuIhmVirtConfMainDlgHidden:
			//this is not implemented for web
			break;

		default:
			TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processVirtConfig: unknown object id %1").arg(pVirtObj->getId()));
			break;

	}
}

void MIhmWebCommonObjLogic::processConfigShiftType(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewShiftType;
	
	sNewShiftType = pVirtObj->m_sDynConfigSetting;
	
	QString sShiftColour;
	
	if(MIhmConfigWeb::getCfg()->getConfigColors()->getColourForShift(sNewShiftType, sShiftColour))
	{
		TRACE_WEB_D( QString("MIhmWebCommonObjLogic::processConfigShiftType: shift:%1, colour:%2") 
											.arg(sNewShiftType)
											.arg(sShiftColour));
		MIhmWebVisibleObject * pCurrent;

		pCurrent = MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, MIhmWebVisibleObject::enuCNT_WEB_MAIN_DIALOG);
		
		if(pCurrent==NULL)
		{
			TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processConfigShiftType:  findVisibleObjectID %1 return NULL").arg(CNT_REG_VAL_WEB_MAIN_DIALOG));
		}
		else
			pCurrent->setStyle(	MIhmConfigColor::createBkgColorStyle(sShiftColour));
	}
	else
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processConfigShiftType: ERROR No shift colour defined for %1")
							.arg(sNewShiftType));
	}

}


void MIhmWebCommonObjLogic::processConfigPrjName(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewPrjName = pVirtObj->m_sDynConfigSetting; 
	
	MIhmWebGenericScript * pCurrent;

	pCurrent = (MIhmWebGenericScript*)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, MIhmWebVisibleObject::enuCMD_WEB_WINDOW_TITLE);
	
	if(pCurrent==NULL)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::processConfigPrjName:  findVisibleObjectByID: enuCMD_WEB_WINDOW_TITLE return NULL").arg(MIhmWebVisibleObject::enuCMD_WEB_WINDOW_TITLE));
	}
	else
	{
		pCurrent->resetParameters();
		pCurrent->updateParameter("%PARAMETER_1%", sNewPrjName);
	}

}

void MIhmWebCommonObjLogic::processConfigPrjVersion(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewPrjVersion = pVirtObj->m_sDynConfigSetting; 
	TRACE_WEB_D( "MIhmWebCommonObjLogic::processConfigPrjVersion:It is not dynamicaly set, but is part of the web configuration parameter!");
	
}





void MIhmWebCommonObjLogic::processConfigLaneType(MIhmVirtDynConfig* pVirtObj)
{
	QString sNewLaneType;
	
	sNewLaneType = pVirtObj->m_sDynConfigSetting;

	MIhmConfigWeb *pCfg = MIhmConfigWeb::getCfg();

	MIhmLaneTypeSettings *pLaneSettings = pCfg->getLaneTypeSetting(MIhmLaneTypeSettings::enuWEB,sNewLaneType);

	if(pLaneSettings!=NULL)
	{
		//load new HTML templates to all objects
		for(int i=MIhmWebVisibleObject::enuWebVisibleObjectUnknown+1;i<m_vectCommonWebVisibleObjects.size();i++)
		{
			if(m_vectCommonWebVisibleObjects.at(i)==NULL)
				continue;
	
			MIhmWebVisibleObject * pCurrent = m_vectCommonWebVisibleObjects.at(i);

			QString sObjName = pCurrent->getName();

			//init initial visibility style 
			QString sVisible = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_VISIBLE);
			
			if(sVisible.compare("0",Qt::CaseInsensitive) == 0 || sVisible.compare("false",Qt::CaseInsensitive)==0)
				pCurrent->setVisible(false);
			else
				pCurrent->setVisible(true);

			QString sUsedAtMiniWeb = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_USED_AT_MINI_WEB);
			
			if(sUsedAtMiniWeb.compare("1",Qt::CaseInsensitive) == 0 || sUsedAtMiniWeb.compare("true",Qt::CaseInsensitive)==0)
				pCurrent->setUsedAtMiniWeb(true);
			else
				pCurrent->setUsedAtMiniWeb(false);
			
			
			//init NOT_USED flag
			QString sNotUsed = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_NOT_USED);
			
			if(sNotUsed.compare("1",Qt::CaseInsensitive) == 0 || sNotUsed.compare("true",Qt::CaseInsensitive)==0)
				pCurrent->setNotUsed(true);
			else
				pCurrent->setNotUsed(false);

			//check for action_id if any
			QString sActionId = pLaneSettings->getVisObjParamValue(sObjName, CFG_VISIBLE_OBJ_ATTRIBUTE_ACTION_ID);

			if(!sActionId.isEmpty())
			{
				pCurrent->setSingleAction(sActionId);
			}

			if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlImage ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlDynImage ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTextLabel||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlRVideoView)
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlList||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInputDialog ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTableView ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlHMenuView ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlButton ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlHPrioInputDialog ||
				pCurrent->getType() == MIhmWebVisibleObject::enuHtmlTabView)
			{ 
				  //if the web visible object is more complex we 
				  //also need to initialize sub templates
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
				pCurrent->initSubTemplates(pLaneSettings, sObjName);
			}	
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInputDialogEx)
			{
				//No need to initialize DlgInputDialogEx template because 
				//each DialogId (defined in registry InputEx) contains a different template file
				//Names from definition and values xml files are used 
				//to generate the final structure of html for the dialog 
				;
			}	
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlInControlMode)
			{
				//NO NEED to initialize. No style nor html for this object only current mode is kept to 
				// send ChangeMode command when updated!
				;
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlOpenLink)
			{
				//NO NEED to initialize. No style nor html for this object only current mode is kept to 
				// send OpenLink command when updated!
				;
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlContainer || 
					pCurrent->getType() == MIhmWebVisibleObject::enuHtmlCtrlDependContainer)
			{
				//NO NEED to initialize: For containers only additional style is updated 
				;
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlAsyncInput)
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlGenericScript)
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
			}
			else if(pCurrent->getType() == MIhmWebVisibleObject::enuHtmlGenericCtrlDependentScript)
			{
				QString sHtmlTemplate = pLaneSettings->getVisObjHtmlTemplate(sObjName);
				pCurrent->initTemplateHtml(sHtmlTemplate);
				pCurrent->initSubTemplates(pLaneSettings, sObjName);
				
				MParamList oLstParams;

				if(pCfg->getGenericScriptParameters(sObjName,  &oLstParams))
				{
					((MIhmWebGenericCtrlDependentScript*)pCurrent)->updateStaticParemeters(&oLstParams);
				}
				else
				{
					TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processConfigLaneType:Generic script parameters (reg key: \\GenericScripts\\%1) not found") 
									.arg(sObjName));
				}

			}
			else
			{
				TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processConfigLaneType:Error unknown web visible object type:%1") 
								.arg((int)pCurrent->getType()));

			}
							
		}	

	}
	else
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::processConfigLaneType:Error loading LaneType:%1") 
					.arg(sNewLaneType.toLatin1().data()));
	}

}








void MIhmWebCommonObjLogic::updateTheDateObject()
{
	TRACE_WEB_D( QString("MIhmWebCommonObjLogic::updateTheDateObject..."));

	MIhmWebVisibleTextLabel * pCurrent;
	pCurrent = (MIhmWebVisibleTextLabel *)MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, MIhmWebVisibleObject::enuCLK_WEB_DATE);

	if(pCurrent==NULL)
	{
		TRACE_WEB_W(QString("MIhmWebCommonObjLogic::updateTheDateObject: findVisibleObjectByID return NULL for the visible object name %1")
										.arg(CLK_REG_VAL_WEB_DATE));
		return;
	}

	if(pCurrent->getType()!=MIhmWebVisibleObject::enuHtmlTextLabel)
	{
		TRACE_WEB_W( QString("MIhmWebCommonObjLogic::updateTheDateObject: invalid visible object type: %1")
									.arg(CLK_REG_VAL_WEB_DATE));
		return;
	}

	QDate dt = QDate::currentDate();
	QString sDateFormat = MIhmConfigWeb::getCfg()->getDefaultDateFormat();
	QString sDateText = dt.toString(sDateFormat);//"dd/MM/yyyy"
	
	sDateText = MIhmConfigWeb::getCfg()->htmlEncodeStr(sDateText);
	pCurrent->setTextLabel(sDateText);
	pCurrent->setVisible(true);
	pCurrent->forceUpdate();
}





