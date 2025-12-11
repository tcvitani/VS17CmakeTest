#include "Automat.h"
#include "InstTracer.h"

/*!
* --------------------------------------------------------------------
MAutState
* --------------------------------------------------------------------
*/
AutTransition * AutState::getTransition(int iEventID)
{
	for (int i = 0; i < m_lstTransitions.size(); ++i)
	{
		if (m_lstTransitions.at(i) != NULL)
			if (m_lstTransitions.at(i)->getEventId() == iEventID)
			{
				return m_lstTransitions.at(i);
			}
	}

	return NULL;
}

/*!
* --------------------------------------------------------------------
Automat
* --------------------------------------------------------------------
*/

bool Automat::test()
{
	//Test the initial automat state

	if (m_lstAuthStates.size() < 1)
	{
		TRACE_W(m_siInstId, "Automat::init Undefined m_lstAuthStates list!");
		m_iErrorStatus = AUT_TABLE_NOK;
		return false;
	}

	if (m_iInitialStateID < 0)
	{
		TRACE_W(m_siInstId, "Automat::init Undefined m_iInitialStateID < 0!");
		m_iErrorStatus = AUT_INITIAL_STATE_NOK;
		return false;
	}

	m_iCurrentStateID = m_iInitialStateID;

	m_bRecursion = false;

	m_bAutInitialized = true;
	m_iErrorStatus = AUT_OK;
	return true;
}

bool Automat::processEvent(AutEvent * event)
{
	if (!isAutInitialized())	 //if something wrong with the automat definition
	{
		TRACE_W(m_siInstId, "ERROR : Automat::sendEvent AUTOMAT: not initialized! Call test() before using automat!");
		m_iErrorStatus = AUT_NOK;
		return false;
	}


	//Test if the event is OK
	if (event == NULL)
	{
		TRACE_W(m_siInstId, QString("ERROR : Automat::sendEvent Received event==NULL. m_iCurrentStateID = %1").arg(m_iCurrentStateID));
		this->reset();

		m_iErrorStatus = AUT_EVT_UNKNOWN;
		return false;
	}


	m_iCurrentEventID = event->getEventId();

	saveEvent(event); //Save the current event data (virtual)

	//Get the current state object
	AutState * pCurrentState = getCurrentState();

	//Test if the state is OK 
	if (pCurrentState == NULL)
	{
		QString sMsg = "The automat state cannot be found!";
		TRACE_W(m_siInstId, sMsg);
		this->reset();

		m_iErrorStatus = AUT_TABLE_NOK;
		return false;
	}


	//Find the transition in the current state that corresponds for the currently received event 
	AutTransition * pTransition = NULL;
	pTransition = pCurrentState->getTransition(m_iCurrentEventID);


	//Test if the state is OK 
	if (pTransition == NULL)
	{
		QString sMsg = "TRANSITION NOT FOUND!";
		TRACE_W(m_siInstId, sMsg);

		m_iErrorStatus = AUT_NEXT_STATE_NOK;
		return false;
	}


	//Change the current state than call the transition function
	this->m_iCurrentStateID = pTransition->getNextStateId();



	// Execute the provided action 
	if (pTransition->getFunctor() != NULL)
	{
		AutomatFunctor <Automat> *pFunctor = pTransition->getFunctor();
		pFunctor->Call(event);
	}

	if (event != NULL) delete event;

	m_iErrorStatus = AUT_OK;
	return true;

}

AutState * Automat::getCurrentState()
{
	//Find the current state object
	AutState * pCurrentState = NULL;
	AutState *current;

	for (int i = 0; i < m_lstAuthStates.size(); ++i)
	{
		current = m_lstAuthStates.at(i);

		if (current != NULL)
			if (current->getStateID() == m_iCurrentStateID)
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
void Automat::saveEvent(AutEvent * event)
{
	if (m_pCurrentEvent != NULL)
		delete m_pCurrentEvent;

	m_pCurrentEvent = new AutEvent(event->getEventId(), event->getEventDesc(), event->hasData());

	return;
}

//Keeps the pointer to the current Input package
// and delete the old one when the new one come
void Automat::saveLastInputPck(QString sInPCK)
{
	m_sLastInputPCK = sInPCK;
}

//Keeps the pointer to the current Output package
// and delete the old one when the new one come
void Automat::saveLastOutputPck(QString sOutPCK)
{


	m_sLastOutputPCK = sOutPCK;
}

Automat::~Automat()
{
	while (!m_lstAuthStates.isEmpty())
		delete m_lstAuthStates.takeFirst();

	if (m_pCurrentEvent != NULL)
		delete m_pCurrentEvent;

};


