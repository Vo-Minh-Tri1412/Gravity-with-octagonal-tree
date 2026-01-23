# ============================================================================
# SETUP SCRIPT - Tự động cài đặt môi trường và build project
# Chạy: .\setup.ps1
# ============================================================================

Write-Host "=============================================" -ForegroundColor Cyan
Write-Host "  N-Body Gravity Simulation - Setup Script" -ForegroundColor Cyan
Write-Host "=============================================" -ForegroundColor Cyan
Write-Host ""

# Kiểm tra quyền admin (không bắt buộc nhưng khuyến nghị)
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)

# ============================================================================
# 1. Kiểm tra và cài đặt vcpkg
# ============================================================================
Write-Host "[1/4] Kiểm tra vcpkg..." -ForegroundColor Yellow

$vcpkgPaths = @(
    "C:\vcpkg",
    "$env:USERPROFILE\vcpkg",
    "$env:VCPKG_ROOT"
)

$vcpkgRoot = $null
foreach ($path in $vcpkgPaths) {
    if ($path -and (Test-Path "$path\vcpkg.exe")) {
        $vcpkgRoot = $path
        Write-Host "  -> Tìm thấy vcpkg tại: $vcpkgRoot" -ForegroundColor Green
        break
    }
}

if (-not $vcpkgRoot) {
    Write-Host "  -> Không tìm thấy vcpkg. Đang cài đặt..." -ForegroundColor Yellow
    
    $installPath = "C:\vcpkg"
    if (-not $isAdmin) {
        $installPath = "$env:USERPROFILE\vcpkg"
    }
    
    Write-Host "  -> Đang clone vcpkg vào $installPath..." -ForegroundColor Yellow
    git clone https://github.com/microsoft/vcpkg.git $installPath
    
    if ($LASTEXITCODE -ne 0) {
        Write-Host "  -> LỖI: Không thể clone vcpkg. Kiểm tra kết nối mạng và git." -ForegroundColor Red
        exit 1
    }
    
    Write-Host "  -> Đang bootstrap vcpkg..." -ForegroundColor Yellow
    Push-Location $installPath
    .\bootstrap-vcpkg.bat -disableMetrics
    Pop-Location
    
    $vcpkgRoot = $installPath
    
    # Set environment variable cho session hiện tại
    $env:VCPKG_ROOT = $vcpkgRoot
    
    Write-Host "  -> Đã cài đặt vcpkg thành công!" -ForegroundColor Green
    Write-Host ""
    Write-Host "  [KHUYẾN NGHỊ] Thêm VCPKG_ROOT vào Environment Variables:" -ForegroundColor Magenta
    Write-Host "    setx VCPKG_ROOT `"$vcpkgRoot`"" -ForegroundColor White
    Write-Host ""
}

# ============================================================================
# 2. Kiểm tra MinGW
# ============================================================================
Write-Host "[2/4] Kiểm tra MinGW/G++..." -ForegroundColor Yellow

$gpp = Get-Command "g++.exe" -ErrorAction SilentlyContinue
if ($gpp) {
    Write-Host "  -> Tìm thấy G++ tại: $($gpp.Source)" -ForegroundColor Green
} else {
    Write-Host "  -> CẢNH BÁO: Không tìm thấy G++!" -ForegroundColor Red
    Write-Host "  -> Vui lòng cài đặt MinGW-w64 từ: https://winlibs.com/" -ForegroundColor Yellow
    Write-Host "  -> Hoặc dùng MSYS2: https://www.msys2.org/" -ForegroundColor Yellow
    Write-Host ""
    $continue = Read-Host "Tiếp tục anyway? (y/n)"
    if ($continue -ne "y") {
        exit 1
    }
}

# ============================================================================
# 3. Cấu hình CMake
# ============================================================================
Write-Host "[3/4] Cấu hình CMake..." -ForegroundColor Yellow

$projectRoot = $PSScriptRoot
$buildDir = Join-Path $projectRoot "build"

# Xóa build cũ nếu có
if (Test-Path $buildDir) {
    Write-Host "  -> Xóa thư mục build cũ..." -ForegroundColor Yellow
    Remove-Item -Recurse -Force $buildDir
}

# Tạo thư mục build
New-Item -ItemType Directory -Path $buildDir -Force | Out-Null

Push-Location $buildDir

Write-Host "  -> Đang chạy CMake (vcpkg sẽ tự động cài thư viện)..." -ForegroundColor Yellow
Write-Host "  -> Quá trình này có thể mất vài phút lần đầu tiên..." -ForegroundColor Yellow

cmake .. -DCMAKE_TOOLCHAIN_FILE="$vcpkgRoot/scripts/buildsystems/vcpkg.cmake" -DVCPKG_TARGET_TRIPLET=x64-mingw-dynamic

if ($LASTEXITCODE -ne 0) {
    Write-Host "  -> LỖI: CMake configuration thất bại!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Write-Host "  -> CMake cấu hình thành công!" -ForegroundColor Green

# ============================================================================
# 4. Build project
# ============================================================================
Write-Host "[4/4] Build project..." -ForegroundColor Yellow

cmake --build .

if ($LASTEXITCODE -ne 0) {
    Write-Host "  -> LỖI: Build thất bại!" -ForegroundColor Red
    Pop-Location
    exit 1
}

Pop-Location

# ============================================================================
# Hoàn tất
# ============================================================================
Write-Host ""
Write-Host "=============================================" -ForegroundColor Green
Write-Host "  HOÀN TẤT!" -ForegroundColor Green
Write-Host "=============================================" -ForegroundColor Green
Write-Host ""
Write-Host "Chạy chương trình:" -ForegroundColor Cyan
Write-Host "  .\build\UNIVERSE.exe" -ForegroundColor White
Write-Host ""
Write-Host "Hoặc:" -ForegroundColor Cyan
Write-Host "  cd build" -ForegroundColor White
Write-Host "  .\UNIVERSE.exe" -ForegroundColor White
Write-Host ""
