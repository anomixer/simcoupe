#include "SimCoupe.h"
#include "Main.h"
#include "Audio.h"
#include "AVI.h"
#include "Actions.h"
#include "CPU.h"
#include "Debug.h"
#include "Events.h"
#include "Frame.h"
#include "GIF.h"
#include "GUI.h"
#include "Input.h"
#include "Options.h"
#include "SAMIO.h"
#include "SavePNG.h"
#include "Sound.h"
#include "Tape.h"
#include "UI.h"
#include "Video.h"
#include "WAV.h"
#include "AtaAdapter.h"
#include "Atom.h"
#include "AtomLite.h"


#ifdef __EMSCRIPTEN__
#include <SDL.h>
#include <emscripten.h>

    extern bool g_fPaused;
    extern "C" {
  EMSCRIPTEN_KEEPALIVE
  int EMS_InsertDisk(int drive, const char *filename) {
    int drive2 = GetOption(drive2);
    printf("WASM: EMS_InsertDisk(unit=%d, interface=%d, file=%s)\n", drive, drive2, filename);
    
    bool res = false;
    if (drive == 1) { // Disk 2 / Pri. Master
      if (drive2 == drvAtom && pAtom) {
        printf("WASM: Attaching to Atom Primary Master...\n");
        res = pAtom->Attach(filename, 0);
      } else if (drive2 == drvAtomLite && pAtomLite) {
        printf("WASM: Attaching to Atom Lite Primary Master...\n");
        res = pAtomLite->Attach(filename, 0);
      } else if (pFloppy2) {
        printf("WASM: Inserting into Floppy 2...\n");
        res = pFloppy2->Insert(filename);
      }
    } else if (drive == 3) { // Sec. Master
      if (drive2 == drvAtom && pAtom) {
        printf("WASM: Attaching to Atom Secondary Master...\n");
        res = pAtom->Attach(filename, 1);
      } else if (drive2 == drvAtomLite && pAtomLite) {
        printf("WASM: Attaching to Atom Lite Secondary Master...\n");
        res = pAtomLite->Attach(filename, 1);
      }
    } else { // Disk 1
      if (pFloppy1) {
        printf("WASM: Inserting into Floppy 1...\n");
        res = pFloppy1->Insert(filename);
      }
    }

    if (res) {
      printf("WASM: Insert/Attach success\n");
      // If we are at the startup screen, check if we need a hard reset to boot
      if (IO::TestStartupScreen()) {
        int drive2 = GetOption(drive2);
        // Only force Hard Reset for IDE devices (Atom/AtomLite Pri/Sec)
        // For Floppy Disk 1, the engine's internal AutoLoad (Numpad 9) will handle it.
        if ((drive == 1 || drive == 3) && (drive2 == drvAtom || drive2 == drvAtomLite)) {
          printf("WASM: IDE device detected at startup, triggering reset for boot\n");
          Actions::Do(Action::Reset, true);
          Actions::Do(Action::Reset, false);
        } else if (drive == 0) {
          // For Floppy Disk 1, explicitly trigger AutoLoad key injection (Numpad 9)
          printf("WASM: Floppy D1 detected at startup, triggering AutoLoad\n");
          IO::AutoLoad(AutoLoadType::Disk);
        }
      }
      return 1;
    }

    printf("WASM: EMS_InsertDisk failed: No matching device or pointer null\n");
    return 0;
  }

  EMSCRIPTEN_KEEPALIVE
  int EMS_InsertTape(const char *filename) {
    printf("WASM: Inserting tape %s\n", filename);
    bool fRet = Tape::Insert(filename);
    if (fRet)
      IO::AutoLoad(AutoLoadType::Tape);
    return fRet ? 1 : 0;
  }

  EMSCRIPTEN_KEEPALIVE
  int EMS_IsDiskModified(int drive) {
    if (drive == 0) return pFloppy1 && pFloppy1->IsModified();

    int drive2_type = GetOption(drive2);
    if (drive == 1) { // Disk 2 / Pri Master
        if (drive2_type == drvAtom && pAtom) return pAtom->IsModified(0);
        if (drive2_type == drvAtomLite && pAtomLite) return pAtomLite->IsModified(0);
        if (drive2_type == drvSDIDE && pSDIDE) return pSDIDE->IsModified(0);
        if (pFloppy2) return pFloppy2->IsModified();
    } else if (drive == 3) { // Sec Master
        if (drive2_type == drvAtom && pAtom) return pAtom->IsModified(1);
        if (drive2_type == drvAtomLite && pAtomLite) return pAtomLite->IsModified(1);
    }
    return 0;
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_ClearDiskModified(int drive) {
    if (drive == 0 && pFloppy1) pFloppy1->ClearModified();

    int drive2_type = GetOption(drive2);
    if (drive == 1) { // Disk 2 / Pri Master
        if (drive2_type == drvAtom && pAtom) pAtom->ClearModified(0);
        else if (drive2_type == drvAtomLite && pAtomLite) pAtomLite->ClearModified(0);
        else if (drive2_type == drvSDIDE && pSDIDE) pSDIDE->ClearModified(0);
        else if (pFloppy2) pFloppy2->ClearModified();
    } else if (drive == 3) { // Sec Master
        if (drive2_type == drvAtom && pAtom) pAtom->ClearModified(1);
        else if (drive2_type == drvAtomLite && pAtomLite) pAtomLite->ClearModified(1);
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_FlushDisk(int drive) {
    if (drive == 0 && pFloppy1) pFloppy1->Flush();

    int drive2_type = GetOption(drive2);
    if (drive == 1) { // Disk 2 / Pri Master
        if (drive2_type == drvAtom && pAtom) pAtom->Flush();
        else if (drive2_type == drvAtomLite && pAtomLite) pAtomLite->Flush();
        else if (drive2_type == drvSDIDE && pSDIDE) pSDIDE->Flush();
        else if (pFloppy2) pFloppy2->Flush();
    } else if (drive == 3) { // Sec Master
        if (drive2_type == drvAtom && pAtom) pAtom->Flush();
        else if (drive2_type == drvAtomLite && pAtomLite) pAtomLite->Flush();
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_Reset() {
    printf("WASM: Resetting emulator\n");
    Actions::Do(Action::Reset, true);
    Actions::Do(Action::Reset, false);
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_Boot(int type) {
    printf("WASM: Booting media type %d\n", type);
    IO::QueueAutoBoot(static_cast<AutoLoadType>(type));
    Actions::Do(Action::Reset, true);
    Actions::Do(Action::Reset, false);
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_EjectDisk(int drive) {
    int drive2 = GetOption(drive2);
    if (drive == 1) { // Disk 2 / Pri. Master
      if (drive2 == drvAtom && pAtom) pAtom->Attach("", 0);
      else if (drive2 == drvAtomLite && pAtomLite) pAtomLite->Attach("", 0);
      else if (pFloppy2) pFloppy2->Eject();
    } else if (drive == 3) { // Sec. Master
      if (drive2 == drvAtom && pAtom) pAtom->Attach("", 1);
      else if (drive2 == drvAtomLite && pAtomLite) pAtomLite->Attach("", 1);
    } else { // Disk 1
      if (pFloppy1) pFloppy1->Eject();
    }
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_EjectTape() {
    printf("WASM: Ejecting tape\n");
    Tape::Eject();
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_DoAction(int action_id) {
    printf("WASM: Triggering Action %d\n", action_id);
    Actions::Do(static_cast<Action>(action_id), true);
    // For toggle actions like Reset, we also need to release
    if (static_cast<Action>(action_id) == Action::Reset) {
      Actions::Do(Action::Reset, false);
    }
  }

  EMSCRIPTEN_KEEPALIVE
  const char *EMS_GetLastWavPath() {
    static std::string last_path;
    last_path = WAV::GetLastPath();
    return last_path.c_str();
  }

  EMSCRIPTEN_KEEPALIVE
  const char *EMS_GetLastGifPath() {
    static std::string last_path;
    last_path = GIF::GetLastPath();
    return last_path.c_str();
  }

  EMSCRIPTEN_KEEPALIVE
  const char *EMS_GetLastPngPath() {
    static std::string last_path;
    last_path = PNG::GetLastPath();
    return last_path.c_str();
  }

  static int g_frameCount = 0;

  EMSCRIPTEN_KEEPALIVE
  int EMS_GetFPS() {
    int fps = g_frameCount;
    g_frameCount = 0;
    return fps;
  }

  EMSCRIPTEN_KEEPALIVE
  const char *EMS_GetLastAviPath() {
    static std::string last_path;
    last_path = AVI::GetLastPath();
    return last_path.c_str();
  }

  EMSCRIPTEN_KEEPALIVE
  int EMS_GetOption(const char *name) {
    std::string optName = name;
    if (optName == "smooth")
      return GetOption(smooth) ? 1 : 0;
    if (optName == "tvaspect")
      return GetOption(tvaspect) ? 1 : 0;
    if (optName == "turbo")
      return GetOption(speed) > 100 ? 1 : 0;
    if (optName == "mainmem")
      return GetOption(mainmem);
    if (optName == "externalmem")
      return GetOption(externalmem);
    if (optName == "sid")
      return GetOption(sid);
    if (optName == "voicebox")
      return GetOption(voicebox) ? 1 : 0;
    if (optName == "saahighpass")
      return GetOption(saahighpass) ? 1 : 0;
    if (optName == "speed")
      return GetOption(speed);
    if (optName == "turbodisk")
      return GetOption(turbodisk) ? 1 : 0;
    if (optName == "dosboot")
      return GetOption(dosboot) ? 1 : 0;
    if (optName == "joytype1")
      return GetOption(joytype1);
    if (optName == "dac7c")
      return GetOption(dac7c);
    if (optName == "turbotape")
      return GetOption(turbotape) ? 1 : 0;
    if (optName == "tapetraps")
      return GetOption(tapetraps) ? 1 : 0;
    if (optName == "fastreset")
      return GetOption(fastreset) ? 1 : 0;
    if (optName == "asicdelay")
      return GetOption(asicdelay) ? 1 : 0;
    if (optName == "drivelights")
      return GetOption(drivelights);
    if (optName == "status")
      return GetOption(status) ? 1 : 0;
    if (optName == "profile")
      return GetOption(profile) ? 1 : 0;
    if (optName == "usewebgl")
      return GetOption(usewebgl) ? 1 : 0;
    if (optName == "visiblearea")
      return GetOption(visiblearea);
    if (optName == "pause")
      return g_fPaused ? 1 : 0;
    return 0;
  }

  EMSCRIPTEN_KEEPALIVE
  void EMS_SetOption(const char *name, int value) {
    std::string optName = name;
    if (optName == "smooth")
      SetOption(smooth, value != 0);
    else if (optName == "tvaspect")
      SetOption(tvaspect, value != 0);
    else if (optName == "mainmem")
      SetOption(mainmem, value);
    else if (optName == "externalmem")
      SetOption(externalmem, value);
    else if (optName == "sid")
      SetOption(sid, value);
    else if (optName == "voicebox")
      SetOption(voicebox, value != 0);
    else if (optName == "saahighpass")
      SetOption(saahighpass, value != 0);
    else if (optName == "speed")
      SetOption(speed, value);
    else if (optName == "turbodisk")
      SetOption(turbodisk, value != 0);
    else if (optName == "dosboot")
      SetOption(dosboot, value != 0);
    else if (optName == "joytype1")
      SetOption(joytype1, value);
    else if (optName == "dac7c")
      SetOption(dac7c, value);
    else if (optName == "turbotape")
      SetOption(turbotape, value != 0);
    else if (optName == "tapetraps")
      SetOption(tapetraps, value != 0);
    else if (optName == "fastreset")
      SetOption(fastreset, value != 0);
    else if (optName == "asicdelay")
      SetOption(asicdelay, value != 0);
    else if (optName == "drivelights")
      SetOption(drivelights, value);
    else if (optName == "status")
      SetOption(status, value != 0);
    else if (optName == "profile")
      SetOption(profile, value != 0);
    else if (optName == "usewebgl") {
      SetOption(usewebgl, value != 0);
      Options::Save(); // Persist before restart
      // Dynamic Video::Init() is unstable in WASM, require restart
    } else if (optName == "visiblearea") {
      SetOption(visiblearea, value);
      Frame::Init();
    } else if (optName == "pause") {
      g_fPaused = (value != 0);
    }
  }
}
#endif

static double g_last_time = 0;
static double g_accumulator = 0;

int main(int argc_, char *argv_[]) {
  if (Main::Init(argc_, argv_)) {
#ifdef __EMSCRIPTEN__
    g_last_time = emscripten_get_now();
    g_accumulator = 0;

    // WASM main loop: Hybrid Accumulator + Audio Throttle for maximum smoothness.
    emscripten_set_main_loop(
        []() {
          if (!UI::CheckEvents())
            return;
          if (g_fPaused) {
            g_last_time = emscripten_get_now();
            return;
          }

          double now = emscripten_get_now();
          double delta = now - g_last_time;
          g_last_time = now;
          
          // Cap delta to 100ms to avoid huge burst after lag (e.g. tab switched back)
          if (delta > 200.0) delta = 200.0;
          g_accumulator += delta;

          // 100% speed = 20ms per frame (50Hz)
          int nSpeed = Frame::TurboMode() ? 1000 : GetOption(speed);
          double frame_duration = 20.0 * (100.0 / std::max(10, nSpeed));

          int frames_run = 0;
          while (g_accumulator >= frame_duration) {
            
            // Audio Throttle: If we have > 150ms queued, slow down.
            // But ONLY if audio is actually being consumed (user has clicked).
            // If audio context is suspended, data piles up but never drains,
            // which would freeze the main loop entirely.
            auto queued = Audio::GetQueuedSize();
            if (queued > 28000) {
                // Check if audio is actually playing by seeing if queue drains
                static uint32_t s_last_queued = 0;
                static int s_stuck_count = 0;
                if (queued >= s_last_queued) {
                    s_stuck_count++;
                    if (s_stuck_count > 30) {
                        // Audio stuck (context suspended), clear queue and skip throttle
                        { extern SDL_AudioDeviceID dev; SDL_ClearQueuedAudio(dev); }
                        s_stuck_count = 0;
                        s_last_queued = 0;
                    } else {
                        s_last_queued = queued;
                        break;
                    }
                } else {
                    // Audio IS draining, normal throttle
                    s_last_queued = queued;
                    break;
                }
            }

            g_accumulator -= frame_duration;
            frames_run++;

            // Run until the end of the emulated frame (CPU_CYCLES_PER_FRAME)
            for (int i = 0; i < 1000000; i++) {
              if (!Debug::IsActive() && !GUI::IsModal())
                CPU::ExecuteChunk();

              if (CPU::frame_cycles >= CPU_CYCLES_PER_FRAME) {
                Frame::End();
                EventFrameEnd(CPU_CYCLES_PER_FRAME);
                IO::FrameUpdate();
                Debug::FrameEnd();
                Frame::Flyback();
                CPU::frame_cycles -= CPU_CYCLES_PER_FRAME;
                Input::Update();
                g_frameCount++;
                break;
              }
            }

            // Safety: don't spend more than 32ms in one browser tick (max 2-3 frames at 100%)
            if (emscripten_get_now() - now > 32.0)
              break;
          }
        },
        0, 1);
#endif
  }

#ifndef __EMSCRIPTEN__
  Main::Exit();
#endif

  return 0;
}

namespace Main {
bool Init(int argc_, char *argv_[]) {
  printf("Main::Init: Starting...\n");
  if (libspectrum_init() != LIBSPECTRUM_ERROR_NONE)
    return false;
  if (!Options::Load(argc_, argv_))
    return false;

  // WASM Defaults Overrides (only if not already set or first run)
  if (GetOption(visiblearea) == 0) SetOption(visiblearea, 1); // Default to Small Border
  SetOption(status, true); // Ensure status messages are on by default for WASM
  SetOption(profile, true); // Show FPS by default
  
  SetOption(autoload, true);
  if (!OSD::Init())
    return false;
  if (!Frame::Init())
    return false;
  if (!CPU::Init(true))
    return false;
  if (!UI::Init())
    return false;
  if (!Sound::Init())
    return false;
  if (!Input::Init())
    return false;
  if (!Video::Init())
    return false;
  printf("Main::Init: Success!\n");
  return true;
}

void Exit() {
  GUI::Stop();
  Video::Exit();
  Input::Exit();
  Sound::Exit();
  UI::Exit();
  CPU::Exit();
  Frame::Exit();
  OSD::Exit();
  Options::Save();
  libspectrum_end();
}
} // namespace Main
