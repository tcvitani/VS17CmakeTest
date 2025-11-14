

#include <QKeyEvent>
#include <QPixmapCache>

#include "MIhmDskVisibleObject.h"
#include "MFormMain.h"
#include "MButtonLabel.h"


MButtonLabel::MButtonLabel(QWidget * parent):
	QPushButton(parent)
{
	m_parent = parent;
}

MButtonLabel::~MButtonLabel()
{
}

//to force the icon size to size of the button
void MButtonLabel::showEvent(QShowEvent * event)
{
	QSize sSize = this->size();
	setIconSize(sSize);
	QPushButton::showEvent(event);
}

//initialize signal connection to be able to send correct action for the objects
void MButtonLabel::initialize(MIhmButton *pVisObj, MFormMain *pMainForm)
{
	m_pVisObj = pVisObj;
	
	connect(this, SIGNAL(actionTriggered(QString,QString)), pMainForm, SLOT(onAction(QString,QString)));
	connect(this, SIGNAL(clicked(bool)), this, SLOT(onClicked(bool)));

}


void MButtonLabel::onClicked(bool bChecked)
{
	Q_UNUSED(bChecked)

	if(m_pVisObj!=NULL)
	{
		emit actionTriggered(m_pVisObj->getActionID(), "");
	}	
}


void MButtonLabel::updateButton(QString sTranslatedText, QString sImagePath)
{
	loadIcon(sImagePath);
	
	setText(sTranslatedText);
}

bool MButtonLabel::loadIcon(QString sPath)
{
	
	QPixmap oPixmap;
	QPixmap* pp;
	bool bPixmapOk = false;

	setIcon(QIcon());

	if (QPixmapCache::find(sPath, &oPixmap))
	{
		bPixmapOk = true;
	} 
	else 
	{
		if(oPixmap.load(sPath))
		{
			QPixmapCache::insert(sPath, oPixmap);
			bPixmapOk = true;
		}
	}

	if(bPixmapOk)
	{
		QIcon icon(oPixmap);
		icon.addPixmap(oPixmap, QIcon::Disabled, QIcon::On);
		icon.addPixmap(oPixmap, QIcon::Disabled, QIcon::Off);

		setIcon(icon);
		return true;
	}
	
	return false;
}



