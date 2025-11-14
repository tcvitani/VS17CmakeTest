

#ifndef MCTL_LIST_H
#define MCTL_LIST_H


#include <QStringList>
#include <QWidget>
class QLabel;
class QLineEdit;
class QListWidget;


class MCtlList
{
public:
	MCtlList(QWidget *parent);
	~MCtlList();

	void setCaption(QString sCaption);
	bool addListItemDesc(QString sDesc);	


	int getNumItems(){return m_iNumItems;};
	QLabel * getLabelWidget(){return m_lblCaption;};
	QListWidget * getListWidget(){return m_listWidget;};

	void setFont ( const QFont & fnt);
	void setFontSize(int iSize);
	int getWidth();
	int getHeight();

	void setColors(QString sCaptionBack,
						QString sCaptionFore,
						QString sEditFore,
						QString sEditBack);
	bool getSelection(QString &sText, QString &sData);
signals:
	void onEnter();
	void onEscape();
private:
	void updateFont();

	QLabel * m_lblCaption;
	QWidget * m_parentWidget;
	QListWidget * m_listWidget;
	QStringList m_lstUserInfo;

	QString m_sLabelText;
	bool m_bLocked;
	
	int m_iNumItems;
	
	QString m_sDefaultValue;
	QFont m_font;

	QString m_sCaptionBack;
	QString m_sCaptionFore;
	QString m_sEditFore;
	QString m_sEditBack;


};



#endif
