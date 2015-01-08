#pragma once

#define _CRT_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" string functions
#define _SCL_SECURE_NO_WARNINGS // disable useless warnings about "unsafe" iterators

#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS

#include "jnc_Pch.h"

#undef min
#undef max

// QT

#include <QtGui>
#include <QApplication>
#include <QMainWindow>
#include <QDockWidget>
#include <QBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QCheckBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QSlider>
#include <QFileDialog>
#include <QMessageBox>

// Jancy

#include "jnc_Module.h"
#include "jnc_Runtime.h"
#include "jnc_StdLib.h"
#include "jnc_Disassembler.h"

using namespace axl;

#if (_AXL_ENV == AXL_ENV_WIN)

// Memory Leak Detection

#	define _CRTDBG_MAP_ALLOC
#	include <stdlib.h>
#	include <crtdbg.h>

#	ifdef _DEBUG
#		ifndef DBG_NEW
#			define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#			define new DBG_NEW
#		endif
#	endif

#elif (_AXL_ENV == AXL_ENV_POSIX)
#	include <sys/socket.h>
#	include <netinet/in.h>
#	include <netinet/ip.h>
#endif