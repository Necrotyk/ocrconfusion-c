// main.c

#include <windows.h>
#include <commctrl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdio.h>
#include "resource.h"

// Define constants
#define MAX_TEXT_LENGTH 128
#define DEFAULT_TEXT "c0d3h3r3"

// Globals for settings
COLORREF textColor = RGB(8, 46, 208);
COLORREF bgColor = RGB(19, 22, 26);
char displayedText[MAX_TEXT_LENGTH] = DEFAULT_TEXT;
int fontSize = 20;
BOOL enableNoise = FALSE;
int noiseLevel = 5;
BOOL loopAnimation = FALSE;


// Overlay related globals
HWND hOverlay = NULL;
HBRUSH hBackgroundBrush = NULL;
HFONT hFont = NULL;

// Tab dialog handles
static HWND hTabGeneral = NULL;
static HWND hTabAdvanced = NULL;

// Overlay position and size
RECT overlayRect = {200, 200, 1000, 600}; // Initial position and size

// Animation related
UINT_PTR animationTimer = 1;
int animationStep = 0;

// Dragging related
static BOOL isDraggingOverlay = FALSE;
static POINT ptDragStartOverlay;
static POINT ptOverlayStart;

// Forward declarations
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralTabProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AdvancedTabProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK SelectionWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void CreateOverlayWindow(HINSTANCE hInstance);
void ShowOverlay(HINSTANCE hInstance);
void HideOverlay();
void RedrawOverlay();
void DrawOverlay(HDC hdc, RECT *rect);
void CleanupOverlayResources();
void InitiateAreaSelection(HINSTANCE hInstance);
void StartAnimation();
void StopAnimation();
void AnimateOverlay();
void DrawNoise(HDC hdc, RECT *rect);
void DrawShapes(HDC hdc, RECT *rect);
void InitializeSpinBuddy(HWND hDlg);

// Declare selectedLogFont as a global variable
LOGFONT selectedLogFont = {0};

// Function to draw noise
void DrawNoise(HDC hdc, RECT *rect) {
    if (!enableNoise) return;

    for (int i = 0; i < noiseLevel; i++) {
        int x1 = rand() % (rect->right - rect->left);
        int y1 = rand() % (rect->bottom - rect->top);
        int x2 = rand() % (rect->right - rect->left);
        int y2 = rand() % (rect->bottom - rect->top);

        // Create a random color for noise
        COLORREF noiseColor = RGB(rand() % 256, rand() % 256, rand() % 256);
        HPEN hPen = CreatePen(PS_SOLID, 1, noiseColor);
        HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
        MoveToEx(hdc, x1, y1, NULL);
        LineTo(hdc, x2, y2);
        SelectObject(hdc, hOldPen);
        DeleteObject(hPen);
    }
}

// Global shapes count
int shapesCount = 5; // Default number of shapes

// Function to draw shapes
void DrawShapes(HDC hdc, RECT *rect) {
    for (int i = 0; i < shapesCount * 3; i++) { // Increase shape count
        int x1 = rand() % (rect->right - rect->left);
        int y1 = rand() % (rect->bottom - rect->top);
        int x2 = x1 + (rand() % 50) + 10; // Random size
        int y2 = y1 + (rand() % 50) + 10;

        COLORREF shapeColor = RGB(rand() % 256, rand() % 256, rand() % 256); // Random colors
        HBRUSH hBrush = CreateSolidBrush(shapeColor);
        HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);
        Rectangle(hdc, x1, y1, x2, y2);
        SelectObject(hdc, hOldBrush);
        DeleteObject(hBrush);
    }
}

// Function to handle animations (simple typing animation)
void StartAnimation() {
    // Reset animation step
    animationStep = 0;
    // Set a timer to trigger animation steps
    if (hOverlay) {
        SetTimer(hOverlay, animationTimer, 100, NULL); // 100 ms interval
    }
}

void StopAnimation() {
    if (hOverlay) {
        KillTimer(hOverlay, animationTimer);
    }
}

