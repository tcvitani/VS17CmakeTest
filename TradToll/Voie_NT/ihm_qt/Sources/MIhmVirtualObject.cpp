


#include <QDateTime>

#include "MIhmVirtualObject.h"
#include "MStrDetectionPair.h"
#include "MInputDialogReq.h"
#include "MInputDialogExReq.h"
#include "MInputAsyncExReq.h"
#include "MTracer.h"
#include "MRowsCommand.h"
#include "MTableModelRow.h"
#include "MHelpFuncs.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <ihm.H>
	#include "ihm_loc.h"
};

QString MIhmVirtualObject::getNameForId(enumVirtualObjectId eID)
{
	switch(eID)
	{
			case enuIhmNoVirtualObjId: return QString("enuIhmNoVirtualObjId");
			
			//text labels
			case enuIhmVirtAllObject: return QString("enuIhmVirtAllObject");
			case enuIhmVirtPlazaName: return QString("enuIhmVirtPlazaName");	
			case enuIhmVirtPlazaName1: return QString("enuIhmVirtPlazaName1");	
			case enuIhmVirtPlazaName2: return QString("enuIhmVirtPlazaName2");	
			case enuIhmVirtPlazaName3: return QString("enuIhmVirtPlazaName3");	
			case enuIhmVirtPlazaNameTitle: return QString("enuIhmVirtPlazaNameTitle");
			case enuIhmVirtPlazaNbr: return QString("enuIhmVirtPlazaNbr");
			case enuIhmVirtPlazaNbrTitle: return QString("enuIhmVirtPlazaNbrTitle");
			case enuIhmVirtLaneNbr: return QString("enuIhmVirtLaneNbr");
			case enuIhmVirtLaneNbrTitle: return QString("enuIhmVirtLaneNbrTitle");
			case enuIhmVirtLaneName: return QString("enuIhmVirtLaneName");          
			case enuIhmVirtLaneName1: return QString("enuIhmVirtLaneName1");          
			case enuIhmVirtLaneName2: return QString("enuIhmVirtLaneName2");          
			case enuIhmVirtLaneNameTitle: return QString("enuIhmVirtLaneNameTitle");	
			case enuIhmVirtCollectorID: return QString("enuIhmVirtCollectorID");
			case enuIhmVirtCollectorName: return QString("enuIhmVirtCollectorName");

			case enuIhmVirtTxtSpare1: return QString("enuIhmVirtTxtSpare1");
			case enuIhmVirtTxtSpare2: return QString("enuIhmVirtTxtSpare2");
			case enuIhmVirtTxtSpare3: return QString("enuIhmVirtTxtSpare3");
			case enuIhmVirtTxtSpare4: return QString("enuIhmVirtTxtSpare4");
			case enuIhmVirtTxtSpare5: return QString("enuIhmVirtTxtSpare5");
			case enuIhmVirtTxtSpare6: return QString("enuIhmVirtTxtSpare6");
			case enuIhmVirtTxtSpare7: return QString("enuIhmVirtTxtSpare7");
			case enuIhmVirtTxtSpare8: return QString("enuIhmVirtTxtSpare8");
			case enuIhmVirtTxtSpare9: return QString("enuIhmVirtTxtSpare9");
			case enuIhmVirtTxtSpare10: return QString("enuIhmVirtTxtSpare10");

			case enuIhmVirtTxtSpare11: return QString("enuIhmVirtTxtSpare11");
			case enuIhmVirtTxtSpare12: return QString("enuIhmVirtTxtSpare12");
			case enuIhmVirtTxtSpare13: return QString("enuIhmVirtTxtSpare13");
			case enuIhmVirtTxtSpare14: return QString("enuIhmVirtTxtSpare14");
			case enuIhmVirtTxtSpare15: return QString("enuIhmVirtTxtSpare15");
			case enuIhmVirtTxtSpare16: return QString("enuIhmVirtTxtSpare16");
			case enuIhmVirtTxtSpare17: return QString("enuIhmVirtTxtSpare17");
			case enuIhmVirtTxtSpare18: return QString("enuIhmVirtTxtSpare18");
			case enuIhmVirtTxtSpare19: return QString("enuIhmVirtTxtSpare19");
			case enuIhmVirtTxtSpare20: return QString("enuIhmVirtTxtSpare20");

			case enuIhmVirtTxtSpare21: return QString("enuIhmVirtTxtSpare21");
			case enuIhmVirtTxtSpare22: return QString("enuIhmVirtTxtSpare22");
			case enuIhmVirtTxtSpare23: return QString("enuIhmVirtTxtSpare23");
			case enuIhmVirtTxtSpare24: return QString("enuIhmVirtTxtSpare24");
			case enuIhmVirtTxtSpare25: return QString("enuIhmVirtTxtSpare25");
			case enuIhmVirtTxtSpare26: return QString("enuIhmVirtTxtSpare26");
			case enuIhmVirtTxtSpare27: return QString("enuIhmVirtTxtSpare27");
			case enuIhmVirtTxtSpare28: return QString("enuIhmVirtTxtSpare28");
			case enuIhmVirtTxtSpare29: return QString("enuIhmVirtTxtSpare29");
			case enuIhmVirtTxtSpare30: return QString("enuIhmVirtTxtSpare30");

			case enuIhmVirtTxtSpare31: return QString("enuIhmVirtTxtSpare31");
			case enuIhmVirtTxtSpare32: return QString("enuIhmVirtTxtSpare32");
			case enuIhmVirtTxtSpare33: return QString("enuIhmVirtTxtSpare33");
			case enuIhmVirtTxtSpare34: return QString("enuIhmVirtTxtSpare34");
			case enuIhmVirtTxtSpare35: return QString("enuIhmVirtTxtSpare35");
			case enuIhmVirtTxtSpare36: return QString("enuIhmVirtTxtSpare36");
			case enuIhmVirtTxtSpare37: return QString("enuIhmVirtTxtSpare37");
			case enuIhmVirtTxtSpare38: return QString("enuIhmVirtTxtSpare38");
			case enuIhmVirtTxtSpare39: return QString("enuIhmVirtTxtSpare39");
			case enuIhmVirtTxtSpare40: return QString("enuIhmVirtTxtSpare40");

			case enuIhmVirtTxtSpare41: return QString("enuIhmVirtTxtSpare41");
			case enuIhmVirtTxtSpare42: return QString("enuIhmVirtTxtSpare42");
			case enuIhmVirtTxtSpare43: return QString("enuIhmVirtTxtSpare43");
			case enuIhmVirtTxtSpare44: return QString("enuIhmVirtTxtSpare44");
			case enuIhmVirtTxtSpare45: return QString("enuIhmVirtTxtSpare45");
			case enuIhmVirtTxtSpare46: return QString("enuIhmVirtTxtSpare46");
			case enuIhmVirtTxtSpare47: return QString("enuIhmVirtTxtSpare47");
			case enuIhmVirtTxtSpare48: return QString("enuIhmVirtTxtSpare48");
			case enuIhmVirtTxtSpare49: return QString("enuIhmVirtTxtSpare49");
			case enuIhmVirtTxtSpare50: return QString("enuIhmVirtTxtSpare50");

			case enuIhmVirtTxtSpare51: return QString("enuIhmVirtTxtSpare51");
			case enuIhmVirtTxtSpare52: return QString("enuIhmVirtTxtSpare52");
			case enuIhmVirtTxtSpare53: return QString("enuIhmVirtTxtSpare53");
			case enuIhmVirtTxtSpare54: return QString("enuIhmVirtTxtSpare54");
			case enuIhmVirtTxtSpare55: return QString("enuIhmVirtTxtSpare55");
			case enuIhmVirtTxtSpare56: return QString("enuIhmVirtTxtSpare56");
			case enuIhmVirtTxtSpare57: return QString("enuIhmVirtTxtSpare57");
			case enuIhmVirtTxtSpare58: return QString("enuIhmVirtTxtSpare58");
			case enuIhmVirtTxtSpare59: return QString("enuIhmVirtTxtSpare59");
			case enuIhmVirtTxtSpare60: return QString("enuIhmVirtTxtSpare60");


			case enuIhmVirtTransactionNbr: return QString("enuIhmVirtTransactionNbr");
			case enuIhmVirtTransactionNbrTitle: return QString("enuIhmVirtTransactionNbrTitle");
			case enuIhmVirtReceiptNbr: return QString("enuIhmVirtReceiptNbr");
			case enuIhmVirtReceiptNbrTitle: return QString("enuIhmVirtReceiptNbrTitle");
			case enuIhmVirtAxles: return QString("enuIhmVirtAxles");
			case enuIhmVirtAxlesTitle: return QString("enuIhmVirtAxlesTitle");
			case enuIhmVirtBalance: return QString("enuIhmVirtBalance");
			case enuIhmVirtBalanceTitle: return QString("enuIhmVirtBalanceTitle");
			case enuIhmVirtClass: return QString("enuIhmVirtClass");
			case enuIhmVirtClassTitle: return QString("enuIhmVirtClassTitle");  
			case enuIhmVirtAddRevenue: return QString("enuIhmVirtAddRevenue");
			case enuIhmVirtAddRevenueTitle: return QString("enuIhmVirtAddRevenueTitle");

			case enuIhmVirtNbCar: return QString("enuIhmVirtNbCar");
			case enuIhmVirtEntry: return QString("enuIhmVirtEntry");
			case enuIhmVirtEntryTitle: return QString("enuIhmVirtEntryTitle");
			case enuIhmVirtCurrency: return QString("enuIhmVirtCurrency");
			case enuIhmVirtCurrencyTitle: return QString("enuIhmVirtCurrencyTitle");

			case enuIhmVirtFare: return QString("enuIhmVirtFare");
			case enuIhmVirtFareTitle: return QString("enuIhmVirtFareTitle");
			case enuIhmVirtSale: return QString("enuIhmVirtSale");
			case enuIhmVirtSaleTitle: return QString("enuIhmVirtSaleTitle");
			case enuIhmVirtSaleDue: return QString("enuIhmVirtSaleDue");
			case enuIhmVirtSaleDueTitle: return QString("enuIhmVirtSaleDueTitle");
			case enuIhmVirtSaleDuePaid: return QString("enuIhmVirtSaleDuePaid");
			case enuIhmVirtTransactionType: return QString("enuIhmVirtTransactionType");
			case enuIhmVirtTransactionTypeTitle: return QString("enuIhmVirtTransactionTypeTitle");
			case enuIhmVirtFareType: return QString("enuIhmVirtFareType");
			case enuIhmVirtFareTypeTitle: return QString("enuIhmVirtFareTypeTitle");

			case enuIhmVirtTxtModeTitle: return QString("enuIhmVirtTxtModeTitle");
			case enuIhmVirtTxtMode: return QString("enuIhmVirtTxtMode");
			case enuIhmVirtTxtCollectorTitle: return QString("enuIhmVirtTxtCollectorTitle");
			case enuIhmVirtTxtInCtrlUserTitle: return QString("enuIhmVirtTxtInCtrlUserTitle");
			case enuIhmVirtTxtInCtrlUser: return QString("enuIhmVirtTxtInCtrlUser");		
			case enuIhmVirtTxtInCtrlUser1: return QString("enuIhmVirtTxtInCtrlUser1");		
			case enuIhmVirtTxtTrsContainerTitle: return QString("enuIhmVirtTxtTrsContainerTitle");	
			case enuIhmVirtTxtAlarmsTitle: return QString("enuIhmVirtTxtAlarmsTitle");
			case enuIhmVirtTxtTrsHistoryTitle: return QString("enuIhmVirtTxtTrsHistoryTitle");
			case enuIhmVirtTxtAlertesTitle: return QString("enuIhmVirtTxtAlertesTitle");	
			case enuIhmVirtTxtCamNumPic: return QString("enuIhmVirtTxtCamNumPic");	
			case enuIhmVirtTxtScanNumPic: return QString("enuIhmVirtTxtScanNumPic");	
			case enuIhmVirtTxtNbCarSAS: return QString("enuIhmVirtTxtNbCarSAS");	

			//instructions
			case enuIhmVirtErrPayment: return QString("enuIhmVirtErrPayment");
			case enuIhmVirtPayment: return QString("enuIhmVirtPayment");
			case enuIhmVirtRemark: return QString("enuIhmVirtRemark");
			case enuIhmVirtInstruction: return QString("enuIhmVirtInstruction");   
			case enuIhmVirtInstruction2: return QString("enuIhmVirtInstruction2");
			case enuIhmVirtCurrentDate: return QString("enuIhmVirtCurrentDate");

			// Icons
			case enuIhmVirtIcoLaneStatus: return QString("enuIhmVirtIcoLaneStatus");
			case enuIhmVirtIcoLaneMode: return QString("enuIhmVirtIcoLaneMode");
			case enuIhmVirtIcoCollector: return QString("enuIhmVirtIcoCollector");       
			case enuIhmVirtIcoEntryGate: return QString("enuIhmVirtIcoEntryGate");			
			case enuIhmVirtIcoExitGate: return QString("enuIhmVirtIcoExitGate");		
			case enuIhmVirtIcoCar: return QString("enuIhmVirtIcoCar");
			case enuIhmVirtIcoTrafficLight: return QString("enuIhmVirtIcoTrafficLight");
			case enuIhmVirtIcoViolation: return QString("enuIhmVirtIcoViolation");       
			case enuIhmVirtIcoEntryLoop: return QString("enuIhmVirtIcoEntryLoop");		
			case enuIhmVirtIcoExitLoop: return QString("enuIhmVirtIcoExitLoop");
			case enuIhmVirtIcoBeacon: return QString("enuIhmVirtIcoBeacon");
			case enuIhmVirtIcoEntOpticalBarrier: return QString("enuIhmVirtIcoEntOpticalBarrier");
			
			case enuIhmVirtIcoSpare1: return QString("enuIhmVirtIcoSpare1");
			case enuIhmVirtIcoSpare2: return QString("enuIhmVirtIcoSpare2");
			case enuIhmVirtIcoSpare3: return QString("enuIhmVirtIcoSpare3");
			case enuIhmVirtIcoSpare4: return QString("enuIhmVirtIcoSpare4");
			case enuIhmVirtIcoSpare5: return QString("enuIhmVirtIcoSpare5");
			case enuIhmVirtIcoSpare6: return QString("enuIhmVirtIcoSpare6");
			case enuIhmVirtIcoSpare7: return QString("enuIhmVirtIcoSpare7");
			case enuIhmVirtIcoSpare8: return QString("enuIhmVirtIcoSpare8");
			case enuIhmVirtIcoSpare9: return QString("enuIhmVirtIcoSpare9");
			case enuIhmVirtIcoSpare10: return QString("enuIhmVirtIcoSpare10");
			case enuIhmVirtIcoSpare11: return QString("enuIhmVirtIcoSpare11");
			case enuIhmVirtIcoSpare12: return QString("enuIhmVirtIcoSpare12");
			case enuIhmVirtIcoSpare13: return QString("enuIhmVirtIcoSpare13");
			case enuIhmVirtIcoSpare14: return QString("enuIhmVirtIcoSpare14");
			case enuIhmVirtIcoSpare15: return QString("enuIhmVirtIcoSpare15");
			case enuIhmVirtIcoSpare16: return QString("enuIhmVirtIcoSpare16");
			case enuIhmVirtIcoSpare17: return QString("enuIhmVirtIcoSpare17");
			case enuIhmVirtIcoSpare18: return QString("enuIhmVirtIcoSpare18");
			case enuIhmVirtIcoSpare19: return QString("enuIhmVirtIcoSpare19");
			case enuIhmVirtIcoSpare20: return QString("enuIhmVirtIcoSpare20");
			case enuIhmVirtIcoSpare21: return QString("enuIhmVirtIcoSpare21");
			case enuIhmVirtIcoSpare22: return QString("enuIhmVirtIcoSpare22");
			case enuIhmVirtIcoSpare23: return QString("enuIhmVirtIcoSpare23");
			case enuIhmVirtIcoSpare24: return QString("enuIhmVirtIcoSpare24");
			case enuIhmVirtIcoSpare25: return QString("enuIhmVirtIcoSpare25");
			case enuIhmVirtIcoSpare26: return QString("enuIhmVirtIcoSpare26");
			case enuIhmVirtIcoSpare27: return QString("enuIhmVirtIcoSpare27");
			case enuIhmVirtIcoSpare28: return QString("enuIhmVirtIcoSpare28");
			case enuIhmVirtIcoSpare29: return QString("enuIhmVirtIcoSpare29");
			case enuIhmVirtIcoSpare30: return QString("enuIhmVirtIcoSpare30");
			case enuIhmVirtIcoSpare31: return QString("enuIhmVirtIcoSpare31");
			case enuIhmVirtIcoSpare32: return QString("enuIhmVirtIcoSpare32");
			case enuIhmVirtIcoSpare33: return QString("enuIhmVirtIcoSpare33");
			case enuIhmVirtIcoSpare34: return QString("enuIhmVirtIcoSpare34");
			case enuIhmVirtIcoSpare35: return QString("enuIhmVirtIcoSpare35");
			case enuIhmVirtIcoSpare36: return QString("enuIhmVirtIcoSpare36");
			case enuIhmVirtIcoSpare37: return QString("enuIhmVirtIcoSpare37");
			case enuIhmVirtIcoSpare38: return QString("enuIhmVirtIcoSpare38");
			case enuIhmVirtIcoSpare39: return QString("enuIhmVirtIcoSpare39");
			case enuIhmVirtIcoSpare40: return QString("enuIhmVirtIcoSpare40");
			case enuIhmVirtIcoSpare41: return QString("enuIhmVirtIcoSpare41");
			case enuIhmVirtIcoSpare42: return QString("enuIhmVirtIcoSpare42");
			case enuIhmVirtIcoSpare43: return QString("enuIhmVirtIcoSpare43");
			case enuIhmVirtIcoSpare44: return QString("enuIhmVirtIcoSpare44");
			case enuIhmVirtIcoSpare45: return QString("enuIhmVirtIcoSpare45");
			case enuIhmVirtIcoSpare46: return QString("enuIhmVirtIcoSpare46");
			case enuIhmVirtIcoSpare47: return QString("enuIhmVirtIcoSpare47");
			case enuIhmVirtIcoSpare48: return QString("enuIhmVirtIcoSpare48");
			case enuIhmVirtIcoSpare49: return QString("enuIhmVirtIcoSpare49");
			case enuIhmVirtIcoSpare50: return QString("enuIhmVirtIcoSpare50");

			case enuIhmVirtIcoCamera1: return QString("enuIhmVirtIcoCamera1");
			case enuIhmVirtIcoCamera2: return QString("enuIhmVirtIcoCamera2");
			case enuIhmVirtIcoBeacon2: return QString("enuIhmVirtIcoBeacon2");
			case enuIhmVirtIcoSignalLight: return QString("enuIhmVirtIcoSignalLight");
			case enuIhmVirtIcoExtOpticalBarrier: return QString("enuIhmVirtIcoExtOpticalBarrier");
			
			//Buttons
			case enuIhmVirtBtnClass: return QString("enuIhmVirtBtnClass");
			case enuIhmVirtBtnEntryPoint: return QString("enuIhmVirtBtnEntryPoint");
			case enuIhmVirtBtnEntryPointNbr: return QString("enuIhmVirtBtnEntryPointNbr");
			case enuIhmVirtBtnMode: return QString("enuIhmVirtBtnMode");
			case enuIhmVirtBtnArrowOn: return QString("enuIhmVirtBtnArrowOn");
			case enuIhmVirtBtnArrowOff: return QString("enuIhmVirtBtnArrowOff");
			case enuIhmVirtBtnCrossOn: return QString("enuIhmVirtBtnCrossOn");
			case enuIhmVirtBtnCrossOff: return QString("enuIhmVirtBtnCrossOff");
			case enuIhmVirtBtnDsrcOn: return QString("enuIhmVirtBtnDsrcOn");
			case enuIhmVirtBtnDsrcOff: return QString("enuIhmVirtBtnDsrcOff");
			case enuIhmVirtBtnMagneticOn: return QString("enuIhmVirtBtnMagneticOn");
			case enuIhmVirtBtnMagneticOff: return QString("enuIhmVirtBtnMagneticOff");
			case enuIhmVirtBtnCollectorOn: return QString("enuIhmVirtBtnCollectorOn");
			case enuIhmVirtBtnCollectorOff: return QString("enuIhmVirtBtnCollectorOff");
			case enuIhmVirtBtnCamera: return QString("enuIhmNoVirtualObjId");
			case enuIhmVirtBtnScanner: return QString("enuIhmVirtBtnScanner");
			case enuIhmVirtBtnComment: return QString("enuIhmVirtBtnComment");
			case enuIhmVirtBtnGabarite: return QString("enuIhmVirtBtnGabarite");
			case enuIhmVirtBtnEntryGate: return QString("enuIhmVirtBtnEntryGate");
			case enuIhmVirtBtnExitGate: return QString("enuIhmVirtBtnExitGate");
			case enuIhmVirtBtnCar3: return QString("enuIhmVirtBtnCar3");
			case enuIhmVirtBtnCar2: return QString("enuIhmVirtBtnCar2");
			case enuIhmVirtBtnCar: return QString("enuIhmVirtBtnCar");
			case enuIhmVirtBtnEntryLoop: return QString("enuIhmVirtBtnEntryLoop");
			case enuIhmVirtBtnExitLoop: return QString("enuIhmVirtBtnExitLoop");
			case enuIhmVirtBtnTblBeacon: return QString("enuIhmVirtBtnTblBeacon");

			case enuIhmVirtBtnCamGrab: return QString("enuIhmVirtBtnCamGrab");
			case enuIhmVirtBtnCamHide: return QString("enuIhmVirtBtnCamHide");
			case enuIhmVirtBtnScanGrab: return QString("enuIhmVirtBtnScanGrab");
			case enuIhmVirtBtnScanHide: return QString("enuIhmVirtBtnScanHide");
			case enuIhmVirtBtnScanSend: return QString("enuIhmVirtBtnScanSend");
			case enuIhmVirtBtnScanSave: return QString("enuIhmVirtBtnScanSave");
			case enuIhmVirtBtnCommHide: return QString("enuIhmVirtBtnCommHide");
			case enuIhmVirtBtnExternalDisplay: return QString("enuIhmVirtBtnExternalDisplay");
			case enuIhmVirtBtnTakeCtrl: return QString("enuIhmVirtBtnTakeCtrl");
			case enuIhmVirtBtnReturnCtrl: return QString("enuIhmVirtBtnReturnCtrl");

			case enuIhmVirtButton1: return QString("enuIhmVirtButton1");
			case enuIhmVirtButton2: return QString("enuIhmVirtButton2");
			case enuIhmVirtButton3: return QString("enuIhmVirtButton3");
			case enuIhmVirtButton4: return QString("enuIhmVirtButton4");
			case enuIhmVirtButton5: return QString("enuIhmVirtButton5");

			case enuIhmVirtBtnSpeedLimitOn: return QString("enuIhmVirtBtnSpeedLimitOn");
			case enuIhmVirtBtnSpeedLimitOff: return QString("enuIhmVirtBtnSpeedLimitOff");
			case enuIhmVirtBtnWarningOn: return QString("enuIhmVirtBtnWarningOn");
			case enuIhmVirtBtnWarningOff: return QString("enuIhmVirtBtnWarningOff");
			case enuIhmVirtBtnDsrcTSAOn: return QString("enuIhmVirtBtnDsrcTSAOn");
			case enuIhmVirtBtnDsrcTSAOff: return QString("enuIhmVirtBtnDsrcTSAOff");
			case enuIhmVirtBtnSpare1: return QString("enuIhmVirtBtnSpare1");
			case enuIhmVirtBtnSpare2: return QString("enuIhmVirtBtnSpare2");
			case enuIhmVirtBtnSpare3: return QString("enuIhmVirtBtnSpare3");
			case enuIhmVirtBtnSpare4: return QString("enuIhmVirtBtnSpare4");
			case enuIhmVirtBtnSpare5: return QString("enuIhmVirtBtnSpare5");
			case enuIhmVirtBtnSpare6: return QString("enuIhmVirtBtnSpare6");
			case enuIhmVirtBtnSpare7: return QString("enuIhmVirtBtnSpare7");
			case enuIhmVirtBtnSpare8: return QString("enuIhmVirtBtnSpare8");
			case enuIhmVirtBtnSpare9: return QString("enuIhmVirtBtnSpare9");
			case enuIhmVirtBtnSpare10: return QString("enuIhmVirtBtnSpare10");
			case enuIhmVirtBtnSpare11: return QString("enuIhmVirtBtnSpare11");
			case enuIhmVirtBtnSpare12: return QString("enuIhmVirtBtnSpare12");
			case enuIhmVirtBtnSpare13: return QString("enuIhmVirtBtnSpare13");
			case enuIhmVirtBtnSpare14: return QString("enuIhmVirtBtnSpare14");
			case enuIhmVirtBtnSpare15: return QString("enuIhmVirtBtnSpare15");
			case enuIhmVirtBtnSpare16: return QString("enuIhmVirtBtnSpare16");
			case enuIhmVirtBtnSpare17: return QString("enuIhmVirtBtnSpare17");
			case enuIhmVirtBtnSpare18: return QString("enuIhmVirtBtnSpare18");
			case enuIhmVirtBtnSpare19: return QString("enuIhmVirtBtnSpare19");
			case enuIhmVirtBtnSpare20: return QString("enuIhmVirtBtnSpare20");
			case enuIhmVirtBtnSpare21: return QString("enuIhmVirtBtnSpare21");
			case enuIhmVirtBtnSpare22: return QString("enuIhmVirtBtnSpare22");
			case enuIhmVirtBtnSpare23: return QString("enuIhmVirtBtnSpare23");
			case enuIhmVirtBtnSpare24: return QString("enuIhmVirtBtnSpare24");
			case enuIhmVirtBtnSpare25: return QString("enuIhmVirtBtnSpare25");
			case enuIhmVirtBtnSpare26: return QString("enuIhmVirtBtnSpare26");
			case enuIhmVirtBtnSpare27: return QString("enuIhmVirtBtnSpare27");
			case enuIhmVirtBtnSpare28: return QString("enuIhmVirtBtnSpare28");
			case enuIhmVirtBtnSpare29: return QString("enuIhmVirtBtnSpare29");
			case enuIhmVirtBtnSpare30: return QString("enuIhmVirtBtnSpare30");
				
			//lists
			case enuIhmVirtLstAlarms: return QString("enuIhmVirtLstAlarms");
			case enuIhmVirtLstWarnings: return QString("enuIhmVirtLstWarnings");

			// Config - case enuIhmDynConfig
			case enuIhmVirtConfProjectName: return QString("enuIhmVirtConfProjectName");
			case enuIhmVirtConfProjectVersion: return QString("enuIhmVirtConfProjectVersion");
			case enuIhmVirtConfLanguage: return QString("enuIhmVirtConfLanguage");
			case enuIhmVirtConfLaneType: return QString("enuIhmVirtConfLaneType");
			case enuIhmVirtConfMainDlgHidden: return QString("enuIhmVirtConfMainDlgHidden");
			case enuIhmVirtConfShiftType: return QString("enuIhmVirtConfShiftType");
			
			//	StringDetection - case enuIhmDynStringDetection
			case enuIhmVirtStrDetectConfigID: return QString("enuIhmVirtStrDetectConfigID");
			
			// Async inputs - case enuIhmAsyncInput
			case enuIhmVirtAsyncInputsID: return QString("enuIhmVirtAsyncInputsID");

			//Others:

			case enuIhmVirtVideo: return QString("enuIhmVirtVideo"); //visible video when showing video
			
			//Visible image when showing image
			//it is the same virtual object type like other icons
			// but also includes its own container
			case enuIhmVirtImage: return QString("enuIhmVirtImage");  
			case enuIhmVirtImage2: return QString("enuIhmVirtImage2");
			
			case enuIhmVirtInputDialogID: return QString("enuIhmVirtInputDialogID");
			case enuIhmVirtInputDlgExID: return QString("enuIhmVirtInputDlgExID");
			
			//to handle sowing file in table view 
			case enuIhmVirtTableView1: return QString("enuIhmVirtTableView1");
			case enuIhmVirtTableView2: return QString("enuIhmVirtTableView2");
			case enuIhmVirtTableView3: return QString("enuIhmVirtTableView3");

			//to handle showing menus on the main form
			case enuIhmVirtHMenuView1: return QString("enuIhmVirtHMenuView1");
			case enuIhmVirtHMenuView2: return QString("enuIhmVirtHMenuView2");
			
			case enuIhmVirtRestreamVideoView1: return QString("enuIhmVirtRestreamVideoView1");
			case enuIhmVirtRestreamVideoView2: return QString("enuIhmVirtRestreamVideoView2");

			case enuIhmVirtCntHeaderGroup: return QString("enuIhmVirtCntHeaderGroup");
			case enuIhmVirtCntInstructionGroup: return QString("enuIhmVirtCntInstructionGroup");
			case enuIhmVirtCntModeGroup: return QString("enuIhmVirtCntModeGroup");
			case enuIhmVirtCntPaymentGroup: return QString("enuIhmVirtCntPaymentGroup");
			case enuIhmVirtCntTrsGroup: return QString("enuIhmVirtCntTrsGroup");

			case enuIhmVirtCntComments: return QString("enuIhmVirtCntComments");
			case enuIhmVirtCntCamera: return QString("enuIhmVirtCntCamera");
			case enuIhmVirtCntScanner: return QString("enuIhmVirtCntScanner");

			case enuIhmVirtCntSpare1: return QString("enuIhmVirtCntSpare1");
			case enuIhmVirtCntSpare2: return QString("enuIhmVirtCntSpare2");
			case enuIhmVirtCntSpare3: return QString("enuIhmVirtCntSpare3");
			case enuIhmVirtCntSpare4: return QString("enuIhmVirtCntSpare4");
			case enuIhmVirtCntSpare5: return QString("enuIhmVirtCntSpare5");
			case enuIhmVirtCntSpare6: return QString("enuIhmVirtCntSpare6");
			case enuIhmVirtCntSpare7: return QString("enuIhmVirtCntSpare7");
			case enuIhmVirtCntSpare8: return QString("enuIhmVirtCntSpare8");
			case enuIhmVirtCntSpare9: return QString("enuIhmVirtCntSpare9");
			case enuIhmVirtCntSpare10: return QString("enuIhmVirtCntSpare10");
			case enuIhmVirtCntSpare11: return QString("enuIhmVirtCntSpare11");
			case enuIhmVirtCntSpare12: return QString("enuIhmVirtCntSpare12");
			case enuIhmVirtCntSpare13: return QString("enuIhmVirtCntSpare13");
			case enuIhmVirtCntSpare14: return QString("enuIhmVirtCntSpare14");
			case enuIhmVirtCntSpare15: return QString("enuIhmVirtCntSpare15");
			case enuIhmVirtCntSpare16: return QString("enuIhmVirtCntSpare16");
			case enuIhmVirtCntSpare17: return QString("enuIhmVirtCntSpare17");
			case enuIhmVirtCntSpare18: return QString("enuIhmVirtCntSpare18");
			case enuIhmVirtCntSpare19: return QString("enuIhmVirtCntSpare19");
			case enuIhmVirtCntSpare20: return QString("enuIhmVirtCntSpare20");

			case enuIhmVirtCntMiniWeb: return QString("enuIhmVirtCntMiniWeb");
			case enuIhmVirtCntAlarmsGroup: return QString("enuIhmVirtCntAlarmsGroup");

			case enuIhmVirtTabControl1: return QString("enuIhmVirtTabControl1");
			case enuIhmVirtTabControl2: return QString("enuIhmVirtTabControl2");
			case enuIhmVirtTabControl3: return QString("enuIhmVirtTabControl3");
			case enuIhmVirtTabControl4: return QString("enuIhmVirtTabControl4");
				
			case enuIhmVirtLastObject: return QString("enuIhmVirtLastObject");

			default:
				TRACE_W(QString("MIhmVirtualObject::getNameForId: Mapping not defined for object %1").arg(eID));
				return QString("Object unknown");
	}
}



