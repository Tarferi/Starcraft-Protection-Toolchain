#include "Process.h"
#include "Desktop.h"

#include <windows.h>
#include <uiautomation.h>
#include <tlhelp32.h>
#include <comdef.h> 
#include <locale.h>
#include <objidl.h>

#pragma push_macro("GetClassName")
#ifdef GetClassName
#undef GetClassName
#endif

struct UINodeData {
	IUIAutomation* pClientUIA;
	IUIAutomationTreeWalker* pControlWalker;
	IUIAutomationElement* node;
	HWND wnd;
};

UINode::UINode(struct UINodeData* pdata) {
	static_assert(sizeof(struct UINodeData) <= sizeof(buffer), "Buffer too small");
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	memcpy(data, pdata, sizeof(struct UINodeData));
}

UINode::~UINode() {
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	if (data->node) {
		data->node->Release();
		data->node = nullptr;
	}
}

char UINode::NameBuffer[4096];

const char* UINode::GetName() {
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	BSTR tmp;
	if (data->node->get_CurrentName(&tmp) == S_OK) {
		const WCHAR* wc = tmp;
		_bstr_t b(wc);
		const char* c = b;
		if (c != nullptr) {
			strcpy_s(NameBuffer, c);
		} else {
			sprintf_s(NameBuffer, "(null)");
		}
		SysFreeString(tmp);
	} else {
		sprintf_s(NameBuffer, "(error)");
	}
	return NameBuffer;
}

char UINode::ClassNameBuffer[4096];

const char* UINode::GetClassName() {
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	BSTR tmp;
	if (data->node->get_CurrentClassName(&tmp) == S_OK) {
		const WCHAR* wc = tmp;
		_bstr_t b(wc);
		const char* c = b;
		if (c != nullptr) {
			strcpy_s(ClassNameBuffer, c);
		} else {
			sprintf_s(ClassNameBuffer, "(null)");
		}
		SysFreeString(tmp);
	} else {
		sprintf_s(ClassNameBuffer, "(error)");
	}
	return ClassNameBuffer;
}

int32 UINode::GetRuntimeID() {
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	SAFEARRAY* arr;
	if (data->node->GetRuntimeId(&arr) == S_OK) {
		if (arr->cbElements == sizeof(PID)) {
			return ((PID*)arr->pvData)[0];
		} else {
			LOG_ERROR("Invalid size of runtime id elements: %d", arr->cbElements);
		}
	}
	return 0;
}

bool UINode::Click() {
	bool bRet = false;
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	IUnknown* tmp;
	HRESULT hr = data->node->GetCurrentPattern(UIA_InvokePatternId, (IUnknown**)&tmp);
	if (hr == S_OK) {
		if (tmp) {
			IInvokeProvider* invoker = static_cast<IInvokeProvider*>(tmp);
			if (invoker) {
				hr = data->node->SetFocus();
				if (hr == S_OK) {
					hr = invoker->Invoke();
					if (hr == S_OK) {
						bRet = true;
					}
				}
			}
		} else {
			
			hr = data->node->GetCurrentPattern(UIA_LegacyIAccessiblePatternId, (IUnknown**)&tmp);
			if (tmp && hr == S_OK) {
				ILegacyIAccessibleProvider* acc = static_cast<ILegacyIAccessibleProvider*>(tmp);
				if (acc) {
					HWND phwnd = GetForegroundWindow();
					SetForegroundWindow(data->wnd);
					hr = data->node->SetFocus();
					if (hr == S_OK) {
						INPUT in;
						memset(&in, 0, sizeof(INPUT));
						in.ki.wScan = MapVirtualKeyEx(VK_RETURN, 0, GetKeyboardLayout(0));
						in.type = INPUT_KEYBOARD;
						in.ki.wVk = VK_RETURN;
						in.ki.time = 0;
						Sleep(5);
						uint32 evts = SendInput(1, &in, sizeof(INPUT));
						if (evts == 1) {
							in.ki.dwFlags = KEYEVENTF_KEYUP;
							evts = SendInput(1, &in, sizeof(INPUT));
							if (evts == 1) {
								bRet = true;
							}
						}
						Sleep(5);
					}
					SetForegroundWindow(phwnd);
				}
			}
			
		}
	}
	if (tmp) {
		tmp->Release();
		tmp = nullptr;
	}
	return bRet;
}

std::wstring s2ws(const std::string& str) {
	int size_needed = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
	std::wstring wstrTo(size_needed, 0);
	MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], size_needed);
	return wstrTo;
}

