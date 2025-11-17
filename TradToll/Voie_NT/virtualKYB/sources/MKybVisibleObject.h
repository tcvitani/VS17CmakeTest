#ifndef MKyb_VISIBLE_OBJECT_H
#define MKyb_VISIBLE_OBJECT_H


#include <QObject.h>
#include <QString>
#include <MKybConfigGeneral.h>

extern "C" {
	#include "virt_kyb.h"
}

class QWidget;


class MKybButton:public QObject
{
    Q_OBJECT

	public:
		MKybButton(QString sName);
		~MKybButton();
				
		void initialize(QWidget * pWidget, MKybKeyCfg* pKeyCfg);
	public slots:
		void onTriggered();

	private:
		bool m_bInitialized;
		QString m_sName; 

		QWidget * m_pWidget;
		
		MKybKeyCfg m_KeyCfg;

};


#endif


