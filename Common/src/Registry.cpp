#include "Registry.h"
#include <Windows.h>

bool Registry::KeyExists(const char* key) {
	HKEY subKey = nullptr;
	LONG result = RegOpenKeyExA(HKEY_CLASSES_ROOT, key, 0, KEY_READ, &subKey);
	if (result == ERROR_SUCCESS) {
		return true;
	}
	return false;
}