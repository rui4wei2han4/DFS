#include <windows.h>
#include <string>

// 获取剪贴板文本内容的函数
bool GetClipboardText(HGLOBAL hGlobal, string& text) {
	// 获取全局对象的指针
	char* lpData = static_cast<char*>(GlobalLock(hGlobal));
	if (lpData == nullptr) {
		// 如果GlobalLock失败，返回false
		return false;
	}

	// 复制文本内容到字符串
	text.assign(lpData);

	// 解锁全局对象
	GlobalUnlock(hGlobal);
	return true;
}

// 全局变量
const char g_szClassName[] = "Fanle Studio";

// 定义按钮的ID
#define ID_BUTTON1 101
#define ID_BUTTON2 102
#define ID_BUTTON3 103
#define ID_BUTTON4 104

// 步骤4：窗口过程函数
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case ID_BUTTON1:
			system("newfile.exe");
			// 尝试打开剪贴板
			if (!OpenClipboard(NULL)) {
				MessageBox(NULL, "无法打开剪贴板。", "错误", MB_OK);
				return 0;
			}

			// 检查剪贴板中是否有文本格式的数据
			if (IsClipboardFormatAvailable(CF_TEXT)) {
				// 从剪贴板获取文本数据句柄
				HANDLE hClip = GetClipboardData(CF_TEXT);
				if (hClip != nullptr) {
					// 获取剪贴板文本并将其存储在FilePath中
					if (GetClipboardText(hClip, FilePath)) {
						// 尝试打开文件
						FileContents.open(FilePath, ios::in); // 打开文件以读取
						if (FileContents.is_open()) {
							string line;
							string fileContents;
							while (getline(FileContents, line)) {
								fileContents += line + "\n"; // 读取文件的每一行并添加到字符串中
							}
							FileContents.close(); // 读取完成后关闭文件

							// 将读取的内容设置为hEdit1的内容
							SetWindowText(hEdit1, fileContents.c_str());
							saveCurrentEdit(hEdit1, undoStack1, redoStack1); // 保存当前编辑状态到撤销栈
						} else {
							MessageBox(hwnd, "无法打开文件", "错误", MB_OK);
						}
					} else {
						MessageBox(NULL, "无法从剪贴板获取文件路径。", "错误", MB_OK);
					}
				}
			}

			// 关闭剪贴板
			CloseClipboard();
			// 清空剪贴板
			if (!OpenClipboard(NULL)) {
				MessageBox(NULL, "无法打开文件", "错误", MB_OK);
				return 1;
			}
			if (!EmptyClipboard()) {
				MessageBox(NULL, "无法打开文件", "错误", MB_OK);
			} else {
				// 剪贴板已清空，不做任何操作
			}
			CloseClipboard();
			break;
		case WM_CLOSE:
			DestroyWindow(hwnd);
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hwnd, msg, wParam, lParam);
	}
	return 0;
}

// 步骤1：注册窗口类
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	HWND hButton1, hButton2, hButton3, hButton4, hStaticText;
	MSG Msg;

	// 注册窗口类
	wc.cbSize        = sizeof(WNDCLASSEX);
	wc.style         = 0;
	wc.lpfnWndProc   = WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = NULL;
	wc.lpszClassName = g_szClassName;
	wc.hIconSm       = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wc)) {
		MessageBox(NULL, "Window Registration Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// 步骤2：创建窗口
	hwnd = CreateWindowEx(
	           WS_EX_CLIENTEDGE,
	           g_szClassName,
	           "Fanle Studio",
	           WS_OVERLAPPEDWINDOW,
	           CW_USEDEFAULT, CW_USEDEFAULT, 1500, 1000,
	           NULL, NULL, hInstance, NULL);

	if (hwnd == NULL) {
		MessageBox(NULL, "Window Creation Failed!", "Error!", MB_ICONEXCLAMATION | MB_OK);
		return 0;
	}

	// 创建静态文本控件
	hStaticText = CreateWindowEx(
	                  0, // 无扩展样式
	                  "STATIC",
	                  "开始使用",
	                  WS_VISIBLE | WS_CHILD | SS_SIMPLE,
	                  1000, 50, 400, 30, // 位置和大小
	                  hwnd,
	                  NULL, // 没有ID
	                  hInstance,
	                  NULL);

	// 创建按钮
	// 按钮1
	hButton1 = CreateWindow("BUTTON", "新建", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 100, 400, 100, hwnd, (HMENU)ID_BUTTON1, hInstance, NULL);
	// 按钮2
	hButton2 = CreateWindow("BUTTON", "打开", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 220, 400, 100, hwnd, (HMENU)ID_BUTTON2, hInstance, NULL);
	// 按钮3
	hButton3 = CreateWindow("BUTTON", "Button 3", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 340, 400, 100, hwnd, (HMENU)ID_BUTTON3, hInstance, NULL);
	// 按钮4
	hButton4 = CreateWindow("BUTTON", "Button 4", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 460, 400, 100, hwnd, (HMENU)ID_BUTTON4, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// 消息循环
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
