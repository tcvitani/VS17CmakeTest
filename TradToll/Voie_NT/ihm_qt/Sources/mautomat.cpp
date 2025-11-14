


#include "mautomat.h"
#include "MTracer.h"

/*!
* --------------------------------------------------------------------
	MAutState
* --------------------------------------------------------------------
*/
MAutTransition * MAutState::getTransition(int iEventID)
{
    for (int i = 0; i < m_lstTransitions.size(); ++i) 
	{
		if(m_lstTransitions.at(i)!=NULL)		
			if(m_lstTransitions.at(i)->getEventId() == iEventID)
			{	
				return m_lstTransitions.at(i);
			}
	}

	return NULL;
}





/*!
* --------------------------------------------------------------------
	MAUTOMAT
* --------------------------------------------------------------------
*/

bool MAutomat::test()
{
  //Test the initial automat state

	if (m_lstAuthStates.size() < 1) 
    {
        TRACE_W("MAutomat::init Undefined m_lstAuthStates list!");
        m_iErrorStatus = AUT_TABLE_NOK;
		return false;
    }
    
    if (m_iInitialStateID < 0)
    {
        TRACE_W("MAutomat::init Undefined m_iInitialStateID < 0!");
        m_iErrorStatus = AUT_INITIAL_STATE_NOK;
		return false;
    }

	m_iCurrentStateID = m_iInitialStateID;
   
    m_bRecursion = FALSE;

	m_bAutInitialized = true;
    m_iErrorStatus = AUT_OK;
	return true;
}




/*
	TRACE_D(QString("MIHMAuthAutomate::doTrans1_1TakeCtrlPool state = %d event:%s",
							m_iCurrentStateID,
							pEvent->getEventDesc().toLocal8Bit().data());

*/
bool MAutomat::processEvent(MAutEvent * event)
{
	if(!isAutInitialized())	 //if something wrong with the automat definition
	{ 
		TRACE_W("ERROR : MAutomat::sendEvent AUTOMAT: not initialized! Call test() before using automat!");
		m_iErrorStatus = AUT_NOK;
		return false;
	}


	//Test if the event is OK
	if (event == NULL )
    {
		TRACE_W(QString("ERROR : MAutomat::sendEvent Received event==NULL. m_iCurrentStateID = %1").arg(m_iCurrentStateID));
        this->reset();
        
		m_iErrorStatus = AUT_EVT_UNKNOWN;
		return false;
    }


  	m_iCurrentEventID = event->getEventId();

	saveEvent(event); //Save the current event data (virtual)
	
	//Get the current state object
	MAutState * pCurrentState = getCurrentState();
	
	QString sMsg; 
	sMsg = QString("MAutomat::sendEvent event = %1   m_iCurrentStateID = %2 ")
				.arg(event->getEventDesc())
				.arg(m_iCurrentStateID);
// 
// 	TRACE_D(sMsg);
// 
	//Test if the state is OK 
	if (pCurrentState == NULL )
    {

		sMsg = sMsg + "The automat state cannot be found!";
		TRACE_W(sMsg);
        this->reset();

		m_iErrorStatus = AUT_TABLE_NOK;
		return false;
    }


	//Find the transition in the current state that corresponds for the currently recived event 
	MAutTransition * pTransition = NULL;
	pTransition = pCurrentState->getTransition(m_iCurrentEventID);

	
	//Test if the state is OK 
	if (pTransition == NULL )
    {
		sMsg = sMsg + "TRANSITION NOT FOUND!";
		TRACE_W(sMsg);
		
		m_iErrorStatus = AUT_NEXT_STATE_NOK;
		return false;
    }

	
	//Change the current state than call the transition function
	this->m_iCurrentStateID = pTransition->getNextStateId();


   
    // Execute the provided action 
	if(pTransition->getFunctor()!= NULL)
	{	MAutomatFunctor <MAutomat> *pFunctor = pTransition->getFunctor();
		pFunctor->Call(event);
    }
	
	if(event!=NULL) delete event;
     
	m_iErrorStatus = AUT_OK;
	return true;

}

MAutState * MAutomat::getCurrentState()
{
	//Find the current state object
	MAutState * pCurrentState = NULL;
	MAutState *current;

	for (int i = 0; i < m_lstAuthStates.size(); ++i) 
	{
		current = m_lstAuthStates.at(i);
		
		if(current!=NULL)
			if(current->getStateID() == m_iCurrentStateID)
			{	
				pCurrentState = current;
				break;
			}
	}

	return pCurrentState;
}


/*
	Set the automat to initial state
* --------------------------------------------------------------------



/*!
//Copy the data of the current event so it can be used in automat transitions
*/
void MAutomat::saveEvent(MAutEvent * event)
{
	if(m_pCurrentEvent!=NULL) 
			delete m_pCurrentEvent;
	
	m_pCurrentEvent = new MAutEvent(event->getEventId(),event->getEventDesc(),event->hasData());

	return;
}

//Keeps the pointer to the current Input package
// and delete the old one when the new one come
void MAutomat::saveLastInputPck(QString sInPCK)
{
	m_sLastInputPCK = sInPCK;
}

//Keeps the pointer to the current Output package
// and delete the old one when the new one come
void MAutomat::saveLastOutputPck(QString sOutPCK)
{

	
	m_sLastOutputPCK = sOutPCK;		
}



MAutomat::~MAutomat()
{ 
	 while (!m_lstAuthStates.isEmpty())
			delete m_lstAuthStates.takeFirst();

	if(m_pCurrentEvent!=NULL) 
			delete m_pCurrentEvent;

};


