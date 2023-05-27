#include "common.h"
#include <Windows.h>

#pragma push_macro("CreateDirectory")
#ifdef CreateDirectory
#undef CreateDirectory
#endif

#pragma push_macro("DeleteFile")
#ifdef DeleteFile
#undef DeleteFile
#endif

const char* stripProjectPath(const char* path) {
	for (uint32 i = 0; path[i] != '\0'; i++) {
		if ((path[i] == '\\' || path[i] == '/') && path[i + 1] != '\0') {
			if (path[i + 1] == 's' && path[i + 2] != '\0') {
				if (path[i + 2] == 'r' && path[i + 3] != '\0') {
					if (path[i + 3] == 'c' && path[i + 4] != '\0') {
						if ((path[i + 4] == '\\' || path[i + 4] == '/') && path[i + 5] != '\0') {
							return &(path[i + 5]);
						}
					}
				}
			}
		}
	}
	return path;
}

void log_error(char* buffer, const char* fileName, uint32 lineNumber) {
	Logger::LogError(fileName, lineNumber, buffer);
}

void log_info(char* buffer, const char* fileName, uint32 lineNumber) {
	Logger::LogInfo(fileName, lineNumber, buffer);
}

bool FileExists(const char* path) {
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool DirectoryExists(const char* path) {
	DWORD dwAttrib = GetFileAttributesA(path);

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

bool CreateDirectory(const char* path) {
	return CreateDirectoryA(path, NULL);
}

bool DeleteFile(const char* path) {
	return DeleteFileA(path);
}

void Logger::LogError(const char* fileName, uint32 lineNumber, const char* buffer) {
	fprintf(stderr, "[ERROR]%s:%d - %s", fileName, lineNumber, buffer);
}

void Logger::LogInfo(const char* fileName, uint32 lineNumber, const char* buffer) {
	fprintf(stdout, "[INFO] %s:%d - %s", fileName, lineNumber, buffer);
}

static char CommonSha1[128];

#define MIN(a,b) (a<b?a:b)

bool Sha1(uint8* data, uint32 dataSize, const char** lowerCaseHash) {
	const uint32 BUFSIZE = 1024;
	const uint32 MD5LEN = 20;
	HCRYPTPROV hProv = 0;
	HCRYPTHASH hHash = 0;
	BYTE rgbHash[MD5LEN];
	DWORD cbHash = 0;
	CHAR rgbDigits[] = "0123456789abcdef";
	*lowerCaseHash = nullptr;

	// Get handle to the crypto provider
	if (!CryptAcquireContext(&hProv, NULL, NULL, PROV_RSA_FULL, CRYPT_VERIFYCONTEXT)) {
		return false;
	}

	if (!CryptCreateHash(hProv, CALG_SHA1, 0, 0, &hHash)) {
		CryptReleaseContext(hProv, 0);
		return false;
	}

	int32 remaining = dataSize;
	int32 pos = 0;
	while (remaining > 0) {
		int32 readNow = MIN(remaining, BUFSIZE);
		uint8* bufNow = &(data[pos]);
		pos += readNow;
		remaining -= readNow;

		if (!CryptHashData(hHash, (const BYTE*)bufNow, (DWORD)readNow, 0)) {
			CryptReleaseContext(hProv, 0);
			CryptDestroyHash(hHash);
			return false;
		}
	}

	cbHash = MD5LEN;
	if (CryptGetHashParam(hHash, HP_HASHVAL, rgbHash, &cbHash, 0)) {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		int32 pos = 0;
		for (DWORD i = 0; i < cbHash; i++) {
			mchar tmp[64];
			sprintf_s(tmp, "%c%c", rgbDigits[rgbHash[i] >> 4], rgbDigits[rgbHash[i] & 0xf]);
			char* ap = &(CommonSha1[pos]);
			sprintf_s(ap, sizeof(CommonSha1) - pos, "%s", tmp);
			pos += 2;
		}
	} else {
		CryptDestroyHash(hHash);
		CryptReleaseContext(hProv, 0);
		return false;
	}
	*lowerCaseHash = CommonSha1;
	return true;
}

bool ReadFile(const char* path, uint8** data, uint32* dataLength) {
	FILE* f = nullptr;
	if (!fopen_s(&f, path, "rb")) {
		std::vector<uint8> vec;
		uint8 buffer[256];
		
		while (true) {
			int32 read = fread(buffer, 1, sizeof(buffer), f);
			if (read <= 0) {
				break;
			} else {
				for (int32 i = 0; i < read; i++) {
					vec.push_back(buffer[i]);
				}
			}
		}
		fclose(f);

		MALLOC_N(nb, uint8, vec.size(), { return false; });
		uint8* raw = &(vec.at(0));
		memcpy(nb, raw, vec.size());

		*data = nb;
		*dataLength = vec.size();
		return true;
	}
	return false;
}

bool WriteFile(const char* path, uint8* data, uint32 dataLength) {
	FILE* f = nullptr;
	if (!fopen_s(&f, path, "wb")) {
		uint32 written = 0;
		while (written != dataLength) {
			int32 writtenNow = fwrite(&(data[written]), 1, dataLength - written, f);
			if (writtenNow <= 0) {
				fclose(f);
				LOG_ERROR("Failed to write file %s", path);
				return false;
			} else {
				written += writtenNow;
			}
		}
		fclose(f);
		return true;
	}
	return false;
}

#pragma pop_macro("DeleteFile")
#pragma pop_macro("CreateDirectory")
