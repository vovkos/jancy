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

	char buffer[256];wait pid timeout
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

JNC_DEFINE_OPAQUE_CLASS_TYPE(
	ChildProcess,
	"io.ChildProcess",
	g_ioLibGuid,
	IoLibCacheSlot_ChildProcess,
	ChildProcess,
	NULL
	)

JNC_BEGIN_TYPE_FUNCTION_MAP(ChildProcess)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<ChildProcess>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<ChildProcess>)
	JNC_MAP_FUNCTION("start", &ChildProcess::start)
	JNC_MAP_FUNCTION("close", &ChildProcess::close)
	JNC_MAP_FUNCTION("wait", &ChildProcess::wait)
	JNC_MAP_FUNCTION("waitAndClose", &ChildProcess::waitAndClose)
	JNC_MAP_FUNCTION("terminate", &ChildProcess::terminate)
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

ChildProcess::ChildProcess()
{
	jnc::construct<FileStream>(m_stdin);
	jnc::construct<FileStream>(m_stdout);
	jnc::construct<FileStream>(m_stderr);
}

ChildProcess::~ChildProcess()
{
	close();
}

bool
JNC_CDECL
ChildProcess::start(
	DataPtr commandLinePtr,
	uint_t flags
	)
{
	close();

	bool isMergedStdoutStderr = (flags & ChildProcessFlag_MergeStdoutStderr) != 0;

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
		(isMergedStdoutStderr || createStdioPipe(parentStderr.p(), childStderr.p(), FILE_FLAG_OVERLAPPED, 0));

	if (!result)
		return false;

	STARTUPINFOW startupInfo = { 0 };
	startupInfo.cb = sizeof(STARTUPINFO);
	startupInfo.dwFlags = STARTF_USESTDHANDLES | STARTF_USESHOWWINDOW;
	startupInfo.hStdInput = childStdin;
	startupInfo.hStdOutput = childStdout;
	startupInfo.hStdError = isMergedStdoutStderr ? childStdout : childStderr;
	startupInfo.wShowWindow = SW_HIDE;

	result = m_process.createProcess(cmdLine, true, CREATE_NEW_CONSOLE, &startupInfo);
	if (!result)
		return false;

	attachFileStream(m_stdin, &parentStdin);
	attachFileStream(m_stdout, &parentStdout);
	attachFileStream(m_stderr, &parentStderr);
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
		(isMergedStdoutStderr ||
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
		::dup2(isMergedStdoutStderr ? stdoutPipe.m_writeFile : stderrPipe.m_writeFile, STDERR_FILENO);

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

	attachFileStream(m_stdin, &stdinPipe.m_writeFile);
	attachFileStream(m_stdout, &stdoutPipe.m_readFile);
	attachFileStream(m_stderr, &stderrPipe.m_readFile);
#endif

	return true;
}

void
JNC_CDECL
ChildProcess::close()
{
	m_stdin->close();
	m_stdout->close();
	m_stderr->close();
#if (_JNC_OS_WIN)
	m_process.close();
#endif
}

uint_t
JNC_CDECL
ChildProcess::getExitCode()
{
#if (_JNC_OS_WIN)
	dword_t exitCode = 0;
	m_process.getExitCode(&exitCode);
	return exitCode;
#endif
	return 0;
}

bool
JNC_CDECL
ChildProcess::wait(uint_t timeout)
{
	return true;
}

void
JNC_CDECL
ChildProcess::waitAndClose(uint_t timeout)
{
	close();
}

bool
JNC_CDECL
ChildProcess::terminate()
{
	return true;
}

void
ChildProcess::attachFileStream(
	io::FileStream* fileStream,
	AxlOsFile* file
	)
{
	fileStream->close();

	if (!file->isOpen())
		return;

	fileStream->m_file.m_file.attach(file->detach());
	fileStream->m_fileStreamKind = FileStreamKind_Pipe;
/*	fileStream->setReadParallelism(m_readParallelism);
	fileStream->setReadBlockSize(m_readBlockSize);
	fileStream->setReadBufferSize(m_readBufferSize);
	fileStream->setWriteBufferSize(m_writeBufferSize);
	fileStream->setOptions(m_options); */
#if (_JNC_OS_WIN)
	ASSERT(!fileStream->m_overlappedIo);
	fileStream->m_overlappedIo = AXL_MEM_NEW(FileStream::OverlappedIo);
#endif
	fileStream->AsyncIoDevice::open();
	fileStream->m_ioThread.start();
}

//..............................................................................

} // namespace io
} // namespace jnc
