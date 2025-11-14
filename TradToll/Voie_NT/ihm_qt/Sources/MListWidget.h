
#ifndef MLIST_WIDGET_H
#define MLIST_WIDGET_H

#include <QListWidget>
class QLabel;

class MListWidget: public QListWidget
{
    Q_OBJECT

public:
    MListWidget(QWidget *parent = 0);
	~MListWidget();
	void initialize(QLabel *pToolTipLabel, bool bShowText, bool bShowIcons, bool bTooltipShowLastItem);
	void addItemsToListView(QStringList &lstToAdd);
	void installEventFilter(QObject * filterObj);
	
public slots:
	void clear();
private slots:
	void onItemEntered(QListWidgetItem * item);

protected:
	bool loadIconPixmap(QPixmap &oPixmap, QString sImgPath);
	void updateToolTip();
	virtual void leaveEvent(QEvent * event);
	void updateToolTipToLastItem();

	QLabel *m_pToolTipLabel;
	QObject * m_filterObj;
	bool m_bShowText;
	bool m_bShowIcons;
	bool m_bTooltipShowLastItem;
	QString m_sLastItem;
};

#endif

