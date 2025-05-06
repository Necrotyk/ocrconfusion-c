#ifndef RESOURCE_H
#define RESOURCE_H

// Dialog Identifiers
#define IDD_MAIN         201
#define IDD_TAB_GENERAL  202
#define IDD_TAB_ADVANCED 203

// Controls on the main dialog
#define IDC_TABCTRL               3001
#define IDC_BUTTON_TOGGLE_OVERLAY 3107
#define IDC_BUTTON_CHOOSEFONT     3110

// General tab controls
#define IDC_EDIT_TEXT         3101
#define IDC_SLIDER_FONT       3102
#define IDC_BUTTON_COLORBG    3103
#define IDC_BUTTON_COLORTXT   3104
#define IDC_SLIDER_OPACITY    3105
#define IDC_BUTTON_SELECTAREA 3106

// Advanced tab controls
#define IDC_CHECK_NOISE        3201
#define IDC_EDIT_NOISE         3202
#define IDC_SPIN_NOISE         3203
#define IDC_BUTTON_APPLY       3204
#define IDC_SLIDER_ANIM        3205
#define IDC_BUTTON_RANDOM      3206
#define IDC_SLIDER_SHAPES      3207
#define IDC_BUTTON_COLOR_UI_BG 3208 // Button to change UI background color
#define IDC_BUTTON_COLOR_UI_TAB 3209 // Button to change Tab control color
#define IDC_BUTTON_COLOR_UI_BTN 3210 // Button to change Button color
#define IDC_CHECK_LOOP         3211 // Loop Animation Checkbox

#ifndef IDC_STATIC
#define IDC_STATIC -1
#endif

#endif // RESOURCE_H