void AnimateOverlay() {
    // Simple typing animation: reveal one character at a time
    if (animationStep < strlen(displayedText)) {
        animationStep++;
        RedrawOverlay();
    } else if (loopAnimation) {
        // Reset the animation step if looping is enabled
        animationStep = 0;
        RedrawOverlay();
    } else {
        // Stop the timer when done
        StopAnimation();
    }
}


// Overlay window creation and drawing
void CreateOverlayWindow(HINSTANCE hInstance) {
    const char *CLASS_NAME = "OverlayWindowClass";

    WNDCLASSEX wc = {0};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = NULL; // We'll handle background drawing

    if (!RegisterClassEx(&wc)) {
        MessageBox(NULL, "Failed to register overlay class.", "Error", MB_ICONERROR);
        return;
    }

    hOverlay = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_TRANSPARENT,
        CLASS_NAME,
        "Overlay",
        WS_POPUP,
        overlayRect.left, overlayRect.top,
        overlayRect.right - overlayRect.left,
        overlayRect.bottom - overlayRect.top,
        NULL, NULL, hInstance, NULL
    );

    if (!hOverlay) {
        MessageBox(NULL, "Failed to create overlay window.", "Error", MB_ICONERROR);
        return;
    }

    BYTE transparency = 230; // Initial transparency
    SetLayeredWindowAttributes(hOverlay, 0, transparency, LWA_ALPHA);

    ShowWindow(hOverlay, SW_SHOW);
    UpdateWindow(hOverlay);
    RedrawOverlay();
}

void ShowOverlay(HINSTANCE hInstance) {
    if (!hOverlay) {
        CreateOverlayWindow(hInstance);
    } else {
        ShowWindow(hOverlay, SW_SHOW);
        RedrawOverlay();
    }

    // Start animation
    StartAnimation();
}

void HideOverlay() {
    if (hOverlay) {
        ShowWindow(hOverlay, SW_HIDE);
        StopAnimation();
    }
}

void ToggleOverlayButtonText(HWND hButton) {
    char btnText[20];
    GetWindowText(hButton, btnText, sizeof(btnText));
    if (strcmp(btnText, "Show Overlay") == 0) {
        SetWindowText(hButton, "Hide Overlay");
    } else {
        SetWindowText(hButton, "Show Overlay");
    }
}

