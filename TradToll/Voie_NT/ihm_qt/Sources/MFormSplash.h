
#ifndef MFORM_SPLASH_H
#define MFORM_SPLASH_H

#include <QWidget>

class QLabel;
class QVBoxLayout;
 
class MFormSplash: public QWidget
{
    Q_OBJECT

public:
    MFormSplash(QWidget *parent = 0);
	~MFormSplash();

	bool loadTemplate(QString sUIFilePath);
	void showSplash();

public slots:
	void displayInfo(QString sInfo);

private:
	void center();
	bool m_bTemplateLoaded;

    QLabel *ui_lblInfo;

	QWidget *m_dynFormWidget;
	QVBoxLayout *m_layout;


};

#endif

