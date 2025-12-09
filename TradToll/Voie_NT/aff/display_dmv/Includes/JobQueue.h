/********* (v) 2024 EMOVIS tehnologije d.o.o. - All rights reserved ******** */
/*                                                                           */
/* --------------------------------------------------------------------------*/
/* MODULE: 	 AFF														     */
/* FILE:	 JobQueue.h					     								 */
/* LANGUAGE: C++                                                             */
/* --------------------------------------------------------------------------*/
/* DESCRIPTION:                                                              */
/*										                                     */
/* --------------------------------------------------------------------------*/
/* HISTORY: 2024/06/03 Creation                                              */
/*****************************************************************************/

#ifndef JOBQUEUE_H_
#define JOBQUEUE_H_

#include <QObject>
#include <QMutex>
#include <QDateTime>
#include <QUuid>
#include <OblakMsg.h>
#include <atomic>

#include <tlm_dmv.h>
#include <fic_gere.h>
#include <aff_ext.h>
#include <aff_util.h>
#include <csr_tlm.h>

class JobQueue;

class Job
{
public:
	Job() {}
	Job(const Job& toCopy) : m_eJobType(toCopy.m_eJobType) { }
	virtual ~Job() {}

	virtual OblakMsg::eMessageType getType() const { return m_eJobType; }

	JobQueue* getSender() const { return m_pSender; }
	void setSender(JobQueue* pSender) { m_pSender = pSender; }

protected:
	Job(OblakMsg::eMessageType eJobType) : m_eJobType(eJobType) { }

	OblakMsg::eMessageType m_eJobType = OblakMsg::eLastMessageType;

private:
	JobQueue* m_pSender = nullptr;
};

class JobQueue : public QObject
{
	Q_OBJECT

public:
	JobQueue();
	~JobQueue();

	// Used by sender
	void EnqueueJob(Job* pJob);
	void ResetPending();

	void AddJob(Job* pJob);
	void DeleteJobs();

	// Used by receiver/owner
	Job* ConsumeJob();

private: signals:
	void newJobNotification(void);

private:
	QMutex m_mtxQueueLock;
	QList<Job*> m_lstJobQueue;
	std::atomic<bool> m_fPending;
};

class JobAffRequest : public Job
{
public:
	JobAffRequest() : Job(OblakMsg::eOblakMsg_SetPropertiesAndText) { }
	JobAffRequest(const JobAffRequest& toCopy) : Job(toCopy), m_iTransactionId(toCopy.m_iTransactionId), m_sCurrentScreen(toCopy.m_sCurrentScreen) { }
	~JobAffRequest() { }

	int getPriority() const { return m_iTransactionId; }
	void setPriority(int value) { m_iTransactionId = value; }

	struct_screen getCurrentScreen() const { return m_sCurrentScreen; }
	void setCurrentScreen(struct_screen value) { m_sCurrentScreen = value; }

private:
	int m_iTransactionId;
	struct_screen m_sCurrentScreen;
};

class JobTlmRequest : public Job
{
public:
	JobTlmRequest() : Job(OblakMsg::eOblakMsg_SetImage) { }
	JobTlmRequest(const JobTlmRequest& toCopy) : Job(toCopy), m_eTlmImage(toCopy.m_eTlmImage) { }
	~JobTlmRequest() { }

	enum_tlm_dmv_images getTlmImage() const { return m_eTlmImage; }
	void setTlmImage(enum_tlm_dmv_images value) { m_eTlmImage = value; }

private:
	enum_tlm_dmv_images m_eTlmImage;
};

class JobFlasherRequest : public Job
{
public:
	JobFlasherRequest() : Job(OblakMsg::eOblakMsg_SetFlasher) { }
	JobFlasherRequest(const JobFlasherRequest& toCopy) : Job(toCopy), m_eFlasherState(toCopy.m_eFlasherState) { }
	~JobFlasherRequest() { }

	enum_flasher_state getFlasherState() const { return m_eFlasherState; }
	void setFlasherState(enum_flasher_state value) { m_eFlasherState = value; }

private:
	enum_flasher_state m_eFlasherState;
};

// This job should only be enqueued to confirm JobFlasherRequest
class JobChangeFlasherParamteter : public Job
{
public:
	JobChangeFlasherParamteter() : Job(OblakMsg::eOblakMsg_ChangeFlasherParameter) { }
	JobChangeFlasherParamteter(const JobChangeFlasherParamteter& toCopy) : Job(toCopy) { }
	~JobChangeFlasherParamteter() { }

private:

};

#endif