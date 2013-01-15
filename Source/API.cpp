/********************************************************************************
 Copyright (C) 2012 Hugh Bailey <obs.jim@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/


#include "Main.h"


void OBS::RegisterSceneClass(CTSTR lpClassName, CTSTR lpDisplayName, OBSCREATEPROC createProc, OBSCONFIGPROC configProc)
{
    if(!lpClassName || !*lpClassName)
    {
        AppWarning(TEXT("OBS::RegisterSceneClass: No class name specified"));
        return;
    }

    if(!createProc)
    {
        AppWarning(TEXT("OBS::RegisterSceneClass: No create procedure specified"));
        return;
    }

    if(GetSceneClass(lpClassName))
    {
        AppWarning(TEXT("OBS::RegisterSceneClass: Tried to register '%s', but it already exists"), lpClassName);
        return;
    }

    ClassInfo *classInfo  = sceneClasses.CreateNew();
    classInfo->strClass   = lpClassName;
    classInfo->strName    = lpDisplayName;
    classInfo->createProc = createProc;
    classInfo->configProc = configProc;
}

void OBS::RegisterImageSourceClass(CTSTR lpClassName, CTSTR lpDisplayName, OBSCREATEPROC createProc, OBSCONFIGPROC configProc)
{
    if(!lpClassName || !*lpClassName)
    {
        AppWarning(TEXT("OBS::RegisterImageSourceClass: No class name specified"));
        return;
    }

    if(!createProc)
    {
        AppWarning(TEXT("OBS::RegisterImageSourceClass: No create procedure specified"));
        return;
    }

    if(GetImageSourceClass(lpClassName))
    {
        AppWarning(TEXT("OBS::RegisterImageSourceClass: Tried to register '%s', but it already exists"), lpClassName);
        return;
    }

    ClassInfo *classInfo  = imageSourceClasses.CreateNew();
    classInfo->strClass   = lpClassName;
    classInfo->strName    = lpDisplayName;
    classInfo->createProc = createProc;
    classInfo->configProc = configProc;
}

Scene* OBS::CreateScene(CTSTR lpClassName, XElement *data)
{
    for(UINT i=0; i<sceneClasses.Num(); i++)
    {
        if(sceneClasses[i].strClass.CompareI(lpClassName))
            return (Scene*)sceneClasses[i].createProc(data);
    }

    AppWarning(TEXT("OBS::CreateScene: Could not find scene class '%s'"), lpClassName);
    return NULL;
}

ImageSource* OBS::CreateImageSource(CTSTR lpClassName, XElement *data)
{
    for(UINT i=0; i<imageSourceClasses.Num(); i++)
    {
        if(imageSourceClasses[i].strClass.CompareI(lpClassName))
            return (ImageSource*)imageSourceClasses[i].createProc(data);
    }

    AppWarning(TEXT("OBS::CreateImageSource: Could not find image source class '%s'"), lpClassName);
    return NULL;
}

void OBS::ConfigureScene(XElement *element)
{
    if(!element)
    {
        AppWarning(TEXT("OBS::ConfigureScene: NULL element specified"));
        return;
    }

    CTSTR lpClassName = element->GetString(TEXT("class"));
    if(!lpClassName)
    {
        AppWarning(TEXT("OBS::ConfigureScene: No class specified for scene '%s'"), element->GetName());
        return;
    }

    for(UINT i=0; i<sceneClasses.Num(); i++)
    {
        if(sceneClasses[i].strClass.CompareI(lpClassName))
        {
            if(sceneClasses[i].configProc)
                sceneClasses[i].configProc(element, false);
            return;
        }
    }

    AppWarning(TEXT("OBS::ConfigureScene: Could not find scene class '%s'"), lpClassName);
}

void OBS::ConfigureImageSource(XElement *element)
{
    if(!element)
    {
        AppWarning(TEXT("OBS::ConfigureImageSource: NULL element specified"));
        return;
    }

    CTSTR lpClassName = element->GetString(TEXT("class"));
    if(!lpClassName)
    {
        AppWarning(TEXT("OBS::ConfigureImageSource: No class specified for image source '%s'"), element->GetName());
        return;
    }

    for(UINT i=0; i<imageSourceClasses.Num(); i++)
    {
        if(imageSourceClasses[i].strClass.CompareI(lpClassName))
        {
            if(imageSourceClasses[i].configProc)
                imageSourceClasses[i].configProc(element, false);
            return;
        }
    }

    AppWarning(TEXT("OBS::ConfigureImageSource: Could not find scene class '%s'"), lpClassName);
}

void OBS::AddSourceItem(LPWSTR name, bool checked, UINT index)
{
    LVITEM lvI;
    // Initialize LVITEM members that are common to all items.
    lvI.mask      = LVIF_TEXT | LVIF_IMAGE | LVIF_STATE;
    lvI.stateMask = 0;
    lvI.iSubItem  = 0;
    lvI.state     = 0;
    lvI.pszText = name;
    lvI.iItem = index;


    HWND hwndSources = GetDlgItem(hwndMain, ID_SOURCES);
    
    ListView_InsertItem(hwndSources, &lvI);
    ListView_SetCheckState(hwndSources, index, checked);       
}

bool OBS::SetScene(CTSTR lpScene)
{
    if(bDisableSceneSwitching)
        return false;

    HWND hwndScenes = GetDlgItem(hwndMain, ID_SCENES);
    UINT curSel = (UINT)SendMessage(hwndScenes, LB_GETCURSEL, 0, 0);

    //-------------------------

    if(curSel != LB_ERR)
    {
        UINT textLen = (UINT)SendMessage(hwndScenes, LB_GETTEXTLEN, curSel, 0);

        String strLBName;
        strLBName.SetLength(textLen);

        SendMessage(hwndScenes, LB_GETTEXT, curSel, (LPARAM)strLBName.Array());
        if(!strLBName.CompareI(lpScene))
        {
            UINT id = (UINT)SendMessage(hwndScenes, LB_FINDSTRINGEXACT, -1, (LPARAM)lpScene);
            if(id == LB_ERR)
                return false;

            SendMessage(hwndScenes, LB_SETCURSEL, id, 0);
        }
    }
    else
    {
        UINT id = (UINT)SendMessage(hwndScenes, LB_FINDSTRINGEXACT, -1, (LPARAM)lpScene);
        if(id == LB_ERR)
            return false;

        SendMessage(hwndScenes, LB_SETCURSEL, id, 0);
    }

    //-------------------------

    XElement *scenes = scenesConfig.GetElement(TEXT("scenes"));
    XElement *newSceneElement = scenes->GetElement(lpScene);
    if(!newSceneElement)
        return false;

    if(sceneElement == newSceneElement)
        return true;

    sceneElement = newSceneElement;

    CTSTR lpClass = sceneElement->GetString(TEXT("class"));
    if(!lpClass)
    {
        AppWarning(TEXT("OBS::SetScene: no class found for scene '%s'"), newSceneElement->GetName());
        return false;
    }

    if(bRunning)
    {
        Log(TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
        Log(TEXT("  New Scene"));
    }

    XElement *sceneData = newSceneElement->GetElement(TEXT("data"));

    //-------------------------

    Scene *newScene = NULL;
    if(bRunning)
        newScene = CreateScene(lpClass, sceneData);

    //-------------------------

    HWND hwndSources = GetDlgItem(hwndMain, ID_SOURCES);
    bChangingSources = true;
    ListView_DeleteAllItems(hwndSources);
    
    XElement *sources = sceneElement->GetElement(TEXT("sources"));
    if(sources)
    {
        UINT numSources = sources->NumElements();
        ListView_SetItemCount(hwndSources, numSources);

        for(UINT i=0; i<numSources; i++)
        {
            XElement *sourceElement = sources->GetElementByID(i);
            bool render = sourceElement->GetInt(TEXT("render"), 1) > 0;
            
            
            AddSourceItem((LPWSTR)sourceElement->GetName(), render, i);
            
           
            if(bRunning && newScene)
                newScene->AddImageSource(sourceElement);
        }
    }
    bChangingSources = false;

    if(scene && newScene->HasMissingSources())
        MessageBox(hwndMain, Str("Scene.MissingSources"), NULL, 0);

    if(bRunning)
    {
        //todo: cache scenes maybe?  undecided.  not really as necessary with global sources
        OSEnterMutex(hSceneMutex);

        if(scene)
            scene->EndScene();

        Scene *previousScene = scene;
        scene = newScene;

        scene->BeginScene();

        if(!bTransitioning)
        {
            bTransitioning = true;
            transitionAlpha = 0.0f;
        }

        OSLeaveMutex(hSceneMutex);

        delete previousScene;
    }

    return true;
}

struct HotkeyInfo
{
    DWORD hotkeyID;
    DWORD hotkey;
    OBSHOTKEYPROC hotkeyProc;
    UPARAM param;
    bool bModifiersDown, bHotkeyDown, bDownSent;
};

class OBSAPIInterface : public APIInterface
{
    friend class OBS;

    List<HotkeyInfo> hotkeys;
    DWORD curHotkeyIDVal;

    void HandleHotkeys();

    virtual bool UseHighQualityResampling() const {return AppConfig->GetInt(TEXT("Audio"), TEXT("UseHighQualityResampling"), FALSE) != 0;}

public:
    OBSAPIInterface() {bSSE2Availabe = App->bSSE2Available;}

    virtual void EnterSceneMutex() {App->EnterSceneMutex();}
    virtual void LeaveSceneMutex() {App->LeaveSceneMutex();}

    virtual void RegisterSceneClass(CTSTR lpClassName, CTSTR lpDisplayName, OBSCREATEPROC createProc, OBSCONFIGPROC configProc)
    {
        App->RegisterSceneClass(lpClassName, lpDisplayName, createProc, configProc);
    }

    virtual void RegisterImageSourceClass(CTSTR lpClassName, CTSTR lpDisplayName, OBSCREATEPROC createProc, OBSCONFIGPROC configProc)
    {
        App->RegisterImageSourceClass(lpClassName, lpDisplayName, createProc, configProc);
    }

    virtual ImageSource* CreateImageSource(CTSTR lpClassName, XElement *data)
    {
        return App->CreateImageSource(lpClassName, data);
    }

    virtual XElement* GetSceneListElement()         {return App->scenesConfig.GetElement(TEXT("scenes"));}
    virtual XElement* GetGlobalSourceListElement()  {return App->scenesConfig.GetElement(TEXT("global sources"));}

    virtual bool SetScene(CTSTR lpScene, bool bPost)
    {
        assert(lpScene && *lpScene);

        if(!lpScene || !*lpScene)
            return false;

        if(bPost)
        {
            SendMessage(hwndMain, OBS_SETSCENE, 0, (LPARAM)sdup(lpScene));
            return true;
        }

        return App->SetScene(lpScene);
    }
    virtual Scene* GetScene() const             {return App->scene;}

    virtual CTSTR GetSceneName() const          {return App->GetSceneElement()->GetName();}
    virtual XElement* GetSceneElement()         {return App->GetSceneElement();}

    virtual UINT CreateHotkey(DWORD hotkey, OBSHOTKEYPROC hotkeyProc, UPARAM param);
    virtual void DeleteHotkey(UINT hotkeyID);

    virtual Vect2 GetBaseSize() const           {return Vect2(float(App->baseCX), float(App->baseCY));}
    virtual Vect2 GetRenderFrameSize() const    {return Vect2(float(App->renderFrameWidth), float(App->renderFrameHeight));}
    virtual Vect2 GetOutputSize() const         {return Vect2(float(App->outputCX), float(App->outputCY));}

    virtual void GetBaseSize(UINT &width, UINT &height) const           {App->GetBaseSize(width, height);}
    virtual void GetRenderFrameSize(UINT &width, UINT &height) const    {App->GetRenderFrameSize(width, height);}
    virtual void GetOutputSize(UINT &width, UINT &height) const         {App->GetOutputSize(width, height);}

    virtual UINT GetMaxFPS() const              {return App->bRunning ? App->fps : AppConfig->GetInt(TEXT("Video"), TEXT("FPS"), 30);}

    virtual CTSTR GetLanguage() const           {return App->strLanguage;}

    virtual CTSTR GetAppDataPath() const        {return lpAppDataPath;}
    virtual String GetPluginDataPath() const    {return String() << lpAppDataPath << TEXT("\\pluginData");}

    virtual HWND GetMainWindow() const          {return hwndMain;}

    virtual UINT AddStreamInfo(CTSTR lpInfo, StreamInfoPriority priority)           {return App->AddStreamInfo(lpInfo, priority);}
    virtual void SetStreamInfo(UINT infoID, CTSTR lpInfo)                           {App->SetStreamInfo(infoID, lpInfo);}
    virtual void SetStreamInfoPriority(UINT infoID, StreamInfoPriority priority)    {App->SetStreamInfoPriority(infoID, priority);}
    virtual void RemoveStreamInfo(UINT infoID)                                      {App->RemoveStreamInfo(infoID);}

    virtual bool UseMultithreadedOptimizations() const {return App->bUseMultithreadedOptimizations;}

    virtual void AddAudioSource(AudioSource *source) {App->AddAudioSource(source);}
    virtual void RemoveAudioSource(AudioSource *source) {App->RemoveAudioSource(source);}

    virtual QWORD GetAudioTime() const          {return App->GetAudioTime();}
};

APIInterface* CreateOBSApiInterface()
{
    return new OBSAPIInterface;
}


void STDCALL SceneHotkey(DWORD hotkey, UPARAM param, bool bDown)
{
    if(!bDown) return;

    XElement *scenes = API->GetSceneListElement();
    if(scenes)
    {
        UINT numScenes = scenes->NumElements();
        for(UINT i=0; i<numScenes; i++)
        {
            XElement *scene = scenes->GetElementByID(i);
            DWORD sceneHotkey = (DWORD)scene->GetInt(TEXT("hotkey"));
            if(sceneHotkey == hotkey)
            {
                App->SetScene(scene->GetName());
                return;
            }
        }
    }
}

DWORD STDCALL OBS::HotkeyThread(LPVOID lpUseless)
{
    //-----------------------------------------------
    // check hotkeys.
    //   Why are we handling hotkeys like this?  Because RegisterHotkey and WM_HOTKEY
    // does not work with fullscreen apps.  Therefore, we use GetAsyncKeyState once
    // per frame instead.

    while(!App->bShuttingDown)
    {
        static_cast<OBSAPIInterface*>(API)->HandleHotkeys();
        OSSleep(30);
    }

    return 0;
}

void OBS::CallHotkey(DWORD hotkeyID, bool bDown)
{
    OBSAPIInterface *apiInterface = (OBSAPIInterface*)API;
    OBSHOTKEYPROC hotkeyProc = NULL;
    DWORD hotkey = 0;
    UPARAM param = NULL;

    OSEnterMutex(hHotkeyMutex);

    for(UINT i=0; i<apiInterface->hotkeys.Num(); i++)
    {
        HotkeyInfo &hi = apiInterface->hotkeys[i];
        if(hi.hotkeyID == hotkeyID)
        {
            if(!hi.hotkeyProc)
                return;

            hotkeyProc  = hi.hotkeyProc;
            param       = hi.param;
            hotkey      = hi.hotkey;
            break;
        }
    }

    OSLeaveMutex(hHotkeyMutex);

    hotkeyProc(hotkey, param, bDown);
}

UINT OBSAPIInterface::CreateHotkey(DWORD hotkey, OBSHOTKEYPROC hotkeyProc, UPARAM param)
{
    if(!hotkey)
        return 0;

    DWORD vk = LOBYTE(hotkey);
    DWORD modifier = HIBYTE(hotkey);
    DWORD fsModifiers = 0;

    if(modifier & HOTKEYF_ALT)
        fsModifiers |= MOD_ALT;
    if(modifier & HOTKEYF_CONTROL)
        fsModifiers |= MOD_CONTROL;
    if(modifier & HOTKEYF_SHIFT)
        fsModifiers |= MOD_SHIFT;

    OSEnterMutex(App->hHotkeyMutex);
    HotkeyInfo &hi      = *hotkeys.CreateNew();
    hi.hotkeyID         = ++curHotkeyIDVal;
    hi.hotkey           = hotkey;
    hi.hotkeyProc       = hotkeyProc;
    hi.param            = param;
    hi.bModifiersDown   = false;
    hi.bHotkeyDown      = false;
    OSLeaveMutex(App->hHotkeyMutex);

    return curHotkeyIDVal;
}

void OBSAPIInterface::DeleteHotkey(UINT hotkeyID)
{
    OSEnterMutex(App->hHotkeyMutex);
    for(UINT i=0; i<hotkeys.Num(); i++)
    {
        if(hotkeys[i].hotkeyID == hotkeyID)
        {
            hotkeys.Remove(i);
            break;
        }
    }
    OSLeaveMutex(App->hHotkeyMutex);
}

void OBSAPIInterface::HandleHotkeys()
{
    List<DWORD> hitKeys;

    DWORD modifiers = 0;
    if(GetAsyncKeyState(VK_MENU) & 0x8000)
        modifiers |= HOTKEYF_ALT;
    if(GetAsyncKeyState(VK_CONTROL) & 0x8000)
        modifiers |= HOTKEYF_CONTROL;
    if(GetAsyncKeyState(VK_SHIFT) & 0x8000)
        modifiers |= HOTKEYF_SHIFT;

    OSEnterMutex(App->hHotkeyMutex);

    for(UINT i=0; i<hotkeys.Num(); i++)
    {
        HotkeyInfo &info = hotkeys[i];

        DWORD hotkeyVK          = LOBYTE(info.hotkey);
        DWORD hotkeyModifiers   = HIBYTE(info.hotkey);

        hotkeyModifiers &= ~(HOTKEYF_EXT);

        //changed so that it allows hotkeys to be pressed even if extra modiifers are pushed
        bool bModifiersMatch = ((hotkeyModifiers & modifiers) == hotkeyModifiers);//(hotkeyModifiers == modifiers);

        if(hotkeyModifiers && !hotkeyVK) //modifier-only hotkey
        {
            if((hotkeyModifiers & modifiers) == hotkeyModifiers)
            {
                if(!info.bHotkeyDown)
                {
                    PostMessage(hwndMain, OBS_CALLHOTKEY, TRUE, info.hotkeyID);
                    info.bDownSent = true;
                    info.bHotkeyDown = true;
                }

                continue;
            }
        }
        else
        {
            if(bModifiersMatch)
            {
                short keyState   = GetAsyncKeyState(hotkeyVK);
                bool bDown       = (keyState & 0x8000) != 0;
                bool bWasPressed = (keyState & 0x1) != 0;

                if(bDown || bWasPressed)
                {
                    if(!info.bHotkeyDown && info.bModifiersDown) //only triggers the hotkey if the actual main key was pressed second
                    {
                        PostMessage(hwndMain, OBS_CALLHOTKEY, TRUE, info.hotkeyID);
                        info.bDownSent = true;
                    }

                    info.bHotkeyDown = true;
                    if(bDown)
                        continue;
                }
            }
        }

        info.bModifiersDown = bModifiersMatch;

        if(info.bHotkeyDown) //key up
        {
            if(info.bDownSent)
            {
                PostMessage(hwndMain, OBS_CALLHOTKEY, FALSE, info.hotkeyID);
                info.bDownSent = false;
            }

            info.bHotkeyDown = false;
        }
    }

    OSLeaveMutex(App->hHotkeyMutex);
}

UINT OBS::AddStreamInfo(CTSTR lpInfo, StreamInfoPriority priority)
{
    OSEnterMutex(hInfoMutex);

    StreamInfo &streamInfo = *streamInfoList.CreateNew();
    UINT id = streamInfo.id = ++streamInfoIDCounter;
    streamInfo.priority = priority;
    streamInfo.strInfo = lpInfo;

    OSLeaveMutex(hInfoMutex);

    return id;
}

void OBS::SetStreamInfo(UINT infoID, CTSTR lpInfo)
{
    OSEnterMutex(hInfoMutex);

    for(UINT i=0; i<streamInfoList.Num(); i++)
    {
        if(streamInfoList[i].id == infoID)
        {
            streamInfoList[i].strInfo = lpInfo;
            break;
        }
    }

    OSLeaveMutex(hInfoMutex);
}

void OBS::SetStreamInfoPriority(UINT infoID, StreamInfoPriority priority)
{
    OSEnterMutex(hInfoMutex);

    for(UINT i=0; i<streamInfoList.Num(); i++)
    {
        if(streamInfoList[i].id == infoID)
        {
            streamInfoList[i].priority = priority;
            break;
        }
    }

    OSLeaveMutex(hInfoMutex);
}

void OBS::RemoveStreamInfo(UINT infoID)
{
    OSEnterMutex(hInfoMutex);

    for(UINT i=0; i<streamInfoList.Num(); i++)
    {
        if(streamInfoList[i].id == infoID)
        {
            streamInfoList[i].FreeData();
            streamInfoList.Remove(i);
            break;
        }
    }

    OSLeaveMutex(hInfoMutex);
}

//todo: get rid of this and use some sort of info window.  this is a really dumb design.  what was I thinking?
String OBS::GetMostImportantInfo()
{
    OSEnterMutex(hInfoMutex);

    int bestInfoPriority = -1;
    CTSTR lpBestInfo = NULL;

    for(UINT i=0; i<streamInfoList.Num(); i++)
    {
        if((int)streamInfoList[i].priority > bestInfoPriority)
        {
            lpBestInfo = streamInfoList[i].strInfo;
            bestInfoPriority = streamInfoList[i].priority;
        }
    }

    String strInfo = lpBestInfo;
    OSLeaveMutex(hInfoMutex);

    return strInfo;
}
