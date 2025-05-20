#include <windows.h>
#include <string>

// ��ȡ�������ı����ݵĺ���
bool GetClipboardText(HGLOBAL hGlobal, string& text) {
	// ��ȡȫ�ֶ����ָ��
	char* lpData = static_cast<char*>(GlobalLock(hGlobal));
	if (lpData == nullptr) {
		// ���GlobalLockʧ�ܣ�����false
		return false;
	}

	// �����ı����ݵ��ַ���
	text.assign(lpData);

	// ����ȫ�ֶ���
	GlobalUnlock(hGlobal);
	return true;
}

// ȫ�ֱ���
const char g_szClassName[] = "Fanle Studio";

// ���尴ť��ID
#define ID_BUTTON1 101
#define ID_BUTTON2 102
#define ID_BUTTON3 103
#define ID_BUTTON4 104

// ����4�����ڹ��̺���
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	switch (msg) {
		case ID_BUTTON1:
			system("newfile.exe");
			// ���Դ򿪼�����
			if (!OpenClipboard(NULL)) {
				MessageBox(NULL, "�޷��򿪼����塣", "����", MB_OK);
				return 0;
			}

			// �����������Ƿ����ı���ʽ������
			if (IsClipboardFormatAvailable(CF_TEXT)) {
				// �Ӽ������ȡ�ı����ݾ��
				HANDLE hClip = GetClipboardData(CF_TEXT);
				if (hClip != nullptr) {
					// ��ȡ�������ı�������洢��FilePath��
					if (GetClipboardText(hClip, FilePath)) {
						// ���Դ��ļ�
						FileContents.open(FilePath, ios::in); // ���ļ��Զ�ȡ
						if (FileContents.is_open()) {
							string line;
							string fileContents;
							while (getline(FileContents, line)) {
								fileContents += line + "\n"; // ��ȡ�ļ���ÿһ�в���ӵ��ַ�����
							}
							FileContents.close(); // ��ȡ��ɺ�ر��ļ�

							// ����ȡ����������ΪhEdit1������
							SetWindowText(hEdit1, fileContents.c_str());
							saveCurrentEdit(hEdit1, undoStack1, redoStack1); // ���浱ǰ�༭״̬������ջ
						} else {
							MessageBox(hwnd, "�޷����ļ�", "����", MB_OK);
						}
					} else {
						MessageBox(NULL, "�޷��Ӽ������ȡ�ļ�·����", "����", MB_OK);
					}
				}
			}

			// �رռ�����
			CloseClipboard();
			// ��ռ�����
			if (!OpenClipboard(NULL)) {
				MessageBox(NULL, "�޷����ļ�", "����", MB_OK);
				return 1;
			}
			if (!EmptyClipboard()) {
				MessageBox(NULL, "�޷����ļ�", "����", MB_OK);
			} else {
				// ����������գ������κβ���
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

// ����1��ע�ᴰ����
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
	WNDCLASSEX wc;
	HWND hwnd;
	HWND hButton1, hButton2, hButton3, hButton4, hStaticText;
	MSG Msg;

	// ע�ᴰ����
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

	// ����2����������
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

	// ������̬�ı��ؼ�
	hStaticText = CreateWindowEx(
	                  0, // ����չ��ʽ
	                  "STATIC",
	                  "��ʼʹ��",
	                  WS_VISIBLE | WS_CHILD | SS_SIMPLE,
	                  1000, 50, 400, 30, // λ�úʹ�С
	                  hwnd,
	                  NULL, // û��ID
	                  hInstance,
	                  NULL);

	// ������ť
	// ��ť1
	hButton1 = CreateWindow("BUTTON", "�½�", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 100, 400, 100, hwnd, (HMENU)ID_BUTTON1, hInstance, NULL);
	// ��ť2
	hButton2 = CreateWindow("BUTTON", "��", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 220, 400, 100, hwnd, (HMENU)ID_BUTTON2, hInstance, NULL);
	// ��ť3
	hButton3 = CreateWindow("BUTTON", "Button 3", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 340, 400, 100, hwnd, (HMENU)ID_BUTTON3, hInstance, NULL);
	// ��ť4
	hButton4 = CreateWindow("BUTTON", "Button 4", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, 1000, 460, 400, 100, hwnd, (HMENU)ID_BUTTON4, hInstance, NULL);

	ShowWindow(hwnd, nCmdShow);
	UpdateWindow(hwnd);

	// ��Ϣѭ��
	while (GetMessage(&Msg, NULL, 0, 0) > 0) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return Msg.wParam;
}