LRESULT CALLBACK OverlayWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);
        if (!hdc) break;

        RECT rect;
        if (!GetClientRect(hwnd, &rect)) {
            EndPaint(hwnd, &ps);
            break;
        }

        // Fill background
        FillRect(hdc, &rect, hBackgroundBrush);

        // Draw text
        DrawOverlay(hdc, &rect);

        // Draw noise
        DrawNoise(hdc, &rect);

        // Draw shapes
        DrawShapes(hdc, &rect);

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_ERASEBKGND:
        return 1;

    case WM_DESTROY:
        return 0;

    case WM_TIMER:
        if (wParam == animationTimer) {
            AnimateOverlay();
        }
        return 0;

    case WM_LBUTTONDOWN:
    {
        isDraggingOverlay = TRUE;
        ptDragStartOverlay.x = LOWORD(lParam);
        ptDragStartOverlay.y = HIWORD(lParam);
        RECT rect;
        GetWindowRect(hwnd, &rect);
        ptOverlayStart.x = rect.left;
        ptOverlayStart.y = rect.top;
        SetCapture(hwnd);
        return 0;
    }

    case WM_MOUSEMOVE:
        if (isDraggingOverlay) {
            POINT pt;
            pt.x = LOWORD(lParam);
            pt.y = HIWORD(lParam);
            POINT ptScreen;
            GetCursorPos(&ptScreen);

            int dx = ptScreen.x - (overlayRect.left + ptDragStartOverlay.x);
            int dy = ptScreen.y - (overlayRect.top + ptDragStartOverlay.y);

            // Update overlayRect based on movement
            overlayRect.left += dx;
            overlayRect.top += dy;
            overlayRect.right += dx;
            overlayRect.bottom += dy;

            SetWindowPos(hwnd, HWND_TOPMOST, overlayRect.left, overlayRect.top,
                        overlayRect.right - overlayRect.left,
                        overlayRect.bottom - overlayRect.top,
                        SWP_NOACTIVATE | SWP_NOZORDER);

            RedrawOverlay();
        }
        return 0;

    case WM_LBUTTONUP:
        if (isDraggingOverlay) {
            isDraggingOverlay = FALSE;
            ReleaseCapture();
        }
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void DrawOverlay(HDC hdc, RECT *rect) {
    if (!hdc || !rect) return;

    SetTextColor(hdc, textColor);
    SetBkMode(hdc, TRANSPARENT);

    // Recreate font if needed
    if (hFont) {
        DeleteObject(hFont);
        hFont = NULL;
    }

    // Use the selected font if available
    if (selectedLogFont.lfFaceName[0]) {
        hFont = CreateFont(
            fontSize, 0, 0, 0,
            FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, selectedLogFont.lfFaceName
        );
    } else {
        hFont = CreateFont(
            fontSize, 0, 0, 0,
            FW_BOLD, FALSE, FALSE, FALSE,
            ANSI_CHARSET, OUT_TT_PRECIS,
            CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
            DEFAULT_PITCH | FF_SWISS, "Arial"
        );
    }

    if (!hFont) return;

    HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
    if (!hOldFont) return;

    SIZE textSize;
    int displayLength = animationStep > 0 ? animationStep : strlen(displayedText);
    if (displayLength > strlen(displayedText)) displayLength = strlen(displayedText);

    if (!GetTextExtentPoint32(hdc, displayedText, displayLength, &textSize)) {
        SelectObject(hdc, hOldFont);
        return;
    }

    // Calculate base position
    int baseX = (rect->right - textSize.cx) / 2;
    int baseY = (rect->bottom - textSize.cy) / 2;

    // Apply jittered drawing for each character
    for (int i = 0; i < displayLength; i++) {
        int offsetX = rand() % 5 - 2; // Random horizontal jitter (-2 to 2)
        int offsetY = rand() % 5 - 2; // Random vertical jitter (-2 to 2)

        // Calculate position of each character
        int charWidth = textSize.cx / displayLength;
        int charX = baseX + i * charWidth + offsetX;
        int charY = baseY + offsetY;

        // Draw one character at a time with offset
        TextOut(hdc, charX, charY, &displayedText[i], 1);
    }

    SelectObject(hdc, hOldFont);
}


void RedrawOverlay() {
    if (hOverlay) {
        InvalidateRect(hOverlay, NULL, TRUE);
        UpdateWindow(hOverlay);
    }
}

void CleanupOverlayResources() {
    if (hFont) {
        DeleteObject(hFont);
        hFont = NULL;
    }

    if (hBackgroundBrush) {
        DeleteObject(hBackgroundBrush);
        hBackgroundBrush = NULL;
    }

    if (hOverlay) {
        DestroyWindow(hOverlay);
        hOverlay = NULL;
    }
}

