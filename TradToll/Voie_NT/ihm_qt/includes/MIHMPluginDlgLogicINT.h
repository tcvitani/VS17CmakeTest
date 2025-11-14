#ifndef MIHM_PLUGIN_DLG_LOGIC_INT_H
#define MIHM_PLUGIN_DLG_LOGIC_INT_H

#include <QObject>


class MIHMPluginDlgLogicINT :public QObject
{
    Q_OBJECT
public:
	virtual ~MIHMPluginDlgLogicINT(){};

	virtual bool initialize(QWidget *pParentWindow, QWidget *pHostDlg, QString sParameters, QString sLanguage) = 0;
	virtual bool doValidate() = 0;
	virtual bool doCancel() = 0;

	virtual void pressedKeyValidate() = 0;
	virtual void pressedKeyCancel() = 0;

	virtual void forcedClose() = 0;
signals:
	void accepted();
	void rejected();

};


#endif	
/*---------------------------- END OF FILE -------------------------*/
