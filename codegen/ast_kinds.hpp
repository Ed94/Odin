/*
AST_KINDS swap content
These are not to be used directly. They are parsed by gen_src.cpp

Everything within the Ast_Kinds region has a specific format pair:
	1. char const* strings 
	2. struct or typedef definition.

The string descriptions are put into the Ast_strings lookup table.
The actual struct definitions used will have the Ast prefix-namespace appended to them.
*/

char const* desc_Ident = "identifier";
struct Ident {
	Token   token;
	Entity *entity;
};

char const* desc_Implicit = "implicit";
typedef Token Implicit;

char const* desc_Uninit = "uninitialized value";
typedef Token Uninit;

char const* desc_BasicLit = "basic literal";
struct BasicLit {
	Token token;
};

char const* desc_BasicDirective = "basic directive";
struct BasicDirective {
	Token token;
	Token name;
};

char const* desc_Ellipsis = "ellipsis";
struct Ellipsis {
	Token  token;
	Ast   *expr;
};

char const* desc_ProcGroup = "procedure group";
struct ProcGroup {
	Token        token;
	Token        open;
	Token        close;
	Slice<Ast *> args;
};

char const* desc_ProcLit = "procedure literal";
struct ProcLit {
	Ast          *type;
	Ast          *body;
	u64           tags;
	ProcInlining  inlining;
	Token         where_token;
	Slice<Ast *>  where_clauses;
	DeclInfo     *decl;
};

char const* desc_CompountLit = "compound literal";
struct CompoundLit {
	Ast          *type;
	Slice<Ast *>  elems;
	Token open,   close;
	i64           max_count;
	Ast          *tag;
};

#pragma region Expressions
char const* desc_ExprBegin = "";
typedef bool _ExprBegin;

char const* desc_BadExpr = "bad expression";
struct BadExpr {
	Token begin, end;
};

char const* desc_TagExpr = "tag expression";
struct TagExpr {
	Token  token, name;
	Ast   *expr;
};

char const* desc_UnaryExpr = "unary expression";
struct UnaryExpr {
	Token op;
	Ast *expr;
};

char const* desc_BinaryExpr = "binary expression";
struct BinaryExpr {
	Token  op;
	Ast   *left, *right;
};

char const* desc_ParenExpr = "parentheses expression";
struct ParenExpr {
	Ast   *expr;
	Token  open, close;
};

char const* desc_SelectorExpr = "selector expression";
struct SelectorExpr {
	Token  token;
	Ast   *expr, *selector;
	u8     swizzle_count; /*maximum of 4 components, if set, count >= 2*/
	u8     swizzle_indices; /*2 bits per component*/
	bool   is_bit_field;
};

char const* desc_ImplicitSelectorExpr = "implicit selector expression";
struct ImplicitSelectorExpr {
	Token token;
	Ast *selector;
};

char const* desc_SelectorCallExpr = "selector call expression";
struct SelectorCallExpr {
	Token token;
	Ast *expr, *call;
	bool modified_call;
};

char const* desc_IndentExpr = "index expression";
struct IndexExpr {
	Ast   *expr, *index;
	Token  open, close;
};

char const* desc_DerefExpr = "dereference expression";
struct DerefExpr {
	Ast   *expr;
	Token  op;
};

char const* desc_SliceExpr = "slice expression";
struct SliceExpr {
	Ast   *expr;
	Token  open, close;
	Token  interval;
	Ast   *low, *high;
};

char const* desc_CallExpr = "call expression";
struct CallExpr {
	Ast          *proc;
	Slice<Ast *>  args;
	Token         open;
	Token         close;
	Token         ellipsis;
	ProcInlining  inlining;
	bool          optional_ok_one;
	bool          was_selector;
	AstSplitArgs *split_args;
	Entity       *entity_procedure_of;
};

char const* desc_FieldValue = "field value";
struct FieldValue {
	Token  eq;
	Ast   *field, *value;
};