// Selection window procedure
LRESULT CALLBACK SelectionWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static POINT ptStart = {0, 0};
    static POINT ptEnd = {0, 0};
    static BOOL isSelecting = FALSE;

    switch (uMsg) {
    case WM_LBUTTONDOWN:
        ptStart.x = LOWORD(lParam);
        ptStart.y = HIWORD(lParam);
        isSelecting = TRUE;
        SetCapture(hwnd);
        return 0;

    case WM_MOUSEMOVE:
        if (isSelecting) {
            ptEnd.x = LOWORD(lParam);
            ptEnd.y = HIWORD(lParam);
            InvalidateRect(hwnd, NULL, TRUE);
        }
        return 0;

    case WM_LBUTTONUP:
        if (isSelecting) {
            ptEnd.x = LOWORD(lParam);
            ptEnd.y = HIWORD(lParam);
            isSelecting = FALSE;
            ReleaseCapture();

            // Calculate selected rectangle
            RECT selectedRect;
            selectedRect.left = min(ptStart.x, ptEnd.x);
            selectedRect.top = min(ptStart.y, ptEnd.y);
            selectedRect.right = max(ptStart.x, ptEnd.x);
            selectedRect.bottom = max(ptStart.y, ptEnd.y);

            // Convert to screen coordinates
            POINT pt1 = { selectedRect.left, selectedRect.top };
            POINT pt2 = { selectedRect.right, selectedRect.bottom };
            ClientToScreen(hwnd, &pt1);
            ClientToScreen(hwnd, &pt2);

            selectedRect.left = pt1.x;
            selectedRect.top = pt1.y;
            selectedRect.right = pt2.x;
            selectedRect.bottom = pt2.y;

            // Update overlay position and size
            overlayRect = selectedRect;

            // Move and resize overlay window
            SetWindowPos(hOverlay, HWND_TOPMOST, overlayRect.left, overlayRect.top,
                        overlayRect.right - overlayRect.left,
                        overlayRect.bottom - overlayRect.top,
                        SWP_NOACTIVATE | SWP_NOZORDER);

            RedrawOverlay();

            // Close selection window
            DestroyWindow(hwnd);
        }
        return 0;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw selection rectangle
        if (isSelecting) {
            HPEN hPen = CreatePen(PS_DASH, 1, RGB(255, 0, 0));
            HBRUSH hBrush = (HBRUSH)GetStockObject(NULL_BRUSH); // Transparent brush
            HPEN hOldPen = (HPEN)SelectObject(hdc, hPen);
            HBRUSH hOldBrush = (HBRUSH)SelectObject(hdc, hBrush);

            RECT rect;
            rect.left = min(ptStart.x, ptEnd.x);
            rect.top = min(ptStart.y, ptEnd.y);
            rect.right = max(ptStart.x, ptEnd.x);
            rect.bottom = max(ptStart.y, ptEnd.y);

            // Draw rectangle outline
            Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);

            SelectObject(hdc, hOldPen);
            SelectObject(hdc, hOldBrush);

            DeleteObject(hPen);
            DeleteObject(hBrush);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }

    case WM_DESTROY:
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

// Function to initiate area selection
void InitiateAreaSelection(HINSTANCE hInstance) {
    const char *CLASS_NAME = "SelectionWindowClass";
    WNDCLASSEX wc = {0};

    // Check if the class is already registered
    if (!GetClassInfoEx(hInstance, CLASS_NAME, &wc)) {
        // Class not registered yet, proceed to register
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.lpfnWndProc = SelectionWndProc;
        wc.hInstance = hInstance;
        wc.lpszClassName = CLASS_NAME;
        wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
        wc.hCursor = LoadCursor(NULL, IDC_CROSS); // Use cross cursor

        if (!RegisterClassEx(&wc)) {
            MessageBox(NULL, "Failed to register selection window class.", "Error", MB_ICONERROR);
            return;
        }
    }

    // Create a fullscreen window to cover the entire screen for selection
    HWND hSelectionWnd = CreateWindowEx(
        WS_EX_LAYERED | WS_EX_TOPMOST,
        CLASS_NAME,
        "Select Area",
        WS_POPUP,
        0, 0,
        GetSystemMetrics(SM_CXSCREEN),
        GetSystemMetrics(SM_CYSCREEN),
        NULL, NULL, hInstance, NULL
    );

    if (!hSelectionWnd) {
        MessageBox(NULL, "Failed to create selection window.", "Error", MB_ICONERROR);
        return;
    }

    // Make the window semi-transparent
    SetLayeredWindowAttributes(hSelectionWnd, 0, 128, LWA_ALPHA);

    ShowWindow(hSelectionWnd, SW_SHOW);
    UpdateWindow(hSelectionWnd);
}

// Function to initialize the spin control buddy
void InitializeSpinBuddy(HWND hDlg) {
    HWND hSpin = GetDlgItem(hDlg, IDC_SPIN_NOISE);
    HWND hEdit = GetDlgItem(hDlg, IDC_EDIT_NOISE);
    SendMessage(hSpin, UDM_SETBUDDY, (WPARAM)hEdit, 0);
    SendMessage(hSpin, UDM_SETRANGE, 0, MAKELPARAM(100, 0)); // Set range 0-100
    SendMessage(hSpin, UDM_SETPOS, 0, MAKELPARAM(noiseLevel, 0)); // Set initial position
}