MIhmVirtualObject::enumVirtualObjectId MIhmVirtualObject::mapIhmObjIdToVirtObjId(enum_ihm_objets eIhmOBJId)
{
	switch(eIhmOBJId)
	{
		case IHM_OBJ: return enuIhmNoVirtualObjId;
		
		//text labels
		case IHM_OBJ_All_Object: return enuIhmVirtAllObject;
		case IHM_OBJ_PlazaName: return enuIhmVirtPlazaName;
		case IHM_OBJ_PlazaName1: return enuIhmVirtPlazaName1;	
		case IHM_OBJ_PlazaName2: return enuIhmVirtPlazaName2;	
		case IHM_OBJ_PlazaName3: return enuIhmVirtPlazaName3;	
		case IHM_OBJ_PlazaNameTitle: return enuIhmVirtPlazaNameTitle;
		case IHM_OBJ_PlazaNbr: return enuIhmVirtPlazaNbr;
		case IHM_OBJ_PlazaNbrTitle: return enuIhmVirtPlazaNbrTitle;
		case IHM_OBJ_LaneNbr: return enuIhmVirtLaneNbr;
		case IHM_OBJ_LaneNbrTitle: return enuIhmVirtLaneNbrTitle;
		case IHM_OBJ_LaneName: return enuIhmVirtLaneName;          
		case IHM_OBJ_LaneName1: return enuIhmVirtLaneName1;          
		case IHM_OBJ_LaneName2: return enuIhmVirtLaneName2;          
		case IHM_OBJ_LaneNameTitle: return enuIhmVirtLaneNameTitle;	
		case IHM_OBJ_CollectorID: return enuIhmVirtCollectorID;
		case IHM_OBJ_CollectorName: return enuIhmVirtCollectorName;
		
		case IHM_OBJ_TxtSpare1: return enuIhmVirtTxtSpare1;
		case IHM_OBJ_TxtSpare2: return enuIhmVirtTxtSpare2;
		case IHM_OBJ_TxtSpare3: return enuIhmVirtTxtSpare3;
		case IHM_OBJ_TxtSpare4: return enuIhmVirtTxtSpare4;
		case IHM_OBJ_TxtSpare5: return enuIhmVirtTxtSpare5;
		case IHM_OBJ_TxtSpare6: return enuIhmVirtTxtSpare6;
		case IHM_OBJ_TxtSpare7: return enuIhmVirtTxtSpare7;
		case IHM_OBJ_TxtSpare8: return enuIhmVirtTxtSpare8;
		case IHM_OBJ_TxtSpare9: return enuIhmVirtTxtSpare9;
		case IHM_OBJ_TxtSpare10: return enuIhmVirtTxtSpare10;
		case IHM_OBJ_TxtSpare11: return enuIhmVirtTxtSpare11;
		case IHM_OBJ_TxtSpare12: return enuIhmVirtTxtSpare12;
		case IHM_OBJ_TxtSpare13: return enuIhmVirtTxtSpare13;
		case IHM_OBJ_TxtSpare14: return enuIhmVirtTxtSpare14;
		case IHM_OBJ_TxtSpare15: return enuIhmVirtTxtSpare15;
		case IHM_OBJ_TxtSpare16: return enuIhmVirtTxtSpare16;
		case IHM_OBJ_TxtSpare17: return enuIhmVirtTxtSpare17;
		case IHM_OBJ_TxtSpare18: return enuIhmVirtTxtSpare18;
		case IHM_OBJ_TxtSpare19: return enuIhmVirtTxtSpare19;
		case IHM_OBJ_TxtSpare20: return enuIhmVirtTxtSpare20;

		case IHM_OBJ_TxtSpare21: return enuIhmVirtTxtSpare21;
		case IHM_OBJ_TxtSpare22: return enuIhmVirtTxtSpare22;
		case IHM_OBJ_TxtSpare23: return enuIhmVirtTxtSpare23;
		case IHM_OBJ_TxtSpare24: return enuIhmVirtTxtSpare24;
		case IHM_OBJ_TxtSpare25: return enuIhmVirtTxtSpare25;
		case IHM_OBJ_TxtSpare26: return enuIhmVirtTxtSpare26;
		case IHM_OBJ_TxtSpare27: return enuIhmVirtTxtSpare27;
		case IHM_OBJ_TxtSpare28: return enuIhmVirtTxtSpare28;
		case IHM_OBJ_TxtSpare29: return enuIhmVirtTxtSpare29;
		case IHM_OBJ_TxtSpare30: return enuIhmVirtTxtSpare30;
		case IHM_OBJ_TxtSpare31: return enuIhmVirtTxtSpare31;
		case IHM_OBJ_TxtSpare32: return enuIhmVirtTxtSpare32;
		case IHM_OBJ_TxtSpare33: return enuIhmVirtTxtSpare33;
		case IHM_OBJ_TxtSpare34: return enuIhmVirtTxtSpare34;
		case IHM_OBJ_TxtSpare35: return enuIhmVirtTxtSpare35;
		case IHM_OBJ_TxtSpare36: return enuIhmVirtTxtSpare36;
		case IHM_OBJ_TxtSpare37: return enuIhmVirtTxtSpare37;
		case IHM_OBJ_TxtSpare38: return enuIhmVirtTxtSpare38;
		case IHM_OBJ_TxtSpare39: return enuIhmVirtTxtSpare39;
		case IHM_OBJ_TxtSpare40: return enuIhmVirtTxtSpare40;
		case IHM_OBJ_TxtSpare41: return enuIhmVirtTxtSpare41;
		case IHM_OBJ_TxtSpare42: return enuIhmVirtTxtSpare42;
		case IHM_OBJ_TxtSpare43: return enuIhmVirtTxtSpare43;
		case IHM_OBJ_TxtSpare44: return enuIhmVirtTxtSpare44;
		case IHM_OBJ_TxtSpare45: return enuIhmVirtTxtSpare45;
		case IHM_OBJ_TxtSpare46: return enuIhmVirtTxtSpare46;
		case IHM_OBJ_TxtSpare47: return enuIhmVirtTxtSpare47;
		case IHM_OBJ_TxtSpare48: return enuIhmVirtTxtSpare48;
		case IHM_OBJ_TxtSpare49: return enuIhmVirtTxtSpare49;
		case IHM_OBJ_TxtSpare50: return enuIhmVirtTxtSpare50;
		case IHM_OBJ_TxtSpare51: return enuIhmVirtTxtSpare51;
		case IHM_OBJ_TxtSpare52: return enuIhmVirtTxtSpare52;
		case IHM_OBJ_TxtSpare53: return enuIhmVirtTxtSpare53;
		case IHM_OBJ_TxtSpare54: return enuIhmVirtTxtSpare54;
		case IHM_OBJ_TxtSpare55: return enuIhmVirtTxtSpare55;
		case IHM_OBJ_TxtSpare56: return enuIhmVirtTxtSpare56;
		case IHM_OBJ_TxtSpare57: return enuIhmVirtTxtSpare57;
		case IHM_OBJ_TxtSpare58: return enuIhmVirtTxtSpare58;
		case IHM_OBJ_TxtSpare59: return enuIhmVirtTxtSpare59;
		case IHM_OBJ_TxtSpare60: return enuIhmVirtTxtSpare60;


		case IHM_OBJ_TransactionNbr: return enuIhmVirtTransactionNbr;
		case IHM_OBJ_TransactionNbrTitle: return enuIhmVirtTransactionNbrTitle;
		case IHM_OBJ_ReceiptNbr: return enuIhmVirtReceiptNbr;
		case IHM_OBJ_ReceiptNbrTitle: return enuIhmVirtReceiptNbrTitle;
		case IHM_OBJ_Axles: return enuIhmVirtAxles;
		case IHM_OBJ_AxlesTitle: return enuIhmVirtAxlesTitle;
		case IHM_OBJ_Balance: return enuIhmVirtBalance;
		case IHM_OBJ_BalanceTitle: return enuIhmVirtBalanceTitle;
		case IHM_OBJ_Class: return enuIhmVirtClass;
		case IHM_OBJ_ClassTitle: return enuIhmVirtClassTitle;  
		case IHM_OBJ_AdditionalRevenue: return enuIhmVirtAddRevenue;
		case IHM_OBJ_AdditionalRevenueTitle: return enuIhmVirtAddRevenueTitle;

		case IHM_OBJ_NbCar: return enuIhmVirtNbCar;
		case IHM_OBJ_Entry: return enuIhmVirtEntry;
		case IHM_OBJ_EntryTitle: return enuIhmVirtEntryTitle;
		case IHM_OBJ_Currency: return enuIhmVirtCurrency;
		case IHM_OBJ_CurrencyTitle: return enuIhmVirtCurrencyTitle;

		case IHM_OBJ_Fare: return enuIhmVirtFare;
		case IHM_OBJ_FareTitle: return enuIhmVirtFareTitle;
		case IHM_OBJ_Sale: return enuIhmVirtSale;
		case IHM_OBJ_SaleTitle: return enuIhmVirtSaleTitle;
		case IHM_OBJ_SaleDue: return enuIhmVirtSaleDue;
		case IHM_OBJ_SaleDueTitle: return enuIhmVirtSaleDueTitle;
		case IHM_OBJ_SaleDuePaid: return enuIhmVirtSaleDuePaid;
		case IHM_OBJ_TransactionType: return enuIhmVirtTransactionType;
		case IHM_OBJ_TransactionTypeTitle: return enuIhmVirtTransactionTypeTitle;
		case IHM_OBJ_FareType: return enuIhmVirtFareType;
		case IHM_OBJ_FareTypeTitle: return enuIhmVirtFareTypeTitle;

		case IHM_OBJ_TxtModeTitle: return enuIhmVirtTxtModeTitle;
		case IHM_OBJ_TxtMode: return enuIhmVirtTxtMode;
		case IHM_OBJ_TxtCollectorTitle: return enuIhmVirtTxtCollectorTitle;
		case IHM_OBJ_TxtInCtrlUserTitle: return enuIhmVirtTxtInCtrlUserTitle;
		case IHM_OBJ_TxtInCtrlUser: return enuIhmVirtTxtInCtrlUser;		
		case IHM_OBJ_TxtInCtrlUser1: return enuIhmVirtTxtInCtrlUser1;		
		case IHM_OBJ_TxtTrsContainerTitle: return enuIhmVirtTxtTrsContainerTitle;	
		case IHM_OBJ_TxtAlarmsTitle: return enuIhmVirtTxtAlarmsTitle;
		case IHM_OBJ_TxtTrsHistoryTitle: return enuIhmVirtTxtTrsHistoryTitle;
		case IHM_OBJ_TxtAlertesTitle: return enuIhmVirtTxtAlertesTitle;	
		case IHM_OBJ_TxtCamNumPic: return enuIhmVirtTxtCamNumPic;	
		case IHM_OBJ_TxtScanNumPic: return enuIhmVirtTxtScanNumPic;	
		case IHM_OBJ_TxtNbCarSAS: return enuIhmVirtTxtNbCarSAS;	

		//instructions
		case IHM_OBJ_ErrPayment: return enuIhmVirtErrPayment;
		case IHM_OBJ_Payment: return enuIhmVirtPayment;
		case IHM_OBJ_Remark: return enuIhmVirtRemark;
		case IHM_OBJ_Instruction: return enuIhmVirtInstruction;   
		case IHM_OBJ_Instruction2: return enuIhmVirtInstruction2;
		case IHM_OBJ_CurrentDate: return enuIhmVirtCurrentDate;

		// Icons
		case IHM_OBJ_IcoLaneStatus: return enuIhmVirtIcoLaneStatus;
		case IHM_OBJ_IcoLaneMode: return enuIhmVirtIcoLaneMode;
		case IHM_OBJ_IcoCollector: return enuIhmVirtIcoCollector;       
		case IHM_OBJ_IcoEntryGate: return enuIhmVirtIcoEntryGate;			
		case IHM_OBJ_IcoExitGate: return enuIhmVirtIcoExitGate;		
		case IHM_OBJ_IcoCar: return enuIhmVirtIcoCar;
		case IHM_OBJ_IcoTrafficLight: return enuIhmVirtIcoTrafficLight;
		case IHM_OBJ_IcoViolation: return enuIhmVirtIcoViolation;       
		case IHM_OBJ_IcoEntryLoop: return enuIhmVirtIcoEntryLoop;		
		case IHM_OBJ_IcoExitLoop: return enuIhmVirtIcoExitLoop;
		case IHM_OBJ_IcoBeacon: return enuIhmVirtIcoBeacon;
		case IHM_OBJ_IcoEntOpticalBarrier: return enuIhmVirtIcoEntOpticalBarrier;
		
		case IHM_OBJ_IcoSpare1: return enuIhmVirtIcoSpare1;
		case IHM_OBJ_IcoSpare2: return enuIhmVirtIcoSpare2;
		case IHM_OBJ_IcoSpare3: return enuIhmVirtIcoSpare3;	
		case IHM_OBJ_IcoSpare4: return enuIhmVirtIcoSpare4;	
		case IHM_OBJ_IcoSpare5: return enuIhmVirtIcoSpare5;	
		case IHM_OBJ_IcoSpare6: return enuIhmVirtIcoSpare6;	
		case IHM_OBJ_IcoSpare7:	return enuIhmVirtIcoSpare7;	
		case IHM_OBJ_IcoSpare8:	return enuIhmVirtIcoSpare8;	
		case IHM_OBJ_IcoSpare9:	return enuIhmVirtIcoSpare9;	
		case IHM_OBJ_IcoSpare10: return enuIhmVirtIcoSpare10;
		case IHM_OBJ_IcoSpare11: return enuIhmVirtIcoSpare11;
		case IHM_OBJ_IcoSpare12: return enuIhmVirtIcoSpare12;
		case IHM_OBJ_IcoSpare13: return enuIhmVirtIcoSpare13;
		case IHM_OBJ_IcoSpare14: return enuIhmVirtIcoSpare14;
		case IHM_OBJ_IcoSpare15: return enuIhmVirtIcoSpare15;
		case IHM_OBJ_IcoSpare16: return enuIhmVirtIcoSpare16;
		case IHM_OBJ_IcoSpare17: return enuIhmVirtIcoSpare17;
		case IHM_OBJ_IcoSpare18: return enuIhmVirtIcoSpare18;
		case IHM_OBJ_IcoSpare19: return enuIhmVirtIcoSpare19;
		case IHM_OBJ_IcoSpare20:  return enuIhmVirtIcoSpare20;
		case IHM_OBJ_IcoSpare21:  return enuIhmVirtIcoSpare21;
		case IHM_OBJ_IcoSpare22:  return enuIhmVirtIcoSpare22;
		case IHM_OBJ_IcoSpare23:  return enuIhmVirtIcoSpare23;
		case IHM_OBJ_IcoSpare24:  return enuIhmVirtIcoSpare24;
		case IHM_OBJ_IcoSpare25:  return enuIhmVirtIcoSpare25;
		case IHM_OBJ_IcoSpare26:  return enuIhmVirtIcoSpare26;
		case IHM_OBJ_IcoSpare27:  return enuIhmVirtIcoSpare27;
		case IHM_OBJ_IcoSpare28:  return enuIhmVirtIcoSpare28;
		case IHM_OBJ_IcoSpare29:  return enuIhmVirtIcoSpare29;
		case IHM_OBJ_IcoSpare30:  return enuIhmVirtIcoSpare30;
		case IHM_OBJ_IcoSpare31:  return enuIhmVirtIcoSpare31;
		case IHM_OBJ_IcoSpare32:  return enuIhmVirtIcoSpare32;
		case IHM_OBJ_IcoSpare33:  return enuIhmVirtIcoSpare33;
		case IHM_OBJ_IcoSpare34:  return enuIhmVirtIcoSpare34;
		case IHM_OBJ_IcoSpare35:  return enuIhmVirtIcoSpare35;
		case IHM_OBJ_IcoSpare36:  return enuIhmVirtIcoSpare36;
		case IHM_OBJ_IcoSpare37:  return enuIhmVirtIcoSpare37;
		case IHM_OBJ_IcoSpare38:  return enuIhmVirtIcoSpare38;
		case IHM_OBJ_IcoSpare39:  return enuIhmVirtIcoSpare39;
		case IHM_OBJ_IcoSpare40:  return enuIhmVirtIcoSpare40;
		case IHM_OBJ_IcoSpare41:  return enuIhmVirtIcoSpare41;
		case IHM_OBJ_IcoSpare42:  return enuIhmVirtIcoSpare42;
		case IHM_OBJ_IcoSpare43:  return enuIhmVirtIcoSpare43;
		case IHM_OBJ_IcoSpare44:  return enuIhmVirtIcoSpare44;
		case IHM_OBJ_IcoSpare45:  return enuIhmVirtIcoSpare45;
		case IHM_OBJ_IcoSpare46:  return enuIhmVirtIcoSpare46;
		case IHM_OBJ_IcoSpare47:  return enuIhmVirtIcoSpare47;
		case IHM_OBJ_IcoSpare48:  return enuIhmVirtIcoSpare48;
		case IHM_OBJ_IcoSpare49:  return enuIhmVirtIcoSpare49;
		case IHM_OBJ_IcoSpare50:  return enuIhmVirtIcoSpare50;

		case IHM_OBJ_IcoCamera1: return enuIhmVirtIcoCamera1;
		case IHM_OBJ_IcoCamera2: return enuIhmVirtIcoCamera2;
		
		case IHM_OBJ_IcoBeacon2: return enuIhmVirtIcoBeacon2;
		case IHM_OBJ_IcoSignalLight: return enuIhmVirtIcoSignalLight;
		case IHM_OBJ_IcoExtOpticalBarrier: return enuIhmVirtIcoExtOpticalBarrier;
		
		//Buttons
		case IHM_OBJ_btnClass: return enuIhmVirtBtnClass;
		case IHM_OBJ_btnEntryPoint: return enuIhmVirtBtnEntryPoint;
		case IHM_OBJ_btnEntryPointNbr: return enuIhmVirtBtnEntryPointNbr;
		case IHM_OBJ_btnMode: return enuIhmVirtBtnMode;
		case IHM_OBJ_btnArrowOn: return enuIhmVirtBtnArrowOn;
		case IHM_OBJ_btnArrowOff: return enuIhmVirtBtnArrowOff;
		case IHM_OBJ_btnCrossOn: return enuIhmVirtBtnCrossOn;
		case IHM_OBJ_btnCrossOff: return enuIhmVirtBtnCrossOff;
		case IHM_OBJ_btnDsrcOn: return enuIhmVirtBtnDsrcOn;
		case IHM_OBJ_btnDsrcOff: return enuIhmVirtBtnDsrcOff;
		case IHM_OBJ_btnMagneticOn: return enuIhmVirtBtnMagneticOn;
		case IHM_OBJ_btnMagneticOff: return enuIhmVirtBtnMagneticOff;
		case IHM_OBJ_btnCollectorOn: return enuIhmVirtBtnCollectorOn;
		case IHM_OBJ_btnCollectorOff: return enuIhmVirtBtnCollectorOff;
		case IHM_OBJ_btnCamera: return enuIhmVirtBtnCamera;
		case IHM_OBJ_btnScanner: return enuIhmVirtBtnScanner;
		case IHM_OBJ_btnComment: return enuIhmVirtBtnComment;
		case IHM_OBJ_btnGabarite: return enuIhmVirtBtnGabarite;
		case IHM_OBJ_btnEntryGate: return enuIhmVirtBtnEntryGate;
		case IHM_OBJ_btnExitGate: return enuIhmVirtBtnExitGate;
		case IHM_OBJ_btnCar3: return enuIhmVirtBtnCar3;
		case IHM_OBJ_btnCar2: return enuIhmVirtBtnCar2;
		case IHM_OBJ_btnCar: return enuIhmVirtBtnCar;
		case IHM_OBJ_btnEntryLoop: return enuIhmVirtBtnEntryLoop;
		case IHM_OBJ_btnExitLoop: return enuIhmVirtBtnExitLoop;
		case IHM_OBJ_btnTblBeacon: return enuIhmVirtBtnTblBeacon;

		case IHM_OBJ_btnCamGrab: return enuIhmVirtBtnCamGrab;
		case IHM_OBJ_btnCamHide: return enuIhmVirtBtnCamHide;
		case IHM_OBJ_btnScanGrab: return enuIhmVirtBtnScanGrab;
		case IHM_OBJ_btnScanHide: return enuIhmVirtBtnScanHide;
		case IHM_OBJ_btnScanSend: return enuIhmVirtBtnScanSend;
		case IHM_OBJ_btnScanSave: return enuIhmVirtBtnScanSave;
		case IHM_OBJ_btnCommHide: return enuIhmVirtBtnCommHide;
		case IHM_OBJ_btnExternalDisplay: return enuIhmVirtBtnExternalDisplay;
		case IHM_OBJ_btnExternalDisplay2: return enuIhmVirtBtnExternalDisplay2;
		case IHM_OBJ_btnTakeControl: return enuIhmVirtBtnTakeCtrl;
		case IHM_OBJ_btnReturnControl: return enuIhmVirtBtnReturnCtrl;

		case IHM_OBJ_Product1: return enuIhmVirtButton1;
		case IHM_OBJ_Product2: return enuIhmVirtButton2;
		case IHM_OBJ_Product3: return enuIhmVirtButton3;
		case IHM_OBJ_Product4: return enuIhmVirtButton4;
		case IHM_OBJ_Product5: return enuIhmVirtButton5;

		case IHM_OBJ_btnSpeedLimitOn: return enuIhmVirtBtnSpeedLimitOn;
		case IHM_OBJ_btnSpeedLimitOff: return enuIhmVirtBtnSpeedLimitOff;
		case IHM_OBJ_btnWarningOn: return enuIhmVirtBtnWarningOn;
		case IHM_OBJ_btnWarningOff: return enuIhmVirtBtnWarningOff;
		case IHM_OBJ_btnDsrcTSAOn: return enuIhmVirtBtnDsrcTSAOn;
		case IHM_OBJ_btnDsrcTSAOff: return enuIhmVirtBtnDsrcTSAOff;
		case IHM_OBJ_btnSpare1: return enuIhmVirtBtnSpare1;
		case IHM_OBJ_btnSpare2: return enuIhmVirtBtnSpare2;
		case IHM_OBJ_btnSpare3: return enuIhmVirtBtnSpare3;
		case IHM_OBJ_btnSpare4: return enuIhmVirtBtnSpare4;
		case IHM_OBJ_btnSpare5: return enuIhmVirtBtnSpare5;
		case IHM_OBJ_btnSpare6: return enuIhmVirtBtnSpare6;
		case IHM_OBJ_btnSpare7: return enuIhmVirtBtnSpare7;
		case IHM_OBJ_btnSpare8: return enuIhmVirtBtnSpare8;
		case IHM_OBJ_btnSpare9: return enuIhmVirtBtnSpare9;
		case IHM_OBJ_btnSpare10: return enuIhmVirtBtnSpare10;
		case IHM_OBJ_btnSpare11: return enuIhmVirtBtnSpare11;
		case IHM_OBJ_btnSpare12: return enuIhmVirtBtnSpare12;
		case IHM_OBJ_btnSpare13: return enuIhmVirtBtnSpare13;
		case IHM_OBJ_btnSpare14: return enuIhmVirtBtnSpare14;
		case IHM_OBJ_btnSpare15: return enuIhmVirtBtnSpare15;
		case IHM_OBJ_btnSpare16: return enuIhmVirtBtnSpare16;
		case IHM_OBJ_btnSpare17: return enuIhmVirtBtnSpare17;
		case IHM_OBJ_btnSpare18: return enuIhmVirtBtnSpare18;
		case IHM_OBJ_btnSpare19: return enuIhmVirtBtnSpare19;
		case IHM_OBJ_btnSpare20: return enuIhmVirtBtnSpare20;
		case IHM_OBJ_btnSpare21: return enuIhmVirtBtnSpare21;
		case IHM_OBJ_btnSpare22: return enuIhmVirtBtnSpare22;
		case IHM_OBJ_btnSpare23: return enuIhmVirtBtnSpare23;
		case IHM_OBJ_btnSpare24: return enuIhmVirtBtnSpare24;
		case IHM_OBJ_btnSpare25: return enuIhmVirtBtnSpare25;
		case IHM_OBJ_btnSpare26: return enuIhmVirtBtnSpare26;
		case IHM_OBJ_btnSpare27: return enuIhmVirtBtnSpare27;
		case IHM_OBJ_btnSpare28: return enuIhmVirtBtnSpare28;
		case IHM_OBJ_btnSpare29: return enuIhmVirtBtnSpare29;
		case IHM_OBJ_btnSpare30: return enuIhmVirtBtnSpare30;
			
		//lists
		case IHM_OBJ_LstAlarms: return enuIhmVirtLstAlarms;
		case IHM_OBJ_LstWarnings: return enuIhmVirtLstWarnings;

	// 	// Config - case enuIhmDynConfig
	// 	case IHM_OBJ: return enuIhmVirtConfProjectName;
	// 	case IHM_OBJ: return enuIhmVirtConfProjectVersion;
	// 	case IHM_OBJ: return enuIhmVirtConfLanguage;
	// 	case IHM_OBJ: return enuIhmVirtConfLaneType;
	// 	case IHM_OBJ: return enuIhmVirtConfShiftType;
	// 	
	// 	//	StringDetection - case enuIhmDynStringDetection
	// 	case IHM_OBJ: return enuIhmVirtStrDetectConfigID;
	// 	
	// 	// Async inputs - case enuIhmAsyncInput
	// 	case IHM_OBJ: return enuIhmVirtAsyncInputsID;
	// 
	// 	//Others:
	// 	case IHM_OBJ: return enuIhmVirtVideo; 
	// 	
		//Visible image when showing image
		//it is the same virtual object type like other icons
		// but also includes its own container
		case IHM_OBJ_Image: return enuIhmVirtImage;  
		case IHM_OBJ_Image2: return enuIhmVirtImage2;
		
// 		case IHM_OBJ: return enuIhmVirtInputDialogID;
// 		case IHM_OBJ: return enuIhmVirtInputDlgExID;
		
		//to handle sowing file in table view 
		case IHM_OBJ_TableView1: return enuIhmVirtTableView1;
		case IHM_OBJ_TableView2: return enuIhmVirtTableView2;
		case IHM_OBJ_TableView3: return enuIhmVirtTableView3;

		//to handle showing menus on the main form
		case IHM_OBJ_HMenuView1: return enuIhmVirtHMenuView1;
		case IHM_OBJ_HMenuView2: return enuIhmVirtHMenuView2;
		
		//to handle showing video vindows on the main form
		case IHM_OBJ_RVideoView1: return enuIhmVirtRestreamVideoView1;
		case IHM_OBJ_RVideoView2: return enuIhmVirtRestreamVideoView2;

		case IHM_OBJ_CNT_HeaderGroup: return enuIhmVirtCntHeaderGroup;
		case IHM_OBJ_CNT_InstructionGroup: return enuIhmVirtCntInstructionGroup;
		case IHM_OBJ_CNT_ModeGroup: return enuIhmVirtCntModeGroup;
		case IHM_OBJ_CNT_PaymentGroup: return enuIhmVirtCntPaymentGroup;
		case IHM_OBJ_CNT_TrsGroup: return enuIhmVirtCntTrsGroup;

		case IHM_OBJ_CNT_Comments: return enuIhmVirtCntComments;
		case IHM_OBJ_CNT_Camera: return enuIhmVirtCntCamera;
		case IHM_OBJ_CNT_Scanner: return enuIhmVirtCntScanner;

		case IHM_OBJ_CNT_Spare1: return enuIhmVirtCntSpare1;
		case IHM_OBJ_CNT_Spare2: return enuIhmVirtCntSpare2;
		case IHM_OBJ_CNT_Spare3: return enuIhmVirtCntSpare3;
		case IHM_OBJ_CNT_Spare4: return enuIhmVirtCntSpare4;
		case IHM_OBJ_CNT_Spare5: return enuIhmVirtCntSpare5;
		case IHM_OBJ_CNT_Spare6: return enuIhmVirtCntSpare6;
		case IHM_OBJ_CNT_Spare7: return enuIhmVirtCntSpare7;
		case IHM_OBJ_CNT_Spare8: return enuIhmVirtCntSpare8;
		case IHM_OBJ_CNT_Spare9: return enuIhmVirtCntSpare9;
		case IHM_OBJ_CNT_Spare10: return enuIhmVirtCntSpare10;
		case IHM_OBJ_CNT_Spare11: return enuIhmVirtCntSpare11;
		case IHM_OBJ_CNT_Spare12: return enuIhmVirtCntSpare12;
		case IHM_OBJ_CNT_Spare13: return enuIhmVirtCntSpare13;
		case IHM_OBJ_CNT_Spare14: return enuIhmVirtCntSpare14;
		case IHM_OBJ_CNT_Spare15: return enuIhmVirtCntSpare15;
		case IHM_OBJ_CNT_Spare16: return enuIhmVirtCntSpare16;
		case IHM_OBJ_CNT_Spare17: return enuIhmVirtCntSpare17;
		case IHM_OBJ_CNT_Spare18: return enuIhmVirtCntSpare18;
		case IHM_OBJ_CNT_Spare19: return enuIhmVirtCntSpare19;
		case IHM_OBJ_CNT_Spare20: return enuIhmVirtCntSpare20;
		case IHM_OBJ_CNT_MiniWeb: return enuIhmVirtCntMiniWeb;
		case IHM_OBJ_CNT_AlarmsGroup: return enuIhmVirtCntAlarmsGroup;

		case IHM_OBJ_TabControl1: return enuIhmVirtTabControl1;
		case IHM_OBJ_TabControl2: return enuIhmVirtTabControl2;
		case IHM_OBJ_TabControl3: return enuIhmVirtTabControl3;
		case IHM_OBJ_TabControl4: return enuIhmVirtTabControl4;

		//case IHM_OBJ: return enuIhmVirtLastObject;
		default:
			return enuIhmNoVirtualObjId;
	}

}


