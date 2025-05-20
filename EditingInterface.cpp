#include <windows.h>
#include <commctrl.h>
#include <richedit.h>
#include <iostream>
#include <fstream>
#include <string>
#include <commdlg.h>
#include <stack>
#include <regex>
#include <filesystem>
#include <wininet.h>

using namespace std;
namespace fs = std::filesystem;

// 文件资源管理器相关
HWND hExplorerList;   // 资源管理器列表框句柄
HWND hUpButton;       // 上级目录按钮句柄
fs::path currentPath; // 当前路径

// 全局变量
const char g_szClassName[] = "Dev Fanle Studio";
HMENU hFileMenu, hToolsMenu, hLanguageMenu, hPluginsMenu, hRunMenu, hEditMenu, hHelpMenu, hUSBMenu;
HWND hEdit1, hEdit2, hLineNumbers1, hLineNumbers2, hTrackbar;
HFONT hExplorerFont = NULL; // 新增文件资源管理器字体变量
HFONT hCustomFont = NULL;
int fontSize = 20;
string FilePath;
bool FileChanged = false;
int Language = 1;
fstream FileContents;
stack<string> undoStack1, redoStack1;
stack<string> undoStack2, redoStack2;
WNDPROC OriginalEditProc1, OriginalEditProc2;
const char* a = R"(
#include <bits/stdc++.h>
using namespace std;

int main() {
    return 0;
}
)";

// 控件ID定义
#define ID_UPDIR    27
#define ID_NEW      1
#define ID_OPEN     2
#define ID_SAVE     3
#define ID_EXIT     4
#define ID_ABOUT    5
#define ID_VM       6
#define ID_RUN      7
#define ID_QUASH    8
#define ID_RECOVER  10
#define ID_CUSTOM_CMD      11
#define ID_AICODE   12
#define ID_USE      13
#define ID_CAR      14
#define ID_COMPILE  15
#define ID_SAVEAS   18
#define ID_EFI      20
#define ID_BOOT     21
#define ID_DD       221
#define ID_NSUDO    23
#define ID_CF       24
#define ID_PIM      25
#define ID_SETTINGS 26

#define ID_TRACKBAR 100
#define IDT_UPD_LINE_NUM 101

#define ID_TEXT     1000
#define ID_CPP      1001
#define ID_C        1002
#define ID_PYTHON   1003

// 文件资源管理器更新函数
void UpdateExplorerList(HWND hExplorerList, const fs::path& path) {
    SendMessage(hExplorerList, LB_RESETCONTENT, 0, 0);

    try {
        for (const auto& entry : fs::directory_iterator(path)) {
            const fs::path& entryPath = entry.path();
            string displayName = entryPath.filename().string();
            
            if (fs::is_directory(entryPath)) {
                displayName = "[DIR] " + displayName;
            }
            
            SendMessage(hExplorerList, 
                       LB_ADDSTRING, 
                       0, 
                       (LPARAM)displayName.c_str());
        }
    } catch (const fs::filesystem_error& e) {
        MessageBox(NULL, 
                  ("无法访问目录: " + string(e.what())).c_str(), 
                  "错误", 
                  MB_OK | MB_ICONERROR);
    }
}

// 获取右括号函数
char GetRightBrace(WPARAM leftBrace) {
	switch (leftBrace) {
		case '(':
			return ')';
		case '{':
			return '}';
		case '[':
			return ']';
		case '"':
			return '"';
		case '\'':
			return '\'';
		default:
			return '\0'; 
	}
}

// 补全括号函数
void CompleteBrackets(HWND hEdit, WPARAM wParam) {
	DWORD start, end;
	SendMessage(hEdit, EM_GETSEL, (WPARAM)&start, (LPARAM)&end);

	if (start == end) { 
		switch (wParam) {
			case '(':
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)")");
				SendMessage(hEdit, EM_SETSEL, start, start);
				break;
			case '{':
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)"}");
				SendMessage(hEdit, EM_SETSEL, start, start);
				break;
			case '[':
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)"]");
				SendMessage(hEdit, EM_SETSEL, start, start);
				break;
			case '"':
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)"\"");
				SendMessage(hEdit, EM_SETSEL, start, start);
				break;
			case '\'':
				SendMessage(hEdit, EM_REPLACESEL, FALSE, (LPARAM)"'");
				SendMessage(hEdit, EM_SETSEL, start, start);
				break;
		}
	}
}

