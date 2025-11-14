
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGridLayout>
#include <QFrame>
#include <QKeyEvent>
#include <QComboBox>
#include <QAction>
#include <QModelIndex>
#include <QApplication>

#include <QtUiTools>

#include "MFormFileOpen.h"
#include "MIhmConfigGeneral.h"
#include "MHelpFuncs.h"
#include "MInputExFieldObj.h"
#include "MIhmComboBox.h"
#include "MIhmListWidget.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include <reg.h>
	#include "ihm.h"
	#include "ihm_loc.h"
}

#define _BUTTON_UP "btnUp"
#define _BUTTON_OK "btnOK"
#define _BUTTON_CANCEL "btnCancel"
#define _TXT_PATH "txtPath"
#define _TXT_FILE "txtFile"
#define _LIST_VIEW_FILES "listViewFiles"
#define _CB_DRIVE "cbDrive"
#define _CB_FILTER "cbFilter"
#define _LBL_TITLE "lblTitle"

#define IMAGE_ID_ICO_FILE		"ImgIcoFile"
#define IMAGE_ID_ICO_DIR		"ImgIcoDir"

#define IMAGE_ID_ICO_BUTTON_UP	"ImgButtonUp"

 

MFormFileOpen::MFormFileOpen(QWidget *parent, MInputDialogExReq::enuDialogExType eType)
    : MFormInputEx(parent)
{

	setWindowFlags(Qt::Dialog
						| Qt::MSWindowsFixedSizeDialogHint
						| Qt::FramelessWindowHint);

	m_bTemplateLoaded = false;
	m_sResult = "";
	m_bLoadingDriveList = false;
	m_bLoadingFolderContent = false;
	m_bReloadFolderContentAgain = false;
	m_bFormInitializing = true;

	m_txtPath = NULL;
	m_txtFile = NULL;
	m_lstFoldersAndFiles = NULL;
	m_cmbDriveList = NULL;
	m_cmbFileFilter = NULL;
	m_btnOK = NULL;
	m_btnCancel = NULL;
	m_btnUp = NULL;


	switch(eType)
	{
		case MInputDialogExReq::enuSAISIE_EX_SEL_FILE:
			m_bIsFile = true;
			break;
		case MInputDialogExReq::enuSAISIE_EX_SEL_FOLDER:
			m_bIsFile = false;
			break;
		default:
			break;
	}

	m_loadDriveListThread = new MDriveThread();
	QObject::connect(m_loadDriveListThread, SIGNAL(drivesLoaded(QStringList* )), this, SLOT(onDrivesLoaded(QStringList*)), Qt::QueuedConnection);
	
	m_loadFileListThread = new MFileThread();
	QObject::connect(m_loadFileListThread, SIGNAL(filesLoaded(QFileInfoList*, bool)), this, SLOT(onFolderContentListLoaded(QFileInfoList*, bool)), Qt::QueuedConnection);

	m_pxFile = NULL;
	m_pxDir = NULL;

		//load icons for directory and file in list
	QString sFileIconPath = MIhmConfigGeneral::getCfg()->getDskConfigImages()->getImageFullPath(IMAGE_ID_ICO_FILE);
	QString sDirIconPath = MIhmConfigGeneral::getCfg()->getDskConfigImages()->getImageFullPath(IMAGE_ID_ICO_DIR);

	m_pxFile = new QPixmap();

	if(sFileIconPath=="" || !m_pxFile->load(sFileIconPath))
	{
		TRACE_W(QString( "MFormFileOpen::MFormFileOpen: Unable to load file icon %1!").arg(sFileIconPath));
	}

	m_pxDir = new QPixmap();
	if(sDirIconPath==""|| !m_pxDir->load(sDirIconPath))
	{
		TRACE_W(QString( "MFormFileOpen::MFormFileOpen: Unable to load folder icon %s!").arg(sDirIconPath));
	}


}

MFormFileOpen::~MFormFileOpen()
{
	if(m_pxFile!=NULL)	
		delete m_pxFile;

	if(m_pxDir!=NULL)	
		delete m_pxDir;

	if(m_loadDriveListThread->isRunning())
	{
		m_loadDriveListThread->terminate();
		m_loadDriveListThread->wait();
	}

	delete m_loadDriveListThread;

	if(m_loadFileListThread->isRunning())
	{
		m_loadFileListThread->terminate();
		m_loadDriveListThread->wait();
	}

	delete m_loadFileListThread;

}


