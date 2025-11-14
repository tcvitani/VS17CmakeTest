

#ifndef MINPUT_EX_FIELD_H
#define MINPUT_EX_FIELD_H

#include <QString>
#include <QList>


#define INPUT_MASK_PLACEHOLDER_SEPARATOR ';'

class QDomNode;

class MOptionData
{
public:
	virtual MOptionData& operator=(const MOptionData& right);
	QString m_sText;
	QString	m_sValue;
};



class MInputExField
{
public:	
	MInputExField();
	MInputExField(MInputExField &obj);
	virtual MInputExField& operator=(const MInputExField& right);

	~MInputExField();

	enum enuInputExFieldFormatType
	{
		enuFIELD_FORMAT_UNKNOWN = 0,
		enuFIELD_FORMAT_FILTER,
		enuFIELD_FORMAT_LUHN_KEY,
		enuFIELD_FORMAT_LUHN_KEY_MOD16,
		enuFIELD_FORMAT_LUHN_KEY_MOD16_M3,
		enuFIELD_FORMAT_NUMBER,
		enuFIELD_FORMAT_TEXT,
		enuFIELD_FORMAT_MASK,
		enuFIELD_FORMAT_RIGHTMASK,
		enuFIELD_FORMAT_CALCULATION,
		enuFIELD_FORMAT_FILTER_BEGINS_WITH
	};

	enum enuInputExFieldType
	{
		enuDLG_EX_FIELD_UNKNOWN = 0,
		enuDLG_EX_FIELD_CHECK_BOX,
		enuDLG_EX_FIELD_RADIO_BUTTON,
		enuDLG_EX_FIELD_COMBO_BOX,
		enuDLG_EX_FIELD_LIST_WIDGET,
		enuDLG_EX_FIELD_LINE_EDIT,
		enuDLG_EX_FIELD_LINE_EDIT_EXT, //this one is used for formated input
		enuDLG_EX_FIELD_HIDDEN_EDIT,
		enuDLG_EX_FIELD_TEXT_EDIT,
		enuDLG_EX_FIELD_LABEL,
		enuDLG_EX_FIELD_LABEL_LUHN_CHECK,
		enuDLG_EX_FIELD_BUTTON_OK,
		enuDLG_EX_FIELD_BUTTON_CANCEL,
		enuDLG_EX_FIELD_BUTTON
	};

	enuInputExFieldType getType(){return m_eType;};
	QString getName(){return m_sName;};
	enuInputExFieldFormatType getFormatType(){return m_eFormat;};
	
	void initDef(QString sType, 
					QString sName, 
					QString sFormat, 
					QString sFormatDetail, 
					int		iMinLen, 
					int		iMaxLen, 
					bool bReadOnly,
					bool m_bMultiselection,
					QString sValidator,
					QString sKeyID);

	
	void initCalculationDetails(int iResultPrec, 
								bool bEnableLabelSwitching, 
								QString sLabelFalse, 
								QString sLabelTrue, 
								QString sLblSwitchFormula);
	
	static enuInputExFieldType getTypeForString(QString sType);
	static QString getStringForType(enuInputExFieldType eType);
	static enuInputExFieldFormatType getFormatEnumFromString(QString sFormat);
	
	bool setValueFromXmlNode(QDomNode *pFieldNode);
	virtual QString getXMLValue();

	void setValue(QString s){m_sValue = s;};
	QString getValue(){ return m_sValue;};

	bool isValueHtml(){return m_bIsValueHtml;}	

	//Note: that on some field types there are value and text property used
	// The value is used for all the objects,
	// but the text property is only used for fields of QCheckBox and QRadioButton type
	void setText(QString s){m_sText = s;};
	QString getText(){ return m_sText;};
	
	QString getKeyID(){ return m_sKeyID;}; //used for test keyboard

	//NOTE: that most of the objects are using QWidget::setEnabled 
	// to update the field m_bIsReadOnly property to object
	//(QCheckBox, QRadioButton, QLineEdit,QComboBox,QListWidget)
	//only the QTextEdit is using QTextEdit::setReadOnly because otherwise 
	//there would not be possible to scroll the content
	bool isReadOnly(){ return m_bIsReadOnly;};
	bool isMultiselection(){ return m_bMultiselection;};
	bool hasReturningValue();
	QString getSecondaryValidator(){ return m_sSecondaryValidator;};


	QString getFormatStr(){return m_sFormat;};
	QString getFormatDetail(){return m_sFormatDetail;};
	int getMinLen(){return m_iMinLen;};
	int getMaxLen(){return m_iMaxLen;};

	QList <MOptionData*> *getOptions(){return &m_lstOptions;};

	int getResultPrecision(){return m_iResultPrec;};
	bool isLblSwitchingEnabled(){return m_bEnableLabelSwitching;};
	QString getLbLSwitchingLabelFalse(){return m_sLabelFalse;};
	QString getLbLSwitchingLabelTrue(){return m_sLabelTrue;};
	QString getLbLSwitchingFormula(){return m_sLblSwitchFormula;};

	int getNumericPreDecimals(){return m_iNumericPreDecimals; };
	int getNumericDecimals(){return m_iNumericDecimals; };
	bool getNumericAllowNegative(){return m_bNumericAllowNegative; };
	QChar getNumericSeparatorChar(){return m_chNumericSeparatorChar;};
	bool getNumericLeftPadZeroes(){return m_bNumericLeftPadZeroes; };

	QString getInputMask(){return m_sInputMask;};
	QString getMaskPlaceholder(){return m_sMaskPlaceholder;};
private:
	bool parseNumberFormatStr(QString sNumInputFormat);
	bool loadOptionsFromFile(QString sSourceFile);

	enuInputExFieldType m_eType;

	QString m_sName;
	QString m_sValue;
	bool m_bIsValueHtml;
	QString m_sText;
	bool m_bIsReadOnly;
	QString m_sFormat;
	QString m_sFormatDetail;
	int m_iMinLen;
	int m_iMaxLen;
	bool m_bMultiselection;
	QString m_sKeyID; //used for keyboard test
	
	QString m_sSecondaryValidator;

	//Calculation Details 
	int m_iResultPrec; 
	bool m_bEnableLabelSwitching; 
	QString m_sLabelFalse; 
	QString m_sLabelTrue; 
	QString m_sLblSwitchFormula; //the formula used to evaluate which label needs to be shown (boolean expression)
	// formula set in m_sFormatDetail is used to show the result

	// numeric input properties
	int m_iNumericPreDecimals;
	int m_iNumericDecimals;
	bool m_bNumericAllowNegative;
	bool m_bNumericLeftPadZeroes;
	QChar m_chNumericSeparatorChar;

	//mask input properties
	QString	m_sInputMask;
	QString	m_sMaskPlaceholder;

	enuInputExFieldFormatType m_eFormat;

	//QString m_sXml;

	QList <MOptionData*> m_lstOptions;
		
};






#endif
