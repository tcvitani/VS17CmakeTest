
#include "MLibToggleKeys.h"
#include "MTracer.h"

extern "C" {
	#include <CSRLC32.H>
	#include <run.H>
	#include "winuser.h"
}

MLibToggleKeys::MLibToggleKeys()
{
}


MLibToggleKeys::~MLibToggleKeys()
{
}


bool MLibToggleKeys::GetToggleKeyState(int lKey)
{
	unsigned char cKeys[256] = {0};
		
	GetKeyboardState(cKeys);
		
	return ((cKeys[lKey] & 1) != 0);
}


void MLibToggleKeys::SetToggleKeyState(int lKey, bool bState, bool bToggle)
{
	bool bKeyState;
	unsigned char cKeys[256] = { 0 };
	
	// if bToggle true simulate press key
	if(bToggle)
	{
		keybd_event(lKey, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(lKey, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	// else set key to bState value
	else
	{
		GetKeyboardState(cKeys);
	
		bKeyState = ((cKeys[lKey] & 1) != 0);
		
		if((bState && !bKeyState) || (bKeyState && !bState))
		{
			keybd_event(lKey, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
			keybd_event(lKey, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
		}
	}
}

