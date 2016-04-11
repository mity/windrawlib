
#include <tchar.h>
#include <windows.h>

#include <wdl.h>


static HWND hwndMain = NULL;
static WD_HCANVAS hCachedCanvas = NULL;


static void
MainWinPaintToCanvas(WD_HCANVAS hCanvas, BOOL* pbCanCache)
{
    BOOL bCanCache;
    WD_HBRUSH hBrush;

    struct {
        WD_RECT rect;
        WD_COLOR color;
    } r[] = {
        { { 10.0f, 10.0f, 110.0f, 110.0f }, WD_RGB(255,0,0) },
        { { 20.0f, 20.0f, 120.0f, 120.0f }, WD_RGB(0,255,0) },
        { { 30.0f, 30.0f, 130.0f, 130.0f }, WD_RGB(0,0,255) }
    };

    int i;

    wdBeginPaint(hCanvas);
    wdClear(hCanvas, WD_RGB(255,255,255));
    hBrush = wdCreateSolidBrush(hCanvas, 0);

    for(i = 0; i < sizeof(r) / sizeof(r[0]); i++) {
        wdSetSolidBrushColor(hBrush, r[i].color);
        wdDrawRect(hCanvas, hBrush, &r[i].rect, 1.0f);
    }

    wdDestroyBrush(hBrush);
    bCanCache = wdEndPaint(hCanvas);

    if(pbCanCache != NULL)
        *pbCanCache = bCanCache;
}

static void
MainWinPaint(void)
{
    PAINTSTRUCT ps;
    WD_HCANVAS hCanvas;

    BeginPaint(hwndMain, &ps);
    if(hCachedCanvas != NULL)
        hCanvas = hCachedCanvas;
    else
        hCanvas = wdCreateCanvasWithPaintStruct(hwndMain, &ps, WD_CANVAS_DOUBLEBUFFER);

    if(hCanvas != NULL) {
        BOOL bCanCache = FALSE;

        MainWinPaintToCanvas(hCanvas, &bCanCache);

        if(bCanCache) {
            hCachedCanvas = hCanvas;
        } else {
            wdDestroyCanvas(hCanvas);
            hCachedCanvas = NULL;
        }
    }
}

/* Main window procedure */
static LRESULT CALLBACK
MainWinProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg) {
        case WM_PAINT:
            MainWinPaint();
            break;

        case WM_SIZE:
            /* If we are caching the canvas for WM_PAINT, we need to resize it. */
            if(wParam == SIZE_RESTORED  ||  wParam == SIZE_MAXIMIZED) {
                if(hCachedCanvas != NULL)
                    wdResizeCanvas(hCachedCanvas, LOWORD(lParam), HIWORD(lParam));
            }
            return 0;

        case WM_DISPLAYCHANGE:
            if(hCachedCanvas != NULL) {
                wdDestroyCanvas(hCachedCanvas);
                hCachedCanvas = NULL;
            }
            InvalidateRect(hwndMain, NULL, FALSE);
            break;

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
        _T("main_window"), _T("LibWinDraw Example"),
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
