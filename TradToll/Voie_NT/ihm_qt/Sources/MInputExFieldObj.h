

#ifndef MINPUT_EX_FIELD_OBJ_H
#define MINPUT_EX_FIELD_OBJ_H

#include <QString>
#include <QObject>
#include <QPixmap>

#include "MInputExField.h"

class QCheckBox;
class QRadioButton;
class QComboBox;
class QListWidget;
class QLineEdit;
class QTextEdit;
class QLabel;
class QPushButton;
class MIhmListWidget;



class MInputExFieldObj: public QObject
{
    Q_OBJECT
public:
	
	MInputExFieldObj();
	virtual ~MInputExFieldObj();

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData) = 0;
	virtual QString getEncStrValue() = 0;
	virtual QString getXMLValue();

	virtual QWidget * getWidget() = 0;
	
	virtual void updateData(MInputExField* pData);

	virtual bool isValidValue() = 0;
	virtual bool retunsValue() = 0;
	virtual bool canGetFocus() = 0;

	bool isReadOnly(){return m_fieldData.isReadOnly();};

	QString getName(){ return m_fieldData.getName();};
	MInputExField::enuInputExFieldFormatType getFormatType(){return m_fieldData.getFormatType();};
	QString getFormatDetail(){return m_fieldData.getFormatDetail();};

	MInputExField::enuInputExFieldType getType()
						{return m_fieldData.getType();};

	static MInputExFieldObj * findFieldObject(QList <MInputExFieldObj*> *plst, QString sName);
	static MInputExFieldObj * createNewFieldObj(MInputExField * pFieldDef);
	
	MInputExField * getFieldData(){return &m_fieldData;};

signals:
	void dataChanged();
	void focusNext();

protected:
	virtual	void updateDataToWidget() = 0;
	MInputExField m_fieldData;
	
	bool m_bObjConnected;	
	bool m_bUpdateToWidgetInProgress;
};


class MFieldObjCheckBox: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjCheckBox():m_pCheckBox(NULL){};
	virtual ~MFieldObjCheckBox(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pCheckBox;};
	virtual bool canGetFocus();
	virtual bool isValidValue(){return true;};
	virtual bool retunsValue(){return true;};
	QString getCurrentValue();

private slots:
	void onDataChanged(int iState);

private:
	virtual	void updateDataToWidget();
	QCheckBox * m_pCheckBox;
};



class MFieldObjRadioButton: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjRadioButton():m_pRadio(NULL){};
	virtual ~MFieldObjRadioButton(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pRadio;};
	virtual bool canGetFocus();
	virtual bool isValidValue(){return true;};
	virtual bool retunsValue(){return true;};
	QString getCurrentValue();

private slots:
	void onDataChanged(bool bState);


private:
	virtual	void updateDataToWidget();
	
	QRadioButton * m_pRadio;
};

class MIhmComboBox;
class QFrame;

class MFieldObjComboBox: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjComboBox():m_pComboBox(NULL){};
	virtual ~MFieldObjComboBox(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pComboBox;};
	virtual bool canGetFocus();
	virtual bool isValidValue();
	virtual bool retunsValue(){return true;};
	QString getCurrentValue();

private slots:
	void onValueChanged(int);

private:
	virtual	void updateDataToWidget();
		
	MIhmComboBox* m_pComboBox;
	QFrame * m_pComboBoxFrame;

};



class MFieldObjListWidget: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjListWidget():m_pListWidget(NULL){};
	virtual ~MFieldObjListWidget(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pListWidget;};
	virtual bool canGetFocus();
	virtual bool isValidValue();
	virtual bool retunsValue(){return true;};

	enum enuFilterType
	{
		enuFILTER_ANYWHERE,
		enuFILTER_BEGINS_WITH
	};
	
	void setFilter(QString sFilter, enuFilterType = enuFILTER_ANYWHERE);
	QString getCurrentValue();

private slots:
	void onValueChanged(int iIndex);

private:
	virtual	void updateDataToWidget();
	
	MIhmListWidget* m_pListWidget;
	QFrame * m_pListWidgetFrame;

	enuFilterType m_eFilterType;
	QString m_sFilter;
};


