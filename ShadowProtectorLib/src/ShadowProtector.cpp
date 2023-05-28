#include "ShadowProtector.h"
#include <Process.h>
#include "bin.h"

ShadowProtector::ShadowProtector(DesktopFactory* df) : Protection(df, "ShadowProtector") {}

ShadowProtector::~ShadowProtector() {

}

static char ShadowProtectorExec[4096];

bool ShadowProtector::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}
	}

	bool bRet = true;
	bRet &= CheckFile(root, "Shadow Protector.exe", "6cbfb793993aba89b377b0e4994a0ec836b8b056", (uint8*)shadow_protector_exe, shadow_protector_exe_size);
	bRet &= CheckFile(root, "Shadow Protector.ini", "d388214d9b3ffbc6ddab29938a1878955835f8ce", (uint8*)shadow_protector_ini, shadow_protector_ini_size);
	return bRet;
}

bool ShadowProtector::Check() {
	bool bRet = true;
	if (CheckFiles(GetWorkingDir())) {
	} else {
		LOG_ERROR("Files not valid");
		bRet = false;
	}
	return bRet;
}

bool ShadowProtector::HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {
	Sleep(500);
	bool bRet = true;

	bRet &= wnd->GetRootNode()->NthChildOfClass("Button", 1, [&](UINode* node) {
		bRet &= node->Click();
	});

	ProcessWindow* wndOpen = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndOpen);
	if (!wndOpen) {
		LOG_ERROR("Window for file open was not found");
		return false;
	}

	if (!HandleOpenFileDialog2(wndOpen, file)) {
		LOG_ERROR("Failed to handle open file dialog");
		return false;
	}

	Sleep(500);

	// TODO: Wait for child, verify status icon of the dialog

	bRet &= wndOpen->GetRootNode()->NthChildOfClass("#32770", 0, [&](UINode* confirmDlg) {
		bRet &= confirmDlg->NthChildOfClass("Button", 0, [&](UINode* btnClose) {
			bRet &= btnClose->Click();
		});
	});

	Sleep(500);

	return bRet;
}

bool ShadowProtector::HandleMapProtect(Process* p, ProcessWindow* wnd) {
	Sleep(250);
	bool bRet = true;

	bRet &= wnd->GetRootNode()->NthChildOfClass("Button", 4, [&](UINode* node) {
		bRet &= node->Click();
	});

	Sleep(500);

	// TODO: Wait for child, verify status icon of the dialog

	bRet &= wnd->GetRootNode()->NthChildOfClass("#32770", 0, [&](UINode* confirmDlg) {
		bRet &= confirmDlg->NthChildOfClass("Button", 0, [&](UINode* btnClose) {
			bRet &= btnClose->Click();
		});
	});

	Sleep(500);

	return bRet;
}

bool ShadowProtector::HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	Sleep(250);
	bool bRet = true;

	bRet &= wnd->GetRootNode()->NthChildOfClass("Button", 2, [&](UINode* node) {
		bRet &= node->Click();
	});

	ProcessWindow* wndOpen = p->WaitForWindowClass("#32770", 5000);
	DelGuard(wndOpen);
	if (!wndOpen) {
		LOG_ERROR("Window for file save was not found");
		return false;
	}

	if (!HandleSaveFileDialog2(wndOpen, file)) {
		LOG_ERROR("Failed to handle save file dialog");
		return false;
	}

	Sleep(500);

	// TODO: Wait for child, verify status icon of the dialog

	bRet &= wndOpen->GetRootNode()->NthChildOfClass("#32770", 0, [&](UINode* confirmDlg) {
		bRet &= confirmDlg->NthChildOfClass("Button", 0, [&](UINode* btnClose) {
			bRet &= btnClose->Click();
		});
	});

	Sleep(500);
	
	return bRet;
}

bool ShadowProtector::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("ShadowProtectorDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(ShadowProtectorExec, "%s%s", GetWorkingDir(), "Shadow Protector.exe");
	Process p(GetDesktop(), ShadowProtectorExec);

	LOG_INFO("Running Shadow Protector... ");
	if (!p.Run()) {
		return false;
	}

	ProcessWindow* wnd = p.WaitForWindow("Shadow Protector v1.5", 5000);
	DelGuard(wnd);
	if (!wnd) {
		LOG_ERROR("Window for Shadow Protector was not created");
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