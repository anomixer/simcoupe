# SimCoupe WebAssembly Port (Sam Coupé Emulator for Web)
by anomixer (anomy@mail.com)

[English](#english) | [繁體中文](#繁體中文)

---

<a name="english"></a>
## English

This directory contains the settings and source code for porting the SimCoupe emulator to WebAssembly (Wasm).

**Live Demo**: [https://anomixer.github.io/simcoupe/](https://anomixer.github.io/simcoupe/)

### Feature Comparison: Win32 vs. Web

| Feature | Win32 Version | Web (Wasm) Version |
| :--- | :--- | :--- |
| **User Interface** | Native Win32 Menus | Modern Web UI (Dark Mode) |
| **Rendering** | DirectDraw / GDI / OpenGL | WebGL 2.0 / Software Hybrid |
| **Options Dialog** | Modal Windows (Blocking) | Dynamic Web Menu (Non-blocking) |
| **File System** | Local File System (Native) | Virtual File System (VFS) / Downloads |
| **Debugger** | Integrated Modal Debugger | **Disabled** (Prevents browser hangs) |
| **New Disk/Tape** | Native Dialogs | **Disabled** (Upload existing images) |
| **Import/Export Data** | Supported | **Disabled** (No memory load/save) |
| **Cloud Loading** | Local Files Only | **Advanced URL Parameters** (with **CORS Proxy** support) |
| **Media Export** | Manual "Save As" Dialog | **Automatic Browser Download** |
| **Data Protection** | Native File Writes | **Save on Eject Confirmation** (Prompts to download modified images) |
| **Persistence** | .ini files / Registry | LocalStorage / URL Arguments |
| **Archive Support** | .zip, .gz (Native) | .zip, .gz, **.7z (Enhanced)** |

---

### Directory Structure

- `wasm/CMakeLists.txt`: Build settings designed for Emscripten.
- `wasm/src/`: Custom source overrides for the Wasm environment (non-intrusive).
- `wasm/deploy/`: The final build artifacts and web template.
- `package/make_wasm_release.ps1`: Fully automated build script.

### How to Build

1. Open PowerShell.
2. Run the build script:
   ```powershell
   .\package\make_wasm_release.ps1
   ```
   or
   ```cmd
   .\Compile.bat
   ```
   *The script automatically checks for EMSDK and downloads necessary dependencies.*

### How to Run

After building, the artifacts are located in `wasm/deploy/`. Due to Wasm security restrictions, they must be served via a Web Server:

#### Using Node.js (Recommended)
```powershell
npx serve wasm/deploy
```

#### Using Python
```powershell
python -m http.server --directory wasm/deploy
```
or
```cmd
.\Run.bat
```

Then open your browser and navigate to `http://localhost:8000`.

### Key Features

- **Hybrid Rendering Engine**: Supports both **WebGL 2.0 (High Performance)** and **Software (High Compatibility)** backends. If you encounter a black screen or rendering issues, you can manually switch to Software mode via the `View` menu.
- **Cloud Loading**: Supports loading disk and tape images directly from URLs (HTTP/HTTPS). Use URL parameters like `?d1=...` or `?tape=...` to automatically load and **Auto-Boot** the software on startup.
- **Media Auto-Export System**: Recorded WAV, GIF, AVI, or screenshots (PNG) automatically trigger a browser download upon completion.
- **Desktop-Grade UI**: Professional menu bar with real-time state sync, a beautifully centered three-column status bar with live FPS monitoring, and **Smart Path Truncation** for long filenames.
- **Window Zoom System**: Options from 50% to 500% zoom, including a **Fit to Window** auto-adaptation mode.
- **Precision 50 FPS & Multi-Speed**: A microsecond-level accumulator ensures flawless 50 FPS synchronization across any browser refresh rate. Features a full multi-speed menu (50% to 1000%) with continuous audio playback even at high speeds.
- **Atom/AtomLite IDE Support**: Integrated support for Atom and AtomLite hardware interfaces, allowing the mounting of **.hdf hard disk images** to Primary and Secondary master IDE units.
- **Configurable Visible Area**: Ported the classic Win32 border settings, offering **No Border**, **Small Border (Default)**, **TV Visible**, and **Full Active** modes with real-time viewport adaptation.
- **Intelligent UI Feedback**: Menu items like **Eject Disk/Tape** and **Stop Recording** automatically grey out when inactive, providing clear guidance and preventing accidental clicks.
- **Automatic CORS Proxy Fallback**: Integrated a fallback mechanism for cloud loading. If a remote server (like Archive.org) blocks a direct download due to CORS, the emulator automatically retries via **corsfix** proxy to ensure the software loads successfully.
- **Interactive Start-up Splash**: A Win32-style 'About' window automatically appears at a refined vertical position on startup (or closes on click), providing a premium first impression and clear version information.
- **Professional Visual Identity**: Restored the original high-fidelity blue/red "SIM Coupé" logo and robot icon with pixel-perfect transparency. Includes a custom favicon for the web environment.
- **Native 7z Archive Support**: Integrated `7z-wasm` to provide full support for compressed disk, hard disk, and tape images. The emulator automatically extracts and mounts the media in-memory, significantly reducing data transmission time for large HDF images.
- **Non-blocking Custom Modals**: Replaced all blocking browser dialogs (`alert`, `confirm`, `prompt`) with a custom UI system. This ensures the emulator and its audio continue to run seamlessly in the background while users interact with menu commands or error prompts.
- **Save on Eject Protection**: Added a data protection layer that detects disk modifications. When ejecting a modified disk or hard disk image, the emulator prompts the user to download the updated image, ensuring that in-game progress or code changes are never lost.

### Cloud Loading Syntax
You can load software directly via URL parameters. This will automatically download, mount, and **Auto-Boot** the emulator:

- `interface`: Set hardware interface (`floppy`, `atom`, or `atomlite`). Automatically triggers a BIOS/ROM reload.
- `d1`: URL to Disk 1 image (.dsk, .mgt, .sad, .gz, .zip, .7z).
- `d2`: URL to Disk 2 / Primary Master HDF (.hdf, .gz, .zip, .7z).
- `d3`: URL to Secondary Master HDF (.hdf, .gz, .zip, .7z).
- `tape`: URL to Tape image (.tap, .tzx, .gz, .zip, .7z).

**Example 1**: Floppy Disk: Sam Coupé official Demo
[https://anomixer.github.io/simcoupe/?d1=https://archive.org/download/Sam_Coupe_TOSEC_2012_04_23/Sam_Coupe_TOSEC_2012_04_23.zip/Sam%20Coupe%5BTOSEC%5D%2FMGT%20Sam%20Coupe%20-%20Demos%20-%20%5BDSK%5D%20%28TOSEC-v2008-12-21_CM%29%2FZZZ-UNK-officialsamcomputersdemo1991.zip](https://anomixer.github.io/simcoupe/?d1=https://archive.org/download/Sam_Coupe_TOSEC_2012_04_23/Sam_Coupe_TOSEC_2012_04_23.zip/Sam%20Coupe%5BTOSEC%5D%2FMGT%20Sam%20Coupe%20-%20Demos%20-%20%5BDSK%5D%20%28TOSEC-v2008-12-21_CM%29%2FZZZ-UNK-officialsamcomputersdemo1991.zip)

**Example 2**: HDF Disk: Sample for SAMCon '94 Music Demo (AtomLite)
[https://anomixer.github.io/simcoupe/?interface=atomlite&d2=https://sam.speccy.cz/demo/samples4samcon_hdf.7z](https://anomixer.github.io/simcoupe/?interface=atomlite&d2=https://sam.speccy.cz/demo/samples4samcon_hdf.7z)

**Example 3**: Floppy Disk: Prince of Persia Game (Floppy)
https://anomixer.github.io/simcoupe/?interface=floppy&d1=https://www.worldofsam.org/index.php/system/files/2022-02/PrinceOfPersia_HDD.dsk

### Technical Notes

- **Non-Intrusive Modification**: Uses a temporary file shadowing technique. Custom versions in `wasm/src/` are used during build and original files are restored immediately after, preserving the native development environment.
- **Main Loop Refactoring**: Since browsers don't allow blocking calls, `CPU::Run()` was refactored into `Iteration()` and managed via `emscripten_set_main_loop`.
- **WebGL 2.0**: Uses GLES 3.0 standards with corrected Shader syntax and texture formats for browser compatibility.
- **Dependency Management**: Automatically fetches and sanitizes third-party libraries (e.g., z80, saasound) to prevent build conflicts in the Wasm environment.

### Differences from Native Version

To ensure browser stability and prevent hangs, some native Win32 features are intentionally disabled or modified:
- **Internal Modal Dialogs**: Features like `New Disk`, `Import/Export`, `Tape Browser`, and the native `Debugger` are disabled as they use blocking Win32 dialogs that are incompatible with Wasm's single-threaded nature.
- **Hardware Configuration**: All hardware settings (Memory, SID, DAC, Joysticks) have been ported to the Web Menu Bar. Users should use the web interface instead of the native Options dialog.
- **File System**: Uses a Virtual File System (VFS). Files are accessed via the web file picker and exported via automatic browser downloads.
- **Speed Emulation**: The Wasm version natively supports precise speed controls from 50% up to 1000%, ensuring full audio functionality even during fast-forwarding, overcoming traditional browser refresh rate limitations.

### Known Limitations
- **Hybrid Rendering**: Defaults to `SimCoupe/GL3` (WebGL 2.0). If you experience display issues, go to `View -> Use WebGL Acceleration` and uncheck it. The emulator will prompt for a refresh and persist the setting using `localStorage` and command-line arguments.
- Audio may require a user gesture (click) to start in some browsers.
- **Third-Party Services**:
  - **7z-wasm**: Distributed via [jsDelivr](https://www.jsdelivr.com/) for in-browser decompression.
  - **CORS Proxy**: Uses [corsfix](https://corsfix.com/) as a fallback for cloud loading from restrictive servers.

### Q&A (Frequently Asked Questions)

**Q: How is this version different from the native executable?**

A: This version runs directly in your browser using WebAssembly. It features a modern web UI, integrated cloud loading, and automated media downloads. However, some native features that cause browser hangs (like the built-in Debugger or New Disk dialogs) have been disabled to ensure stability.

**Q: What features has been added?**

A: We added **native 7z archive support**, **Cloud Media Loading** (via URL parameters), a **CORS Proxy fallback**, and a high-performance **WebGL 2.0 rendering engine**.

**Q: Why is there no sound?**

A: Modern browsers require a user gesture (like a click) to enable audio. Just click anywhere on the canvas or UI to activate the sound.

**Q: I see a black screen or no video. What should I do?**

A: Some browsers or hardware may have WebGL 2.0 compatibility issues. Try a different browser (Chrome, Edge, or Firefox are recommended), or go to `View -> Use WebGL Acceleration` and uncheck it to switch to software rendering.

**Q: Why are menu checkmarks or statuses incorrect?**

A: This can happen if the UI hasn't fully synchronized with the engine state on first load. Try refreshing the page (F5) to force a sync.

**Q: I tried to use URL cloud parameters after the page was already loaded, but it didn't work.**

A: URL parameters like `?d1=...` are only parsed during the initial boot sequence. Please open the link in a **new browser tab** to trigger the auto-load logic.

**Q: I get a CORS error message when loading from a URL. What should I do?**

A: This usually happens when the remote server fails to respond to the proxy request. Just **refresh the page (F5)** and try again; it usually resolves on the second attempt.

### Credits

- **Original Emulator (SimCoupe)**: Simon Owen & contributors
- **WebAssembly Port & Web UI**: anomixer & Antigravity (Agentic AI)

---

<a name="繁體中文"></a>
## 繁體中文

這個目錄包含了將 SimCoupe 模擬器移植到 WebAssembly (Wasm) 的相關設定與源碼。

**線上體驗**: [https://anomixer.github.io/simcoupe/](https://anomixer.github.io/simcoupe/)

### 功能特性比較：Win32 vs. Web

| 功能特性 | Win32 原生版本 | Web (Wasm) 版本 |
| :--- | :--- | :--- |
| **使用者介面** | 原生 Win32 選單 | 現代化 Web UI (黑夜模式) |
| **渲染後端** | DirectDraw / GDI / OpenGL | WebGL 2.0 / 軟體渲染混合引擎 |
| **設定對話框** | 阻塞式視窗 (Modal) | 動態網頁選單 (非阻塞) |
| **檔案系統** | 本地檔案系統 (原生) | 虛擬檔案系統 (VFS) / 瀏覽器下載 |
| **除錯器 (Debugger)** | 內置阻塞式除錯器 | **已禁用** (避免瀏覽器凍結) |
| **建立空白磁碟/帶** | 內置對話框 | **已禁用** (請於本地建立後上傳) |
| **匯入/匯出資料** | 支援 | **已禁用** (無直接記憶體存取) |
| **雲端載入** | 僅支援本地檔案 | **進階 URL 參數載入** (支援 **CORS 修正**) |
| **多媒體導出** | 彈出對話框手動存檔 | **自動導出並觸發下載** |
| **資料保護機制** | 直接寫入本地檔案 | **退出存檔提示** (偵測修改並提醒使用者下載存檔) |
| **設定持久化** | .ini 檔案 / 登錄表 | LocalStorage / URL 參數 |
| **壓縮檔支援** | .zip, .gz (原生) | .zip, .gz, **.7z (進階支援)** |

---

### 目錄結構

- `wasm/CMakeLists.txt`: 專為 Emscripten 設計的建置設定。
- `wasm/src/`: 存放針對 Wasm 環境覆寫的源碼（非侵入式）。
- `wasm/deploy/`: 編譯後的成品與網頁範本。
- `package/make_wasm_release.ps1`: 全自動建置腳本。

### 如何建置

1. 開啟 PowerShell。
2. 執行建置腳本：
   ```powershell
   .\package\make_wasm_release.ps1
   ```
   或
   ```cmd
   .\Compile.bat
   ```
   *腳本會自動檢查並安裝 EMSDK，並下載必要的依賴庫。*

### 如何執行

編譯完成後，檔案會存放在 `wasm/deploy/`。由於 Wasm 的安全限制，必須透過 Web Server 執行：

#### 使用 Node.js (推荐)
```powershell
npx serve wasm/deploy
```

#### 使用 Python
```powershell
python -m http.server --directory wasm/deploy
```
或
```cmd
.\Run.bat
```

然後開啟瀏覽器， 瀏覽 `http://localhost:8000`。

### 主要功能與特點

- **雙模渲染引擎**: 同時支援 **WebGL 2.0 (高性能)** 與 **軟體渲染 (高相容性)**。若在特定設備遇到黑屏，可透過 `View` 選單手動切換至軟體模式。
- **媒體自動導出系統**: 錄製的 WAV, GIF, AVI 或螢幕截圖 (PNG) 會在停止錄製後自動觸發瀏覽器下載，無需手動管理虛擬檔案系統。
- **桌面級選單介面**: 仿作業系統的選單列，具備即時狀態同步、完美置中的三欄式狀態列與即時 FPS 監控，並支援長檔名的智慧截斷顯示。
- **Window Zoom 系統**: 提供 50% 到 500% 的縮放選項，並包含 **Fit to Window** 自動適應模式。
- **Atom/AtomLite IDE 支援**: 深度整合 Atom 與 AtomLite 硬體介面，支援將 **.hdf 硬碟影像檔** 掛載至 IDE Primary 或 Secondary Master 單元。
- **可自定義顯示區域**: 移植了 Win32 版經典邊框設定，提供 **No Border**、**Small Border (預設)**、**TV Visible** 與 **Full Active** 四種模式，並實作即時畫面適應。
- **智慧型 UI 回饋**: 選單中的「彈出磁碟」與「停止錄影」選項會在無效時自動變灰 (Grey-out)，提供更直覺的操作引導。
- **精確 50 FPS 與多段速切換**: 透過微秒級時間累加器，無論瀏覽器更新率為何都能維持完美的 50 FPS。內建 50% 到 1000% 的多段速選單，且快轉時依然保留流暢的音效體驗。
- **自動 CORS Proxy 備援**: 針對雲端載入實作了自動代理機制。若遠端伺服器（如 Archive.org）因 CORS 限制擋下直接下載，系統會自動透過 **corsfix** 代理伺服器重試，大幅提升載入成功率。
- **互動式開場畫面**: 仿 Win32 風格的「About」視窗會在啟動時出現在優化後的垂直位置，提供極佳的第一印象與清晰的版本資訊。
- **專業視覺標誌**: 找回並修復了原版高質感的藍紅「SIM Coupé」標誌與機器人圖示，實作了像素級去背 (Transparency) ，並整合為網頁專屬 Favicon。
- **原生 7z 壓縮檔支援**: 整合了 `7z-wasm` 技術，全面支援壓縮格式的磁碟、硬碟與磁帶影像。系統會自動在記憶體中完成解壓與掛載，大幅縮短了大型 HDF 映像檔的傳輸時間。
- **非阻塞式自定義模態視窗**: 將所有會導致執行中斷的瀏覽器原生彈窗 (`alert`, `confirm`, `prompt`) 替換為自定義 UI 系統。確保使用者在處理選單指令或錯誤提示時，模擬器的畫面與音效依然能在背景流暢運作。
- **退出磁碟存檔提示**: 針對 WASM 虛擬環境實作了資料保護層。當使用者退出被修改過的磁碟或硬碟影像時，系統會自動彈出提示詢問是否存檔，確保遊戲進度或程式修改不會遺失。

### 雲端載入語法範例
你可以透過網址參數直接載入軟體，系統會自動下載、掛載並執行 **Auto-Boot**：

- `interface`: 設定硬體介面 (`floppy`, `atom`, 或 `atomlite`)，系統會自動觸發 BIOS/ROM 重載。
- `d1`: 磁碟 1 的 URL (.dsk, .mgt, .sad, .gz, .zip, .7z)。
- `d2`: 磁碟 2 / Primary Master HDF 的 URL (.hdf, .gz, .zip, .7z)。
- `d3`: Secondary Master HDF 的 URL (.hdf, .gz, .zip, .7z)。
- `tape`: 錄音帶的 URL (.tap, .tzx, .gz, .zip, .7z)。

**範例連結1**: 磁碟: Sam Coupé 官方示範片
[https://anomixer.github.io/simcoupe/?d1=https://archive.org/download/Sam_Coupe_TOSEC_2012_04_23/Sam_Coupe_TOSEC_2012_04_23.zip/Sam%20Coupe%5BTOSEC%5D%2FMGT%20Sam%20Coupe%20-%20Demos%20-%20%5BDSK%5D%20%28TOSEC-v2008-12-21_CM%29%2FZZZ-UNK-officialsamcomputersdemo1991.zip](https://anomixer.github.io/simcoupe/?d1=https://archive.org/download/Sam_Coupe_TOSEC_2012_04_23/Sam_Coupe_TOSEC_2012_04_23.zip/Sam%20Coupe%5BTOSEC%5D%2FMGT%20Sam%20Coupe%20-%20Demos%20-%20%5BDSK%5D%20%28TOSEC-v2008-12-21_CM%29%2FZZZ-UNK-officialsamcomputersdemo1991.zip)

**範例連結2**: 硬碟: Sample for SAMCon '94 音樂示範片 (AtomLite)
[https://anomixer.github.io/simcoupe/?interface=atomlite&d2=https://sam.speccy.cz/demo/samples4samcon_hdf.7z](https://anomixer.github.io/simcoupe/?interface=atomlite&d2=https://sam.speccy.cz/demo/samples4samcon_hdf.7z)

**範例連結3**: 磁碟: Prince of Persia 遊戲 (Floppy)
[https://anomixer.github.io/simcoupe/?interface=floppy&d1=https://www.worldofsam.org/index.php/system/files/2022-02/PrinceOfPersia_HDD.dsk](https://anomixer.github.io/simcoupe/?interface=floppy&d1=https://www.worldofsam.org/index.php/system/files/2022-02/PrinceOfPersia_HDD.dsk)

### 技術細節

- **非侵入式修改**: 採用了暫時性檔案更名技術，在編譯期間優先使用 `wasm/src/` 中的自定義版本，建置結束後會自動還原原始檔案，確保不破壞原作者的開發環境。
- **主迴圈重構**: 由於瀏覽器環境不允許阻塞，我們將 `CPU::Run()` 拆解為 `Iteration()`，並透過 `emscripten_set_main_loop` 呼叫。
- **WebGL 2.0**: 使用 OpenGL ES 3.0 規範，修正了 Shader 語法與紋理格式以符合瀏覽器標準。
- **依賴項管理**: 自動下載並清理第三方庫（如 z80, saasound）中的測試程式，避免 Wasm 環境下的編譯衝突。

### 與原生版本的差異

為了確保瀏覽器穩定性並防止畫面凍結，部分原生 Win32 功能在 WASM 版中已被封鎖或修改：
- **內置對話框禁用**: `New Disk`、`Import/Export`、`Tape Browser` 以及內置的 `Debugger` 已被禁用，因為這些功能會呼叫阻塞式視窗，導致瀏覽器環境當機。
- **硬體設定移植**: 所有的硬體設定（記憶體、SID、DAC、搖桿）都已移植至網頁選單列。請使用網頁介面進行設定，而非呼叫原版的 Options 對話框。
- **檔案系統**: 使用虛擬檔案系統 (VFS)。檔案透過網頁上傳器載入，並透過瀏覽器自動下載導出。
- **模擬速度限制**: Wasm 版現已完全支援 Win32 版的 50% 到 1000% 多段百分比調整，且解處理了快轉時的靜音限制，提供原汁原味的流暢加速體驗。

### 已知限制
- **混合渲染**: 目前預設使用 `SimCoupe/GL3` (WebGL 2.0)。若遇到顯示問題，請至 `View -> Use WebGL Acceleration` 取消勾選。系統會提示並自動重新整理，透過 `localStorage` 與命令行參數確保設定持久生效。
- 音訊在某些瀏覽器可能需要使用者點擊頁面後才能啟動。
- **第三方服務使用聲明**:
  - **7z-wasm**: 透過 [jsDelivr](https://www.jsdelivr.com/) 載入 ES Module，實作瀏覽器內建解壓功能。
  - **CORS 代理**: 整合了 [corsfix](https://corsfix.com/) 作為備援，確保能從 Archive.org 等限制嚴格的伺服器載入雲端媒體。

### Q&A (常見問答)

**Q: 這個版本與原本的執行檔版有什麼不同？**

A: 此版本透過 WebAssembly 技術直接在瀏覽器中執行。它具備現代化的網頁介面、雲端載入功能以及自動下載導出。為了確保瀏覽器穩定性，我們禁用了部分會導致網頁凍結的原生功能（如內置除錯器或建立磁碟對話框）。

**Q: 增加了什麼新功能？**

A: 我們加入了 **原生 7z 壓縮檔支援**、**雲端媒體載入**（透過網址參數）、**CORS 代理自動備援**，以及高性能的 **WebGL 2.0 渲染引擎**。

**Q: 為什麼沒有聲音？**

A: 現代瀏覽器規定必須有使用者互動（如點擊）後才能啟動音訊。請在畫面（Canvas）上點擊任意處即可啟動聲音。

**Q: 沒畫面或顯示黑屏怎麼辦？**

A: 部分瀏覽器或硬體可能存在 WebGL 2.0 相容性問題。請嘗試更換瀏覽器（推薦使用 Chrome、Edge 或 Firefox），或到選單 `View -> Use WebGL Acceleration` 取消勾選以切換至軟體渲染模式。

**Q: 為什麼選單的功能表沒打勾或狀態不對？**

A: 這可能是因為介面在初始載入時尚未與引擎狀態完全同步。請嘗試重新載入網頁（F5）即可解決。

**Q: 我在網址列輸入雲端參數（如 ?d1=...），但載入後沒反應？**

A: 雲端載入參數僅在模擬器啟動初期進行解析。請在**新的瀏覽器分頁**中開啟完整網址，以觸發自動載入邏輯。

**Q: 載入 URL 時出現 CORS 錯誤訊息怎麼辦？**

A: 這通常是遠端伺服器回應代理請求失敗所致。只要**重新整理頁面 (F5)** 再試一次即可，第二次通常就能成功載入。

### 鳴謝 (Credits)

- **原始模擬器 (SimCoupe)**: Simon Owen 及貢獻者
- **WebAssembly 移植與網頁介面**: anomixer 與 Antigravity (Agentic AI)
