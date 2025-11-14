
#ifndef MFORM_TEST_KEYBOARD_H
#define MFORM_TEST_KEYBOARD_H

#include <QDialog>
#include <QFileInfoList>
#include <QThread>
#include <QTextEdit>
#include "MFormInputEx.h"
#include <QRegularExpression>

class QPushButton;
class QLineEdit;
class QComboBox;
class QListWidgetItem;
class QLabel;
class QIcon;
class MIhmComboBox;
class MIhmListWidget;




class MFormTestKeyboard: public MFormInputEx
{
    Q_OBJECT

public:
    MFormTestKeyboard(QWidget *parent = 0);
	~MFormTestKeyboard();

	virtual bool initialize(MInputDialogExReq * pReq);
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();
	virtual void show();

	void onKeyDetected(int iKeyID, int iAsciiCode);
	void onTollKeyDetected(int iKeyID, QString sDetectedString);
	void onStringDetected(QString sCoupleID, QString sDetectedString);

public slots:
	void onButtonCancel();

protected:
	virtual	void initMouseCursorInitPos();
	virtual void focusFirstInput();
	virtual bool keyPressEventHandler(QObject *obj, QKeyEvent * event);


	bool connectDefinedObjects();
	bool doValidate();
	bool doCancel();
	bool loadTemplate(QString sUIFilePath);

	void displayPressedKey(int iKeyType, int iKeyID, QString sKeyCode);
	void updateMsgList(QString sListItemText);

	QString getKbdSpecialKeyTypeLabel();
	QString getKbdKeyTypeLabel();
	QString getKeyboardTestTitleLabel();
	QString getKeyTextTranslation(QString sConfLabelName);

	//button flags
	bool m_bOkButton;
	bool m_bCancelButton;
	bool m_bButton;

	MIhmListWidget * m_pListWidget;
	
	void maskDetectedString(const QString &sInput, QString &sOutput);
	bool	m_bFilterDisplayedStr;
	QRegularExpression m_rxMaskDetectedString;
	QChar	m_chReplacementChar;

};

#endif

