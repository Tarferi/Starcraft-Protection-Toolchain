#include "Protection.h"
#include "Desktop.h"
#include "Registry.h"
#include "Process.h"
#include <Windows.h>

#pragma push_macro("CreateDesktop")
#ifdef CreateDesktop
#undef CreateDesktop
#endif

#pragma push_macro("DeleteFile")
#ifdef DeleteFile
#undef DeleteFile
#endif

std::wstring s2ws(const std::string& str);

Protection::Protection(DesktopFactory* df, const char* name) {
	this->df = df;
	this->name = name;
}

Protection::~Protection() {
	LeaveDesktop();
}

static char ProtectionWorkingDirs[4096];

const char* Protection::GetWorkingDir() {
	char szFileName[MAX_PATH];
#ifdef _DEBUG
	sprintf_s(szFileName, "C:\\Users\\Tom\\Documents\\Visual Studio Projects\\ScProtectionToolchain");
#else
	GetModuleFileNameA(NULL, szFileName, MAX_PATH);
	int32 sz = strlen(szFileName);
	for (int32 i = sz - 1; i >= 0; i--) {
		if (szFileName[i] == '\\' || szFileName[i] == '/') {
			szFileName[i] = 0;
			break;
		}
	}
#endif
	ProtectionWorkingDirs[0] = 0;
	sprintf_s(ProtectionWorkingDirs, "%s\\%s\\", szFileName, this->name);
	return ProtectionWorkingDirs;
}

bool Protection::HandleOpenFileDialog(ProcessWindow* wnd, const char* file) {
	if (!FileExists(file)) {
		LOG_ERROR("Input file does not exist");
		return false;
	}

	bool bRet = true;
	wnd->PrintHierarchy(NodeInfo::ClassAndName);
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

bool Protection::HandleOpenFileDialog2(ProcessWindow* wnd, const char* file) {
	if (!FileExists(file)) {
		LOG_ERROR("Input file does not exist");
		return false;
	}

	bool bRet = true;
	
	bRet &= wnd->GetRootNode()->NthChildOfClass("#32770", 0, [&](UINode* wndNode) {
		bRet &= wndNode->NthChildOfClass("ComboBox", 1, [&](UINode* comboNode) {
			bRet &= comboNode->NthChildOfClass("Edit", 0, [&](UINode* editNode) {
				bRet &= editNode->SetText(file);
			});
		});

		bRet &= wndNode->NthChildOfClass("Button", 0, [&](UINode* btnNode) {
			bRet &= btnNode->Click();
		});
	});

	return bRet;
}

static void sSleep(int32 ms) {
	Sleep(ms);
}

void Protection::Sleep(int32 sleepMS) {
	sSleep(sleepMS);
}

bool Protection::HandleSaveFileDialog(ProcessWindow* wnd, const char* file) {
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

bool Protection::HandleSaveFileDialog2(ProcessWindow* wnd, const char* file) {
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

	bRet &= wnd->GetRootNode()->NthChildOfClass("#32770", 0, [&](UINode* wndNode) {
		bRet &= wndNode->NthChildOfClass("ComboBox", 1, [&](UINode* comboNode) {
			bRet &= comboNode->NthChildOfClass("Edit", 0, [&](UINode* editNode) {
				bRet &= editNode->SetText(file);
			});
		});

		bRet &= wndNode->NthChildOfClass("Button", 0, [&](UINode* btnNode) {
			bRet &= btnNode->Click();
		});

	});
	return bRet;
}


bool Protection::EnterDesktop(const char* name) {
	if (d && strcmp(d->GetName(), name)) {
		LeaveDesktop();
	}
	d = df->CreateDesktop(name);
	if (d->Create()) {
		if (d->Enter()) {
			return true;
		}
	}
	LeaveDesktop();
	return false;
}

bool Protection::LeaveDesktop() {
	if (d) {
		d->Leave();
		DELETE_NOT_NULL(d);
	}
	return true;
}

Desktop* Protection::GetDesktop() {
	return d;
}

bool Protection::HasDLL(const char* name) {
	HMODULE h = LoadLibraryA(name);
	if (h == NULL) {
		LOG_ERROR("Failed to load %s", name);
		return false;
	}
	FreeLibrary(h);
	return true;
}

bool Protection::HasSysFile(const char* name, const char* sha1, uint8* data, uint32 dataLength) {
	char tmp [1024];
	GetSystemDirectoryA(tmp, sizeof(tmp) - 3);
	uint32 sz = (uint32)strlen(tmp);
	tmp[sz] = '\\';
	tmp[sz + 1] = 0;
	if (!CheckFile(tmp, name, sha1, data, dataLength)) {
		LOG_ERROR("Failed to check system file");
		return false;
	}

	for (int32 i = sz - 1; i >= 0; i--) {
		if (tmp[i] == '\\' || tmp[i] == '/') {
			char* t = &(tmp[i + 1]);
			memcpy(t, "SysWOW64\\\0", strlen("SysWOW64\\\0") + 1);
			break;
		}
	}

	if (!CheckFile(tmp, name, sha1, data, dataLength)) {
		LOG_ERROR("Failed to check system file");
		return false;
	}

	return true;
}

static char ProtectionPathTmp[4096];

bool Protection::HasDLLClass(const char* name, const char* cls) {
	char tmp[256];
	sprintf_s(tmp, "CLSID\\%s", cls);
	if (Registry::KeyExists(tmp)) {
		return true;
	} else {
		// Run regsvr32
		LOG_INFO("Class %s for file %s is not registered. Running DllRegisterServer on %s", cls, name, name);
		sprintf_s(ProtectionPathTmp, "regsvr32 -s \"%s%s\"", GetWorkingDir(), name);
		Process p(nullptr, ProtectionPathTmp);
		if (p.Run()) {
			int32 res = p.WaitFor();
			Sleep(1000);
			if (!Registry::KeyExists(tmp)) {
				LOG_ERROR("Failed to register %s. Make sure you run this program as administrator", name);
				return false;
			}
			return true;
		} else {
			LOG_ERROR("Failed to run regsvr32");
			return false;
		}
	}
	return false;
}

static char FileEditCheckFile[4096];

bool Protection::CheckFile(const char* root, const char* file, const char* sha1, uint8* data, uint32 dataLength) {
	sprintf_s(FileEditCheckFile, "%s%s", root, file);
	uint8* contents = nullptr;
	uint32 contentsLength = 0;
	bool create = false;
	if (ReadFile(FileEditCheckFile, &contents, &contentsLength)) {
		const char* hash = nullptr;
		if (Sha1(contents, contentsLength, &hash)) {
			if (strcmp(hash, sha1)) {
				LOG_INFO("Sha1 for file %s differs, recreating", file);
				DeleteFile(FileEditCheckFile);
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
		if (!WriteFile(FileEditCheckFile, data, dataLength)) {
			LOG_ERROR("Failed to write file %s to %s", file, root);
			return false;
		}
	}
	return true;
}

#pragma pop_macro("DeleteFile")
#pragma pop_macro("CreateDesktop")