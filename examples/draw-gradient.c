
#include <tchar.h>
#include <windows.h>

#include <wdl.h>


static HWND hwndMain = NULL;


static const WD_COLOR stopColors[3] =
        { WD_RGB(255,0,0), WD_RGB(0,255,0), WD_RGB(0,0,255) };
static float stopOffsets[3] = { 0, 0.5f, 1.0f };

static void
MainWinPaintToCanvas(WD_HCANVAS hCanvas)
{
    WD_HBRUSH hBrush;
    int i;

    wdBeginPaint(hCanvas);
    wdClear(hCanvas, WD_RGB(255,255,255));

    hBrush = wdCreateLinearGradientBrushEx(hCanvas, 10.0f, 10.0f, 110.0f, 110.0f,
                stopColors, stopOffsets, 3);
    wdFillRect(hCanvas, hBrush, 10.0f, 10.0f, 110.0f, 110.0f);
    wdDestroyBrush(hBrush);

    hBrush = wdCreateLinearGradientBrush(hCanvas, 130.0f, 10.0f, stopColors[0],
                230.0f, 110.0f, stopColors[1]);
    wdFillRect(hCanvas, hBrush, 130.0f, 10.0f, 230.0f, 110.0f);
    wdDestroyBrush(hBrush);

    hBrush = wdCreateRadialGradientBrushEx(hCanvas, 60.0f, 170.0f, 50.0f,
                80.0f, 190.0f, stopColors, stopOffsets, 3);
    wdFillRect(hCanvas, hBrush, 10.0f, 120.0f, 110.0f, 220.0f);
    wdDestroyBrush(hBrush);

    hBrush = wdCreateRadialGradientBrush(hCanvas, 180.0f, 170.0f, 50.0f,
                stopColors[0], stopColors[1]);
    wdFillRect(hCanvas, hBrush, 130.0f, 120.0f, 230.0f, 220.0f);
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
