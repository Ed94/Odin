function Get-ScriptRepoRoot {
    $currentPath = $PSScriptRoot
    while ($currentPath -ne $null -and $currentPath -ne "")
	{
        if (Test-Path (Join-Path $currentPath ".git")) {
            return $currentPath
        }
        # Also check for .git file which indicates a submodule
        $gitFile = Join-Path $currentPath ".git"
        if (Test-Path $gitFile -PathType Leaf)
		{
            $gitContent = Get-Content $gitFile
            if ($gitContent -match "gitdir: (.+)") {
                return $currentPath
            }
        }
        $currentPath = Split-Path $currentPath -Parent
    }
    throw "Unable to find repository root"
}

$path_repo_root = Get-ScriptRepoRoot

& "$PSScriptRoot/build_and_run_gen_src_pass"

$compiler_build_script = join-path $path_repo_root 'build.bat'
& $compiler_build_script release

$odin_exe       = Get-ChildItem -Path $path_repo_root -Recurse -Filter "odin.exe" -File | Select-Object -First 1
$odin_sectr_exe = Get-ChildItem -Path $path_repo_root -Recurse -Filter "odin_sectr.exe" -File | Select-Object -First 1

if ($odin_sectr_exe) {
	remove-item $odin_sectr_exe
}

if ($odin_exe) {
    Rename-Item -Path $odin_exe.FullName -NewName "odin_sectr.exe" -Force
    Write-Host "Renamed $($odin_exe.FullName) to odin_sectr.exe"
} else {
    Write-Error "Could not find odin.exe in repository after build."
}