enum_ihm_objets  MIhmVirtualObject::mapVirtObjIdToIhmObjId(enumVirtualObjectId eVirtId)
{
	for(enum_ihm_objets i = IHM_OBJ_All_Object; i < IHM_OBJ_LastObject; i=(enum_ihm_objets)(i+1))
	{
		enumVirtualObjectId eVirtIdMapped = mapIhmObjIdToVirtObjId(i);
		
		if(eVirtId == eVirtIdMapped)
			return i;
	}

	TRACE_W(QString("MIhmVirtualObject::mapVirtObjIdToIhmObjId: Mapping not defined for virtual object object %1").arg(eVirtId));

	return IHM_OBJ;
}




// MIhmVirtualObject * MIhmVirtualObject::newCopy()
// {
// 	MIhmVirtualObject * pNewObjRet = NULL;
// 
// 	switch(this->getType())
// 	{
// 		case enuIhmContainer:
// 			{
// 				MIhmVirtContainer *pNewObj = new MIhmVirtContainer(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmIco:
// 			{
// 				MIhmVirtIco *pNewObj = new MIhmVirtIco(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmText:
// 			{
// 				MIhmVirtText *pNewObj = new MIhmVirtText(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmButton:
// 			{
// 				MIhmVirtButton *pNewObj = new MIhmVirtButton(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmList:
// 			{
// 				MIhmVirtList * pNewObj = new MIhmVirtList(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmDynConfig:
// 			{
// 				MIhmVirtDynConfig * pNewObj = new MIhmVirtDynConfig(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmDynStrDetection:
// 			{
// 				MIhmVirtStrDetectConfig * pNewObj = new MIhmVirtStrDetectConfig;	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 
// 		case enuIhmInputDialogReq:
// 			{
// 				MIhmVirtInputDialogReq * pNewObj = new MIhmVirtInputDialogReq;	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmInputDlgExReq:
// 			{
// 				MIhmVirtInputDlgExReq * pNewObj = new MIhmVirtInputDlgExReq;
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmTableViewReq:
// 			{
// 				MIhmVirtTableViewReq * pNewObj = new MIhmVirtTableViewReq(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmHMenuViewReq:
// 			{
// 				MIhmVirtHMenuViewReq * pNewObj = new MIhmVirtHMenuViewReq(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmVideoViewReq:
// 			{
// 				MIhmVirtVideoViewReq * pNewObj = new MIhmVirtVideoViewReq(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmTabView:
// 			{
// 				MIhmVirtTabView * pNewObj = new MIhmVirtTabView(this->getId());	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		case enuIhmAsyncInput:
// 			{
// 				MIhmVirtAsyncInput * pNewObj = new MIhmVirtAsyncInput();	
// 				*pNewObj = *this;
// 				pNewObjRet = pNewObj;
// 			}
// 			break;
// 		
// 	
// 
// 	default: //enuIhmNoVirtualObjType
// 		TRACE_W(QString( "MIhmVirtualObject::copy: Error copying object type: %1")
// 									.arg((int)this->getType()));
// 	
// 	    break;
// 	}
// 
// 	return pNewObjRet;
// }


