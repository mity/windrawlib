
#include <tchar.h>
#include <windows.h>

#include <wdl.h>


static HWND hwndMain = NULL;


static void
PaintToCanvas(WD_HCANVAS hCanvas)
{
    WD_HBRUSH hBrush;

    struct {
        WD_RECT rect;
        WD_COLOR color;
        WD_COLOR fill_color;
    } r[] = {
        { { 10.0f, 10.0f, 110.0f, 110.0f }, WD_RGB(255,0,0), WD_ARGB(63,255,0,0) },
        { { 30.0f, 30.0f, 130.0f, 130.0f }, WD_RGB(0,255,0), WD_ARGB(63,0,255,0) },
        { { 50.0f, 50.0f, 150.0f, 150.0f }, WD_RGB(0,0,255), WD_ARGB(63,0,0,255) }
    };

    struct {
        WD_CIRCLE circle;
        WD_COLOR color;
        WD_COLOR fill_color;
    } c[] = {
        { { 250.0f,  60.0f, 55.0f }, WD_RGB(255,0,0), WD_ARGB(63,255,0,0) },
        { { 270.0f,  80.0f, 55.0f }, WD_RGB(0,255,0), WD_ARGB(63,0,255,0) },
        { { 290.0f, 100.0f, 55.0f }, WD_RGB(0,0,255), WD_ARGB(63,0,0,255) }
    };

    int i;

    wdBeginPaint(hCanvas);
    wdClear(hCanvas, WD_RGB(255,255,255));
    hBrush = wdCreateSolidBrush(hCanvas, 0);

    for(i = 0; i < sizeof(r) / sizeof(r[0]); i++) {
        wdSetSolidBrushColor(hBrush, r[i].color);
        wdDrawRect(hCanvas, hBrush, &r[i].rect, 3.0f);

        wdSetSolidBrushColor(hBrush, r[i].fill_color);
        wdFillRect(hCanvas, hBrush, &r[i].rect);
    }

    for(i = 0; i < sizeof(c) / sizeof(c[0]); i++) {
        wdSetSolidBrushColor(hBrush, c[i].color);
        wdDrawCircle(hCanvas, hBrush, &c[i].circle, 3.0f);

        wdSetSolidBrushColor(hBrush, c[i].fill_color);
        wdFillCircle(hCanvas, hBrush, &c[i].circle);
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
            PaintToCanvas(hCanvas);
            wdDestroyCanvas(hCanvas);
            EndPaint(hwndMain, &ps);
            return 0;
        }

        case WM_PRINTCLIENT:
        {
            HDC dc = (HDC) wParam;
            WD_HCANVAS hCanvas;

            hCanvas = wdCreateCanvasWithHDC(dc, NULL, 0);
            PaintToCanvas(hCanvas);
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
