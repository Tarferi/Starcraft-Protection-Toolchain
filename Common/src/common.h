#pragma once
#include <stdio.h>
#include <string>
#include <vector>
#include "types.h"


#define GET_CLONED_STRING(newVarName, originalName, onFail)                                 \
	decltype(originalName) newVarName = nullptr;                                            \
	{                                                                                       \
		uint32 sz = (uint32)strlen(originalName);                                           \
		newVarName = (decltype(newVarName))malloc(((int32)sz + 1) * sizeof(const char));   \
		if(newVarName) {                                                                    \
			memcpy((void*)newVarName, (void*)originalName, (int32)sz + 1);                  \
		} else {                                                                            \
			onFail														                    \
		}                                                                                   \
	}

#define MALLOC(varName, type, size, OnFail)       \
	varName = (type*)malloc(sizeof(type)*(size)); \
	if(!varName) {                                \
		OnFail                                    \
	}

#define MALLOC_N(newVarName, type, size, OnFail) \
	type* newVarName = nullptr;                  \
	MALLOC(newVarName, type, size, OnFail)

#define DELETE_NOT_NULL(x) if(x) {delete x; x = nullptr;}

#define sprintf_error sprintf_s
#define LOG_ERROR(fmt, ...) {char buffer[256]; sprintf_error(buffer, fmt "\r\n", ##__VA_ARGS__); log_error(buffer, stripProjectPath(__FILE__), __LINE__ );}
#define LOG_INFO(fmt, ...) {char buffer[256]; sprintf_s(buffer, fmt "\r\n", ##__VA_ARGS__); log_info(buffer, stripProjectPath(__FILE__), __LINE__ );}

const mchar* stripProjectPath(const mchar* path);
void log_error(mchar* buffer, const mchar* fileName, uint32 lineNumber);
void log_info(mchar* buffer, const mchar* fileName, uint32 lineNumber);

template<typename T>
class Guard {

public:
	
	Guard(T t) {
		this->t = t;
	}

	~Guard() {
		DELETE_NOT_NULL(t);
	}

private:
	
	T t = nullptr;

};

#define DelGuard(x) Guard<decltype(x)> DelGuard##x(x)

bool FileExists(const char* path);

bool DirectoryExists(const char* path);

bool CreateDirectory(const char* path);

bool DeleteFile(const char* path);

bool Sha1(uint8* data, uint32 dataSize, const char** lowerCaseHash);

bool ReadFile(const char* path, uint8** data, uint32* dataLength);

bool WriteFile(const char* path, uint8* data, uint32 dataLength);

class Logger {

public:
	
	static void LogError(const char* fileName, uint32 lineNumber, const char* buffer);

	static void LogInfo(const char* fileName, uint32 lineNumber, const char* buffer);


};