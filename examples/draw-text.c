
#include <tchar.h>
#include <windows.h>

#include <wdl.h>


static HWND hwndMain = NULL;



static WCHAR pszTitle[] = L"Lorem Ipsum";
static WCHAR pszText[] =
    L"Lorem ipsum dolor sit amet, consectetuer adipiscing elit. Etiam dui "
    L"sem, fermentum vitae, sagittis id, malesuada in, quam. Morbi imperdiet, "
    L"mauris ac auctor dictum, nisl ligula egestas nulla, et sollicitudin "
    L"sem purus in lacus. Fusce aliquam vestibulum ipsum. Excepteur sint "
    L"occaecat cupidatat non proident, sunt in culpa qui officia deserunt "
    L"mollit anim id est laborum. Morbi imperdiet, mauris ac auctor dictum, "
    L"nisl ligula egestas nulla, et sollicitudin sem purus in lacus. "
    L"Excepteur sint occaecat cupidatat non proident, sunt in culpa qui "
    L"officia deserunt mollit anim id est laborum. Et harum quidem rerum "
    L"facilis est et expedita distinctio. Ut enim ad minim veniam, quis "
    L"nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo "
    L"consequat. Donec vitae arcu. Vivamus ac leo pretium faucibus.\n"
    L"\n"
    L"In dapibus augue non sapien. Aenean id metus id velit ullamcorper "
    L"pulvinar. Sed convallis magna eu sem. Maecenas sollicitudin. Nulla "
    L"turpis magna, cursus sit amet, suscipit a, interdum id, felis. Nulla "
    L"non lectus sed nisl molestie malesuada. Aliquam ante. Integer lacinia. "
    L"Nullam sapien sem, ornare ac, nonummy non, lobortis a enim. Integer "
    L"lacinia. Mauris metus. Fusce suscipit libero eget elit.\n"
    L"\n"
    L"Nam quis nulla. Fusce nibh. Nulla pulvinar eleifend sem. Curabitur "
    L"sagittis hendrerit ante. Cras pede libero, dapibus nec, pretium sit "
    L"amet, tempor quis. Nulla non lectus sed nisl molestie malesuada. "
    L"Maecenas sollicitudin. Duis condimentum augue id magna semper rutrum. "
    L"Proin mattis lacinia justo. Ut enim ad minima veniam, quis nostrum "
    L"exercitationem ullam corporis suscipit laboriosam, nisi ut aliquid "
    L"ex ea commodi consequatur? Integer in sapien. Nemo enim ipsam "
    L"voluptatem quia voluptas sit aspernatur aut odit aut fugit, sed quia "
    L"consequuntur magni dolores eos qui ratione voluptatem sequi nesciunt. "
    L"Fusce aliquam vestibulum ipsum. In enim a arcu imperdiet malesuada. "
    L"Maecenas ipsum velit, consectetuer eu lobortis ut, dictum at dui.\n"
    L"\n"
    L"Duis viverra diam non justo. Class aptent taciti sociosqu ad litora "
    L"torquent per conubia nostra, per inceptos hymenaeos. Mauris metus. "
    L"Nam libero tempore, cum soluta nobis est eligendi optio cumque nihil "
    L"impedit quo minus plus id quod maxime placeat facere possimus, omnis "
    L"voluptas assumenda est, omnis dolor repellendus. Class aptent taciti "
    L"sociosqu ad litora torquent per conubia nostra, per inceptos hymenaeos.";

static void
PaintToCanvas(WD_HCANVAS hCanvas)
{
    RECT client;
    WD_HBRUSH hBrush;
    WD_HFONT hFont;
    WD_RECT rect;
    WD_HTEXT hTitle;
    WD_HTEXT hText;
    WD_FONTMETRICS fontMetrics;
    WD_TEXTMETRICS titleMetrics;

    GetClientRect(hwndMain, &client);
    rect.x0 = 10.0f;
    rect.x1 = client.right - 10.0f;

    wdBeginPaint(hCanvas);
    wdClear(hCanvas, WD_RGB(255,255,255));
    hBrush = wdCreateSolidBrush(hCanvas, WD_RGB(0,0,0));
    hFont = wdCreateFontWithGdiHandle(GetStockObject(DEFAULT_GUI_FONT));
    wdFontMetrics(hFont, &fontMetrics);

    /* Create and paint title. */
    rect.y0 = 10.0f;
    rect.y1 = client.bottom - 10.0f;
    hTitle = wdCreateText(hFont, &rect, pszTitle, -1, 0);
    wdSetTextFontSize(hTitle, 0, wcslen(pszTitle), 1.66f * fontMetrics.fEmHeight);
    wdSetTextFontSize(hTitle, 0, 1, 1.85f * fontMetrics.fEmHeight);
    wdSetTextFontSize(hTitle, 6, 1, 1.85f * fontMetrics.fEmHeight);
    wdTextMetrics(hTitle, &titleMetrics);
    wdDrawText(hCanvas, hTitle, hBrush, rect.x0, rect.y0, 0);

    /* Create body text, customize some its parts, and paint it. */
    rect.y0 = 20.0f + titleMetrics.fHeight;
    rect.y1 = client.bottom - 10.0f;
    hText = wdCreateText(hFont, &rect, pszText, -1, 0);
    wdSetTextFontWeight(hText, 103, 12, FW_BOLD);
    wdSetTextFontStyle(hText, 932, 68, WD_TEXTSTYLE_ITALIC);
    wdSetTextStrikethrough(hText, 2159, 5, TRUE);
    wdSetTextUnderline(hText, 2165, 4, TRUE);
    wdDrawText(hCanvas, hText, hBrush, rect.x0, rect.y0, 0);

    wdDestroyText(hTitle);
    wdDestroyText(hText);
    wdDestroyBrush(hBrush);
    wdDestroyFont(hFont);
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

    wdInitialize(WD_INIT_TEXTAPI);

    /* Register main window class */
    wc.lpfnWndProc = MainWinProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszClassName = _T("main_window");
    wc.style = CS_HREDRAW | CS_VREDRAW;
    RegisterClass(&wc);

    /* Create main window */
    hwndMain = CreateWindow(
        _T("main_window"), _T("LibWinDraw Example: Draw Text"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 500, 650,
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

    wdTerminate(WD_INIT_TEXTAPI);

    /* Return exit code of WM_QUIT */
    return (int)msg.wParam;
}
