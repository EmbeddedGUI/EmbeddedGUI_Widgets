[CmdletBinding()]
param(
    [Parameter(Position = 0)]
    [ValidateSet("build", "clean", "rebuild", "run")]
    [string]$Action = "build",

    [Parameter(Position = 1)]
    [string]$App = "HelloCustomWidgets",

    [Parameter(Position = 2)]
    [AllowEmptyString()]
    [string]$AppSub = "",

    [Parameter(Position = 3)]
    [AllowEmptyString()]
    [string]$Port = "",

    [Parameter(Position = 4)]
    [AllowEmptyString()]
    [string]$Configuration = "Debug"
)

$ErrorActionPreference = "Stop"

function Convert-ToMakePath {
    param([Parameter(Mandatory = $true)][string]$Path)
    return $Path.Replace("\", "/")
}

function Get-MakeExecutable {
    if ($env:EGUI_MAKE) {
        return $env:EGUI_MAKE
    }

    foreach ($name in @("make", "mingw32-make")) {
        $command = Get-Command $name -ErrorAction SilentlyContinue
        if ($command) {
            return $command.Source
        }
    }

    throw "Cannot find make or mingw32-make. Run setup.bat or set EGUI_MAKE to the make executable."
}

function Get-OutputSuffix {
    param([AllowEmptyString()][string]$Value)
    if ([string]::IsNullOrWhiteSpace($Value)) {
        return "default"
    }

    return ($Value -replace '[\\/:*?"<>|]', "_")
}

function Get-ShortHash {
    param([AllowEmptyString()][string]$Value)

    $sha1 = [System.Security.Cryptography.SHA1]::Create()
    try {
        $bytes = [System.Text.Encoding]::UTF8.GetBytes($Value)
        $hash = $sha1.ComputeHash($bytes)
        return -join ($hash[0..3] | ForEach-Object { $_.ToString("x2") })
    }
    finally {
        $sha1.Dispose()
    }
}

function Get-BuildFlavor {
    param([AllowEmptyString()][string]$Value)

    if ([string]::IsNullOrWhiteSpace($Value)) {
        return "Debug"
    }

    if ($Value -eq "Debug" -or $Value.EndsWith("_Debug")) {
        return "Debug"
    }

    if ($Value -eq "Release" -or $Value.EndsWith("_Release")) {
        return "Release"
    }

    throw "Unknown Visual Studio configuration '$Value'. Expected Debug, Release, *_Debug, or *_Release."
}

function Get-MakeJobs {
    if ($env:EGUI_VS_MAKE_JOBS) {
        $jobs = 0
        if (-not [int]::TryParse($env:EGUI_VS_MAKE_JOBS, [ref]$jobs) -or $jobs -lt 1) {
            throw "EGUI_VS_MAKE_JOBS must be a positive integer."
        }
        return $jobs
    }

    return [Math]::Max(1, [Environment]::ProcessorCount)
}

function Invoke-RootMake {
    param(
        [Parameter(Mandatory = $true)][string]$Target,
        [Parameter(Mandatory = $true)][string[]]$MakeOptions,
        [Parameter(Mandatory = $true)][string[]]$MakeArgs
    )

    Push-Location $RepoRoot
    try {
        Write-Host "make $($MakeOptions -join ' ') $Target $($MakeArgs -join ' ')"
        & $MakeExe @MakeOptions $Target @MakeArgs
        if ($LASTEXITCODE -ne 0) {
            throw "make $Target failed with exit code $LASTEXITCODE"
        }
    }
    finally {
        Pop-Location
    }
}

function Remove-VsObjectRoot {
    param([Parameter(Mandatory = $true)][string]$Path)

    $vsObjRoot = [System.IO.Path]::GetFullPath((Join-Path $RepoRoot "output\vs\o"))
    $fullPath = [System.IO.Path]::GetFullPath($Path)

    if (-not $fullPath.StartsWith($vsObjRoot, [System.StringComparison]::OrdinalIgnoreCase)) {
        throw "Refusing to remove object directory outside output\vs\o: $fullPath"
    }

    if (Test-Path -LiteralPath $fullPath) {
        Remove-Item -LiteralPath $fullPath -Recurse -Force
    }
}

$RepoRoot = (Resolve-Path (Join-Path $PSScriptRoot "..\..")).Path

if ($App -eq "HelloCustomWidgets" -and [string]::IsNullOrWhiteSpace($AppSub)) {
    $AppSub = "showcase"
}

if (-not [string]::IsNullOrWhiteSpace($AppSub)) {
    $AppSub = $AppSub.Replace("\", "/")
}

if ([string]::IsNullOrWhiteSpace($Port)) {
    if ($App -eq "HelloUnitTest") {
        $Port = "pc_test"
    }
    else {
        $Port = "pc"
    }
}

$OutputSuffix = Get-OutputSuffix $AppSub
$OutputPath = Join-Path $RepoRoot (Join-Path "output\vs" (Join-Path $App (Join-Path $OutputSuffix $Configuration)))
$ObjRootPath = Join-Path $RepoRoot (Join-Path "output\vs\o" (Get-ShortHash "$App|$AppSub|$Configuration|$Port"))
New-Item -ItemType Directory -Force -Path $OutputPath | Out-Null
New-Item -ItemType Directory -Force -Path $ObjRootPath | Out-Null

$MakeExe = Get-MakeExecutable
$MakeJobs = Get-MakeJobs
$MakeOptions = @("-j$MakeJobs")
$BuildFlavor = Get-BuildFlavor $Configuration

if ($BuildFlavor -eq "Debug") {
    $CompileOptLevel = "-O0"
    $CompileDebug = "-g"
    $VsCFlags = "-mwindows -DEGUI_CONFIG_PLATFORM_CUSTOM_PRINTF=1 -DEGUI_PC_LOG_TO_DEBUG_OUTPUT=1 -DEGUI_PC_DEFAULT_DEBUG_LOG_LEVEL=3"
}
else {
    $CompileOptLevel = "-O2"
    $CompileDebug = "-g0"
    $VsCFlags = "-mwindows -DEGUI_CONFIG_PLATFORM_CUSTOM_PRINTF=1 -DEGUI_PC_LOG_TO_DEBUG_OUTPUT=1"
}

$MakeArgs = @(
    "APP=$App",
    "PORT=$Port",
    "OUTPUT_PATH=$(Convert-ToMakePath $OutputPath)",
    "OBJROOT_PATH=$(Convert-ToMakePath $ObjRootPath)",
    "COMPILE_OPT_LEVEL=$CompileOptLevel",
    "COMPILE_DEBUG=$CompileDebug",
    "USER_CFLAGS=$VsCFlags"
)

if (-not [string]::IsNullOrWhiteSpace($AppSub)) {
    $MakeArgs += "APP_SUB=$AppSub"
}

switch ($Action) {
    "clean" {
        Invoke-RootMake "clean" $MakeOptions $MakeArgs
        Remove-VsObjectRoot $ObjRootPath
    }
    "build" {
        Invoke-RootMake "all" $MakeOptions $MakeArgs
    }
    "rebuild" {
        Invoke-RootMake "clean" $MakeOptions $MakeArgs
        Remove-VsObjectRoot $ObjRootPath
        Invoke-RootMake "all" $MakeOptions $MakeArgs
    }
    "run" {
        Invoke-RootMake "all" $MakeOptions $MakeArgs
        Push-Location $RepoRoot
        try {
            & python "scripts/run_app.py" "--output-dir" $OutputPath
            if ($LASTEXITCODE -ne 0) {
                throw "run_app.py failed with exit code $LASTEXITCODE"
            }
        }
        finally {
            Pop-Location
        }
    }
}
