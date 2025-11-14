
#ifndef MFORM_INPUT_EX_H
#define MFORM_INPUT_EX_H

#include <QWidget>
#include <MInputDialogExReq.h>
#include "MInputExFiledsKbdInterface.h"

class QVBoxLayout;
class MInputDialogReq;
class QFrame;
class QGridLayout;



class MFormInputEx: public QWidget, public MInputExFiledsKbdInterface
{
    Q_OBJECT

public:
    MFormInputEx(QWidget *pMainWindow = 0);
	~MFormInputEx();

	virtual void show();
	virtual bool initialize(MInputDialogExReq * pReq) = 0;
	QString getResult(){return m_sResult;};

	//Important to implement
	bool eventFilter(QObject *obj, QEvent *event);
	
signals:
	void accepted();
	void rejected();

protected:
	virtual void closeEvent (QCloseEvent * event);
	
	QString m_sResult;	
	bool m_bTemplateLoaded;

	QWidget *m_dynFormWidget;
	QVBoxLayout *m_layout;
	QFrame *m_frContainer;	
	
	MInputDialogExReq m_oReq;
	QSize m_sTemplateSize;

	QWidget *m_pParentWindow;
};

#endif