char const* desc_EnumFieldValue = "enum field value";
struct EnumFieldValue {
	Ast          *name;
	Ast          *value;
	CommentGroup *docs;
	CommentGroup *comment;
};

char const* desc_TernaryIfExpr = "ternary if expression";
struct TernaryIfExpr {
	Ast *x, *cond, *y;
};

char const* desc_TernaryWhenExpr = "ternary when expression";
struct TernaryWhenExpr {
	Ast *x, *cond, *y;
};

char const* desc_OrElseExpr = "or_else expression";
struct OrElseExpr {
	Ast   *x;
	Token  token;
	Ast   *y;
};

char const* desc_OrReturnExpr = "or_return expression";
struct OrReturnExpr {
	Ast   *expr;
	Token  token;
};

char const* desc_OrBranchExpr = "or branch expression";
struct OrBranchExpr {
	Ast   *expr;
	Token  token;
	Ast   *label;
};

char const* desc_TypeAssertion = "type assertion";
struct TypeAssertion {
	Ast   *expr;
	Token  dot;
	Ast   *type;
	Type  *type_hint;
	bool   ignores[2];
};

char const* desc_TypeCAst = "type cAst";
struct TypeCast {
	Token  token;
	Ast   *type, *expr;
};

char const* desc_AutoCAst = "auto_cAst";
struct AutoCast {
	Token  token;
	Ast   *expr;
};

char const* desc_InlineAsmExpr = "inline asm expression";
struct InlineAsmExpr {
	Token                token;
	Token                open, close;
	Slice<Ast *>         param_types;
	Ast                 *return_type;
	Ast                 *asm_string;
	Ast                 *constraints_string;
	bool                 has_side_effects;
	bool                 is_align_stack;
	InlineAsmDialectKind dialect;
};

char const* desc_MatrixIndexExpr = "matrix index expression";
struct MatrixIndexExpr {
	Ast   *expr, *row_index, *column_index;
	Token  open, close;
};

char const* desc__ExprEnd = "";
typedef bool _ExprEnd;
#pragma endregion Expressions

#pragma region Statements
char const* desc__StmtBegin = "";
typedef bool _StmtBegin;

char const* desc_BadStmt = "bad statement";
struct BadStmt {
	Token begin, end;
};

char const* desc_EmptyStmt = "empty statement";
struct EmptyStmt {
	Token token;
};

char const* desc_ExprStmt = "expression statement";
struct ExprStmt {
	Ast* expr;
};

char const* desc_AssignStmt = "assign statement";
struct AssignStmt {
	Token        op;
	Slice<Ast *> lhs, rhs;
};

#pragma region Complex Statements
char const* desc__ComplexStmtBegin = "";
typedef bool _ComplexStmtBegin;

char const* desc_BlockStmt = "block statement";
struct BlockStmt {
	Scope        *scope;
	Slice<Ast *>  stmts;
	Ast          *label;
	Token         open, close;
};

char const* desc_IfStmt = "if statement";
struct IfStmt {
	Scope *scope;
	Token  token;
	Ast   *label;
	Ast   *init;
	Ast   *cond;
	Ast   *body;
	Ast   *else_stmt;
};

char const* desc_WhenStmt = "when statement";
struct WhenStmt {
	Token  token;
	Ast   *cond;
	Ast   *body;
	Ast   *else_stmt;
	bool   is_cond_determined;
	bool   determined_cond;
};

char const* desc_ReturnStmt = "return statement";
struct ReturnStmt {
	Token        token;
	Slice<Ast *> results;
};

char const* desc_ForStmt = "for statement";
struct ForStmt {
	Scope *scope;
	Token  token;
	Ast   *label;
	Ast   *init;
	Ast   *cond;
	Ast   *post;
	Ast   *body;
};

