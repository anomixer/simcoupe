# WASM build of SimCoupe using Emscripten

$ErrorActionPreference = 'Stop'
$build_dir = "build-wasm"

# 1. Clean environment
$env:INCLUDE = ""
$env:LIB = ""
$env:CPATH = ""

# 2. Files to temporarily override
$overrides = @(
    "Base/Main.cpp",
    "SDL/SDL20_GL3.cpp",
    "SDL/SDL20_GL3.h",
    "Base/SimCoupe.h",
    "SDL/SDL20.h",
    "SDL/OSD.cpp"
)

function Backup-Files {
    foreach ($file in $overrides) {
        $path = Join-Path $PSScriptRoot "..\$file"
        if (Test-Path $path) {
            Write-Host "Backing up $file..."
            Move-Item $path "$path.bak" -Force
        }
    }
}

function Restore-Files {
    foreach ($file in $overrides) {
        $path = Join-Path $PSScriptRoot "..\$file"
        if (Test-Path "$path.bak") {
            Write-Host "Restoring $file..."
            Move-Item "$path.bak" $path -Force
        }
    }
}

try {
    # 3. Check and install CMake/Ninja via winget if missing
    if (!(Get-Command cmake.exe -ErrorAction SilentlyContinue)) {
        Write-Host "CMake not found. Attempting to install via winget..." -ForegroundColor Cyan
        Start-Process winget -ArgumentList "install Kitware.CMake --accept-package-agreements --accept-source-agreements" -Wait -NoNewWindow
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }
    if (!(Get-Command ninja.exe -ErrorAction SilentlyContinue)) {
        Write-Host "Ninja not found. Attempting to install via winget..." -ForegroundColor Cyan
        Start-Process winget -ArgumentList "install Ninja-build.Ninja --accept-package-agreements --accept-source-agreements" -Wait -NoNewWindow
        $env:Path = [System.Environment]::GetEnvironmentVariable("Path", "Machine") + ";" + [System.Environment]::GetEnvironmentVariable("Path", "User")
    }

    # Find Ninja (prioritize PATH, then VS)
    $ninja_path = Get-Command ninja.exe -ErrorAction SilentlyContinue | Select-Object -ExpandProperty Source
    if (!$ninja_path) {
        $vs_ninja = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"
        if (!(Test-Path $vs_ninja)) { $vs_ninja = $vs_ninja -replace "Community", "Professional" }
        if (!(Test-Path $vs_ninja)) { $vs_ninja = $vs_ninja -replace "Professional", "Enterprise" }
        if (Test-Path $vs_ninja) { $ninja_path = $vs_ninja }
    }
    if ($ninja_path) { 
        $env:Path = (Split-Path $ninja_path) + ";" + $env:Path 
        Write-Host "Using Ninja: $ninja_path" -ForegroundColor Gray
    }

    # 4. Initialize Emscripten (Relative to Repo Root)
    $repo_root = [System.IO.Path]::GetFullPath((Join-Path $PSScriptRoot ".."))
    Set-Location $repo_root
    
    $emsdk_path = Join-Path $repo_root "emsdk-wasm"
    $ems_dir = Join-Path $emsdk_path "upstream\emscripten"
    $script:emcmake_cmd = Join-Path $ems_dir "emcmake.bat"

    if (!(Test-Path $script:emcmake_cmd)) {
        Write-Host "`nEMSDK not found. Installing into repo: $emsdk_path" -ForegroundColor Cyan
        
        if (!(Test-Path $emsdk_path)) {
            git clone https://github.com/emscripten-core/emsdk.git $emsdk_path
        }
        
        Push-Location $emsdk_path
        ./emsdk.ps1 install latest
        ./emsdk.ps1 activate latest
        Pop-Location
    }

    # Force inject paths and essential Emscripten variables
    Write-Host "Configuring Emscripten environment variables..." -ForegroundColor Green
    $bin_dir = Join-Path $emsdk_path "upstream\bin"
    $env:Path = "$ems_dir;$bin_dir;" + $env:Path
    
    # Add Node and set EM_NODE_JS
    $node_dir_obj = Get-ChildItem -Path (Join-Path $emsdk_path "node") -Directory | Select-Object -First 1
    if ($node_dir_obj) {
        $node_root = $node_dir_obj.FullName
        $node_bin = Join-Path $node_root "bin"
        if (!(Test-Path (Join-Path $node_bin "node.exe"))) { $node_bin = $node_root }
        $env:Path = "$node_bin;" + $env:Path
        $env:EM_NODE_JS = Join-Path $node_bin "node.exe"
    }
    
    # Add Python and set EMSDK_PYTHON
    $python_dir_obj = Get-ChildItem -Path (Join-Path $emsdk_path "python") -Directory | Select-Object -First 1
    if ($python_dir_obj) {
        $python_dir = $python_dir_obj.FullName
        $python_exe = Join-Path $python_dir "python.exe"
        $env:Path = "$python_dir;" + $env:Path
        $env:EMSDK_PYTHON = $python_exe
        
        # Build py.exe proxy (using absolute path to python.exe)
        $py_exe = Join-Path $python_dir "py.exe"
        if (!(Test-Path $py_exe)) {
            Write-Host "Re-building py.exe proxy..." -ForegroundColor Yellow
            $source = @"
using System; using System.Diagnostics; using System.Linq; 
public class PyProxy { 
    public static void Main(string[] args) { 
        var filteredArgs = args.Where(a => a != "-3").ToArray(); 
        var psi = new ProcessStartInfo(@"$python_exe", string.Join(" ", filteredArgs.Select(a => "\"" + a + "\""))) { 
            UseShellExecute = false, CreateNoWindow = false 
        }; 
        try { var p = Process.Start(psi); p.WaitForExit(); Environment.Exit(p.ExitCode); } 
        catch { Environment.Exit(1); } 
    } 
}
"@
            Add-Type -TypeDefinition $source -OutputAssembly $py_exe -ReferencedAssemblies "System.dll"
        }
    }

    # Final check
    if (!(Test-Path $script:emcmake_cmd)) {
        Write-Error "CRITICAL: emcmake.bat still missing after setup at $script:emcmake_cmd"
        exit 1
    }

    # 5. Backup originals to force overrides
    Backup-Files

    # 6. Build (preserve existing build dir to keep downloaded deps)
    if (!(Test-Path $build_dir)) {
        mkdir $build_dir | Out-Null
    }
    
    # Download dependencies manually if missing
    $deps = @{
        "z80"      = "https://github.com/kosarev/z80/archive/9917a37.zip";
        "fmt"      = "https://github.com/fmtlib/fmt/archive/e69e5f97.zip";
        "resid"    = "https://github.com/simonowen/resid/archive/9ac2b4b.zip";
        "spectrum" = "https://github.com/simonowen/libspectrum/archive/eb93bd3.zip";
        "saasound" = "https://github.com/simonowen/SAASound/archive/7ceef2a.zip"
    }

    mkdir "$build_dir/deps" -ErrorAction SilentlyContinue | Out-Null
    mkdir "$build_dir/_deps" -ErrorAction SilentlyContinue | Out-Null
    foreach ($dep in $deps.Keys) {
        $dest = "$build_dir/deps/$dep.zip"
        $extract_to = "$build_dir/_deps/$dep-src"
        
        if (!(Test-Path $dest) -or (Get-Item $dest).Length -eq 0) {
            Write-Host "Downloading $dep..."
            try {
                Invoke-WebRequest -Uri $deps[$dep] -OutFile $dest -TimeoutSec 60
                Unblock-File $dest
            } catch {
                Write-Warning "Failed to download $dep."
            }
        }

        if (!(Test-Path $extract_to)) {
            Write-Host "Extracting $dep..."
            $temp_extract = "$build_dir/_deps/temp_$dep"
            mkdir $temp_extract | Out-Null
            Expand-Archive -Path $dest -DestinationPath $temp_extract -Force
            $root_folder = Get-ChildItem $temp_extract | Select-Object -First 1
            Move-Item $root_folder.FullName $extract_to -Force
            Remove-Item $temp_extract -Recurse -Force

            if (Test-Path "$extract_to/tests") { Remove-Item "$extract_to/tests" -Recurse -Force }
            if (Test-Path "$extract_to/examples") { Remove-Item "$extract_to/examples" -Recurse -Force }

            $dep_cmake = "$extract_to/CMakeLists.txt"
            if (Test-Path $dep_cmake) {
                $content = Get-Content $dep_cmake
                $content = $content | Where-Object { $_ -notmatch 'add_subdirectory\s*\((tests|examples|test|Example)\)' }
                if ($dep -eq "z80" -and ($content -notmatch "add_library\s*\(\s*z80")) {
                    $content += "`nadd_library(z80 INTERFACE)"
                    $content += "`ntarget_include_directories(z80 INTERFACE .)"
                }
                $content | Set-Content $dep_cmake
            }
        }
    }

    Push-Location $build_dir
    Write-Host "Configuring with emcmake..."
    & $script:emcmake_cmd cmake ../wasm -G Ninja "-DCMAKE_MAKE_PROGRAM=$ninja_path" -DCMAKE_BUILD_TYPE=Release -DBUILD_BACKEND=sdl "-DCMAKE_C_FLAGS=-Dstrnicmp=strncasecmp -D_stricmp=strcasecmp"
    if ($LASTEXITCODE -ne 0) { throw "CMake configuration failed." }
    
    Write-Host "Building..."
    $build_log = "build_output.log"
    & cmake --build . > $build_log 2>&1
    if ($LASTEXITCODE -ne 0) { 
        Write-Host "`nBUILD FAILED! Showing last 100 lines of output:`n" -ForegroundColor Red
        Get-Content $build_log | Select-Object -Last 100
        throw "Build failed with exit code $LASTEXITCODE" 
    }
    
    Pop-Location
    Write-Host "WASM build completed successfully!"

    # Copy artifacts to wasm/deploy
    Write-Host "Copying artifacts to wasm/deploy..."
    $deploy_dir = Join-Path $repo_root "wasm/deploy"
    if (!(Test-Path $deploy_dir)) { mkdir $deploy_dir | Out-Null }
    
    $artifacts = @("simcoupe.wasm", "simcoupe.js", "simcoupe.data", "simcoupe.wasm.map")
    foreach ($art in $artifacts) {
        $src = Join-Path $build_dir $art
        if (Test-Path $src) {
            Write-Host "Copying $art..." -ForegroundColor Gray
            Copy-Item $src $deploy_dir -Force
        } elseif ($art -ne "simcoupe.wasm.map") {
            Write-Warning "Required artifact missing: $src"
        }
    }
    Write-Host "Artifacts updated in wasm/deploy`n"

} finally {
    Restore-Files
}