bool MFormFileOpen::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	TRACE_D(QString("MFormFileOpen::loadTemplate: sUIFilePath:%1").arg(sUIFilePath));
	
	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_dynFormWidget = loader.load(&file, this);
		file.close();

		m_sTemplateSize = m_dynFormWidget->size();

		m_frContainer = this->findChild<QFrame*>("frContainer");

		QPalette pal1 = m_dynFormWidget->palette();
		this->setAttribute(Qt::WA_WindowPropagation, false);
		this->setPalette(pal1);

		if(m_frContainer != NULL)
		{
			
			m_layout = new QVBoxLayout(this);
			this->setContentsMargins(0,0,0,0);
			m_layout->addWidget(m_frContainer);
			m_layout->setSpacing(0);
			m_layout->setContentsMargins(0,0,0,0);
			bRetVal = true;
		}

		m_bTemplateLoaded = true;
	}
	
	if(!bRetVal)
	{
		TRACE_W(QString( "MFormFileOpen::loadTemplate: Error loading %1").arg(sUIFilePath.toLatin1().data()));
	}

	return bRetVal;
}


bool MFormFileOpen::connectDefinedObjects()
{
	QString sTemp;

	while (!m_lstFieldObjList.isEmpty())
		delete m_lstFieldObjList.takeFirst();
	
	QList <MInputExField *> * plstFieldList = m_oReq.getInputFields();

	MInputExField * pCurrentField;

	for (int i=0; i<plstFieldList->size();i++)
	{
		pCurrentField = plstFieldList->at(i);
		
		MInputExFieldObj * pNewObj = MInputExFieldObj::createNewFieldObj(pCurrentField);
		

		if(pNewObj!=NULL)
		{
			if(pNewObj->connectToWidget(this, pCurrentField))
			{
				if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_OK)
				{
					QWidget * w = pNewObj->getWidget();
					
					if(w!=NULL)
					{
						m_btnOK = (QPushButton*)w;
						MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(pCurrentField->getText(), MIhmLanguages::enuTranslTargetDesktop, sTemp);
						m_btnOK->setText(sTemp);
						QObject::connect(m_btnOK, SIGNAL(clicked()), this, SLOT(onButtonOK()));
					}
				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON_CANCEL)
				{
					QWidget * w = pNewObj->getWidget();
					if(w!=NULL)
					{	
						m_btnCancel = (QPushButton*)w;
						MIhmConfigGeneral::getCfg()->getLanguages()->getLabelTranslation(pCurrentField->getText(), MIhmLanguages::enuTranslTargetDesktop, sTemp);
						m_btnCancel->setText(sTemp);
						QObject::connect(m_btnCancel, SIGNAL(clicked()), this, SLOT(onButtonCancel()));
					}
				}
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_BUTTON)
				{
					QWidget * w = pNewObj->getWidget();
					if(w!=NULL)
					{
						m_btnUp = (QPushButton*)w;
						QObject::connect(m_btnUp, SIGNAL(clicked()), this, SLOT(onUp()));
					}
				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_COMBO_BOX)
				{
					QWidget * w = pNewObj->getWidget();
					if(pNewObj->getName() == _CB_FILTER)
					{
						if(w!=NULL)
						{
							m_cmbFileFilter = (MIhmComboBox *)w;
							QList<MOptionData*> * listOpt = pCurrentField->getOptions();
						
							if(listOpt->size() == 0)
								m_cmbFileFilter->addItem("*.*");
							
							if(!m_bIsFile)
								w->hide();
							else
								w->show();

							QObject::connect(m_cmbFileFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(onChangeFilterBox(int)));
						}
					}
					else if(pNewObj->getName() == _CB_DRIVE)
					{
						m_sDrivesFilter = pCurrentField->getFormatDetail();
						m_slstDrivesToFilter = m_sDrivesFilter.split(",");
						
						TRACE_D(QString( "MFormFileOpen::connectDefinedObjects: Drives to filter (from format_detail property) [%1]...").arg(m_sDrivesFilter));

						if(w!=NULL)
						{
							m_cmbDriveList = (MIhmComboBox *)w;
							m_cmbDriveList->setAutoOpen(false);
							QObject::connect(m_cmbDriveList, SIGNAL(currentIndexChanged(int)), this, SLOT(onDriveChange(int)));
							QObject::connect(m_cmbDriveList, SIGNAL(gotFocus()), this, SLOT(onCmbDriveListGotFocus()));
						}
					}
				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_LINE_EDIT)
				{
					QWidget * w = pNewObj->getWidget();

					if(pNewObj->getName() == _TXT_PATH)
					{
						if(w!=NULL)
						{
							m_txtPath = (QLineEdit*)w;
						}
					}
					else if(pNewObj->getName() == _TXT_FILE)
					{
						if(w!=NULL)
						{
							m_txtFile = (QLineEdit*)w;
						}
					}

				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_LIST_WIDGET)
				{
					QWidget * w = pNewObj->getWidget();
					if(w!=NULL)
					{
						m_lstFoldersAndFiles = (MIhmListWidget *)w;
 						m_bMultiSel = pCurrentField->isMultiselection();
						m_lstFoldersAndFiles->setSelectionMode(m_bMultiSel ? QAbstractItemView::ExtendedSelection : QAbstractItemView::SingleSelection);
						QObject::connect(w,SIGNAL(itemDoubleClicked(QListWidgetItem *)), this, SLOT(onDoubleClickList(QListWidgetItem *)));
						QObject::connect(w,SIGNAL(itemSelectionChanged()), this, SLOT(onItemSelectionChanged()));
					}
				}	
				else if(pNewObj->getType()==MInputExField::enuDLG_EX_FIELD_LABEL)
				{
					QWidget * w = pNewObj->getWidget();
					if(w!=NULL)
					{
						((QLabel*)w)->setText(pCurrentField->getText());
					}
				}	

				m_lstFieldObjList.append(pNewObj);
				
				//install the main event filter
				if(pNewObj->getWidget() != NULL)
				{
					pNewObj->getWidget()->installEventFilter(this);
					pNewObj->getWidget()->installEventFilter(m_pParentWindow);
				}
				else
				{
					TRACE_W(QString( "MFormInputGen::connectDefinedObjects: can't get widget fpr object %1 returned false!")
									.arg(pCurrentField->getName()));
					return false;
				}
			}
			else
			{
				delete pNewObj;
				TRACE_W(QString("MFormInputGen::connectDefinedObjects: connectToWidget for object %1 returned false!")
									.arg(pCurrentField->getName()));

				return false;
			}

		}	
		else
		{
			TRACE_W(QString("MFormInputGen::connectDefinedObjects: Unknown object type for object %1 !")
									.arg(pCurrentField->getName()));

			return false;
		}
	
	}
	
	if(m_txtPath == NULL || m_txtFile == NULL || m_lstFoldersAndFiles == NULL || m_cmbDriveList == NULL 
		|| m_cmbFileFilter == NULL || m_btnOK == NULL || m_btnCancel == NULL || m_btnUp == NULL)
	{
		TRACE_W(QString("MFormInputGen::connectDefinedObjects: object not found!"));
		return false;
	}

	return true;
}



