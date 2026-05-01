#include "SimCoupe.h"
#include "UI.h"
#include "Actions.h"
#include "CPU.h"
#include "Frame.h"
#include "GUI.h"
#include "Input.h"
#include "Options.h"
#include "SAMIO.h"
#include "Sound.h"
#include "SDL20.h"
#include "SDL20_GL3.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

bool g_audioEnabled = false;

extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void EMS_SetAudioEnabled(int enabled) {
        g_audioEnabled = (enabled != 0);
        printf("WASM: Audio %s\n", g_audioEnabled ? "enabled" : "disabled");
    }

    EMSCRIPTEN_KEEPALIVE
    void EMS_StartAudio() {
        extern SDL_AudioDeviceID dev;
        if (dev) {
            SDL_PauseAudioDevice(dev, 0);
        }
        EMS_SetAudioEnabled(1);
    }
}

bool UI::Init(bool fFirstInit_/*=false*/)
{
    Exit(true);
    SDL_EventState(SDL_DROPFILE, SDL_ENABLE);
    SDL_ShowCursor(SDL_DISABLE);
    return true;
}

void UI::Exit(bool fReInit_/*=false*/)
{
}

std::unique_ptr<IVideoBase> UI::CreateVideo()
{
#ifdef HAVE_OPENGL
    if (GetOption(usewebgl))
    {
        if (auto backend = std::make_unique<SDL_GL3>(); backend->Init())
        {
            return backend;
        }
        printf("WASM: WebGL initialization failed, falling back to software rendering.\n");
    }
#endif

    if (auto backend = std::make_unique<SDLTexture>(); backend->Init())
    {
        return backend;
    }

    return nullptr;
}

bool UI::CheckEvents()
{
    SDL_Event event;

    if (GUI::IsActive())
        Input::Update();

    while (SDL_PollEvent(&event))
    {
        if (Input::FilterEvent(&event))
            continue;

        switch (event.type)
        {
        case SDL_QUIT:
            return false;

        case SDL_DROPFILE:
            if (GetOption(drive1) != drvFloppy)
                ShowMessage(MsgType::Warning, "Floppy drive 1 is not present");
            else if (pFloppy1->Insert(event.drop.file))
            {
                Frame::SetStatus("{}  inserted into drive 1", pFloppy1->DiskFile());
                IO::AutoLoad(AutoLoadType::Disk);
            }
            SDL_free(event.drop.file);
            break;

        case SDL_USEREVENT:
        {
            switch (event.user.code)
            {
            case UE_RESETBUTTON:
                Actions::Do(Action::Reset, true);
                Actions::Do(Action::Reset, false);
                break;
            case UE_PAUSE:              Actions::Do(Action::Pause);           break;
            case UE_OPTIONS:            Actions::Do(Action::Options);         break;
            case UE_TOGGLEFULLSCREEN:   Actions::Do(Action::ToggleFullscreen); break;
            default: break;
            }
        }
        }
    }
    return true;
}

void UI::ShowMessage(MsgType type, const std::string& str)
{
    constexpr auto caption = "SimCoupe";
    if (type == MsgType::Info)
        GUI::Start(new MsgBox(nullptr, str, caption, mbInformation));
    else if (type == MsgType::Warning)
        GUI::Start(new MsgBox(nullptr, str, caption, mbWarning));
    else
        GUI::Start(new MsgBox(nullptr, str, caption, mbError));
}

bool UI::DoAction(Action action, bool pressed)
{
    if (!pressed) return false;

    // Intercept actions that open internal modal dialogs which cause hangs in WASM
    switch (action)
    {
    case Action::InsertDisk1:
    case Action::InsertDisk2:
    case Action::NewDisk1:
    case Action::NewDisk2:
    case Action::ImportData:
    case Action::ExportData:
    case Action::Options:
    case Action::Debugger:
    case Action::TapeBrowser:
    case Action::ToggleRasterDebug:
        printf("WASM: Blocked internal modal action %d. Please use the web menu instead.\n", (int)action);
        return true; 

    case Action::ExitApp:
    {
        SDL_Event event = { SDL_QUIT };
        SDL_PushEvent(&event);
        return true;
    }
    default:
        break;
    }
    return false;
}
