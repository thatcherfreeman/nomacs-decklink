# make_importlib.ps1 — generate a MSVC import library (.lib) from a DLL
# Usage: powershell -File make_importlib.ps1 <dumpbin-output.txt> <dllname> <output.lib>
#
# This is used in CI when the nomacs Windows release does not ship a .lib file.
# If nomacs ships nomacsCore.lib directly, this script is never called.

param(
    [string]$ExportsFile,
    [string]$DllName,
    [string]$OutLib
)

$defFile = [System.IO.Path]::ChangeExtension($OutLib, ".def")
$lines = Get-Content $ExportsFile

# Write a minimal .def file from dumpbin /exports output
$defs = @("EXPORTS")
foreach ($line in $lines) {
    # dumpbin /exports lines look like:  "    1    0 00001234 FunctionName"
    if ($line -match '^\s+\d+\s+\w+\s+\w+\s+(\S+)\s*$') {
        $defs += "    $($Matches[1])"
    }
}
$defs | Set-Content $defFile

# Use lib.exe (comes with MSVC) to create the import library
& lib.exe /def:$defFile /out:$OutLib /machine:x64 /nologo
if ($LASTEXITCODE -ne 0) {
    Write-Error "lib.exe failed with exit code $LASTEXITCODE"
    exit $LASTEXITCODE
}

Write-Host "Created $OutLib"
