#if __clang__
#pragma clang diagnostic ignored "-Wswitch"
#endif

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRING_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#include "gencpp/gen.cpp"
#include "gencpp/gen.builder.cpp"
#include "gencpp/gen.scanner.cpp"
using namespace gen;

#ifdef GEN_SYSTEM_WINDOWS
	#include <process.h>
#endif

#pragma region Directories

// Program assumes its working directory is the src directory
#define path_root ""
#define path_codegen path_root "codegen/"
#define path_src     path_root "src/"

#pragma endregion Directories

inline
CodeBody parse_file( char const* path ) {
	FileContents content = file_read_contents( GlobalAllocator, true, path );
	CodeBody     code    = parse_global_body( StrC { content.size, (char const*)content.data });
	return code;
}

inline
void git_restore_file( char const* path )
{
	#define git_restore "git restore "
	String command = String::make( GlobalAllocator, git_restore );
	command.append( path );
		log_fmt("Running git restore on: %s\n", path);
		system(command);
	#undef  git_restore
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
		log_fmt("\tRunning clang-format on file: %s\n", path);
		system( command );
		log_fmt("\tclang-format finished reformatting.\n");
	#undef cf_cmd
	#undef cf_format_inplace
	#undef cf_style
	#undef cf_verbse
}

struct Odin_AstKind {
	StringCached desc;
	Code         def;
};

Array<Odin_AstKind> get_odin_ast_kinds()
{
	local_persist Array<Odin_AstKind> kinds = Array<Odin_AstKind>::init_reserve(GlobalAllocator, kilobytes(64));
	{
		local_persist s32 done_once = 0;
		if (done_once)
			return kinds;
		++ done_once;
	}

	CodeType t_char_const_ptr = parse_type(code(char const*));

	CodeBody ast_kinds_header = parse_file( path_codegen "ast_kinds.hpp" );
	for ( Code code = ast_kinds_header.begin(); code != ast_kinds_header.end(); ++ code )
	{
		switch (code->Type)
		{
			using namespace ECode;
			case Comment:
			case Preprocess_Pragma:
				// Ignore
			continue;

			case Variable:
			{
				Odin_AstKind entry { {nullptr}, {} };

				CodeVar var = code.cast<CodeVar>();
				if ( ! var->ValueType.is_equal( t_char_const_ptr ) )
				{
					__debugbreak();
					log_failure("Expected all globally defined variables to be char cons* type");
					return kinds;
				}
				if ( ! var->Value || ! var->Value->Content )
				{
					__debugbreak();
					log_failure("Expected all globally defined variable to have a string assigned to it");
					return kinds;
				}

				// Grab the description
				entry.desc = var->Value->Content;
				++ code;

				// Grab the definition
				if ( code->Type != Struct && code->Type != Typedef )
				{
					__debugbreak();
					log_failure("Expected a struct or typedef for the entry definition");
					return kinds;
				}

				entry.def = code;
				kinds.append(entry);
			}
			continue;

			case Struct:
				__debugbreak();
				log_failure("Expected a description definition as char const* first");
				return kinds;
			break;
		}
	}
	return kinds;
}

Array<Code> get_odin_type_kinds()
{
	local_persist Array<Code> types = Array<Code>::init_reserve(GlobalAllocator, kilobytes(64));
	{
		local_persist s32 done_once = 0;
		if (done_once)
			return types;
		++ done_once;
	}

	CodeBody ast_types_header = parse_file( path_codegen "type_kinds.hpp" );
	for ( Code code = ast_types_header.begin(); code != ast_types_header.end(); ++ code ) switch (code->Type)
	{
		case ECode::Comment:
		case ECode::Preprocess_Pragma:
			// Ignore
		continue;

		case ECode::Typedef:
		case ECode::Struct:
		{
			types.append(code);
		}
	}
	return types;
}

