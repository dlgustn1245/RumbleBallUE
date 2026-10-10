<#
	RumbleBallUE Automation 테스트 실행 스크립트.
	UnrealEditor-Cmd로 테스트를 돌리고 리포트(index.json)를 요약한다.
	종료 코드: 0 전부 통과, 1 실패 있음, 2 리포트 없음(매칭 테스트 없음 또는 크래시)
#>
param(
	[string]$Filter = "RumbleBall",
	[string]$EngineDir = "C:\Program Files\Epic Games\UE_5.8\Engine"
)

$ProjectDir = (Resolve-Path "$PSScriptRoot\..\..\..\..").Path
$Project = Join-Path $ProjectDir "RumbleBallUE.uproject"
$ReportDir = Join-Path $ProjectDir "Saved\Automation\Claude"
$LogFile = Join-Path $ProjectDir "Saved\Logs\ClaudeTests.log"

# 이전 실행 리포트가 남아 있으면 이번 결과로 오인하므로 지운다 (Saved 아래 생성물)
if (Test-Path $ReportDir) { Remove-Item $ReportDir -Recurse -Force }

# ExecCmds 값의 따옴표가 그대로 전달되도록 인자를 한 문자열로 넘긴다
$ArgLine = "`"$Project`" -ExecCmds=`"Automation RunTests $Filter; Quit`" -ReportExportPath=`"$ReportDir`" " +
	"-unattended -nullrhi -nosplash -nosound -nop4 -log=ClaudeTests.log"
$StdOut = Join-Path $env:TEMP "RumbleBallUE-tests-stdout.txt"
$Proc = Start-Process -FilePath "$EngineDir\Binaries\Win64\UnrealEditor-Cmd.exe" -ArgumentList $ArgLine `
	-Wait -PassThru -NoNewWindow -RedirectStandardOutput $StdOut

$Index = Join-Path $ReportDir "index.json"
if (-not (Test-Path $Index)) {
	if ((Test-Path $LogFile) -and (Select-String -Path $LogFile -Pattern "No automation tests matched" -Quiet)) {
		Write-Output "NO TESTS: nothing matched filter '$Filter'."
	} else {
		Write-Output "NO REPORT: editor exited with $($Proc.ExitCode). Check $LogFile"
	}
	exit 2
}

# 리포트는 BOM 포함 UTF-8이라 ReadAllText로 읽어야 한글 메시지가 깨지지 않는다
$Report = [IO.File]::ReadAllText($Index) | ConvertFrom-Json
foreach ($Test in $Report.tests) {
	Write-Output "[$($Test.state)] $($Test.fullTestPath)"
	if ($Test.state -ne "Success") {
		foreach ($Entry in $Test.entries) {
			if ($Entry.event.type -in "Error", "Warning") {
				Write-Output "    $($Entry.event.type): $($Entry.event.message)"
			}
		}
	}
}
Write-Output "Succeeded: $($Report.succeeded), WithWarnings: $($Report.succeededWithWarnings), Failed: $($Report.failed), NotRun: $($Report.notRun)"

if ($Report.failed -gt 0) { exit 1 }
exit 0
