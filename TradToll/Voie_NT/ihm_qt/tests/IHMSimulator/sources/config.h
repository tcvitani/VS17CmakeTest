
#ifndef CONFIGDLG_H
#define CONFIGDLG_H

#include "Ui_config.h"
#include "IHMSimuDld.h"


class ConfigDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	ConfigDlg(QWidget *parent, int iType, QString szMailbox);

	enum
	{
		CONF_GET = 0,
		CONF_SET
	};

public slots:
	void accept();
	void reject();

private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_ConfigDlg ui;
	int m_iSubType;
};

#endif

