#include <QtGui>
#include <QListwidget>
#include <QSettings>
#include "ui_waitUser.h"

#include "constants.h"
#include "MWaitUserDlg.h"

MWaitUserDlg::MWaitUserDlg(QWidget *parent)
	: QDialog(parent)
{
    ui->setupUi(this);
	connect(ui->btnOK, SIGNAL(clicked()), this, SLOT(accept()));
	connect(ui->btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}


