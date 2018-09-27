#include "winnotifier.hpp"
#include <QThread>
#include <QTimer>
#include <QLibrary>
#include <tchar.h>
#include <iostream>

namespace {
	constexpr int DelayInterval = 200;
	void LaunchProcess(const QString& procName) {
		STARTUPINFO lpStartupInfo;
		GetStartupInfo(&lpStartupInfo);
		PROCESS_INFORMATION lpProcessInformation;
		if(!CreateProcess(
				TEXT(procName.toStdString().c_str()),
				0, 0, 0, TRUE, NORMAL_PRIORITY_CLASS,
				0, 0, &lpStartupInfo, &lpProcessInformation
			)
		)
			throw std::runtime_error(QString("Failed to launch %1").arg(procName).toStdString());
	}
	bool IsWow64() noexcept {
		using LPFN_ISWOW64PROCESS = BOOL (WINAPI *)(HANDLE, PBOOL);
		// IsWow64Process関数の有無を調査
		const auto fnIsWow64Process = reinterpret_cast<LPFN_ISWOW64PROCESS>(
			GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process")
		);
		if(fnIsWow64Process) {
			// 呼び出して確認
			BOOL isWow64 = FALSE;
			return fnIsWow64Process(GetCurrentProcess(), &isWow64) && isWow64;
		}
		// 無しの場合は32bit環境
		return false;
	}
	namespace bit32 {
		#define BITVERSION 32
		#include "ghook/names_impl.hpp"
		#undef BITVERSION
	}
	namespace bit64 {
		#define BITVERSION 64
		#include "ghook/names_impl.hpp"
		#undef BITVERSION
	}
}

WinNotifier::WinNotifier(QObject* parent):
	QObject(parent),
	_th(nullptr),
	_event32(bit32::name::Event),
	_event64(bit64::name::Event)
{
	if(IsWow64()) {
		std::cout << "Launch watch64.exe..." << std::endl;
		LaunchProcess("64/watcher64.exe");
	}
	std::cout << "Launch watch32.exe..." << std::endl;
	LaunchProcess("watcher32.exe");

	_th = new QThread(this);
	_th->start();
	QTimer* tim = new QTimer;
	connect(tim, &QTimer::timeout, tim, [this](){
		HANDLE hdl[2] = {
			_event32.ref(),
			_event64.ref()
		};
		const DWORD ret = WaitForMultipleObjects(
			sizeof(hdl)/sizeof(hdl[0]),
			hdl,
			FALSE,
			INFINITE
		);
		if(ret == WAIT_TIMEOUT ||
			ret == WAIT_FAILED)
			throw std::runtime_error("error in timer");

		QMetaObject::invokeMethod(this, "onNotify");
	});
	tim->moveToThread(_th);
	QMetaObject::invokeMethod(tim, "start", Q_ARG(int, 0));

	_timer = new QTimer(this);
	_timer->setInterval(DelayInterval);
	connect(_timer, &QTimer::timeout, this, [this](){
		_timer->stop();
		emit onEvent();
	});
}
void WinNotifier::onNotify() {
	_timer->start();
}

WinNotifier::~WinNotifier() {
	_th->quit();
	_event32.signal();
	_event64.signal();
	_th->wait();

	const auto closeWindow = [](LPCTSTR cname){
		const HWND hw = FindWindow(cname, NULL);
		if(hw)
			SendMessage(hw, WM_CLOSE, 0, 0);
	};
	closeWindow(bit32::name::WndClass);
	closeWindow(bit64::name::WndClass);
}
