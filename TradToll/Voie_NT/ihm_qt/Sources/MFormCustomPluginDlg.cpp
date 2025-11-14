
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QComboBox>
#include <QAction>
#include <QModelIndex>

#include <QtUiTools>

#include "MFormCustomPluginDlg.h"
#include "MIhmConfigGeneral.h"
#include "MHelpFuncs.h"
#include "MInputExFieldObj.h"
#include "MIhmComboBox.h"
#include "MIhmListWidget.h"
#include "MTracer.h"
#include "MIHMPluginINT.h"
#include "MIHMPluginDlgLogicINT.h"
#include "MIhmLoadedPlugins.h"




MFormCustomPluginDlg::MFormCustomPluginDlg(QWidget *parent)
	: MFormInputEx(parent), m_pCustomDlgLogic(NULL), m_pPlugin(NULL)
{
	setWindowFlags(Qt::Dialog
		| Qt::MSWindowsFixedSizeDialogHint
		| Qt::FramelessWindowHint);

	m_bTemplateLoaded = false;
	m_sResult = "";
}

MFormCustomPluginDlg::~MFormCustomPluginDlg()
{
	if (m_pCustomDlgLogic)
	{
		m_pPlugin->releasePluginDlgLogic(m_pCustomDlgLogic);
		m_pCustomDlgLogic = NULL;
	}
}


bool MFormCustomPluginDlg::initialize(MInputDialogExReq * pReq)
{
 	m_oReq = *pReq;
	
	//get pointer to started custom dialog plugins (by name)
	QString sPluginInstName = m_oReq.getCustomDlg_PluginName();
	m_pPlugin = MIhmLoadedPlugins::getLoadedPlugin(sPluginInstName);
	//get a new dialog logic object of the correct type(CustomDlg_DlgId)
	int iDlgType = m_oReq.getCustomDlg_DlgId();
	m_pCustomDlgLogic = m_pPlugin->getPluginDlgLogic(m_oReq.getCustomDlg_DlgId());

	if (m_pCustomDlgLogic)
	{
		connect(m_pCustomDlgLogic, SIGNAL(accepted()), this, SLOT(onDlgLogicAccepted()));
		connect(m_pCustomDlgLogic, SIGNAL(rejected()), this, SLOT(onDlgLogicRejected()));
		m_pCustomDlgLogic->initialize(m_pParentWindow, this, m_oReq.getCustomDlg_Parameters(), MIhmConfigGeneral::getCfg()->getLanguages()->getLanguage());

		m_bTemplateLoaded = true; //set the initialization result to be used at the base class functions  

		installEventFilter(this);
		installEventFilter(m_pParentWindow);
	}
	else
	{
		TRACE_W(QString("MFormCustomPluginDlg::initialize:Error getting MIHMPluginDlgLogicINT for:[%1]!").arg(sPluginInstName));
	}


	return m_bTemplateLoaded;
}




void MFormCustomPluginDlg::focusFirstInput()
{
	//NOP
}

void MFormCustomPluginDlg::initMouseCursorInitPos()
{
	//NOP
}

bool MFormCustomPluginDlg::keyPressEventHandler(QObject *obj, QKeyEvent * event)
{
	return MInputExFiledsKbdInterface::keyPressEventHandler(obj, event);
}


void MFormCustomPluginDlg::pressedKeyValidate()
{	
	m_pCustomDlgLogic->pressedKeyValidate();
}

void MFormCustomPluginDlg::pressedKeyCancel()
{
	m_pCustomDlgLogic->pressedKeyCancel();
}

void MFormCustomPluginDlg::show()
{
	MFormInputEx::show();

}

void MFormCustomPluginDlg::onDlgLogicAccepted()
{
	emit accepted();
}

void MFormCustomPluginDlg::onDlgLogicRejected()
{
	emit rejected();
}