#include <QtGui>
#include <QtUiTools>
#include <QCoreApplication>

#include "MFormSplash.h"
#include "MTracer.h"


MFormSplash::MFormSplash(QWidget *parent)
    : QWidget(parent)
{

	setWindowFlags(Qt::SplashScreen);//
	m_bTemplateLoaded = false;

	ui_lblInfo = NULL;
	m_dynFormWidget = NULL;

}


MFormSplash::~MFormSplash()
{

}

void MFormSplash::center()
{
	//make the window appear at the center
	QScreen *deskWidget = qApp->primaryScreen();
	QRect screenRect = deskWidget->availableGeometry();
	QRect rectTarget = m_dynFormWidget->geometry();

	int x = (screenRect.width() - rectTarget.width());
	x = x/2;

	int y = (screenRect.height() - rectTarget.height()); 
	y= y/2;

	x = (x>0)?x:0;
	y = (y>0)?y:0;

	this->move(x,y);
}



void MFormSplash::showSplash()
{
	//Show only if template correctly loaded
    if(m_bTemplateLoaded)
	{	
		this->show();
		this->center();
	}
}


bool MFormSplash::loadTemplate(QString sUIFilePath)
{
	bool bRetVal = false;
	QUiLoader loader;

	QFile file(sUIFilePath);
	if(file.open(QFile::ReadOnly))
	{
		m_layout = new QVBoxLayout(this);
		m_dynFormWidget = loader.load(&file, this);
		file.close();

		m_layout->addWidget(m_dynFormWidget);
		m_layout->setSpacing(0);

		this->setContentsMargins(0,0,0,0);
		m_layout->setContentsMargins(0,0,0,0);

		//find the widgets by name
		ui_lblInfo = this->findChild<QLabel*>("lblInfo");
	}
		
	if(ui_lblInfo != NULL && m_dynFormWidget!=NULL)
	{	
		TRACE_D(QString("MFormSplash::loadTemplate: %1 Successfull! applicationDirPath:[%2] applicationFilePath:[%3]")
									.arg(sUIFilePath)
									.arg(QCoreApplication::applicationDirPath())
									.arg(QCoreApplication::applicationFilePath()));

		m_bTemplateLoaded = true;
		bRetVal = true;
	}
	else
	{
		TRACE_W(QString("MFormSplash::loadTemplate: Error loading %1 or unable to find QLabel lblInfo !")
									.arg(sUIFilePath));
		
	}

	return bRetVal;
}


void MFormSplash::displayInfo(QString sInfo)
{    
    if(m_bTemplateLoaded)
		ui_lblInfo->setText(sInfo);
}