class MFieldObjLineEdit: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjLineEdit():m_pLineEdit(NULL){};
	virtual ~MFieldObjLineEdit(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pLineEdit;};
	virtual bool canGetFocus();
	virtual bool isValidValue();
	virtual bool retunsValue(){return true;};
	QString getCurrentValue();
private slots:
	void onValueChanged(const QString &sNewValue);

private:	
	virtual	void updateDataToWidget();
	QLineEdit* m_pLineEdit;
	
};


// ' --------------------------------------------------------------------


class MLineEdit;
class QFrame;

// ' DESCRIPTION : Control "champ de saisie avance". Permet l'utilisation
// '               de differents type de formats de saisie :
//                 Les types sont :
// 				----------------------------------------------------------------------------------
//                 Type "C" (chaine)    : Aucun parametre
// 				----------------------------------------------------------------------------------
//                  Type "M/R" (masque): Les caracteres du format decrivent le masque
// 					The table below shows the characters that can be used in an input mask. 
// 					A space character, the default character for a blank, is needed for cases where a 
// 					character is permitted but not required.
// 								Character				Meaning  
// 					----------------------------------------------------------------------------------
// 									A			ASCII alphabetic character required. A-Z, a-z.
// 									a			ASCII alphabetic character permitted but not required.
// 									N			ASCII alphanumeric character required. A-Z, a-z, 0-9.
// 									n			ASCII alphanumeric character permitted but not required.
// 									X			Any character required.
// 									x			Any character permitted but not required.
// 									9			ASCII digit required. 0-9.
// 									0			ASCII digit permitted but not required.
// 									D			ASCII digit required. 1-9.
// 									d			ASCII digit permitted but not required (1-9).
// 									#			ASCII digit or plus/minus sign permitted but not required.
// 									H			Hexadecimal character required. A-F, a-f, 0-9.
// 									h			Hexadecimal character permitted but not required.
// 									B			Binary character required. 0-1.
// 									b			Binary character permitted but not required.
// 									>			All following alphabetic characters are uppercased.
// 									<			All following alphabetic characters are lowercased.
// 									!			Switch off case conversion.
// 									\			Use \ to escape the special characters listed above to use them as separators.
// 					----------------------------------------------------------------------------------
// 
// 								The mask consists of a string of mask characters and separators, 
// 								optionally followed by a semicolon and the character used for blanks. 
// 								The blank characters are always removed from the text after editing.
// 
// 								Examples:
// 
// 								Mask									Notes
// 					----------------------------------------------------------------------------------
// 								000.000.000.000;_					IP address; blanks are _.
// 								HH:HH:HH:HH:HH:HH;_					MAC address
// 								0000-00-00							ISO Date; blanks are space
// 								>AAAAA-AAAAA-AAAAA-AAAAA-AAAAA;#	License number; blanks are - and all (alphabetic) characters are converted to uppercase.
// 				----------------------------------------------------------------------------------
//				   Type "F" - (calculation) - 
//						[fp]^[function] 
// 						-f : Format to be used to show the result. If not set no formating is performed.
// 							Format				Meaning
// 								e			format as [-]9.9e[+|-]999
// 								E			format as [-]9.9E[+|-]999
// 								f			format as [-]9.9
// 								g			use e or f format, whichever is the most concise
// 								G			use E or f format, whichever is the most concise
// 						-p : a 0-9 digit - decimal number precision 
// 						-function : The function to be evaluated 					
// 	  						for the reference to other input boxes the % is used
// 							i.e. %1+%2	
// 				----------------------------------------------------------------------------------
// '               Type "N" (numerique) : Description du format numerique.
// '                     [ "+" ] [ e [ s [ d ] ] ]
// '                     - "+"   : La valeur saisie est signee
// '                     - e     : Nombre de caracteres constituant la partie entiere
// '                               Si commence par "0", la longueur en est forcee et le
// '                               vide est comble par des "0".
// '                     - s      : Separateur des decimal (peut etre tout caractere
// '                               non numerique
// '                     - d     : Nombre de caracteres constituant la partie decimale
// '                               Si commence par "0", la longueur en est forcee et le
// '                               vide est comble par des "0".
// 				----------------------------------------------------------------------------------
// 				----------------------------------------------------------------------------------
// '               Si le format est non valide, le champ est automatiquement desactive.
// '               Lorsqu'il est valide, le champ est rempli avec une valeur
// '               par defaut calculee automatiquement verifiant les conditions imposees
// '               par le format.
// '               En lecture, recupere le format courant.
// ' --------------------------------------------------------------------
// MFieldObjLineEditEx: Used for formated text input
class MFieldObjLineEditEx: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjLineEditEx();
	virtual ~MFieldObjLineEditEx();

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pMLineEdit;};
	virtual bool canGetFocus();
	virtual bool isValidValue();
	virtual bool retunsValue(){return true;};
	QString getCurrentValue();
	
	//this one would update the value only in case 
	//if format type is MInputExField::enuFIELD_FORMAT_CALCULATION
	void updateValue(QString sNewValue);
	