// Main dialog procedure
INT_PTR CALLBACK MainDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hTabCtrl;
    static HINSTANCE hInst;

    switch (message) {
    case WM_INITDIALOG: {
        hInst = (HINSTANCE)GetModuleHandle(NULL);

        hTabCtrl = GetDlgItem(hDlg, IDC_TABCTRL);

        // Add tabs
        TCITEM tie;
        tie.mask = TCIF_TEXT;

        tie.pszText = "General";
        TabCtrl_InsertItem(hTabCtrl, 0, &tie);

        tie.pszText = "Advanced";
        TabCtrl_InsertItem(hTabCtrl, 1, &tie);

        // Create the child dialogs for each tab
        hTabGeneral = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TAB_GENERAL), hDlg, GeneralTabProc);
        hTabAdvanced = CreateDialog(hInst, MAKEINTRESOURCE(IDD_TAB_ADVANCED), hDlg, AdvancedTabProc);

        // Position the tab dialogs within the tab control
        RECT rc;
        GetClientRect(hTabCtrl, &rc);
        TabCtrl_AdjustRect(hTabCtrl, FALSE, &rc);

        SetWindowPos(hTabGeneral, NULL, rc.left + 5, rc.top + 5,
                     rc.right - rc.left - 10, rc.bottom - rc.top - 10,
                     SWP_SHOWWINDOW);
        SetWindowPos(hTabAdvanced, NULL, rc.left + 5, rc.top + 5,
                     rc.right - rc.left - 10, rc.bottom - rc.top - 10,
                     SWP_HIDEWINDOW);

        // Initialize background brush (if needed)
        if (!hBackgroundBrush) {
            hBackgroundBrush = CreateSolidBrush(bgColor);
        }

        // Set initial button text
        SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_TOGGLE_OVERLAY), "Show Overlay");

        return TRUE;
    }

    case WM_NOTIFY: {
        NMHDR* nmh = (NMHDR*)lParam;
        if (nmh->idFrom == IDC_TABCTRL && nmh->code == TCN_SELCHANGE) {
            int iPage = TabCtrl_GetCurSel(GetDlgItem(hDlg, IDC_TABCTRL));
            if (iPage == 0) {
                ShowWindow(hTabGeneral, SW_SHOW);
                ShowWindow(hTabAdvanced, SW_HIDE);
            }
            else {
                ShowWindow(hTabGeneral, SW_HIDE);
                ShowWindow(hTabAdvanced, SW_SHOW);
            }
        }
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == IDC_BUTTON_TOGGLE_OVERLAY && HIWORD(wParam) == BN_CLICKED) {
            // Toggle overlay visibility
            if (hOverlay && IsWindowVisible(hOverlay)) {
                HideOverlay();
                SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_TOGGLE_OVERLAY), "Show Overlay");
            }
            else {
                ShowOverlay(hInst);
                SetWindowText(GetDlgItem(hDlg, IDC_BUTTON_TOGGLE_OVERLAY), "Hide Overlay");
            }
        }
        break;
    }

    case WM_SIZE: {
        if (hTabCtrl) {
            RECT rcClient;
            GetClientRect(hDlg, &rcClient);

            // Resize the tab control to fit the new window size with some padding
            int padding = 10;
            SetWindowPos(hTabCtrl, NULL, padding, padding,
                        rcClient.right - 2 * padding, rcClient.bottom - 60, // Adjust height as needed
                        SWP_NOZORDER | SWP_NOACTIVATE);

            // Adjust child dialogs within the tab control
            RECT rcTab;
            GetClientRect(hTabCtrl, &rcTab);
            TabCtrl_AdjustRect(hTabCtrl, FALSE, &rcTab);

            // Position General Tab
            SetWindowPos(hTabGeneral, NULL, rcTab.left + 5, rcTab.top + 5,
                         rcTab.right - rcTab.left - 10, rcTab.bottom - rcTab.top - 10,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);

            // Position Advanced Tab
            SetWindowPos(hTabAdvanced, NULL, rcTab.left + 5, rcTab.top + 5,
                         rcTab.right - rcTab.left - 10, rcTab.bottom - rcTab.top - 10,
                         SWP_NOZORDER | SWP_NOACTIVATE | SWP_NOREDRAW);
        }
        return TRUE;
    }

    case WM_CLOSE:
        CleanupOverlayResources();
        EndDialog(hDlg, 0);
        return TRUE;
    }

    return FALSE;
}

