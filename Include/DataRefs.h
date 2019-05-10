//
//  DataRefs.h
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

#ifndef DataRefs_h
#define DataRefs_h

// in SwitchLiveATC.xpp:
void MenuUpdateCheckmarks();

//
// MARK: DataRefs
//

// XP standard and LiveTraffic Datarefs being accessed
enum dataRefsXP_LT {
    // XP standard
    DR_XP_RADIO_COM1_FREQ = 0,          // sim/cockpit/radios/com1_freq_hz
    DR_XP_RADIO_COM2_FREQ,              // sim/cockpit/radios/com2_freq_hz
    // LiveTraffic
    DR_LT_FD_BUF_PERIOD,                // LiveTraffic's buffering period
    // always last, number of elements
    CNT_DATAREFS_XP
};
constexpr dataRefsXP_LT DR_FIRST_LT_DR = DR_LT_FD_BUF_PERIOD;

// SLA commands to be offered
enum cmdRefsSLA {
    CR_TOGGLE_ACT_COM1 = 0,
    CR_TOGGLE_ACT_COM2,
    CNT_CMDREFS_SLA                     // always last, number of elements
};

// number of Frequencies to listen to
constexpr int COM_CNT = 2;


class DataRefs
{
//MARK: DataRefs
protected:
    XPLMDataRef adrXP[CNT_DATAREFS_XP];                 // array of XP data refs to read from
public:
    XPLMCommandRef cmdSLA[CNT_CMDREFS_SLA];

//MARK: Provided Data, i.e. global variables
protected:
    XPLMPluginID pluginID       = 0;
    logLevelTy iLogLevel        = logWARN;
    logLevelTy iMsgAreaLevel    = logWARN;
    std::string XPSystemPath;
    std::string PluginPath;                             // path to plugin directory
    std::string DirSeparator;
    
    bool    bActOnCom[COM_CNT] = {true,true};   // which frequency to act upon?
    std::string VLCPath;                        // how to start VLC?
    std::string VLCParams;                      // parameter definition
    
//MARK: Constructor
public:
    DataRefs ( logLevelTy initLogLevel );       // Constructor doesn't do much
    bool Init();                                // Early init DataRefs, return "OK?"
    bool LateInit();                            // Late init like other plugin's dataRefs
    void Stop();                                // unregister what's needed

    inline XPLMPluginID GetMyPluginId() const { return pluginID; }

protected:
    bool FindDataRefs (bool bEarly);
    bool RegisterCommands();

//MARK: DataRef access
public:
    // Log Level
    void SetLogLevel ( logLevelTy ll )  { iLogLevel = ll; }
    void SetMsgAreaLevel ( logLevelTy ll )  { iMsgAreaLevel = ll; }
    inline logLevelTy GetLogLevel()     { return iLogLevel; }
    inline logLevelTy GetMsgAreaLevel()         { return iMsgAreaLevel; }

    inline bool ShallActOnCom(int idx) const { return 0<=idx&&idx<COM_CNT ? bActOnCom[idx] : false; }
    void SetActOnCom(int idx, bool bEnable) { if (0<=idx&&idx<COM_CNT) {bActOnCom[idx] = bEnable; MenuUpdateCheckmarks();} }
    void ToggleActOnCom(int idx) { SetActOnCom(idx,!ShallActOnCom(idx)); }
    
    inline std::string GetVLCPath() const { return VLCPath; }
    void SetVLCPath (const std::string newPath) { VLCPath = newPath; }
    inline std::string GetVLCParams() const { return VLCParams; }
    void SetVLCParams (const std::string newParams) { VLCParams = newParams; }

    // specific access
    inline int   GetComFreq(int idx) const  { return 0<=idx&&idx<COM_CNT ? XPLMGetDatai(adrXP[int(DR_XP_RADIO_COM1_FREQ)+idx]) : 0; }

    // Get XP System Path
    inline std::string GetXPSystemPath() const  { return XPSystemPath; }
    inline std::string GetPluginPath()   const  { return PluginPath; }
    inline std::string GetDirSeparator() const  { return DirSeparator; }
    
    // Load/save config file
    bool LoadConfigFile();
    bool SaveConfigFile();
};

#endif /* DataRefs_h */
