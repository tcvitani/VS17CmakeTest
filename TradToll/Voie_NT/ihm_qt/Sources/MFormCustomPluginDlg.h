
#ifndef MFORM_CUSTOM_PLUGIN_DLG_H
#define MFORM_CUSTOM_PLUGIN_DLG_H

#include <QDialog>
#include <QFileInfoList>
#include <QThread>
#include <QTextEdit>
#include "MFormInputEx.h"

class QPushButton;
class QLineEdit;
class QComboBox;
class QListWidgetItem;
class QLabel;
class QIcon;
class MIhmComboBox;
class MIhmListWidget;
class MIHMPluginDlgLogicINT;
class MIHMPluginINT;

class MFormCustomPluginDlg : public MFormInputEx
{
    Q_OBJECT

public:
	MFormCustomPluginDlg(QWidget *parent = 0);
	~MFormCustomPluginDlg();

	virtual bool initialize(MInputDialogExReq * pReq);
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();
	virtual void show();
	
private slots:
	void onDlgLogicAccepted();
	void onDlgLogicRejected();

protected:
	virtual	void initMouseCursorInitPos();
	virtual void focusFirstInput();
	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event);

	MIHMPluginDlgLogicINT *m_pCustomDlgLogic;
	MIHMPluginINT *m_pPlugin;
};

#endif