char const* desc_RangeStmt = "range statement";
struct RangeStmt {
	Scope        *scope;
	Token         token;
	Ast          *label;
	Slice<Ast *>  vals;
	Token         in_token;
	Ast          *expr;
	Ast          *body;
	bool          reverse;
};

char const* desc_UnrollRangeStmt = "unroll range statement";
struct UnrollRangeStmt {
	Scope        *scope;
	Token         unroll_token;
	Slice<Ast *>  args;
	Token         for_token;
	Ast          *val0;
	Ast          *val1;
	Token         in_token;
	Ast          *expr;
	Ast          *body;
};

char const* desc_CaseClause = "case clause";
struct CaseClause {
    Scope        *scope;
	Token         token;
	Slice<Ast *>  list;
	Slice<Ast *>  stmts;
	Entity       *implicit_entity;
};

char const* desc_SwitchStmt = "switch statement";
struct SwitchStmt {
	Scope *scope;
	Token  token;
	Ast   *label;
	Ast   *init;
	Ast   *tag;
	Ast   *body;
	bool   partial;
};

char const* desc_TypeSwitchStmt = "type switch statement";
struct TypeSwitchStmt {
	Scope *scope;
	Token  token;
	Ast   *label;
	Ast   *tag;
	Ast   *body;
	bool   partial;
};

char const* desc_DeferStmt = "defer statement";
struct DeferStmt {
	Token token;
	Ast*  stmt;
};

char const* desc_BranchStmt = "branch statement";
struct BranchStmt {
	Token token;
	Ast*  label;
};

char const* desc_UsingStmt = "using statement";
struct UsingStmt {
	Token        token;
	Slice<Ast *> list;
};

char const* desc__ComplexStmtEnd = "";
typedef bool _ComplexStmtEnd;
#pragma endregion Complex Statements

char const* desc__StmtEnd = "";
typedef bool _StmtEnd;
#pragma endregion Statements

#pragma region Declarations
char const* desc__DeclBegin = "";
typedef bool _DeclBegin;

char const* desc_BadDecl = "bad declaration";
struct BadDecl {
	Token begin, end;
};

char const* desc_ForeignBlockDecl = "foreign block declaration";
struct ForeignBlockDecl {
	Token         token;
	Ast          *foreign_library;
	Ast          *body;
	Array<Ast *>  attributes;
	CommentGroup *docs;
};

char const* desc_Label = "label";
struct Label {
	Token  token;
	Ast   *name;
};

char const* desc_ValueDecl = "value declaration";
struct ValueDecl {
	Slice<Ast *>  names;
	Ast          *type;
	Slice<Ast *>  values;
	Array<Ast *>  attributes;
	CommentGroup *docs;
	CommentGroup *comment;
	bool          is_using;
	bool          is_mutable;
};

char const* desc_PackageDecl = "package declaration";
struct PackageDecl {
	Token         token;
	Token         name;
	CommentGroup *docs;
	CommentGroup *comment;
};

char const* desc_ImportDecl = "import declaration";
struct ImportDecl {
	AstPackage   *package;
	Token         token;
	Token         relpath;
	String        fullpath;
	Token         import_name;
	Array<Ast *>  attributes;
	CommentGroup *docs;
	CommentGroup *comment;
};

char const* desc_ForeignImportDecl = "foreign import declaration";
struct ForeignImportDecl {
	Token          token;
	Slice<Ast *>   filepaths;
	bool           multiple_filepaths;
	Token          library_name;
	String         collection_name;
	Slice<String>  fullpaths;
	Array<Ast *>   attributes;
	CommentGroup  *docs;
	CommentGroup  *comment;
};

char const* desc__DeclEnd = "";
typedef bool _DeclEnd;
#pragma endregion Declarations

char const* desc_Attribute = "attribute";
struct Attribute {
	Token        token;
	Slice<Ast *> elems;
	Token        open, close;
};

