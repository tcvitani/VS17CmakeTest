
#ifndef MFORM_INPUT_GEN_H
#define MFORM_INPUT_GEN_H

#include <QDialog>
#include <MFormInputEx.h>

class QLabel;
class QPushButton;
class QVBoxLayout;
class MInputDialogReq;
class MIhmLanguages;
class MCtlList;
class MCtlEdit;
class QFrame;
class QGridLayout;
class QDialogButtonBox;
class MLineEdit;
class MInputExFieldObj;

class MFormInputGen: public MFormInputEx
{
    Q_OBJECT

public:
    MFormInputGen(QWidget *pMainWindow = 0);
	~MFormInputGen();

	virtual bool initialize(MInputDialogExReq * pReq);
	virtual void pressedKeyValidate();
	virtual void pressedKeyCancel();

private slots:
	void onButtonOK();
	void onButtonCancel();
	void onDataChanged();
	void onFocusNextReq();

protected:
	
	virtual void initMouseCursorInitPos();
	
	bool doValidate(bool bTriggeredByButton = true);
	bool doCancel();

	bool loadTemplate(QString sUIFilePath);
	bool connectDefinedObjects();
	bool initFieldFormatHandling();
	void initFieldFocusNextHandling();

	//calculator...
	void updateCalcualtedValues();
	QString calculateEvaluation(QString sFormula, QString sSkipObjectName, int iResultPrecision);

	//button flags
	bool m_bOkButton;
	bool m_bCancelButton;

	MInputExFieldObj * m_pFieldObjButtonOK; 

};

#endif

