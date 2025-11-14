
#ifndef MINPUTEX_FIELDS_KEYBOARD_INTERFACE_H
#define MINPUTEX_FIELDS_KEYBOARD_INTERFACE_H

#include "MInputKbdInterface.h"

class QKeyEvent;
class QObject;
class QKeyEvent;
class MInputExFieldObj;

/*
	Class MInputExFiledsKbdInterface:
	Extends the MInputKbdInterface with functions commonly used for 
	manipulation with keyboard where a list of MInputExFieldObj is used
*/
class MInputExFiledsKbdInterface: public MInputKbdInterface
{

public:
    MInputExFiledsKbdInterface();
	~MInputExFiledsKbdInterface();

	virtual void pressedKeyValidate() = 0;
	virtual void pressedKeyCancel() = 0;
	
protected:
	MInputExFieldObj * findFieldObject(QObject *pWidget);
	MInputExFieldObj * findNextFieldObjectToFocus(MInputExFieldObj * pCurrentInFocus);
	MInputExFieldObj * findPreviousFieldObjectToFocus(MInputExFieldObj * pCurrentInFocus);
	MInputExFieldObj * findFieldObjectByName(QString sName);

	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event);
	virtual void focusFirstInput();
	
	virtual void moveFocusToNextInput();
	virtual void moveFocusToPreviousInput();
	virtual void initMouseCursorInitPos() = 0;
	virtual void setCorrectTabOrdering();
	
	QList <MInputExFieldObj *> m_lstFieldObjList;
	bool m_bInitialized;

};

#endif

