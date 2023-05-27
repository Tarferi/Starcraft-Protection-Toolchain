#pragma once

#include "common.h"

class Desktop;
class DesktopFactory;
class ProcessWindow;

class Protection {

public:

	Protection(DesktopFactory* df, const char* name);

	virtual ~Protection();

	virtual bool Protect(char* input, char* output) = 0;

	virtual bool Check() = 0;

protected:

	const char* GetWorkingDir();

	bool EnterDesktop(const char* name);

	bool LeaveDesktop();

	Desktop* GetDesktop();

	bool HasDLL(const char* name);
	
	bool HasDLLClass(const char* name, const char* cls);

	bool CheckFile(const char* root, const char* file, const char* sha1, uint8* data, uint32 dataLength);

	bool HandleOpenFileDialog(ProcessWindow* wnd, const char* file);

	bool HandleSaveFileDialog(ProcessWindow* wnd, const char* file);

private:

	const char* name = nullptr;

	Desktop* d = nullptr;
	DesktopFactory* df = nullptr;

};

