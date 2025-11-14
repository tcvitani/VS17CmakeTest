
#include "MIhmMsg.h"
#include "MHelpFuncs.h"


MIhmMsg::MIhmMsg()
{
	
}


MIhmMsg::~MIhmMsg()
{

}



MIhmMsgOpenLinkReq& MIhmMsgOpenLinkReq::operator=(const MIhmMsgOpenLinkReq& right)
{
	m_iTargetWin = right.m_iTargetWin;
	m_sUrl = MHelpFuncs::deepCopy(right.m_sUrl);
	m_bVisible = right.m_bVisible;
	return *this;
}

MIhmMsgOpenAboutDlgReq& MIhmMsgOpenAboutDlgReq::operator=(const MIhmMsgOpenAboutDlgReq& right)
{
	m_sAboutDlgCnf = MHelpFuncs::deepCopy(right.m_sAboutDlgCnf);
	return *this;
}