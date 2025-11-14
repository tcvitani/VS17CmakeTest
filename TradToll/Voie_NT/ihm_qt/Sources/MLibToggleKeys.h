#ifndef MLIBTOGGLEKEYS_H
#define MLIBTOGGLEKEYS_H



class MLibToggleKeys
{
public:	
	MLibToggleKeys();
	~MLibToggleKeys();
	static bool GetToggleKeyState(int lKey);
	static void SetToggleKeyState(int lKey, bool bState, bool bToggle);


};


#endif