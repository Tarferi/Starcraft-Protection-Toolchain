#pragma once

#include <Protection.h>

class DesktopFactory;
class Process;
class ProcessWindow;

class ProEdit : public Protection {

public:

	ProEdit(DesktopFactory* df, const char* name, const char* email);

	virtual ~ProEdit();

	virtual bool Protect(char* input, char* output) override;

	virtual bool Check() override;

private:

	bool CheckFiles(const char* root);

	bool HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file);

	bool HandleMapProtect(Process* p, ProcessWindow* wnd, const char* username, const char* email);
	
	bool HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file);

	const char* username = nullptr;
	const char* email = nullptr;
};

