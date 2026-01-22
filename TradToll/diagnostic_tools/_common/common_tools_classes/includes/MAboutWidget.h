/****************** (v) 2016 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 common_tools_classes											 */
/* FILE:     MAboutWidget.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef MAbout_WIDGET_H
#define MAbout_WIDGET_H

/*-------------------------------- INCLUDES:  -------------------------------*/
#include <QtWidgets/QWidget>

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/

/*-------------------------------- DEFINES:   -------------------------------*/

#if defined COMMON_TOOLS_CLASSES_EXPORT
	#define COMMON_TOOLS_CLASSES_DLLSPEC  Q_DECL_EXPORT
#else
	#define COMMON_TOOLS_CLASSES_DLLSPEC Q_DECL_IMPORT
#endif

/*-------------------------------- TYPEDEFS:  -------------------------------*/
namespace Ui {
	class MAboutWidgetB;
} // namespace Ui


class COMMON_TOOLS_CLASSES_DLLSPEC MAboutWidget:public QWidget
{
	Q_OBJECT

	public:
		MAboutWidget(QWidget *parent = 0);  
		virtual ~MAboutWidget();

		void setAppVersion(QString sVersion);
		void setLogoPix(QString sLogoPath);

		void retranslate();
	signals:
		void canceled();
	private slots:
		void onCloseBtnClicked();		
	protected:

		Ui::MAboutWidgetB		*m_ui;
		QString		m_sAppNameLabel;
		QString		m_sAppVersion;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/


/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/