#include <QtGui>
#include <QListwidget>
#include <QSettings>
#include "Ui_authorization.h"
#include "constants.h"
#include "Authorization.h"

AuthorizationDlg::AuthorizationDlg(QWidget *parent, int type, QString szMailbox)
	: IHMSimuDld(parent, DLG_AFF, szMailbox)
{
    ui->setupUi(this);

	m_nSubType = type;

}

void AuthorizationDlg::accept()
{
	m_pAction->setProperty(_PROP_TYPE, m_nSubType);
	m_pAction->setProperty(_PROP_USER, ui->lineEditUserName->text());
	m_pAction->setProperty(_PROP_PASSWORD, ui->lineEditSecCode->text());
	m_pAction->setProperty(_PROP_IS_CODE_PWD, ui->checkIsCodePwd->isChecked());
	m_pAction->setProperty(_PROP_SESSION, ui->lineEditSession->text());
	m_pAction->setProperty(_PROP_DESKTOP, ui->checkBoxIsDesktop->isChecked());
	m_pAction->setProperty(_PROP_VALUE, ui->comboBoxRetVal->currentIndex());
	m_pAction->activate(QAction::Trigger);
	

	QDialog::accept();
}

void AuthorizationDlg::reject()
{
	QDialog::reject();
}

void AuthorizationDlg::setData(struct_ihm_authorization data)
{
	ui->lineEditUserName->setText(QString(data.user_id));
	ui->lineEditSecCode->setText(QString(data.sec_code));
	ui->lineEditSession->setText(QString(data.session_id));
	ui->checkBoxIsDesktop->setCheckState((data.is_desktop != 0) ? Qt::Checked : Qt::Unchecked);
}
