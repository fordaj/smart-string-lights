$env:MSYSTEM = $null
$env:MINGW_PREFIX = $null
$env:MSYS = $null
$env:MSYS2_PATH_TYPE = $null

$machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
$userPath = [Environment]::GetEnvironmentVariable("Path", "User")

$env:PATH = "$machinePath;$userPath"

& $env:PIO_EXE @args
exit $LASTEXITCODE