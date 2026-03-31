#include "pch.h"
#include "jnc_CodeTip.h"
#include "moc_jnc_CodeTip.cpp"
#include "jnc_Highlighter.h"

namespace jnc {

//..............................................................................

CodeTip::CodeTip(EditBase* parent):
	CodeTipBase(parent) {

	m_functionTypeOverload = NULL;
	m_template = NULL;
}

void
CodeTip::showQuickInfoTip(
	const QPoint& pos,
	ModuleItem* item
) {
	m_template = NULL;
	m_functionTypeOverload = NULL;

	const char* synopsis = item->getItemString(ModuleItemStringKind_Synopsis);
	QString text = highlightJancySource(synopsis, theme());

	showTip(
		CodeAssistKind_QuickInfoTip,
		pos,
		text,
		0,
		0,
		0
	);
}

void
CodeTip::showArgumentTip(
	const QPoint& pos,
	FunctionTypeOverload* overload,
	size_t argumentIdx
) {
	m_template = NULL;
	m_functionTypeOverload = overload;

	size_t overloadCount = overload->getOverloadCount();
	size_t overloadIdx = tipIndex();

	if (!isVisible() || overloadIdx >= overloadCount)
		overloadIdx = 0;

	showTip(
		CodeAssistKind_ArgumentTip,
		pos,
		getArgumentTipText(overloadIdx),
		overloadCount,
		overloadIdx,
		argumentIdx
	);
}

void
CodeTip::showArgumentTip(
	const QPoint& pos,
	Template* templ,
	size_t argumentIdx
) {
	m_template = templ;
	m_functionTypeOverload = NULL;

	showTip(
		CodeAssistKind_ArgumentTip,
		pos,
		getArgumentTipText(0),
		1,
		0,
		argumentIdx
	);
}

QString
CodeTip::getArgumentTipText(size_t overloadIdx) {
	if (m_template) {
		ASSERT(overloadIdx == 0);
		return getArgumentTipText(m_template, argumentIndex());
	}

	ASSERT(m_functionTypeOverload);
	size_t overloadCount = m_functionTypeOverload->getOverloadCount();
	ASSERT(overloadIdx < overloadCount);

	FunctionType* type = m_functionTypeOverload->getOverload(overloadIdx);
	QString text = getArgumentTipText(type, argumentIndex());

	if (overloadCount > 1)
		text = QString("%1 of %2<hr>%3").
			arg(overloadIdx + 1).
			arg(overloadCount).
			arg(text);

	return text;
}

QString
CodeTip::getArgumentTipText(
	FunctionType* type,
	size_t argumentIdx
) {
	#define ML_ARG_INDENT "&nbsp;&nbsp;&nbsp;&nbsp;"

	bool isConst = false;
	FunctionType* shortType = type->getShortType();
	if (shortType != type) { // a member function
		isConst = type->getArgCount() && (type->getArg(0)->getType()->getFlags() & ConstKind_Const);
		type = shortType;

		if (argumentIdx)
			argumentIdx--;
	}

	Type* returnType = type->getReturnType();
	size_t argCount = type->getArgCount();
	size_t lastArgIdx = argCount - 1;
	bool isMl = argCount >= 2;

	const EditTheme* theme = this->theme();
	QString text = highlightJancySource(returnType->getTypeString(), theme);
	text += isMl ? " (<br>" ML_ARG_INDENT : " (";

	for (size_t i = 0; i < argCount; i++) {
		FunctionArg* arg = type->getArg(i);
		Type* argType = arg->getType();

		if (i == argumentIdx)
			text += "<b>";

		text += highlightJancySource(argType->getTypeStringPrefix(), theme);
		text += ' ';
		text += arg->getDecl()->getName();
		text += highlightJancySource(argType->getTypeStringSuffix(), theme);

		if (arg->hasDefaultValue()) {
			text += " = ";
			text += highlightJancySource(arg->getDefaultValueString(), theme);
		}

		if (i == argumentIdx)
			text += "</b>";

		if (i != lastArgIdx)
			text += ",<br>" ML_ARG_INDENT;
	}

	if (type->getFlags() & FunctionTypeFlag_VarArg)
		text += isMl ? ",<br>" ML_ARG_INDENT "..." : ", ...";

	text += isMl ? "<br>)" : ")";

	if (isConst)
		text += highlightJancySource(" const", theme);

	return text;
}

QString
CodeTip::getArgumentTipText(
	Template* templ,
	size_t argumentIdx
) {
	size_t argCount = templ->getArgCount();
	size_t lastArgIdx = argCount - 1;
	const EditTheme* theme = this->theme();
	QString text = "&lt;";

	for (size_t i = 0; i < argCount; i++) {
		TemplateArgType* arg = templ->getArg(i);
		if (i == argumentIdx)
			text += "<b>";

		text += arg->getTypeString();

		Type* defaultType = arg->getDefaultType();
		if (defaultType) {
			text += " = ";
			text += highlightJancySource(defaultType->getTypeString(), theme);
		}

		if (i == argumentIdx)
			text += "</b>";

		if (i != lastArgIdx)
			text += ", ";
	}

	text += "&gt;";
	return text;
}

//..............................................................................

} // namespace jnc
