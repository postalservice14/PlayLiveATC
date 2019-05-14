//
//  SettingsUI.cpp
//  SwitchLiveATC

/*
 * Copyright (c) 2019, Birger Hoppe
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SwitchLiveATC.h"

//
//MARK: LTSettingsUI
//

LTSettingsUI::LTSettingsUI () :
widgetIds(nullptr)
{}


LTSettingsUI::~LTSettingsUI()
{
    // just in case...
    Disable();
}

//
//MARK: Window Structure
// Basics | Advanced
//

// indexes into the below definition array, must be kept in synch with the same
enum UI_WIDGET_IDX_T {
    UI_MAIN_WND     = 0,
    // Buttons to select 'tabs'
    UI_BTN_BASICS,
    UI_BTN_ADVANCED,
    
    // [?] Help
    UI_BTN_HELP,
    
    // "Basics" tab
    UI_BASICS_SUB_WND,
    UI_BASICS_BTN_COM1,
    UI_BASICS_BTN_COM2,
    UI_BASICS_CAP_PATH_TO_VLC,
    UI_BASICS_TXT_PATH_TO_VLC,
    UI_BASICS_CAP_VALIDATE_PATH,

    UI_BASICS_CAP_VERSION_TXT,
    UI_BASICS_CAP_VERSION,
    UI_BASICS_CAP_BETA_LIMIT,

    // "Advanced" tab
    UI_ADVCD_SUB_WND,
    UI_ADVCD_CAP_LOGLEVEL,
    UI_ADVCD_BTN_LOG_FATAL,
    UI_ADVCD_BTN_LOG_ERROR,
    UI_ADVCD_BTN_LOG_WARNING,
    UI_ADVCD_BTN_LOG_INFO,
    UI_ADVCD_BTN_LOG_DEBUG,
    UI_ADVCD_CAP_MSGAREA_LEVEL,
    UI_ADVCD_BTN_MSGAREA_FATAL,
    UI_ADVCD_BTN_MSGAREA_ERROR,
    UI_ADVCD_BTN_MSGAREA_WARNING,
    UI_ADVCD_BTN_MSGAREA_INFO,

    UI_ADVCD_CAP_VLC_PARAMS,
    UI_ADVCD_TXT_VLC_PARAMS,

    // always last: number of UI elements
    UI_NUMBER_OF_ELEMENTS
};

// for ease of definition coordinates start at (0|0)
// window will be centered shortly before presenting it
TFWidgetCreate_t SETTINGS_UI[] =
{
    {   0,   0, 400, 330, 0, "SwitchLiveATC Settings", 1, NO_PARENT, xpWidgetClass_MainWindow, {xpProperty_MainWindowHasCloseBoxes, 1, xpProperty_MainWindowType,xpMainWindowStyle_Translucent,0,0} },
    // Buttons to select 'tabs'
    {  10,  30,  65,  10, 1, "Basics",              0, UI_MAIN_WND, xpWidgetClass_Button, {xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0, 0,0} },
    {  75,  30,  65,  10, 1, "Advanced",            0, UI_MAIN_WND, xpWidgetClass_Button, {xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0, 0,0} },
    // Push button for help
    { 360,  30,  30,  10, 1, "?",                   0, UI_MAIN_WND, xpWidgetClass_Button, {xpProperty_ButtonBehavior, xpButtonBehaviorPushButton,  0,0, 0,0} },
    // "Basics" tab
    {  10,  50, -10, -10, 0, "Basics",              0, UI_MAIN_WND, xpWidgetClass_SubWindow, {0,0, 0,0, 0,0} },
    {  10,  10,  10,  10, 1, "Watch COM1 frequency",0, UI_BASICS_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox, 0,0} },
    {  10,  25,  10,  10, 1, "Watch COM2 frequency",0, UI_BASICS_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorCheckBox, 0,0} },
    {   5,  50,  -5,  10, 1, "Path to VLC executable:",0, UI_BASICS_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    {  10,  65,  -5,  15, 1, "",                    0, UI_BASICS_SUB_WND, xpWidgetClass_TextField, {0,0, 0,0, 0,0} },
    {   5,  80,  -5,  10, 1, "",                    0, UI_BASICS_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },

    {   5, -15,  -5,  10, 1, "Version",              0, UI_BASICS_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    {  50, -15, 200,  10, 1, "",                     0, UI_BASICS_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    { 200, -15,  -5,  10, 1, "",                     0, UI_BASICS_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    // "Advanced" tab
    {  10,  50, -10, -10, 0, "Advanced",            0, UI_MAIN_WND, xpWidgetClass_SubWindow, {0,0,0,0,0,0} },
    {   5,  10,  -5,  10, 1, "Log Level:",          0, UI_ADVCD_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    {  80,  10,  10,  10, 1, "Fatal",               0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 140,  10,  10,  10, 1, "Error",               0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 200,  10,  10,  10, 1, "Warning",             0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 270,  10,  10,  10, 1, "Info",                0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 320,  10,  10,  10, 1, "Debug",               0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    {   5,  30,  -5,  10, 1, "Msg Area:",           0, UI_ADVCD_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    {  80,  30,  10,  10, 1, "Fatal",               0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 140,  30,  10,  10, 1, "Error",               0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 200,  30,  10,  10, 1, "Warning",             0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    { 270,  30,  10,  10, 1, "Info",                0, UI_ADVCD_SUB_WND, xpWidgetClass_Button, {xpProperty_ButtonType, xpRadioButton, xpProperty_ButtonBehavior, xpButtonBehaviorRadioButton, 0,0} },
    
    {   5,  50,  -5,  10, 1, "VLC Parameters (leave empty if unsure):",0, UI_ADVCD_SUB_WND, xpWidgetClass_Caption, {0,0, 0,0, 0,0} },
    {  10,  65,  -5,  15, 1, "",                    0, UI_ADVCD_SUB_WND, xpWidgetClass_TextField, {0,0, 0,0, 0,0} },
};

constexpr int NUM_WIDGETS = sizeof(SETTINGS_UI)/sizeof(SETTINGS_UI[0]);

static_assert(UI_NUMBER_OF_ELEMENTS == NUM_WIDGETS,
              "UI_WIDGET_IDX_T and SETTINGS_UI[] differ in number of elements!");

// creates the main window and all its widgets based on the above
// definition array
void LTSettingsUI::Enable()
{
    if (!isEnabled()) {
        // array, which receives ids of all created widgets
        widgetIds = new XPWidgetID[NUM_WIDGETS];
        LOG_ASSERT(widgetIds);
        memset(widgetIds, 0, sizeof(XPWidgetID)*NUM_WIDGETS );

        // create all widgets, i.e. the entire window structure, but keep invisible
        if (!TFUCreateWidgetsEx(SETTINGS_UI, NUM_WIDGETS, NULL, widgetIds))
        {
            SHOW_MSG(logERR,ERR_WIDGET_CREATE);
            return;
        }
        setId(widgetIds[0]);        // register in base class for message handling
        
        // some widgets with objects
        subBasics.setId(widgetIds[UI_BASICS_SUB_WND]);
        subAdvcd.setId(widgetIds[UI_ADVCD_SUB_WND]);

        // organise the tab button group
        tabGrp.Add({
            widgetIds[UI_BTN_BASICS],
            widgetIds[UI_BTN_ADVANCED],
        });
        tabGrp.SetChecked(widgetIds[UI_BTN_BASICS]);
        HookButtonGroup(tabGrp);
        
        // *** Basic ***
        btnBasicsCom[0].setId(widgetIds[UI_BASICS_BTN_COM1]);
        btnBasicsCom[1].setId(widgetIds[UI_BASICS_BTN_COM2]);
        txtPathToVLC.setId(widgetIds[UI_BASICS_TXT_PATH_TO_VLC]);
        txtPathToVLC.SetDescriptor(dataRefs.GetVLCPath());
        capValidatePath.setId(widgetIds[UI_BASICS_CAP_VALIDATE_PATH]);
        
        // version number
        XPSetWidgetDescriptor(widgetIds[UI_BASICS_CAP_VERSION],
                              SLA_VERSION_FULL);
        if constexpr (VERSION_BETA)
        {
            char betaLimit[100];
            snprintf(betaLimit,sizeof(betaLimit), BETA_LIMITED_VERSION,SLA_BETA_VER_LIMIT_TXT);
            XPSetWidgetDescriptor(widgetIds[UI_BASICS_CAP_BETA_LIMIT],
                                  betaLimit);
        }
        
        // *** Advanced ***
        logLevelGrp.Add({
            widgetIds[UI_ADVCD_BTN_LOG_DEBUG],      // index 0 equals logDEBUG, which is also 0
            widgetIds[UI_ADVCD_BTN_LOG_INFO],       // ...
            widgetIds[UI_ADVCD_BTN_LOG_WARNING],
            widgetIds[UI_ADVCD_BTN_LOG_ERROR],
            widgetIds[UI_ADVCD_BTN_LOG_FATAL],      // index 4 equals logFATAL, which is also 4
        });
        HookButtonGroup(logLevelGrp);
        
        msgAreaLevelGrp.Add({
            widgetIds[UI_ADVCD_BTN_MSGAREA_INFO],       // index 0 is logINFO, which is 1
            widgetIds[UI_ADVCD_BTN_MSGAREA_WARNING],
            widgetIds[UI_ADVCD_BTN_MSGAREA_ERROR],
            widgetIds[UI_ADVCD_BTN_MSGAREA_FATAL],      // index 4 equals logFATAL, which is also 4
        });
        HookButtonGroup(msgAreaLevelGrp);
        
        txtVLCParams.setId(widgetIds[UI_ADVCD_TXT_VLC_PARAMS]);
        txtVLCParams.SetDescriptor(dataRefs.GetVLCParams());
        
        // set current values
        UpdateValues();

        // center the UI
        Center();
    }
}

void LTSettingsUI::Disable()
{
    if (isEnabled()) {
        // remove widgets and free memory
        XPDestroyWidget(*widgetIds, 1);
        delete widgetIds;
        widgetIds = nullptr;
    }
}

// make sure I'm created before first use
void LTSettingsUI::Show (bool bShow)
{
    if (bShow)              // create before use
        Enable();
    TFWidget::Show(bShow);  // show/hide
    
#ifdef XPLM301
    // make sure we are in the right window mode
    if (GetWndMode() != GetDefaultWndOpenMode()) {      // only possible in XP11
        if (GetDefaultWndOpenMode() == TF_MODE_VR)
            SetWindowPositioningMode(xplm_WindowVR, -1);
        else
            SetWindowPositioningMode(xplm_WindowPositionFree, -1);	
    }
#endif
}

// update state of some buttons from dataRef
void LTSettingsUI::UpdateValues ()
{
    // *** Basics ***
    for (int idx = 0; idx < COM_CNT; idx++)
        btnBasicsCom[idx].SetChecked(dataRefs.ShallActOnCom(idx));
    
    // *** Advanced ***
    logLevelGrp.SetCheckedIndex(dataRefs.GetLogLevel());
    msgAreaLevelGrp.SetCheckedIndex(dataRefs.GetMsgAreaLevel() - 1);
}

// capture entry into text fields like "path to VLC"
bool LTSettingsUI::MsgTextFieldChanged (XPWidgetID textWidget, std::string text)
{
    // *** Basic ***
    if (txtPathToVLC == textWidget) {
        ValidateUpdateVLCPath(text);
        // processed
        return true;
    }
    
    // *** Advanced ***
    if (txtVLCParams == textWidget) {
        if (text.empty()) {                 // empty input == 'return to default'
            dataRefs.SetVLCParams(CFG_PARAMS_DEFAULT);
            txtVLCParams.SetDescriptor(CFG_PARAMS_DEFAULT);
        }
        else {
            dataRefs.SetVLCParams(text);
        }
        return true;
    }
    
    // not ours
    return TFMainWindowWidget::MsgTextFieldChanged(textWidget, text);
}


// writes current values out into config file
bool LTSettingsUI::MsgHidden (XPWidgetID hiddenWidget)
{
    if (hiddenWidget == *this) {        // only if it was me who got hidden
        // then only save the config file
        dataRefs.SaveConfigFile();
    }
    // pass on in class hierarchy
    return TFMainWindowWidget::MsgHidden(hiddenWidget);
}


// handles show/hide of 'tabs', values of logging level
bool LTSettingsUI::MsgButtonStateChanged (XPWidgetID buttonWidget, bool bNowChecked)
{
    // first pass up the class hierarchy to make sure the button groups are handled correctly
    bool bRet = TFMainWindowWidget::MsgButtonStateChanged(buttonWidget, bNowChecked);
    
    // *** Tab Groups ***
    // if the button is one of our tab buttons show/hide the appropriate subwindow
    if (widgetIds[UI_BTN_BASICS] == buttonWidget) {
        subBasics.Show(bNowChecked);
        return true;
    }
    else if (widgetIds[UI_BTN_ADVANCED] == buttonWidget) {
        subAdvcd.Show(bNowChecked);
        return true;
    }

    // *** Basics ***
    
    // Save value for "watch COM#"
    for (int idx = 0; idx < COM_CNT; idx++) {
        if (btnBasicsCom[idx] == buttonWidget) {
            dataRefs.SetActOnCom(idx, bNowChecked);
            gChn[idx].ClearChannel();       // reset channel so that it either fells silent or can be reactivated
            return true;
        }
    }

    // *** Advanced ***
    // if any of the log-level radio buttons changes we set log-level accordingly
    if (bNowChecked && logLevelGrp.isInGroup(buttonWidget))
    {
        dataRefs.SetLogLevel(logLevelTy(logLevelGrp.GetCheckedIndex()));
        return true;
    }
    if (bNowChecked && msgAreaLevelGrp.isInGroup(buttonWidget))
    {
        dataRefs.SetMsgAreaLevel(logLevelTy(msgAreaLevelGrp.GetCheckedIndex() + 1));
        return true;
    }
    
    return bRet;
}

// push buttons
bool LTSettingsUI::MsgPushButtonPressed (XPWidgetID buttonWidget)
{
    // *** Help ***
    if (widgetIds[UI_BTN_HELP] == buttonWidget)
    {
        // open help
        OpenURL(HELP_URL_SETTINGS);
        return true;
    }
    
    // we don't know that button...
    return TFMainWindowWidget::MsgPushButtonPressed(buttonWidget);
}

// update state of some buttons from dataRef every second
bool LTSettingsUI::TfwMsgMain1sTime ()
{
    TFMainWindowWidget::TfwMsgMain1sTime();
    UpdateValues();
    return true;
}

// validate and update VLCPath
void LTSettingsUI::ValidateUpdateVLCPath (const std::string newPath)
{
    // Check if the given path is an existing file
    // (This is not _exactly_ required...VLC could also be just in the path,
    //  but it is useful information nonetheless)
    if (newPath.empty())
        capValidatePath.SetDescriptor(MSG_VLC_NO_PATH);
    else if (existsFile(newPath))
        capValidatePath.SetDescriptor(MSG_VLC_PATH_VERIFIED);
    else
        capValidatePath.SetDescriptor(MSG_VLC_PATH_NO_FILE);
    
    // no matter what, we safe the result
    dataRefs.SetVLCPath(newPath);
}
