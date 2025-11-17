// SimuSendReciveThread.cpp: implementation of the SimuSendReciveThread class.
//
//////////////////////////////////////////////////////////////////////

#include <QFile>
#include <QDomDocument>
#include <QTime>
#include <QTextStream>
#include <QCoreApplication>


extern "C"
{
	#include <noyau.h>
	#include <ihm.h>
}


#include "SimuSendReciveThread.h"
#include "constants.h"


int SimuSendReciveThread::getCommandIdFromString(QString sObjectName)
{
	if(QString::compare(sObjectName, "IHM_VIDEO_CMD_UNKNOWN", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_CMD_UNKNOWN;
	else if(QString::compare(sObjectName, "IHM_VIDEO_ON", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_ON;
	else if(QString::compare(sObjectName, "IHM_VIDEO_OFF", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_OFF;
	else if(QString::compare(sObjectName, "IHM_VIDEO_FREEZE", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_FREEZE;
	else if(QString::compare(sObjectName, "IHM_VIDEO_UNFREEZE", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_UNFREEZE;
	else if(QString::compare(sObjectName, "IHM_VIDEO_ZOOM_IN", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_ZOOM_IN;
	else if(QString::compare(sObjectName, "IHM_VIDEO_ZOOM_OUT", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_ZOOM_OUT;
	else if(QString::compare(sObjectName, "IHM_VIDEO_ZOOM", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_ZOOM;
	else if(QString::compare(sObjectName, "IHM_VIDEO_SELECT_SRC", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_SELECT_SRC;
	else if(QString::compare(sObjectName, "IHM_VIDEO_SAVE_BMP", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_SAVE_BMP;
	else if(QString::compare(sObjectName, "IHM_VIDEO_SAVE_JPG", Qt::CaseInsensitive)==0)
		return IHM_VIDEO_SAVE_JPG;
	else
		return -1;
		
}

QString SimuSendReciveThread::getNameForAuthReasonId(int iReasonId)
{
	switch((enum_ihm_auth_ret_ctrl_reason)iReasonId)
	{
	case enuRETURN_CTRL_VOLUNTARY:
		return "enuRETURN_CTRL_VOLUNTARY";
	case enuRETURN_CTRL_FORCED:
		return "enuRETURN_CTRL_FORCED";
	case enuRETURN_CTRL_IDLE_TIMEOUT:
		return "enuRETURN_CTRL_IDLE_TIMEOUT";
	case enuRETURN_CTRL_TAKEOVER_TIMEOUT:
		return "enuRETURN_CTRL_TAKEOVER_TIMEOUT";
	default:
		return "UNKNOWN REASON";
	}
		
}

QString SimuSendReciveThread::getVideoCmdString(int iCmdId)
{
	switch(iCmdId)
	{
		case IHM_VIDEO_ON:return QString("IHM_VIDEO_ON");
		case IHM_VIDEO_OFF:return QString("IHM_VIDEO_OFF");
		case IHM_VIDEO_FREEZE:return QString("IHM_VIDEO_FREEZE");
		case IHM_VIDEO_UNFREEZE:return QString("IHM_VIDEO_UNFREEZE");
		case IHM_VIDEO_ZOOM_IN:return QString("IHM_VIDEO_ZOOM_IN");
		case IHM_VIDEO_ZOOM_OUT:return QString("IHM_VIDEO_ZOOM_OUT");
		case IHM_VIDEO_ZOOM:return QString("IHM_VIDEO_ZOOM");
		case IHM_VIDEO_SELECT_SRC:return QString("IHM_VIDEO_SELECT_SRC");
		case IHM_VIDEO_SAVE_BMP:return QString("IHM_VIDEO_SAVE_BMP");
		case IHM_VIDEO_SAVE_JPG:return QString("IHM_VIDEO_SAVE_JPG");
		case IHM_VIDEO_SAVE_OK:return QString("IHM_VIDEO_SAVE_OK");
		case IHM_VIDEO_SAVE_NOK:return QString("IHM_VIDEO_SAVE_NOK");
		case IHM_VIDEO_FREEZE_OK:return QString("IHM_VIDEO_FREEZE_OK");
		case IHM_VIDEO_FREEZE_NOK:return QString("IHM_VIDEO_FREEZE_NOK");		
		
		default:
			return "IHM_VIDEO_CMD_UNKNOWN";
			break;
	}
}



QString SimuSendReciveThread::getTableViewCmdString(int iCmdId)
{
	switch(iCmdId)
	{
		case enuIHM_AFF_TABLE_CMD_PREPEND_ROWS:return QString("PREPEND");
		case enuIHM_AFF_TABLE_CMD_APPEND_ROWS:return QString("APPEND");
		case enuIHM_AFF_TABLE_CMD_RESET_ROWS:return QString("RESET");
		case enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS:return QString("RESET_AND_APPEND");
					
		default:
			return "TABLE_CMD_UNKNOWN";
	}
}


int SimuSendReciveThread::getTableViewCmdFromString(QString sCommand)
{

	for(int i=0;i<enuIHM_AFF_TABLE_CMD_COUNT;i++)
	{
		if(sCommand.compare(getTableViewCmdString(i),Qt::CaseInsensitive)==0)
			return i;
	}

	return enuIHM_AFF_TABLE_CMD_UNKNOWN;
}





QString SimuSendReciveThread::getStringFromObjectId(int iObjectId)
{
	switch(iObjectId)
	{
	case IHM_OBJ_All_Object:return QString("IHM_OBJ_All_Object");

// Labels
	case IHM_OBJ_PlazaName:			return QString("IHM_OBJ_PlazaName");         
	case IHM_OBJ_PlazaName1:		return QString("IHM_OBJ_PlazaName1");         
	case IHM_OBJ_PlazaName2:		return QString("IHM_OBJ_PlazaName2");         
	case IHM_OBJ_PlazaName3:		return QString("IHM_OBJ_PlazaName3");         
	case IHM_OBJ_PlazaNameTitle:	return QString("IHM_OBJ_PlazaNameTitle");
	case IHM_OBJ_PlazaNbr:			return QString("IHM_OBJ_PlazaNbr");
	case IHM_OBJ_PlazaNbrTitle:		return QString("IHM_OBJ_PlazaNbrTitle");
	case IHM_OBJ_LaneNbr:			return QString("IHM_OBJ_LaneNbr");
	case IHM_OBJ_LaneNbrTitle:		return QString("IHM_OBJ_LaneNbrTitle");
	case IHM_OBJ_CollectorID:		return QString("IHM_OBJ_CollectorID");
	case IHM_OBJ_CollectorName:		return QString("IHM_OBJ_CollectorName");      

	case IHM_OBJ_TxtSpare1:			return QString("IHM_OBJ_TxtSpare1");		
	case IHM_OBJ_TxtSpare2:			return QString("IHM_OBJ_TxtSpare2");			
	case IHM_OBJ_TxtSpare3:			return QString("IHM_OBJ_TxtSpare3");
	case IHM_OBJ_TxtSpare4:			return QString("IHM_OBJ_TxtSpare4");
	case IHM_OBJ_TxtSpare5:			return QString("IHM_OBJ_TxtSpare5");
	case IHM_OBJ_TxtSpare6:			return QString("IHM_OBJ_TxtSpare6");
	case IHM_OBJ_TxtSpare7:			return QString("IHM_OBJ_TxtSpare7");
	case IHM_OBJ_TxtSpare8:			return QString("IHM_OBJ_TxtSpare8");
	case IHM_OBJ_TxtSpare9:			return QString("IHM_OBJ_TxtSpare9");
	case IHM_OBJ_TxtSpare10:		return QString("IHM_OBJ_TxtSpare10");
	case IHM_OBJ_TxtSpare11:		return QString("IHM_OBJ_TxtSpare11");
	case IHM_OBJ_TxtSpare12:		return QString("IHM_OBJ_TxtSpare12");
	case IHM_OBJ_TxtSpare13:		return QString("IHM_OBJ_TxtSpare13");
	case IHM_OBJ_TxtSpare14:		return QString("IHM_OBJ_TxtSpare14");
	case IHM_OBJ_TxtSpare15:		return QString("IHM_OBJ_TxtSpare15");
	case IHM_OBJ_TxtSpare16:		return QString("IHM_OBJ_TxtSpare16");
	case IHM_OBJ_TxtSpare17:		return QString("IHM_OBJ_TxtSpare17");
	case IHM_OBJ_TxtSpare18:		return QString("IHM_OBJ_TxtSpare18");
	case IHM_OBJ_TxtSpare19:		return QString("IHM_OBJ_TxtSpare19");
	case IHM_OBJ_TxtSpare20:		return QString("IHM_OBJ_TxtSpare20");

	case IHM_OBJ_TxtSpare21:			return QString("IHM_OBJ_TxtSpare21");		
	case IHM_OBJ_TxtSpare22:			return QString("IHM_OBJ_TxtSpare22");			
	case IHM_OBJ_TxtSpare23:			return QString("IHM_OBJ_TxtSpare23");
	case IHM_OBJ_TxtSpare24:			return QString("IHM_OBJ_TxtSpare24");
	case IHM_OBJ_TxtSpare25:			return QString("IHM_OBJ_TxtSpare25");
	case IHM_OBJ_TxtSpare26:			return QString("IHM_OBJ_TxtSpare26");
	case IHM_OBJ_TxtSpare27:			return QString("IHM_OBJ_TxtSpare27");
	case IHM_OBJ_TxtSpare28:			return QString("IHM_OBJ_TxtSpare28");
	case IHM_OBJ_TxtSpare29:			return QString("IHM_OBJ_TxtSpare29");
	case IHM_OBJ_TxtSpare30:			return QString("IHM_OBJ_TxtSpare30");

	case IHM_OBJ_TxtSpare31:			return QString("IHM_OBJ_TxtSpare31");		
	case IHM_OBJ_TxtSpare32:			return QString("IHM_OBJ_TxtSpare32");			
	case IHM_OBJ_TxtSpare33:			return QString("IHM_OBJ_TxtSpare33");
	case IHM_OBJ_TxtSpare34:			return QString("IHM_OBJ_TxtSpare34");
	case IHM_OBJ_TxtSpare35:			return QString("IHM_OBJ_TxtSpare35");
	case IHM_OBJ_TxtSpare36:			return QString("IHM_OBJ_TxtSpare36");
	case IHM_OBJ_TxtSpare37:			return QString("IHM_OBJ_TxtSpare37");
	case IHM_OBJ_TxtSpare38:			return QString("IHM_OBJ_TxtSpare38");
	case IHM_OBJ_TxtSpare39:			return QString("IHM_OBJ_TxtSpare39");
	case IHM_OBJ_TxtSpare40:			return QString("IHM_OBJ_TxtSpare40");

	case IHM_OBJ_TxtSpare41:			return QString("IHM_OBJ_TxtSpare41");		
	case IHM_OBJ_TxtSpare42:			return QString("IHM_OBJ_TxtSpare42");			
	case IHM_OBJ_TxtSpare43:			return QString("IHM_OBJ_TxtSpare43");
	case IHM_OBJ_TxtSpare44:			return QString("IHM_OBJ_TxtSpare44");
	case IHM_OBJ_TxtSpare45:			return QString("IHM_OBJ_TxtSpare45");
	case IHM_OBJ_TxtSpare46:			return QString("IHM_OBJ_TxtSpare46");
	case IHM_OBJ_TxtSpare47:			return QString("IHM_OBJ_TxtSpare47");
	case IHM_OBJ_TxtSpare48:			return QString("IHM_OBJ_TxtSpare48");
	case IHM_OBJ_TxtSpare49:			return QString("IHM_OBJ_TxtSpare49");
	case IHM_OBJ_TxtSpare50:			return QString("IHM_OBJ_TxtSpare50");

	case IHM_OBJ_TxtSpare51:			return QString("IHM_OBJ_TxtSpare51");		
	case IHM_OBJ_TxtSpare52:			return QString("IHM_OBJ_TxtSpare52");			
	case IHM_OBJ_TxtSpare53:			return QString("IHM_OBJ_TxtSpare53");
	case IHM_OBJ_TxtSpare54:			return QString("IHM_OBJ_TxtSpare54");
	case IHM_OBJ_TxtSpare55:			return QString("IHM_OBJ_TxtSpare55");
	case IHM_OBJ_TxtSpare56:			return QString("IHM_OBJ_TxtSpare56");
	case IHM_OBJ_TxtSpare57:			return QString("IHM_OBJ_TxtSpare57");
	case IHM_OBJ_TxtSpare58:			return QString("IHM_OBJ_TxtSpare58");
	case IHM_OBJ_TxtSpare59:			return QString("IHM_OBJ_TxtSpare59");
	case IHM_OBJ_TxtSpare60:			return QString("IHM_OBJ_TxtSpare60");


	case IHM_OBJ_TransactionNbr:		return QString("IHM_OBJ_TransactionNbr");		
	case IHM_OBJ_TransactionNbrTitle:	return QString("IHM_OBJ_TransactionNbrTitle");
	case IHM_OBJ_ReceiptNbr:			return QString("IHM_OBJ_ReceiptNbr");
	case IHM_OBJ_ReceiptNbrTitle:		return QString("IHM_OBJ_ReceiptNbrTitle");
	case IHM_OBJ_Axles:					return QString("IHM_OBJ_Axles");
	case IHM_OBJ_AxlesTitle:			return QString("IHM_OBJ_AxlesTitle");
	case IHM_OBJ_Balance:				return QString("IHM_OBJ_Balance");	
	case IHM_OBJ_BalanceTitle:			return QString("IHM_OBJ_BalanceTitle");
	case IHM_OBJ_Class:					return QString("IHM_OBJ_Class");              
	case IHM_OBJ_ClassTitle:			return QString("IHM_OBJ_ClassTitle"); 
	case IHM_OBJ_AdditionalRevenue:		return QString("IHM_OBJ_AdditionalRevenue");              
	case IHM_OBJ_AdditionalRevenueTitle:return QString("IHM_OBJ_AdditionalRevenueTitle"); 

	case IHM_OBJ_NbCar:					return QString("IHM_OBJ_NbCar");				
	case IHM_OBJ_Entry:					return QString("IHM_OBJ_Entry");
	case IHM_OBJ_EntryTitle:			return QString("IHM_OBJ_EntryTitle");
	case IHM_OBJ_Currency:				return QString("IHM_OBJ_Currency");
	case IHM_OBJ_CurrencyTitle:			return QString("IHM_OBJ_CurrencyTitle");
	case IHM_OBJ_Fare:					return QString("IHM_OBJ_Fare");
	case IHM_OBJ_FareTitle:				return QString("IHM_OBJ_FareTitle");		
	case IHM_OBJ_Sale:					return QString("IHM_OBJ_Sale");
	case IHM_OBJ_SaleTitle:				return QString("IHM_OBJ_SaleTitle");
	case IHM_OBJ_SaleDue:				return QString("IHM_OBJ_SaleDue");			
	case IHM_OBJ_SaleDueTitle:			return QString("IHM_OBJ_SaleDueTitle");	
	case IHM_OBJ_SaleDuePaid:			return QString("IHM_OBJ_SaleDuePaid");			
	case IHM_OBJ_TransactionType:		return QString("IHM_OBJ_TransactionType");
	case IHM_OBJ_TransactionTypeTitle:	return QString("IHM_OBJ_TransactionTypeTitle");
	case IHM_OBJ_LaneName:				return QString("IHM_OBJ_LaneName");          
	case IHM_OBJ_LaneName1:				return QString("IHM_OBJ_LaneName1");          
	case IHM_OBJ_LaneName2:				return QString("IHM_OBJ_LaneName2");          
	case IHM_OBJ_LaneNameTitle:			return QString("IHM_OBJ_LaneNameTitle");	
	case IHM_OBJ_FareType:				return QString("IHM_OBJ_FareType");
	case IHM_OBJ_FareTypeTitle:			return QString("IHM_OBJ_FareTypeTitle");

	case IHM_OBJ_TxtModeTitle:			return QString("IHM_OBJ_TxtModeTitle");		
	case IHM_OBJ_TxtMode:				return QString("IHM_OBJ_TxtMode");		
	case IHM_OBJ_TxtCollectorTitle:		return QString("IHM_OBJ_TxtCollectorTitle");		
	case IHM_OBJ_TxtInCtrlUserTitle:	return QString("IHM_OBJ_TxtInCtrlUserTitle");		
	case IHM_OBJ_TxtInCtrlUser:			return QString("IHM_OBJ_TxtInCtrlUser");		
	case IHM_OBJ_TxtInCtrlUser1:		return QString("IHM_OBJ_TxtInCtrlUser1");		
	case IHM_OBJ_TxtTrsContainerTitle:	return QString("IHM_OBJ_TxtTrsContainerTitle");		
	case IHM_OBJ_TxtAlarmsTitle:		return QString("IHM_OBJ_TxtAlarmsTitle");		
	case IHM_OBJ_TxtTrsHistoryTitle:	return QString("IHM_OBJ_TxtTrsHistoryTitle");		
	case IHM_OBJ_TxtAlertesTitle:		return QString("IHM_OBJ_TxtAlertesTitle");		
	case IHM_OBJ_TxtCamNumPic:			return QString("IHM_OBJ_TxtCamNumPic");
	case IHM_OBJ_TxtScanNumPic:			return QString("IHM_OBJ_TxtScanNumPic");
	case IHM_OBJ_TxtNbCarSAS:			return QString("IHM_OBJ_TxtNbCarSAS");

// Icons
	case IHM_OBJ_IcoLaneStatus:			return QString("IHM_OBJ_IcoLaneStatus");
	case IHM_OBJ_IcoLaneMode:			return QString("IHM_OBJ_IcoLaneMode");
	case IHM_OBJ_IcoCollector:			return QString("IHM_OBJ_IcoCollector");       
	case IHM_OBJ_IcoEntryGate:			return QString("IHM_OBJ_IcoEntryGate");		
	case IHM_OBJ_IcoExitGate:			return QString("IHM_OBJ_IcoExitGate");		
	case IHM_OBJ_IcoCar:				return QString("IHM_OBJ_IcoCar");
	case IHM_OBJ_IcoTrafficLight:		return QString("IHM_OBJ_IcoTrafficLight");
	case IHM_OBJ_IcoViolation:			return QString("IHM_OBJ_IcoViolation");       
	case IHM_OBJ_IcoEntryLoop:			return QString("IHM_OBJ_IcoEntryLoop");		
	case IHM_OBJ_IcoExitLoop:			return QString("IHM_OBJ_IcoExitLoop");
	case IHM_OBJ_IcoBeacon:				return QString("IHM_OBJ_IcoBeacon");
	case IHM_OBJ_IcoEntOpticalBarrier:	return QString("IHM_OBJ_IcoEntOpticalBarrier");
	case IHM_OBJ_IcoSpare1:				return QString("IHM_OBJ_IcoSpare1");
	case IHM_OBJ_IcoSpare2:				return QString("IHM_OBJ_IcoSpare2");
	case IHM_OBJ_IcoSpare3:				return QString("IHM_OBJ_IcoSpare3");
	case IHM_OBJ_IcoSpare4:				return QString("IHM_OBJ_IcoSpare4");
	case IHM_OBJ_IcoSpare5:				return QString("IHM_OBJ_IcoSpare5");
	case IHM_OBJ_IcoSpare6:				return QString("IHM_OBJ_IcoSpare6");
	case IHM_OBJ_IcoSpare7:				return QString("IHM_OBJ_IcoSpare7");
	case IHM_OBJ_IcoSpare8:				return QString("IHM_OBJ_IcoSpare8");
	case IHM_OBJ_IcoSpare9:				return QString("IHM_OBJ_IcoSpare9");
	case IHM_OBJ_IcoSpare10:				return QString("IHM_OBJ_IcoSpare10");
	case IHM_OBJ_IcoSpare11:				return QString("IHM_OBJ_IcoSpare11");
	case IHM_OBJ_IcoSpare12:				return QString("IHM_OBJ_IcoSpare12");
	case IHM_OBJ_IcoSpare13:				return QString("IHM_OBJ_IcoSpare13");
	case IHM_OBJ_IcoSpare14:				return QString("IHM_OBJ_IcoSpare14");
	case IHM_OBJ_IcoSpare15:				return QString("IHM_OBJ_IcoSpare15");
	case IHM_OBJ_IcoSpare16:				return QString("IHM_OBJ_IcoSpare16");
	case IHM_OBJ_IcoSpare17:				return QString("IHM_OBJ_IcoSpare17");
	case IHM_OBJ_IcoSpare18:				return QString("IHM_OBJ_IcoSpare18");
	case IHM_OBJ_IcoSpare19:				return QString("IHM_OBJ_IcoSpare19");
	case IHM_OBJ_IcoSpare20:				return QString("IHM_OBJ_IcoSpare20");
	case IHM_OBJ_IcoSpare21:				return QString("IHM_OBJ_IcoSpare21");
	case IHM_OBJ_IcoSpare22:				return QString("IHM_OBJ_IcoSpare22");
	case IHM_OBJ_IcoSpare23:				return QString("IHM_OBJ_IcoSpare23");
	case IHM_OBJ_IcoSpare24:				return QString("IHM_OBJ_IcoSpare24");
	case IHM_OBJ_IcoSpare25:				return QString("IHM_OBJ_IcoSpare25");
	case IHM_OBJ_IcoSpare26:				return QString("IHM_OBJ_IcoSpare26");
	case IHM_OBJ_IcoSpare27:				return QString("IHM_OBJ_IcoSpare27");
	case IHM_OBJ_IcoSpare28:				return QString("IHM_OBJ_IcoSpare28");
	case IHM_OBJ_IcoSpare29:				return QString("IHM_OBJ_IcoSpare29");
	case IHM_OBJ_IcoSpare30:				return QString("IHM_OBJ_IcoSpare30");
	case IHM_OBJ_IcoSpare31:				return QString("IHM_OBJ_IcoSpare31");
	case IHM_OBJ_IcoSpare32:				return QString("IHM_OBJ_IcoSpare32");
	case IHM_OBJ_IcoSpare33:				return QString("IHM_OBJ_IcoSpare33");
	case IHM_OBJ_IcoSpare34:				return QString("IHM_OBJ_IcoSpare34");
	case IHM_OBJ_IcoSpare35:				return QString("IHM_OBJ_IcoSpare35");
	case IHM_OBJ_IcoSpare36:				return QString("IHM_OBJ_IcoSpare36");
	case IHM_OBJ_IcoSpare37:				return QString("IHM_OBJ_IcoSpare37");
	case IHM_OBJ_IcoSpare38:				return QString("IHM_OBJ_IcoSpare38");
	case IHM_OBJ_IcoSpare39:				return QString("IHM_OBJ_IcoSpare39");
	case IHM_OBJ_IcoSpare40:				return QString("IHM_OBJ_IcoSpare40");
	case IHM_OBJ_IcoSpare41:				return QString("IHM_OBJ_IcoSpare41");
	case IHM_OBJ_IcoSpare42:				return QString("IHM_OBJ_IcoSpare42");
	case IHM_OBJ_IcoSpare43:				return QString("IHM_OBJ_IcoSpare43");
	case IHM_OBJ_IcoSpare44:				return QString("IHM_OBJ_IcoSpare44");
	case IHM_OBJ_IcoSpare45:				return QString("IHM_OBJ_IcoSpare45");
	case IHM_OBJ_IcoSpare46:				return QString("IHM_OBJ_IcoSpare46");
	case IHM_OBJ_IcoSpare47:				return QString("IHM_OBJ_IcoSpare47");
	case IHM_OBJ_IcoSpare48:				return QString("IHM_OBJ_IcoSpare48");
	case IHM_OBJ_IcoSpare49:				return QString("IHM_OBJ_IcoSpare49");
	case IHM_OBJ_IcoSpare50:				return QString("IHM_OBJ_IcoSpare50");

	case IHM_OBJ_IcoCamera1:			return QString("IHM_OBJ_IcoCamera1");
	case IHM_OBJ_IcoCamera2:			return QString("IHM_OBJ_IcoCamera2");

	case IHM_OBJ_IcoBeacon2:			return QString("IHM_OBJ_IcoBeacon2");
	case IHM_OBJ_IcoSignalLight:		return QString("IHM_OBJ_IcoSignalLight");
	case IHM_OBJ_IcoExtOpticalBarrier:	return QString("IHM_OBJ_IcoExtOpticalBarrier");


// Instruction 
	case IHM_OBJ_ErrPayment:	return QString("IHM_OBJ_ErrPayment");		
	case IHM_OBJ_Payment:		return QString("IHM_OBJ_Payment");
	case IHM_OBJ_Remark:		return QString("IHM_OBJ_Remark");
	case IHM_OBJ_Instruction:	return QString("IHM_OBJ_Instruction");   
	case IHM_OBJ_Instruction2:	return QString("IHM_OBJ_Instruction2");
	case IHM_OBJ_CurrentDate:	return QString("IHM_OBJ_CurrentDate");	

//lists
	case IHM_OBJ_LstAlarms:		return QString("IHM_OBJ_LstAlarms");
	case IHM_OBJ_LstWarnings:	return QString("IHM_OBJ_LstWarnings");

//dynamic image (specific icon)
	case IHM_OBJ_Image:		return QString("IHM_OBJ_Image");	
	case IHM_OBJ_Image2:	return QString("IHM_OBJ_Image2");	

//buttons
	case IHM_OBJ_btnClass:			return QString("IHM_OBJ_btnClass");
	case IHM_OBJ_btnEntryPoint:		return QString("IHM_OBJ_btnEntryPoint");
	case IHM_OBJ_btnEntryPointNbr:	return QString("IHM_OBJ_btnEntryPointNbr");
	case IHM_OBJ_btnMode:			return QString("IHM_OBJ_btnMode");
	case IHM_OBJ_btnArrowOn:		return QString("IHM_OBJ_btnArrowOn");
	case IHM_OBJ_btnArrowOff:		return QString("IHM_OBJ_btnArrowOff");
	case IHM_OBJ_btnCrossOn:		return QString("IHM_OBJ_btnCrossOn");
	case IHM_OBJ_btnCrossOff:		return QString("IHM_OBJ_btnCrossOff");
	case IHM_OBJ_btnDsrcOn:			return QString("IHM_OBJ_btnDsrcOn");
	case IHM_OBJ_btnDsrcOff:		return QString("IHM_OBJ_btnDsrcOff");
	case IHM_OBJ_btnMagneticOn:		return QString("IHM_OBJ_btnMagneticOn");
	case IHM_OBJ_btnMagneticOff:	return QString("IHM_OBJ_btnMagneticOff");
	case IHM_OBJ_btnCollectorOn:	return QString("IHM_OBJ_btnCollectorOn");
	case IHM_OBJ_btnCollectorOff:	return QString("IHM_OBJ_btnCollectorOff");

	case IHM_OBJ_btnCamera:return QString("IHM_OBJ_btnCamera");
	case IHM_OBJ_btnScanner:return QString("IHM_OBJ_btnScanner");
	case IHM_OBJ_btnComment:return QString("IHM_OBJ_btnComment");

	case IHM_OBJ_btnGabarite:return QString("IHM_OBJ_btnGabarite");
	case IHM_OBJ_btnEntryGate:return QString("IHM_OBJ_btnEntryGate");
	case IHM_OBJ_btnExitGate:return QString("IHM_OBJ_btnExitGate");
	case IHM_OBJ_btnCar3:return QString("IHM_OBJ_btnCar3");
	case IHM_OBJ_btnCar2:return QString("IHM_OBJ_btnCar2");
	case IHM_OBJ_btnCar:return QString("IHM_OBJ_btnCar");
	case IHM_OBJ_btnEntryLoop:return QString("IHM_OBJ_btnEntryLoop");
	case IHM_OBJ_btnExitLoop:return QString("IHM_OBJ_btnExitLoop");
	case IHM_OBJ_btnTblBeacon:return QString("IHM_OBJ_btnTblBeacon");

	case IHM_OBJ_btnCamGrab:return QString("IHM_OBJ_btnCamGrab");
	case IHM_OBJ_btnCamHide:return QString("IHM_OBJ_btnCamHide");
	case IHM_OBJ_btnScanGrab:return QString("IHM_OBJ_btnScanGrab");
	case IHM_OBJ_btnScanHide:return QString("IHM_OBJ_btnScanHide");	
	case IHM_OBJ_btnScanSend:return QString("IHM_OBJ_btnScanSend");
	case IHM_OBJ_btnScanSave:return QString("IHM_OBJ_btnScanSave");
	case IHM_OBJ_btnCommHide:return QString("IHM_OBJ_btnCommHide");
	case IHM_OBJ_btnExternalDisplay:return QString("IHM_OBJ_btnExternalDisplay");
	case IHM_OBJ_btnExternalDisplay2:return QString("IHM_OBJ_btnExternalDisplay2");
	case IHM_OBJ_btnTakeControl:return QString("IHM_OBJ_btnTakeControl");
	case IHM_OBJ_btnReturnControl:return QString("IHM_OBJ_btnReturnControl");
	
//spare buttons
	case IHM_OBJ_Product1 :return QString("IHM_OBJ_Product1");
	case IHM_OBJ_Product2:return QString("IHM_OBJ_Product2");
	case IHM_OBJ_Product3:return QString("IHM_OBJ_Product3");
	case IHM_OBJ_Product4:return QString("IHM_OBJ_Product4");
	case IHM_OBJ_Product5:return QString("IHM_OBJ_Product5");	

	case IHM_OBJ_btnSpeedLimitOn:return QString("IHM_OBJ_btnSpeedLimitOn");	
	case IHM_OBJ_btnSpeedLimitOff:return QString("IHM_OBJ_btnSpeedLimitOff");	
	case IHM_OBJ_btnWarningOn:return QString("IHM_OBJ_btnWarningOn");	
	case IHM_OBJ_btnWarningOff:return QString("IHM_OBJ_btnWarningOff");	
	case IHM_OBJ_btnDsrcTSAOn:return QString("IHM_OBJ_btnDsrcTSAOn");
	case IHM_OBJ_btnDsrcTSAOff:return QString("IHM_OBJ_btnDsrcTSAOff");

	case IHM_OBJ_btnSpare1:return QString("IHM_OBJ_btnSpare1");
	case IHM_OBJ_btnSpare2:return QString("IHM_OBJ_btnSpare2");
	case IHM_OBJ_btnSpare3:return QString("IHM_OBJ_btnSpare3");
	case IHM_OBJ_btnSpare4:return QString("IHM_OBJ_btnSpare4");
	case IHM_OBJ_btnSpare5:return QString("IHM_OBJ_btnSpare5");
	case IHM_OBJ_btnSpare6:return QString("IHM_OBJ_btnSpare6");
	case IHM_OBJ_btnSpare7:return QString("IHM_OBJ_btnSpare7");
	case IHM_OBJ_btnSpare8:return QString("IHM_OBJ_btnSpare8");
	case IHM_OBJ_btnSpare9:return QString("IHM_OBJ_btnSpare9");
	case IHM_OBJ_btnSpare10:return QString("IHM_OBJ_btnSpare10");

	case IHM_OBJ_btnSpare11:return QString("IHM_OBJ_btnSpare11");
	case IHM_OBJ_btnSpare12:return QString("IHM_OBJ_btnSpare12");
	case IHM_OBJ_btnSpare13:return QString("IHM_OBJ_btnSpare13");
	case IHM_OBJ_btnSpare14:return QString("IHM_OBJ_btnSpare14");
	case IHM_OBJ_btnSpare15:return QString("IHM_OBJ_btnSpare15");
	case IHM_OBJ_btnSpare16:return QString("IHM_OBJ_btnSpare16");
	case IHM_OBJ_btnSpare17:return QString("IHM_OBJ_btnSpare17");
	case IHM_OBJ_btnSpare18:return QString("IHM_OBJ_btnSpare18");
	case IHM_OBJ_btnSpare19:return QString("IHM_OBJ_btnSpare19");
	case IHM_OBJ_btnSpare20:return QString("IHM_OBJ_btnSpare20");

	case IHM_OBJ_btnSpare21:return QString("IHM_OBJ_btnSpare21");
	case IHM_OBJ_btnSpare22:return QString("IHM_OBJ_btnSpare22");
	case IHM_OBJ_btnSpare23:return QString("IHM_OBJ_btnSpare23");
	case IHM_OBJ_btnSpare24:return QString("IHM_OBJ_btnSpare24");
	case IHM_OBJ_btnSpare25:return QString("IHM_OBJ_btnSpare25");
	case IHM_OBJ_btnSpare26:return QString("IHM_OBJ_btnSpare26");
	case IHM_OBJ_btnSpare27:return QString("IHM_OBJ_btnSpare27");
	case IHM_OBJ_btnSpare28:return QString("IHM_OBJ_btnSpare28");
	case IHM_OBJ_btnSpare29:return QString("IHM_OBJ_btnSpare29");
	case IHM_OBJ_btnSpare30:return QString("IHM_OBJ_btnSpare30");


	case IHM_OBJ_TableView1:return QString("IHM_OBJ_TableView1");
	case IHM_OBJ_TableView2:return QString("IHM_OBJ_TableView2");
	case IHM_OBJ_TableView3:return QString("IHM_OBJ_TableView3");

//horizontal menu
	case IHM_OBJ_HMenuView1:return QString("IHM_OBJ_HMenuView1");
	case IHM_OBJ_HMenuView2:return QString("IHM_OBJ_HMenuView2");

	case IHM_OBJ_RVideoView1:return QString("IHM_OBJ_RVideoView1");
	case IHM_OBJ_RVideoView2:return QString("IHM_OBJ_RVideoView2");

//to be able to dynamicaly show hide containers
	case IHM_OBJ_CNT_HeaderGroup:		return QString("IHM_OBJ_CNT_HeaderGroup");
	case IHM_OBJ_CNT_InstructionGroup:	return QString("IHM_OBJ_CNT_InstructionGroup");
	case IHM_OBJ_CNT_ModeGroup:			return QString("IHM_OBJ_CNT_ModeGroup");	
	case IHM_OBJ_CNT_PaymentGroup:		return QString("IHM_OBJ_CNT_PaymentGroup");	
	case IHM_OBJ_CNT_TrsGroup:			return QString("IHM_OBJ_CNT_TrsGroup");	

//We cannot use spare containers since these are 
// not visible when not in control
	case IHM_OBJ_CNT_Comments:return QString("IHM_OBJ_CNT_Comments");
	case IHM_OBJ_CNT_Camera:return QString("IHM_OBJ_CNT_Camera");
	case IHM_OBJ_CNT_Scanner:return QString("IHM_OBJ_CNT_Scanner");

	case IHM_OBJ_CNT_Spare1:return QString("IHM_OBJ_CNT_Spare1");
	case IHM_OBJ_CNT_Spare2:return QString("IHM_OBJ_CNT_Spare2");
	case IHM_OBJ_CNT_Spare3:return QString("IHM_OBJ_CNT_Spare3");
	case IHM_OBJ_CNT_Spare4:return QString("IHM_OBJ_CNT_Spare4");
	case IHM_OBJ_CNT_Spare5:return QString("IHM_OBJ_CNT_Spare5");
	case IHM_OBJ_CNT_Spare6:return QString("IHM_OBJ_CNT_Spare6");
	case IHM_OBJ_CNT_Spare7:return QString("IHM_OBJ_CNT_Spare7");
	case IHM_OBJ_CNT_Spare8:return QString("IHM_OBJ_CNT_Spare8");
	case IHM_OBJ_CNT_Spare9:return QString("IHM_OBJ_CNT_Spare9");
	case IHM_OBJ_CNT_Spare10:return QString("IHM_OBJ_CNT_Spare10");
	case IHM_OBJ_CNT_Spare11:return QString("IHM_OBJ_CNT_Spare11");
	case IHM_OBJ_CNT_Spare12:return QString("IHM_OBJ_CNT_Spare12");
	case IHM_OBJ_CNT_Spare13:return QString("IHM_OBJ_CNT_Spare13");
	case IHM_OBJ_CNT_Spare14:return QString("IHM_OBJ_CNT_Spare14");
	case IHM_OBJ_CNT_Spare15:return QString("IHM_OBJ_CNT_Spare15");
	case IHM_OBJ_CNT_Spare16:return QString("IHM_OBJ_CNT_Spare16");
	case IHM_OBJ_CNT_Spare17:return QString("IHM_OBJ_CNT_Spare17");
	case IHM_OBJ_CNT_Spare18:return QString("IHM_OBJ_CNT_Spare18");
	case IHM_OBJ_CNT_Spare19:return QString("IHM_OBJ_CNT_Spare19");
	case IHM_OBJ_CNT_Spare20:return QString("IHM_OBJ_CNT_Spare20");

	case IHM_OBJ_CNT_MiniWeb:return QString("IHM_OBJ_CNT_MiniWeb");
	case IHM_OBJ_CNT_AlarmsGroup:return QString("IHM_OBJ_CNT_AlarmsGroup");

	case IHM_OBJ_TabControl1:return QString("IHM_OBJ_TabControl1");
	case IHM_OBJ_TabControl2:return QString("IHM_OBJ_TabControl2");
	case IHM_OBJ_TabControl3:return QString("IHM_OBJ_TabControl3");
	case IHM_OBJ_TabControl4:return QString("IHM_OBJ_TabControl4");

	case IHM_OBJ_OpenLink:return QString("IHM_OBJ_OpenLink");
	case IHM_OBJ_OpenLoginDlg:return QString("IHM_OBJ_OpenLoginDlg");
	case IHM_OBJ_AboutDlg:return QString("IHM_OBJ_AboutDlg");		
	
	
	default:
		return "IHM_OBJ_Unknown";
	    break;
	}
}


int SimuSendReciveThread::getObjectIdFromString(QString sObjectName)
{

	for (int i = IHM_OBJ_All_Object; i<=IHM_OBJ_AboutDlg; i++)
	{
		if(QString::compare(sObjectName, getStringFromObjectId(i), Qt::CaseInsensitive)==0)	
		{
			return i;
		}	
	}

	return IHM_OBJ;
}



//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
quint64 SimuSendReciveThread::m_iInstanceCounter = 0;


SimuSendReciveThread::SimuSendReciveThread(QString sIHMMailbox)
{
	QThread::QThread();

	m_sIHMMailbox = sIHMMailbox;
	m_bLoopScript = false;	

	m_iInstanceCounter = QCoreApplication::applicationPid();
	m_bScript = false;

	m_pActionSetText = new QAction(this);
	m_pActionConfig = new QAction(this);
	m_pActionAffichageSet = new QAction(this);
	m_pActionAuth = new QAction(this);
	m_pActionVideo = new QAction(this);
	m_pActionTouche = new QAction(this);
	m_pActionSaisie = new QAction(this);
	m_pActionSaisieEx = new QAction(this);
	m_pActionPolice = new QAction(this);
	m_pActionDetection = new QAction(this);
	m_pActionArret = new QAction(this);
	m_pActionScript = new QAction(this);
	m_pActionAsyncInput = new QAction(this);
	QThread::connect(this->m_pActionConfig, SIGNAL(triggered()), this, SLOT(onConfig()), Qt::DirectConnection);
	QThread::connect(this->m_pActionAffichageSet, SIGNAL(triggered()), this, SLOT(onAffichageSet()), Qt::DirectConnection);
	QThread::connect(this->m_pActionAuth, SIGNAL(triggered()), this, SLOT(onAuth()), Qt::DirectConnection);
	QThread::connect(this->m_pActionVideo, SIGNAL(triggered()), this, SLOT(onVideo()), Qt::DirectConnection);
	QThread::connect(this->m_pActionTouche, SIGNAL(triggered()), this, SLOT(onTouche()), Qt::DirectConnection);
	QThread::connect(this->m_pActionSaisie, SIGNAL(triggered()), this, SLOT(onSaisie()), Qt::DirectConnection);
	QThread::connect(this->m_pActionSaisieEx, SIGNAL(triggered()), this, SLOT(onSaisieEx()), Qt::DirectConnection);
	QThread::connect(this->m_pActionPolice, SIGNAL(triggered()), this, SLOT(onPolice()), Qt::DirectConnection);
	QThread::connect(this->m_pActionDetection, SIGNAL(triggered()), this, SLOT(onDetection()), Qt::DirectConnection);
	QThread::connect(this->m_pActionArret, SIGNAL(triggered()), this, SLOT(onArret()), Qt::DirectConnection);
	QThread::connect(this->m_pActionAsyncInput, SIGNAL(triggered()), this, SLOT(onAsyncInput()), Qt::DirectConnection);

	m_iCountMessagesEvents = 0;
	
	memset(&m_oldDetection,0,sizeof(m_oldDetection));

}

SimuSendReciveThread::~SimuSendReciveThread()
{
	delete m_pActionSetText;
	delete m_pActionConfig;
	delete m_pActionAffichageSet;
	delete m_pActionAuth;
	delete m_pActionVideo;
	delete m_pActionTouche;
	delete m_pActionSaisie;
	delete m_pActionSaisieEx;
	delete m_pActionPolice;
	delete m_pActionDetection;
	delete m_pActionArret;
	delete m_pActionScript;
	delete m_pActionAsyncInput;
}

void SimuSendReciveThread::run()
{
	char pcInstance[MAX_PATH];
    
	
    SetTextToMainDlg(QString("SIMU: Creation/Publication of response BAL..."));
	
	QString sBalName = QString(SIMU_NOM_BAL) + QString("%1").arg(m_iInstanceCounter);

	sprintf_s(pcInstance, sizeof(pcInstance), "%s", sBalName.toLatin1().data());
	
    SetTextToMainDlg(QString("SIMU: Creation BAL=%1 ...").arg(pcInstance));
	SupprimeBAL(pcInstance) ;	// stop if exist
	
	// create
    if((m_stSimuData.m_piBalSimu = PublieBAL(pcInstance, NOYAU_BAL_ILLIMITEE)) < 0)
    {
		SetTextToMainDlg(QString("SIMU ERROR: PublieBAL() return error for BAL %1").arg(pcInstance));
		return;
    }

	SetTextToMainDlg(QString("SIMU: %1 BAL created").arg(pcInstance));
	
	
    SetTextToMainDlg(QString("SIMU: Connection to IHM ..."));
	sprintf_s(pcInstance, sizeof(pcInstance), "%s", m_sIHMMailbox.toLatin1().data());
		
    SetTextToMainDlg(QString("SIMU: Connection BAL=%1 ...").arg(pcInstance));
		
	m_stSimuData.m_piBalModule = AttendBAL(pcInstance) ;
    SetTextToMainDlg(QString("OK : No. of BAL = %1 ").arg(m_stSimuData.m_piBalModule));
		
	m_stSimuData.m_bAlive = true;
    SetTextToMainDlg(QString("SIMU: instances connected."));
    
	
    SetTextToMainDlg(QString("SIMU: start simulation ..."));
	QString sCurrentScript;


	while(m_stSimuData.m_bAlive)
	{
		m_mutex.lock();
			if(!m_slScript.isEmpty())
			{
				sCurrentScript = m_slScript.takeFirst();
			}
			else if(!m_bLoopScript)
						sCurrentScript = "";

		m_mutex.unlock();

		if(sCurrentScript!="")
			OpenXMLScript(sCurrentScript);


		readRecivedMailslot();

		msleep(3);
	}
	
	
    SetTextToMainDlg(QString("SIMU: start ended ..."));
}


void SimuSendReciveThread::setLoopScript(bool bLoop)
{
	m_mutex.lock();
		m_bLoopScript = bLoop;
	m_mutex.unlock();

}


void SimuSendReciveThread::readRecivedMailslot()
{
    noyau_enum_retour           eNoyRet;
    struct_ihm_message *		psMessage = NULL;
	
    // Watch for new message
    eNoyRet = TestRecoit(m_stSimuData.m_piBalSimu, (struct_neutre **)&psMessage);
	
    if(eNoyRet == NOYAU_BAL_MESS)
    {

		QString sMsg = generateReadableMessage(psMessage);
        SetTextToMainDlg(QString("IN: %1:").arg(sMsg));
		
        switch(psMessage->entete.service)
        {
        case M_SRV_ARRET:
            switch (psMessage->entete.type_message)
            {
				// IHM is dead!
				// simulator can be swiched off.
            case SRV_TYP_ARRET_ACQ :
				m_stSimuData.m_bAlive = false;
                break;
            }
            break;
            

 		case M_IHM_AUTHORIZATION:
			switch (psMessage->entete.type_message)
			{
			case SRV_TYP_DEBUT_ACQ:
			case SRV_TYP_DEBUT_NACQ:
			case SRV_TYP_FIN_ACQ:
			case SRV_TYP_FIN_NACQ:
				break;
			case SRV_TYP_DEMANDE :
				{
					BOOL bSendAuto = FALSE;

					struct_ihm_authorization *pNewAuthData = new struct_ihm_authorization;
					memcpy(pNewAuthData, &psMessage->u.srv_auth, sizeof(struct_ihm_authorization));
					emit authData(pNewAuthData);

					switch(psMessage->u.srv_auth.cmd_type)
					{
					case IHM_AUTH_REQ:
						if(m_bAuthAuto)
						{
							psMessage->u.srv_auth.cmd_type = IHM_AUTH_RSP;
							psMessage->u.srv_auth.ret_val = enuAUTH_OK;
							bSendAuto = TRUE;
						}
						break;
					case IHM_TAKE_CTRL_REQ:
						if(m_bTakeControlAuto)
						{
							psMessage->u.srv_auth.cmd_type = IHM_TAKE_CTRL_RSP;
							psMessage->u.srv_auth.ret_val = enuAUTH_OK;
							bSendAuto = TRUE;
						}
						break;
					case IHM_RETURN_CTRL_REQ:
						if(m_bReturnControlAuto)
						{
							psMessage->u.srv_auth.cmd_type = IHM_RETURN_CTRL_RSP;
							psMessage->u.srv_auth.ret_val = enuAUTH_OK;
							bSendAuto = TRUE;
						}
						break;
					default:
						break;
					}

					if((bool)bSendAuto)
					{
						struct_ihm_authorization newAuth; 
						memset(&newAuth, 0, sizeof(newAuth));//initialize to zeroes
						
						//strcpy(psMessage->u.srv_auth.user_name, "Mate Matic");
						
						memcpy(&newAuth, &psMessage->u.srv_auth, sizeof(struct_ihm_authorization));

						sendMessage((enum_ihm_service)M_IHM_AUTHORIZATION, SRV_TYP_EFFECTUE, &newAuth);
					}

				}
				break;
			}
			break;
       
		}
		
        ExitLibere((struct_neutre **)&psMessage);
    }
}

bool SimuSendReciveThread::sendMessage(enum_ihm_service service, int type_message, void *data)
{
	bool bRet = true;
	
    noyau_enum_retour   eNoyRet;
    struct_ihm_message  * p_msg_emis = NULL;
    
    eNoyRet = Alloue((struct_neutre **)(&p_msg_emis), sizeof(struct_ihm_message), NOYAU_GetPoolId("IHM Simu"));
	
    if(eNoyRet == NOYAU_OK)
    {
        p_msg_emis->entete.service = (enum_srv_service)service;
        p_msg_emis->entete.type_message = (enum_srv_type)type_message;

		switch(service)
		{
		case M_IHM_CONFIG:
			switch(type_message)
			{
			// type SRV_TYP_GET dont need anything.
			case SRV_TYP_SET:
				memcpy(&p_msg_emis->u.srv_config, (struct_ihm_srv_config*)data, sizeof(struct_ihm_srv_config));
				break;
			}
			break;
		case M_SRV_ARRET:
			// type SRV_TYP_DEMANDE dont need anything aditional
			break;
		case M_IHM_AFFICHAGE:
			switch(type_message)
			{
				// types SRV_TYP_DEBUT and SRV_TYP_FIN dont need anything aditional
				//case SRV_TYP_DEBUT:
				//	break;
				//case SRV_TYP_FIN:
				//	break;
			case SRV_TYP_SET:
				memcpy(&p_msg_emis->u.srv_affichage, (struct_ihm_srv_affichage*)data, sizeof(struct_ihm_srv_affichage));
				break;
			}
			break;
		case M_IHM_AFFICHAGE_STATUS:
			switch(type_message)
			{
			case SRV_TYP_SET:
				memcpy(&p_msg_emis->u.srv_affichage, (struct_ihm_srv_affichage*)data, sizeof(struct_ihm_srv_affichage));
				break;
			}
			break;
		case M_IHM_POLICE:
			switch(type_message)
			{
			case SRV_TYP_SET:
				memcpy(&p_msg_emis->u.srv_police, (struct_ihm_srv_police*)data, sizeof(struct_ihm_srv_police));
				break;
			}
			break;
		case M_IHM_TOUCHES:
			switch(type_message)
			{
			// types SRV_TYP_DEBUT and SRV_TYP_FIN dont need anything aditional
			//case SRV_TYP_DEBUT:
			//	break;
			//case SRV_TYP_FIN:
			//	break;
			case SRV_TYP_DEMANDE:
				memcpy(&p_msg_emis->u.srv_touches, (struct_ihm_srv_touches*)data, sizeof(struct_ihm_srv_touches));
				break;
			}
			break;
		case M_IHM_SAISIE_VISU:
			switch(type_message)
			{
			case SRV_TYP_DEMANDE:
				memcpy(&p_msg_emis->u.srv_saisie_visu, (struct_ihm_srv_saisie_visu*)data, sizeof(struct_ihm_srv_saisie_visu));
				break;
			case SRV_TYP_DEMANDE_ANNULATION:
				break;
			}
			break;
		case M_IHM_SAISIE_VISU_EX:
			switch(type_message)
			{
			case SRV_TYP_DEMANDE:
			case SRV_TYP_DEMANDE_ANNULATION:
				memcpy(&p_msg_emis->u.srv_saisie_visu_ex, (struct_ihm_srv_saisie_visu_ex*)data, sizeof(struct_ihm_srv_saisie_visu_ex));
				break;
			}
			break;

		case M_IHM_DETECTION_CHAINE:
			switch(type_message)
			{
			case SRV_TYP_DEBUT:
				memcpy(&p_msg_emis->u.srv_detection, (struct_ihm_srv_detection*)data, sizeof(struct_ihm_srv_detection));
				break;
			case SRV_TYP_FIN:
				memcpy(&p_msg_emis->u.srv_detection, (struct_ihm_srv_detection*)data, sizeof(struct_ihm_srv_detection));
				break;
			//default:
			//	break;
			}
			break;

		case M_IHM_AUTHORIZATION:
				memcpy(&p_msg_emis->u.srv_auth, (struct_ihm_authorization*)data, sizeof(struct_ihm_authorization));
			break;
		
		case M_IHM_ASYNC_INPUT:
				memcpy(&p_msg_emis->u.srv_async_input, (struct_ihm_async_input*)data, sizeof(struct_ihm_async_input));
			break;
		


		default:
			break;
		}
        

//         SetTextToMainDlg(QString("SIMU sendMessage() Service: %1 Type: %2 done OK").arg(SimuGetMsgSrv((enum_ihm_service)service)).arg(SimuGetMsgType((enum_ihm_type)type_message)));
// 		QString sMsg = generateReadableMessage(p_msg_emis);
//         SetTextToMainDlg(QString("OUT:%1").arg(sMsg));
        
		eNoyRet = Envoie(m_stSimuData.m_piBalModule, m_stSimuData.m_piBalSimu, (struct_neutre *)(p_msg_emis));
        
        if(eNoyRet != NOYAU_OK)
		{
            SetTextToMainDlg(QString("SIMU ERREUR: sendMessage() retourne %1").arg(eNoyRet));
			ExitLibere((struct_neutre **)&p_msg_emis);
			bRet = false;
		}
        
    }
    else
    {
        SetTextToMainDlg(QString("SIMU ERREUR: Alloue() retourne %1").arg(eNoyRet));
		bRet = false;
    }
	
	return bRet;
}

QString SimuSendReciveThread::generateReadableMessage(struct_ihm_message  * psMessage)
{
	QString sMessage;

       switch(psMessage->entete.service)
        {
        case M_SRV_ARRET:
            switch (psMessage->entete.type_message)
            {
				// IHM is dead!
				// simulator can be swiched off.
				case SRV_TYP_ARRET_ACQ :
					sMessage += QString("ARRET done!");
                break;
 				default:
					sMessage = QString(" M_SRV_ARRET:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
           }
            break;
            
		case M_IHM_CONFIG:
				switch (psMessage->entete.type_message)
				{
				case IHM_CONFIG_ACTUELLE:
					sMessage += QString(" M_IHM_CONFIG: IHM_CONFIG_ACTUELLE");
					sMessage += QString("    Lane type: %1").arg((char*)psMessage->u.srv_config.type_voie);
					sMessage += QString("   main_dlg_hidden: %1").arg((char*)psMessage->u.srv_config.main_dlg_hidden);
					sMessage += QString("    Language: %1").arg((char*)psMessage->u.srv_config.langue);
					sMessage += QString("    Type of shift:: %1").arg((char*)psMessage->u.srv_config.type_poste);
					sMessage += QString("    Project: %1").arg((char*)psMessage->u.srv_config.projet);
					sMessage += QString("    Verstion: %1").arg((char*)psMessage->u.srv_config.no_version);
					break;
				case SRV_TYP_GET_ACQ:
					sMessage += QString(" M_IHM_CONFIG: SRV_TYP_GET_ACQ");
					break;
				case SRV_TYP_GET_NACQ:  
					sMessage += QString(" M_IHM_CONFIG: SRV_TYP_GET_NACQ");
					break;
				case SRV_TYP_SET_ACQ:
					sMessage += QString(" M_IHM_CONFIG: SRV_TYP_SET_ACQ");
					break;
				case SRV_TYP_SET_NACQ:  
					sMessage += QString(" M_IHM_CONFIG: SRV_TYP_SET_NACQ");
					break;
				case SRV_TYP_SET:
					sMessage += QString(" M_IHM_CONFIG: SRV_TYP_SET");
				break;
				default:
					sMessage = QString(" M_IHM_CONFIG:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));

				}
				break;
		case M_IHM_AFFICHAGE_STATUS:
			switch(psMessage->entete.type_message)
			{
			case SRV_TYP_DEBUT:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_DEBUT");
				break;
			case SRV_TYP_DEBUT_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUSM_IHM_AFFICHAGE: SRV_TYP_DEBUT_ACQ");
				break;
			case SRV_TYP_DEBUT_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_DEBUT_NACQ");
				break;
			case SRV_TYP_FIN_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_FIN_ACQ");
				break;
			case SRV_TYP_FIN_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_FIN_NACQ");
				break;
			case SRV_TYP_SET_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_SET_ACQ");
				break;
			case SRV_TYP_SET_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_SET_NACQ");
				break;
			case SRV_TYP_SET:
				sMessage += QString(" M_IHM_AFFICHAGE_STATUS: SRV_TYP_SET");
				break;
				
			default:
				sMessage = QString(" M_IHM_AFFICHAGE_STATUS:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
            
		case M_IHM_AFFICHAGE:
			switch(psMessage->entete.type_message)
			{
			case SRV_TYP_DEBUT:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_DEBUT");
				break;
			case SRV_TYP_DEBUT_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_DEBUT_ACQ");
				break;
			case SRV_TYP_DEBUT_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_DEBUT_NACQ");
				break;
			case SRV_TYP_FIN_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_FIN_ACQ");
				break;
			case SRV_TYP_FIN_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_FIN_NACQ");
				break;
			case SRV_TYP_SET_ACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_SET_ACQ");
				break;
			case SRV_TYP_SET_NACQ:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_SET_NACQ");
				break;
			case SRV_TYP_SET:
				sMessage += QString(" M_IHM_AFFICHAGE: SRV_TYP_SET");
				break;
			case IHM_VIDEO_CMD_RESULT:
				sMessage += QString(" M_IHM_AFFICHAGE: IHM_VIDEO_CMD_RESULT:%1 result %2")
								.arg(getStringFromObjectId(psMessage->u.srv_affichage.objet))
								.arg(getVideoCmdString(psMessage->u.srv_affichage.valeur.video.eCmd));
	
				
				break;

			default:
					sMessage = QString(" M_IHM_AFFICHAGE:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
			
		case M_IHM_POLICE:
			switch(psMessage->entete.type_message)
			{
				case SRV_TYP_SET_ACQ:
					sMessage += QString(" M_IHM_POLICE: SRV_TYP_SET_ACQ");
				break;
				case SRV_TYP_SET_NACQ:
					sMessage += QString(" M_IHM_POLICE: SRV_TYP_SET_NACQ");
				break;
				case SRV_TYP_SET:
					sMessage += QString(" M_IHM_POLICE: SRV_TYP_SET");
				break;
				default:
					sMessage = QString(" M_IHM_POLICE:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
			
		case M_IHM_DETECTION_CHAINE:
			switch (psMessage->entete.type_message)
			{
				
				case SRV_TYP_DEBUT:
					sMessage += QString(" M_IHM_DETECTION_CHAINE: SRV_TYP_DEBUT");
					break;
				case SRV_TYP_DEBUT_ACQ:
					sMessage += QString(" M_IHM_DETECTION_CHAINE: SRV_TYP_DEBUT_ACQ");
					break;
				case SRV_TYP_DEBUT_NACQ:
					sMessage += QString(" M_IHM_DETECTION_CHAINE: SRV_TYP_DEBUT_NACQ");
					break;
				case SRV_TYP_FIN_ACQ:
					sMessage += QString(" M_IHM_DETECTION_CHAINE: SRV_TYP_FIN_ACQ");
					break;
				case SRV_TYP_FIN_NACQ:
					sMessage += QString(" M_IHM_DETECTION_CHAINE: SRV_TYP_FIN_NACQ");
					break;

				case IHM_NOUVELLE_CHAINE :
					sMessage += QString(" Chaine complete = %1").arg(psMessage->u.srv_detection.ChaineComplete);
					sMessage += QString(" EnvoiePrePostambule = %1").arg(psMessage->u.srv_detection.EnvoiePrePostambule);
					sMessage += QString(" Couple ID = %1").arg((char*)psMessage->u.srv_detection.CoupleId);
					sMessage += QString(" Chaine Saisie = %1").arg((char*)psMessage->u.srv_detection.ChaineSaisie);
					break;
				default:
					sMessage = QString(" M_IHM_DETECTION_CHAINE:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
			
		case M_IHM_TOUCHES:
			switch (psMessage->entete.type_message)
			{
				case SRV_TYP_DEMANDE:
					sMessage += QString(" M_IHM_TOUCHES: SRV_TYP_DEMANDE");
					break;
				case SRV_TYP_DEMANDE_ACQ:
					sMessage += QString(" M_IHM_TOUCHES: SRV_TYP_DEMANDE_ACQ");
					break;
				case SRV_TYP_DEMANDE_NACQ:
					sMessage += QString(" M_IHM_TOUCHES: SRV_TYP_DEMANDE_NACQ");
					break;
				case IHM_NOUVELLE_TOUCHE:
					sMessage += QString(" M_IHM_TOUCHES: IHM_NOUVELLE_TOUCHE");
					sMessage += QString("   IHM_NOUVELLE_TOUCHE: type:%1, touche:%2, command:%3, command param:%4")
										.arg(psMessage->u.srv_touches.type)
										.arg(psMessage->u.srv_touches.touche)
										.arg(psMessage->u.srv_touches.command)
										.arg(psMessage->u.srv_touches.command_param);
					
					break;
				default:
					sMessage = QString(" M_IHM_TOUCHES:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
			
		case M_IHM_SAISIE_VISU:
			switch (psMessage->entete.type_message)
			{
			case IHM_NOUVELLE_SAISIE :
			case IHM_SAISIE_ANNULEE :
				if(psMessage->entete.type_message == IHM_NOUVELLE_SAISIE) 
						sMessage += QString("IHM_NOUVELLE_SAISIE");
				else 
					sMessage += QString("IHM_SAISIE_ANNULEE");
				
				sMessage += QString(" B_SAISIE = 0x%1").arg(psMessage->u.srv_saisie_visu.b_saisie, 8, 16);
				sMessage += QString(" BOUTONS  = 0x%1").arg((DWORD)psMessage->u.srv_saisie_visu.boutons, 2, 16);
				sMessage += QString(" STRING   = [%1]").arg((char*)psMessage->u.srv_saisie_visu.string);
				 
				break;
			default:
				sMessage = QString(" M_IHM_SAISIE_VISU:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
		case M_IHM_SAISIE_VISU_EX:
			switch (psMessage->entete.type_message)
			{ 
			case SRV_TYP_DEBUT:
				sMessage += QString(" M_IHM_SAISIE_VISU_EX: SRV_TYP_DEBUT");
				break;
			case SRV_TYP_FIN:
				sMessage += QString(" M_IHM_SAISIE_VISU_EX: SRV_TYP_FIN");
				break;
			case IHM_NOUVELLE_SAISIE :
				sMessage += QString("IHM_NOUVELLE_SAISIE: DIALOG_ID=[%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.dialog_id);
				sMessage += QString(" data=[%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.data);
				break;
			case IHM_SAISIE_ANNULEE :
				sMessage += QString("IHM_SAISIE_ANNULEE: DIALOG_ID  = [%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.dialog_id);
				sMessage += QString(" data=[%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.data);
				break;
			case IHM_RESULT_DEMANDE_ANNULATION :
				sMessage += QString("IHM_RESULT_DEMANDE_ANNULATION: DIALOG_ID  = [%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.dialog_id);
				break;
			case SRV_TYP_DEMANDE_ANNULATION :
				sMessage += QString("SRV_TYP_DEMANDE_ANNULATION: DIALOG_ID  = [%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.dialog_id);
				break;
			case SRV_TYP_DEMANDE_ACQ :
				sMessage += QString("SRV_TYP_DEMANDE_ACQ: ");
				break;
			case SRV_TYP_DEMANDE_NACQ :
				sMessage += QString("SRV_TYP_DEMANDE_NACQ: ");
				break;
			case SRV_TYP_DEMANDE:
				sMessage += QString("SRV_TYP_DEMANDE: DIALOG_ID  = [%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.dialog_id);
				sMessage += QString(" data=[%1]").arg((char*)psMessage->u.srv_saisie_visu_ex.data);
				sMessage += QString(" is_file=[%1]").arg(psMessage->u.srv_saisie_visu_ex.is_file);
				break;
			case SRV_TYP_EFFECTUE:
				sMessage += QString("SRV_TYP_EFFECTUE: ");
				break;

			default:
				sMessage = QString(" M_IHM_SAISIE_VISU_EX: TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;
		case M_IHM_ASYNC_INPUT:
			switch (psMessage->entete.type_message)
			{
			case SRV_TYP_DEBUT_ACQ:
				sMessage += QString(" M_IHM_ASYNC_INPUT: SRV_TYP_DEBUT_ACQ");
				break;
			case SRV_TYP_DEBUT_NACQ:
				sMessage += QString(" M_IHM_ASYNC_INPUT: SRV_TYP_DEBUT_NACQ");
				break;
			case SRV_TYP_FIN_ACQ:
				sMessage += QString(" M_IHM_ASYNC_INPUT: SRV_TYP_FIN_ACQ");
				break;
			case SRV_TYP_FIN_NACQ:
				sMessage += QString(" M_IHM_ASYNC_INPUT: SRV_TYP_FIN_NACQ");
				break;
			case IHM_NOUVELLE_SAISIE :
				sMessage += QString("IHM_NOUVELLE_SAISIE  data = [%1]").arg((char*)psMessage->u.srv_async_input.data);
				break;
			case SRV_TYP_GET_NACQ :
				sMessage += QString(" M_IHM_ASYNC_INPUT: SRV_TYP_GET_NACQ");
				break;
			case SRV_TYP_GET :
				sMessage += QString("SRV_TYP_GET  data = [%1]").arg((char*)psMessage->u.srv_async_input.data);
				break;
			default:
				sMessage = QString(" M_IHM_ASYNC_INPUT: TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
			}
			break;

		

 		case M_IHM_AUTHORIZATION:
			{
				QStringList sCommands;
				sCommands <<"IHM_AUTH_UNKNOWN"
							<<"IHM_AUTH_REQ"
							<<"IHM_AUTH_RSP"
							<<"IHM_TAKE_CTRL_REQ"
							<<"IHM_TAKE_CTRL_RSP"
							<<"IHM_RETURN_CTRL_REQ"
							<<"IHM_RETURN_CTRL_RSP"
							<<"IHM_AUTH_LOGOFF_REQ"
							<<"IHM_AUTH_DLG_CANCELED";
				int iCmd = (psMessage->u.srv_auth.cmd_type>8||psMessage->u.srv_auth.cmd_type<0)?0:psMessage->u.srv_auth.cmd_type;
		
				QString sDetails =	QString(" command:%1; user:%2; sec_code:%3; is_sec_code_pwd:%4; in_session:%5; is desktop:%6; reason:%7 result:%8 ")
											.arg(sCommands.at(iCmd))
											.arg(psMessage->u.srv_auth.user_id)
											.arg(psMessage->u.srv_auth.sec_code)
											.arg((int)psMessage->u.srv_auth.is_sec_code_pwd)
											.arg(psMessage->u.srv_auth.session_id)
											.arg(psMessage->u.srv_auth.is_desktop?"TRUE":"FALSE")
											.arg(getNameForAuthReasonId((int)psMessage->u.srv_auth.reason))
											.arg(psMessage->u.srv_auth.ret_val);


				
				switch (psMessage->entete.type_message)
				{
				case SRV_TYP_DEBUT:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_DEBUT");
					break;
				case SRV_TYP_DEBUT_ACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_DEBUT_ACQ");
					break;
				case SRV_TYP_DEBUT_NACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_DEBUT_NACQ");
					break;
				case SRV_TYP_FIN_ACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_FIN_ACQ");
					break;
				case SRV_TYP_FIN_NACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_FIN_NACQ");
					break;
				case SRV_TYP_EFFECTUE:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_EFFECTUE") + sDetails;
					break;
				case SRV_TYP_SET_ACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_SET_ACQ");
					break;
				case SRV_TYP_SET_NACQ:
					sMessage += QString(" M_IHM_AUTHORIZATION: SRV_TYP_SET_NACQ");
					break;

				case SRV_TYP_DEMANDE :
					{
						sMessage += QString("M_IHM_AUTHORIZATION: SRV_TYP_DEMANDE:") + sDetails;
									
					}
					break;
				default:
					sMessage = QString(" M_IHM_AUTHORIZATION:  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
				}
			}
			break;

			default:
				sMessage = QString("SRV UNKNOWN:%1  TYP: %1!!!").arg( SimuGetMsgType((enum_ihm_type)psMessage->entete.type_message));
		}
		
	return sMessage;
}


QAction * SimuSendReciveThread::getSetTextAction()
{
	return m_pActionSetText;
}



void SimuSendReciveThread::SetTextToMainDlg(const QString &szText)
{
//	incMsgsEvents();

	emit message(QString("%1").arg(szText));
//	QCoreApplication::processEvents();
}

QString SimuSendReciveThread::SimuGetMsgSrv(enum_ihm_service eSrv)
{
	QString szReturn;

    switch( eSrv )
    {
    case SRV_TYP_SERVICE_INCONNU    : szReturn = "SRV_TYP_SERVICE_INCONNU";			break;
    case M_SRV_ARRET                : szReturn = "M_SRV_ARRET";						break;
    case M_IHM_CONFIG               : szReturn = "M_IHM_CONFIG";					break;
    case M_IHM_AFFICHAGE            : szReturn = "M_IHM_AFFICHAGE";					break;
    case M_IHM_TOUCHES              : szReturn = "M_IHM_TOUCHES";					break;
    case M_IHM_SAISIE_VISU          : szReturn = "M_IHM_SAISIE_VISU";				break;
    case M_IHM_SAISIE_VISU_EX       : szReturn = "M_IHM_SAISIE_VISU_EX";			break;
    case M_IHM_INDICATEURS          : szReturn = "M_IHM_INDICATEURS";				break;
	case M_IHM_POLICE				: szReturn = "M_IHM_POLICE";					break;
	case M_IHM_DETECTION_CHAINE		: szReturn = "M_IHM_DETECTION_CHAINE";			break;
	case M_IHM_AUTHORIZATION		: szReturn = "M_IHM_AUTHORIZATION";				break;
	case M_IHM_ASYNC_INPUT			: szReturn = "M_IHM_ASYNC_INPUT";				break;
    default                         : szReturn = QString("Unknown: %1").arg(eSrv);	break;
    }

	return szReturn;
}


QString SimuSendReciveThread::SimuGetMsgType(enum_ihm_type eType)
{
	QString szReturn;
	
    switch( eType )
    {
    case SRV_TYP_DEBUT              : szReturn = "SRV_TYP_DEBUT";					break;
    case SRV_TYP_FIN                : szReturn = "SRV_TYP_FIN";						break;
    case SRV_TYP_DEMANDE            : szReturn = "SRV_TYP_DEMANDE";					break;
    case SRV_TYP_DEMANDE_ANNULATION : szReturn = "SRV_TYP_DEMANDE_ANNULATION";		break;
    case SRV_TYP_SET                : szReturn = "SRV_TYP_SET";						break;
    case SRV_TYP_SET_ACQ            : szReturn = "SRV_TYP_SET_ACQ";					break;
    case SRV_TYP_SET_NACQ           : szReturn = "SRV_TYP_SET_NACQ";				break;
    case SRV_TYP_GET                : szReturn = "SRV_TYP_GET";						break;
    case SRV_TYP_GET_ACQ            : szReturn = "SRV_TYP_GET_ACQ";					break;
    case SRV_TYP_GET_NACQ           : szReturn = "SRV_TYP_GET_NACQ";				break;
    case SRV_TYP_DEBUT_ACQ          : szReturn = "SRV_TYP_DEBUT_ACQ";				break;
    case SRV_TYP_DEBUT_NACQ         : szReturn = "SRV_TYP_DEBUT_NACQ";				break;
    case SRV_TYP_FIN_ACQ            : szReturn = "SRV_TYP_FIN_ACQ";					break;
    case SRV_TYP_FIN_NACQ           : szReturn = "SRV_TYP_FIN_NACQ";				break;
    case SRV_TYP_DEMANDE_ACQ        : szReturn = "SRV_TYP_DEMANDE_ACQ";				break;
    case SRV_TYP_DEMANDE_NACQ       : szReturn = "SRV_TYP_DEMANDE_NACQ";			break;
    case SRV_TYP_ARRET_ACQ          : szReturn = "SRV_TYP_ARRET_ACQ";				break;
    case SRV_TYP_ARRET_NACQ         : szReturn = "SRV_TYP_ARRET_NACQ";				break;
    case SRV_TYP_NOUVEL_ETAT        : szReturn = "SRV_TYP_NOUVEL_ETAT";				break;
    case IHM_CONFIG_ACTUELLE        : szReturn = "IHM_CONFIG_ACTUELLE";				break;
    case IHM_NOUVELLE_TOUCHE        : szReturn = "IHM_NOUVELLE_TOUCHE";				break;
    case IHM_NOUVELLE_SAISIE        : szReturn = "IHM_NOUVELLE_SAISIE";				break;
    case IHM_SAISIE_ANNULEE         : szReturn = "IHM_SAISIE_ANNULEE";				break;
    case IHM_RESULT_DEMANDE_ANNULATION: szReturn = "IHM_RESULT_DEMANDE_ANNULATION";				break;
	case IHM_NOUVELLE_CHAINE		: szReturn = "IHM_NOUVELLE_CHAINE";				break;
    case SRV_TYP_DEMANDE_INCONNUE   : szReturn = "SRV_TYP_DEMANDE_INCONNUE";		break;
    case SRV_TYP_DEMANDE_NON_GEREE  : szReturn = "SRV_TYP_DEMANDE_NON_GEREE";		break;
    case SRV_TYP_EFFECTUE		    : szReturn = "SRV_TYP_EFFECTUE";		break;
    default                         : szReturn = QString("Unknown: %1").arg(eType);	break;
    }
	
	return szReturn;
}

QAction * SimuSendReciveThread::getConfigAction()
{
	return m_pActionConfig;
}

void SimuSendReciveThread::onConfig()
{
	if(m_pActionConfig->property(_PROP_TYPE) == _PROP_VAL_GET)
	{
		sendMessage(M_IHM_CONFIG, SRV_TYP_GET, NULL);
	}
	else if(m_pActionConfig->property(_PROP_TYPE) == _PROP_VAL_SET)
	{
		struct_ihm_srv_config newConf = {0};

		strcpy((char*)newConf.projet, m_pActionConfig->property(_PROP_PROJECT).toByteArray().data());
		strcpy((char*)newConf.no_version, m_pActionConfig->property(_PROP_VERSION).toByteArray().data());
		strcpy((char*)newConf.langue, m_pActionConfig->property(_PROP_LANGUAGE).toByteArray().data());
		strcpy((char*)newConf.type_voie, m_pActionConfig->property(_PROP_TYPE_OF_LANE).toByteArray().data());
		strcpy((char*)newConf.type_poste, m_pActionConfig->property(_PROP_TYPE_OF_SHIFT).toByteArray().data());
		strcpy((char*)newConf.main_dlg_hidden, "");

		sendMessage(M_IHM_CONFIG, SRV_TYP_SET, (void*)&newConf);

		m_szLanguage = m_pActionConfig->property(_PROP_LANGUAGE).toByteArray();
	}
}

QString SimuSendReciveThread::getLanguage()
{
	return m_szLanguage;
}

QAction * SimuSendReciveThread::getAffichageSetAction()
{
	return m_pActionAffichageSet;
}

void SimuSendReciveThread::onAffichageSet()
{
	if(m_pActionAffichageSet->property(_PROP_TYPE).toString() == _PROP_VAL_SET)
	{
		struct_ihm_srv_affichage newAff; 
		memset(&newAff, 0, sizeof(newAff));//initialize to zeroes

		newAff.objet = (enum_ihm_objets)m_pActionAffichageSet->property(_PROP_OBJECT).toInt();
		if(newAff.objet == IHM_OBJ_OpenLink)
		{
			newAff.valeur.link.iTargetDialog = m_pActionAffichageSet->property(_PROP_DIALOG_ID).toInt();
			strcpy(newAff.valeur.link.szUrlValue, m_pActionAffichageSet->property(_PROP_VALUE).toByteArray());
		}
		else if (IHM_OBJ_OpenLoginDlg)
		{
			strcpy((char*)newAff.valeur.szValeur, m_pActionAffichageSet->property(_PROP_VALUE).toByteArray());
		}
		else
		{
			strcpy((char*)newAff.valeur.szValeur, m_pActionAffichageSet->property(_PROP_VALUE).toByteArray());
			newAff.propriete = m_pActionAffichageSet->property(_PROP_PROPERTY).toByteArray().toInt();
		}

		sendMessage(M_IHM_AFFICHAGE, SRV_TYP_SET, (void*)&newAff);
	}
	else if(m_pActionAffichageSet->property(_PROP_TYPE) == _PROP_VAL_DEBUT)
		sendMessage(M_IHM_AFFICHAGE, SRV_TYP_DEBUT, NULL);
	else if(m_pActionAffichageSet->property(_PROP_TYPE) == _PROP_VAL_FIN)
		sendMessage(M_IHM_AFFICHAGE, SRV_TYP_FIN, NULL);
}

QAction * SimuSendReciveThread::getVideoAction()
{
	return m_pActionVideo;
}

void SimuSendReciveThread::onVideo()
{
	struct_ihm_srv_affichage newAff;
	memset(&newAff, 0, sizeof(newAff));//initialize to zeroes

	int iSelected = m_pActionVideo->property(_PROP_VIDEO).toInt();
	newAff.objet = (enum_ihm_objets)((iSelected%4) + IHM_OBJ_RVideoView1);
		

	newAff.valeur.video.eCmd = (enum_ihm_video_cmd)m_pActionVideo->property(_PROP_COMMAND).toInt();
	
	switch(newAff.valeur.video.eCmd)
	{
	case IHM_VIDEO_ON:
	case IHM_VIDEO_OFF:
	case IHM_VIDEO_FREEZE:
	case IHM_VIDEO_UNFREEZE:
	case IHM_VIDEO_ZOOM_IN:
	case IHM_VIDEO_ZOOM_OUT:
	case IHM_VIDEO_ZOOM:
		newAff.valeur.video.cmdval.dwValeur = m_pActionVideo->property(_PROP_VALUE).toInt();
		break;
	case IHM_VIDEO_SELECT_SRC:
		newAff.valeur.video.cmdval.dwValeur = m_pActionVideo->property(_PROP_SOURCE).toInt();
		break;
	case IHM_VIDEO_SAVE_BMP:
		strcpy(newAff.valeur.video.cmdval.szValeur, m_pActionVideo->property(_PROP_VALUE).toString().toLatin1().data());
		break;
	case IHM_VIDEO_SAVE_JPG:
		strcpy(newAff.valeur.video.cmdval.szValeur, m_pActionVideo->property(_PROP_VALUE).toString().toLatin1().data());
		break;
	}		
	
	sendMessage(M_IHM_AFFICHAGE, SRV_TYP_SET, (void*)&newAff);
}

QAction * SimuSendReciveThread::getToucheAction()
{
	return m_pActionTouche;
}

void SimuSendReciveThread::onTouche()
{
	int iService, iType;
	struct_ihm_srv_touches newTouches = {0}; //initialize to zeroes

	if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_DEBUT)
	{
		iService = M_IHM_TOUCHES;
		iType = SRV_TYP_DEBUT;
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_DEMANDE)
	{
		iService = M_IHM_TOUCHES;
		iType = SRV_TYP_DEMANDE;

		strcpy((char*)newTouches.nomgroupe, m_pActionTouche->property(_PROP_GROUPE).toByteArray());
		newTouches.touche = m_pActionTouche->property(_PROP_ASCII).toInt();
		newTouches.etat = m_pActionTouche->property(_PROP_ETAT).toByteArray().at(0);
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_FIN)
	{
		iService = M_IHM_TOUCHES;
		iType = SRV_TYP_FIN;
	}

	sendMessage((enum_ihm_service)iService, iType, &newTouches);
}

QAction * SimuSendReciveThread::getSaisieAction()
{
	return m_pActionSaisie;
}

void SimuSendReciveThread::onSaisie()
{
	if(m_pActionSaisie->property(_PROP_TYPE).toString() == _PROP_VAL_DEMANDE)
	{
		struct_ihm_srv_saisie_visu newSaisie = {0}; //initialize to zeroes

		newSaisie.b_saisie = m_pActionSaisie->property(_PROP_SAISIE).toInt();
		
		newSaisie.boutons = m_pActionSaisie->property(_PROP_BUTTONS).toInt();
		
		strcpy((char*)newSaisie.string, m_pActionSaisie->property(_PROP_DEFINITION).toByteArray());
		
		sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU, SRV_TYP_DEMANDE, &newSaisie);
	}
	else if(m_pActionSaisie->property(_PROP_TYPE).toString() == _PROP_VAL_ANULATE)
	{
		sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU, SRV_TYP_DEMANDE_ANNULATION, NULL);
	}
}

QAction * SimuSendReciveThread::getSaisieExAction()
{
	return m_pActionSaisieEx;
}

void SimuSendReciveThread::onSaisieEx()
{
	QString szTemp;
	struct_ihm_srv_saisie_visu_ex newSaisieEx = {0}; //initialize to zeroes

	if(m_pActionSaisieEx->property(_PROP_TYPE).toString() == _PROP_VAL_DEMANDE)
	{
		
		strcpy((char*)newSaisieEx.dialog_id, m_pActionSaisieEx->property(_PROP_DIALOG_ID).toByteArray());
		newSaisieEx.is_file = m_pActionSaisieEx->property(_PROP_IS_FILE).toInt();
		strcpy((char*)newSaisieEx.data, m_pActionSaisieEx->property(_PROP_VALUE).toByteArray());
		
		sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_DEMANDE, &newSaisieEx);
	}			
	else if(m_pActionSaisieEx->property(_PROP_TYPE).toString() == _PROP_VAL_ANULATE)
	{
		sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_DEMANDE_ANNULATION, &newSaisieEx);
	}
}

QAction * SimuSendReciveThread::getPoliceAction()
{
	return m_pActionPolice;
}

void SimuSendReciveThread::onPolice()
{
	if(m_pActionPolice->property(_PROP_TYPE).toString() == _PROP_VAL_SET)
	{
		struct_ihm_srv_police newPolice;
		memset(&newPolice, 0, sizeof(newPolice));//initialize to zeroes

		newPolice.objet = (enum_ihm_objets)m_pActionPolice->property(_PROP_OBJECT).toInt();
		strcpy((char*)newPolice.valeur.szValeur, m_pActionPolice->property(_PROP_VALUE).toByteArray());
		newPolice.propriete = m_pActionPolice->property(_PROP_PROPERTY).toChar().toLatin1();

		sendMessage((enum_ihm_service)M_IHM_POLICE, SRV_TYP_SET, &newPolice);
	}
}

QAction * SimuSendReciveThread::getDetectionAction()
{
	return m_pActionDetection;
}

void SimuSendReciveThread::onDetection()
{
	if(m_pActionDetection->property(_PROP_TYPE).toString() == _PROP_VAL_DEBUT)
	{
		struct_ihm_srv_detection newDetection = {0}; //initialize to zeroes
		
		strcpy((char*)newDetection.CoupleId, m_pActionDetection->property(_PROP_COUPLE_ID).toByteArray());
		//strcpy((char*)newDetection.Format, m_pActionDetection->property(_PROP_SAISIE).toByteArray());
		newDetection.EnvoiePrePostambule = m_pActionDetection->property(_PROP_PREPOSTAMBULES).toInt();
		newDetection.ChaineComplete= (unsigned char) 0;
		newDetection.ChaineSaisie[0]=0;

		strcpy((char*)m_oldDetection.CoupleId, (char*)newDetection.CoupleId);
		
		sendMessage((enum_ihm_service)M_IHM_DETECTION_CHAINE, SRV_TYP_DEBUT, &newDetection);
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_FIN)
	{
		sendMessage((enum_ihm_service)M_IHM_DETECTION_CHAINE, SRV_TYP_FIN, &m_oldDetection);
	}
}

QAction * SimuSendReciveThread::getArretAction()
{
	return m_pActionArret;
}

void SimuSendReciveThread::onArret()
{
	sendMessage((enum_ihm_service)M_SRV_ARRET, SRV_TYP_DEMANDE, NULL);
}

QAction * SimuSendReciveThread::getScriptAction()
{
	return m_pActionScript;
}

void SimuSendReciveThread::onAppendScript(QString szScript)
{
	m_mutex.lock();
		m_slScript.append(szScript);
	m_mutex.unlock();

}

void SimuSendReciveThread::onPrependScript(QString szScript)
{
	m_mutex.lock();
		m_slScript.prepend(szScript);
	m_mutex.unlock();

}

const char * SimuSendReciveThread::verify_if_no_change_string(char *s)
{
	if(strcmp(SCRIPT_XML_ATTRIBUTE_NO_CHANGE,s) == 0)
		return IHM_NOCHANGE_S;
	else
		return s;
}


void SimuSendReciveThread::OpenXMLScript(QString szScript)
{
	int errorLine;
	int errorColumn;
	QString errorStr;
	QFile fConfFile(szScript);
	QDomDocument xmlConfig;
	
	SetTextToMainDlg(QString("Executing script: %1 ...").arg(szScript));

	if(!fConfFile.exists())
	{
		SetTextToMainDlg(QString("ERROR: Script file [%1] not found").arg(szScript));
	}

	fConfFile.open(QIODevice::ReadOnly | QIODevice::Text);
	
	if(!xmlConfig.setContent((QIODevice*)&fConfFile, true, &errorStr, &errorLine, &errorColumn))
	{
		SetTextToMainDlg(QString("---XML Error---"));
		SetTextToMainDlg(QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr));
	}
	else
	{
		QDomElement root = xmlConfig.documentElement();
		if(root.tagName() != SIMU_ACT_SCRIPT_ROOT_TAG)
		{
			SetTextToMainDlg(QString("---XML Error---"));
			SetTextToMainDlg(QString("XML file not action script!"));
		}
		else
		{
			QDomElement child = xmlConfig.documentElement().firstChildElement(XML_ACTION);
			
			while(!child.isNull())
			{
				usleep(1);
				if(child.attribute(_PROP_SERVICE) == XML_CONFIG)
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_SET)
					{
						struct_ihm_srv_config newConf = {0};

						strcpy((char*)newConf.projet, verify_if_no_change_string(child.attribute(_PROP_PROJECT).toLatin1().data()));
						strcpy((char*)newConf.no_version, verify_if_no_change_string(child.attribute(_PROP_VERSION).toLatin1().data()));
						strcpy((char*)newConf.langue, verify_if_no_change_string(child.attribute(_PROP_LANGUAGE).toLatin1().data()));
						strcpy((char*)newConf.type_voie,  verify_if_no_change_string(child.attribute(_PROP_TYPE_OF_LANE).toLatin1().data()));
						strcpy((char*)newConf.type_poste, verify_if_no_change_string(child.attribute(_PROP_TYPE_OF_SHIFT).toLatin1().data()));
						strcpy((char*)newConf.main_dlg_hidden, verify_if_no_change_string(child.attribute(_PROP_MAIN_DLG_HIDDEN).toLatin1().data()));
						
											
						sendMessage(M_IHM_CONFIG, SRV_TYP_SET, (void*)&newConf);
						
						m_szLanguage = child.attribute(_PROP_LANGUAGE).toLatin1().data();
					}
				}
				else if(child.attribute(_PROP_SERVICE) == XML_AFFICHAGE)
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						sendMessage(M_IHM_AFFICHAGE, SRV_TYP_DEBUT, NULL);
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_SET)
					{
						struct_ihm_srv_affichage newAff;
						memset(&newAff, 0, sizeof(newAff));//initialize to zeroes

						newAff.objet = (enum_ihm_objets)getObjectIdFromString(child.attribute(_PROP_OBJECT));

						if(newAff.objet==IHM_OBJ)
						{
							SetTextToMainDlg(QString("SIMU ERREUR: Unknown object: %1").arg(child.attribute(_PROP_OBJECT)));
						}
						else if(newAff.objet == IHM_OBJ_RVideoView1 ||
							newAff.objet == IHM_OBJ_RVideoView2 )
						{
							newAff.valeur.video.eCmd = (enum_ihm_video_cmd)getCommandIdFromString(child.attribute(_PROP_COMMAND));
							
							switch(newAff.valeur.video.eCmd)
							{
							case IHM_VIDEO_ON:
							case IHM_VIDEO_OFF:
							case IHM_VIDEO_FREEZE:
							case IHM_VIDEO_UNFREEZE:
							case IHM_VIDEO_ZOOM_IN:
							case IHM_VIDEO_ZOOM_OUT:
							case IHM_VIDEO_ZOOM:
							case IHM_VIDEO_SELECT_SRC:
								newAff.valeur.video.cmdval.dwValeur = child.attribute(_PROP_VALUE).toInt();
								break;
							case IHM_VIDEO_SAVE_BMP:
							case IHM_VIDEO_SAVE_JPG:
								strcpy(newAff.valeur.video.cmdval.szValeur, child.attribute(_PROP_VALUE).toLatin1().data());
								break;
							}		
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;
						}
						else if(newAff.objet == IHM_OBJ_OpenLink)
						{
							newAff.valeur.link.iTargetDialog = child.attribute(_PROP_DIALOG_ID).toInt();
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;
							strcpy(newAff.valeur.link.szUrlValue, child.attribute(_PROP_VALUE).toLatin1().data());
						}
						else if(newAff.objet == IHM_OBJ_OpenLoginDlg)
						{
							strcpy((char*)newAff.valeur.login_dlg.szLogin, child.attribute(_PROP_VALUE).toLatin1().data());
							newAff.valeur.login_dlg.bManualLogin = child.attribute(_PROP_MANUAL_LOGIN).toInt();
							newAff.valeur.login_dlg.bLDAPOffline = child.attribute(_PROP_LDAP_OFFLINE).toInt();
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;

						}
						else if(newAff.objet == IHM_OBJ_HMenuView1 || newAff.objet == IHM_OBJ_HMenuView2)
						{
							strcpy((char*)newAff.valeur.menu.szMenuContent, child.attribute(_PROP_VALUE).toLatin1().data());
							newAff.valeur.menu.bUseExternalFile = child.attribute(_PROP_IS_FILE).toInt();
							newAff.valeur.menu.bDoNotReloadMenu = child.attribute(_PROP_DO_NOT_RELOAD).toInt();
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;

						}
						else if(newAff.objet == IHM_OBJ_TableView1 || newAff.objet == IHM_OBJ_TableView2 || newAff.objet == IHM_OBJ_TableView3)
						{
							strcpy((char*)newAff.valeur.table_view.szContent, child.attribute(_PROP_VALUE).toLatin1().data());
							newAff.valeur.table_view.bUseExternalFile = child.attribute(_PROP_IS_FILE).toInt();
							
							enum_aff_table_command iCmd = (enum_aff_table_command)getTableViewCmdFromString(child.attribute(_PROP_COMMAND));
							
							//By default prepend rows (if no command or is invalid)
							if(iCmd<0)
									iCmd = enuIHM_AFF_TABLE_CMD_PREPEND_ROWS;
							newAff.valeur.table_view.eTableCmd =  iCmd;  
							
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;
						}
						else
						{
							if(child.attribute(_PROP_VALUE)!="")
							{
								QString sValue = child.attribute(_PROP_VALUE);
								strcpy((char*)newAff.valeur.szValeur, verify_if_no_change_string(sValue.toLatin1().data()));
							}
							else
							{
								if(child.attribute("BUTTON_ENABLED") == "1")
									newAff.is_enabled = enuIHM_OBJECT_ENABLED;
								else if(child.attribute("BUTTON_ENABLED") == "0")
									newAff.is_enabled = enuIHM_OBJECT_DISABLED;
								else	
									newAff.is_enabled = enuIHM_OBJECT_UNCHANGED;

								strcpy((char*)newAff.valeur.button.szValeurImg, verify_if_no_change_string(child.attribute("BUTTON_IMG").toLatin1().data()));
								strcpy((char*)newAff.valeur.button.szValeurText, verify_if_no_change_string(child.attribute("BUTTON_TEXT").toLatin1().data()));

							}
							
							QString sToolTip = child.attribute(_PROP_TOOLTIP);
							strcpy((char*)newAff.szToolTip, verify_if_no_change_string(sToolTip.toLatin1().data()));
							
							newAff.propriete = child.attribute(_PROP_PROPERTY).toInt();
							newAff.visibility = (child.attribute(_PROP_VISIBLITY) == "0")?enuIHM_AFF_OBJ_HIDDEN:enuIHM_AFF_OBJ_VISIBLE;
						}

						if(newAff.objet!=IHM_OBJ)
							sendMessage(M_IHM_AFFICHAGE, SRV_TYP_SET, (void*)&newAff);
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						sendMessage(M_IHM_AFFICHAGE, SRV_TYP_FIN, NULL);
					}
				}
				else if(child.attribute(_PROP_SERVICE) == "AFFICHAGE_STATUS")
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						sendMessage(M_IHM_AFFICHAGE_STATUS, SRV_TYP_DEBUT, NULL);
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_SET)
					{
						struct_ihm_srv_affichage newAff;
						memset(&newAff, 0, sizeof(newAff));//initialize to zeroes

						if(child.attribute(_PROP_GROUPE)!="")
						{
							QString sValue = child.attribute(_PROP_GROUPE);
							strcpy((char*)newAff.valeur.szValeur, sValue.toLatin1().data());

							if(child.attribute("ENABLED") == "1")
								newAff.is_enabled = enuIHM_OBJECT_ENABLED;
							else
								newAff.is_enabled = enuIHM_OBJECT_DISABLED;
						}
							
						sendMessage(M_IHM_AFFICHAGE_STATUS, SRV_TYP_SET, (void*)&newAff);
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						sendMessage(M_IHM_AFFICHAGE_STATUS, SRV_TYP_FIN, NULL);
					}
				}
				else if(child.attribute(_PROP_SERVICE) == "POLICE")
				{
					struct_ihm_srv_police newPolice;
					memset(&newPolice, 0, sizeof(newPolice));//initialize to zeroes
					
					newPolice.objet = (enum_ihm_objets)getObjectIdFromString(child.attribute(_PROP_OBJECT));
					strcpy((char*)newPolice.valeur.szValeur, child.attribute(_PROP_VALUE).toLatin1().data());

					sendMessage((enum_ihm_service)M_IHM_POLICE, SRV_TYP_SET, &newPolice);
				}
				else if(child.attribute(_PROP_SERVICE) == XML_TOUCHE)
				{
					int iService, iType;
					struct_ihm_srv_touches newTouches  = {0};
					
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						iService = M_IHM_TOUCHES;
						iType = SRV_TYP_DEBUT;
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEMANDE)
					{
						iService = M_IHM_TOUCHES;
						iType = SRV_TYP_DEMANDE;
						
						strcpy((char*)newTouches.nomgroupe, child.attribute(_PROP_GROUPE).toLatin1().data());
						newTouches.touche = child.attribute(_PROP_ASCII).toInt();
						newTouches.etat = child.attribute(_PROP_ETAT).toLatin1().at(0);
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						iService = M_IHM_TOUCHES;
						iType = SRV_TYP_FIN;
					}		
					sendMessage((enum_ihm_service)iService, iType, &newTouches);
				}
				else if(child.attribute(_PROP_SERVICE) == "SAISIE")
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEMANDE)
					{
						QString szDefinition;
						struct_ihm_srv_saisie_visu newSaisie  = {0};
						
						newSaisie.b_saisie = child.attribute(_PROP_SAISIE).toInt();
						
						newSaisie.boutons = child.attribute(_PROP_BUTTONS).toInt();
						
						szDefinition.append(QString("|%1|%2").arg(child.attribute(_PROP_DLG_NAME)).arg(child.attribute(_PROP_NO_CONTROLS)));
						szDefinition.append(child.attribute(_PROP_CTRL_STRING));
						strcpy((char*)newSaisie.string, szDefinition.toLatin1().data());
						
						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU, SRV_TYP_DEMANDE, &newSaisie);
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_ANULATE)
					{
						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU, SRV_TYP_DEMANDE_ANNULATION, NULL);
					}
				}
				else if(child.attribute(_PROP_SERVICE) == "SAISIE_EX")
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_DEBUT, NULL);
					}			
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_FIN, NULL);
					}			
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEMANDE)
					{
						QString szTemp;
						struct_ihm_srv_saisie_visu_ex newSaisieEx  = {0};
						
						strcpy((char*)newSaisieEx.dialog_id, child.attribute(_PROP_DIALOG_ID).toLatin1().data());
						newSaisieEx.is_file = child.attribute(_PROP_IS_FILE).toInt();
						//if(newSaisieEx.is_file == 0)
						//{
						//	//QString ss;
						//	//QTextStream s(&ss);
						//	//s << child.firstChild();
						//	//strcpy((char*)newSaisieEx.data, ss.toLatin1().data());
						//	strcpy((char*)newSaisieEx.data, child.attribute(_PROP_VALUE).toLatin1().data());
						//}
						//else
							strcpy((char*)newSaisieEx.data, child.attribute(_PROP_VALUE).toLatin1().data());
		
						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_DEMANDE, &newSaisieEx);
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_ANULATE)
					{
						QString szTemp;
						struct_ihm_srv_saisie_visu_ex newSaisieEx  = {0};
						
						strcpy((char*)newSaisieEx.dialog_id, child.attribute(_PROP_DIALOG_ID).toLatin1().data());

						sendMessage((enum_ihm_service)M_IHM_SAISIE_VISU_EX, SRV_TYP_DEMANDE_ANNULATION,  &newSaisieEx);
					}
				}
				else if(child.attribute(_PROP_SERVICE) == "DETECTION")
				{
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						struct_ihm_srv_detection newDetection = {0};
						
						strcpy((char*)newDetection.CoupleId, child.attribute(_PROP_COUPLE_ID).toLatin1());
						//strcpy((char*)newDetection.Format, child.attribute(_PROP_SAISIE).toLatin1());
						newDetection.EnvoiePrePostambule = child.attribute(_PROP_PREPOSTAMBULES).toInt();
						newDetection.ChaineComplete= (unsigned char) 0;
						newDetection.ChaineSaisie[0]=0;
						
						strcpy((char*)m_oldDetection.CoupleId, (char*)newDetection.CoupleId);
						
						sendMessage((enum_ihm_service)M_IHM_DETECTION_CHAINE, SRV_TYP_DEBUT, &newDetection);
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						struct_ihm_srv_detection newDetection = {0};
						
						strcpy((char*)newDetection.CoupleId, child.attribute(_PROP_COUPLE_ID).toLatin1());
						//strcpy((char*)newDetection.Format, child.attribute(_PROP_SAISIE).toLatin1());
						newDetection.EnvoiePrePostambule = child.attribute(_PROP_PREPOSTAMBULES).toInt();
						newDetection.ChaineComplete= (unsigned char) 0;
						newDetection.ChaineSaisie[0]=0;
						
						strcpy((char*)m_oldDetection.CoupleId, (char*)newDetection.CoupleId);

						sendMessage((enum_ihm_service)M_IHM_DETECTION_CHAINE, SRV_TYP_FIN, &newDetection);
					}
				}
				else if(child.attribute(_PROP_SERVICE) == "WAIT")
				{
					// TYPE = "KEY"
					if (QString::compare(child.attribute("TYPE"), "KEY", Qt::CaseInsensitive) == 0)
					{
						setWaitUserDialog();
						emit openWaitDialog();
						//wait for the wait dlg to close
						bool bCancelScript;
						while (!ifWait(bCancelScript))
						{
							Sleep(10);
							readRecivedMailslot();
						}

						if (bCancelScript)
						{
							return;
							SetTextToMainDlg(QString("Script %1 canceled!").arg(szScript));

						}
					}
					else
					{ 
						QTime dieTime = QTime::currentTime().addMSecs(child.attribute("MILISEC").toInt());
						while (QTime::currentTime() < dieTime)
							;
					}
				}
				else if(child.attribute(_PROP_SERVICE) == XML_AUTHORIZATION)
				{
					int iService, iType;
					struct_ihm_authorization newAuth;
					memset(&newAuth, 0, sizeof(newAuth));//initialize to zeroes
					
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						iService = M_IHM_AUTHORIZATION;
						iType = SRV_TYP_DEBUT;
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_EFFECTUE)
					{
						iService = M_IHM_AUTHORIZATION;
						iType = SRV_TYP_EFFECTUE;
						
						memset(&newAuth,0,sizeof(struct_ihm_authorization));
						newAuth.cmd_type = (enum_ihm_auth_cmd)child.attribute("CMD_TYPE").toInt();
						newAuth.is_desktop = (BYTE)child.attribute("IS_DESKTOP").toInt();
						newAuth.is_sec_code_pwd = (BYTE)child.attribute("IS_SEC_CODE_PWD").toInt();
						newAuth.ret_val = (enum_ihm_auth_ret_val)child.attribute("RET_VAL").toInt();

						strcpy((char*)newAuth.user_id, child.attribute("USER").toLatin1().data());
						strcpy((char*)newAuth.sec_code, child.attribute("SEC_CODE").toLatin1().data());
						strcpy((char*)newAuth.session_id, child.attribute("SESSION_ID").toLatin1().data());

					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						iService = M_IHM_AUTHORIZATION;
						iType = SRV_TYP_FIN;
					}		
					sendMessage((enum_ihm_service)iService, iType, &newAuth);
				}
				else if(child.attribute(_PROP_SERVICE) == "ASYNC_INPUT")
				{
					struct_ihm_async_input newAsync = {0};
					int iService, iType;
					
					if(child.attribute(_PROP_TYPE) == _PROP_VAL_DEBUT)
					{
						iService = M_IHM_ASYNC_INPUT;
						iType = SRV_TYP_DEBUT;
					}			
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_SET)
					{
						iService = M_IHM_ASYNC_INPUT;
						iType = SRV_TYP_SET;
						newAsync.is_file = child.attribute(_PROP_IS_FILE).toInt();
						strcpy((char*)newAsync.data, child.attribute(_PROP_VALUE).toLatin1().data());
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_GET)
					{
						iService = M_IHM_ASYNC_INPUT;
						iType = SRV_TYP_GET;
					}
					else if(child.attribute(_PROP_TYPE) == _PROP_VAL_FIN)
					{
						iService = M_IHM_ASYNC_INPUT;
						iType = SRV_TYP_FIN;
					}		
					
					sendMessage((enum_ihm_service)iService, iType, &newAsync);
				}
				
			
				readRecivedMailslot();
				
				child = child.nextSiblingElement(XML_ACTION);
			}
		}
	}

	SetTextToMainDlg(QString("...Done!").arg(szScript));

}


/*
enum_ihm_auth_cmd             cmd_type;

char    user[IHM_LG_CHAINES_SRV];	
char	pwd[IHM_LG_CHAINES_SRV];
char	session_id[IHM_LG_SAISIE_MAX];
BYTE	is_desktop;
BYTE	ret_val;
*/

QAction * SimuSendReciveThread::getAuthAction()
{
	return m_pActionAuth;
}

void SimuSendReciveThread::onAuth()
{
	struct_ihm_authorization newAuth;
	memset(&newAuth, 0, sizeof(newAuth));//initialize to zeroes
	
	newAuth.cmd_type = (enum_ihm_auth_cmd)m_pActionAuth->property(_PROP_TYPE).toInt();
	strcpy((char*)newAuth.user_id, m_pActionAuth->property(_PROP_USER).toByteArray());
	strcpy((char*)newAuth.sec_code, m_pActionAuth->property(_PROP_PASSWORD).toByteArray());
	strcpy((char*)newAuth.session_id, m_pActionAuth->property(_PROP_SESSION).toByteArray());
	newAuth.is_desktop = m_pActionAuth->property(_PROP_DESKTOP).toBool();
	newAuth.is_sec_code_pwd = m_pActionAuth->property(_PROP_IS_CODE_PWD).toBool();
	newAuth.ret_val = (enum_ihm_auth_ret_val)m_pActionAuth->property(_PROP_VALUE).toInt();
	
	sendMessage((enum_ihm_service)M_IHM_AUTHORIZATION, SRV_TYP_EFFECTUE, &newAuth);
}

QAction * SimuSendReciveThread::getAsyncInputAction()
{
	return m_pActionAsyncInput;
}

void SimuSendReciveThread::onAsyncInput()
{
	struct_ihm_async_input newAsync = {0};
	int iService, iType;
	
	if(m_pActionAsyncInput->property(_PROP_TYPE).toString() == _PROP_VAL_DEBUT)
	{
		iService = M_IHM_ASYNC_INPUT;
		iType = SRV_TYP_DEBUT;
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_SET)
	{
		iService = M_IHM_ASYNC_INPUT;
		iType = SRV_TYP_SET;
		newAsync.is_file = m_pActionTouche->property(_PROP_IS_FILE).toInt();
		strcpy((char*)newAsync.data, m_pActionTouche->property(_PROP_VALUE).toByteArray());
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_GET)
	{
		iService = M_IHM_ASYNC_INPUT;
		iType = SRV_TYP_GET;
	}
	else if(m_pActionTouche->property(_PROP_TYPE).toString() == _PROP_VAL_FIN)
	{
		iService = M_IHM_ASYNC_INPUT;
		iType = SRV_TYP_FIN;
	}

	sendMessage((enum_ihm_service)iService, iType, &newAsync);
}

void SimuSendReciveThread::incMsgsEvents()
{
	m_mutexMsgs.lock();
		m_iCountMessagesEvents++;
	
	m_mutexMsgs.unlock();
}

int SimuSendReciveThread::decMsgsEvents()
{
	int i;
	
	m_mutexMsgs.lock();
		i = --m_iCountMessagesEvents;
	m_mutexMsgs.unlock();

	return i;
}


void SimuSendReciveThread::setUserDialogConfirmed(bool bCancelScript)
{
	m_mutex.lock();
		m_bWaitShouldContinue = true;
		m_bCancelScript = bCancelScript;
	m_mutex.unlock();

}

void SimuSendReciveThread::setWaitUserDialog()
{
	m_mutex.lock();
		m_bWaitShouldContinue = false;
		m_bCancelScript = false;
	m_mutex.unlock();

}

bool SimuSendReciveThread::ifWait(bool& bShouldCancel)
{
	bool bWait = false;

	m_mutex.lock();
		bWait = m_bWaitShouldContinue;
		bShouldCancel = m_bCancelScript;
	m_mutex.unlock();

	return bWait;
}