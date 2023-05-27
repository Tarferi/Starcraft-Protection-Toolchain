#include "Desktop.h"
#include <Windows.h>


Desktop::Desktop(Desktop&& another) {
	this->name = another.name;
}

Desktop::Desktop(const char* name) {
	this->name = name;
}

Desktop::~Desktop() {
	
}

const char* Desktop::GetName() {
	return name;
}

VisibleDesktop::VisibleDesktop(const char* name) : Desktop(name) {

}

VisibleDesktop::~VisibleDesktop() {

}

bool VisibleDesktop::Create() {
	return true;
}

bool VisibleDesktop::Enter() {
	return true;
}

bool VisibleDesktop::Leave() {
	return true;
}

const char* VisibleDesktop::GetDesktopNameForCreateProcess() {
	return nullptr;
}


struct HiddenDesktopData {
	HDESK original;
	HDESK hd;
};

HiddenDesktop::HiddenDesktop(const char* name) : Desktop(name) {
	static_assert(sizeof(struct HiddenDesktopData) <= sizeof(buffer), "Invalid buffer size");
	struct HiddenDesktopData* data = reinterpret_cast<struct HiddenDesktopData*>(buffer);
	memset(data, 0, sizeof(struct HiddenDesktopData));
	data->original = GetThreadDesktop(GetCurrentThreadId());
}

HiddenDesktop::~HiddenDesktop() {
	struct HiddenDesktopData* data = reinterpret_cast<struct HiddenDesktopData*>(buffer);
	if (data->hd) {
		CloseDesktop(data->hd);
		memset(data, 0, sizeof(struct HiddenDesktopData));
	}
}

HiddenDesktop::HiddenDesktop(HiddenDesktop&& another) : Desktop(another.GetName()) {
	memcpy(buffer, another.buffer, sizeof(buffer));
}

bool HiddenDesktop::Create() {
	struct HiddenDesktopData* data = reinterpret_cast<struct HiddenDesktopData*>(buffer);
	if (!data->hd) {
		data->hd = OpenDesktopA(GetName(), 0, FALSE, GENERIC_ALL);
		if (!data->hd) {
			data->hd = CreateDesktopA(GetName(), NULL, NULL, 0, GENERIC_ALL, NULL);
			if (!data->hd) {
				LOG_ERROR("Failed to open desktop %s: %d", GetName(), GetLastError());
				return false;
			}
		}
	}
	return true;
}

bool HiddenDesktop::Enter() {
	struct HiddenDesktopData* data = reinterpret_cast<struct HiddenDesktopData*>(buffer);
	if (SetThreadDesktop(data->hd)) {
		return true;
	}
	LOG_ERROR("Failed to enter desktop %s: %d", GetName(), GetLastError());
	return false;
}

bool HiddenDesktop::Leave() {
	struct HiddenDesktopData* data = reinterpret_cast<struct HiddenDesktopData*>(buffer);
	if (SetThreadDesktop(data->original)) {
		return true;
	}
	LOG_ERROR("Failed to leave desktop %s: %d", GetName(), GetLastError());
	return false;
}

const char* HiddenDesktop::GetDesktopNameForCreateProcess() {
	return GetName();
}

#pragma push_macro("CreateDesktop")
#ifdef CreateDesktop
#undef CreateDesktop
#endif

Desktop&& VisibleDesktopFactory::CreateDesktopRef(const char* name) {
	VisibleDesktop desktop(name);
	return std::move(desktop);
}

Desktop* VisibleDesktopFactory::CreateDesktop(const char* name) {
	return new VisibleDesktop(name);
}

Desktop&& HiddenDesktopFactory::CreateDesktopRef(const char* name) {
	HiddenDesktop desktop(name);
	return std::move(desktop);
}

Desktop* HiddenDesktopFactory::CreateDesktop(const char* name) {
	return new HiddenDesktop(name);
}

#pragma pop_macro("CreateDesktop")