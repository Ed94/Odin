$target_arch        = Join-Path $PSScriptRoot 'helpers/target_arch.psm1'
$devshell           = Join-Path $PSScriptRoot 'helpers/devshell.ps1'
$format_cpp	        = Join-Path $PSScriptRoot 'helpers/format_cpp.psm1'
$incremental_checks = Join-Path $PSScriptRoot 'helpers/incremental_checks.ps1'
$vendor_toolchain   = Join-Path $PSScriptRoot 'helpers/vendor_toolchain.ps1'
$update_deps        = Join-Path $PSScriptRoot 'update_deps.ps1'

$path_root = git rev-parse --show-toplevel

Import-Module $target_arch
Import-Module $format_cpp

#region Arguments
$vendor       = $null
$release      = $null
$verbose      = $false
[bool] $bootstrap    = $false
[bool] $singleheader = $false
[bool] $test         = $false

[array] $vendors = @( "clang", "msvc" )

# This is a really lazy way of parsing the args, could use actual params down the line...

if ( $args ) { $args | ForEach-Object {
	switch ($_){
	{ $_ -in $vendors }   { $vendor       = $_; break }
	"verbose"			  { $verbose      = $true }
	"release"             { $release      = $true }
	"debug"               { $release      = $false }
	}
}}
#endregion Arguments

#region Configuration
if ($IsWindows) {
	# This library was really designed to only run on 64-bit systems.
	# (Its a development tool after all)
    & $devshell -arch amd64
}

if ( $vendor -eq $null ) {
	write-host "No vendor specified, assuming clang available"
	$compiler = "clang"
}

if ( $release -eq $null ) {
	write-host "No build type specified, assuming debug"
	$release = $false
}
elseif ( $release -eq $false ) {
	$debug = $true
}
else {
	$optimize = $true
}
#endregion Configuration

. $vendor_toolchain
. $incremental_checks

#region Building
$path_codegen = join-path $path_root    'codegen'
$path_src     = join-path $path_root    'src'
$path_build   = join-path $path_codegen 'build'

$exe_gensrc  = join-path $path_build 'gen_src.exe'

if (-not(test-path $path_build)) {
	new-item -ItemType Directory -Path $path_build
}

function build-gensrc
{
	write-host "Building gen_src with $vendor"
	write-host "Build Type: $(if ($release) {"Release"} else {"Debug"} )"

	$includes   = @( $path_codegen )
	$unit       = join-path $path_codegen 'gen_src.cpp'

	$compiler_args = @()
	$compiler_args += ($flag_define + 'GEN_TIME')
	$compiler_args += ($flag_cpp_version + 'c++14')

	$linker_args = @()
	$linker_args += $flag_link_win_subsystem_console

	build-simple $path_build $includes $compiler_args $linker_args $unit $exe_gensrc
}
build-gensrc

function run-gensrc
{
	Push-Location $path_root
	if ( Test-Path( $exe_gensrc ) ) {
		write-host "`nRunning gen_src"
		$time_taken = Measure-Command { & $exe_gensrc
				| ForEach-Object {
					write-host `t $_ -ForegroundColor Green
				}
			}
		write-host "`gen_src completed in $($time_taken.TotalMilliseconds) ms"
	}
	Pop-Location
}
run-gensrc
#endregion Building
