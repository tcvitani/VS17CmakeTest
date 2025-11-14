

#include "MRVideoMsg.h"



MRVideoMsg::MRVideoMsg()
{
	
}


MRVideoMsg::~MRVideoMsg()
{

}


QString MRVideoMsg::getMessageDescription()
{
	QString sRetVal;
	
	switch(getType())
	{
	case enuRVideoActivateSourceReq:
		{
			QString sNewHost = ((MRVideoMsgVideoActivateSrc*)this)->m_sNewHost;;
			QString sNewSourceId = ((MRVideoMsgVideoActivateSrc*)this)->m_sNewSourceId;
			quint16 uiNewPort = ((MRVideoMsgVideoActivateSrc*)this)->m_uiNewPort;

			sRetVal = QString("Message type enuRVideoActivateSourceReq: Host:%1;Port:%2;SourceID:%3 ")
						.arg(sNewHost).arg(uiNewPort).arg(sNewSourceId);
		}
		break;
	case enuRVideoReactivateReq:
		{
			sRetVal = QString("Message type enuRVideoReactivateReq");
		}
		break;
	case enuRVideoDeactivateReq:
		{
			sRetVal = QString("Message type enuRVideoDeactivateReq");
		}
		break;
	case enuRVideoUnfreezeReq:
		{
			sRetVal = QString("Message type enuRVideoUnfreezeReq");
		}
	    break;
	case enuRVideoFreezeReq:
		{
			sRetVal = QString("Message type enuRVideoFreezeReq");
		}
	    break;
	case enuRVideoFreezeRsp:
		{
			bool bOK = ((MRVideoMsgVideoFreezeRsp*)this)->m_bOK;
			sRetVal = QString("Message type enuRVideoFreezeRsp %1").arg(bOK?"OK":"NOK");
		}
	    break;
	case enuRVideoSaveReq:
		{
			sRetVal = QString("Message type enuRVideoSaveReq");
		}
	    break;
	case enuRVideoSaveRsp:
		{
			bool bOK = ((MRVideoMsgVideoSaveRsp*)this)->m_bOK;
			sRetVal = QString("Message type enuRVideoSaveRsp %1").arg(bOK?"OK":"NOK");
		}
	    break;
	    break;
	default:
		sRetVal = QString("Unknown message type %1").arg(getType());
	    break;
	}

	return sRetVal;
}