int gen_main()
{
	gen::init();
	log_fmt("Generating code for Odin's src\n");

	StrC str_GB_STATIC_ASSERT = txt("GB_STATIC_ASSERT(");
	PreprocessorDefines.append( get_cached_string(str_GB_STATIC_ASSERT) );

	// Remove TOKEN_KINDS usage in tokenizer.cpp
	// Note this doesn't account for an already swapped file. Make sure to discard changes or shut this path off if already generated.
	if (1)
	{
		CSV_Object csv_nodes;
		{
			char  scratch_mem[kilobytes(32)];
			Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );
			file_read_contents( scratch, zero_terminate, path_codegen "token_kinds.csv" );

			csv_parse( &csv_nodes, scratch_mem, GlobalAllocator, false );
		}
		Array<ADT_Node> enum_strs = csv_nodes.nodes[0].nodes;
		Array<ADT_Node> str_strs  = csv_nodes.nodes[1].nodes;

		String enum_entries   = String::make_reserve( GlobalAllocator, kilobytes(32) );
		String to_str_entries = String::make_reserve( GlobalAllocator, kilobytes(32) );

		to_str_entries.append(txt("{"));
		for (uw idx = 0; idx < enum_strs.num(); idx++)
		{
			char const* enum_str     = enum_strs[idx].string;
			StrC        entry_to_str = to_str(str_strs [idx].string);

			enum_entries.append_fmt( "Token_%s,\n", enum_str );
			to_str_entries.append( token_fmt( "str", (StrC)entry_to_str, stringize(
				{ cast(u8 *) "<str>", gb_size_of("<str>") -1 },\n
			)));
		}
		to_str_entries.append(txt("}"));

		char const* path_tokenizer = path_src "tokenizer.cpp";
		git_restore_file( path_tokenizer );
		CodeBody src_tokenizer_cpp = parse_file( path_src "tokenizer.cpp" );
		CodeBody body = def_body( ECode::Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of tokenizer.cpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		for (Code code = src_tokenizer_cpp.begin(); code != src_tokenizer_cpp.end(); ++ code) switch (code->Type)
		{
			case ECode::Preprocess_Define:
				if ( code->Name.starts_with( txt("TOKEN_KINDS"))) {
					// Skip, we don't want it.
					continue;
				}
			continue;

			case ECode::Enum:
			{
				if ( code->Name.starts_with(txt("TokenKind")))
				{
					CodeEnum enum_code = code.cast<CodeEnum>();
					enum_code->Body = untyped_str(enum_entries);
				}
				body.append(code);
			}
			continue;

			case ECode::Variable:
				if ( code->Name.starts_with(txt("token_strings")))
				{
					CodeVar var = code.cast<CodeVar>();
					var->Value = untyped_str(to_str_entries);
				}
				body.append(code);
			continue;

			default:
				body.append(code);
			continue;
		}

		Builder src = Builder::open( path_src "tokenizer.cpp" );
		src.print(body);
		src.write();
		format_file( path_src "tokenizer.cpp" );
	}

	// Remove AST_KINDS macro usage in parser.hpp
	// Note this doesn't account for an already swapped file. Make sure to discard changes or shut this path off if already generated.
	if (1)
	{
		char const* path_parser = path_src "parser.cpp";
		git_restore_file( path_parser );
		CodeBody src_parser_header = parse_file( path_src "parser.hpp" );
		CodeBody body = def_body( ECode::Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of parser.hpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		Array<Odin_AstKind> ast_kinds = get_odin_ast_kinds();

		for (Code code = src_parser_header.begin(); code != src_parser_header.end(); ++ code)
		{
			switch (code->Type)
			{
				case ECode::Preprocess_Define:
					if ( code->Name.starts_with( txt("AST_KINDS"))) {
						// Skip, we don't want it.
						continue;
					}
					if ( code->Name.starts_with( txt("AST_KIND"))) {
						// Skip the next 3 definitions
						++ code;
						++ code;
						continue;
					}
					body.append(code);
				continue;

				case ECode::Untyped:
					if (code->Content.starts_with(txt("AST_KINDS")))
						continue;

					body.append(code);
				continue;

				case ECode::Enum:
				{
					if (code->Name.starts_with( txt("AstKind")))
					{
						// Swap with generated variant
						CodeBody swap_body = def_body( ECode::Enum_Body );
						{
							swap_body.append( code_str(Ast_Invalid,));
							for (Odin_AstKind& kind : ast_kinds)
								swap_body.append( untyped_str( String::fmt_buf(GlobalAllocator, "Ast_%S,", kind.def->Name )));
							swap_body.append( code_str(Ast_COUNT));
						}
						CodeEnum swapped_enum = code.cast<CodeEnum>().duplicate();
						swapped_enum->Body = swap_body;
						body.append(swapped_enum);
					}
					else
						body.append(code);
				}
				continue;

				case ECode::Variable:
				{
					if (code->Name.starts_with(txt("ast_strings")))
					{
						// Swap with generated table
						String generated_table = String::make_reserve(GlobalAllocator, kilobytes(32));
						{
							for (Odin_AstKind& kind : ast_kinds)
								generated_table.append(token_fmt("desc", (StrC)kind.desc, stringize(
									{ cast(u8 *) <desc>, gb_size_of(<desc>) -1 },
								)));
						}
						CodeVar swapped_table = code.cast<CodeVar>().duplicate();
						swapped_table->Value = code_fmt( "kinds", (StrC)generated_table, stringize(
						{
							{cast(u8 *)"invalid node", gb_size_of("invalid node")},\n
							<kinds>
						}));
						body.append(swapped_table);
						body.append(fmt_newline);

						// Right after is where the struct definitions were defined, we'll insert them here
						body.append(def_pragma(txt("region AST_KINDS")));
						body.append(fmt_newline);
						for (Odin_AstKind& kind : ast_kinds)
						{
							Code def = kind.def.duplicate();
							def->Name = get_cached_string( String::fmt_buf(GlobalAllocator, "Ast%S", kind.def->Name));
							body.append( def );
							body.append(fmt_newline);
						}
						body.append(def_pragma(txt("endregion AST_KINDS")));
						continue;
					}
					if (code->Name.starts_with(txt("ast_variant_sizes")))
					{
						// Swap with generated table
						String generated_table = String::make_reserve(GlobalAllocator, kilobytes(32));
						{
							for (Odin_AstKind& kind : ast_kinds)
								generated_table.append(token_fmt( "name", (StrC)kind.def->Name, stringize(
									gb_size_of(Ast<name>),\n
								)));
						}
						CodeVar swapped_table = code.cast<CodeVar>().duplicate();
						swapped_table->Value = code_fmt( "kinds", (StrC)generated_table, stringize(
						{
							0,\n
							<kinds>
						}));
						body.append(swapped_table);
						continue;
					}
					body.append(code);
				}
				continue;

				case ECode::Struct:
				{
					CodeStruct code_struct = code.cast<CodeStruct>();
					if (code->Name.starts_with(txt("Ast")))
						for (Code ast_code : code_struct->Body) switch (ast_code->Type)
						{
							case ECode::Union:
							{
								// Swap out the union's contents with the generated member definitions
								CodeBody body_swap = def_body(ECode::Union_Body);
								for (Odin_AstKind kind : ast_kinds)
									body_swap.append( parse_variable( token_fmt( "name", (StrC)kind.def->Name, stringize(
										Ast<name> <name>;
									))));
								ast_code->Body = rcast(AST*, body_swap.ast);
							}
							break;
							default:
								continue;
						}
					body.append(code);
				}
				continue;

				default:
					body.append(code);
				continue;
			}
		}

		Builder header = Builder::open( path_src "parser.hpp" );
		header.print(body);
		header.write();
		format_file( path_src "parser.hpp" );
	}

	// Remove TYPE_KINDS usage in types.cpp
	// Note this doesn't account for an already swapped file. Make sure to discard changes or shut this path off if already generated.
	if (1)
	{
		char const* path_types = path_src "types.cpp";
		git_restore_file( path_types );
		CodeBody src_types_cpp = parse_file( path_src "types.cpp" );
		CodeBody body = def_body( ECode::Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of types.cpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		Array<Code> type_kinds = get_odin_type_kinds();

		for (Code code = src_types_cpp.begin(); code != src_types_cpp.end(); ++ code) switch (code->Type)
		{
			case ECode::Preprocess_Define:
			{
				if ( code->Name.starts_with( txt("TYPE_KINDS"))) {
					// Skip, we don't want it.
					continue;
				}
				if ( code->Name.starts_with( txt("TYPE_KIND"))) {
					// Skip the next 3 definitions
					++ code;
					++ code;
					continue;
				}
				body.append(code);
			}
			continue;

			case ECode::Enum:
			{
				if ( code->Name.starts_with( txt("TypeKind")))
				{
					CodeBody swap_body = def_body( ECode::Enum_Body);
					{
						swap_body.append( code_str(Type_Invalid, ));
						{
							for (Code type : type_kinds)
									swap_body.append( untyped_str( String::fmt_buf(GlobalAllocator, "Type_%S,", type->Name )));
							swap_body.append( code_str(Type_COUNT));
						}
						CodeEnum swapped_enum = code.cast<CodeEnum>().duplicate();
						swapped_enum->Body = swap_body;
						body.append(swapped_enum);
					}
				}
				else
					body.append(code);
			}
			continue;

			case ECode::Variable:
			{
				if (code->Name.starts_with(txt("type_strings")))
				{
					// Swap with generated table
					String generated_table = String::make_reserve(GlobalAllocator, kilobytes(32));
					{
						for (Code type : type_kinds)
							generated_table.append(token_fmt("type", (StrC)type->Name, stringize(
								{ cast(u8 *) "<type>", gb_size_of("<type>") -1 },
							)));
					}
					CodeVar swapped_table = code.cast<CodeVar>().duplicate();
					swapped_table->Value = code_fmt( "types", (StrC)generated_table, stringize(
					{
						{cast(u8 *)"invalid node", gb_size_of("invalid node")},\n
						<types>
					}));
					body.append(swapped_table);
					body.append(fmt_newline);

					// Right after is where the struct definitions were defined, we'll insert them here
					body.append(def_pragma(txt("region TYPE_KINDS")));
					body.append(fmt_newline);
					for (Code type : type_kinds)
					{
						Code def = type.duplicate();
						def->Name = get_cached_string( String::fmt_buf(GlobalAllocator, "Type%S", type->Name));
						body.append( def );
						body.append(fmt_newline);
					}
					body.append(def_pragma(txt("endregion TYPE_KINDS")));
					continue;
				}
				body.append(code);
			}
			continue;

			case ECode::Struct:
			{
				CodeStruct code_struct = code.cast<CodeStruct>();
				if ( String::are_equal(code->Name, txt("Type")))
					for (Code type_code : code_struct->Body) switch (type_code->Type)
					{
						case ECode::Union:
						{
							// Swap out the union's contents with the generated member definitions
							CodeBody body_swap = def_body(ECode::Union_Body);
							for (Code type : type_kinds)
								body_swap.append( parse_variable( token_fmt( "name", (StrC)type->Name, stringize(
									Type<name> <name>;
								))));
							type_code->Body = rcast(AST*, body_swap.ast);
						}
						break;
						default:
							continue;
					}
				body.append(code);
			}
			continue;

			default:
				body.append(code);
			continue;
		}

		Builder src = Builder::open( path_src "types.cpp" );
		src.print(body);
		src.write();
		format_file( path_src "types.cpp" );
	}

	// gen::deinit();
	return 0;
}
