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

#include "jnc_io_FileStream.h"

namespace jnc {
namespace io {

JNC_DECLARE_OPAQUE_CLASS_TYPE(ChildProcess)

//..............................................................................

enum ChildProcessFlag
{
	ChildProcessFlag_SeparateStderr = 0x01
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

enum ChildProcessEvent
{
	ChildProcessEvent_Finished = 0x0020,
	ChildProcessEvent_Crashed  = 0x0040,
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

class ChildProcess: public FileStream
{
public:
	JNC_DECLARE_CLASS_TYPE_STATIC_METHODS(ChildProcess)

protected:
	axl::io::File m_stdin;
	FileStream*& m_stderr;

#if (_JNC_OS_WIN)
	sys::win::Process m_process;
#else
	pid_t m_pid;
#endif

	uint_t m_exitCode;

public:
	ChildProcess();

	~ChildProcess()
	{
		close();
	}

	uint_t
	JNC_CDECL
	getExitCode();

	bool
	JNC_CDECL
	start(
		DataPtr commandLinePtr,
		uint_t flags
		);

	bool
	JNC_CDECL
	terminate();

	void
	JNC_CDECL
	close();

protected:
#if (_JNC_OS_WIN)
	typedef axl::io::win::File AxlOsFile;
#else
	typedef axl::io::psx::File AxlOsFile;
#endif
	FileStream*
	createFileStream(AxlOsFile* file);

	static
	void
	finalizeIoThread(FileStream* self)
	{
		((ChildProcess*)self)->finalizeIoThreadImpl();
	}

	void
	finalizeIoThreadImpl();
};

//..............................................................................

} // namespace io
} // namespace jnc
