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
		void onClose();


		void onBtnConnect_clicked();
		void onBtnDisonnect_clicked();
		void onBtnBrowse_clicked();
		void onBtnSendCmd_clicked();
		void onBtnClear_clicked();
		void onchkRedirect_stateChanged(int);
		void onCommand_returnPressed();

		void onBtnGenerateENL_REQ_clicked();
		void onBtnGenerateEXL_REQ_clicked();

		void onBtnGenerateENL_REQ2_clicked();
		void onBtnGenerateEXL_REQ2_clicked();

		void onMessage(const QString &value);

		void onConnected();
		void onDisconnected();
		void onACOMMessageReceived(QByteArray);
		void onACOMMessageSent(QByteArray);

	protected:
		;
	private:
		QString MessageToHex(QString strMessage);

		bool raiseMsgBox(QWidget *pParent,
			QString sMessage,
			QString sBtnOkText,
			QString sBtnCancelText,
			int iMsgBoxIcon);


		//-----------------
		enum enumDialogStatus
		{
			enuDisconnected = 0,
			enuConnecting,
			enuConnected
		};
		enumDialogStatus m_eDialogStatus;
		void updateDialogStatus(enumDialogStatus eDialogStatus);

		QMap <QString, struct _ServiceCmdData> mapServiceCmdData;


		MCmdACOM * m_pCmdACOM;
		QFile * m_pfileCommLog;
		void openRedirectionFile();
		//-------------------------

		void displayMessage(QString sMsg);
		void displayMessageIN(QString sMsg);
		void displayMessageOUT(QString sMsg);

		Ui::MMainDialogB	*m_ui;
		QString				m_sTitle;

		//reading
		QString				m_languageName;
		char				*m_pLanguage;

		QString				m_HighStateLabelColor;
		QString				m_LowStateLabelColor;

};

/*-------------------------------- FUNCTIONS: -------------------------------*/

/*-------------------------------- VARIABLES: -------------------------------*/

#endif

/*-------------------------------- END OF FILE ------------------------------*/