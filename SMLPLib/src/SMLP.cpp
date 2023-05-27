#include "SMLP.h"
#include <Process.h>
#include "bin.h"

SMLP::SMLP(DesktopFactory* df) : Protection(df, "SMLP") {
}

SMLP::~SMLP() {

}

static char SMLPExec[4096];

bool SMLP::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}
	}

	bool bRet = true;
	bRet &= CheckFile(root, "StarcraftMapLockerProject-2.5.00.exe", "21d136252a7f0fac53957a3820c1f0b98d666894", (uint8*)smlp_exe, smlp_exe_size);
	return bRet;
}

bool SMLP::Check() {
	bool bRet = true;
	if (CheckFiles(GetWorkingDir())) {
	} else {
		LOG_ERROR("Files not valid");
		bRet = false;
	}
	return bRet;
}

bool SMLP::HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {
	Sleep(500);
	if (!wnd->ClickElement("Menu/Open Map")) {
		LOG_ERROR("Failed to click on opet map button");
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

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	return true;
}

bool SMLP::HandleMapProtect(Process* p, ProcessWindow* wnd) {
	Sleep(250);
	bool bRet = true;
	if (!wnd->NodeForPath("Menu/Protect", [&](UINode* node) {
		bRet &= node->Click();
	})) {
		LOG_ERROR("Failed to find protect button");
		return false;
	}

	return true;
}

bool SMLP::HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	Sleep(250);
	if (!wnd->ClickElement("Menu/Save Map")) {
		LOG_ERROR("Failed to click on opet map button");
		return false;
	}

	ProcessWindow* wndOpen = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndOpen);
	if (!wndOpen) {
		LOG_ERROR("Window for file save was not found");
		return false;
	}

	if (!HandleSaveFileDialog(wndOpen, file)) {
		LOG_ERROR("Failed to handle save file dialog");
		return false;
	}

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	return true;
}

bool SMLP::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("SMLPDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(SMLPExec, "%s%s", GetWorkingDir(), "StarcraftMapLockerProject-2.5.00.exe");
	Process p(GetDesktop(), SMLPExec);

	LOG_INFO("Running SMLP... ");
	if (!p.Run()) {
		return false;
	}

	ProcessWindow* wnd = p.WaitForWindow("Starcraft Map Locker Project", 5000);
	DelGuard(wnd);
	if (!wnd) {
		LOG_ERROR("Window for SMLP was not created");
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