// 加载字体函数（文件资源管理器使用微软雅黑）
void LoadExplorerFont(HWND hwnd) {
	hExplorerFont = CreateFont(
	                  0,
	                  0,
	                  0,
	                  0,
	                  FW_NORMAL,
	                  FALSE,
	                  FALSE,
	                  FALSE,
	                  DEFAULT_CHARSET,
	                  OUT_DEFAULT_PRECIS,
	                  CLIP_DEFAULT_PRECIS,
	                  DEFAULT_QUALITY,
	                  DEFAULT_PITCH | FF_DONTCARE,
	                  "微软雅黑"
	              );

	if (hExplorerFont == NULL) {
		MessageBox(hwnd, "无法创建字体", "错误", MB_OK);
	}
}

// 加载默认字体函数
void LoadCustomFont(HWND hwnd) {
	hCustomFont = CreateFont(
	                  fontSize,
	                  0,
	                  0,
	                  0,
	                  FW_NORMAL,
	                  FALSE,
	                  FALSE,
	                  FALSE,
	                  DEFAULT_CHARSET,
	                  OUT_DEFAULT_PRECIS,
	                  CLIP_DEFAULT_PRECIS,
	                  DEFAULT_QUALITY,
	                  DEFAULT_PITCH | FF_DONTCARE,
	                  "Consolas"
	              );

	if (hCustomFont == NULL) {
		MessageBox(hwnd, "无法创建字体", "错误", MB_OK);
	}
}

// 更新字体大小函数
void UpdateFontSize(HWND hEdit, int newSize) {
	HFONT hNewFont = CreateFont(
	                     newSize,
	                     0,
	                     0,
	                     0,
	                     FW_NORMAL,
	                     FALSE,
	                     FALSE,
	                     FALSE,
	                     DEFAULT_CHARSET,
	                     OUT_DEFAULT_PRECIS,
	                     CLIP_DEFAULT_PRECIS,
	                     DEFAULT_QUALITY,
	                     DEFAULT_PITCH | FF_DONTCARE,
	                     "Consolas"
	                 );

	if (hNewFont != NULL) {
		SendMessage(hEdit1, WM_SETFONT, (WPARAM)hNewFont, TRUE);
		SendMessage(hLineNumbers1, WM_SETFONT, (WPARAM)hNewFont, TRUE);
		SendMessage(hEdit2, WM_SETFONT, (WPARAM)hNewFont, TRUE);
		SendMessage(hLineNumbers2, WM_SETFONT, (WPARAM)hNewFont, TRUE);

		if (hCustomFont != NULL) {
			DeleteObject(hCustomFont);
		}
		hCustomFont = hNewFont;
	} else {
		MessageBox(NULL, "无法创建字体", "错误", MB_OK);
	}

	InvalidateRect(hEdit1, NULL, TRUE);
	InvalidateRect(hLineNumbers1, NULL, TRUE);
	InvalidateRect(hEdit2, NULL, TRUE);
	InvalidateRect(hLineNumbers2, NULL, TRUE);
}

// 保存文件函数
void SaveFile(HWND hEdit, const string& filePath) {
	int len = GetWindowTextLength(hEdit);
	char* buffer = new char[len + 1];
	GetWindowText(hEdit, buffer, len + 1);
	string content(buffer);
	delete[] buffer;

	ofstream file(filePath, ios::out | ios::trunc);
	if (!file.is_open()) {
		MessageBox(NULL, "无法保存文件", "错误", MB_OK);
		return;
	}

	file << content;
	file.close();
}

// 保存当前编辑内容函数
void saveCurrentEdit(HWND hEdit, stack<string>& undoStack, stack<string>& redoStack) {
	int len = GetWindowTextLength(hEdit);
	char* buffer = new char[len + 1];
	GetWindowText(hEdit, buffer, len + 1);
	string content(buffer);
	delete[] buffer;

	undoStack.push(content);
	redoStack = stack<string>();
}

// 获取剪贴板文本函数
bool GetClipboardText(HGLOBAL hGlobal, string& text) {
	char* lpData = static_cast<char*>(GlobalLock(hGlobal));
	if (lpData == nullptr) {
		return false;
	}

	text = lpData;
	GlobalUnlock(hGlobal);
	return true;
}

// 转换行结束符函数
string ConvertLineEndings(const string& input) {
	string output;
	for (size_t i = 0; i < input.length(); ++i) {
		if (input[i] == '\n') {
			if (i > 0 && input[i - 1] != '\r') {
				output += '\r';
			}
			output += '\n';
		} else {
			output += input[i];
		}
	}
	return output;
}

