<#
	RumbleBallUE 빌드 스크립트.
	UBT 출력에서 에러/경고 줄만 추려 보여주고, 전체 로그는 Saved/Logs/ClaudeBuild.log에 남긴다.
	종료 코드: 0 성공, 1 빌드 실패, 2 에디터 실행 중이라 빌드 거부
#>
param(
	[string]$Target = "RumbleBallUEEditor",
	[ValidateSet("DebugGame", "Development", "Shipping")]
	[string]$Config = "Development",
	[string]$EngineDir = "C:\Program Files\Epic Games\UE_5.8\Engine"
)

$ProjectDir = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
$Project = Join-Path $ProjectDir "RumbleBallUE.uproject"
$LogFile = Join-Path $ProjectDir "Saved\Logs\ClaudeBuild.log"

# 에디터 타깃은 에디터(또는 테스트용 UnrealEditor-Cmd)가 DLL을 잡고 있으면 빌드가 거부된다.
if ($Target -like "*Editor" -and (Get-Process "UnrealEditor*" -ErrorAction SilentlyContinue)) {
	Write-Output "BLOCKED: Unreal Editor is running. Close it, or use Live Coding (Ctrl+Alt+F11) for .cpp-only changes."
	exit 2
}

New-Item -ItemType Directory -Force (Split-Path $LogFile) | Out-Null
$Output = & "$EngineDir\Build\BatchFiles\Build.bat" $Target Win64 $Config "-Project=$Project" -WaitMutex
$Code = $LASTEXITCODE
$Output | Set-Content -Path $LogFile -Encoding UTF8

# 컴파일러/링커/UHT 에러와 경고만 중복 없이 출력
$Issues = $Output | Select-String -Pattern '\berror\b|\bwarning\b|Unable to build' | ForEach-Object { $_.Line.Trim() } | Select-Object -Unique
$Issues | Select-Object -First 60

if ($Code -eq 0) {
	Write-Output "BUILD SUCCEEDED ($Target $Config), warnings/errors listed: $(@($Issues).Count)"
	exit 0
}

# 패턴에 걸린 줄이 없으면 원인 파악용으로 로그 끝부분을 보여준다
if (-not $Issues) { $Output | Select-Object -Last 40 }
Write-Output "BUILD FAILED (exit $Code). Full log: $LogFile"
exit 1
