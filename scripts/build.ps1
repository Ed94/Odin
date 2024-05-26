
$path_root = git rev-parse --show-toplevel

& "$PSScriptRoot/build_and_run_gen_src_pass"

Push-Location $path_root

& build.bat

Pop-Location