// 检查并显示日志函数
bool CheckAndDisplayLog() {
	string logFilePath = "Temp\\temp.log";
	if (fs::exists(logFilePath)) {
		ifstream file(logFilePath);
		if (file.is_open()) {
			string content((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
			file.close();
			if (!content.empty()) {
				content = ConvertLineEndings(content);
				SetWindowText(hEdit2, content.c_str());
				return true;
			}
		}
	}
	SetWindowText(hEdit2, "");
	return false;
}

// 运行可执行文件函数
bool RunExecutable(const string& executablePath) {
	STARTUPINFO si;
	PROCESS_INFORMATION pi;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));

	if (!CreateProcess(
	            NULL,
	            const_cast<char*>(executablePath.c_str()),
	            NULL,
	            NULL,
	            FALSE,
	            0,
	            NULL,
	            NULL,
	            &si,
	            &pi
	        )) {
		cerr << "CreateProcess failed (" << GetLastError() << ").\n";
		return false;
	}

	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);
	return true;
}

// 更新行号函数
void UpdateLineNumbers(HWND hEdit, HWND hLineNumbers) {
	int firstLine = SendMessage(hEdit, EM_GETFIRSTVISIBLELINE, 0, 0);
	int lineCount = SendMessage(hEdit, EM_GETLINECOUNT, 0, 0);

	if (lineCount == 0) {
		SetWindowText(hLineNumbers, "1");
		return;
	}

	RECT rc;
	GetClientRect(hEdit, &rc);
	int clientHeight = rc.bottom - rc.top;

	HDC hdc = GetDC(hLineNumbers);
	TEXTMETRIC tm;
	GetTextMetrics(hdc, &tm);
	int lineHeight = tm.tmHeight;
	ReleaseDC(hLineNumbers, hdc);

	int visibleLines = clientHeight / lineHeight;
	int lastLine = min(firstLine + visibleLines + 1, lineCount);

	string lineNumbers;
	for (int i = firstLine; i < lastLine; ++i) {
		lineNumbers += to_string(i + 1) + "\n";
	}

	SetWindowText(hLineNumbers, lineNumbers.c_str());
}

// 打开文件函数
void OpenFile(HWND hwnd) {
	OPENFILENAME ofn;
	char filePath[260] = {0};

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "C/C++ Files\0*.c;*.cpp\0Text Files\0*.txt\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileName(&ofn)) {
		FilePath = filePath;
		ifstream file(filePath, ios::in);
		if (file.is_open()) {
			string fileContents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
			file.close();

			fileContents = ConvertLineEndings(fileContents);
			SetWindowText(hEdit1, fileContents.c_str());
			FileChanged = false;
			saveCurrentEdit(hEdit1, undoStack1, redoStack1);
			UpdateLineNumbers(hEdit1, hLineNumbers1);
		} else {
			MessageBox(hwnd, "无法打开文件", "错误", MB_OK | MB_ICONERROR);
		}
	}
}

// 另存为文件函数
void SaveAsFile(HWND hwnd) {
	OPENFILENAME ofn;
	char filePath[260] = {0};

	ZeroMemory(&ofn, sizeof(ofn));
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = hwnd;
	ofn.lpstrFile = filePath;
	ofn.nMaxFile = sizeof(filePath);
	ofn.lpstrFilter = "C/C++ Files\0*.c;*.cpp\0Text Files\0*.txt\0All Files\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_OVERWRITEPROMPT;

	if (GetSaveFileName(&ofn)) {
		FilePath = filePath;
		ofstream file(filePath, ios::out | ios::trunc);
		if (file.is_open()) {
			file.close();
			SetWindowText(hEdit1, "");
			FileChanged = false;
		} else {
			MessageBox(hwnd, "无法创建新文件", "错误", MB_OK | MB_ICONERROR);
		}
	}
}

// 编辑框子类化过程
LRESULT CALLBACK SubclassEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	if (msg == WM_CHAR) {
		switch (wParam) {
			case '(':
			case '{':
			case '[':
			case '"':
			case '\'':
				CompleteBrackets(hwnd, wParam);
				break;
		}
	}
	return CallWindowProc(OriginalEditProc1, hwnd, msg, wParam, lParam);
}

