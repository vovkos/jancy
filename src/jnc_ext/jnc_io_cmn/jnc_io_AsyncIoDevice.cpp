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
#include "jnc_io_AsyncIoDevice.h"

namespace jnc {
namespace io {

//..............................................................................

void
AsyncIoDevice::open() {
	AsyncIoBase::open();

	m_readBuffer.clear();
	m_readOverflowBuffer.clear();
	m_writeBuffer.clear();
}

void
AsyncIoDevice::close() {
	AsyncIoBase::close();

#if (_JNC_OS_WIN)
	m_freeReadWriteMetaList.insertListTail(&m_readMetaList);
	m_freeReadWriteMetaList.insertListTail(&m_writeMetaList);
#endif
}

bool
AsyncIoDevice::setReadBufferSize(
	size_t* targetField,
	size_t size
) {
	m_lock.lock();

	if (m_readBuffer.getBufferSize() == size) {
		m_lock.unlock();
		return true;
	}

	// the easiest way to ensure buffer consistency on resize is just to drop everything

	m_readMetaList.clear();
	m_readBuffer.clear();
	m_readOverflowBuffer.clear();

	if (m_activeEvents & (AsyncIoDeviceEvent_ReadBufferFull | AsyncIoDeviceEvent_IncomingData)) {
		m_activeEvents &= ~(AsyncIoDeviceEvent_ReadBufferFull | AsyncIoDeviceEvent_IncomingData);
		wakeIoThread();
	}

	bool result = m_readBuffer.setBufferSize(size);
	if (!result) {
		m_lock.unlock();
		return false;
	}

	*targetField = size;
	m_lock.unlock();
	return true;
}

bool
AsyncIoDevice::setWriteBufferSize(
	size_t* targetField,
	size_t size
) {
	m_lock.lock();

	if (m_writeBuffer.getBufferSize() == size) {
		m_lock.unlock();
		return true;
	}

	// the easiest way to ensure buffer consistency on resize is just to drop everything

	m_writeMetaList.clear();
	m_writeBuffer.clear();

	if (!(m_activeEvents & AsyncIoDeviceEvent_WriteBufferEmpty)) // set active events in IO thread
		wakeIoThread();

	bool result = m_writeBuffer.setBufferSize(size);
	if (!result) {
		m_lock.unlock();
		return false;
	}

	*targetField = size;
	m_lock.unlock();
	return true;
}

size_t
AsyncIoDevice::getMetaListDataSize(const sl::ConstList<ReadWriteMeta>& metaList) {
	size_t size = 0;

	sl::ConstIterator<ReadWriteMeta> it = metaList.getHead();
	for (; it; it++)
		size += it->m_dataSize;

	return size;
}

#if (_JNC_DEBUG)

bool
AsyncIoDevice::isReadBufferValid() {
	return
		m_readBuffer.isValid() &&
		(m_readBuffer.isFull() || m_readOverflowBuffer.isEmpty()) &&
		(m_readMetaList.isEmpty() ||
		m_readBuffer.getDataSize() + m_readOverflowBuffer.getCount() == getMetaListDataSize(m_readMetaList));
}

bool
AsyncIoDevice::isWriteBufferValid() {
	return
		m_writeBuffer.isValid() &&
		(m_writeBuffer.isFull() || m_writeOverflowBuffer.isEmpty()) &&
		(m_writeMetaList.isEmpty() ||
		m_writeBuffer.getDataSize() + m_writeOverflowBuffer.getCount() == getMetaListDataSize(m_writeMetaList));
}

#endif

size_t
AsyncIoDevice::bufferedRead(
	DataPtr ptr,
	size_t size,
	sl::Array<char>* params
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	size_t result;
	char* p = (char*)ptr.m_p;

	m_lock.lock();
	if (m_readMetaList.isEmpty()) {
		if (params)
			params->clear();
	} else {
		ReadWriteMeta* meta = *m_readMetaList.getHead();
		if (params)
			params->copy(meta->m_params, meta->m_params.getCount());

		if (size >= meta->m_dataSize) {
			size = meta->m_dataSize;
			m_readMetaList.remove(meta);
			m_freeReadWriteMetaList.insertHead(meta);
		} else if (m_ioThreadFlags & IoThreadFlag_Datagram) {
			m_readMetaList.remove(meta);
			m_freeReadWriteMetaList.insertHead(meta);
		} else {
			meta->m_dataSize -= size;
		}
	}

	result = m_readBuffer.read(p, size);

	if (result && !m_readOverflowBuffer.isEmpty()) {
		size_t overflowSize = m_readOverflowBuffer.getCount();

		if (!m_readBuffer.isEmpty()) { // refill the main buffer first
			size_t movedSize = m_readBuffer.write(m_readOverflowBuffer, overflowSize);
			m_readOverflowBuffer.remove(0, movedSize);
		} else { // we can read some extra data directly from the overflow buffer
			p += result;
			size -= result;

			size_t extraSize = AXL_MIN(overflowSize, size);
			memcpy(p, m_readOverflowBuffer, extraSize);
			result += extraSize;

			// pump the remainder into the main buffer

			size_t movedSize = m_readBuffer.write(m_readOverflowBuffer + extraSize, overflowSize - extraSize);
			m_readOverflowBuffer.remove(0, extraSize + movedSize);
		}
	}

	if (!m_readBuffer.isFull())
		m_activeEvents &= ~AsyncIoDeviceEvent_ReadBufferFull;

	if (m_readBuffer.isEmpty() && m_readMetaList.isEmpty())
		m_activeEvents &= ~AsyncIoDeviceEvent_IncomingData;

	wakeIoThread();

	ASSERT(isReadBufferValid());
	m_lock.unlock();

	return result;
}

size_t
AsyncIoDevice::bufferedWrite(
	const void* p,
	size_t dataSize,
	const void* params,
	size_t paramSize
) {
	if (!m_isOpen) {
		jnc::setError(err::Error(err::SystemErrorCode_InvalidDeviceState));
		return -1;
	}

	m_lock.lock();

	size_t result = addToWriteBuffer(p, dataSize, params, paramSize);
	if (result)
		wakeIoThread();

	m_lock.unlock();
	return result;
}

void
AsyncIoDevice::addToReadBuffer(
	const void* p,
	size_t dataSize,
	const void* params,
	size_t paramSize
) {
	ASSERT((m_options & AsyncIoDeviceOption_KeepReadBlockSize) || paramSize == 0);

	if (!dataSize && !(m_options & AsyncIoDeviceOption_KeepReadBlockSize))
		return;

	size_t addedSize = m_readBuffer.write(p, dataSize);
	if (addedSize < dataSize) {
		size_t overflowSize = dataSize - addedSize;
		m_readOverflowBuffer.append((char*)p + addedSize, overflowSize);
	}

	if (m_options & AsyncIoDeviceOption_KeepReadBlockSize) {
		ReadWriteMeta* meta = createReadWriteMeta(dataSize, params, paramSize);
		m_readMetaList.insertTail(meta);
	}

	ASSERT(isReadBufferValid());
}

size_t
AsyncIoDevice::addToWriteBuffer(
	const void* p,
	size_t dataSize,
	const void* params,
	size_t paramSize
) {
	ASSERT((m_options & AsyncIoDeviceOption_KeepWriteBlockSize) || paramSize == 0);

	if (m_writeBuffer.isFull())
		return 0;

	size_t result = m_writeBuffer.write(p, dataSize);
	if (result < dataSize && (m_ioThreadFlags & IoThreadFlag_Datagram)) {
		m_writeOverflowBuffer.append((char*)p + result, dataSize - result);
		result = dataSize;
	}

	if (m_options & AsyncIoDeviceOption_KeepWriteBlockSize) {
		ReadWriteMeta* meta = createReadWriteMeta(result, params, paramSize);
		m_writeMetaList.insertTail(meta);
	}

	if (!m_writeBuffer.isEmpty() || !m_writeMetaList.isEmpty())
		m_activeEvents &= ~AsyncIoDeviceEvent_WriteBufferEmpty;

	ASSERT(isWriteBufferValid());
	return result;
}

AsyncIoDevice::ReadWriteMeta*
AsyncIoDevice::createReadWriteMeta(
	size_t dataSize,
	const void* params,
	size_t paramSize
) {
	ReadWriteMeta* meta = !m_freeReadWriteMetaList.isEmpty() ?
		m_freeReadWriteMetaList.removeHead() :
		new ReadWriteMeta;

	meta->m_dataSize = dataSize;
	meta->m_params.copy((char*)params, paramSize);
	return meta;
}

void
AsyncIoDevice::getNextWriteBlock(
	sl::Array<char>* data,
	sl::Array<char>* params
) {
	if (!data->isEmpty())
		return;

	if (m_writeMetaList.isEmpty()) {
		m_writeBuffer.readAll(data);

		if (!m_writeOverflowBuffer.isEmpty()) {
			data->append(m_writeOverflowBuffer);
			m_writeOverflowBuffer.clear();
		}

		if (params)
			params->clear();
	} else {
		ReadWriteMeta* meta = m_writeMetaList.removeHead();
		ASSERT(
			meta->m_dataSize <= m_writeBuffer.getDataSize() ||
			meta->m_dataSize == m_writeBuffer.getDataSize() + m_writeOverflowBuffer.getCount()
		);

		data->setCount(meta->m_dataSize);
		char* p = data->p();
		size_t result = m_writeBuffer.read(p, meta->m_dataSize);
		if (result < meta->m_dataSize) {
			size_t overflowSize = meta->m_dataSize - result;
			ASSERT(overflowSize == m_writeOverflowBuffer.getCount());

			memcpy(p + result, m_writeOverflowBuffer, overflowSize);
			m_writeOverflowBuffer.clear();
		}

		if (params)
			params->copy(meta->m_params, meta->m_params.getCount());

		m_freeReadWriteMetaList.insertHead(meta);
	}

	ASSERT(isWriteBufferValid());
}

void
AsyncIoDevice::updateReadWriteBufferEvents() {
	if (m_readBuffer.isFull())
		m_activeEvents |= AsyncIoDeviceEvent_ReadBufferFull;

	if (!m_readBuffer.isEmpty() || !m_readMetaList.isEmpty())
		m_activeEvents |= AsyncIoDeviceEvent_IncomingData;

	if (m_writeBuffer.isEmpty() && m_writeMetaList.isEmpty())
		m_activeEvents |= AsyncIoDeviceEvent_WriteBufferEmpty;
}

//..............................................................................

} // namespace io
} // namespace jnc
