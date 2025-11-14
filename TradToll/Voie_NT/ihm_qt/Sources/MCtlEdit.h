

#ifndef MCTL_EDIT_H
#define MCTL_EDIT_H

#include <QWidget>

// ' --------------------------------------------------------------------
// ' DESCRIPTION : Control "champ de saisie avance". Permet l'utilisation
// '               de differents type de formats de saisie :
//                 Les types sont :
// 				----------------------------------------------------------------------------------
//                 Type "C" (chaine)    : Aucun parametre
// 				----------------------------------------------------------------------------------
//                 Type "M"/"R" (masque): Les caracteres du format decrivent le masque
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
// '               Si le format est non valide, le champ est automatiquement desactive.
// '               Lorsqu'il est valide, le champ est rempli avec une valeur
// '               par defaut calculee automatiquement verifiant les conditions imposees
// '               par le format.
// '               En lecture, recupere le format courant.
// ' --------------------------------------------------------------------
class QLabel;
class MLineEdit;

/*!
	\class MCtlEdit
	\brief The class creates and handles the controls used for a line input
			at the input dialogs of type MInputDialogReq::enuIHMSAISIE_SAISIE 
			and MInputDialogReq::enuIHMSAISIE_VISUSEUL

	
	
	\author Tihomir Cvitanic
	\version 1.0
	\date    2010/01/20
	\sa  
*/

class MCtlEdit
{
public:
	MCtlEdit(QWidget *parent);
	~MCtlEdit();

	void setColors(QString sCaptionBack,
					QString sEditFocusBack,
					QString sEditNoFocusBack,
					QString sLockedBack,
					QString sCaptionFore,
					QString sEditFore);

	/*! The function parses the part of definition string that is to be used for this 
		input line and creates and initialize the controls 
	*/
	bool setDefinition(QString sDef);
	
	/*! The function parses the part of description string that is to be used for this 
		input line and initializes its data
	*/
	bool setDescription(QString sDesc);	

	int getLineHeight(){return m_iLineHeight;};

	int getNumItems(){return m_iNumItems;};
	QWidget * getItemWidget(int i);
	QLabel * getLabelWidget(){return m_lblCaption;};

	//!If this one is set make it read only and use the QLabel to show all the text
	void setIsAlarm(bool bIsAlarm){m_bIsAlarm = bIsAlarm;};
	
	void setFont ( const QFont & fnt);
	void setFontSize(int iSize);
	
	int getMaxWidth();

	QString getItemText(int iItem);
	QString getResult();

	bool isInputValid();

	enum FormatType{
		enuFORMAT_TYPE_INVALID,
		enuFORMAT_TYPE_STRING,
		enuFORMAT_TYPE_MASK,
		enuFORMAT_TYPE_NUMBER,
		enuFORMAT_TYPE_RIGHTMASK,
		enuFORMAT_TYPE_FUNCTION
	};

	FormatType getFormatType() {return m_eFormatType;};
	void updateCalculation(QStringList * psLst, int iCol);


signals:
	void onEnter();
	void onEscape();

	
private:
	void updateFont();
	bool setValidValue(QString s, int i);
	bool createControls();
	bool setFormat(QString sFormat);

	QLabel * m_lblCaption;
	QWidget * m_parentWidget;
	QList <MLineEdit *> m_lstLineEdits;

	bool m_bIsAlarm;
	QString m_sLabelText;
	bool m_bLocked;
	
	QString m_sFormat;

	QString m_sFormula;
	QChar	m_cResultNumberFormat;
	int		m_iResultPrecision;	


	int m_iMinLen;
	int m_iMaxLen;
	int m_iNumItems;
	int m_iLineHeight;
	int m_iNumDecimals;
	QChar m_chDecimalSeparator;

	QString m_sUserInfo;

	QString m_sCaptionBack;
	QString m_sEditFocusBack;
	QString m_sEditNoFocusBack;
	QString m_sLockedBack;
	QString m_sCaptionFore;
	QString m_sEditFore;
	bool m_bClearFieldWhenKeyPressedForFirstTime;
	QString m_sDefaultValue;
	QFont m_font;
	bool m_bPasword;
	QString m_sMask;
	

	FormatType m_eFormatType;


};



#endif
