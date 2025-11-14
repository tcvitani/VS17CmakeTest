

#ifndef MINPUT_ASYNC_EX_REQ_H
#define MINPUT_ASYNC_EX_REQ_H

#include <QString>
#include "MInputExField.h"
#include "MInputExReq.h"

class QDomNode;


class MInputAsyncExReq:public MInputExReq
{
public:

//-----------------------------------------------------

	MInputAsyncExReq();
	~MInputAsyncExReq();
	MInputAsyncExReq(MInputAsyncExReq &obj);
	virtual MInputAsyncExReq& operator=(const MInputAsyncExReq& right);

	bool init(QString sLaneType);
	void updateInputValues(QString sEncStrValues);
	QString getEncStrValues();
	
	
	bool verifyResponseFormat(QString sDlgResult);

};



#endif