bool UINode::SetText(const char* text) {
	bool bRet = false;
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	IUnknown* tmp;
	HRESULT hr = data->node->GetCurrentPattern(UIA_ValuePatternId, (IUnknown**)&tmp);
	if (tmp && hr == S_OK) {
		IValueProvider* edit = static_cast<IValueProvider*>(tmp);
		if (edit) {
			hr = data->node->SetFocus();
			if (hr == S_OK) {
				std::string texts(text);
				std::wstring textw = s2ws(texts);
				hr = edit->SetValue(textw.c_str());
				if (hr == S_OK) {
					bRet = true;
				}
			}
		}
	}
	if (tmp) {
		tmp->Release();
		tmp = nullptr;
	}
	return bRet;
}

bool UINode::IterChildren2(void(*cb)(void* instance, UINode* wnd), void* instance) {
	struct UINodeData* data = reinterpret_cast<struct UINodeData*>(buffer);
	bool bRet = true;
	IUIAutomationElement* pNode = NULL;
	data->pControlWalker->GetFirstChildElement(data->node, &pNode);
	while (pNode) {
		struct UINodeData nodeData;
		memset(&nodeData, 0, sizeof(nodeData));
		nodeData.wnd = data->wnd;
		nodeData.pClientUIA = data->pClientUIA;
		nodeData.node = pNode;
		nodeData.pControlWalker = data->pControlWalker;
		UINode node(&nodeData);
		cb(instance, &node);
		IUIAutomationElement* pNext;
		data->pControlWalker->GetNextSiblingElement(pNode, &pNext);
		pNode = pNext;
	}
	return bRet;
}

struct ProcessWindowData {
	HWND handle;
};

ProcessWindow::ProcessWindow(struct ProcessWindowData* pdata) {
	static_assert(sizeof(struct ProcessWindowData) <= sizeof(buffer), "Buffer too small");
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	memcpy(data, pdata, sizeof(struct ProcessWindowData));

	static_assert(sizeof(struct UINodeData) <= sizeof(rootNodeData), "Buffer too small");
	struct UINodeData* ndata = reinterpret_cast<struct UINodeData*>(rootNodeData);
	memset(ndata, 0, sizeof(struct UINodeData));
}

ProcessWindow::~ProcessWindow() {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	struct UINodeData* ndata = reinterpret_cast<struct UINodeData*>(rootNodeData);
	if (ndata->pControlWalker) {
		ndata->pControlWalker->Release();
		ndata->pControlWalker = nullptr;
	}
	if (ndata->node) {
		ndata->node->Release();
		ndata->node = nullptr;
	}
	if (ndata->pClientUIA) {
		ndata->pClientUIA->Release();
		ndata->pClientUIA = nullptr;
	}
}

bool ProcessWindow::IsSameAs(ProcessWindow* another) {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	struct ProcessWindowData* anotherData = reinterpret_cast<struct ProcessWindowData*>(another->buffer);
	if (data->handle == anotherData->handle) {
		return true;
	}
	return false;
}

static char WindowNames[1024];

const char* ProcessWindow::GetWindowName() {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	GetWindowTextA(data->handle, WindowNames, sizeof(WindowNames) - 1);
	return WindowNames;
}

static char ClassNames[1024];

const char* ProcessWindow::GetWindowClassName() {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	GetClassNameA(data->handle, ClassNames, sizeof(ClassNames) - 1);
	return ClassNames;
}

void PrintNode(UINode* node, int indent, NodeInfo nodeInfo) {
	auto printIndent = [](int indent) {
		for (int i = 0; i < indent; i++) {
			putchar(' ');
		}
	};
	printIndent(indent);
	switch (nodeInfo) {
		case NodeInfo::NameOnly:
			printf("%s\n", node->GetName());
			break;

		case NodeInfo::ClassAndName:
			printf("%s (%s)\n", node->GetClassName(), node->GetName());
			break;

		case NodeInfo::IDAndName:
			printf("%d (%s)\n", node->GetRuntimeID(), node->GetName());
			break;
	}
	node->IterChildren([&](UINode* n) {
		PrintNode(n, indent + 3, nodeInfo);
	});
}

void ProcessWindow::PrintHierarchy(NodeInfo nodeInfo) {
	UINode* node = GetRootNode();
	if (node) {
		PrintNode(node, 0, nodeInfo);
	}
}

struct UINodeData* ProcessWindow::GetRootNodeData() {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	struct UINodeData* ndata = reinterpret_cast<struct UINodeData*>(rootNodeData);
	if (ndata->node) {
		return ndata;
	}

	ndata->wnd = data->handle;

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (hr != S_OK && hr != S_FALSE) {
		LOG_ERROR("CoInitializeEx error: %d", hr);
		return nullptr;
	}

	hr = CoCreateInstance(CLSID_CUIAutomation, NULL, CLSCTX_INPROC_SERVER, IID_IUIAutomation, reinterpret_cast<void**>(&ndata->pClientUIA));
	if (S_OK != hr) {
		LOG_ERROR("CoCreateInstance error: %d", hr);
		return nullptr;
	}

