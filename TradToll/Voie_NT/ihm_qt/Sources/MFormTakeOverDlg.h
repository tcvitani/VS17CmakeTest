
#ifndef MFORM_TAKE_OVER_DLG_H
#define MFORM_TAKE_OVER_DLG_H

#include "MFormClientDlg.h"

class QLabel;
class QPushButton;
class QFrame;

class MFormTakeOverDlg: public MFormClientDlg
{
    Q_OBJECT

public:
    MFormTakeOverDlg(QWidget *parent = 0);
	~MFormTakeOverDlg();

	enum enumDialogType{
		enuNone = 0,
		enuTakeOverQuestion,
		enuTakeOverMsg,
		enuClientInfoMsg,
		enuClientConnectingMsg
	};

	bool initialize(enumDialogType eType, 
					QString sInputTemplate, 
					QString sEmbedTo, 
					QString sCSSPath);

	void update(QString sTitle, QString sMsg);

	enumDialogType getDlgType(){return m_eDlgType;};
	
private:
	
	bool loadTemplate(QString sUIFilePath);

	
	QLabel* m_lblTitle;
	QLabel* m_lblMessageText;
	QFrame *m_frContainer;
	enumDialogType m_eDlgType;
};

#endif