bool MFormFileOpen::setInitialPath(QString strDrive)
{
	QString sDrive;
	bool bFound= false;


	TRACE_D(QString("MFormFileOpen::setInitialPath..."));
	
	if(!strDrive.isEmpty())
	{
		sDrive = strDrive.left(1).toUpper();
		
		//set selected item by first letter in path
		int j = 0;
		while(j < m_cmbDriveList->count())
			if(m_cmbDriveList->itemText(j++).left(1).toUpper() == sDrive)
			{
				m_cmbDriveList->setCurrentIndex(j-1);
				bFound = true;
				break;
			}
	}


		if(!bFound)
		{
			TRACE_D(QString("MFormFileOpen::setInitialPath: [%1] drive not found selecting the first in the list!")
										.arg(strDrive));

			if(m_cmbDriveList->count()>0)
			{
				//assure it will reload folder list
				onDriveChange(0);
			}
			else
			{
				//clear list 
				m_lstFoldersAndFiles->clear();
				m_txtPath->setText("");
				TRACE_W(QString("MFormFileOpen::setInitialPath:No dirive found"));
			}

		}
		else 
		{
			QDir dir;
			bool bPathExists = dir.exists(strDrive);

			// verify that the path exists and if it does 
			if(bPathExists)
			{
				reloadFolderList();
			}
			else //otherwise 
			{
				// get the drive from the path and set path to root
				m_txtPath->setText(QString("%1:\\").arg(sDrive));

				reloadFolderList();
			}

		}


	return bFound;
}


