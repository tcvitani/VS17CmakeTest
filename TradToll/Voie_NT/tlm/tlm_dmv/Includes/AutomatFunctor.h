#ifndef AUTOMATFUNCTOR_H
#define AUTOMATFUNCTOR_H

#include "Automat.h"


class AutEvent;
// abstract base class
class MFunctor
{
public:

	// two possible functions to call member function. virtual cause derived
	// classes will use a pointer to an object and a pointer to a member function
	// to make the function call
	virtual void operator()(AutEvent * pEvent) = 0;  // call using operator
	virtual void Call(AutEvent * pEvent) = 0;        // call using function
};


// derived template class
template <class TClass> class AutomatFunctor : public MFunctor
{
private:
	bool (TClass::*fpt)(AutEvent * pEvent);   // pointer to member function
	TClass* pt2Object;                  // pointer to object

public:

	// constructor - takes pointer to an object and pointer to a member and stores
	// them in two private variables
	AutomatFunctor(TClass* _pt2Object, bool(TClass::*_fpt)(AutEvent * pEvent))
	{
		pt2Object = _pt2Object;  fpt = _fpt;
	};

	// override operator "()"
	virtual void operator()(AutEvent * pEvent)
	{
		(*pt2Object.*fpt)(pEvent);
	};              // execute member function

	// override function "Call"
	virtual void Call(AutEvent * pEvent)
	{
		(*pt2Object.*fpt)(pEvent);
	};             // execute member function
};

#endif
