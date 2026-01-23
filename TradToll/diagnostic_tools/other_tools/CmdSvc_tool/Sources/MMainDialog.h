/****************** (v) 2017 EMOVIS - All rights reserved ********************/
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 CMDSVC_tool												 */
/* FILE:     MMainDialog.h													 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY:                                                                  */
/*****************************************************************************/

#ifndef CMDSVC_MAIN_DIALOG_H
#define CMDSVC_MAIN_DIALOG_H

/*-------------------------------- INCLUDES:  -------------------------------*/
#include "ui_MMainDialogB.h"
#include "MDefines.h"
#include "MGlobalStructs.h"

extern "C"
{

};

/*-------------------------------- RESERVED:  -------------------------------*/

/*-------------------------------- EXTERNALS: -------------------------------*/
class QHBoxLayout;
class MStyledTitleBar;
class MStyledToolBox;
class MStyledLangPick;
class MAboutWidget;
class MCmdACOM;

/*-------------------------------- DEFINES:   -------------------------------*/

/*-------------------------------- TYPEDEFS:  -------------------------------*/

class MMainDialog: public QDialog
{
    Q_OBJECT

	public:
		MMainDialog(QWidget *parent = 0);
		~MMainDialog();

		void SetupGui();
		
	public slots:
		//default tool slots
		void onDialogResized();
		void onLangSelectionChanged(QString);
		void onBtnAbout();
		void onClose();

		//CMS svc...
		void onBtnConnect_clicked();
		void onBtnDisonnect_clicked();
		void onBtnRefresh_clicked();
		void onBtnBrowse_clicked();
		void onBtnSendCmd_clicked();
		void onBtnClear_clicked();
		void onchkRedirect_stateChanged(int);
		void onCurrentCmdIndexChanged(int);
		void onMachineName_editingFinished();
		void onMachineName_textChanged(const QString &);
		void onCommand_returnPressed();

		void onMessage(const QString &value);

		void onConnected();
		void onDisconnected();
		void onACOMMessageReceived(QByteArray);
		void onACOMMessageSent(QByteArray);

	protected:
		void changeEvent(QEvent* event);
		void retranslate();

	private:
		void updateAboutDlgSizeAndPos();
		void createLangPick();
		void createAboutDlg();
		bool refreshServiceList();

		QString MessageToHex(QString strMessage);

		bool raiseMsgBox(QWidget *pParent,
			QString sMessage,
			QString sBtnOkText,
			QString sBtnCancelText,
			int iMsgBoxIcon);


		//-----------------
		enum enumDialogStatus
		{
			enuNoServicesEnumerated = 0,
			enuDisconnected,
			enuConnecting,
			enuConnected
		};
		enumDialogStatus m_eDialogStatus;
		void updateDialogStatus(enumDialogStatus eDialogStatus);

		QMap <QString, struct _ServiceCmdData> mapServiceCmdData;


		bool ExtractServiceList(QString sServerName);
		MCmdACOM * m_pCmdACOM;
		QFile * m_pfileCommLog;
		void openRedirectionFile();
		//-------------------------

		void displayMessage(QString sMsg);
		void displayMessageIN(QString sMsg);
		void displayMessageOUT(QString sMsg);

		Ui::MMainDialogB	*m_ui;
		MStyledTitleBar		*m_pTitle;
		QString				m_sTitle;

		//reading
		QString				m_languageName;
		char				*m_pLanguage;

		QString				m_HighStateLabelColor;
		QString				m_LowStateLabelColor;


		MStyledLangPick		*m_pLangPick;
		MAboutWidget		*m_pAboutDlg;
};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/