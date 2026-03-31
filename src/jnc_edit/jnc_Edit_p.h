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

#include "jnc_Edit.h"
#include "jnc_EditBase_p.h"

namespace jnc {

class CodeTip;

//..............................................................................

class EditPrivate: public EditBasePrivate {
	Q_OBJECT
	Q_DECLARE_PUBLIC(Edit)

protected:
	enum Role {
		Role_ModuleItem = EditBase::CaseInsensitiveSortRole + 1
	};

protected:
	rc::Ptr<Module> m_activeCodeAssistModule;

protected:
	void
	createCodeAssist(
		const rc::Ptr<Module>& module,
		CodeAssist* codeAssist
	);

	void
	createQuickInfoTip(ModuleItem* item);

	void
	createArgumentTip(
		FunctionTypeOverload* typeOverload,
		size_t argumentIdx
	);

	void
	createArgumentTip(
		Template* templ,
		size_t argumentIdx
	);

	void
	createAutoComplete(
		Namespace* nspace,
		uint_t flags
	);

	void
	createImportAutoComplete(Module* module);

	void
	addAutoCompleteNamespace(
		QStandardItemModel* model,
		Namespace* nspace
	);

	size_t
	getItemIconIdx(ModuleItem* item);

	Function*
	getPrototypeFunction(const QModelIndex& index);
};

//..............................................................................

} // namespace jnc