bool MFormFileOpen::initialize(MInputDialogExReq * pReq)
{
 	QString sInputTemplate;

	m_oReq = *pReq;

	sInputTemplate = pReq->getDskUITemplate();

	if(sInputTemplate!="")
	{
		QString sTemplatePath = MIhmConfigGeneral::getCfg()->getUIFullPath(sInputTemplate);

		if(!loadTemplate(sTemplatePath))
		{
			TRACE_W(QString("MFormFileOpen::initialize: template not loaded!"));
			return false;
		}
	}
	else
	{
		TRACE_W(QString("MFormFileOpen::initialize: No UI template file defined for AboutDialogTemplate!"));
		return false;
	}

	//set additional style sheets
	QString sCSSFilePath = m_oReq.getDskCSSFile();
	if(sCSSFilePath!="")
	{
		sCSSFilePath = MIhmConfigGeneral::getCfg()->getCSSFullPath(sCSSFilePath);
		MHelpFuncs::setFileCSSToWidget(sCSSFilePath,this);
	}

	//connect defined objects on form
	if(connectDefinedObjects())
	{
		m_bInitialized = true;
	}

	return m_bInitialized;
}

void MFormFileOpen::onUp()
{
	//if directory enter in it
	if(!m_bLoadingFolderContent && !m_bLoadingDriveList)	
	{
		if(m_dfList.size()>0)
		{
			QFileInfo oClickedFileInfo = m_dfList.at(0);
			
			if(oClickedFileInfo.isDir())
			{
				QString sNewPath;
				
				if(oClickedFileInfo.fileName()=="..")
				{
					QDir dr = oClickedFileInfo.dir();

					if(!dr.isRoot() && dr.cdUp())
					{
						sNewPath = dr.absolutePath();				
					}
					else
					{
						sNewPath = dr.rootPath();	
					}

					sNewPath = QDir::cleanPath(sNewPath);
					m_txtPath->setText(sNewPath);
					reloadFolderList();
				}
			}
		}
	}

}

void MFormFileOpen::onCmbDriveListGotFocus()
{
	reloadDriveList();
}


void MFormFileOpen::onDriveChange(int index)
{
	if(!m_bLoadingDriveList)
	{
		QString sTemp;
		QString sTargetDrive;
		QString sCurentDrive;
		//change path to root of chosen drive
		sTemp = m_cmbDriveList->itemText(index);

		if(sTemp.size()>=2)
		{
			sTargetDrive = sTemp.left(2) + "/";
			sTargetDrive = QDir::cleanPath(sTargetDrive);
		}		
		
		sTemp = m_txtPath->text();
		
		if(sTemp.size()>=2)
		{
			sCurentDrive = sTemp.left(2) + "/";
			sCurentDrive = QDir::cleanPath(sCurentDrive.left(2));
		}

		//reload only if different than current
		if(!sTargetDrive.isEmpty())
			if(QString::compare(sCurentDrive,sTargetDrive,Qt::CaseInsensitive)!=0)
			{
				m_txtPath->setText(sTargetDrive);
				reloadFolderList();
			}
	}

}

void MFormFileOpen::onChangeFilterBox(int index)
{
	Q_UNUSED(index)
	//load list with new filter
	reloadFolderList();
}

void MFormFileOpen::onItemSelectionChanged()
{
	if(m_bIsFile)
	{
		//add item to file edit
		m_txtFile->setText("");
 		
		for(int i = 0; i < m_lstFoldersAndFiles->count(); i++)
		{
			if(m_lstFoldersAndFiles->item(i)->isSelected())
			{
				QString sFileName = m_lstFoldersAndFiles->item(i)->text();
				bool bIsDots = false;
				
				if(sFileName.size()>=2)
					bIsDots = (sFileName.left(2) == "..");

				if(bIsDots
					|| (m_bIsFile && m_dfList.at(i).isDir())
					|| (!m_bIsFile && !m_dfList.at(i).isDir()))
				{
					//getList()->item(i)->setSelected(false);
				}
				else
				{
					if(m_bMultiSel)
						m_txtFile->setText(m_txtFile->text() + m_lstFoldersAndFiles->item(i)->text() + ";");
					else
						m_txtFile->setText(m_lstFoldersAndFiles->item(i)->text());
				}
			}
		}

		if(m_bMultiSel && m_txtFile->text().count() > 0)
			m_txtFile->setText(m_txtFile->text().left(m_txtFile->text().count()-1));
	}
}


