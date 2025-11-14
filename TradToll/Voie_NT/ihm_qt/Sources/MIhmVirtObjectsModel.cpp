
#include "MIhmVirtObjectsModel.h"
#include "MTracer.h"


 
MIhmVirtObjectsModel::MIhmVirtObjectsModel()
{
	int i;
	MIhmVirtualObject *pNewObj;

	m_vectVirtObjects.fill(NULL, (int)MIhmVirtualObject::enuIhmVirtLastObject);

	//Initialize all virtual objects 
	for(i=(int)MIhmVirtualObject::enuIhmVirtFirstObject;i<(int)MIhmVirtualObject::enuIhmVirtLastObject;i++)
	{
		pNewObj = NULL;

		if(i==(int)MIhmVirtualObject::enuIhmVirtAllObject)
		{
			//this one was used to be able to update the logo image
// 			pNewObj = new MIhmVirtIco((MIhmVirtualObject::enumVirtualObjectId)i);
// 			m_lstVirtObjects.append(pNewObj);

		}
		else if((i>=(int)MIhmVirtualObject::enuIhmVirtPlazaName) && (i<=(int)MIhmVirtualObject::enuIhmVirtCurrentDate))
		{
			pNewObj = new MIhmVirtText((MIhmVirtualObject::enumVirtualObjectId)i);

		}
		else if((i>=(int)MIhmVirtualObject::enuIhmVirtIcoLaneStatus) && (i<=(int)MIhmVirtualObject::enuIhmVirtIcoExtOpticalBarrier))
		{
			pNewObj = new MIhmVirtIco((MIhmVirtualObject::enumVirtualObjectId)i);
			
		}
		else if((i>=(int)MIhmVirtualObject::enuIhmVirtLstAlarms) && (i<=(int)MIhmVirtualObject::enuIhmVirtLstWarnings))
		{
			pNewObj = new MIhmVirtList((MIhmVirtualObject::enumVirtualObjectId)i);

		}
		else if ((i >= (int)MIhmVirtualObject::enuIhmVirtConfLaneType) && (i <= (int)MIhmVirtualObject::enuIhmVirtConfShiftType))
		{
			pNewObj = new MIhmVirtDynConfig((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtVideo)
		{
// 			pNewObj = new MIhmVirtContainer((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtImage)
		{
			pNewObj = new MIhmVirtIco((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtImage2)
		{
			pNewObj = new MIhmVirtIco((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtStrDetectConfigID )
		{
			pNewObj = new MIhmVirtStrDetectConfig();
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtAsyncInputsID )
		{
			pNewObj = new MIhmVirtAsyncInput();
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtInputDialogID )
		{
			pNewObj = new MIhmVirtInputDialogReq();
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtInputDlgExID )
		{
			pNewObj = new MIhmVirtInputDlgExReq();
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtTableView1||
				i==(int)MIhmVirtualObject::enuIhmVirtTableView2||
				i==(int)MIhmVirtualObject::enuIhmVirtTableView3)
		{
			pNewObj = new MIhmVirtTableViewReq((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtHMenuView1||
			i==(int)MIhmVirtualObject::enuIhmVirtHMenuView2)
		{
			pNewObj = new MIhmVirtHMenuViewReq((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtRestreamVideoView1||
				i==(int)MIhmVirtualObject::enuIhmVirtRestreamVideoView2)
		{
			pNewObj = new MIhmVirtVideoViewReq((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtCntHeaderGroup||
				i==(int)MIhmVirtualObject::enuIhmVirtCntInstructionGroup||
				i==(int)MIhmVirtualObject::enuIhmVirtCntModeGroup||
				i==(int)MIhmVirtualObject::enuIhmVirtCntPaymentGroup||
				i==(int)MIhmVirtualObject::enuIhmVirtCntTrsGroup||
				i==(int)MIhmVirtualObject::enuIhmVirtCntComments||
				i==(int)MIhmVirtualObject::enuIhmVirtCntCamera||
				i==(int)MIhmVirtualObject::enuIhmVirtCntScanner||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare1 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare2 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare3 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare4 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare5 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare6 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare7 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare8 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare9 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare10||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare11 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare12 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare13 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare14 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare15 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare16 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare17 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare18 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare19 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntSpare20 ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntMiniWeb ||
				i == (int)MIhmVirtualObject::enuIhmVirtCntAlarmsGroup)
		{
			pNewObj = new MIhmVirtContainer((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtBtnClass||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnEntryPoint||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnEntryPointNbr||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnMode||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnArrowOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnArrowOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCrossOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCrossOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnDsrcOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnDsrcOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnMagneticOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnMagneticOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCollectorOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCollectorOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCamera||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnScanner||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnComment||
				i==(int)MIhmVirtualObject::enuIhmVirtButton1||
				i==(int)MIhmVirtualObject::enuIhmVirtButton2||
				i==(int)MIhmVirtualObject::enuIhmVirtButton3||
				i==(int)MIhmVirtualObject::enuIhmVirtButton4||
				i==(int)MIhmVirtualObject::enuIhmVirtButton5||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnSpeedLimitOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnWarningOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnWarningOff||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOn||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnDsrcTSAOff||
				(i>=(int)MIhmVirtualObject::enuIhmVirtBtnSpare1 && i<=(int)MIhmVirtualObject::enuIhmVirtBtnSpare30)||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnGabarite||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnEntryGate||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnExitGate||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCar3||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCar2||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCar||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnEntryLoop||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnExitLoop||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnTblBeacon||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCamGrab||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCamHide||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnScanGrab||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnScanHide||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnScanSend||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnScanSave||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnCommHide||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnExternalDisplay||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnExternalDisplay2||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnTakeCtrl||
				i==(int)MIhmVirtualObject::enuIhmVirtBtnReturnCtrl)
		{
			pNewObj = new MIhmVirtButton((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else if(i==(int)MIhmVirtualObject::enuIhmVirtTabControl1||
				i==(int)MIhmVirtualObject::enuIhmVirtTabControl2||
				i==(int)MIhmVirtualObject::enuIhmVirtTabControl3||
				i==(int)MIhmVirtualObject::enuIhmVirtTabControl4)
		{
			pNewObj = new MIhmVirtTabView((MIhmVirtualObject::enumVirtualObjectId)i);
		}
		else
		{
			TRACE_W(QString("MIhmVirtObjectsModel::MIhmAniVirtObjects: ERROR creating object ID %1!")
									.arg(i));
		}


		m_vectVirtObjects.replace(i, pNewObj);
	}

}


MIhmVirtObjectsModel::~MIhmVirtObjectsModel()
{
	for(int i=0;i<m_vectVirtObjects.size();i++)
	{
		if(m_vectVirtObjects.at(i)!=NULL)
			delete m_vectVirtObjects.at(i);
	}

	m_vectVirtObjects.clear();

}


MIhmVirtualObject * MIhmVirtObjectsModel::getVirtualObjectByID(MIhmVirtualObject::enumVirtualObjectId eID)
{
	return m_vectVirtObjects.at((int)eID);
}


bool MIhmVirtObjectsModel::getIfObjectsChanged()
{
	for(int i=0;i<m_vectVirtObjects.size();i++)
	{
		if(m_vectVirtObjects.at(i)!=NULL)
			if(m_vectVirtObjects.at(i)->isUpdated())
			{
				return true;
			}
	}

	return false;
}

int MIhmVirtObjectsModel::getNumUpdatedObjects()
{
	int iCount = 0;

	for(int i=0;i<m_vectVirtObjects.size();i++)
	{
		if(m_vectVirtObjects.at(i)!=NULL)
			if(m_vectVirtObjects.at(i)->isUpdated())
			{
				iCount++;
			}
	}

	return iCount;
}


bool MIhmVirtObjectsModel::copyUpdatedVirtObjects(MIhmVirtObjectsModel * pOtherModel, bool bResetUpdated)
{
	MIhmVirtualObject * pMy;
	MIhmVirtualObject * pOther;
	bool bChanged = false;

	for(int i=0;i<m_vectVirtObjects.size();i++)
	{
		if(m_vectVirtObjects.at(i)!=NULL)
		if(m_vectVirtObjects.at(i)->isUpdated())
		{
			pMy = m_vectVirtObjects.at(i);
			pOther = pOtherModel->m_vectVirtObjects.at(i);
			
			Q_ASSERT(pMy);Q_ASSERT(pOther);
			Q_ASSERT(pMy->getId() == pOther->getId());

			*pOther = *pMy;
			
			pOther->setInitialized();

			if(bResetUpdated)
			{
				pMy->clearUpdated();							
			}

			bChanged = true;
			//TODO add code for verification
		}
	}

	return bChanged;
}


