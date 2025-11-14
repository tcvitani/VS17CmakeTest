
#ifndef MFORM_ABOUT_H
#define MFORM_ABOUT_H

#include <QDialog>

class QLabel;
class QPushButton;
class QLineEdit;
class QFrame;
class QVBoxLayout;


class MFormAbout: public QDialog
{
    Q_OBJECT

public:
    MFormAbout(QWidget *parent = 0);
	~MFormAbout();

	bool initialize(QString sCustomButtonsCfgFile);
	
public slots:
	void onButtonExecute();
	void onButtonClose();
	void onCustomButtonClicked();

signals:
	void action(QString sAction, QString sParams);

private:
	bool loadTemplate(QString sUIFilePath);
	void keyPressEvent (QKeyEvent * event);
	void initCustomButtons();
	bool loadConfigFromRegistry(QString sRegKeyActions);
	QString getLastCommand();
	bool setLastCommand(QString sCmd);
	bool loadOptionsFromFile(QString sSourceFile);

	bool m_bTemplateLoaded;
	
	QWidget *m_dynFormWidget;

	QPushButton* m_btnClose;

	QFrame * m_frButtonsFrame;
	QFrame * m_frExecute;

	bool		m_bShowGeneric;
	QPushButton* m_btnExec;
	QLineEdit * m_txtExecuteLine;

	QVBoxLayout *m_Layout;

	struct CustomButton
	{
		QString m_sCaption;
		QString m_sActionId;
		QWidget * m_pWidget;
	};

	QString m_sInputTemplate;

	QList <CustomButton*> m_lstCustomButtons;
};

#endif