	ndata->pClientUIA->get_ControlViewWalker(&ndata->pControlWalker);
	if (ndata->pControlWalker == NULL) {
		return nullptr;
	}

	hr = ndata->pClientUIA->ElementFromHandle(data->handle, &ndata->node);
	if (S_OK != hr) {
		LOG_ERROR("ElementFromHandle error: %d", hr);
		ndata->pControlWalker->Release();
		return nullptr;
	}
	return ndata;
}

UINode* ProcessWindow::GetRootNode() {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);

	struct UINodeData* rootData = GetRootNodeData();
	if (!rootData) {
		LOG_ERROR("Failed to create root node data");
		return nullptr;
	}

	return new UINode(rootData);
}

bool ProcessWindow::NodeForPath2(const char* path, void(*cb)(void* instance, UINode* node), void* instance) {

	using iterT = bool(*)(void* iterT, UINode*, const char*, void(*)(void*, UINode*), void*);

	iterT t = nullptr;

	t = [](void* iterTv, UINode* node, const char* path, void(*cb)(void* instance, UINode* node), void* instance) {
		iterT iterTf = reinterpret_cast<iterT>(iterTv);

		const char* subPath = strstr(path, "/");
		if (subPath) {
			// Recursive call
			char tmp[128];
			uint32 sz = (uint32)(subPath - path);
			memcpy(tmp, path, sz + 1);
			tmp[sz] = 0;

			bool found = false;
			bool subFound = false;
			node->IterChildren([&](UINode* child) {
				if (!found) {
					if (!strcmp(child->GetName(), tmp)) {
						found = true;
						subFound = iterTf(iterTv, child, &(subPath[1]), cb, instance);
					}
				}
			});
			return found && subFound;

		} else {
			// Find direct child
			bool found = false;
			node->IterChildren([&](UINode* child) {
				if (!found) {
					if (!strcmp(child->GetName(), path)) {
						found = true;
						cb(instance, child);
					}
				}
			});
			return found;
		}

		return false;
	};


	UINode* node = this->GetRootNode();
	if (node) {
		return t(t, node, path, cb, instance);
	}
	LOG_ERROR("Failed to get root node");
	return false;
}

bool ProcessWindow::ClickElement(const char* path) {
	return NodeForPath(path, [&](UINode* node) {
		node->Click();
		return;
	});
}

void ProcessWindow::ExportData(struct ProcessWindowData* pdata) {
	struct ProcessWindowData* data = reinterpret_cast<struct ProcessWindowData*>(buffer);
	memcpy(pdata, data, sizeof(struct ProcessWindowData));
}

struct ProcessData {
	IUIAutomation* pClientUIA;
	IUIAutomationElement* pRootElement;

	PID pid;
	HANDLE pHandle;
};

Process::Process(Desktop* desktop, const char* exePath) {
	static_assert(sizeof(struct ProcessData) <= sizeof(buffer), "Buffer too small");
	struct ProcessData* data = reinterpret_cast<struct ProcessData*>(buffer);
	memset(data, 0, sizeof(struct ProcessData));
	this->exePath = exePath;
	this->desktop = desktop;
}

Process::~Process() {
	struct ProcessData* data = reinterpret_cast<struct ProcessData*>(buffer);
	if (data->pHandle) {
		//if (WaitForSingleObject(data->pHandle, 10) == WAIT_TIMEOUT) {
		TerminateProcess(data->pHandle, 0);
		data->pHandle = NULL;
		data->pid = 0;
		//}
	}
}

bool Process::Run() {
	struct ProcessData* data = reinterpret_cast<struct ProcessData*>(buffer);

	STARTUPINFOA si;
	memset(&si, 0, sizeof(si));
	si.cb = sizeof(si);
	si.lpDesktop = (LPSTR)(desktop ? desktop->GetDesktopNameForCreateProcess() : nullptr);
	PROCESS_INFORMATION pi;
	memset(&pi, 0, sizeof(pi));

	BOOL result = CreateProcessA(NULL,   // Name of program to execute
		(LPSTR)exePath,
		NULL,                      // Process handle not inheritable
		NULL,                      // Thread handle not inheritable
		FALSE,                     // Set handle inheritance to FALSE
		0,                         // No creation flags
		NULL,                      // Use parent's environment block
		NULL,                      // Use parent's starting directory
		&si,                       // Pointer to STARTUPINFO structure
		&pi);

	if (!result) {
		LOG_ERROR("Failed to run %s", exePath);
	} else {
		data->pid = pi.dwProcessId;
		data->pHandle = pi.hProcess;
	}
	return result;
}

int32 Process::WaitFor() {
	struct ProcessData* data = reinterpret_cast<struct ProcessData*>(buffer);
	if (data->pHandle) {
		DWORD ret = WaitForSingleObject(data->pHandle, INFINITE);
		data->pHandle = NULL;
		data->pid = 0;
		return ret;
	}
	LOG_ERROR("Waiting for terminated process");
	return -1;
}