MIhmVirtualObject * MIhmVirtualObject::findVirtualObjectByID(QList <MIhmVirtualObject*> *plst,   MIhmVirtualObject::enumVirtualObjectId eID)
{
	for (int i = 0; i < plst->size(); i++) 
	{
		if(plst->at(i)->getId()==eID)
		{
			return plst->at(i);
		}
	}

	return NULL;
}

bool MIhmVirtualObject::removeVirtualObjectByID(QList <MIhmVirtualObject*> *plst,   MIhmVirtualObject::enumVirtualObjectId eID)
{
	bool bOK = false;

	for (int i = 0; i < plst->size(); ++i) 
	{
		if(plst->at(i)->getId()==eID)
		{
			delete plst->takeAt(i);
			bOK = true;
			break;
		}
	}

	return bOK;
}


MIhmVirtualObject& MIhmVirtualObject::operator=(const MIhmVirtualObject& right)
{
	m_bVisible = right.m_bVisible;
	m_bObjUpdated = right.m_bObjUpdated;
	m_eID = right.m_eID;

	return *this;
}


void MIhmVirtualObject::clearUpdated()
{ 
	m_bObjUpdated = false; 
}

void MIhmVirtualObject::setUpdated()
{ 
   if(m_bObjInitialized)
		m_bObjUpdated = true;
}

