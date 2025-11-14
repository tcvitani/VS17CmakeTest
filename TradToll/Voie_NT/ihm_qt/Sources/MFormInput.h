
#ifndef MFORM_INPUT_H
#define MFORM_INPUT_H

#include <QDialog>
#include <MInputDialogReq.h>

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


class MFormInput: public QWidget
{
    Q_OBJECT

public:
    MFormInput(QWidget *pMainWindow = 0);
	~MFormInput();

	void show();
	bool initialize(MInputDialogReq * pReq);
	QString getResult(){return m_sResult;};
	
private slots:
	void onButtonOK();
	void onButtonCancel();
	void onLineEditTextChanged(const QString &);

signals:
	void accepted();
	void rejected();

private:
	bool doValidate();
	bool doCancel();

	void updateControlsReadOnlyState(bool m_bReadOnly);
	void keyPressEvent (QKeyEvent * event); 	
	bool initFormAlignment(Qt::Alignment e);
	void initFormColors(QString sColAttributes);
	void arrangeControls();
	void arrangeButtons();
	void moveFocus(bool bUp);
	void focusFirstInput();

	void updateFormColors();
	bool loadTemplate(QString sUIFilePath);

	//calculations
	void installCalculationHandler();
	bool getItemPos(MLineEdit * pEdit, int *piRow, int *piCol);
	void updateCalculationsInCol(int iCol);

	bool m_bInitialized;
	bool m_bTemplateLoaded;
	QString m_sResult;	


    QLabel *ui_lblTitle;
	QDialogButtonBox *ui_buttonBox;
	QPushButton *ui_buttonOK;
	QPushButton *ui_buttonCancel;
	QWidget * ui_FormInput;

	QFrame * ui_pfrInputContainer;	
	QGridLayout *m_gridLayout; //used to arrange input controls inside input container

	QFrame * ui_pfrBody;	
	QFrame * ui_pfrTitle;	

	QWidget *m_dynFormWidget;
	QVBoxLayout *m_layout;
	
	//used to correctly resize the dialog ...
	int m_iInitialWidth;
	int m_iInitialHeight;
	int m_iInitialInputHeight;
	
	MCtlList * m_pCtlList;
	QList <MCtlEdit *> m_lstEditCtl;
	int m_iNumFields;
	
	MInputDialogReq m_oReq;

	//button flags
	bool m_bOkButton;
	bool m_bCancelButton;

	//string detection flags
	bool m_bProtected;
	bool m_bExternalKeyCatch;
	bool m_bAllExternalDevices;

	//dialog colors
	QString m_sCol1;
	QString m_sCol2;
	QString m_sCol3;
	QString m_sCol4;

	QWidget *m_pParentWindow;

};

#endif

