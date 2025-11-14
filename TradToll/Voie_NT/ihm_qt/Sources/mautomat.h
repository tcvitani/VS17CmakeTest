#ifndef MAUTOMAT_H
#define MAUTOMAT_H


#include <qstring.h>
#include <qlist.h>
#include <qfile.h>
#include <qdatetime.h>
#include "mautomatfunctor.h"





class MAutomat;
/*!

*/
class MAutEvent
{

public:
	MAutEvent():m_iEventID(0),m_sDesc(""),m_bHasData(false) {};
	MAutEvent(int iEventID, QString sDesc, bool bHasData)
		:m_iEventID(iEventID),m_sDesc(sDesc),m_bHasData(bHasData) {};// //int IDEvent, QString sDesc, bool bArgs
	
	virtual ~MAutEvent(){}

	int getEventId(){ return m_iEventID; };
	QString getEventDesc(){return m_sDesc;};
	bool hasData(){return m_bHasData;};

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
class MAutTransition
{
public:

	MAutTransition(int IDEvent, int IDNextState, MAutomat * pAut , bool (MAutomat::*pAction ) (MAutEvent * pEvent))
	{
		m_iEventID = IDEvent;
		m_iNextStateID = IDNextState;
		pActionFunctor = new MAutomatFunctor <MAutomat>(pAut, pAction);
	};
	~MAutTransition() { delete pActionFunctor;};
	
	
	int getEventId()  {return m_iEventID;};
	int getNextStateId(){return m_iNextStateID;};

	//This is the functor that is used for the action funtion call
	MAutomatFunctor <MAutomat> * getFunctor() {return pActionFunctor;}; 
	
private:
	int m_iEventID;
	int m_iNextStateID;
	MAutomatFunctor <MAutomat> *pActionFunctor;//The Functor nedeed for the function pointer
	
};




/*
	The state is identified by ID
		
	Contains a list of all possible transitions that can happend
*/
class MAutState
{
public:
	MAutState(int iState): m_iStateID(iState)
	{};

	~MAutState(){
		 while (!m_lstTransitions.isEmpty())
			 delete m_lstTransitions.takeFirst();
	};

	void addTransition(MAutTransition * pT){ m_lstTransitions.append(pT);};
	MAutTransition * getTransition(int iEventID);

	int getStateID() {return m_iStateID;};
private:
	int m_iStateID;
	QList <MAutTransition*> m_lstTransitions; //list of MAutTransitions
};



/*

*/
class MAutomat
{
public:
	MAutomat():m_bAutInitialized(false), m_pCurrentEvent(NULL), m_bTrace(false), m_fout(NULL),
				m_sLastInputPCK(), m_sLastOutputPCK()
	{
		//tmTraceStart.start();
		m_fout = NULL;
	};
	
//	MAutomat(QString name, int iType):m_bAutInitialized(false),m_sName(name),m_iAutomatType(iType) {};
	
	virtual ~MAutomat();

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
	bool processEvent(MAutEvent * event);


	bool isAutInitialized(){return m_bAutInitialized;}; //is the MAutomat class defined well?
	int getCurrentStateId(){ return m_iCurrentStateID;};
	
	int getCurrentEventId(){ return m_iCurrentEventID;};
	bool isInRecursion (){ return m_bRecursion;};
	
	
	void setTracing(bool bTrace); //the automat will not trace unles you initialize log file first 
	

	void logToFile(const QString &s);
		
protected:
	void setTracingFile(const char *pFileName);

	void addAutState(MAutState * p) { m_lstAuthStates.append(p);};
	MAutState * getCurrentState();

	virtual void saveEvent(MAutEvent * event);

	virtual void saveLastInputPck(QString sInPCK);
	virtual void saveLastOutputPck(QString sOutPCK);
	

	int m_iInitialStateID;
	int m_iCurrentStateID;
	
	int m_iCurrentEventID;

	MAutEvent * m_pCurrentEvent;

	QString m_sLastInputPCK;
	QString m_sLastOutputPCK;
	
private:

	bool m_bRecursion;
	
	//Lista stanja po kojoj cemo trazit trenutno stanje
	QList <MAutState*> m_lstAuthStates; 

	int m_iErrorStatus;
	bool m_bAutInitialized;

	QFile *m_fout;
	QString m_sTraceFileNameBase;
	QString m_sTraceFileNamePath;
	QDate m_datLogFileDate;

	bool m_bTrace;
	//QTime tmTraceStart;
	
	
};







#endif



