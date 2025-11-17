#include <QtGui>
#include <QListwidget>
#include <QSettings>

#include "constants.h"
#include "AsyncInputDlg.h"

AsyncInputDlg::AsyncInputDlg(QWidget *parent, QString sIHMMailbox)
	: IHMSimuDld(parent, DLG_AFF, sIHMMailbox)
{
    ui.setupUi(this);

}

void AsyncInputDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, _PROP_VAL_SET);
	m_pAction->setProperty(_PROP_IS_FILE, (ui.checkBoxIsFile->isChecked())? 1 : 0);
	m_pAction->setProperty(_PROP_VALUE, ui.textEditData->toPlainText());
	
	m_pAction->activate(QAction::Trigger);
	
	QDialog::accept();
}

void AsyncInputDlg::reject()
{
	QDialog::reject();
}
