#include "ProEdit.h"
#include <Process.h>
#include "bin.h"

ProEdit::ProEdit(DesktopFactory* df, const char* username, const char* email) : Protection(df, "ProEdit") {
	this->username = username;
	this->email = email;
}

ProEdit::~ProEdit() {

}

static char ProEditExec[4096];
static char ProEditCheckFile[4096];

bool ProEdit::CheckFile(const char* root, const char* file, const char* sha1, uint8* data, uint32 dataLength) {
	sprintf_s(ProEditCheckFile, "%s%s", root, file);
	uint8* contents = nullptr;
	uint32 contentsLength = 0;
	bool create = false;
	if (ReadFile(ProEditCheckFile, &contents, &contentsLength)) {
		const char* hash = nullptr;
		if (Sha1(contents, contentsLength, &hash)) {
			if (strcmp(hash, sha1)) {
				LOG_INFO("Sha1 for file %s differs, recreating", file);
				DeleteFile(ProEditCheckFile);
				create = true;
			}
		} else {
			LOG_ERROR("Sha1 failed");
			return false;
		}
	} else {
		create = true;
	}

	if (create) {
		if (!WriteFile(ProEditCheckFile, data, dataLength)) {
			LOG_ERROR("Failed to write file %s", file);
			return false;
		}
	}
	return true;
}

bool ProEdit::CheckFiles(const char* root) {
	if (!DirectoryExists(root)) {
		CreateDirectory(root);
		if (!DirectoryExists(root)) {
			LOG_ERROR("Failed to create directory root %s", root);
			return false;
		}

	}
	sprintf_s(ProEditExec, "%sTemp", root);
	if (!DirectoryExists(ProEditExec)) {
		CreateDirectory(ProEditExec);
		if (!DirectoryExists(ProEditExec)) {
			LOG_ERROR("Failed to create directory Temp");
			return false;
		}
	}
	
	sprintf_s(ProEditExec, "%sTemp\\staredit", root);
	if (!DirectoryExists(ProEditExec)) {
		CreateDirectory(ProEditExec);
		if (!DirectoryExists(ProEditExec)) {
			LOG_ERROR("Failed to create directory Temp\\staredit");
			return false;
		}
	}

	bool bRet = true;
	bRet &= CheckFile(root, "Comdlg32.ocx", "9bc145b54500fb6fbea9be61fbdd90f65fd1bc14", (uint8*)comdlg_ocx, comdlg_ocx_size);
	bRet &= CheckFile(root, "MpqCtl.ocx", "1e76e6d69afb9224d265a337fe9eb8d7e207197f", (uint8*)mpqctl_ocx, mpqctl_ocx_size);
	bRet &= CheckFile(root, "MSCOMCTL.OCX", "b3f80b06ad6283fc021de1682772c22dd6f2436b", (uint8*)mscomctl_ocx, mscomctl_ocx_size);
	bRet &= CheckFile(root, "PROEdit.exe", "974677ca1bf421b8a73d32f1d96d035547da65e7", (uint8*)proedit_exe, proedit_exe_size);
	bRet &= CheckFile(root, "PROEdit.ini", "fde48ccfd6ead6aa6560c48e03e93bb19651177d", (uint8*)proedit_ini, proedit_ini_size);
	bRet &= CheckFile(root, "richtx32.ocx", "90fec763edfb0b0924700be6b914292c591a152c", (uint8*)richtx32_ocx, richtx32_ocx_size);
	bRet &= CheckFile(root, "UnitList.lst", "eeb2239965690dbe7b78396d315df5e7791a1c7b", (uint8*)unitlist_lst, unitlist_lst_size);
	return bRet;
}

bool ProEdit::Check() {
	bool bRet = true;
	if (CheckFiles(GetWorkingDir())) {
		bRet &= HasDLLClass("Mscomtlc.ocx", "{DD9DA666-8594-11D1-B16A-00C0F0283628}");
		bRet &= HasDLLClass("Comdlg32.OCX", "{F9043C85-F6F2-101A-A3C9-08002B2F49FB}");
		bRet &= HasDLLClass("MpqCtl.ocx", "{DA729166-C84F-11D4-A9EA-00A0C9199875}");
		bRet &= HasDLLClass("richtx32.ocx", "{B617B991-A767-4F05-99BA-AC6FCABB102E}");
	} else {
		LOG_ERROR("Files not valid");
		bRet = false;
	}
	return bRet;
}

