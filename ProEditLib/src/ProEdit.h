#pragma once
#include <common.h>
#include <Protection.h>

class DesktopFactory;

class ProEdit : public Protection {

public:

	ProEdit(DesktopFactory* df, const char* name, const char* email);

	virtual ~ProEdit();

	virtual bool Protect(char* input, char* output) override;

	virtual bool Check() override;

private:

	bool CheckFiles(const char* root);

	bool CheckFile(const char* root, const char* file, const char* sha1, uint8* data, uint32 dataLength);

	const char* username = nullptr;
	const char* email = nullptr;
};

