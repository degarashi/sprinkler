#define _STRINGIZE(s) #s
#define STRINGIZE(s) _STRINGIZE(s)
namespace name {
	LPCTSTR Mutex = _T("Hook") _T(STRINGIZE(BITVERSION)) _T("_Mutex"),
			Event = _T("Hook") _T(STRINGIZE(BITVERSION)) _T("_Event"),
			WndClass = _T("Hook") _T(STRINGIZE(BITVERSION)),
			WndTitle = _T("Hidden Hook") _T(STRINGIZE(BITVERSION));
}
