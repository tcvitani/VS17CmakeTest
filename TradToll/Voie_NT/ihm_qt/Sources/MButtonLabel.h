
#ifndef MBUTTON_LABEL_H
#define MBUTTON_LABEL_H

#include <QPushButton>

class MIhmButton;
class MFormMain;

class MButtonLabel:public QPushButton
{
	Q_OBJECT
public:
	MButtonLabel(QWidget * parent = 0);
	~MButtonLabel();
	
	void updateButton(QString sTranslatedText, QString sImagePath);
	void initialize(MIhmButton *pVisObj, MFormMain *pMainForm);
signals:
	void actionTriggered(QString, QString);

private slots:
	void onClicked(bool bChecked);

protected:
	void showEvent(QShowEvent * event);


private:
	bool loadIcon(QString sPath);

	QWidget *m_parent;
	
	MIhmButton *m_pVisObj;

};



#endif

