#ifndef _DISASSEMBLY_H
#define _DISASSEMBLY_H

#include "editor.h"

#define DisassemblyBase Editor

class Disassembly : public DisassemblyBase
{
	Q_OBJECT

public:
	Disassembly(QWidget *parent);

	QSize sizeHint() const { return QSize(300, 50); }	

	bool build(jnc::CModule *module);

protected:
	void addFunction(jnc::CFunction* function);
};


#endif