// General tab procedure
INT_PTR CALLBACK GeneralTabProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hEditText, hSliderFont, hButtonSelectArea, hButtonColorBG, hButtonColorTxt, hSliderOpacity, hButtonChooseFont;

    switch (message) {
    case WM_INITDIALOG: {
        hEditText = GetDlgItem(hDlg, IDC_EDIT_TEXT);
        hSliderFont = GetDlgItem(hDlg, IDC_SLIDER_FONT);
        hButtonSelectArea = GetDlgItem(hDlg, IDC_BUTTON_SELECTAREA);
        hButtonColorBG = GetDlgItem(hDlg, IDC_BUTTON_COLORBG);
        hButtonColorTxt = GetDlgItem(hDlg, IDC_BUTTON_COLORTXT);
        hSliderOpacity = GetDlgItem(hDlg, IDC_SLIDER_OPACITY);
        hButtonChooseFont = GetDlgItem(hDlg, IDC_BUTTON_CHOOSEFONT);

        // Initialize controls
        SetWindowText(hEditText, displayedText);
        SendMessage(hSliderFont, TBM_SETRANGE, TRUE, MAKELPARAM(10, 100));
        SendMessage(hSliderFont, TBM_SETPOS, TRUE, fontSize);

        SendMessage(hSliderOpacity, TBM_SETRANGE, TRUE, MAKELPARAM(50, 255)); // Opacity from 50 to 255
        SendMessage(hSliderOpacity, TBM_SETPOS, TRUE, 230); // Default opacity

        // Initialize background brush
        if (!hBackgroundBrush) {
            hBackgroundBrush = CreateSolidBrush(bgColor);
        }

        // Start animation
        StartAnimation();

        return TRUE;
    }

    case WM_COMMAND:
        if (LOWORD(wParam) == IDC_BUTTON_COLORBG && HIWORD(wParam) == BN_CLICKED) {
            // Open color picker for background color
            COLORREF chosenColor;
            CHOOSECOLOR cc = {0};
            static COLORREF acrCustClr[16];
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)acrCustClr;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = bgColor;

            if (ChooseColor(&cc)) {
                bgColor = cc.rgbResult;
                DeleteObject(hBackgroundBrush);
                hBackgroundBrush = CreateSolidBrush(bgColor);
                RedrawOverlay();
            }
        }
        else if (LOWORD(wParam) == IDC_BUTTON_COLORTXT && HIWORD(wParam) == BN_CLICKED) {
            // Open color picker for text color
            COLORREF chosenColor;
            CHOOSECOLOR cc = {0};
            static COLORREF acrCustClr[16];
            cc.lStructSize = sizeof(cc);
            cc.hwndOwner = hDlg;
            cc.lpCustColors = (LPDWORD)acrCustClr;
            cc.Flags = CC_FULLOPEN | CC_RGBINIT;
            cc.rgbResult = textColor;

            if (ChooseColor(&cc)) {
                textColor = cc.rgbResult;
                RedrawOverlay();
            }
        }
        else if (LOWORD(wParam) == IDC_BUTTON_SELECTAREA && HIWORD(wParam) == BN_CLICKED) {
            // Initiate area selection
            HINSTANCE hInstance = (HINSTANCE)GetWindowLongPtr(hDlg, GWLP_HINSTANCE);
            InitiateAreaSelection(hInstance);
        }
        else if (LOWORD(wParam) == IDC_BUTTON_CHOOSEFONT && HIWORD(wParam) == BN_CLICKED) {
            // Implement font selection
            CHOOSEFONT cf = {0};
            cf.lStructSize = sizeof(cf);
            cf.hwndOwner = hDlg;
            cf.lpLogFont = &selectedLogFont;
            cf.Flags = CF_SCREENFONTS | CF_INITTOLOGFONTSTRUCT | CF_FORCEFONTEXIST;
            cf.rgbColors = textColor;

            if (ChooseFont(&cf)) {
                // Apply selected font
                fontSize = abs(selectedLogFont.lfHeight); // Font height
                strcpy_s(displayedText, MAX_TEXT_LENGTH, "c0d3h3r3"); // Reset text for demo
                animationStep = 0; // Reset animation
                StartAnimation(); // Restart animation
                RedrawOverlay();
            }
        }
        else if (LOWORD(wParam) == IDC_EDIT_TEXT && HIWORD(wParam) == EN_CHANGE) {
            char buf[MAX_TEXT_LENGTH];
            GetWindowText(hEditText, buf, MAX_TEXT_LENGTH);
            strcpy_s(displayedText, MAX_TEXT_LENGTH, buf);
            animationStep = 0; // Reset animation
            StartAnimation(); // Restart animation
            RedrawOverlay();
        }
        return TRUE;

    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_FONT)) {
            fontSize = (int)SendMessage(GetDlgItem(hDlg, IDC_SLIDER_FONT), TBM_GETPOS, 0, 0);
            RedrawOverlay();
        }
        else if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_OPACITY)) {
            BYTE newOpacity = (BYTE)SendMessage(GetDlgItem(hDlg, IDC_SLIDER_OPACITY), TBM_GETPOS, 0, 0);
            SetLayeredWindowAttributes(hOverlay, 0, newOpacity, LWA_ALPHA);
        }
        return TRUE;
    }
    return FALSE;
}

