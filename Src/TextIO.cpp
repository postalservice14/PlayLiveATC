//
//  TextIO.cpp
//  PlayLiveATC

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

#include "PlayLiveATC.h"

//
// MARK: LiveTraffic Exception classes
//

// standard constructor
LTError::LTError (const char* _szFile, int _ln, const char* _szFunc,
                  logLevelTy _lvl,
                  const char* _szMsg, ...) :
std::logic_error(GetLogString(_szFile, _ln, _szFunc, _lvl, _szMsg, NULL)),
fileName(_szFile), ln(_ln), funcName(_szFunc),
lvl(_lvl)
{
    va_list args;
    va_start (args, _szMsg);
    msg = GetLogString(_szFile, _ln, _szFunc, _lvl, _szMsg, args);
    va_end (args);
    
    // write to log (flushed immediately -> expensive!)
    if (_lvl >= dataRefs.GetLogLevel())
        XPLMDebugString ( msg.c_str() );
}

// protected constructor, only called by LTErrorFD
LTError::LTError (const char* _szFile, int _ln, const char* _szFunc,
                  logLevelTy _lvl) :
std::logic_error(GetLogString(_szFile, _ln, _szFunc, _lvl, "", NULL)),
fileName(_szFile), ln(_ln), funcName(_szFunc),
lvl(_lvl)
{}

const char* LTError::what() const noexcept
{
    return msg.c_str();
}

//
// MARK: custom X-Plane message Window - Globals
//

// An opaque handle to the window we will create
XPLMWindowID    g_window = 0;
// when to remove the window

std::chrono::time_point<std::chrono::steady_clock> timeToRemove;

// time until window is to be shown, will be destroyed after that
struct dispTextTy {
    // until when to display this line?
    std::chrono::time_point<std::chrono::steady_clock> timeDisp;
    logLevelTy lvlDisp;                     // level of msg (defines text color)
    std::string text;                       // text of line
};
std::list<dispTextTy> listTexts;     // lines of text to be displayed

float COL_LVL[logMSG+1][3] = {          // text colors [RGB] depending on log level
    {0.00f, 0.00f, 0.00f},              // 0
    {1.00f, 1.00f, 1.00f},              // INFO (white)
    {1.00f, 1.00f, 0.00f},              // WARN (yellow)
    {1.00f, 0.00f, 0.00f},              // ERROR (red)
    {1.00f, 0.54f, 0.83f},              // FATAL (purple, FF8AD4)
    {1.00f, 1.00f, 1.00f}               // MSG (white)
};

//MARK: custom X-Plane message Window - Private Callbacks
// Callbacks we will register when we create our window
void    draw_msg(XPLMWindowID in_window_id, void * /*in_refcon*/)
{
    // Mandatory: We *must* set the OpenGL state before drawing
    // (we can't make any assumptions about it)
    XPLMSetGraphicsState(
                         0 /* no fog */,
                         0 /* 0 texture units */,
                         0 /* no lighting */,
                         0 /* no alpha testing */,
                         1 /* do alpha blend */,
                         1 /* do depth testing */,
                         0 /* no depth writing */
                         );
    
    int l, t, r, b;
    XPLMGetWindowGeometry(in_window_id, &l, &t, &r, &b);
    
    XPLMDrawTranslucentDarkBox(l, t, r, b);
    
    b = WIN_WIDTH;                          // word wrap width = window width
    t -= WIN_ROW_HEIGHT;                    // move down to text's baseline
    
    // display desync countdown?
    bool bNeedWndForCountdown = false;
    if (dataRefs.GetMsgAreaLevel() <= logINFO)
        for (COMChannel& chn: gChn)
            if (chn.IsDesyncing()) {
                char buf[50];
                snprintf(buf, sizeof(buf), MSG_COM_COUNTDOWN,
                         chn.GetIdx()+1,
                         chn.GetSecTillDesyncDone(),
                         chn.GetStreamCtrlData().streamName.c_str());
                // draw text, take color based on msg level
                XPLMDrawString(COL_LVL[logINFO], l, t,
                               buf,
                               &b, xplmFont_Proportional);
                t -= WIN_ROW_HEIGHT;
                bNeedWndForCountdown = true;
            }

    // for each line of text to be displayed
    std::chrono::time_point<std::chrono::steady_clock> currTime = std::chrono::steady_clock::now();
    for (auto iter = listTexts.cbegin();
         iter != listTexts.cend();
         )             // can't deduce number of rwos (after word wrap)...just assume 2 rows are enough
    {
        // still a valid entry?
        if (iter->timeDisp >= currTime)
        {
            // draw text, take color based on msg level
            XPLMDrawString(COL_LVL[iter->lvlDisp], l, t,
                           const_cast<char*>(iter->text.c_str()),
                           &b, xplmFont_Proportional);
            // cancel any idea of removing the msg window
            timeToRemove = std::chrono::time_point<std::chrono::steady_clock>();
            // next element
            iter++;
            t -= 2*WIN_ROW_HEIGHT;
        }
        else {
            // now outdated. Move on to next line, but remove this one
            iter = listTexts.erase(iter);
        }
    }
    
    // No texts left? Remove window in 1s
    if ((g_window == in_window_id) &&
        listTexts.empty() && !bNeedWndForCountdown)
    {
        if (timeToRemove.time_since_epoch() == std::chrono::steady_clock::duration::zero())
            // set time when to remove
            timeToRemove = currTime + WIN_TIME_REMAIN;
        else if (currTime >= timeToRemove) {
            // time's up: remove
            DestroyWindow();
            timeToRemove = std::chrono::time_point<std::chrono::steady_clock>();;
        }
    }
}

