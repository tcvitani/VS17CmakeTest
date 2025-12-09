#ifndef AUTOMAT_H
#define AUTOMAT_H


#include <qstring.h>
#include <qlist.h>
#include <qfile.h>
#include <qdatetime.h>
#include "AutomatFunctor.h"

class Automat;
/*!

*/
class AutEvent
{

public:
	AutEvent() :m_iEventID(0), m_sDesc(""), m_bHasData(false) {};
	AutEvent(int iEventID, QString sDesc, bool bHasData)
		:m_iEventID(iEventID), m_sDesc(sDesc), m_bHasData(bHasData) {};// //int IDEvent, QString sDesc, bool bArgs

	virtual ~AutEvent(){}

	int getEventId(){ return m_iEventID; };
	QString getEventDesc(){ return m_sDesc; };
	bool hasData(){ return m_bHasData; };

protected:
	int m_iEventID;
	QString m_sDesc;
	bool m_bHasData;
};



/*
A transition is defined by:
event ,
next state
and the function to be called when the state is to be changed
*/
class AutTransition
{
public:

	AutTransition(int IDEvent, int IDNextState, Automat * pAut, bool (Automat::*pAction) (AutEvent * pEvent))
	{
		m_iEventID = IDEvent;
		m_iNextStateID = IDNextState;

		pActionFunctor = new AutomatFunctor <Automat>(pAut, pAction);
	};
	~AutTransition() { delete pActionFunctor; };


	int getEventId()  { return m_iEventID; };
	int getNextStateId(){ return m_iNextStateID; };

	//This is the functor that is used for the action funtion call
	AutomatFunctor <Automat> * getFunctor() { return pActionFunctor; };

private:
	int m_iEventID;
	int m_iNextStateID;
	AutomatFunctor <Automat> *pActionFunctor;//The Functor nedeed for the function pointer

};




/*
The state is identified by ID

Contains a list of all possible transitions that can happend
*/
class AutState
{
public:
	explicit AutState(int iState) : m_iStateID(iState)
	{};

	~AutState(){
		while (!m_lstTransitions.isEmpty())
			delete m_lstTransitions.takeFirst();
	};

	void addTransition(AutTransition * pT){ m_lstTransitions.append(pT); };
	AutTransition * getTransition(int iEventID);

	int getStateID() { return m_iStateID; };
private:
	int m_iStateID;
	QList <AutTransition*> m_lstTransitions; //list of MAutTransitions
};



/*

*/
class Automat
{
public:
	explicit Automat(short int siInstId) :m_siInstId(siInstId), m_bAutInitialized(false), m_pCurrentEvent(NULL), m_bTrace(false),
		m_sLastInputPCK(), m_sLastOutputPCK()
	{
		;
	};

	//	MAutomat(QString name, int iType):m_bAutInitialized(false),m_sName(name),m_iAutomatType(iType) {};

	virtual ~Automat();

	typedef enum
	{
		AUT_OK,
		AUT_TABLE_NOK,
		AUT_INITIAL_STATE_NOK,
		AUT_ALLOC_NOK,
		AUT_FREE_NOK,
		AUT_AUTOMATE_ID_NOK,
		AUT_EVT_UNKNOWN,
		AUT_NEXT_STATE_NOK,
		AUT_DEBUG_NOK,
		AUT_NOK //something else gone wrong

	} enumErrorStatusTypes;

	bool test();
	virtual bool reset() = 0;
	bool processEvent(AutEvent * event);


	bool isAutInitialized(){ return m_bAutInitialized; }; //is the MAutomat class defined well?
	int getCurrentStateId(){ return m_iCurrentStateID; };

	int getCurrentEventId(){ return m_iCurrentEventID; };
	bool isInRecursion(){ return m_bRecursion; };


	void setTracing(bool bTrace); //the automat will not trace unles you initialize log file first 


	void logToFile(const QString &s);

protected:
	void setTracingFile(const char *pFileName);

	void addAutState(AutState * p) { m_lstAuthStates.append(p); };
	AutState * getCurrentState();

	virtual void saveEvent(AutEvent * event);

	virtual void saveLastInputPck(QString sInPCK);
	virtual void saveLastOutputPck(QString sOutPCK);


	int m_iInitialStateID;
	int m_iCurrentStateID;

	int m_iCurrentEventID;

	AutEvent * m_pCurrentEvent;

	QString m_sLastInputPCK;
	QString m_sLastOutputPCK;

	short int m_siInstId;

private:
	bool m_bRecursion;

	//Lista stanja po kojoj cemo trazit trenutno stanje
	QList <AutState*> m_lstAuthStates;

	int m_iErrorStatus;
	bool m_bAutInitialized;

	QString m_sTraceFileNameBase;
	QString m_sTraceFileNamePath;
	QDate m_datLogFileDate;

	bool m_bTrace;


};







#endif