bool HandleOpenFileDialog(ProcessWindow* wnd, const char* file) {
	if (!FileExists(file)) {
		LOG_ERROR("Input file does not exist");
		return false;
	}
	
	bool bRet = true;
	bRet &= wnd->GetRootNode()->NthChildOfClass("ComboBox", 0, [&](UINode* comboNode) {
		bRet &= comboNode->NthChildOfClass("Edit", 0, [&](UINode* editNode) {
			bRet &= editNode->SetText(file);
		});
	});

	bRet &= wnd->GetRootNode()->NthChildOfClass("Button", 0, [&](UINode* btnNode) {
		bRet &= btnNode->Click();
	});
		
	return bRet;
}

bool HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file) {
	if (!wnd->ClickElement("File Operations/Open map")) {
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

bool HandleMapProtect(Process* p, ProcessWindow* wnd, const char* username, const char* email) {
	bool bRet = true;
	if (!wnd->NodeForPath("Protection/Protect", [&](UINode* node) {
		bRet &= node->Click();
	})) {
		LOG_ERROR("Failed to find protect button");
		return false;
	}

	ProcessWindow* pwnd = p->WaitForWindow("Protection data", 5000);
	DelGuard(pwnd);
	if (!pwnd) {
		LOG_ERROR("Protection data window not found");
		return false;
	}

	bRet &= pwnd->GetRootNode()->NthChildOfClass("ThunderRT6PictureBoxDC", 1, [&](UINode* dcNode) {
		bRet &= dcNode->NthChildOfClass("ThunderRT6TextBox", 0, [&](UINode* textNode) {
			textNode->SetText(username);
		});
	});

	bRet &= pwnd->GetRootNode()->NthChildOfClass("ThunderRT6PictureBoxDC", 0, [&](UINode* dcNode) {
		bRet &= dcNode->NthChildOfClass("ThunderRT6TextBox", 0, [&](UINode* textNode) {
			textNode->SetText(email);
		});
	});
	
	if (!pwnd->NodeForPath("Protect", [&](UINode* node) {
		bRet &= node->Click();
	})) {
		LOG_ERROR("Failed to find protect button");
		return false;
	}

	if (!p->WaitForWindowClose("Protection data", 5000)) {
		LOG_ERROR("Window for protection data did not close");
		return false;
	}
	return bRet;
}

bool HandleSaveFileDialog(ProcessWindow* wnd, const char* file) {
	bool bRet = true;
	if (FileExists(file)) {
		if (!DeleteFile(file)) {
			LOG_ERROR("Could not delete output file");
			return false;
		}
	}

	if (FileExists(file)) {
		LOG_ERROR("Could not delete output file");
		return false;
	}

	bRet &= wnd->GetRootNode()->NthChildOfClass("DUIViewWndClassName", 0, [&](UINode* dclNode) {
		bRet &= dclNode->NthChildOfClass("AppControlHost", 0, [&](UINode* comboNode) {
			bRet &= comboNode->NthChildOfClass("Edit", 0, [&](UINode* editNode) {
				bRet &= editNode->SetText(file);
			});
		});
	});

	bRet &= wnd->GetRootNode()->NthChildOfClass("Button", 0, [&](UINode* btnNode) {
		bRet &= btnNode->Click();
	});
		
	return bRet;
}

bool HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file) {
	if (!wnd->ClickElement("File Operations/Save map")) {
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

bool ProEdit::Protect(char* input, char* output) {
	LOG_INFO("Entering virtual desktop... ");
	if (!EnterDesktop("ProEditDesktop")) {
		LOG_ERROR("Failed to enter desktop");
		return false;
	}

	sprintf_s(ProEditExec, "%s%s", GetWorkingDir(), "Proedit.exe");
	Process p(GetDesktop(), ProEditExec);

	LOG_INFO("Running proedit... ");
	if (!p.Run()) {
		return false;
	}

	ProcessWindow* wnd = p.WaitForWindow("PROEdit v.1.4.1  [NothingOpened]", 5000);
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

	if (!HandleMapProtect(&p, wnd, username, email)) {
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