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

#pragma once

#include "jnc_io_AsyncIoBase.h"

namespace jnc {
namespace io {

//..............................................................................

enum AsyncIoDeviceEvent
{
	AsyncIoDeviceEvent_IncomingData     = 0x0002,
	AsyncIoDeviceEvent_ReadBufferFull   = 0x0004,
	AsyncIoDeviceEvent_WriteBufferEmpty = 0x0008,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum AsyncIoDeviceOption
{
	AsyncIoDeviceOption_KeepReadBlockSize      = 0x01,
	AsyncIoDeviceOption_KeepWriteBlockSize     = 0x02,
	AsyncIoDeviceOption_KeepReadWriteBlockSize = 0x03,
};

//..............................................................................

class AsyncIoDevice: public AsyncIoBase
{
protected:
	enum IoThreadFlag
	{
		IoThreadFlag_Datagram = 0x0004,
	};

	struct ReadWriteMeta: sl::ListLink
	{
		size_t m_dataSize;
		sl::Array<char> m_params;
	};

protected:
	sl::CircularBuffer m_readBuffer;
	sl::CircularBuffer m_writeBuffer;
	sl::Array<char> m_readOverflowBuffer;
	sl::Array<char> m_writeOverflowBuffer; // used only for datagrams

	sl::List<ReadWriteMeta> m_readMetaList;
	sl::List<ReadWriteMeta> m_writeMetaList;
	sl::List<ReadWriteMeta> m_freeReadWriteMetaList;

protected:
	void
	open();

	void
	close();

	bool
	setReadBufferSize(
		size_t* targetField,
		size_t size
		);

	bool
	setWriteBufferSize(
		size_t* targetField,
		size_t size
		);

	static
	size_t
	getMetaListDataSize(const sl::ConstList<ReadWriteMeta>& metaList);

	bool
	isReadBufferValid();

	bool
	isWriteBufferValid();

	size_t
	bufferedRead(
		DataPtr ptr,
		size_t size,
		sl::Array<char>* params = NULL
		);

	size_t
	bufferedWrite(
		DataPtr dataPtr,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		)
	{
		return bufferedWrite(dataPtr.m_p, dataSize, params, paramSize);
	}

	size_t
	bufferedWrite(
		const void* p,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		);

	void
	addToReadBuffer(
		const void* p,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		);

	size_t
	addToWriteBuffer(
		const void* p,
		size_t dataSize,
		const void* params = NULL,
		size_t paramSize = 0
		);

	void
	getNextWriteBlock(
		sl::Array<char>* data,
		sl::Array<char>* params = NULL
		);

	void
	updateReadWriteBufferEvents();

	ReadWriteMeta*
	createReadWriteMeta(
		size_t dataSize,
		const void* params,
		size_t paramSize
		);
};

//..............................................................................

#if (_AXL_OS_WIN)

struct OverlappedRead: sl::ListLink
{
	axl::io::win::StdOverlapped m_overlapped;
	sl::Array<char> m_buffer;
};

#endif

//..............................................................................

} // namespace io
} // namespace jnc