bool MIhmVirtualObject::isUpdated() const 
{ 
		return m_bObjUpdated; 
}


bool MIhmVirtualObject::setVisible(bool b)
{  
	if(m_bVisible != b)
	{
		m_bVisible = b;
		return true;
	}
	else
		return false;
}

void MIhmVirtualObject::setInitialized()
{
	m_bObjInitialized = true;
}




//--------------------------------------------
//MIhmVirtDynConfig
//--------------------------------------------
MIhmVirtDynConfig::MIhmVirtDynConfig(enumVirtualObjectId eID)
{ 
	m_bVisible = false;
	m_eID = eID; 
	m_bObjUpdated = false;
}


MIhmVirtDynConfig::MIhmVirtDynConfig(MIhmVirtDynConfig &obj)
{

	*this = obj;
}


MIhmVirtualObject& MIhmVirtDynConfig::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtDynConfig *pD = dynamic_cast<const MIhmVirtDynConfig*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_sDynConfigSetting = MHelpFuncs::deepCopy(pD->m_sDynConfigSetting);

	return *this;
}

bool MIhmVirtDynConfig::setDynConfigSetting(QString sDynConfigSetting)
{	
	if(!(m_sDynConfigSetting==sDynConfigSetting))
	{
		m_sDynConfigSetting = sDynConfigSetting;
		return true;
	}
	else
		return false;
}

