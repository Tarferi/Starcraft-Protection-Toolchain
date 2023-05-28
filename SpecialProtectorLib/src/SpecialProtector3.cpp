#include "SpecialProtector3.h"
#include "bin.h"

#include <Process.h>

SpecialProtector3::SpecialProtector3(DesktopFactory* df) : Protection(df, "SpecialProtector") {

}

SpecialProtector3::~SpecialProtector3() {

}

static char SpecialProtector3EditExec[4096];

bool SpecialProtector3::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}

	}

	bool bRet = true;
	bRet &= CheckFile(root, "comdlg32.ocx", "eb49323be4384a0e7e36053f186b305636e82887", (uint8*)comdlg32_ocx, comdlg32_ocx_size);
	bRet &= CheckFile(root, "MpqCtl.ocx", "1e76e6d69afb9224d265a337fe9eb8d7e207197f", (uint8*)mpqctl_ocx, mpqctl_ocx_size);
	bRet &= CheckFile(root, "msinet.ocx", "bfd4c0d56391b0ecdb8bd048d08efe7372852f45", (uint8*)msinet_ocx, msinet_ocx_size);
	bRet &= CheckFile(root, "Special Protector2.exe", "4521a5b50363687b94f72339ddc64ad5c8050941", (uint8*)special_protector2_exe, special_protector2_exe_size);
	bRet &= CheckFile(root, "Specialprotector3.exe", "10c44ccd308776b0f956bec9ba26f1d038b79e53", (uint8*)special_protector3_exe, special_protector3_exe_size);
	bRet &= CheckFile(root, "vb6ko.dll", "d5f4a8e686c441a5bca4d20f31297cadd017301a", (uint8*)vb6ko_dll, vb6ko_dll_size);
	return bRet;
}

bool SpecialProtector3::Check() {
	bool bRet = true;
	if (CheckFiles(GetWorkingDir())) {
		bRet &= HasDLLClass("Comdlg32.OCX", "{F9043C85-F6F2-101A-A3C9-08002B2F49FB}");
		bRet &= HasDLLClass("MpqCtl.ocx", "{DA729166-C84F-11D4-A9EA-00A0C9199875}");
		bRet &= HasDLLClass("msinet.ocx", "{48E59295-9880-11CF-9754-00AA00C00908}");
		bRet &= HasSysFile("vb6ko.dll", "d5f4a8e686c441a5bca4d20f31297cadd017301a", (uint8*)vb6ko_dll, vb6ko_dll_size);
	} else {
		LOG_ERROR("Files not valid");
		bRet = false;
	}
	return bRet;
}

bool SpecialProtector3::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("SpecialProtectorDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(SpecialProtector3EditExec, "%s%s", GetWorkingDir(), "SpecialProtector3.exe");
	Process p(GetDesktop(), SpecialProtector3EditExec);

	LOG_INFO("Running special protector... ");
	if (!p.Run()) {
		return false;
	}

	ProcessWindow* wnd = p.WaitForWindowClass("ThunderRT6FormDC", 5000);
	DelGuard(wnd);
	if (!wnd) {
		LOG_ERROR("Window for special protector was not created");
		return false;
	}

	LOG_INFO("Opening file open dialog... ");

	if (!HandleOpenFile(&p, wnd, input)) {
		LOG_ERROR("Failed to open file");
		return false;
	}

	LOG_INFO("Performing protections... ");

	if (!HandleMapProtect(&p, wnd)) {
		LOG_ERROR("Failed to protect the map");
		return false;
	}

	LOG_INFO("Opening file save dialog... ");

	if (!HandleSaveFile(&p, wnd, output)) {
		LOG_ERROR("Failed to save file");
		return false;
	}
	Sleep(100);

	return true;
}

bool SpecialProtector3::HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {
	bool bRet = true;
	bRet &= wnd->GetRootNode()->NthChildOfClass("ThunderRT6UserControlDC", 7, [&](UINode* btnOpen) {
		bRet &= btnOpen->Click();
	});
	ProcessWindow* wndOpen = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndOpen);
	if (!wndOpen) {
		LOG_ERROR("Window for file open was not found");
		return false;
	}

	if (!HandleOpenFileDialog(wndOpen, file)) {
		LOG_ERROR("Failed to handle open file dialog");
		return false;
	}

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	return bRet;
}

bool SpecialProtector3::HandleMapProtect(Process* p, ProcessWindow* wnd) {
	bool bRet = true;
	bRet &= wnd->GetRootNode()->NthChildOfClass("ThunderRT6UserControlDC", 6, [&](UINode* btnOpen) {
		bRet &= btnOpen->Click();
	});
	return bRet;
}

bool SpecialProtector3::HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	bool bRet = true;
	
	bRet &= wnd->GetRootNode()->NthChildOfClass("ThunderRT6UserControlDC", 4, [&](UINode* btnOpen) {
		bRet &= btnOpen->Click();
	});

	ProcessWindow* wndOpen = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndOpen);
	if (!wndOpen) {
		LOG_ERROR("Window for file open was not found");
		return false;
	}

	if (!HandleSaveFileDialog(wndOpen, file)) {
		LOG_ERROR("Failed to handle open file dialog");
		return false;
	}

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}
	
	return bRet;
}