<#
.SYNOPSIS
    Local Docker-based devkitPro build wrapper for Mission-Control (Ryazhenka).

.DESCRIPTION
    Wraps `docker run devkitpro/devkita64 make ...` so you can build on a
    Windows host without installing msys2 / devkitPro pacman. Detects Docker
    and prints msys2 install instructions if it's missing.

.PARAMETER Clean
    Run `make clean` before building.

.PARAMETER Dist
    Build the distribution zip (equivalent to `make dist`). Default is `make`.

.PARAMETER Tag
    Create a local git tag (e.g. v15.1.2) before building, so the version
    string baked into the binary matches.

.PARAMETER Jobs
    Parallel make jobs. Defaults to number of logical CPUs.

.EXAMPLE
    .\scripts\build.ps1 -Dist
    Build the release zip with default parallelism.

.EXAMPLE
    .\scripts\build.ps1 -Clean -Dist -Tag v15.1.2
    Clean, tag locally, build the release zip.
#>
[CmdletBinding()]
param(
    [switch]$Clean,
    [switch]$Dist,
    [string]$Tag = '',
    [int]$Jobs = 0
)

$ErrorActionPreference = 'Stop'

$repoRoot = Resolve-Path "$PSScriptRoot\.."
Set-Location $repoRoot

function Write-Section($msg) {
    Write-Host ''
    Write-Host '============================================================' -ForegroundColor Cyan
    Write-Host " $msg" -ForegroundColor Cyan
    Write-Host '============================================================' -ForegroundColor Cyan
}

function Test-Docker {
    try {
        $null = & docker version --format '{{.Server.Version}}' 2>&1
        return $LASTEXITCODE -eq 0
    } catch {
        return $false
    }
}

function Show-DockerHelp {
    Write-Host ''
    Write-Host 'Docker is required for the containerised build path.' -ForegroundColor Yellow
    Write-Host ''
    Write-Host 'Option A — install Docker Desktop:' -ForegroundColor Yellow
    Write-Host '  https://www.docker.com/products/docker-desktop/'
    Write-Host ''
    Write-Host 'Option B — build natively via msys2 + devkitPro pacman:' -ForegroundColor Yellow
    Write-Host '  See docs/RU/build.md for the full step-by-step.'
    Write-Host ''
    Write-Host 'Once Docker is running, re-run this script.' -ForegroundColor Yellow
}

Write-Section 'Mission-Control (Ryazhenka) — local build'

if (-not (Test-Docker)) {
    Show-DockerHelp
    exit 1
}

if ($Tag) {
    Write-Host "Tagging working tree as $Tag" -ForegroundColor Green
    if (& git rev-parse "refs/tags/$Tag" 2>$null) {
        Write-Host "  Tag $Tag already exists, skipping git tag." -ForegroundColor DarkYellow
    } else {
        & git tag -a $Tag -m "Local build tag $Tag"
        if ($LASTEXITCODE -ne 0) { throw "git tag failed" }
    }
}

if ($Jobs -le 0) { $Jobs = [Environment]::ProcessorCount }

$image = 'devkitpro/devkita64:latest'
Write-Host "Pulling/refreshing $image..." -ForegroundColor Green
& docker pull $image
if ($LASTEXITCODE -ne 0) { throw "docker pull failed" }

# Compose the make command(s)
$makeCmd = @()
if ($Clean) { $makeCmd += 'make clean || true' }
$makeCmd += 'dkp-pacman -S --noconfirm switch-libjpeg-turbo >/dev/null'
$makeCmd += 'git config --global --add safe.directory /project'
if ($Dist) {
    $makeCmd += "make dist -j$Jobs"
} else {
    $makeCmd += "make -j$Jobs"
}
$shellCmd = $makeCmd -join ' && '

Write-Section "Running build (jobs=$Jobs, dist=$Dist, clean=$Clean)"
Write-Host "Command: $shellCmd" -ForegroundColor DarkGray
Write-Host ''

# Convert Windows path to a form Docker on WSL2/Hyper-V will mount cleanly
$mount = ($repoRoot.Path -replace '\\', '/')
$mount = $mount -replace '^([A-Za-z]):', { "/$($args[0].Groups[1].Value.ToLower())" }

& docker run --rm `
    -v "${repoRoot}:/project" `
    -w /project `
    --user "$([System.Environment]::ProcessId):$([System.Environment]::ProcessId)" `
    $image `
    bash -c $shellCmd

if ($LASTEXITCODE -ne 0) {
    Write-Host ''
    Write-Host 'Build failed.' -ForegroundColor Red
    exit $LASTEXITCODE
}

Write-Section 'Build complete'

if ($Dist) {
    $zips = Get-ChildItem "$repoRoot\dist" -Filter '*.zip' -ErrorAction SilentlyContinue
    if (-not $zips) {
        Write-Host 'No zip found in dist/. Check the build output above.' -ForegroundColor Red
        exit 1
    }
    foreach ($z in $zips) {
        $sha = (Get-FileHash $z.FullName -Algorithm SHA256).Hash
        $size = [math]::Round($z.Length / 1KB, 1)
        Write-Host ''
        Write-Host "  $($z.Name)" -ForegroundColor Green
        Write-Host "  Size:   $size KB"
        Write-Host "  SHA256: $sha"
    }
    Write-Host ''
    Write-Host "Artifacts in: $repoRoot\dist" -ForegroundColor Green
}