//---------------------------------------
//MIhmVirtContainer
//---------------------------------------

MIhmVirtContainer::MIhmVirtContainer(enumVirtualObjectId eID)
{ 
	m_bVisible = false;
	m_bObjUpdated = false;
	m_eID = eID; 
}


MIhmVirtContainer::MIhmVirtContainer(MIhmVirtContainer &obj)
{
	*this = obj;

}

MIhmVirtualObject& MIhmVirtContainer::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtContainer *pD = dynamic_cast<const MIhmVirtContainer*>(&right);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();
	
	m_oPoliceValue = pD->m_oPoliceValue;

	if(pD!=NULL)
		m_sToolTip = MHelpFuncs::deepCopy(pD->m_sToolTip);
	
	return *this;
}

bool MIhmVirtContainer::setToolTip(QString sToolTip)
{	
	if(!(m_sToolTip==sToolTip))
	{
		m_sToolTip = sToolTip;
		return true;
	}
	else
		return false;
}

//---------------------------------------
//MIhmVirtIco
//---------------------------------------
MIhmVirtIco::MIhmVirtIco(enumVirtualObjectId eID)
				:MIhmVirtContainer(eID)
{
	m_sImageID = "";
}



MIhmVirtIco::MIhmVirtIco(MIhmVirtIco &obj)
	:MIhmVirtContainer(obj.getId())
{
	*this = obj;
}


MIhmVirtualObject& MIhmVirtIco::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtIco *pD = dynamic_cast<const MIhmVirtIco*>(&right);

	Q_ASSERT(pD!=NULL);
	
	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_oPoliceValue = pD->m_oPoliceValue;
	m_sImageID = MHelpFuncs::deepCopy(pD->m_sImageID);
	m_sToolTip = MHelpFuncs::deepCopy(pD->m_sToolTip);

	return *this;
}

bool MIhmVirtIco::setImageID(QString sImage)
{	
	if(!(m_sImageID == sImage))
	{
		m_sImageID = sImage;
		return true;
	}
	else
		return false;
}

bool MIhmVirtIco::setEnabled(bool b)
{	
	if(m_bEnabled!=b)
	{
		m_bEnabled = b;
		return true;
	}
	else
		return false;
}

//---------------------------------------
//MIhmVirtText
//---------------------------------------
MIhmVirtText::MIhmVirtText(enumVirtualObjectId eID)
			:MIhmVirtContainer(eID)
{
	m_sTextID = "";	   
}


MIhmVirtText::MIhmVirtText(MIhmVirtText &obj)
	:MIhmVirtContainer(obj.getId())
{
	*this = obj;

}

MIhmVirtualObject& MIhmVirtText::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtText *pD = dynamic_cast<const MIhmVirtText*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_oPoliceValue = pD->m_oPoliceValue;
	m_sTextID = MHelpFuncs::deepCopy(pD->m_sTextID);
	m_sToolTip = MHelpFuncs::deepCopy(pD->m_sToolTip);	   

	return *this;
}


bool MIhmVirtText::setTextID(QString sText)
{
	if(!(m_sTextID==sText))
	{
		m_sTextID = sText;
		return true;
	}
	else
		return false;
}

bool MIhmVirtText::setEnabled(bool b)
{	
	if(m_bEnabled!=b)
	{
		m_bEnabled = b;
		return true;
	}
	else
		return false;
}
//---------------------------------------
//END MIhmVirtText
//---------------------------------------


//---------------------------------------
//START MIhmVirtButton
//---------------------------------------

MIhmVirtButton::MIhmVirtButton(enumVirtualObjectId eID)
			:MIhmVirtContainer(eID)
{
	m_sTextID = "";	   
	m_sImageID = "";
	m_bEnabled = false;
}


MIhmVirtButton::MIhmVirtButton(MIhmVirtButton &obj)
	:MIhmVirtContainer(obj.getId())
{
	*this = obj;

}

MIhmVirtualObject& MIhmVirtButton::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtButton *pD = dynamic_cast<const MIhmVirtButton*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_sImageID = MHelpFuncs::deepCopy(pD->m_sImageID);
	m_sTextID = MHelpFuncs::deepCopy(pD->m_sTextID);	   
	m_bEnabled = pD->m_bEnabled;
	m_sToolTip = MHelpFuncs::deepCopy(pD->m_sToolTip);	  

	return *this;
}

bool MIhmVirtButton::setTextID(QString sText)
{
	if(!(sText==m_sTextID))
	{
		m_sTextID = sText;
		return true;
	}
	else
		return false;
}

bool MIhmVirtButton::setImageID(QString sImage)
{	
	if(!(m_sImageID==sImage))
	{
		m_sImageID = sImage;
		return true;
	}
	else
		return false;
}	

bool MIhmVirtButton::setEnabled(bool b)
{	
	if(m_bEnabled!=b)
	{
		m_bEnabled = b;
		return true;
	}
	else
		return false;
}

//---------------------------------------
//END MIhmVirtButton
//---------------------------------------














//---------------------------------------
//MIhmVirtList
//---------------------------------------
MIhmVirtList::MIhmVirtList(enumVirtualObjectId eID)
		:MIhmVirtContainer(eID)
{

}


MIhmVirtList::MIhmVirtList(MIhmVirtList &obj)
	:MIhmVirtContainer(obj.getId())
{
	*this = obj;

}


MIhmVirtualObject& MIhmVirtList::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtList *pD = dynamic_cast<const MIhmVirtList*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_oPoliceValue = pD->m_oPoliceValue;

	m_slistIDs.clear();

	const QStringList *lst = &(pD->m_slistIDs);

	for(int i=0; i<lst->size(); ++i )
	{
		m_slistIDs.append(MHelpFuncs::deepCopy(lst->at(i)));
	}
		
	

	return *this;
}

//---------------------------------------
//MIhmVirtStrDetectConfig
//---------------------------------------

MIhmVirtStrDetectConfig::MIhmVirtStrDetectConfig(MIhmVirtStrDetectConfig &obj)
{
	*this = obj;
}
 
MIhmVirtStrDetectConfig::MIhmVirtStrDetectConfig()
{ 
	m_bVisible = false;
	m_bObjUpdated = false;

	m_eID = enuIhmVirtStrDetectConfigID;
	m_lstActivePairs.clear();
}


MIhmVirtualObject& MIhmVirtStrDetectConfig::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtStrDetectConfig *pD = dynamic_cast<const MIhmVirtStrDetectConfig*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	 while (!m_lstActivePairs.isEmpty())
		delete m_lstActivePairs.takeFirst();

	const QList <MStrDetectionPair*> *lst = &(pD->m_lstActivePairs);

	for(int i=0; i<lst->size(); ++i )
	{
		MStrDetectionPair *pCurrentPair = lst->at(i);
		MStrDetectionPair *pNewPair = new MStrDetectionPair(*pCurrentPair);	  
		m_lstActivePairs.append(pNewPair);
	}

	return *this;
}


MIhmVirtStrDetectConfig::~MIhmVirtStrDetectConfig()
{
	while(!m_lstActivePairs.isEmpty())
		delete m_lstActivePairs.takeLast();

}


bool MIhmVirtStrDetectConfig::updateDetectionPair(MStrDetectionPair* pNewPair)
{
	bool bFound = false;

	for(int i=0; i<m_lstActivePairs.size(); ++i )
	{
		MStrDetectionPair *pCurrentPair = m_lstActivePairs.at(i);
		
		if(pNewPair->getName() == pCurrentPair->getName())
		{
			*pCurrentPair = *pNewPair;
			bFound = true;
		}
	}

	if(!bFound)
	{
		MStrDetectionPair *pAddPair = new MStrDetectionPair(pNewPair->getName());	  
		*pAddPair = *pNewPair;
		m_lstActivePairs.append(pAddPair);

		bFound = true;
	}

	return bFound;
}

