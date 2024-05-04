#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRING_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gencpp/gen.cpp"
#include "gencpp/gen.builder.cpp"
using namespace gen;

#ifdef GEN_SYSTEM_WINDOWS
	#include <process.h>
#endif

#pragma region Directories
// Program assumes its working directory is the src directory
#define path_src ""
#pragma endregion Directories

inline
CodeBody parse_file( char const* path ) {
	FileContents content = file_read_contents( GlobalAllocator, true, path );
	CodeBody     code    = parse_global_body( StrC { content.size, (char const*)content.data });
	return code;
}

inline
void format_file( char const* path )
{
	// Need to execute clang format on the generated file to get it to match the original.
	#define clang_format      "clang-format "
	#define cf_format_inplace "-i "
	#define cf_style          "-style=file:" "./scripts/.clang-format "
	#define cf_verbose        "-verbose "
	String command = String::make( GlobalAllocator, clang_format );
	command.append( cf_format_inplace );
	command.append( cf_style );
	command.append( cf_verbose );
	command.append( path );
		log_fmt("\tRunning clang-format on file:\n");
		system( command );
		log_fmt("\tclang-format finished reformatting.\n");
	#undef cf_cmd
	#undef cf_format_inplace
	#undef cf_style
	#undef cf_verbse
}

int gen_main()
{
	gen::init();
	log_fmt("Generating code for Odin's src\n");



	// gen::deinit();
	return 0;
}
