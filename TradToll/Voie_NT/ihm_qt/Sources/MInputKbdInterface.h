
#ifndef MINPUT_KEYBOARD_INTERFACE_H
#define MINPUT_KEYBOARD_INTERFACE_H

class QKeyEvent;
class QObject;
class QEvent;

/*
	class: MInputKbdInterface 
	- Used as an iterface for all the inputs that should implement the behaviour:
	ENTER key : 
		Focus on text input : 
		Change focus to next control in the dialog (only text input with a single line is used)

		Focus on radio button (i don't talk about checkbox since we don't have)

		Change focus to next control (change of button is made by arrows) 
		Focus on drop-down listbox (not : when getting focus, the list shall automatically drop down) 
		Close the list keeping the current item selected and change focus to the next control

		Button 
		Trigger the associated action 
	ESC key : 

		close the dialog (cancel), when possible

	VALID key :

		Trigger the action associated with the default button 
	SPACE key : 
		No effect on the objects except in text input where the space character is added 
	TAB key : 
		Focus jumps to next focus-enabled object including buttons

*/
class MInputKbdInterface
{

public:
    MInputKbdInterface();
	~MInputKbdInterface();

	virtual void pressedKeyValidate() = 0;
	virtual void pressedKeyCancel() = 0;
	
protected:

	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event) = 0;
	virtual void focusFirstInput() = 0;
	virtual void moveFocusToNextInput() = 0;
	virtual void moveFocusToPreviousInput() = 0;
	virtual void initMouseCursorInitPos() = 0;
	void positionMousePointer();

	//to move the pointer over the default button
	int m_iInitMousePosX, m_iInitMousePosY;

};

#endif

