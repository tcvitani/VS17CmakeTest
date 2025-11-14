
#ifndef MDSK_ASYNC_INPUTS_H
#define MDSK_ASYNC_INPUTS_H

#include <QObject>
#include "MInputAsyncExReq.h"
#include "MInputExFiledsKbdInterface.h"

class MInputExFieldObj;
class QWidget;

class MDskAsyncInputs: public QObject, public MInputExFiledsKbdInterface
{
    Q_OBJECT

public:
    MDskAsyncInputs(QWidget * pParentForm);
	~MDskAsyncInputs();

	bool initialize(MInputAsyncExReq * pReq, QWidget *dynFormWidget);
	bool updateValues(MInputAsyncExReq * pReq);
	
	bool isInitialized()
			{return m_bInitialized;};

	QString getEncStrValues();
	
	//Important to write implementation to call the keyPressEventHandler when necessary
	bool eventFilter(QObject *obj, QEvent *event);
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();

signals:
	void dataChanged();

protected:
	virtual void initMouseCursorInitPos();


	bool createAndConnectDefinedObjects(MInputAsyncExReq * pReq);

	QWidget *m_dynFormWidget;

	QWidget * m_pParentForm;
};

#endif

