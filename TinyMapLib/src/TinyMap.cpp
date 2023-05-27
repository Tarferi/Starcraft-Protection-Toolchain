#include "TinyMap.h"
#include "bin.h"

#include <Process.h>

TinyMap::TinyMap(DesktopFactory* df) : Protection(df, "TinyMap") {

}

TinyMap::~TinyMap() {

}

static char TinyMapEditExec[4096];

bool TinyMap::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}

	}
	sprintf_s(TinyMapEditExec, "%sTemp", root);
	if (!DirectoryExists(TinyMapEditExec)) {
		CreateDirectory(TinyMapEditExec);
		if (!DirectoryExists(TinyMapEditExec)) {
			LOG_ERROR("Failed to create directory Temp");
			return false;
		}
	}

	sprintf_s(TinyMapEditExec, "%sTemp\\staredit", root);
	if (!DirectoryExists(TinyMapEditExec)) {
		CreateDirectory(TinyMapEditExec);
		if (!DirectoryExists(TinyMapEditExec)) {
			LOG_ERROR("Failed to create directory Temp\\staredit");
			return false;
		}
	}

	bool bRet = true;
	bRet &= CheckFile(root, "TinyMap2.exe", "fb793ddcb7fea6cccd4cf55e99c6ecdeec5da299", (uint8*)tinymap2_exe, tinymap2_exe_size);
	bRet &= CheckFile(root, "TinyMap2.ini", "a47ea3967dffbf6a125ae61582f05aaba210bca6", (uint8*)tinymap2_ini, tinymap2_ini_size);
	return bRet;
}

bool TinyMap::Check() {
	bool bRet = true;
	if (CheckFiles(GetWorkingDir())) {
	} else {
		LOG_ERROR("Files not valid");
		bRet = false;
	}
	return bRet;
}


bool TinyMap::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("ProEditDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(TinyMapEditExec, "%s%s", GetWorkingDir(), "TinyMap2.exe");
	Process p(GetDesktop(), TinyMapEditExec);

	LOG_INFO("Running tinymap... ");
	if (!p.Run()) {
		return false;
	}
	
	/*
	p.IterWindows([&](ProcessWindow* wnd) {
		LOG_ERROR("%s", wnd->GetWindowName());
	});
	*/

	ProcessWindow* wnd = p.WaitForWindow("TinyMap v2.1.3", 5000);
	DelGuard(wnd);
	if (!wnd) {
		LOG_ERROR("Window for proedit was not created");
		return false;
	}

	LOG_INFO("Opening file open dialog... ");

	if (!HandleOpenFile(&p, wnd, input)) {
		LOG_ERROR("Failed to open file");
		return false;
	}

	LOG_INFO("Performing protections... ");

	if (!HandleMapCompress(&p, wnd)) {
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


bool TinyMap::HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {

	if (!wnd->ClickElement("Open Map")) {
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

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}

	return true;
}

bool TinyMap::HandleMapCompress(Process* p, ProcessWindow* wnd) {
	/*
	if (!wnd->ClickElement("Open Map")) {
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

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}
	*/
	return true;
}

bool TinyMap::HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	if (!wnd->ClickElement("Save Map")) {
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

	if (!p->WaitForWindowClassClose("#32770", 5000)) {
		LOG_ERROR("Open file dialog failed to close");
		return false;
	}
	return true;
}