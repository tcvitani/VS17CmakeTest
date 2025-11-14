#ifndef MIHM_WEB_COMMON_OBJ_LOGIC_H
#define MIHM_WEB_COMMON_OBJ_LOGIC_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QVector>
#include "MIhmWebVisibleObject.h"


class MIhmWebInterface;
class MIhmVirtualObject;
class MIhmVirtContainer;
class MIhmVirtIco;
class MIhmVirtText;
class MIhmVirtList;
class MIhmVirtDynConfig;
class MIhmVirtStrDetectConfig;
class MIhmVirtInputDialogReq;
class MIhmVirtInputDlgExReq;
class MIhmVirtTableViewReq;
class MIhmVirtVideoViewReq;
class MIhmVirtButton;
class MIhmVirtTabView;
class MIhmVirtAsyncInput;
class MIhmVirtHMenuViewReq;
class MInputDialogReq;
class MInputDialogExReq;
class MIhmMsg;
class MIhmMsgVideo;
class MIhmHttpRspRefresh;

class MIhmWebCommonObjLogic:public QObject
{
    Q_OBJECT
public:
	MIhmWebCommonObjLogic(MIhmWebInterface *parent);
	~MIhmWebCommonObjLogic();

	void processVirtualObject(MIhmVirtualObject* pVirtObj);
	QString getDynImageFullPath(QString sDynImageObjectName);
	void updateTheDateObject();
	
	//QVector <MIhmWebVisibleObject*> * getObjectsVector(){return &m_vectCommonWebVisibleObjects;};
	MIhmWebVisibleObject* getVisibleObject(MIhmWebVisibleObject::enumWebVisibleObjectId eId)
					{ return MIhmWebVisibleObject::findVisibleObjectByID(&m_vectCommonWebVisibleObjects, eId);};
private:
	MIhmWebInterface * m_pWebInterface;

	//Common web visible objects processing
	//-----------------------------------------------------------------------
	void initializeCommonVisibleObjects();
	QVector <MIhmWebVisibleObject*> m_vectCommonWebVisibleObjects;

	void processVirtContainer(MIhmVirtContainer* pVirtObj);
	void updateContainerView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtContainer* pVirtObj);

	void processVirtIcon(MIhmVirtIco*pVirtObj);
	MIhmWebVisibleImage * getVisibleImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId);
	void updateImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtIco* pVirtObj);
	MIhmWebVisibleDynImage * getVisibleDynImage(MIhmWebVisibleObject::enumWebVisibleObjectId eId);
	MIhmWebVisibleDynImage * getVisibleDynImageByName(QString sDynImageObjectName);
	void updateDynImage(QString sImagePath, bool bShow, MIhmWebVisibleObject::enumWebVisibleObjectId eId);

	void processVirtText(MIhmVirtText*pVirtObj);
	void updateTextLabel(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtText* pVirtObj);

	void processVirtButton(MIhmVirtButton* pVirtObj);
	void updateHtmlButton(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtButton* pVirtObj);
	
	void processVirtList(MIhmVirtList*pVirtObj);
	void updateListContainer(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtList* pVirtObj);
	
	void processVirtConfig(MIhmVirtDynConfig*pVirtObj);
	void processConfigPrjVersion(MIhmVirtDynConfig* pVirtObj);
	void processConfigPrjName(MIhmVirtDynConfig* pVirtObj);
	void processConfigLanguage(MIhmVirtDynConfig* pVirtObj);
	void processConfigLaneType(MIhmVirtDynConfig* pVirtObj);
	void processConfigShiftType(MIhmVirtDynConfig* pVirtObj);

	void processVirtStrDetConfig(MIhmVirtStrDetectConfig*pVirtObj);

	//Table file view
	void processFileViewReq(MIhmVirtTableViewReq* pVirtObj);
	void updateFileViewContainer(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtTableViewReq* pVirtObj);

	void processAsyncInput(MIhmVirtAsyncInput* pVirtObj);
	
	void processHMenuViewReq(MIhmVirtHMenuViewReq* pVirtObj);
	void updateHMenuView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtHMenuViewReq* pVirtObj);

	void updateTabView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, QStringList& lstTabText, int iCurrentIndex ,bool bVisible);
	void processTabViewReq(MIhmVirtTabView* pVirtObj);

	MIhmWebRVideoView * getRVideoObject(MIhmWebVisibleObject::enumWebVisibleObjectId eId);
	void processVideoViewReq(MIhmVirtVideoViewReq* pVirtObj);
	void updateRVideoView(MIhmWebVisibleObject::enumWebVisibleObjectId eId, MIhmVirtVideoViewReq* pVirtObj);
};

#endif