void MFormFileOpen::onDoubleClickList(QListWidgetItem * item)
{
	Q_UNUSED(item)

	onFileListItemExecuted();
}

void MFormFileOpen::onFileListItemExecuted()
{
	//if directory enter in it
	if(!m_bLoadingFolderContent)	
	{
		int i = -1;
		
		if(m_lstFoldersAndFiles->count()>0)
		{
			i = m_lstFoldersAndFiles->currentRow();
		
			if(m_dfList.count()>i && i>=0)
			{
				QFileInfo oClickedFileInfo = m_dfList.at(i);

				if(oClickedFileInfo.isDir())
				{
					QString sNewPath;
					
					if(oClickedFileInfo.fileName()=="..")
					{
						QDir dr = oClickedFileInfo.dir();

						if(!dr.isRoot() && dr.cdUp())
						{
							sNewPath = dr.absolutePath();				
						}
						else
						{
							sNewPath = dr.rootPath();	
						}
					
					}
					else
						sNewPath = oClickedFileInfo.absoluteFilePath();

					sNewPath = QDir::cleanPath(sNewPath);

					m_txtPath->setText(sNewPath);
					reloadFolderList();
				}
				else
				{
					;
					//do not use file double click as auto validate
// 					if(!m_bMultiSel)
// 					{
// 						if(m_bIsFile)
// 						{
// 							QString sName = oClickedFileInfo.fileName();
// 							m_txtFile->setText(sName);
// 							onButtonOK();
// 						}
// 
// 					}
				
				}
			}
		}
	}
}

void MFormFileOpen::focusFirstInput()
{
	//set focus to list
	if(m_lstFoldersAndFiles!=NULL)
		m_lstFoldersAndFiles->setFocus();
}

void MFormFileOpen::initMouseCursorInitPos()
{
	QPoint pt = m_btnOK->pos();

	m_iInitMousePosX = pt.x() + (m_btnOK->width()/2);
	m_iInitMousePosY = pt.y() + (m_btnOK->height()/2);
}

bool MFormFileOpen::keyPressEventHandler(QObject *obj, QKeyEvent * event)
{
	int iKey = event->key();

	if((iKey == Qt::Key_Enter || iKey == Qt::Key_Return) && m_lstFoldersAndFiles->hasFocus())
	{
		onFileListItemExecuted();
		return true;
	}

	return MInputExFiledsKbdInterface::keyPressEventHandler(obj, event);
}



void MFormFileOpen::pressedKeyValidate()
{
	onButtonOK();
}

void MFormFileOpen::pressedKeyCancel()
{
	onButtonCancel();
}

void MFormFileOpen::onButtonOK()
{
	if(doValidate())
		emit accepted();
}

void MFormFileOpen::onButtonCancel()
{
	if(doCancel())
		emit rejected();
}

bool MFormFileOpen::doValidate()
{
	bool bResult = true;

	if((!m_bLoadingFolderContent && !m_bLoadingDriveList) &&
			(!m_txtFile->text().isEmpty()) || 
			(!m_bIsFile && !m_txtPath->text().isEmpty()))
		{
			QString sRes = IHM_SAISIE_EX_SEPARATEUR_CHAMP;
			sRes += QString("%1~%2").arg(_TXT_PATH).arg(m_txtPath->text());
			sRes += IHM_SAISIE_EX_SEPARATEUR_CHAMP;
			sRes += QString("%1~%2").arg(_TXT_FILE).arg(m_txtFile->text().left(m_txtFile->text().count()));
			sRes += IHM_SAISIE_EX_SEPARATEUR_CHAMP;

			if(bResult)
				m_sResult = sRes;
		}
		else
			bResult = false;

	return bResult;
}

bool MFormFileOpen::doCancel()
{
	if((!m_bLoadingFolderContent && !m_bLoadingDriveList))
	{	m_sResult = "";
		return true;
	}
	else
	{
		return false;
	}
}


void MFormFileOpen::show()
{
	MFormInputEx::show();
	reloadDriveList();
}

