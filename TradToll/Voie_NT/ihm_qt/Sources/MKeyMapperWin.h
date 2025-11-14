#ifndef MKEY_MAPPER_WIN_H
#define MKEY_MAPPER_WIN_H

#include "windows.h"



class QKeyEvent;

class MKeyMapperWin 
{
public:
	enum WindowsNativeModifiers {
		ShiftLeft            = 0x00000001,
		ControlLeft          = 0x00000002,
		AltLeft              = 0x00000004,
		MetaLeft             = 0x00000008,
		ShiftRight           = 0x00000010,
		ControlRight         = 0x00000020,
		AltRight             = 0x00000040,
		MetaRight            = 0x00000080,
		CapsLock             = 0x00000100,
		NumLock              = 0x00000200,
		ScrollLock           = 0x00000400,
		ExtendedKey          = 0x01000000,

		// Convenience mappings
		ShiftAny             = 0x00000011,
		ControlAny           = 0x00000022,
		AltAny               = 0x00000044,
		MetaAny              = 0x00000088,
		LockAny              = 0x00000700
	};

	static QKeyEvent * translateToKeyEvent(const MSG &msg);

private:

// 	static bool createAndPostEvent(QWidget * pWidget, QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
//                              const QString &text, bool autorepeat, int count,
//                              quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);

	static QKeyEvent * createKeyEvent(QEvent::Type type, int code, Qt::KeyboardModifiers modifiers,
                             const QString &text, bool autorepeat, int count,
                             quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers);
};


#endif