// Advanced tab procedure
INT_PTR CALLBACK AdvancedTabProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static HWND hCheckNoise, hEditNoise, hSpinNoise, hButtonApply, hSliderAnim, hButtonRandom, hSliderShapes;

    switch (message) {
    case WM_INITDIALOG: {
        hCheckNoise = GetDlgItem(hDlg, IDC_CHECK_NOISE);
        hEditNoise = GetDlgItem(hDlg, IDC_EDIT_NOISE);
        hSpinNoise = GetDlgItem(hDlg, IDC_SPIN_NOISE);
        hButtonApply = GetDlgItem(hDlg, IDC_BUTTON_APPLY);
        hSliderAnim = GetDlgItem(hDlg, IDC_SLIDER_ANIM);
        hButtonRandom = GetDlgItem(hDlg, IDC_BUTTON_RANDOM);
        hSliderShapes = GetDlgItem(hDlg, IDC_SLIDER_SHAPES);

        // Initialize controls
        SendMessage(hCheckNoise, BM_SETCHECK, enableNoise ? BST_CHECKED : BST_UNCHECKED, 0);
        char buf[16];
        sprintf_s(buf, sizeof(buf), "%d", noiseLevel);
        SetWindowText(hEditNoise, buf);

        // Initialize spin control buddy
        InitializeSpinBuddy(hDlg);

        // Initialize sliders
        SendMessage(hSliderAnim, TBM_SETRANGE, TRUE, MAKELPARAM(1, 100)); // Animation speed range: 1-100
        SendMessage(hSliderAnim, TBM_SETPOS, TRUE, 50); // Default speed

        SendMessage(hSliderShapes, TBM_SETRANGE, TRUE, MAKELPARAM(1, 20)); // Shapes count range: 1-20
        SendMessage(hSliderShapes, TBM_SETPOS, TRUE, shapesCount); // Default shapes count

        return TRUE;
    }

    case WM_COMMAND:

        if (LOWORD(wParam) == IDC_CHECK_LOOP && HIWORD(wParam) == BN_CLICKED) {
            loopAnimation = (SendMessage(GetDlgItem(hDlg, IDC_CHECK_LOOP), BM_GETCHECK, 0, 0) == BST_CHECKED);
    }
        if (LOWORD(wParam) == IDC_BUTTON_APPLY && HIWORD(wParam) == BN_CLICKED) {
            // Read current values
            enableNoise = (SendMessage(hCheckNoise, BM_GETCHECK, 0, 0) == BST_CHECKED);

            char buf[16];
            GetWindowText(hEditNoise, buf, 16);
            noiseLevel = atoi(buf);

            // Read animation speed and shapes
            int animSpeed = (int)SendMessage(hSliderAnim, TBM_GETPOS, 0, 0);
            shapesCount = (int)SendMessage(hSliderShapes, TBM_GETPOS, 0, 0);

            // Clamp animSpeed to prevent very fast intervals
            if (animSpeed < 1) animSpeed = 1;
            if (animSpeed > 100) animSpeed = 100;

            // Update animation speed
            if (animSpeed > 0) {
                KillTimer(hOverlay, animationTimer);
                SetTimer(hOverlay, animationTimer, 1000 / animSpeed, NULL); // Adjust timer based on speed
            }

            // Redraw overlay with new settings
            RedrawOverlay();
        }
        else if (LOWORD(wParam) == IDC_BUTTON_RANDOM && HIWORD(wParam) == BN_CLICKED) {
            // Implement randomization logic
            // Example: Randomize text color, background color, font size, etc.
            textColor = RGB(rand() % 256, rand() % 256, rand() % 256);
            bgColor = RGB(rand() % 256, rand() % 256, rand() % 256);
            fontSize = (rand() % 90) + 10; // Random font size between 10 and 100
            enableNoise = rand() % 2;
            noiseLevel = rand() % 101;
            int animSpeed = (rand() % 100) + 1; // 1 to 100
            shapesCount = (rand() % 20) + 1; // 1 to 20

            // Update controls to reflect random values
            char tempBuf[16];
            sprintf_s(tempBuf, sizeof(tempBuf), "%d", noiseLevel);
            SetWindowText(hEditNoise, tempBuf);
            SendMessage(hCheckNoise, BM_SETCHECK, enableNoise ? BST_CHECKED : BST_UNCHECKED, 0);
            SendMessage(GetDlgItem(hDlg, IDC_SLIDER_FONT), TBM_SETPOS, TRUE, fontSize);
            SendMessage(GetDlgItem(hDlg, IDC_SLIDER_ANIM), TBM_SETPOS, TRUE, animSpeed);
            SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SHAPES), TBM_SETPOS, TRUE, shapesCount);

            // Clamp animSpeed
            if (animSpeed < 1) animSpeed = 1;
            if (animSpeed > 100) animSpeed = 100;

            // Update animation speed
            if (animSpeed > 0) {
                KillTimer(hOverlay, animationTimer);
                SetTimer(hOverlay, animationTimer, 1000 / animSpeed, NULL); // Adjust timer based on speed
            }

            // Update overlay
            DeleteObject(hBackgroundBrush);
            hBackgroundBrush = CreateSolidBrush(bgColor);
            RedrawOverlay();
        }
        return TRUE;

    case WM_HSCROLL:
        if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_ANIM)) {
            // Handle animation speed changes
            int animSpeed = (int)SendMessage(GetDlgItem(hDlg, IDC_SLIDER_ANIM), TBM_GETPOS, 0, 0);
            if (animSpeed < 1) animSpeed = 1; // Clamp to prevent division by zero
            if (animSpeed > 100) animSpeed = 100; // Clamp to upper limit

            KillTimer(hOverlay, animationTimer);
            SetTimer(hOverlay, animationTimer, 1000 / animSpeed, NULL); // Adjust timer based on speed

            return TRUE;
        }
        else if ((HWND)lParam == GetDlgItem(hDlg, IDC_SLIDER_SHAPES)) {
            // Handle shapes slider changes
            shapesCount = (int)SendMessage(GetDlgItem(hDlg, IDC_SLIDER_SHAPES), TBM_GETPOS, 0, 0);
            RedrawOverlay();
            return TRUE;
        }
        return TRUE;
    }
    return FALSE;
}

// Function to draw text
// Note: This function is already defined above. Ensure it's defined only once.


// Function to draw shapes
// Note: This function is already defined above. Ensure it's defined only once.

// Function to initialize the spin control buddy
// Note: This function is already defined above. Ensure it's defined only once.

// Main application entry point
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    // Seed the random number generator
    srand((unsigned int)time(NULL));

    // Initialize common controls (needed for tab control, sliders, etc.)
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(icex);
    icex.dwICC = ICC_WIN95_CLASSES | ICC_BAR_CLASSES | ICC_UPDOWN_CLASS | ICC_PROGRESS_CLASS;
    InitCommonControlsEx(&icex);

    // Show the main dialog
    DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAIN), NULL, MainDlgProc);

    return 0;
}
