
#ifndef MFORM_FILE_OPEN_H
#define MFORM_FILE_OPEN_H

#include <QDialog>
#include <QFileInfoList>
#include <QThread>
#include "MFormInputEx.h"

class QPushButton;
class QLineEdit;
class QComboBox;
class QListWidgetItem;
class QLabel;
class QIcon;
class MIhmComboBox;
class MIhmListWidget;

class MDriveThread : public QThread
{
    Q_OBJECT

public:
    void run();

signals:
	void drivesLoaded(QStringList*);

protected:

};

class MFileThread : public QThread
{
    Q_OBJECT

public:
	void setListPathAndFilter(QString sPath, QString sFilter, bool bIsFile)
		{ m_sPath = sPath; m_sFilter = sFilter; m_bIsFile = bIsFile; };
    void run();

signals:
	void filesLoaded(QFileInfoList*, bool);

protected:
	QString m_sPath, m_sFilter;
	bool m_bIsFile;

};

class MFormFileOpen: public MFormInputEx
{
    Q_OBJECT

public:
    MFormFileOpen(QWidget *parent = 0, MInputDialogExReq::enuDialogExType eType = MInputDialogExReq::enuSAISIE_EX_UNKNOWN);
	~MFormFileOpen();

	virtual bool initialize(MInputDialogExReq * pReq);
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();
	virtual void show();

public slots:
	void onButtonOK();
	void onButtonCancel();
	void onUp();
	void onDriveChange(int index);
	void onChangeFilterBox(int index);
	void onDoubleClickList(QListWidgetItem * item);
	void onItemSelectionChanged();

	void onDrivesLoaded(QStringList* pStrList);
	void onFolderContentListLoaded(QFileInfoList* pdfList, bool bIsRoot);

private slots:	
	void onCmbDriveListGotFocus();

protected:
	virtual	void initMouseCursorInitPos();
	virtual void focusFirstInput();
	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event);
	void onFileListItemExecuted();

	bool connectDefinedObjects();
	bool doValidate();
	bool doCancel();
	bool setInitialPath(QString strDrive);
	void reloadFolderList();
	void reloadDriveList();
	bool loadTemplate(QString sUIFilePath);

	bool isDriveFiltered(QString sDrive);


	//conf
	bool m_bIsFile;
	bool m_bMultiSel;

	QFileInfoList m_dfList;
	QFileInfoList m_dfDrives;

	QString m_sDrivesFilter;
	QStringList m_slstDrivesToFilter; 

	bool m_bFormInitializing;

	QPixmap * m_pxFile;
	QPixmap * m_pxDir;
	bool m_bLoadingDriveList;
	bool m_bLoadingFolderContent;
	bool m_bReloadFolderContentAgain;

	MDriveThread *m_loadDriveListThread;
	MFileThread *m_loadFileListThread;

	QLineEdit * m_txtPath;
	QLineEdit * m_txtFile;
	MIhmListWidget * m_lstFoldersAndFiles;
	MIhmComboBox * m_cmbDriveList;
	MIhmComboBox * m_cmbFileFilter;
	QPushButton * m_btnOK;
	QPushButton * m_btnCancel;
	QPushButton * m_btnUp;

};

#endif

