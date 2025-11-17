
#ifndef MFORM_MAIN_H
#define MFORM_MAIN_H

#include <QMainWindow>
#include <QString>
#include <QList>
#include <QStringList>
#include <QVector>
#include <QHBoxLayout>

class QFrame;
class MKybTypeSettings;
class MKybKeyCfg;
class MKybButton;

class MKybFormMain: public QMainWindow
{
    Q_OBJECT

public:
    MKybFormMain(bool bShowTitle, QWidget *parent = 0);
	~MKybFormMain();

	bool initialize(MKybTypeSettings * pNewSettings);
	void showMainDlg();
	void center();
protected:
	void closeEvent(QCloseEvent * event );

signals:
	void stopRequested();


private:
	bool loadTemplate(QString sUIFilePath);
	bool connectVisibleObjects(QList <MKybKeyCfg*> * pLst);

	void checkResolution();
	
	void handleMouseClick(QObject *obj);

	QList <MKybButton *> m_lstCurrentButtons;
	
	QString m_sDynaFormStyleSheet;
	bool m_bTemplateLoaded;
	
	QWidget *m_dynFormWidget;

	QString	m_sCfgKyboardFileName;
	QFrame* m_frContainer;
};

#endif