void MFormFileOpen::reloadDriveList()
{
	TRACE_D(QString("MFormFileOpen::reloadDriveList..."));

	if(!m_bLoadingDriveList)
	{
		m_bLoadingDriveList = true;
		m_loadDriveListThread->start();
	}
}

void MFormFileOpen::reloadFolderList()
{
	TRACE_D(QString("MFormFileOpen::reloadFolderList..."));

	if(!m_bLoadingFolderContent)	
	{
		QString sCurrentPath = m_txtPath->text();
		QDir dir;

		m_bLoadingFolderContent = true;
		
		QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));

		if(!sCurrentPath.isEmpty() )
			if(!dir.exists(sCurrentPath) || isDriveFiltered(sCurrentPath))
			{
				sCurrentPath = "";
				m_txtPath->setText(sCurrentPath);
			}

		m_loadFileListThread->setListPathAndFilter(sCurrentPath, m_cmbFileFilter->currentText(), m_bIsFile);
		m_loadFileListThread->start();
	}
	else
	{
		//make it reload again, after the first loading is finished because something might be changed
		m_bReloadFolderContentAgain = true;
	}
}


void MDriveThread::run()
{
	TRACE_D(QString("MDriveThread::run... started!"));
	QFileInfoList oLst;
	oLst = QDir::drives();
	
	QString sVolumeType;QString sAbsolutePath;QString sDriveLetter;

	QStringList * pStrList = new QStringList();
	QFileInfo oDrive;

	for(int i = 0; i < oLst.count(); i++)
	{
		oDrive = oLst.at(i);

		sAbsolutePath = oDrive.absolutePath();
		UINT uRes = GetDriveType((LPCWSTR)sAbsolutePath. utf16());
		
		if(sAbsolutePath.size()>=2)
			sDriveLetter = sAbsolutePath.left(2).toUpper();
		else
			continue;

		sVolumeType = "";

		switch(uRes)
		{
		case DRIVE_UNKNOWN://The drive type cannot be determined. 
			break;
		case DRIVE_NO_ROOT_DIR:	//The root path is invalid. For example, no volume is mounted at the path. 
			break;
		case DRIVE_REMOVABLE://The disk can be removed from the drive. 
		    sVolumeType = "USB Drive";
			break;
		case DRIVE_FIXED:// The disk cannot be removed from the drive.
		    break;
		case DRIVE_REMOTE:// The drive is a remote (network) drive. 
		    sVolumeType = "Network Drive";
		    break;
		case DRIVE_CDROM:// The drive is a CD-ROM drive. 
		    sVolumeType = "CDROM";
		    break;
		case DRIVE_RAMDISK:// The drive is a RAM disk. 
		    sVolumeType = "RAM Drive";
		    break;
		default:
		    break;
		}
		 
		
		if(sVolumeType=="" || i==0) //skip A 
			pStrList->append(sDriveLetter);
		else
		{
			sDriveLetter.append(" (");
			sDriveLetter.append(sVolumeType);
			sDriveLetter.append(")");
			pStrList->append(sDriveLetter);
		}
	}

	emit drivesLoaded(pStrList);
}



bool MFormFileOpen::isDriveFiltered(QString sDrive)
{
	bool bRetVal = false;
	QString sCurrent;
	
	if(!sDrive.isEmpty())
	{
		for(int i=0; i< m_slstDrivesToFilter.count(); i++)
		{
			sCurrent = m_slstDrivesToFilter.at(i);

			if(!sCurrent.isEmpty())	
				if(sCurrent.at(0).toUpper() == sDrive.at(0).toUpper())
				{
					bRetVal = true;
					break;
				}
		}

	}

	return bRetVal;
}

void MFormFileOpen::onDrivesLoaded(QStringList* pStrList)
{
	TRACE_D(QString("MFormFileOpen::onDrivesLoaded:m_sDrivesFilter %1").arg(m_sDrivesFilter));

	if(pStrList!=NULL)
	{
		m_cmbDriveList->clear();
		
		while (!pStrList->isEmpty())
		{
			QString sNewDrive = pStrList->takeFirst();
			
			if(!isDriveFiltered(sNewDrive))
			{
				m_cmbDriveList->addItem(sNewDrive);
			}
			else
			{
				TRACE_D(QString("MFormFileOpen::onDrivesLoaded filtering drive %1").arg(sNewDrive));
			}
		}
		
		delete pStrList;

		m_bLoadingDriveList = false;

		setInitialPath(m_txtPath->text());
	}
	
}