void MIhmVirtStrDetectConfig::removeDetectionPair(MStrDetectionPair* pPair2Remove)
{
	for(int i=0; i<m_lstActivePairs.size(); ++i )
	{
		MStrDetectionPair *pCurrentPair = m_lstActivePairs.at(i);

		if(pPair2Remove->getName() == pCurrentPair->getName())
		{
			pCurrentPair->m_bActive = false;
		}
	}

}

//---------------------------------------
//MIhmVirtInputDialogReq
//---------------------------------------

MIhmVirtInputDialogReq::MIhmVirtInputDialogReq(MIhmVirtInputDialogReq &obj)
{
	*this = obj;

}

MIhmVirtInputDialogReq::MIhmVirtInputDialogReq()
{
	m_bVisible = false;
	m_bObjUpdated = false;
	m_bIsCancel = false;

	m_eID = enuIhmVirtInputDialogID;
	m_lstSasieVisuRequests.clear();

}

MIhmVirtInputDialogReq::~MIhmVirtInputDialogReq()
{
	while(!m_lstSasieVisuRequests.isEmpty())
		delete m_lstSasieVisuRequests.takeLast();

}


MIhmVirtualObject& MIhmVirtInputDialogReq::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtInputDialogReq *pD = dynamic_cast<const MIhmVirtInputDialogReq*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	{
		m_bIsCancel = pD->isCancel();

		 while (!m_lstSasieVisuRequests.isEmpty())
			delete m_lstSasieVisuRequests.takeFirst();

		const QList <MInputDialogReq*> *lst = &(pD->m_lstSasieVisuRequests);

		for(int i=0; i<lst->size(); ++i )
		{
			MInputDialogReq *pCurrent = lst->at(i);
			MInputDialogReq *pNew = new MInputDialogReq(*pCurrent);	   
			m_lstSasieVisuRequests.append(pNew);
		}
	}

	return *this;
}


void MIhmVirtInputDialogReq::addReq(MInputDialogReq* pReq)
{
	MInputDialogReq *pNew = new MInputDialogReq(*pReq);	
	m_lstSasieVisuRequests.append(pNew);
}

void MIhmVirtInputDialogReq::removeReq(MInputDialogReq* pReq)
{
	for(int i=0; i<m_lstSasieVisuRequests.size(); ++i )
	{
		MInputDialogReq *pCurrent = m_lstSasieVisuRequests.at(i);

		if(*pReq == *pCurrent)
		{
			delete m_lstSasieVisuRequests.takeAt(i);
			break;
		}
	}

}

bool MIhmVirtInputDialogReq::updateReqDescription(MInputDialogReq* pReq, QString sCurrInputResult)
{
	for(int i=0; i<m_lstSasieVisuRequests.size(); ++i )
	{
		MInputDialogReq *pCurrent = m_lstSasieVisuRequests.at(i);

		if(*pReq == *pCurrent)
		{
			pCurrent->setDescription(sCurrInputResult);
			return pCurrent->isValid();
		}
	}

	return false;
}


void MIhmVirtInputDialogReq::clearAllReq()
{
	 while (!m_lstSasieVisuRequests.isEmpty())
		delete m_lstSasieVisuRequests.takeFirst();
}


bool MIhmVirtInputDialogReq::isCancel() const
{
	return m_bIsCancel;
}

void MIhmVirtInputDialogReq::setCancel()
{
	m_bIsCancel = true;
}


bool MIhmVirtInputDialogReq::isEmpty()
{
	if(m_lstSasieVisuRequests.isEmpty())
		return true;
	else
		return false;
}

void MIhmVirtInputDialogReq::confirmCanceled()
{
	m_bIsCancel = false;
}


bool MIhmVirtInputDialogReq::getCurrentReq(MInputDialogReq* pReq)
{
	if(!m_lstSasieVisuRequests.isEmpty())
	{
		*pReq =	*(m_lstSasieVisuRequests.first());
		return true;	
	}
	else 
		return false;
}





//---------------------------------------
//MIhmVirtInputDlgExReq
//---------------------------------------

MIhmVirtInputDlgExReq::MIhmVirtInputDlgExReq(MIhmVirtInputDlgExReq &obj)
{
	*this = obj;

}

MIhmVirtInputDlgExReq::MIhmVirtInputDlgExReq()
{
	m_bVisible = false;
	m_bObjUpdated = false;
	m_bIsCancel = false;

	m_eID = enuIhmVirtInputDlgExID;
	m_lstSasieVisuRequests.clear();

}

MIhmVirtInputDlgExReq::~MIhmVirtInputDlgExReq()
{
	while(!m_lstSasieVisuRequests.isEmpty())
		delete m_lstSasieVisuRequests.takeLast();
}


MIhmVirtualObject& MIhmVirtInputDlgExReq::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtInputDlgExReq *pD = dynamic_cast<const MIhmVirtInputDlgExReq*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_bIsCancel = pD->isCancel();

	 while (!m_lstSasieVisuRequests.isEmpty())
		delete m_lstSasieVisuRequests.takeFirst();

	const QList <MInputDialogExReq*> *lst = &(pD->m_lstSasieVisuRequests);

	for(int i=0; i<lst->size(); ++i )
	{
		MInputDialogExReq *pCurrent = lst->at(i);
		MInputDialogExReq *pNew = new MInputDialogExReq(*pCurrent);	  
		m_lstSasieVisuRequests.append(pNew);
	}

	return *this;
}


void MIhmVirtInputDlgExReq::addReq(MInputDialogExReq* pReq)
{
	MInputDialogExReq *pNew = new MInputDialogExReq(*pReq);	
	m_lstSasieVisuRequests.append(pNew);
}

bool MIhmVirtInputDlgExReq::removeReq(MInputDialogExReq* pReq)
{
	bool bFound = false;

	for(int i=0; i<m_lstSasieVisuRequests.size(); ++i )
	{
		MInputDialogExReq *pCurrent = m_lstSasieVisuRequests.at(i);

		if(*pReq == *pCurrent)
		{
			delete m_lstSasieVisuRequests.takeAt(i);
			bFound = true;
			break;
		}
	}

	return bFound;
}

MInputDialogExReq* MIhmVirtInputDlgExReq::takeReq(QString sDlgId)
{
	MInputDialogExReq* pWantedRequest = NULL;

	for(int i=0; i<m_lstSasieVisuRequests.size(); ++i )
	{
		MInputDialogExReq *pCurrent = m_lstSasieVisuRequests.at(i);

		if(pCurrent->getDialogId() == sDlgId)
		{
			pWantedRequest = m_lstSasieVisuRequests.takeAt(i);
			break;
		}
	}

	return pWantedRequest;
}


MInputDialogExReq* MIhmVirtInputDlgExReq::takeFirstReq()
{
	MInputDialogExReq* pWantedRequest = NULL;

	if(!m_lstSasieVisuRequests.isEmpty())
	{
		pWantedRequest = m_lstSasieVisuRequests.takeFirst();
	}

	return pWantedRequest;
}

MInputDialogExReq* MIhmVirtInputDlgExReq::getFirstReq()
{
	MInputDialogExReq* pWantedRequest = NULL;

	if(!m_lstSasieVisuRequests.isEmpty())
	{
		pWantedRequest = m_lstSasieVisuRequests.first();
	}

	return pWantedRequest;
}


bool MIhmVirtInputDlgExReq::updateReqDescription(MInputDialogExReq* pReq, QString sInputDescUpdate)
{
	for(int i=0; i<m_lstSasieVisuRequests.size(); ++i )
	{
		MInputDialogExReq *pCurrent = m_lstSasieVisuRequests.at(i);

		if(*pReq == *pCurrent)
		{
			pCurrent->setDescription(sInputDescUpdate, 0);
			return pCurrent->isValid();
		}
	}

	return false;
}


void MIhmVirtInputDlgExReq::clearAllReq()
{
	 while (!m_lstSasieVisuRequests.isEmpty())
		delete m_lstSasieVisuRequests.takeFirst();
}


bool MIhmVirtInputDlgExReq::isCancel() const
{
	return m_bIsCancel;
}

void MIhmVirtInputDlgExReq::setCancel()
{
	m_bIsCancel = true;
}


bool MIhmVirtInputDlgExReq::isEmpty()
{
	if(m_lstSasieVisuRequests.isEmpty())
		return true;
	else
		return false;
}

void MIhmVirtInputDlgExReq::confirmCanceled()
{
	m_bIsCancel = false;
}


bool MIhmVirtInputDlgExReq::getCurrentExReq(MInputDialogExReq* pReq)
{
	if(!m_lstSasieVisuRequests.isEmpty())
	{
		*pReq =	*(m_lstSasieVisuRequests.first());
		return true;	
	}
	else 
		return false;
}

//---------------------------------------
//MIhmVirtTableViewReq
//---------------------------------------
MIhmVirtTableViewReq::MIhmVirtTableViewReq(MIhmVirtTableViewReq &obj)
{
	*this = obj;

}

MIhmVirtTableViewReq::MIhmVirtTableViewReq(enumVirtualObjectId eID)
{
	m_eID = eID;
	m_uiLastUpdateTimestamp = 0;
	m_lstRowsCommands.clear();
	m_iTimeStampCounter = 0;
}

MIhmVirtTableViewReq::~MIhmVirtTableViewReq()
{
	clearCommandList();
}




MIhmVirtualObject& MIhmVirtTableViewReq::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtTableViewReq *pD = dynamic_cast<const MIhmVirtTableViewReq*>(&right);
	bool bDone = false;
	int iCount; 

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

		iCount = m_lstRowsCommands.count();

		if(iCount <= pD->m_lstRowsCommands.count())
		{
			int i = 0;
			
			MRowsCommand * pCmd1;
			MRowsCommand * pCmd2;

			while( i<iCount )
			{
				pCmd1 = m_lstRowsCommands.at(i);
				pCmd2 = pD->m_lstRowsCommands.at(i);

				if(pCmd1==NULL || pCmd2==NULL)
				{ 
					TRACE_W(QString("MIhmVirtTableViewReq::operator=: Fatal error found NULL in the list!"))
					ExitBad();	
				}

				if(pCmd1->m_uiCommandTimestamp != pCmd2->m_uiCommandTimestamp)
					break;

				i++;
			}

			//make the additional commands copy
			MRowsCommand * pNewCommand;
			MRowsCommand * pCurrentCommand;

			for(;i<iCount; i++)
			{
				pCurrentCommand = pD->m_lstRowsCommands.at(i);
				pNewCommand = m_lstRowsCommands.at(i);
				*pNewCommand = *pCurrentCommand;
			}

			int iCount2 = pD->m_lstRowsCommands.count();

			for(;i<iCount2; i++)
			{
				pCurrentCommand = pD->m_lstRowsCommands.at(i);
				pNewCommand = new MRowsCommand();
				*pNewCommand = *pCurrentCommand;
				m_lstRowsCommands.append(pNewCommand);
			}

			if(pD->m_lstRowsCommands.count()==m_lstRowsCommands.count()) 
			{
				bDone = true;
			}
			else
			{
				TRACE_W(QString("MIhmVirtTableViewReq::operator=: Error while copying! Will do the complete copy..."))
			}
		}

		
		if(!bDone) // do the complete copy
		{
			clearCommandList();
			
			//make a commands list copy
			MRowsCommand * pNewCommand;
			MRowsCommand * pCurrentCommand;

			for(int i=0; i<pD->m_lstRowsCommands.count(); i++)
			{
				pCurrentCommand = pD->m_lstRowsCommands.at(i);
				pNewCommand = new MRowsCommand();
				*pNewCommand = *pCurrentCommand;
				m_lstRowsCommands.append(pNewCommand);
			}
		}


		//verify if copy is ok
// 		iCount = m_lstRowsCommands.count();
// 		
// 		if(pD->m_lstRowsCommands.count()==iCount)
// 		{
// 			MRowsCommand * pCmd1;
// 			MRowsCommand * pCmd2;
// 			bool bOK = true;
// 
// 			for(int i=0; i<pD->m_lstRowsCommands.count(); i++)
// 			{
// 				pCmd1 = m_lstRowsCommands.at(i);
// 				pCmd2 = pD->m_lstRowsCommands.at(i);
// 
// 				if(pCmd1==NULL || pCmd2==NULL)
// 				{ 
// 					bOK = false;
// 					break;
// 				}
// 
// 				if(pCmd1->m_uiCommandTimestamp != pCmd2->m_uiCommandTimestamp)
// 				{ 
// 					bOK = false;
// 					break;
// 				}
// 				
// 				if(!(*pCmd1==*pCmd2))
// 				{
// 					bOK = false;
// 					break;
// 				}
// 			}
// 
// 			if(!bOK)
// 			{
// 				TRACE_W(QString("MIhmVirtTableViewReq::operator=: Virtual objects do not match!"));
// 			}
// 		}
		
		m_uiLastUpdateTimestamp =  pD->m_uiLastUpdateTimestamp;

	return *this;
}

