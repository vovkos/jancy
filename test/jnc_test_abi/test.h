//..............................................................................
//
//  This file is part of the Jancy toolkit.
//
//  Jancy is distributed under the MIT license.
//  For details see accompanying license.txt file,
//  the public copy of which is also available at:
//  http://tibbo.com/downloads/archive/jancy/license.txt
//
//..............................................................................

struct struct32 {
	int32_t m_a;
};

struct struct64 {
	int64_t m_a;
};

struct struct128 {
	int64_t m_a;
	int64_t m_b;
};

typedef
int32_t
FuncInt32(
	int32_t a1,
	int32_t a2,
	int32_t a3,
	int32_t a4,
	int32_t a5,
	int32_t a6,
	int32_t a7,
	int32_t a8
);

typedef
int64_t
FuncInt64(
	int64_t a1,
	int64_t a2,
	int64_t a3,
	int64_t a4,
	int64_t a5,
	int64_t a6,
	int64_t a7,
	int64_t a8
);

typedef
struct32
FuncStruct32(
	struct32 s1,
	struct32 s2,
	struct32 s3,
	struct32 s4,
	struct32 s5,
	struct32 s6,
	struct32 s7,
	struct32 s8
);

typedef
struct64
FuncStruct64(
	struct64 s1,
	struct64 s2,
	struct64 s3,
	struct64 s4,
	struct64 s5,
	struct64 s6,
	struct64 s7,
	struct64 s8
);

typedef
struct128
FuncStruct128(
	struct128 s1,
	struct128 s2,
	struct128 s3,
	struct128 s4
);

typedef
jnc::Variant
FuncVariant(
	jnc::Variant v1,
	jnc::Variant v2,
	jnc::Variant v3,
	jnc::Variant v4
);

typedef
jnc::DataPtr
FuncPtr(
	jnc::DataPtr p1,
	char* p2
);

typedef
float
FuncFloat(
	float x,
	float y
);

typedef
double
FuncDouble(
	double x,
	double y
);

typedef
void
TestFunc();

namespace c2jnc {

//..............................................................................

void
testInt32(jnc::Module* module);

void
testInt64(jnc::Module* module);

void
testStruct32(jnc::Module* module);

void
testStruct64(jnc::Module* module);

void
testStruct128(jnc::Module* module);

void
testVariant(jnc::Module* module);

void
testFloat(jnc::Module* module);

void
testDouble(jnc::Module* module);

//..............................................................................

} // namespace c2jnc

namespace jnc2c {

//..............................................................................

void
test(
	jnc::Module* module,
	const char* funcName
);

//..............................................................................

} // namespace jnc2c

JNC_DECLARE_LIB(TestLib)

