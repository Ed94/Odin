/*
TYPE_KINDS swap content
These are not to be used directly. They are parsed by gen_src.cpp
*/

typedef BasicType Basic;

struct Named {
	String  name;
	Type   *base;
	Entity *type_name; /* Entity_TypeName */
};

struct Generic {
	i64     id;
	String  name;
	Type   *specialized;
	Scope  *scope;
	Entity *entity;
};

struct Pointer {
	Type *elem;
};

struct MultiPointer {
	Type *elem;
};

struct Array {
	Type *elem;
	i64   count;
	Type *generic_count;
};

struct EnumeratedArray {
	Type       *elem;
	Type       *index;
	ExactValue *min_value;
	ExactValue *max_value;
	i64         count;
	TokenKind   op;
	bool        is_sparse;
};

struct Slice {
	Type *elem;
};

struct DynamicArray {
	Type *elem;
};

struct Map {
	Type *key;
	Type *value;
	Type *lookup_result_type;
	Type *debug_metadata_type;
};

typedef TypeStruct Struct;
typedef TypeUnion  Union;

struct Enum {
	Array<Entity *>  fields;
	Ast             *node;
	Scope           *scope;
	Type            *base_type;
	ExactValue      *min_value;
	ExactValue      *max_value;
	isize            min_value_index;
	isize            max_value_index;
};

struct Tuple {
	Slice<Entity *>  variables; /* Entity_Variable */
	i64             *offsets;
	BlockingMutex    mutex; /* for settings offsets */
	bool             are_offsets_being_processed;
	bool             are_offsets_set;
	bool             is_packed;
};

typedef TypeProc Proc;

struct BitSet {
	Type *elem;
	Type *underlying;
	i64   lower;
	i64   upper;
	Ast  *node;
};

struct SimdVector {
	i64   count;
	Type *elem;
	Type *generic_count;
};

struct RelativePointer {
	Type *pointer_type;
	Type *base_integer;
};

struct RelativeMultiPointer {
	Type *pointer_type;
	Type *base_integer;
};

struct Matrix {
	Type *elem;
	i64   row_count;
	i64   column_count;
	Type *generic_row_count;
	Type *generic_column_count;
	i64   stride_in_bytes;
	bool  is_row_major;
};

struct BitField {
	Scope           *scope;
	Type            *backing_type;
	Slice<Entity *>  fields;
	String          *tags; /*count == fields.count*/
	Slice<u8>        bit_sizes;
	Slice<i64>       bit_offsets;
	Ast             *node;
};

struct SoaPointer {
	Type* elem;
};
