
#ifndef MLINE_EDIT_H
#define MLINE_EDIT_H

#include <QLineEdit>


class MLineEdit: public QLineEdit
{
    Q_OBJECT

public:
    MLineEdit(QWidget *parent = 0);
	~MLineEdit();

	void setRightMask(QString sMask, QString sMaskPlaceholder);
	bool setNumberFormat(QString &sFormat);
	void setTxtInputMask(QString sMask);
	const QValidator * getValidator();

private slots:
	void onCursorPositionChanged(int iOld, int iNew);

protected:

	enum enuKeyAction{
		enuNoAction,
		enuIgnore,
		enuOverwriteR,
		enuOverwriteL,
		enuInsertR,
		enuInsertL,
		enuDeleteL,
		enuDeleteR,
		enuReplaceWithZeroL,
		enuReplaceWithZeroR,
		enuMoveCursorL,
		enuMoveCursorR,
		enuMoveSeparatorR,
		enuMoveSeparatorL,
		enuAppendZeroL
	};
		
	virtual void keyPressEvent (QKeyEvent * event);
	virtual void focusInEvent (QFocusEvent * event);
	bool m_bRightMask;

	void rightMaskKeyHandler(QKeyEvent * event);
	int  rightMaskGenerateKeyActions(enuKeyAction *eActionsToPerform, 
										int iCur, 
										QChar chCharEntered, 
										int iQtEventKey, 
										QString& s);

	
	
	void numberKeyHandler(QKeyEvent * event);
	
	void decimalKeyHandler(QKeyEvent * event);
	bool generateNumberValidator(QString sMaskFormat, QString& sReg);
	bool isSeparator(QChar ch);
	int getNumDigits(QString sDisplayedText);

	int  generateKeyActions(enuKeyAction *eActionsToPerform, 
									   int iCur, 
									   QChar chCharEntered, 
									   int iQtEventKey, 
									   QString& s);

	bool executeKeyActions(enuKeyAction *eAction, int iNumActions, QString sKey);
	bool removeLeadingZeroes(QString &s, int& iCur);
	
	//bool generateMaskValidator(QString sMaskFormat, QString& sReg);
	bool getRegExEscapeForChar(QChar chInMask, QString &sForRegExp);
	
	bool m_bSigned;
	bool m_bLeftPadZeroes;
	QChar m_chDecimalSeparator;
	QValidator * m_validator;
	
	int m_iNumDigits;
	int m_iNumDecimals;
	bool m_bNumber;
	bool m_bDecimal;
	QWidget *m_parentWidget;

	//right mask...
	bool m_bBlockRecursion;
	QString m_sRightMask;
	int m_iRightMaskSize;
	QChar m_cRightMaskPlaceholder;
	int m_iRightMaskSeparatorPosition;
	int m_iRightMaskCurNumDecimals;

};

#endif

