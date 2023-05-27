#pragma once

#include <Protection.h>

class DesktopFactory;
class Process;
class ProcessWindow;

class TinyMap : public Protection {

public:

	TinyMap(DesktopFactory* df);

	virtual ~TinyMap();

	virtual bool Protect(char* input, char* output) override;

	virtual bool Check() override;

private:

	bool CheckFiles(const char* root);

	bool HandleOpenFile(Process* p, ProcessWindow* wnd, const char* file);

	bool HandleMapCompress(Process* p, ProcessWindow* wnd);

	bool HandleSaveFile(Process* p, ProcessWindow* wnd, const char* file);

};

