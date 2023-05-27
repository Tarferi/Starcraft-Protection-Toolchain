#pragma once

#include "common.h"

class Desktop;

struct UINodeData;
struct ProcessWindowData;

class UINode {

public:

	UINode(struct UINodeData* pdata);

	~UINode();

	template<typename callable>
	bool IterChildren(callable cb) {
		struct tmp {
			callable* cb;
		} tmpI{ &cb };

		auto mcb = [](void* instance, UINode* wnd) {
			struct tmp* tmpI = reinterpret_cast<struct tmp*>(instance);
			(*tmpI->cb)(wnd);
		};

		return IterChildren2(mcb, &tmpI);
	}

	bool Click();

	bool SetText(const char* text);

	const char* GetName();

	const char* GetClassName();

	int32 GetRuntimeID();

	template<typename callable>
	bool NthChildOfClass(const char* className, int32 childIdx, callable cb) {
		int32 found = 0;
		
		if (!IterChildren([&](UINode* child) {
			if (!strcmp(child->GetClassName(), className)) {
				if (found == childIdx) {
					cb(child);
				}
				found++;
			}
		})) {
			LOG_ERROR("Failed to iterate children");
			return false;
		}
		
		return found > childIdx;
	}

private:

	static char NameBuffer[4096];
	static char ClassNameBuffer[4096];
	
	uint8 buffer[64];

	bool IterChildren2(void(*cb)(void* instance, UINode* child), void* instance);

};

E_TYPE(NodeInfo, 
	NameOnly,
	ClassAndName,
	IDAndName
)

class ProcessWindow {

	friend class Process;

private:

	ProcessWindow(struct ProcessWindowData* data);
	
public:

	virtual ~ProcessWindow();

	const char* GetWindowName();

	const char* GetWindowClassName();

	void PrintHierarchy(NodeInfo nodeInfo = NodeInfo::NameOnly);

	bool ClickElement(const char* path);

	UINode* GetRootNode();

	template<typename callable>
	bool NodeForPath(const char* path, callable cb) {
		struct tmp {
			callable* cb;
		} tmpI{ &cb };

		auto mcb = [](void* instance, UINode* node) {
			struct tmp* tmpI = reinterpret_cast<struct tmp*>(instance);
			(*tmpI->cb)(node);
		};

		return NodeForPath2(path, mcb, &tmpI);
	}

protected:

	void ExportData(struct ProcessWindowData* data);

	struct UINodeData* GetRootNodeData();

private:

	uint8 buffer[64];

	uint8* rootNodeData[64];

	bool NodeForPath2(const char* ptah, void(*cb)(void* instance, UINode* node), void* instance);
};

class Process {

public:

	Process(Desktop* desktop, const char* exePath);
		
	virtual ~Process();

	template<typename callable>
	bool IterWindows(callable cb) {
		struct tmp {
			callable* cb;
		} tmpI{ &cb };

		auto mcb = [](void* instance, ProcessWindow* wnd) {
			struct tmp* tmpI = reinterpret_cast<struct tmp*>(instance);
			(*tmpI->cb)(wnd);
		};

		return IterWindows2(mcb, &tmpI);
	};

	ProcessWindow* WaitForWindow(const char* name, int32 timeoutMS);
	
	bool WaitForWindowClose(const char* name, int32 timeoutMS);

	ProcessWindow* WaitForWindowClass(const char* className, int32 timeoutMS);
	
	bool WaitForWindowClassClose(const char* className, int32 timeoutMS);

	bool Run();

	int32 WaitFor();

private:

	bool IterWindows2(void(*cb)(void* instance, ProcessWindow* wnd), void* instance);

	const char* exePath = nullptr;

	Desktop* desktop = nullptr;

	uint8 buffer[64];
};