private slots:
	void onValueChanged(const QString &sNewValue);

private:	
	bool verifyRightMask(QString sRightMask);
	bool initLineEditFormat();
	virtual	void updateDataToWidget();
	MLineEdit * m_pMLineEdit;
	QFrame *	m_pContainerFrame;
	
};


//------------------------------------------------------

class MFieldObjTextEdit: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjTextEdit():m_pTextEdit(NULL){};
	virtual ~MFieldObjTextEdit(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pTextEdit;};
	virtual bool canGetFocus();
	virtual bool isValidValue();
	virtual bool retunsValue(){return !m_fieldData.isReadOnly();};
	QString getCurrentValue();
private slots:
	void onValueChanged();

private:	
	virtual	void updateDataToWidget();
	QTextEdit* m_pTextEdit;
};


class MFieldObjLabel: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjLabel():m_pLabel(NULL){};
	virtual ~MFieldObjLabel(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pLabel;};
	virtual bool canGetFocus(){return false;};
	virtual bool isValidValue(){return true;};
	virtual bool retunsValue(){return false;};

private:		
	virtual	void updateDataToWidget();
	QLabel* m_pLabel;
};



class MFieldObjLuhnCheckLabel: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjLuhnCheckLabel():m_pLabel(NULL){};
	virtual ~MFieldObjLuhnCheckLabel(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pLabel;};
	virtual bool canGetFocus(){return false;};
	virtual bool isValidValue(){return true;};
	virtual bool retunsValue(){return false;};

	void initLuhnKeyIcons(QString sImgLuhnOK, QString sImgLuhnNOK);
	void updateLuhnKeyCheckIcon(bool bOK);

private:		
	virtual	void updateDataToWidget();
	QLabel* m_pLabel;
	QPixmap m_PixmapLuhnOK;
	QPixmap m_PixmapLuhnNOK;
};





class MFieldObjButton: public MInputExFieldObj
{
    Q_OBJECT
public:
	MFieldObjButton():m_pButton(NULL){};
	virtual ~MFieldObjButton(){};

	virtual bool connectToWidget(QWidget *parentForm, MInputExField* pData);
	virtual QString getEncStrValue();
	virtual QWidget * getWidget(){return (QWidget *)m_pButton;};
	virtual bool canGetFocus();
	virtual bool isValidValue(){return true;};
	virtual bool retunsValue(){return false;};

	// Keyboard test specific start
	QString	getKeyID(){return m_fieldData.getKeyID();};
	QString	getKeyText(){return m_fieldData.getText();};
	static MFieldObjButton * findFieldObjectByKeyID(QList <MInputExFieldObj*> *plst, QString sKeyID);
	static MFieldObjButton * findNextFieldObjectByKeyID(MFieldObjButton *pCurrent, QList <MInputExFieldObj*> *plst, QString sKeyID);
	// Keyboard test specific end

private:		
	virtual	void updateDataToWidget();
	QPushButton* m_pButton;
};



#endif
