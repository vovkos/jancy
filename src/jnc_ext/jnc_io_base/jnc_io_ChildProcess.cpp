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
#include "jnc_io_ChildProcess.h"
#include "jnc_io_IoLib.h"

namespace jnc {
namespace io {

//..............................................................................

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ChildProcess,
	"io.ChildProcess",
	g_ioLibGuid,
	IoLibCacheSlot_ChildProcess,
	ChildProcess,
	&ChildProcess::markOpaqueGcRoots
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ChildProcess)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<ChildProcess>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<ChildProcess>)

	JNC_MAP_AUTOGET_PROPERTY("m_readParallelism", &ChildProcess::setReadParallelism)
	JNC_MAP_AUTOGET_PROPERTY("m_readBlockSize",   &ChildProcess::setReadBlockSize)
	JNC_MAP_AUTOGET_PROPERTY("m_readBufferSize",  &ChildProcess::setReadBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_writeBufferSize", &ChildProcess::setWriteBufferSize)
	JNC_MAP_AUTOGET_PROPERTY("m_options",         &ChildProcess::setOptions)
	JNC_MAP_CONST_PROPERTY("m_exitCode",          &ChildProcess::getExitCode)

	JNC_MAP_FUNCTION("start",        &ChildProcess::start)
	JNC_MAP_FUNCTION("terminate",    &ChildProcess::terminate)
	JNC_MAP_FUNCTION("close",        &ChildProcess::close)
	JNC_MAP_FUNCTION("read",         &ChildProcess::read)
	JNC_MAP_FUNCTION("write",        &ChildProcess::write)
	JNC_MAP_FUNCTION("wait",         &ChildProcess::wait)
	JNC_MAP_FUNCTION("cancelWait",   &ChildProcess::cancelWait)
	JNC_MAP_FUNCTION("blockingWait", &ChildProcess::blockingWait)
	JNC_MAP_FUNCTION("asyncWait",    &ChildProcess::asyncWait)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

#if (_JNC_OS_WIN)
bool
createStdioPipe(
	handle_t* readHandle,
	handle_t* writeHandle,
	dword_t readMode,
	dword_t writeMode
	)
{
	static int32_t pipeId = 0;

	char buffer[256];
	sl::String_w pipeName(ref::BufKind_Stack, buffer, sizeof(buffer));
	pipeName.format(
		L"\\\\.\\pipe\\jnc.ChildProcess.%p.%d",
		sys::getCurrentProcessId(),
		sys::atomicInc(&pipeId)
		);

	axl::io::win::NamedPipe pipe;
	axl::io::win::File file;

	SECURITY_ATTRIBUTES secAttr = { 0 };
	secAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
	secAttr.bInheritHandle = true;

	bool result =
		pipe.create(
			pipeName,
			PIPE_ACCESS_INBOUND | readMode,
			PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			1,
			0, 0, 0,
			&secAttr
			) &&
		file.create(
			pipeName,
			GENERIC_WRITE,
			0,
			&secAttr,
			OPEN_EXISTING,
			writeMode
			);

	if (!result)
		return false;

	*readHandle = pipe.detach();
	*writeHandle = file.detach();
	return true;
}
#else
void
exec(const char* commandLine) // returns on failure only
{
	char buffer[256];
	sl::Array<char*> argv(ref::BufKind_Stack, buffer, sizeof(buffer));

	sl::String string = commandLine;

	size_t length = string.getLength();
	for (;;)
	{
		string.trimLeft();
		if (string.isEmpty())
			break;

		argv.append(string.p());

		size_t pos = string.findOneOf(sl::StringDetails::getWhitespace());
		if (pos == -1)
			break;

		string[pos] = 0;
		string = string.getSubString(pos + 1);
	}

	if (argv.isEmpty())
	{
		err::setError("empty command line");
		return;
	}

	argv.append(NULL);

	int result = ::execvp(argv[0], argv.p());
	ASSERT(result == -1);
	err::setLastSystemError();
}
#endif

//..............................................................................

ChildProcess::ChildProcess():
	m_stderr((FileStream*&)m_reserved)
{
	m_writeFile = &m_stdin;
	m_finalizeIoThreadFunc = finalizeIoThread;
}

bool
JNC_CDECL
ChildProcess::start(
	DataPtr commandLinePtr,
	uint_t flags
	)
{
	close();

	bool isSeparateStderr = (flags & ChildProcessFlag_SeparateStderr) != 0;

#if (_JNC_OS_WIN)
	sl::String_w cmdLine = (char*)commandLinePtr.m_p;

	bool_t result;

	axl::io::win::File parentStdin;
	axl::io::win::File parentStdout;
	axl::io::win::File parentStderr;
	axl::io::win::File childStdin;
	axl::io::win::File childStdout;
	axl::io::win::File childStderr;

	result =
		createStdioPipe(childStdin.p(), parentStdin.p(), 0, FILE_FLAG_OVERLAPPED) &&
		createStdioPipe(parentStdout.p(), childStdout.p(), FILE_FLAG_OVERLAPPED, 0) &&
		(!isSeparateStderr || createStdioPipe(parentStderr.p(), childStderr.p(), FILE_FLAG_OVERLAPPED, 0));

	if (!result)
		return false;

	STARTUPINFOW startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.hStdInput = childStdin;
	startupInfo.hStdOutput = childStdout;
	startupInfo.hStdError = isSeparateStderr ? childStderr : childStdout;
	startupInfo.wShowWindow = SW_HIDE;

	result = m_process.create(cmdLine, true, CREATE_NEW_CONSOLE, &startupInfo);
	if (!result)
		return false;

	m_stdin.m_file.attach(parentStdin.detach());
	m_file.m_file.attach(parentStdout.detach());

	ASSERT(!m_overlappedIo);
	m_overlappedIo = AXL_MEM_NEW(FileStream::OverlappedIo);

	if (isSeparateStderr)
		m_stderr = createFileStream(&parentStderr);
#else
	axl::io::psx::Pipe stdinPipe;
	axl::io::psx::Pipe stdoutPipe;
	axl::io::psx::Pipe stderrPipe;
	axl::io::psx::Pipe execPipe;

	bool result =
		stdinPipe.create() &&
		stdinPipe.m_writeFile.setBlockingMode(false) &&
		stdoutPipe.create() &&
		stdoutPipe.m_readFile.setBlockingMode(false) &&
		(!isSeparateStderr ||
		stderrPipe.create() &&
		stderrPipe.m_readFile.setBlockingMode(false)) &&
		execPipe.create();

	if (!result)
		return false;

	execPipe.m_writeFile.fcntl(F_SETFD, FD_CLOEXEC);

	err::Error error;

	pid_t pid = ::fork();
	switch (pid)
	{
	case -1:
		err::setLastSystemError();
		return false;

	case 0:
		::dup2(stdinPipe.m_readFile, STDIN_FILENO);
		::dup2(stdoutPipe.m_writeFile, STDOUT_FILENO);
		::dup2(isSeparateStderr ? stderrPipe.m_writeFile : stdoutPipe.m_writeFile, STDERR_FILENO);

		exec((char*)commandLinePtr.m_p);

		error = err::getLastError();
		execPipe.m_writeFile.write(error, error->m_size);
		execPipe.m_writeFile.flush();

		::_exit(-1);
		ASSERT(false);

	default:
		execPipe.m_writeFile.close();

		fd_set rdset;
		FD_ZERO(&rdset);
		FD_SET(execPipe.m_readFile, &rdset);

		char buffer[256];
		size_t size = execPipe.m_readFile.read(buffer, sizeof(buffer));
		if (size == 0 || size == -1)
		{
			m_pid = pid;
			break;
		}

		if (((err::ErrorHdr*)buffer)->m_size == size)
			err::setError((err::ErrorHdr*)buffer);
		else
			err::setError("POSIX execvp failed"); // unlikely fallback

		return false;
	}

	m_stdin.m_file.attach(stdinPipe.m_writeFile.detach());
	m_file.m_file.attach(stdoutPipe.m_readFile.detach());

	if (isSeparateStderr)
		m_reserved = createFileStream(&stderrPipe.m_readFile);
#endif

	AsyncIoDevice::open();
	m_ioThread.start();
	return true;
}

void
JNC_CDECL
ChildProcess::close()
{
	FileStream::close();
	m_stdin.close();

	if (m_reserved)
	{
		((FileStream*)m_reserved)->close();
		m_reserved = NULL;
	}

#if (_JNC_OS_WIN)
	m_process.close();
#else
	m_pid = 0;
#endif

	m_exitCode = 0;
}

uint_t
JNC_CDECL
ChildProcess::getExitCode()
{
	m_lock.lock();
	if (!(m_activeEvents & ChildProcessEvent_Finished))
	{
		m_lock.unlock();
		return 0;
	}

	uint_t exitCode = m_exitCode;
	m_lock.unlock();
	return exitCode;
}

bool
JNC_CDECL
ChildProcess::terminate()
{
#if (_JNC_OS_WIN)
	return m_process.terminate(STATUS_CONTROL_C_EXIT);
#else
	int result = ::kill(m_pid, SIGKILL);
	return err::complete(result == 0);
#endif
}

FileStream*
ChildProcess::createFileStream(AxlOsFile* file)
{
	FileStream* fileStream = jnc::createClass<FileStream> (m_runtime);
	fileStream->m_file.m_file.attach(file->detach());
	fileStream->setReadParallelism(m_readParallelism);
	fileStream->setReadBlockSize(m_readBlockSize);
	fileStream->setReadBufferSize(m_readBufferSize);
	fileStream->setWriteBufferSize(m_writeBufferSize);
	fileStream->setOptions(m_options);

#if (_JNC_OS_WIN)
	ASSERT(!fileStream->m_overlappedIo);
	fileStream->m_overlappedIo = AXL_MEM_NEW(FileStream::OverlappedIo);
#endif

	fileStream->AsyncIoDevice::open();
	fileStream->m_ioThread.start();
	return fileStream;
}

void
ChildProcess::finalizeIoThreadImpl()
{
	m_lock.lock();
	if (m_ioThreadFlags & IoThreadFlag_Closing)
	{
		m_lock.unlock();
		return;
	}

	m_lock.unlock();

	uint_t exitCode;
	bool isCrashed;

#if (_JNC_OS_WIN)
	m_process.wait();
	m_process.getExitCode((dword_t*)&exitCode);
	isCrashed = exitCode >= 0xc0000000 && exitCode < 0xd0000000;
#else
	int status;
	::waitpid(m_pid, &status, 0);
	exitCode = WEXITSTATUS(status);
	isCrashed = WIFSIGNALED(status);
#endif

	m_lock.lock();
	m_exitCode = exitCode;

	setEvents_l(isCrashed ?
		ChildProcessEvent_Finished | ChildProcessEvent_Crashed :
		ChildProcessEvent_Finished
		);
}

//..............................................................................

} // namespace io
} // namespace jnc