char const* desc_Field = "field";
struct Field {
	Slice<Ast *>  names;
	Ast          *type;
	Ast          *default_value;
	Token         tag;
	u32           flags;
	CommentGroup *docs;
	CommentGroup *comment;
};

char const* desc_BitFieldField = "bit field field";
struct BitFieldField {
	Ast          *name;
	Ast          *type;
	Ast          *bit_size;
	Token         tag;
	CommentGroup *docs;
	CommentGroup *comment;
};

char const* desc_FieldList = "field list";
struct FieldList {
	Token        token;
	Slice<Ast *> list;
};

#pragma region Types
char const* desc__TypeBegin = "";
typedef bool _TypeBegin;

char const* desc_TypeidType = "typeid";
struct TypeidType
{
	Token  token;
	Ast   *specialization;
};

char const* desc_HelperType = "helper type";
struct HelperType {
	Token token;
	Ast   *type;
};

char const* desc_DistinctType = "distinct type";
struct DistinctType {
	Token  token;
	Ast   *type;
};

char const* desc_PolyType = "polymorphic type";
struct PolyType {
	Token  token;
	Ast   *type;
	Ast   *specialization;
};

char const* desc_ProcType = "procedure type";
struct ProcType {
	Scope                 *scope;
	Token                  token;
	Ast                   *params;
	Ast                   *results;
	u64                    tags;
	ProcCallingConvention  calling_convention;
	bool                   generic;
	bool                   diverging;
};

char const* desc_PointerType = "pointer type";
struct PointerType {
	Token  token;
	Ast   *type;
	Ast   *tag;
};

char const* desc_RelativeType = "relative type";
struct RelativeType {
	Ast *tag;
	Ast *type;
};

char const* desc_MultiPointerType = "multi pointer type";
struct MultiPointerType {
	Token  token;
	Ast   *type;
};

char const* desc_ArrayType = "array type";
struct ArrayType {
	Token  token;
	Ast   *count;
	Ast   *elem;
	Ast   *tag;
};

char const* desc_DynamicType = "dynamic array type";
struct DynamicArrayType {
	Token  token;
	Ast   *elem;
	Ast   *tag;
};

char const* desc_StructType = "struct type";
struct StructType {
	Scope        *scope;
	Token         token;
	Slice<Ast *>  fields;
	isize         field_count;
	Ast          *polymorphic_params;
	Ast          *align;
	Ast          *min_field_align;
	Ast          *max_field_align;
	Token         where_token;
	Slice<Ast *>  where_clauses;
	bool          is_packed;
	bool          is_raw_union;
	bool          is_no_copy;
};

char const* desc_UnionType = "union type";
struct UnionType {
	Scope         *scope;
	Token          token;
	Slice<Ast *>   variants;
	Ast           *polymorphic_params;
	Ast           *align;
	UnionTypeKind  kind;
	Token          where_token;
	Slice<Ast *>   where_clauses;
};

char const* desc_EnumType = "enum type";
struct EnumType {
	Scope        *scope;
	Token         token;
	Ast          *base_type;
	Slice<Ast *>  fields; /* FieldValue */
	bool          is_using;
};

char const* dec_BitSetType = "bitset type";
struct BitSetType {
	Token  token;
	Ast   *elem;
	Ast   *underlying;
};

char const* desc_BitfieldType = "bitfield type";
struct BitFieldType {
	Scope        *scope;
	Token         token;
	Ast          *backing_type;
	Token         open;
	Slice<Ast *>  fields; /* BitFieldField */
	Token         close;
};

char const* desc_MapType = "map type";
struct MapType {
	Token  token;
	Ast   *count;
	Ast   *key;
	Ast   *value;
};

char const* desc_MatrixType = "matrix type";
struct MatrixType {
	Token  token;
	Ast   *row_count;
	Ast   *column_count;
	Ast   *elem;
	bool   is_row_major;
};

char const* desc__TypeEnd = "";
typedef bool _TypeEnd;
#pragma endregion Types
