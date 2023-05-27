#pragma once

#include "common.h"

class Desktop {

	Desktop(Desktop&) = delete;

	Desktop(Desktop&&);

public:
	
	Desktop(const char* name);

	virtual ~Desktop();

	virtual bool Create() = 0;

	virtual bool Enter() = 0;

	virtual bool Leave() = 0;

	const char* GetName();

	virtual const char* GetDesktopNameForCreateProcess() = 0;

private:

	const char* name = nullptr;

};

class VisibleDesktop : public Desktop {

public:

	VisibleDesktop(const char* name);

	virtual ~VisibleDesktop();

	virtual bool Create() override;

	virtual bool Enter() override;

	virtual bool Leave() override;

	virtual const char* GetDesktopNameForCreateProcess() override;
};

class HiddenDesktop : public Desktop {
	
	HiddenDesktop(HiddenDesktop&) = delete;
	
	HiddenDesktop(HiddenDesktop&&);

public:

	HiddenDesktop(const char* name);

	virtual ~HiddenDesktop();

	virtual bool Create() override;

	virtual bool Enter() override;

	virtual bool Leave() override;

	virtual const char* GetDesktopNameForCreateProcess() override;

private:

	uint8 buffer[64];
};

class DesktopFactory {

public: 

	DesktopFactory() {}

	virtual ~DesktopFactory() {}

	virtual Desktop&& CreateDesktopRef(const char* name) = 0;
	
	virtual Desktop* CreateDesktop(const char* name) = 0;
};

class VisibleDesktopFactory : public DesktopFactory {

public:
	VisibleDesktopFactory() {}

	virtual ~VisibleDesktopFactory() {}

	virtual Desktop&& CreateDesktopRef(const char* name) override;
	
	virtual Desktop* CreateDesktop(const char* name) override;
};

class HiddenDesktopFactory : public DesktopFactory {

public:
	HiddenDesktopFactory() {}

	virtual ~HiddenDesktopFactory() {}

	virtual Desktop&& CreateDesktopRef(const char* name) override;
	
	virtual Desktop* CreateDesktop(const char* name) override;
};