
#ifndef MFORM_CLIENT_DLG_H
#define MFORM_CLIENT_DLG_H

#include <QWidget>
#include "MInputKbdInterface.h"
class QPushButton;

class MFormClientDlg: public QWidget, public MInputKbdInterface
{
    Q_OBJECT

public:
    MFormClientDlg(QWidget *parent = 0);
	~MFormClientDlg();

	void show();

	//Important to write implementation to call the keyPressEventHandler when necessary
	bool eventFilter(QObject *obj, QEvent *event);
	
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();

signals:
	void accepted();
	void rejected();

protected slots:
	void onButtonOK();
	void onButtonCancel();
	
protected:
	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event);
	virtual void moveFocusToNextInput();
	virtual void moveFocusToPreviousInput();
	virtual void focusFirstInput();
	virtual void initMouseCursorInitPos();

	QWidget * findNextInputToFocus(QWidget * pCurrentInFocus);
	void installInputCtrlEventFilters();
	void setCorrectTabOrdering();

	QPushButton *m_btnOK;
	QPushButton *m_btnCancel;
	
	bool m_bTemplateLoaded;
	QWidget *m_dynFormWidget;

	QWidget *m_pParentWindow;

	QString m_sEmbedTo;

	QList <QWidget *> m_lstInputWidgets;

};

#endif