quint64 MIhmVirtTableViewReq::generateTimeStamp()
{
	QString	sUpdTimeStamp = QDateTime::currentDateTime().toString("ddhhmmsszzz");
	
	//osiguraj da su razliciti
	m_iTimeStampCounter++;
	if(m_iTimeStampCounter>10)
		m_iTimeStampCounter = 0;
	
	sUpdTimeStamp.append(QString("%1").arg(m_iTimeStampCounter));

	return sUpdTimeStamp.toULongLong();
}

void MIhmVirtTableViewReq::appendNewCommand(QString sData, enum_aff_table_command eCmd, int iMaxRows)
{
	m_uiLastUpdateTimestamp = generateTimeStamp();
	
	switch(eCmd)
	{
	case enuIHM_AFF_TABLE_CMD_RESET_ROWS:
		{
			clearCommandList();
		}
	    break;
	case enuIHM_AFF_TABLE_CMD_RESET_AND_APPEND_ROWS:
		{
			clearCommandList();
		}
	    //break; note that break is ommited for this case
	case enuIHM_AFF_TABLE_CMD_PREPEND_ROWS:
	case enuIHM_AFF_TABLE_CMD_APPEND_ROWS:
		{	
			MRowsCommand * pNewCommand = new MRowsCommand();
			
			//this is to parse row list
			if(readRowsFromBuffer(sData, &(pNewCommand->m_lstRows)))
			{
				pNewCommand->m_eCmd = eCmd;
				pNewCommand->m_uiCommandTimestamp = m_uiLastUpdateTimestamp;

				m_lstRowsCommands.append(pNewCommand);
				
				//To restrict the maximum size of the object we use approximation
				// that 1 command = 1 row
				// and that all the commands are of the same type (or prepend or append)
				if(iMaxRows>0)
					while(m_lstRowsCommands.count()>iMaxRows)
						delete m_lstRowsCommands.takeFirst();
			}
			else
			{
				TRACE_D(QString("MIhmVirtTableViewReq::appendNewCommand: No rows were read from the buffer:[%1]!").arg(sData))
			}
		}
		break;
	default:
	    break;
	}

}

void MIhmVirtTableViewReq::clearCommandList()
{
	while (!m_lstRowsCommands.isEmpty())
		delete m_lstRowsCommands.takeFirst();
}

bool MIhmVirtTableViewReq::readRowsFromBuffer(QString sXmlData, QList <MTableRow*> *plstNewRows)
{
	bool bRet = false;

	if(OpenXMLBuffer(sXmlData))			
	{
		bRet = readParsedRows(plstNewRows);
	}
		
	return bRet;
}

bool MIhmVirtTableViewReq::OpenXMLBuffer(QString sXmlData)
{
	bool bRet = false;
	QString errorStr;
	int errorLine;
	int errorColumn;

	bRet = m_xmlData.setContent(sXmlData, true, &errorStr, &errorLine, &errorColumn);

	if(bRet)
	{
		QDomElement root = m_xmlData.documentElement();
		if(root.tagName() != IHM_CFG_VAL_TABLEVIEW_ROOT_ELEMENT)
		{
			TRACE_W(QString( "MIhmVirtTableViewReq::OpenXML: Invalid root element!"));
			bRet = false;
		}
	}
	else
	{
		QString sMsg = QString("Parse error at line %1, column %2:\n%3").arg(errorLine).arg(errorColumn).arg(errorStr);
		TRACE_W(QString("MIhmVirtTableViewReq::OpenXML: %1").arg(sMsg.toLatin1().data()));
	}

	return bRet;
}

bool MIhmVirtTableViewReq::readParsedRows(QList <MTableRow*> *plstNewRows)
{
	bool bRet = false;

	QDomElement root = m_xmlData.documentElement();
	QDomElement elBody = root.firstChildElement(IHM_CFG_VAL_TABLEVIEW_BODY_ELEMENT);
	QDomNodeList lstRows = elBody.elementsByTagName(IHM_CFG_VAL_TABLEVIEW_ROW_ELEMENT);
	
	QDomNode currRow; 
	QDomNode currColumn; 
	QDomNodeList lstColumn;
	QString sToolTip, sTextAlign, sFontWeightBold,sTextStrikeOut,sStyle;
	QString sText, sIcon, sAction, sActionParam, sRowHeight;
	bool bBold, bStrikeOut;

	MTableRow * pNewRow;
	MTableCell * pNewCell;
	int iRowsCount = lstRows.count();

	for(int i=0;i<iRowsCount;i++)
	{
		currRow = lstRows.at(i);
		lstColumn = currRow.toElement().elementsByTagName(IHM_CFG_VAL_TABLEVIEW_COLUMN_ELEMENT);

		pNewRow = new MTableRow();
		plstNewRows->append(pNewRow);
		int iColCount = lstColumn.count();

		for(int j=0;j<iColCount;j++)
		{
			currColumn = lstColumn.at(j);
			sText = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_TEXT_ATTRIBUTE);
			sAction = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ACTION_ATTRIBUTE);
			sActionParam = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ACTION_PARAMS_ATTRIBUTE);
			sToolTip = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_TOOLTIP_ATTRIBUTE);

			sFontWeightBold = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_BOLD_ATTRIBUTE);
			bBold = sFontWeightBold.toInt() > 0;

			sTextAlign = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_ALIGN_ATTRIBUTE);
			int iAlignement = MHelpFuncs::convertAlignment(sTextAlign);
			
			sTextStrikeOut = MHelpFuncs::getAttributeText(&currColumn, IHM_CFG_VAL_TABLEVIEW_TEXT_STRIKEOUT_ATTRIBUTE);
			bStrikeOut = sTextStrikeOut.toInt() > 0;

			pNewCell = new MTableCell(sText, sAction, sActionParam, sToolTip, iAlignement, bBold, bStrikeOut);
			pNewRow->m_lstColumns.append(pNewCell);
		}
	}
	
	if(plstNewRows->count()>0)
		bRet = true;

	return bRet;
}


//---------------------------------------
//MIhmVirtHMenuViewReq
//---------------------------------------

MIhmVirtHMenuViewReq::MIhmVirtHMenuViewReq(MIhmVirtHMenuViewReq &obj)
{
	*this = obj;
	
}

MIhmVirtHMenuViewReq::MIhmVirtHMenuViewReq(enumVirtualObjectId eID)
{
	m_eID = eID;
	
	m_bUseExternalFile = false; 
	m_bDoNotReloadMenu = false; 
}

MIhmVirtHMenuViewReq::~MIhmVirtHMenuViewReq()
{
	;
}


MIhmVirtualObject& MIhmVirtHMenuViewReq::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtHMenuViewReq *pD = dynamic_cast<const MIhmVirtHMenuViewReq*>(&right);
	
	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();
	
	m_sMenuContent = MHelpFuncs::deepCopy(pD->m_sMenuContent);	   
	m_bUseExternalFile = pD->m_bUseExternalFile;
	m_bDoNotReloadMenu = pD->m_bDoNotReloadMenu;
	
	return *this;
}


bool MIhmVirtHMenuViewReq::setMenuContent(QString sMenuContent)
{	
	if(!(m_sMenuContent==sMenuContent))
	{
		m_sMenuContent = sMenuContent;
		return true;
	}
	else
		return false;
}

bool MIhmVirtHMenuViewReq::setUseExternalFile(bool b)
{  
	if(m_bUseExternalFile != b)
	{
		m_bUseExternalFile = b;
		return true;
	}
	else
		return false;
}

bool MIhmVirtHMenuViewReq::setDoNotReloadMenu(bool b)
{  
	m_bDoNotReloadMenu = b;

	return !m_bDoNotReloadMenu; //if menu needs to be reloaded signal that it needs update
}

//---------------------------------------
//MIhmVirtVideoViewReq
//---------------------------------------

MIhmVirtVideoViewReq::MIhmVirtVideoViewReq(MIhmVirtVideoViewReq &obj)
{
	*this = obj;
	
}

MIhmVirtVideoViewReq::MIhmVirtVideoViewReq(enumVirtualObjectId eID)
{
	m_eID = eID;
	m_nSource = 0;
	m_nZoom = 0;
	m_bVideoOn = false;
}

MIhmVirtVideoViewReq::~MIhmVirtVideoViewReq()
{
	;
}


MIhmVirtualObject& MIhmVirtVideoViewReq::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtVideoViewReq *pD = dynamic_cast<const MIhmVirtVideoViewReq*>(&right);
	
	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();
	
	m_nSource = pD->m_nSource;
	m_nZoom = pD->m_nZoom;
	m_bVideoOn = pD->m_bVideoOn;
	m_sStreamSourceUrlLocal = MHelpFuncs::deepCopy(pD->m_sStreamSourceUrlLocal);
	m_sStreamSourceUrlRemote = MHelpFuncs::deepCopy(pD->m_sStreamSourceUrlRemote);
	m_sBaseUrl = MHelpFuncs::deepCopy(pD->m_sBaseUrl);	
	
	return *this;
}
//---------------------------------------
//---------------------------------------
//---------------------------------------


//---------------------------------------
//MIhmVirtTabView
//---------------------------------------


MIhmVirtTabView::MIhmVirtTabView(enumVirtualObjectId eID)
		:MIhmVirtContainer(eID)
{
	m_iCurrentIndex = 0;
	m_slstTabText.clear();
}


MIhmVirtTabView::MIhmVirtTabView(MIhmVirtTabView &obj)
	:MIhmVirtContainer(obj.getId())
{
	*this = obj;

}


MIhmVirtualObject& MIhmVirtTabView::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtTabView *pD = dynamic_cast<const MIhmVirtTabView*>(&right);

	Q_ASSERT(pD!=NULL);

	m_bVisible = right.isVisible();
	m_bObjUpdated = right.isUpdated();
	m_eID = right.getId();

	m_iCurrentIndex = pD->m_iCurrentIndex;

	m_slstTabText.clear();

	const QStringList *lst = &(pD->m_slstTabText);

	for(int i=0; i<lst->size(); ++i )
	{
		m_slstTabText.append(MHelpFuncs::deepCopy(lst->at(i)));
	}

	return *this;
}

bool MIhmVirtTabView::setEnabled(bool b)
{	
	if(m_bEnabled!=b)
	{
		m_bEnabled = b;
		return true;
	}
	else
		return false;
}


//---------------------------------------
//---------------------------------------
//---------------------------------------

//---------------------------------------
//MIhmVirtAsyncInput
//---------------------------------------

MIhmVirtAsyncInput::MIhmVirtAsyncInput()
{
	m_eID = enuIhmVirtAsyncInputsID;
	m_pAsyncReq = new MInputAsyncExReq();
}

MIhmVirtAsyncInput::~MIhmVirtAsyncInput()
{
	if(m_pAsyncReq != NULL)
		delete m_pAsyncReq;
}


MIhmVirtAsyncInput::MIhmVirtAsyncInput(MIhmVirtAsyncInput &obj)
{
	m_pAsyncReq = new MInputAsyncExReq();
	*this = obj;
}


MIhmVirtualObject& MIhmVirtAsyncInput::operator=(const MIhmVirtualObject& right)
{
	const MIhmVirtAsyncInput *pD = dynamic_cast<const MIhmVirtAsyncInput*>(&right);

	m_bObjUpdated = right.isUpdated();
	
	*(this->m_pAsyncReq) = *(pD->m_pAsyncReq);
	
	return *this;
}

//on config lane type change do the reinitialize
bool MIhmVirtAsyncInput::initialize(QString sLaneType)
{
	bool bOK =  m_pAsyncReq->init(sLaneType);
	
	if(bOK)
		setUpdated();

	return bOK;
}

		
//on new SRV_TYP_SET message of the M_IHM_ASYNC_INPUT service
void MIhmVirtAsyncInput::update(QString sDescription, int is_file)
{
   m_pAsyncReq->setDescription(sDescription, is_file);		
}

//on user input (key or click) used at the ANI thread to update the state
// of the asyn input controls
void MIhmVirtAsyncInput::updateInputValues(QString sEncStrValues)
{
	m_pAsyncReq->updateInputValues(sEncStrValues);
}

QString MIhmVirtAsyncInput::getEncStrValues()
{
	return m_pAsyncReq->getEncStrValues();
}

//---------------------------------------
//---------------------------------------
