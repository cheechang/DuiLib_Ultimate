#ifndef _VERSIONHELPERS_H_INCLUDED_
#define _VERSIONHELPERS_H_INCLUDED_
#include <specstrings.h>
#include <wchar.h>	// wcsstr：解析 szCSDVersion 中的 Service Pack 版本

namespace DuiLib 
{
#define _WIN32_WINNT_NT4                    0x0400
#define _WIN32_WINNT_WIN2K                  0x0500
#define _WIN32_WINNT_WINXP                  0x0501
#define _WIN32_WINNT_WS03                   0x0502
#define _WIN32_WINNT_WIN6                   0x0600
#define _WIN32_WINNT_VISTA                  0x0600
#define _WIN32_WINNT_WS08                   0x0600
#define _WIN32_WINNT_LONGHORN               0x0600
#define _WIN32_WINNT_WIN7                   0x0601
#define _WIN32_WINNT_WIN8                   0x0602
#define _WIN32_WINNT_WINBLUE                0x0603
#define _WIN32_WINNT_WINTHRESHOLD           0x0A00 /* ABRACADABRA_THRESHOLD*/
#define _WIN32_WINNT_WIN10                  0x0A00 /* ABRACADABRA_THRESHOLD*/

#define WM_DPICHANGED                   0x02E0

#if(WINVER < 0x0601)
#define WM_TOUCH                        0x0240

/*
 * Touch Input defines and functions
 */

/*
 * Touch input handle
 */
DECLARE_HANDLE(HTOUCHINPUT);

typedef struct tagTOUCHINPUT {
    LONG x;
    LONG y;
    HANDLE hSource;
    DWORD dwID;
    DWORD dwFlags;
    DWORD dwMask;
    DWORD dwTime;
    ULONG_PTR dwExtraInfo;
    DWORD cxContact;
    DWORD cyContact;
} TOUCHINPUT, *PTOUCHINPUT;
typedef TOUCHINPUT const * PCTOUCHINPUT;


/*
 * Conversion of touch input coordinates to pixels
 */
#define TOUCH_COORD_TO_PIXEL(l)         ((l) / 100)

/*
 * Touch input flag values (TOUCHINPUT.dwFlags)
 */
#define TOUCHEVENTF_MOVE            0x0001
#define TOUCHEVENTF_DOWN            0x0002
#define TOUCHEVENTF_UP              0x0004
#define TOUCHEVENTF_INRANGE         0x0008
#define TOUCHEVENTF_PRIMARY         0x0010
#define TOUCHEVENTF_NOCOALESCE      0x0020
#define TOUCHEVENTF_PEN             0x0040
#define TOUCHEVENTF_PALM            0x0080

/*
 * Touch input mask values (TOUCHINPUT.dwMask)
 */
#define TOUCHINPUTMASKF_TIMEFROMSYSTEM  0x0001  // the dwTime field contains a system generated value
#define TOUCHINPUTMASKF_EXTRAINFO       0x0002  // the dwExtraInfo field is valid
#define TOUCHINPUTMASKF_CONTACTAREA     0x0004  // the cxContact and cyContact fields are valid
/*
 * RegisterTouchWindow flag values
 */
#define TWF_FINETOUCH       (0x00000001)
#define TWF_WANTPALM        (0x00000002)

#endif

	// ==================== 真实版本获取（RtlGetVersion） ====================
	// 说明：VerifyVersionInfo/GetVersionEx 在应用缺少兼容性清单（manifest）时，
	//       会被系统限制报告为 Win8.1(6.3) 及以下，导致 Win10/Win11 检测不到；
	//       RtlGetVersion（ntdll.dll）始终返回真实版本，不受清单影响。
	//       Win11 版本号为 10.0.22000+（与 Win10 同为 major=10/minor=0，仅 build 号不同）。
	typedef struct _DUILIB_RTL_OSVERSIONINFOW {
	 ULONG dwOSVersionInfoSize;
	 ULONG dwMajorVersion;
	 ULONG dwMinorVersion;
	 ULONG dwBuildNumber;
	 ULONG dwPlatformId;
	 WCHAR szCSDVersion[128];
	} DUILIB_RTL_OSVERSIONINFOW, *PDUILIB_RTL_OSVERSIONINFOW;

	typedef LONG(NTAPI *PFN_DUILIB_RtlGetVersion)(PDUILIB_RTL_OSVERSIONINFOW);

	static BOOL GetRealOSVersion(PDUILIB_RTL_OSVERSIONINFOW pVersion)
	{
	 if (pVersion == NULL) return FALSE;
	 HMODULE hNtdll = ::GetModuleHandleW(L"ntdll.dll");
	 if (hNtdll == NULL) return FALSE;
	 PFN_DUILIB_RtlGetVersion pfnRtlGetVersion = (PFN_DUILIB_RtlGetVersion)::GetProcAddress(hNtdll, "RtlGetVersion");
	 if (pfnRtlGetVersion == NULL) return FALSE;
	 pVersion->dwOSVersionInfoSize = sizeof(DUILIB_RTL_OSVERSIONINFOW);
	 return pfnRtlGetVersion(pVersion) == 0;
	}

