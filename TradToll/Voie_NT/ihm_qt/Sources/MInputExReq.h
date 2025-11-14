

#ifndef MINPUT_EX_REQ_H
#define MINPUT_EX_REQ_H

#include <QString>
#include "MInputExField.h"

class QDomNode;


class MInputExReq
{
public:

//-----------------------------------------------------

	MInputExReq();
	MInputExReq(MInputExReq &obj);
	~MInputExReq();

	MInputExReq& operator=(const MInputExReq& right);

	bool setDescription(QString sDescription, int is_file);
	QString getDefinition(){return m_sDefinition;};
	QString getDescription(){return m_sDescription;};
	
	QList <MInputExField *> * getInputFields(){ return &m_lstInputFields;};
	bool isValid(){return m_bValid;};

protected:
	bool setDefinition(QString sDefinitionFile);

	MInputExField * createInputField(QString sType, 
							 QString sName, 
							 QString sFormat, 
							 QString sFormatDetail, 
							 QString sMinLen,
							 QString sMaxLen,
							 QString sReadOnly,
							 QString sMultiselection,
							 QString sValidator,
							 QString sBtnId);

	MInputExField * getInputField(QString sName, QString sType = QString(""));
	
	void updateInputField(QString sType, 
							 QString sName,
							QDomNode *pFiledNode);
	
	bool parseDescription();

	QString m_sDescription;
	QString m_sDefinition;
	QString m_sDskCSSFile;
	
	QList <MInputExField *> m_lstInputFields;

	bool m_bValid;
};



#endif