void MFileThread::run()
{
	TRACE_D(QString("MFileThread::run... started!"));
	QDir dDir;
	QDir::Filters filter = QDir::Drives | QDir::AllDirs;
	QFileInfoList *pfList = new QFileInfoList();
	
	if(!m_sPath.isEmpty() && dDir.exists(m_sPath))
	{
		//find directory and load
		dDir.setPath(m_sPath);

		dDir.setSorting(QDir::DirsFirst | QDir::Name | QDir::IgnoreCase);
		if(m_bIsFile)
		{
			filter |= QDir::Files;

			int iIndexOfComment = m_sFilter.indexOf('(');
			QString sCurrFilter;
				
			if(m_sFilter.size()>iIndexOfComment && iIndexOfComment>0)
			{
				sCurrFilter	= m_sFilter.left(iIndexOfComment);
			}
			else
				sCurrFilter = m_sFilter;

			sCurrFilter = sCurrFilter.trimmed();
			
			TRACE_D(QString("MFileThread::run: filter [%1]!").arg(sCurrFilter));

			QStringList sLst; 
			if(sCurrFilter != "*.*" && sCurrFilter!="")
			{
				sLst.append(sCurrFilter);
				dDir.setNameFilters(sLst);
			}
		}

		dDir.setFilter(filter);
		
		// get the filtered list to the structure to be returned
		*pfList = dDir.entryInfoList();
		
		QFileInfo fInfo; // entry to loop with
		bool bRemovedDot = false;
		
		// remove "." and move ".." to the first position
		for (int i=0 ; i< pfList->count();i++)
		{
			fInfo = pfList->at(i);

			TRACE_D(QString("MFileThread::run: Adding file:[%1]").arg(fInfo.fileName()));

			if(fInfo.fileName()==".")
			{	
				pfList->removeAt(i);
				i--;
				if(bRemovedDot)
					break;
				bRemovedDot = true;
			}
			else if(fInfo.fileName()=="..")
			{
				if(dDir.isRoot())
				{
					pfList->removeAt(i);
					i--;
					if(bRemovedDot)
						break;
					bRemovedDot = true;
				}
				else //if not root, force .. to the first position in the list
				{	
					if(i!=0)
					{
						fInfo = pfList->takeAt(i);
						pfList->insert(0,fInfo);
					}

					if(bRemovedDot)
						break;
					bRemovedDot = true;
				}
			}
		}

		emit filesLoaded(pfList, dDir.isRoot());
	}
	else // if path empty
	{
		emit filesLoaded(pfList, TRUE);
	}


	TRACE_D(QString("MFileThread::run: filesLoaded emitted!"));
}



void MFormFileOpen::onFolderContentListLoaded(QFileInfoList* pdfList, bool bIsRoot)
{
	Q_UNUSED(bIsRoot)

	TRACE_D(QString("MFormFileOpen::onFolderContentListLoaded..."));
	m_dfList = *pdfList;
	delete pdfList;
	
	//clear list 
	m_lstFoldersAndFiles->clear();
	
	//read files and set items in list
	QFileInfo fInfo;
	QListWidgetItem *pNewItem;

	for (int i=0 ; i< m_dfList.count(); i++)
	{
		fInfo = m_dfList.at(i);

		if(fInfo.isDir())
		{
			pNewItem = new QListWidgetItem(fInfo.fileName(), m_lstFoldersAndFiles);
			
			if(!m_pxDir->isNull())
				pNewItem->setIcon(QIcon(*m_pxDir));
		}
		else
		{
			pNewItem = new QListWidgetItem(fInfo.fileName(), m_lstFoldersAndFiles);
			
			if(!m_pxFile->isNull())
				pNewItem->setIcon(QIcon(*m_pxFile));
		}

	}
	

	if(m_bIsFile) //fixed bug #53873 - for folder selection m_txtFile should stay unchanged
		m_txtFile->setText("");

	if(m_lstFoldersAndFiles->count()>0)
		m_lstFoldersAndFiles->setCurrentRow(0);

	m_bLoadingFolderContent = false;
	QApplication::restoreOverrideCursor();

	//reload again because something might be changed while the load was in progress
	if(m_bReloadFolderContentAgain)
	{
		m_bReloadFolderContentAgain = false;
		reloadFolderList();
	}
}