int dummy_mouse_handler(XPLMWindowID /*in_window_id*/, int /*x*/, int /*y*/, int /*is_down*/, void * /*in_refcon*/)
{ return 0; }

XPLMCursorStatus dummy_cursor_status_handler(XPLMWindowID /*in_window_id*/, int /*x*/, int /*y*/, void * /*in_refcon*/)
{ return xplm_CursorDefault; }

int dummy_wheel_handler(XPLMWindowID /*in_window_id*/, int /*x*/, int /*y*/, int /*wheel*/, int /*clicks*/, void * /*in_refcon*/)
{ return 0; }

void dummy_key_handler(XPLMWindowID /*in_window_id*/, char /*key*/, XPLMKeyFlags /*flags*/, char /*virtual_key*/, void * /*in_refcon*/, int /*losing_focus*/)
{ }


//MARK: custom X-Plane message Window - Create / Destroy
XPLMWindowID CreateMsgWindow(float fTimeToDisplay, logLevelTy lvl, const char* szMsg, ...)
{
    // consider configured level for msg area
    // for this consideration we treat logMSG the same way as logINFO,
    // i.e. logMSG is more likely to be suppressed
    if ( (lvl == logMSG ? logINFO : lvl) < dataRefs.GetMsgAreaLevel())
        return g_window;
    
    va_list args;

    // save the text in a static buffer queried by the drawing callback
    char aszMsgTxt[500];
    va_start (args, szMsg);
    vsnprintf(aszMsgTxt,
              sizeof(aszMsgTxt),
              szMsg,
              args);
    va_end (args);
    
    // define the text to display:
    std::chrono::time_point<std::chrono::steady_clock> currTime = std::chrono::steady_clock::now();
    dispTextTy dispTxt = {
        // set the timer if a limit is given
        fTimeToDisplay >= 0.0f ?
        currTime + std::chrono::milliseconds(lround(fTimeToDisplay * 1000.0)) :
        currTime + std::chrono::milliseconds(lround(WIN_TIME_DISPLAY * 1000.0)),
        // log level to define the color
        lvl,
        // finally the text
        aszMsgTxt
    };
    
    // add to list of display texts
    listTexts.emplace_back(std::move(dispTxt));
    
    // Otherwise: Create the message window
    XPLMCreateWindow_t params;
    // XPLM301 only:    params.structSize = IS_XPLM301 ? sizeof(params) : XPLMCreateWindow_s_210;
    params.structSize = sizeof(params);
    params.visible = 1;
    params.drawWindowFunc = draw_msg;
    // Note on "dummy" handlers:
    // Even if we don't want to handle these events, we have to register a "do-nothing" callback for them
    params.handleMouseClickFunc = dummy_mouse_handler;
    // XPLM301 only:   params.handleRightClickFunc = dummy_mouse_handler;
    params.handleMouseWheelFunc = dummy_wheel_handler;
    params.handleKeyFunc = dummy_key_handler;
    params.handleCursorFunc = dummy_cursor_status_handler;
    params.refcon = NULL;
    // XPLM301 only:    params.layer = xplm_WindowLayerFloatingWindows;
    // No decoration...this is just message output and shall stay where it is
    // params.decorateAsFloatingWindow = xplm_WindowDecorationNone;
    
    // Set the window's initial bounds
    // Note that we're not guaranteed that the main monitor's lower left is at (0, 0)...
    // We'll need to query for the global desktop bounds!
    // LT_GetScreenSize(params.left, params.top, params.right, params.bottom,
       //              LT_SCR_RIGHT_TOP_MOST);
    
    // In XPLM210 this is simplified:
    params.left = params.bottom = 0;
    XPLMGetScreenSize(&params.right,&params.top);
    
    // define a window in the top right corner,
    // WIN_FROM_TOP point down from the top, WIN_WIDTH points wide,
    // enough height for all lines of text
    params.top -= WIN_FROM_TOP;
    params.right -= WIN_FROM_RIGHT;
    params.left = params.right - WIN_WIDTH;
    params.bottom = params.top - (WIN_ROW_HEIGHT * (2*int(listTexts.size())+1));
    
    // if the window still exists just resize it
    if (g_window)
        XPLMSetWindowGeometry(g_window, params.left, params.top, params.right, params.bottom);
    else {
        // otherwise create a new one
        g_window = XPLMCreateWindowEx(&params);
        LOG_ASSERT(g_window);
    }
    
    return g_window;
}