// 触发复活节彩蛋函数
void TriggerEasterEgg(HWND hwnd) {
	HWND hEggWindow = CreateWindowEx(
	                      WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
	                      "EASTEREGGCLASS",
	                      "Easter Egg!",
	                      WS_BORDER | WS_VISIBLE | WS_SYSMENU | WS_CAPTION,
	                      CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
	                      hwnd,
	                      NULL,
	                      (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
	                      NULL
	                  );

	HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255));
	SetClassLongPtr(hEggWindow, GCLP_HBRBACKGROUND, (LONG_PTR)hBrush);

	MessageBox(hEggWindow, "You found the Easter Egg!", "Congratulations!", MB_OK | MB_ICONINFORMATION);

	DestroyWindow(hEggWindow);
}

// 撤消函数
void undo(HWND hEdit, stack<string>& undoStack, stack<string>& redoStack) {
	if (!undoStack.empty()) {
		string lastContent = undoStack.top();
		undoStack.pop();
		redoStack.push(lastContent);
		SetWindowText(hEdit, lastContent.c_str());
	}
}

// 重做函数
void redo(HWND hEdit, stack<string>& undoStack, stack<string>& redoStack) {
	if (!redoStack.empty()) {
		string lastContent = redoStack.top();
		redoStack.pop();
		undoStack.push(lastContent);
		SetWindowText(hEdit, lastContent.c_str());
	}
}

