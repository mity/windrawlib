
#include <tchar.h>
#include <windows.h>

#include <wdl.h>


static HWND hwndMain = NULL;


static const WD_COLOR drawColors[3] =
        { WD_RGB(255,0,0), WD_RGB(0,255,0), WD_RGB(0,0,255) };
static const WD_COLOR fillColors[3] =
        { WD_ARGB(63,255,0,0), WD_ARGB(63,0,255,0), WD_ARGB(63,0,0,255) };

static void
MainWinPaintToCanvas(WD_HCANVAS hCanvas)
{
    WD_HBRUSH hBrush;
    int i;

    wdBeginPaint(hCanvas);
    wdClear(hCanvas, WD_RGB(255,255,255));
    hBrush = wdCreateSolidBrush(hCanvas, 0);

    for(i = 0; i < 3; i++) {
        float x = 10.0f + i * 20.0f;
        float y = 10.0f + i * 20.0f;
        
        wdTranslateWorld(hCanvas, x, y);

        wdSetSolidBrushColor(hBrush, drawColors[i]);
        wdDrawRect(hCanvas, hBrush, 0, 0, 50.0f, 50.0f, 3.0f);
    }

    wdResetWorld(hCanvas);

    for(i = 0; i < 3; i++) {
        float x = 200.0f;
        float y = 30.0f;

        wdRotateWorld(hCanvas, x + 50.0f, y + 50.0f, 15.0f * i);

        wdSetSolidBrushColor(hBrush, drawColors[i]);
        wdDrawRect(hCanvas, hBrush, x, y, x + 100.0f, y + 100.0f, 3.0f);
    }

    wdResetWorld(hCanvas);

    wdTranslateWorld(hCanvas, 350, 30);
    for(i = 0; i < 3; i++) {
        WD_MATRIX m;
        m.m11 = 1.0f + i * 0.2f;
        m.m12 = 0;
        m.m21 = 0;
        m.m22 = 1.0f - i * 0.2f;
        m.dx = 0;
        m.dy = 0;
        wdTransformWorld(hCanvas, &m);

        wdSetSolidBrushColor(hBrush, drawColors[i]);
        wdDrawRect(hCanvas, hBrush, 0, 0, 100.0f, 100.0f, 3.0f);
    }

    wdDestroyBrush(hBrush);
    wdEndPaint(hCanvas);
}


/* Main window procedure */
static LRESULT CALLBACK
MainWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            WD_HCANVAS hCanvas;

            BeginPaint(hwndMain, &ps);
            hCanvas = wdCreateCanvasWithPaintStruct(hwndMain, &ps, 0);
            MainWinPaintToCanvas(hCanvas);
            wdDestroyCanvas(hCanvas);
            EndPaint(hwndMain, &ps);
            return 0;
        }

        case WM_PRINTCLIENT:
        {
            HDC dc = (HDC) wParam;
            WD_HCANVAS hCanvas;

            hCanvas = wdCreateCanvasWithHDC(dc, NULL, 0);
            MainWinPaintToCanvas(hCanvas);
            wdDestroyCanvas(hCanvas);
            return 0;
        }

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


int APIENTRY
_tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = { 0 };
    MSG msg;

    wdPreInitialize(NULL, NULL, 0);
    wdInitialize(0);

    /* Register main window class */
    wc.lpfnWndProc = MainWinProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = _T("main_window");
    RegisterClass(&wc);

    /* Create main window */
    hwndMain = CreateWindow(
        _T("main_window"), _T("LibWinDraw Example: Simple Draw"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 550, 350,
        NULL, NULL, hInstance, NULL
    );
    SendMessage(hwndMain, WM_SETFONT, (WPARAM) GetStockObject(DEFAULT_GUI_FONT),
            MAKELPARAM(TRUE, 0));
    ShowWindow(hwndMain, nCmdShow);

    /* Message loop */
    while(GetMessage(&msg, NULL, 0, 0)) {
        if(IsDialogMessage(hwndMain, &msg))
            continue;

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    wdTerminate(0);

    /* Return exit code of WM_QUIT */
    return (int)msg.wParam;
}
