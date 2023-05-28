#pragma once

#include <Protection.h>

class DesktopFactory;
class Process;
class ProcessWindow;

class ShadowProtector : public Protection {

public:

	ShadowProtector(DesktopFactory* df);

	virtual ~ShadowProtector();

	virtual bool Protect(char* input, char* output) override;

	virtual bool Check() override;

private:

	bool CheckFiles(const char* root);

	bool HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file);

	bool HandleMapProtect(Process* p, ProcessWindow* wnd);

	bool HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file);

};

