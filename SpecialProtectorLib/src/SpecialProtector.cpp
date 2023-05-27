#include "SpecialProtector.h"
#include "bin.h"

#include <Process.h>

SpecialProtector::SpecialProtector(DesktopFactory* df) : Protection(df, "SpecialProtector") {

}

SpecialProtector::~SpecialProtector() {

}

static char SpecialProtectorEditExec[4096];

bool SpecialProtector::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}

	}
	sprintf_s(SpecialProtectorEditExec, "%sTemp", root);
	if (!DirectoryExists(SpecialProtectorEditExec)) {
		CreateDirectory(SpecialProtectorEditExec);
		if (!DirectoryExists(SpecialProtectorEditExec)) {
			LOG_ERROR("Failed to create directory Temp");
			return false;
		}
	}

	sprintf_s(SpecialProtectorEditExec, "%sTemp\\staredit", root);
	if (!DirectoryExists(SpecialProtectorEditExec)) {
		CreateDirectory(SpecialProtectorEditExec);
		if (!DirectoryExists(SpecialProtectorEditExec)) {
			LOG_ERROR("Failed to create directory Temp\\staredit");
			return false;
		}
	}

	bool bRet = true;
	bRet &= CheckFile(root, "comdlg32.ocx", "eb49323be4384a0e7e36053f186b305636e82887", (uint8*)comdlg32_ocx, comdlg32_ocx_size);
	bRet &= CheckFile(root, "MpqCtl.ocx", "1e76e6d69afb9224d265a337fe9eb8d7e207197f", (uint8*)mpqctl_ocx, mpqctl_ocx_size);
	bRet &= CheckFile(root, "msinet.ocx", "bfd4c0d56391b0ecdb8bd048d08efe7372852f45", (uint8*)msinet_ocx, msinet_ocx_size);
	bRet &= CheckFile(root, "Special Protector2.exe", "4521a5b50363687b94f72339ddc64ad5c8050941", (uint8*)special_protector2_exe, special_protector2_exe_size);
	bRet &= CheckFile(root, "vb6ko.dll", "d5f4a8e686c441a5bca4d20f31297cadd017301a", (uint8*)vb6ko_dll, vb6ko_dll_size);
	return bRet;
}

bool SpecialProtector::Check() {
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

bool SpecialProtector::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("SpecialProtectorDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(SpecialProtectorEditExec, "%s%s", GetWorkingDir(), "Special Protector2.exe");
	Process p(GetDesktop(), SpecialProtectorEditExec);

	LOG_INFO("Running special protector... ");
	if (!p.Run()) {
		return false;
	}

	ProcessWindow* wnd = p.WaitForWindow("Special Protector2 v 1.4", 5000);
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

	return true;
}

bool SpecialProtector::HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {

	if (!wnd->ClickElement("File Option/Open")) {
		LOG_ERROR("Failed to click on open map");
		return false;
	}

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

	if (!p->WaitForWindowClassClose("#32770", 5000, wndOpen)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	ProcessWindow* wndResult = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndResult);
	if (!wndResult) {
		LOG_ERROR("Map open dialog not shown");
		return false;
	}
	bool bRet = true;
	bRet &= wndResult->GetRootNode()->NthChildOfClass("Static", 1, [&](UINode* node) {
		if (strcmp(node->GetName(), "Success")) {
			LOG_ERROR("Failed to open map: %s", node->GetName());
			bRet = false;
		}
	});

	bRet &= wndResult->GetRootNode()->NthChildOfClass("Button", 0, [&](UINode* node) {
		node->Click();
	});

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}
	return bRet;
}

bool SpecialProtector::HandleMapProtect(Process* p, ProcessWindow* wnd) {
	if (!wnd->ClickElement("Protection/Special")) {
		LOG_ERROR("Failed to click on protect");
		return false;
	}

	ProcessWindow* wndResult = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndResult);
	if (!wndResult) {
		LOG_ERROR("Map protection result not shown");
		return false;
	}
	bool bRet = true;
	bRet &= wndResult->GetRootNode()->NthChildOfClass("Static", 1, [&](UINode* node) {
		if (strcmp(node->GetName(), "Success")) {
			LOG_ERROR("Failed to protect map: %s", node->GetName());
			bRet = false;
		}
	});

	bRet &= wndResult->GetRootNode()->NthChildOfClass("Button", 0, [&](UINode* node) {
		node->Click();
	});

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Protection dialog failed to close");
		return false;
	}
	return bRet;
}

bool SpecialProtector::HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	if (!wnd->ClickElement("File Option/Save")) {
		LOG_ERROR("Failed to click on open map");
		return false;
	}

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

	if (!p->WaitForWindowClassClose("#32770", 5000, wndOpen)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	ProcessWindow* wndResult = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndResult);
	if (!wndResult) {
		LOG_ERROR("Map open dialog not shown");
		return false;
	}
	bool bRet = true;
	bRet &= wndResult->GetRootNode()->NthChildOfClass("Static", 1, [&](UINode* node) {
		if (strcmp(node->GetName(), "Success")) {
			LOG_ERROR("Failed to open map: %s", node->GetName());
			bRet = false;
		}
	});

	bRet &= wndResult->GetRootNode()->NthChildOfClass("Button", 0, [&](UINode* node) {
		node->Click();
	});

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}
	return bRet;
}