// 窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	bool EasterEggActive = false;
	static fs::path currentPath = fs::current_path(); // 当前路径

	switch (msg) {
		case WM_CREATE: {
			HMODULE hRichEdit = LoadLibrary("Msftedit.dll");
			if (hRichEdit == NULL) {
				hRichEdit = LoadLibrary("Riched20.dll");
			}
			if (hRichEdit == NULL) {
				MessageBox(hwnd, "无法加载 Rich Edit 控件", "错误", MB_OK | MB_ICONERROR);
				return -1;
			}

			INITCOMMONCONTROLSEX icex;
			icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
			icex.dwICC = ICC_LISTVIEW_CLASSES;
			InitCommonControlsEx(&icex);

			HMENU hMenu = CreateMenu();
			hFileMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "文件");
			AppendMenu(hFileMenu, MF_STRING, ID_NEW, "新建");
			AppendMenu(hFileMenu, MF_STRING, ID_OPEN, "打开");
			AppendMenu(hFileMenu, MF_STRING, ID_SAVE, "保存");
			AppendMenu(hFileMenu, MF_STRING, ID_SAVEAS, "另存为");
			AppendMenu(hFileMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hFileMenu, MF_STRING, ID_SETTINGS, "设置");
			AppendMenu(hFileMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hFileMenu, MF_STRING, ID_EXIT, "退出");

			hEditMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hEditMenu, "编辑");
			AppendMenu(hEditMenu, MF_STRING, ID_QUASH, "撤销");
			AppendMenu(hEditMenu, MF_STRING, ID_RECOVER, "恢复");

			hToolsMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hToolsMenu, "工具");
			AppendMenu(hToolsMenu, MF_STRING, ID_VM, "虚拟机");
			AppendMenu(hToolsMenu, MF_STRING, ID_CUSTOM_CMD, "命令行");
			AppendMenu(hToolsMenu, MF_STRING, ID_DD, "dd");
			AppendMenu(hToolsMenu, MF_STRING, ID_NSUDO, "NSudo");
			AppendMenu(hToolsMenu, MF_STRING, ID_AICODE, "AI代码");
			AppendMenu(hToolsMenu, MF_STRING, ID_CF, "代码框架编写");

			hLanguageMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hLanguageMenu, "语言");
			AppendMenu(hLanguageMenu, MF_STRING, ID_TEXT, "文本");
			AppendMenu(hLanguageMenu, MF_SEPARATOR, 0, "");
			AppendMenu(hLanguageMenu, MF_STRING, ID_C, "C");
			AppendMenu(hLanguageMenu, MF_STRING, ID_CPP, "C++");
			AppendMenu(hLanguageMenu, MF_STRING, ID_PYTHON, "Python");

			hPluginsMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hPluginsMenu, "插件");
			AppendMenu(hPluginsMenu, MF_STRING, ID_PIM, "插件管理");

			hRunMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hRunMenu, "运行");
			AppendMenu(hRunMenu, MF_STRING, ID_RUN, "运行");
			AppendMenu(hRunMenu, MF_STRING, ID_CAR, "编译运行");
			AppendMenu(hRunMenu, MF_STRING, ID_COMPILE, "编译");

			hHelpMenu = CreateMenu();
			AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hHelpMenu, "帮助");
			AppendMenu(hHelpMenu, MF_STRING, ID_ABOUT, "关于");
			AppendMenu(hHelpMenu, MF_STRING, ID_USE, "使用说明");

			SetMenu(hwnd, hMenu);

			// 创建行号框1
			hLineNumbers1 = CreateWindowEx(
			                    0,
			                    "STATIC",
			                    "",
			                    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
			                    310, 10, 40, 400,
			                    hwnd,
			                    NULL,
			                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			                    NULL
			                );

			// 创建编辑框1
			hEdit1 = CreateWindowEx(
			             WS_EX_CLIENTEDGE,
			             "EDIT",
			             "",
			             WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			             360, 10, 700, 400,
			             hwnd,
			             NULL,
			             (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			             NULL
			         );

			// 创建行号框2
			hLineNumbers2 = CreateWindowEx(
			                    0,
			                    "STATIC",
			                    "",
			                    WS_CHILD | WS_VISIBLE | SS_LEFT | SS_NOPREFIX,
			                    310, 420, 40, 200,
			                    hwnd,
			                    NULL,
			                    (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			                    NULL
			                );

			// 创建编辑框2
			hEdit2 = CreateWindowEx(
			             WS_EX_CLIENTEDGE,
			             "EDIT",
			             "",
			             WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
			             360, 420, 700, 200,
			             hwnd,
			             NULL,
			             (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			             NULL
			         );

			// 创建滚动条
			hTrackbar = CreateWindowEx(
			                0,
			                TRACKBAR_CLASS,
			                NULL,
			                WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_NOTICKS | TBS_TOOLTIPS,
			                100, 630, 1100, 40,
			                hwnd,
			                (HMENU)ID_TRACKBAR,
			                (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
			                NULL
			            );

			// 子类化编辑框1
			OriginalEditProc1 = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hEdit1, GWLP_WNDPROC));
			SetWindowLongPtr(hEdit1, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SubclassEditProc));

			// 设置初始焦点
			SetFocus(hEdit1);

			// 设置滚动条范围
			SendMessage(hTrackbar, TBM_SETRANGE, TRUE, MAKELONG(5, 72));
			SendMessage(hTrackbar, TBM_SETPOS, TRUE, fontSize);

			// 加载默认字体
			LoadCustomFont(hwnd);
			SendMessage(hEdit1, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
			SendMessage(hLineNumbers1, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
			SendMessage(hEdit2, WM_SETFONT, (WPARAM)hCustomFont, TRUE);
			SendMessage(hLineNumbers2, WM_SETFONT, (WPARAM)hCustomFont, TRUE);

			// 允许拖放文件
			DragAcceptFiles(hwnd, TRUE);

			// 创建定时器
			SetTimer(hwnd, 1, 1000, NULL);
			SetTimer(hwnd, IDT_UPD_LINE_NUM, 100, NULL);

			// 创建资源管理器列表框
			hExplorerList = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"LISTBOX",
				"",
				WS_CHILD | WS_VISIBLE | WS_VSCROLL | 
				LBS_NOINTEGRALHEIGHT | LBS_NOTIFY,
				10, 10, 250, 400,
				hwnd,
				NULL,
				(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
				NULL);

			// 创建上级目录按钮
			hUpButton = CreateWindowEx(
				WS_EX_CLIENTEDGE,
				"BUTTON",
				"上级目录",
				WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
				10, 420, 250, 30,
				hwnd,
				(HMENU)ID_UPDIR,
				(HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
				NULL);

			// 加载文件资源管理器字体
			LoadExplorerFont(hwnd);
			SendMessage(hExplorerList, WM_SETFONT, (WPARAM)hExplorerFont, TRUE);

			// 初始化资源管理器
			UpdateExplorerList(hExplorerList, currentPath);

			return 0;
		}

		case WM_COMMAND: {
			int wmId = LOWORD(wParam);
			switch (wmId) {
				case ID_AICODE:
					system("start Deepseek.py");
					break;
				case ID_NEW:
					SaveAsFile(hwnd);
					break;
				case ID_OPEN:
					OpenFile(hwnd);
					break;
				case ID_SAVEAS:
					SaveAsFile(hwnd);
					break;
				case ID_EXIT:
					PostQuitMessage(0);
					break;
				case ID_ABOUT:
					MessageBox(hwnd, "Dev Fanle Studio\n版权所有 (C) 2024 * Fusion Rise", "关于 Dev Fanle Studio", MB_OK | MB_ICONINFORMATION);
					break;
				case ID_VM:
					MessageBox(hwnd, "FR Color VM 敬请期待", "FR Color VM", MB_OK);
					break;
				case ID_USE:
					system("start Help.html");
					break;
				case ID_C:
					Language = 1;
					break;
				case ID_CPP:
					Language = 2;
					break;
				case ID_PYTHON:
					Language = 3;
					break;
				case ID_TEXT:
					Language = 0;
					break;
				case ID_CUSTOM_CMD:
					system("start bin\\cmd");
					break;
				case ID_CF: {
					size_t len = strlen(a);
					size_t new_len = len + 1;
					char* new_a = static_cast<char*>(malloc(new_len));
					if (new_a != NULL) {
						const char* src = a;
						char* dst = new_a;
						while (*src) {
							if (*src == '\n') {
								*dst++ = '\r';
								if (*(src + 1) != '\0') {
									*dst++ = '\n';
								}
							} else {
								*dst++ = *src;
							}
							src++;
						}
						*dst = '\0';
						SetWindowText(hEdit1, new_a);
						free(new_a);
					}
					break;
				}
				case ID_SETTINGS:
					break;
				case ID_COMPILE: {
					cout << Language << endl;
					if (Language == 2) { 
						string compileCommand = "g++ \"" + FilePath + "\" -o \"" + FilePath.substr(0, FilePath.find_last_of(".")) + "\" 2>&1";
						ofstream logFile("compile_log.txt");
						if (logFile.is_open()) {
							logFile << compileCommand << endl;
							logFile.close();
						}
						int result = system(compileCommand.c_str());
						if (result == 0) {
							CheckAndDisplayLog();
						}
					} else if (Language == 1) { 
						string compileCommand = "gcc \"" + FilePath + "\" -o \"" + FilePath.substr(0, FilePath.find_last_of(".")) + "\" 2>&1";
						ofstream logFile("compile_log.txt");
						if (logFile.is_open()) {
							logFile << compileCommand << endl;
							logFile.close();
						}
						int result = system(compileCommand.c_str());
						if (result == 0) {
							CheckAndDisplayLog();
						}
					} else if (Language == 3) { 
						string runPython = "start " + FilePath;
						int PythonResult = system(runPython.c_str());
					}
					break;
				}
				case ID_QUASH:
					if (GetFocus() == hEdit1) {
						saveCurrentEdit(hEdit1, undoStack1, redoStack1);
						undo(hEdit1, undoStack1, redoStack1);
					} else if (GetFocus() == hEdit2) {
						saveCurrentEdit(hEdit2, undoStack2, redoStack2);
						undo(hEdit2, undoStack2, redoStack2);
					}
					break;
				case ID_RECOVER:
					if (GetFocus() == hEdit1) {
						redo(hEdit1, undoStack1, redoStack1);
					} else if (GetFocus() == hEdit2) {
						redo(hEdit2, undoStack2, redoStack2);
					}
					break;
				case ID_CAR: {
					if (Language == 1 || Language == 2) {
						string compileCommand = "g++ \"" + FilePath + "\" -o \"" + FilePath.substr(0, FilePath.find_last_of(".")) + "\" 2>&1";
						ofstream logFile("compile_log.txt");
						if (logFile.is_open()) {
							logFile << compileCommand << endl;
							logFile.close();
						}
						int result = system(compileCommand.c_str());
						if (result == 0) {
							CheckAndDisplayLog();
							string executablePath = FilePath.substr(0, FilePath.find_last_of(".")) + ".exe";

							STARTUPINFO si;
							PROCESS_INFORMATION pi;
							ZeroMemory(&si, sizeof(si));
							si.cb = sizeof(si);
							ZeroMemory(&pi, sizeof(pi));
							if (!CreateProcess(
							            NULL,
							            const_cast<char*>(executablePath.c_str()),
							            NULL,
							            NULL,
							            FALSE,
							            0,
							            NULL,
							            NULL,
							            &si,
							            &pi
							        )) {
								MessageBox(NULL, "无法运行生成的程序，请检查可执行文件路径！", "错误", MB_ICONERROR | MB_OK);
							} else {
								WaitForSingleObject(pi.hProcess, INFINITE);
								CloseHandle(pi.hProcess);
								CloseHandle(pi.hThread);
							}
						}
					} else if (Language == 3) {
						string runPython = "start " + FilePath;
						int PythonResult = system(runPython.c_str());
					}
					break;
				}
				case ID_RUN: {
					if (Language == 1 || Language == 2) {
						string executablePath = FilePath.substr(0, FilePath.find_last_of(".")) + ".exe";

						STARTUPINFO si;
						PROCESS_INFORMATION pi;
						ZeroMemory(&si, sizeof(si));
						si.cb = sizeof(si);
						ZeroMemory(&pi, sizeof(pi));
						if (!CreateProcess(
						            NULL,
						            const_cast<char*>(executablePath.c_str()),
						            NULL,
						            NULL,
						            FALSE,
						            0,
						            NULL,
						            NULL,
						            &si,
						            &pi
						        )) {
							MessageBox(NULL, "无法运行程序", "错误", MB_OK | MB_ICONERROR);
						} else {
							CloseHandle(pi.hProcess);
							CloseHandle(pi.hThread);
						}
					} else if (Language == 3) {
						string runPython = "start " + FilePath;
						int PythonResult = system(runPython.c_str());
					}
					break;
				}
				case ID_NSUDO:
					system("start bin\\NSudo.exe");
					break;
				case ID_UPDIR: {
					if (currentPath.has_parent_path()) {
						currentPath = currentPath.parent_path();
						UpdateExplorerList(hExplorerList, currentPath);
					}
					break;
				}
				default:
					return DefWindowProc(hwnd, msg, wParam, lParam);
			}
			return 0;
		}

		case WM_HSCROLL: {
			if (LOWORD(wParam) == TB_THUMBTRACK && HIWORD(wParam) == ID_TRACKBAR) {
				int pos = SendMessage(hTrackbar, TBM_GETPOS, 0, 0);
				fontSize = pos;
				UpdateFontSize(hEdit1, fontSize);
				UpdateLineNumbers(hEdit1, hLineNumbers1);
			}
			break;
		}

		case WM_TIMER: {
			switch (wParam) {
				case 1: {
					if (!FilePath.empty()) {
						SaveFile(hEdit1, FilePath);
					}
					break;
				}
				case IDT_UPD_LINE_NUM: 
					UpdateLineNumbers(hEdit1, hLineNumbers1);
					UpdateLineNumbers(hEdit2, hLineNumbers2);
					break;
			}
			break;
		}

		case WM_DROPFILES: {
			HDROP hDrop = reinterpret_cast<HDROP>(wParam);
			char szFileName[MAX_PATH];
			DragQueryFile(hDrop, 0, szFileName, MAX_PATH);
			FilePath = szFileName;
			DragFinish(hDrop);

			ifstream file(szFileName, ios::in);
			if (file.is_open()) {
				string fileContents((istreambuf_iterator<char>(file)), istreambuf_iterator<char>());
				file.close();

				fileContents = ConvertLineEndings(fileContents);
				SetWindowText(hEdit1, fileContents.c_str());
				FileChanged = false;
				saveCurrentEdit(hEdit1, undoStack1, redoStack1);
				UpdateLineNumbers(hEdit1, hLineNumbers1);
			} else {
				MessageBox(hwnd, "无法打开文件", "错误", MB_OK);
			}
			return 0;
		}

		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;

		case WM_DESTROY: {
			KillTimer(hwnd, IDT_UPD_LINE_NUM);
			KillTimer(hwnd, 1);
			if (hCustomFont != NULL) {
				DeleteObject(hCustomFont);
			}
			if (hExplorerFont != NULL) {
				DeleteObject(hExplorerFont);
			}
			PostQuitMessage(0);
			return 0;
		}

		case WM_SIZE: {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);

			MoveWindow(hLineNumbers1, 310, 10, 40, (height - 20) * 2 / 3, TRUE);
			MoveWindow(hEdit1, 360, 10, width - 360 - 40, (height - 20) * 2 / 3, TRUE);
			MoveWindow(hLineNumbers2, 310, (height - 20) * 2 / 3 + 10, 40, (height - 20) / 3, TRUE);
			MoveWindow(hEdit2, 360, (height - 20) * 2 / 3 + 10, width - 360 - 40, (height - 20) / 3, TRUE);
			MoveWindow(hTrackbar, 100, height - 40, width - 100, 40, TRUE);
			MoveWindow(hExplorerList, 10, 10, 250, height - 60, TRUE);
			MoveWindow(hUpButton, 10, height - 40, 250, 30, TRUE);
			return 0;
		}

		case WM_NOTIFY: {
			LPNMHDR lpnmh = reinterpret_cast<LPNMHDR>(lParam);
			if (lpnmh->code == EN_CHANGE) {
				if (lpnmh->hwndFrom == hEdit1) {
					UpdateLineNumbers(hEdit1, hLineNumbers1);
				} else if (lpnmh->hwndFrom == hEdit2) {
					UpdateLineNumbers(hEdit2, hLineNumbers2);
				}
			} else if (lpnmh->code == EN_VSCROLL || lpnmh->code == EN_HSCROLL) {
				if (lpnmh->hwndFrom == hEdit1 || lpnmh->hwndFrom == hEdit2) {
					UpdateLineNumbers(lpnmh->hwndFrom == hEdit1 ? hEdit1 : hEdit2, lpnmh->hwndFrom == hEdit1 ? hLineNumbers1 : hLineNumbers2);
				}
			}
			break;
		}

		case WM_MOUSEWHEEL: {
			if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
				int zDelta = static_cast<short>(HIWORD(wParam));
				if (zDelta > 0) {
					fontSize += 1;
				} else if (zDelta < 0) {
					fontSize -= 1;
				}
				fontSize = max(8, min(fontSize, 72));
				UpdateFontSize(hEdit1, fontSize);
				UpdateLineNumbers(hEdit1, hLineNumbers1);
			}
			break;
		}

		case WM_PAINT: {
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hwnd, &ps);

			if (hwnd == hLineNumbers1 || hwnd == hLineNumbers2) {
				SetBkMode(hdc, TRANSPARENT);
				SetTextColor(hdc, RGB(0, 0, 0));

				int len = GetWindowTextLength(hwnd);
				if (len > 0) {
					char* buffer = new char[len + 1];
					GetWindowText(hwnd, buffer, len + 1);
					string lineNumbers(buffer);
					delete[] buffer;

					DrawText(hdc, lineNumbers.c_str(), -1, &ps.rcPaint, DT_LEFT | DT_TOP | DT_SINGLELINE);
				}
			}

			EndPaint(hwnd, &ps);
			return 0;
		}

		case WM_KEYDOWN: {
			if (wParam == VK_CONTROL) {
				if (GetAsyncKeyState(VK_MENU) & 0x8000) {
					if (!EasterEggActive) {
						EasterEggActive = true;
						TriggerEasterEgg(hwnd);
					}
				}
			}
			break;
		}

		case WM_KEYUP: {
			if (wParam == VK_CONTROL || wParam == VK_MENU) {
				EasterEggActive = false;
			}
			break;
		}

		case WM_LBUTTONDBLCLK: {
			if ((HWND)lParam == hExplorerList) {
				int index = SendMessage(hExplorerList, LB_GETCURSEL, 0, 0);
				if (index != LB_ERR) {
					CHAR filename[260];
					SendMessage(hExplorerList, LB_GETTEXT, index, (LPARAM)filename);

					std::string cleanName(filename);
					size_t dirPos = cleanName.find("[DIR] ");
					if (dirPos != std::string::npos) {
						cleanName.erase(dirPos, 6);
					}

					fs::path selectedPath = currentPath / cleanName;

					if (fs::is_directory(selectedPath)) {
						currentPath = selectedPath;
						UpdateExplorerList(hExplorerList, currentPath);
					} else {
						ShellExecute(NULL, NULL, selectedPath.string().c_str(), NULL, NULL, SW_SHOWNORMAL);
					}
				}
			}
			return 0;
		}

		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}

	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	MSG Msg;

	wc.cbSize = sizeof(WNDCLASSEX);
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = hInstance;
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Dev Fanle Studio 启动失败，错误代码 0xc00741", "错误", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	hwnd = CreateWindowEx(
	           WS_EX_CLIENTEDGE,
	           g_szClassName,
	           "Dev Fanle Studio",
	           WS_OVERLAPPEDWINDOW,
	           CW_USEDEFAULT, CW_USEDEFAULT, 1200, 700,
	           NULL, NULL, hInstance, NULL
	       );

	if (hwnd == NULL) {
		MessageBox(NULL, "Dev Fanle Studio 启动失败，错误代码 0xc00825", "错误", MB_ICONEXCLAMATION | MB_OK);
		return -1;
	}

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	return Msg.wParam;
}
