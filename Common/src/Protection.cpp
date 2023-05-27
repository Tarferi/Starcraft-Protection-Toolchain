#include "Protection.h"
#include "Desktop.h"
#include "Registry.h"
#include "Process.h"
#include <Windows.h>

#pragma push_macro("CreateDesktop")
#ifdef CreateDesktop
#undef CreateDesktop
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
	sprintf_s(ProtectionWorkingDirs, "%s\\ProEdit\\", szFileName);
	return ProtectionWorkingDirs;
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

#pragma pop_macro("CreateDesktop")