ProcessWindow* Process::WaitForWindow(const char* name, int32 timeoutMS) {
	int32 waited = 0;
	struct ProcessWindowData data;
	memset(&data, 0, sizeof(data));
	while (waited < timeoutMS) {
		bool bRetNow = false;
		if (!IterWindows([&](ProcessWindow* wnd) {
			if (!strcmp(wnd->GetWindowName(), name)) {
				bRetNow = true;
				wnd->ExportData(&data);
			}
		})) {
			LOG_ERROR("Failed to iterate windows");
			return nullptr;
		}
		if (bRetNow) {
			return new ProcessWindow(&data);
		}
		waited += 50;
		Sleep(50);
	}
	return nullptr;
}

bool Process::WaitForWindowClose(const char* name, int32 timeoutMS) {
	int32 waited = 0;
	while (waited < timeoutMS) {
		bool bRetNow = true;
		if (!IterWindows([&](ProcessWindow* wnd) {
			if (!strcmp(wnd->GetWindowName(), name)) {
				bRetNow = false;
			}
			})) {
			LOG_ERROR("Failed to iterate windows");
			return false;
		}
		if (bRetNow) {
			return true;
		}
		waited += 50;
		Sleep(50);
	}
	return false;
}

ProcessWindow* Process::WaitForWindowClass(const char* className, int32 timeoutMS) {
	int32 waited = 0;
	struct ProcessWindowData data;
	memset(&data, 0, sizeof(data));
	while (waited < timeoutMS) {
		bool bRetNow = false;
		if (!IterWindows([&](ProcessWindow* wnd) {
			if (!strcmp(wnd->GetWindowClassName(), className)) {
				bRetNow = true;
				wnd->ExportData(&data);
			}
			})) {
			LOG_ERROR("Failed to iterate windows");
			return nullptr;
		}
		if (bRetNow) {
			return new ProcessWindow(&data);
		}
		waited += 50;
		Sleep(50);
	}
	return nullptr;
}

bool Process::WaitForWindowClassClose(const char* className, int32 timeoutMS, ProcessWindow* wndThatMustNotExistAnymore) {
	int32 waited = 0;
	while (waited < timeoutMS) {
		bool bRetNow = true;
		if (!IterWindows([&](ProcessWindow* wnd) {
			if (!strcmp(wnd->GetWindowClassName(), className)) {
				if (wndThatMustNotExistAnymore != nullptr) {
					if (wnd->IsSameAs(wndThatMustNotExistAnymore)) {
						bRetNow = false;
					}
				} else {
					bRetNow = false;
				}
			}
			})) {
			LOG_ERROR("Failed to iterate windows");
			return false;
		}
		if (bRetNow) {
			return true;
		}
		waited += 50;
		Sleep(50);
	}
	return false;
}

bool Process::IterWindows2(void(*cb)(void* instance, ProcessWindow* wnd), void* instance) {
	struct ProcessData* data = reinterpret_cast<struct ProcessData*>(buffer);
	bool bRet = true;

	struct tmp {
		Process* self;
		void(*cb)(void* instance, ProcessWindow* wnd);
		void* instance;
	} tmpI{ this, cb, instance };

	auto forEachThread = [&data, &bRet](auto threadCB) {

		HANDLE h = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
		if (h != INVALID_HANDLE_VALUE) {
			THREADENTRY32 te;
			te.dwSize = sizeof(te);
			if (Thread32First(h, &te)) {
				do {
					if (te.dwSize >= FIELD_OFFSET(THREADENTRY32, th32OwnerProcessID) + sizeof(te.th32OwnerProcessID)) {
						if (te.th32OwnerProcessID == data->pid) {
							threadCB(te.th32ThreadID);
						}
					}
					te.dwSize = sizeof(te);
				} while (Thread32Next(h, &te));
			}
			CloseHandle(h);
		} else {
			bRet = false;
		}
	};

	auto enumWnd = [](HWND wnd, LPARAM param) -> BOOL {
		struct tmp* tmpI = reinterpret_cast<struct tmp*>(param);
		char buffer[1024];
		GetWindowTextA(wnd, buffer, sizeof(buffer) - 1);

		struct ProcessWindowData data;
		memset(&data, 0, sizeof(data));
		data.handle = wnd;
		ProcessWindow pw(&data);
		tmpI->cb(tmpI->instance, &pw);

		return TRUE;
	};

	auto forEachThreadCB = [&](DWORD thread) {
		EnumThreadWindows(thread, enumWnd, (LPARAM)&tmpI);
	};

	forEachThread(forEachThreadCB);

	return bRet;
}

#pragma pop_macro ("GetClassName")