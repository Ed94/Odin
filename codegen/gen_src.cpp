#if __clang__
#pragma clang diagnostic ignored "-Wswitch"
#endif

#define GEN_DEFINE_LIBRARY_CODE_CONSTANTS
#define GEN_ENFORCE_STRING_CODE_TYPES
#define GEN_EXPOSE_BACKEND
#define GEN_IMPLEMENTATION
// #define GEN_DONT_USE_FATAL
#include "gencpp/gen.hpp"
using namespace gen;

#ifdef GEN_SYSTEM_WINDOWS
	#include <process.h>
	// #include <Windows.h>
#endif

#pragma region Directories

// Program assumes its working directory is the src directory
#define path_root ""
#define path_codegen path_root "codegen/"
#define path_src     path_root "src/"

#pragma endregion Directories

inline
void git_restore_file( char const* path )
{
	#define git_restore_cmd "git restore "
	StrBuilder command = StrBuilder::make( GlobalAllocator, git_restore_cmd );
	command.append( path );
		log_fmt("Running git restore on: %s", path);
		system(command);
	#undef git_restore_cmd
	Sleep(5);
}

inline
void format_file( char const* path )
{
	// Need to execute clang format on the generated file to get it to match the original.
	#define clang_format      "clang-format "
	#define cf_format_inplace "-i "
	#define cf_style          "-style=file:" "./scripts/.clang-format "
	#define cf_verbose        "-verbose "
	StrBuilder command = StrBuilder::make( GlobalAllocator, clang_format );
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

	CodeTypename t_char_const_ptr = parse_type(code(char const*));

	CodeBody ast_kinds_header = parse_file( path_codegen "ast_kinds.hpp" );
	for ( Code code = ast_kinds_header.begin(); code != ast_kinds_header.end(); ++ code )
	{
		switch (code->Type)
		{
			case CT_Comment:
			case CT_Preprocess_Pragma:
				// Ignore
			continue;

			case CT_Variable:
			{
				Odin_AstKind entry {};

				CodeVar var = cast(CodeVar, code);
				if ( ! var->ValueType.is_equal( t_char_const_ptr ) )
				{
					log_failure("Expected all globally defined variables to be char cons* type");
					return kinds;
				}
				if ( ! var->Value || ! var->Value->Content )
				{
					log_failure("Expected all globally defined variable to have a string assigned to it");
					return kinds;
				}

				// Grab the description
				entry.desc = var->Value->Content;
				++ code;

				// Grab the definition
				if ( code->Type != CT_Struct && code->Type != CT_Typedef )
				{
					log_failure("Expected a struct or typedef for the entry definition");
					return kinds;
				}

				entry.def = code;
				kinds.append(entry);
			}
			continue;

			case CT_Struct:
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
		case CT_Comment:
		case CT_Preprocess_Pragma:
			// Ignore
		continue;

		case CT_Typedef:
		case CT_Struct:
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

	Str str_GB_STATIC_ASSERT = txt("GB_STATIC_ASSERT(");
	PreprocessorDefines.append( get_cached_string(str_GB_STATIC_ASSERT) );

	// Remove TOKEN_KINDS usage in tokenizer.cpp
	// Note this doesn't account for an already swapped file. Make sure to discard changes or shut this path off if already generated.
	if (1)
	{
		CSV_Object csv_nodes;
		{
			char  scratch_mem[kilobytes(32)];
			Arena scratch = Arena::init_from_memory( scratch_mem, sizeof(scratch_mem) );
			file_read_contents( scratch, file_zero_terminate, path_codegen "token_kinds.csv" );

			csv_parse( &csv_nodes, scratch_mem, GlobalAllocator, false );
		}
		Array<ADT_Node> enum_strs = csv_nodes.nodes[0].nodes;
		Array<ADT_Node> str_strs  = csv_nodes.nodes[1].nodes;

		StrBuilder enum_entries   = StrBuilder::make_reserve( GlobalAllocator, kilobytes(32) );
		StrBuilder to_str_entries = StrBuilder::make_reserve( GlobalAllocator, kilobytes(32) );

		to_str_entries.append(txt("{"));
		for (usize idx = 0; idx < enum_strs.num(); idx++)
		{
			char const* enum_str     = enum_strs[idx].string;
			Str         entry_to_str = to_str_from_c_str(str_strs [idx].string);

		#pragma push_macro("cast")
		#undef cast
			enum_entries.append_fmt( "Token_%s,\n", enum_str );
			to_str_entries.append( token_fmt( "str", (Str)entry_to_str, stringize(
				{ cast(u8 *) "<str>", gb_size_of("<str>") -1 },\n
			)));
		#pragma pop_macro("cast")
		}
		to_str_entries.append(txt("}"));

		char const* path_tokenizer = path_src "tokenizer.cpp";
		git_restore_file( path_tokenizer );
		CodeBody src_tokenizer_cpp = parse_file( path_src "tokenizer.cpp" );
		CodeBody body = def_body( CT_Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of tokenizer.cpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		for (Code code = src_tokenizer_cpp.begin(); code != src_tokenizer_cpp.end(); ++ code) switch (code->Type)
		{
			case CT_Preprocess_Define:
				if ( code->Name.starts_with( txt("TOKEN_KINDS"))) {
					// Skip, we don't want it.
					continue;
				}
			continue;

			case CT_Enum:
			{
				if ( code->Name.starts_with(txt("TokenKind")))
				{
					CodeEnum enum_code = cast(CodeEnum, code);
					enum_code->Body = untyped_str(enum_entries);
				}
				body.append(code);
			}
			continue;

			case CT_Variable:
				if ( code->Name.starts_with(txt("token_strings")))
				{
					CodeVar var = cast(CodeVar, code);
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
		char const* path_parser = path_src "parser.hpp";
		git_restore_file( path_parser );
		CodeBody src_parser_header = parse_file( path_src "parser.hpp" );
		CodeBody body = def_body( CT_Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of parser.hpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		Array<Odin_AstKind> ast_kinds = get_odin_ast_kinds();

		for (Code code = src_parser_header.begin(); code != src_parser_header.end(); ++ code)
		{
			switch (code->Type)
			{
				case CT_Preprocess_Define:
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

				case CT_Untyped:
					if (code->Content.starts_with(txt("AST_KINDS")))
						continue;

					body.append(code);
				continue;

				case CT_Enum:
				{
					if (code->Name.starts_with( txt("AstKind")))
					{
						// Swap with generated variant
						CodeBody swap_body = def_body( CT_Enum_Body );
						{
							swap_body.append( code_str(Ast_Invalid,));
							for (Odin_AstKind& kind : ast_kinds)
								swap_body.append( untyped_str( StrBuilder::fmt_buf(GlobalAllocator, "Ast_%S,", kind.def->Name )));
							swap_body.append( code_str(Ast_COUNT));
						}
						CodeEnum swapped_enum = cast(CodeEnum, code).duplicate();
						swapped_enum->Body = swap_body;
						body.append(swapped_enum);
					}
					else
						body.append(code);
				}
				continue;

				case CT_Variable:
				{
					if (code->Name.starts_with(txt("ast_strings")))
					{
						// Swap with generated table
						StrBuilder generated_table = StrBuilder::make_reserve(GlobalAllocator, kilobytes(32));
						{
						#pragma push_macro("cast")
						#undef cast
							for (Odin_AstKind& kind : ast_kinds)
								generated_table.append(token_fmt("desc", (Str)kind.desc, stringize(
									{ cast(u8 *) <desc>, gb_size_of(<desc>) -1 },
								)));
						#pragma pop_macro("cast")
						}

						CodeVar swapped_table = cast(CodeVar, code).duplicate();
					#pragma push_macro("cast")
					#undef cast
						swapped_table->Value = code_fmt( "kinds", (Str)generated_table, stringize(
						{
							{ cast(u8 *)"invalid node", gb_size_of("invalid node")},\n
							<kinds>
						}));
					#pragma pop_macro("cast")

						body.append(swapped_table);
						body.append(fmt_newline);

						// Right after is where the struct definitions were defined, we'll insert them here
						body.append(def_pragma(txt("region AST_KINDS")));
						body.append(fmt_newline);
						for (Odin_AstKind& kind : ast_kinds)
						{
							Code def = kind.def.duplicate();
							def->Name = get_cached_string( StrBuilder::fmt_buf(GlobalAllocator, "Ast%S", kind.def->Name));
							body.append( def );
							body.append(fmt_newline);
						}
						body.append(def_pragma(txt("endregion AST_KINDS")));
						continue;
					}
					if (code->Name.starts_with(txt("ast_variant_sizes")))
					{
						// Swap with generated table
						StrBuilder generated_table = StrBuilder::make_reserve(GlobalAllocator, kilobytes(32));
						{
							for (Odin_AstKind& kind : ast_kinds)
								generated_table.append(token_fmt( "name", (Str)kind.def->Name, stringize(
									gb_size_of(Ast<name>),\n
								)));
						}
						CodeVar swapped_table = cast(CodeVar, code).duplicate();
						swapped_table->Value = code_fmt( "kinds", (Str)generated_table, stringize(
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

				case CT_Struct:
				{
					CodeStruct code_struct = cast(CodeStruct, code);
					if (code->Name.starts_with(txt("Ast")))
						for (Code ast_code : code_struct->Body) switch (ast_code->Type)
						{
							case CT_Union:
							{
								// Swap out the union's contents with the generated member definitions
								CodeBody body_swap = def_body(CT_Union_Body);
								for (Odin_AstKind kind : ast_kinds)
									body_swap.append( parse_variable( token_fmt( "name", (Str)kind.def->Name, stringize(
										Ast<name> <name>;
									))));
								ast_code->Body = body_swap;
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
		CodeBody body = def_body( CT_Global_Body );

		body.append( def_comment(txt("NOTICE(github: Ed94): This is a generated variant of types.cpp using <repo_root>/codegen/gen_src.cpp")));
		body.append(fmt_newline);

		Array<Code> type_kinds = get_odin_type_kinds();

		for (Code code = src_types_cpp.begin(); code != src_types_cpp.end(); ++ code) switch (code->Type)
		{
			case CT_Preprocess_Define:
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

			case CT_Enum:
			{
				if ( code->Name.starts_with( txt("TypeKind")))
				{
					CodeBody swap_body = def_body( CT_Enum_Body);
					{
						swap_body.append( code_str(Type_Invalid, ));
						{
							for (Code type : type_kinds)
								swap_body.append( untyped_str( StrBuilder::fmt_buf(GlobalAllocator, "Type_%S,", type->Name )));
							swap_body.append( code_str(Type_COUNT));
						}
						CodeEnum swapped_enum = cast(CodeEnum, code).duplicate();
						swapped_enum->Body = swap_body;
						body.append(swapped_enum);
					}
				}
				else
					body.append(code);
			}
			continue;

			case CT_Variable:
			{
				if (code->Name.starts_with(txt("type_strings")))
				{
				#pragma push_macro("cast")
				#undef cast
					// Swap with generated table
					StrBuilder generated_table = StrBuilder::make_reserve(GlobalAllocator, kilobytes(32));
					{
						for (Code type : type_kinds)
							generated_table.append(token_fmt("type", (Str)type->Name, stringize(
								{ cast(u8 *) "<type>", gb_size_of("<type>") -1 },
							)));
					}
					CodeVar swapped_table = ((CodeVar)code).duplicate();
					swapped_table->Value = code_fmt( "types", (Str)generated_table, stringize(
					{
						{ cast(u8 *)"invalid node", gb_size_of("invalid node")},\n
						<types>
					}));
				#pragma pop_macro("cast")
					body.append(swapped_table);
					body.append(fmt_newline);

					// Right after is where the struct definitions were defined, we'll insert them here
					body.append(def_pragma(txt("region TYPE_KINDS")));
					body.append(fmt_newline);
					for (Code type : type_kinds)
					{
						Code def = type.duplicate();
						def->Name = get_cached_string( StrBuilder::fmt_buf(GlobalAllocator, "Type%S", type->Name));
						body.append( def );
						body.append(fmt_newline);
					}
					body.append(def_pragma(txt("endregion TYPE_KINDS")));
					continue;
				}
				body.append(code);
			}
			continue;

			case CT_Struct:
			{
				CodeStruct code_struct = cast(CodeStruct, code);
				if ( str_are_equal(code->Name, txt("Type")))
					for (Code type_code : code_struct->Body) switch (type_code->Type)
					{
						case CT_Union:
						{
							// Swap out the union's contents with the generated member definitions
							CodeBody body_swap = def_body(CT_Union_Body);
							for (Code type : type_kinds)
								body_swap.append( parse_variable( token_fmt( "name", (Str)type->Name, stringize(
									Type<name> <name>;
								))));
							type_code->Body = body_swap;
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
