#include "pch.h"
#include "jnc_io_HidDevice.h"
#include "jnc_io_HidLib.h"
#include "jnc_Error.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	HidDevice,
	"io.HidDevice",
	g_hidLibGuid,
	HidLibCacheSlot_HidDevice,
	HidDevice,
	&HidDevice::markOpaqueGcRoots
)

JNC_BEGIN_TYPE_FUNCTION_MAP(HidDevice)
	JNC_MAP_CONSTRUCTOR(&sl::construct<HidDevice>)
	JNC_MAP_DESTRUCTOR(&sl::destruct<HidDevice>)

	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",  &HidDevice::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize", &HidDevice::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",        &HidDevice::setOptions)

	JNC_MAP_FUNCTION("open",                &HidDevice::open_0)
	JNC_MAP_OVERLOAD(&HidDevice::open_1)
	JNC_MAP_FUNCTION("close",               &HidDevice::close)
	JNC_MAP_FUNCTION("getReportDescriptor", &HidDevice::getReportDescriptor)
	JNC_MAP_FUNCTION("read",                &HidDevice::read)
	JNC_MAP_FUNCTION("write",               &HidDevice::write)
	JNC_MAP_FUNCTION("getInputReport",      &HidDevice::getInputReport)
	JNC_MAP_FUNCTION("getFeatureReport",    &HidDevice::getFeatureReport)
	JNC_MAP_FUNCTION("sendFeatureReport",   &HidDevice::sendFeatureReport)
	JNC_MAP_FUNCTION("wait",                &HidDevice::wait)
	JNC_MAP_FUNCTION("cancelWait",          &HidDevice::cancelWait)
	JNC_MAP_FUNCTION("blockingWait",        &HidDevice::blockingWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

HidDevice::HidDevice() {
	m_readBlockSize = Def_ReadBlockSize;
	m_readBufferSize = Def_ReadBufferSize;
	m_options = AsyncIoDeviceOption_KeepReadBlockSize;

	m_readBuffer.setBufferSize(Def_ReadBufferSize);
}

bool
JNC_CDECL
HidDevice::setReadTimeout(uint_t timeout) {
	m_lock.lock();
	m_readTimeout = timeout ? timeout : Def_ReadTimeout;
	m_lock.unlock();
	return true;
}

bool
JNC_CDECL
HidDevice::setOptions(uint_t options) {
	if (m_isOpen)
		return err::fail(err::SystemErrorCode_InvalidDeviceState);

	if (((m_options ^ options) & HidDeviceOption_NonBlocking) && m_device.isOpen())	{
		bool result = m_device.setNonBlocking((options & HidDeviceOption_NonBlocking) != 0);
		if (!result)
			return false;
	}

	m_options = options | AsyncIoDeviceOption_KeepReadBlockSize;
	return true;
}

bool
JNC_CDECL
HidDevice::open_0(DataPtr pathPtr) {
	close();

	bool result =
		requireHidCapability() &&
		m_device.open((const char*)pathPtr.m_p);

	if (!result)
		return false;

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

bool
JNC_CDECL
HidDevice::open_1(
	uint16_t vid,
	uint16_t pid,
	DataPtr serialNumberPtr
) {
	close();

	bool result =
		requireHidCapability() &&
		m_device.open(vid, pid, (const char*)serialNumberPtr.m_p);

	if (!result)
		return false;

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

bool
HidDevice::finishOpen() {
	AsyncIoDevice::open();

	m_options |= AsyncIoDeviceOption_KeepReadWriteBlockSize;
	m_ioThreadFlags |= IoThreadFlag_Datagram;
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
HidDevice::close() {
	if (!m_device.isOpen())
		return;

	m_lock.lock();
	m_ioThreadFlags |= IoThreadFlag_Closing;
	wakeIoThread();
	m_lock.unlock();

	GcHeap* gcHeap = m_runtime->getGcHeap();
	gcHeap->enterWaitRegion();
	m_ioThread.waitAndClose();
	gcHeap->leaveWaitRegion();

	m_device.close();
	AsyncIoDevice::close();
}

void
HidDevice::ioThreadFunc() {
	ASSERT(m_device.isOpen());

	sl::Array<char> readBuffer;

	for (;;) {
		m_lock.lock();
		uint_t readTimeout = m_readTimeout;
		readBuffer.setCount(Def_ReadBlockSize);
		m_lock.unlock();

		size_t readResult = m_device.read(readBuffer, readBuffer.getCount(), readTimeout);
		if (readResult == -1) {
			setIoErrorEvent();
			break;
		}

		m_lock.lock();
		if (m_ioThreadFlags & IoThreadFlag_Closing) {
			m_lock.unlock();
			break;
		}

		if (readResult == 0) { // timeout
			m_lock.unlock();
			continue;
		}

		while (m_readBuffer.isFull()) {
			m_lock.unlock();
			sleepIoThread();
			m_lock.lock();

			if (m_ioThreadFlags & IoThreadFlag_Closing) {
				m_lock.unlock();
				return;
			}
		}

		addToReadBuffer(readBuffer, readResult);

		uint_t prevActiveEvents = m_activeEvents;
		m_activeEvents = 0;

		updateReadWriteBufferEvents();

		if (m_activeEvents != prevActiveEvents)
			processWaitLists_l();
		else
			m_lock.unlock();
	}
}

//..............................................................................

} // namespace io
} // namespace jnc