void DestroyWindow()
{
    if ( g_window )
    {
        XPLMDestroyWindow(g_window);
        g_window = NULL;
        listTexts.clear();
   }
}

//
//MARK: Log
//

const char* LOG_LEVEL[] = {
    "DEBUG", "INFO ", "WARN ", "ERROR", "FATAL", "MSG  "
};

// returns ptr to static buffer filled with log string
const char* GetLogString (const char* szPath, int ln, const char* szFunc,
                          logLevelTy lvl, const char* szMsg, va_list args )
{
    static char aszMsg[2048];
    
    // current Zulu time
    std::time_t t = std::time(nullptr);
    char tZuluS[100];
    std::strftime(tZuluS, sizeof(tZuluS), "%d-%b %T", gmtime(&t));

    // prepare timestamp
    if ( lvl < logMSG )                             // normal messages without, all other with location info
    {
        const char* szFile = strrchr(szPath,'/');   // extract file from path
        if ( !szFile ) szFile = szPath; else szFile++;
        snprintf(aszMsg, sizeof(aszMsg), "%s %sZ %s %s:%d/%s: ",
                SWITCH_LIVE_ATC, tZuluS, LOG_LEVEL[lvl],
                szFile, ln, szFunc);
    }
    else
        snprintf(aszMsg, sizeof(aszMsg), "%s: ", SWITCH_LIVE_ATC);
    
    // append given message
    if (args) {
        vsnprintf(&aszMsg[strlen(aszMsg)],
                  sizeof(aszMsg)-strlen(aszMsg)-1,      // we save one char for the CR
                  szMsg,
                  args);
    }

    // ensure there's a trailing CR
    size_t l = strlen(aszMsg);
    if ( aszMsg[l-1] != '\n' )
    {
        aszMsg[l]   = '\n';
        aszMsg[l+1] = 0;
    }

    // return the static buffer
    return aszMsg;
}

void LogMsg ( const char* szPath, int ln, const char* szFunc, logLevelTy lvl, const char* szMsg, ... )
{
    va_list args;

    va_start (args, szMsg);
    // write to log (flushed immediately -> expensive!)
    XPLMDebugString ( GetLogString(szPath, ln, szFunc, lvl, szMsg, args) );
    va_end (args);
}
