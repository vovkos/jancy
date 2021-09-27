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

#include "pch.h"
#include "jnc_sys_Registry.h"
#include "jnc_sys_SysLib.h"

namespace jnc {
namespace sys {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	RegKey,
	"sys.RegKey",
	g_sysLibGuid,
	SysLibCacheSlot_RegKey,
	RegKey,
	NULL
)

JNC_BEGIN_TYPE_FUNCTION_MAP(RegKey)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<RegKey>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<RegKey>)
	JNC_MAP_FUNCTION("close",       &RegKey::close)
	JNC_MAP_FUNCTION("create",      &RegKey::create0)
	JNC_MAP_OVERLOAD(&RegKey::create1)
	JNC_MAP_FUNCTION("open",        &RegKey::open0)
	JNC_MAP_OVERLOAD(&RegKey::open1)
	JNC_MAP_FUNCTION("read",        &RegKey::read)
	JNC_MAP_FUNCTION("readDword",   &RegKey::readDword)
	JNC_MAP_FUNCTION("readString",  &RegKey::readString)
	JNC_MAP_FUNCTION("write",       &RegKey::write)
	JNC_MAP_FUNCTION("writeString", &RegKey::writeString)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

void
JNC_CDECL
RegKey::close() {
	if (!m_key)
		return;

	::RegCloseKey(m_key);
	m_key = NULL;
}

DataPtr
JNC_CDECL
RegKey::read(
	RegKey* self,
	DataPtr namePtr,
	DataPtr typePtr
) {
	char buffer[256];
	sl::Array<byte_t> value(rc::BufKind_Stack, buffer, sizeof(buffer));

	dword_t type;
	size_t size = self->readImpl(&value, (char*)namePtr.m_p, &type);
	if (size == -1)
		return g_nullDataPtr;

	return memDup(namePtr.m_p, size);
}

uint32_t
JNC_CDECL
RegKey::readDword(DataPtr namePtr) {
	char buffer[256];
	sl::Array<byte_t> value(rc::BufKind_Stack, buffer, sizeof(buffer));

	dword_t type;
	size_t size = readImpl(&value, (char*)namePtr.m_p, &type);
	if (size == -1)
		return 0;

	sl::String string;

	switch (type) {
	case REG_DWORD:
	case REG_QWORD:
		return *(uint32_t*)value.cp();

	case REG_DWORD_BIG_ENDIAN:
		return sl::swapByteOrder32(*(uint32_t*)value.cp());

	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
		string.copy((wchar_t*)value.cp(), size / sizeof(wchar_t));
		return atoi(string);

	default:
		return size >= sizeof(uint32_t) ? *(uint32_t*)value.cp() : 0;
	}
}

DataPtr
JNC_CDECL
RegKey::readString(
	RegKey* self,
	DataPtr namePtr
) {
	char buffer[256];
	sl::Array<byte_t> value(rc::BufKind_Stack, buffer, sizeof(buffer));

	dword_t type;
	size_t size = self->readImpl(&value, (char*)namePtr.m_p, &type);
	if (size == -1)
		return g_nullDataPtr;

	uint32_t dword;
	sl::String string;

	switch (type) {
	case REG_DWORD:
	case REG_QWORD:
		dword = *(uint32_t*)value.cp();
		string.format("%u", dword);
		break;

	case REG_DWORD_BIG_ENDIAN:
		dword = sl::swapByteOrder32(*(uint32_t*)value.cp());
		string.format("%u", dword);
		break;

	case REG_SZ:
	case REG_EXPAND_SZ:
	case REG_MULTI_SZ:
		string.copy((wchar_t*)value.cp(), size / sizeof(wchar_t));
		break;

	default:
		value.append(0); // just in case
		return strDup((char*)value.cp());
	}

	return strDup(string);
}

bool
JNC_CDECL
RegKey::writeString(
	DataPtr namePtr,
	DataPtr valuePtr
) {
	sl::String_w string((char*)valuePtr.m_p);
	return writeImpl((char*)namePtr.m_p, REG_SZ, string.sz(), (string.getLength() + 1) * sizeof(wchar_t));
}

bool
RegKey::createImpl(
	HKEY parent,
	const char* path
) {
	close();

	long result = ::RegCreateKeyExW(
		parent,
		sl::String_w(path),
		0,
		NULL,
		0,
		KEY_ALL_ACCESS,
		NULL,
		&m_key,
		NULL
	);

	return err::complete(result == ERROR_SUCCESS);
}

bool
RegKey::openImpl(
	HKEY parent,
	const char* path,
	REGSAM access
) {
	close();

	long result = ::RegOpenKeyExW(
		parent,
		sl::String_w(path),
		0,
		access,
		&m_key
	);

	return err::complete(result == ERROR_SUCCESS);
}

size_t
RegKey::readImpl(
	sl::Array<byte_t>* buffer,
	const char* name,
	dword_t* type
) {
	dword_t size = 0;

	long result = ::RegQueryValueExW(
		m_key,
		sl::String_w(name),
		NULL,
		type,
		NULL,
		&size
	);

	if (result != ERROR_SUCCESS) {
		err::setError(result);
		return -1;
	}

	result = buffer->setCount(size);
	if (!result)
		return -1;

	result = ::RegQueryValueExW(
		m_key,
		sl::String_w(name),
		NULL,
		type,
		buffer->p(),
		&size
	);

	if (result != ERROR_SUCCESS) {
		err::setError(result);
		return -1;
	}

	return buffer->getCount();
}

bool
RegKey::writeImpl(
	const char* name,
	dword_t type,
	const void* p,
	size_t size
) {
	long result = ::RegSetValueExW(
		m_key,
		sl::String_w(name),
		0,
		type,
		(const byte_t*)p,
		size
	);

	return err::complete(result == ERROR_SUCCESS);
}

//..............................................................................

} // namespace sys
} // namespace jnc
