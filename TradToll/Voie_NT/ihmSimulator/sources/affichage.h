
#ifndef AFFICHAGEDLG_H
#define AFFICHAGEDLG_H

#include "IHMSimuDld.h"

class Ui_AffichageDlg;

class AffichageDlg : public IHMSimuDld
{
    Q_OBJECT

public:
	AffichageDlg(QWidget *parent, int iType, QString szMailbox, QString szLanguage = NULL);

	enum
	{
		AFF_ICON = 0,
		AFF_LABEL,
		AFF_IMAGE
	};
	
	void setLanguage(QString &szLanguage);
	
public slots:
	void accept();
	void reject();
	
private slots:

protected:
	virtual bool ReadXMLData();
	bool ReadRegValues();

	
private:
    Ui_AffichageDlg *ui;
	int m_iSubType;
	QString m_szLanguage;
	QString m_szImageRoot;
};

#endif