	static BOOL IsWindowsVersionOrGreater(WORD wMajorVersion, WORD wMinorVersion, WORD wServicePackMajor)
	{
	 DUILIB_RTL_OSVERSIONINFOW osvi;
	 ZeroMemory(&osvi, sizeof(osvi));
	 if (!GetRealOSVersion(&osvi)) return FALSE;

	 // RtlGetVersion 不提供 wServicePackMajor 字段，从 szCSDVersion 文本解析（如 "Service Pack 1"）
	 WORD wServicePack = 0;
	 const WCHAR* pszSP = wcsstr(osvi.szCSDVersion, L"Service Pack");
	 if (pszSP != NULL) {
	  for (const WCHAR* p = pszSP; *p != 0; ++p) {
	   if (*p >= L'0' && *p <= L'9') { wServicePack = (WORD)(*p - L'0'); break; }
	  }
	 }
	 // 语义与原 VerifyVersionInfo(VER_GREATER_EQUAL) 一致：major -> minor -> SP 字典序比较
	 if (osvi.dwMajorVersion != wMajorVersion) return osvi.dwMajorVersion > wMajorVersion;
	 if (osvi.dwMinorVersion != wMinorVersion) return osvi.dwMinorVersion > wMinorVersion;
	 return wServicePack >= wServicePackMajor;
	}

	static BOOL IsWindowsXPOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 0);
	}

	static BOOL IsWindowsXPSP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 1);
	}

	static BOOL IsWindowsXPSP2OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 2);
	}

	static BOOL IsWindowsXPSP3OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINXP), LOBYTE(_WIN32_WINNT_WINXP), 3);
	}

	static BOOL IsWindowsVistaOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 0);
	}

	static BOOL IsWindowsVistaSP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 1);
	}

	static BOOL IsWindowsVistaSP2OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_VISTA), LOBYTE(_WIN32_WINNT_VISTA), 2);
	}

	static BOOL IsWindows7OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 0);
	}

	static BOOL IsWindows7SP1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN7), LOBYTE(_WIN32_WINNT_WIN7), 1);
	}

	static BOOL IsWindows8OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WIN8), LOBYTE(_WIN32_WINNT_WIN8), 0);
	}

	static BOOL IsWindows8Point1OrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINBLUE), LOBYTE(_WIN32_WINNT_WINBLUE), 0);
	}

	static BOOL IsWindowsThresholdOrGreater()
	{
		return IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0);
	}

	// 底层RtlGetVersion 不受manifest虚拟化影响，兜底方案
	typedef NTSTATUS(WINAPI* PRtlGetVersion)(PRTL_OSVERSIONINFOW);
	static inline bool IsWindows10ViaRtl()
	{
		HMODULE hNtdll = GetModuleHandleW(L"ntdll.dll");
		if (!hNtdll) return false;

		PRtlGetVersion fnRtlGetVersion = (PRtlGetVersion)GetProcAddress(hNtdll, "RtlGetVersion");
		if (!fnRtlGetVersion) return false;

		RTL_OSVERSIONINFOW verInfo = { sizeof(verInfo) };
		if (fnRtlGetVersion(&verInfo) != 0)
			return false;

		return verInfo.dwMajorVersion == 10 && verInfo.dwMinorVersion == 0;
	}

	static BOOL IsWindows10OrGreater()
	{
		if (IsWindowsVersionOrGreater(HIBYTE(_WIN32_WINNT_WINTHRESHOLD), LOBYTE(_WIN32_WINNT_WINTHRESHOLD), 0))
		{
			return TRUE;
		}
		return IsWindows10ViaRtl();
	}

	// Windows 11 及以上（Win11 首个版本 21H2 的 build 为 22000；Win10 最高 build 19045）
	static BOOL IsWindows11OrGreater()
	{
	 DUILIB_RTL_OSVERSIONINFOW osvi;
	 ZeroMemory(&osvi, sizeof(osvi));
	 if (!GetRealOSVersion(&osvi)) return FALSE;
	 return (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 22000)
	  || osvi.dwMajorVersion > 10;
	}

	static BOOL IsWindowsServer()
	{
		OSVERSIONINFOEXW osvi = { sizeof(osvi), 0, 0, 0, 0, {0}, 0, 0, 0, VER_NT_WORKSTATION };
		DWORDLONG const dwlConditionMask = VerSetConditionMask(0, VER_PRODUCT_TYPE, VER_EQUAL);
		return !VerifyVersionInfoW(&osvi, VER_PRODUCT_TYPE, dwlConditionMask);
	}
}
#endif // _VERSIONHELPERS_H_INCLUDED_
