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
#include "jnc_rtl_Reactor.h"
#include "jnc_rtl_Multicast.h"
#include "jnc_rt_Runtime.h"
#include "jnc_ct_Module.h"
#include "jnc_ct_ReactorClassType.h"
#include "jnc_CallSite.h"

namespace jnc {
namespace rtl {

//..............................................................................

// can't use JNC_DEFINE_OPAQUE_CLASS_TYPE cause it relies on namespace lookups

JNC_EXTERN_C
jnc_ClassType*
ReactorImpl_getType(jnc_Module* module) {
	return (jnc_ClassType*)module->m_typeMgr.getStdType(StdType_ReactorBase);
}

JNC_EXTERN_C
const char*
ReactorImpl_getQualifiedName() {
	return "jnc.ReactorBase";
}

JNC_EXTERN_C
const jnc_OpaqueClassTypeInfo*
ReactorImpl_getOpaqueClassTypeInfo() {
	static jnc_OpaqueClassTypeInfo typeInfo = {
		sizeof(ReactorImpl), // m_size
		NULL,                // m_markOpaqueGcRootsFunc
		NULL,                // m_requireOpaqueItemsFunc
		false,               // m_isNonCreatable
	};

	return &typeInfo;
}

JNC_BEGIN_TYPE_FUNCTION_MAP(ReactorImpl)
	JNC_MAP_CONSTRUCTOR(&jnc::construct<ReactorImpl>)
	JNC_MAP_DESTRUCTOR(&jnc::destruct<ReactorImpl>)
	JNC_MAP_FUNCTION("start", &ReactorImpl::start);
	JNC_MAP_FUNCTION("stop", &ReactorImpl::stop);
	JNC_MAP_FUNCTION("restart", &ReactorImpl::restart);
	JNC_MAP_FUNCTION("!addOnChangedBinding", &ReactorImpl::addOnChangedBinding);
	JNC_MAP_FUNCTION("!addOnEventBinding", &ReactorImpl::addOnEventBinding);
	JNC_MAP_FUNCTION("!enterReactiveStmt", &ReactorImpl::enterReactiveStmt);
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

ReactorImpl::ReactorImpl() {
	m_state = State_Stopped;

	ASSERT(isClassType(m_box->m_type, ClassTypeKind_Reactor));

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_box->m_type;
	size_t reactionCount = reactorType->getReactionCount();

	m_reactionArray.setCount(reactionCount);
	sl::Array<Reaction*>::Rwi rwi = m_reactionArray;
	for (size_t i = 0; i < reactionCount; i++)
		rwi[i] = new Reaction;

	m_pendingReactionMap.setBitCount(reactionCount);
}

void
JNC_CDECL
ReactorImpl::start() {
	if (m_state != State_Stopped) // already running
		return;

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_box->m_type;
	ReactorFunc* reactorFunc = (ReactorFunc*)reactorType->getReactor()->getMachineCode();

	m_state = State_Starting;
	reactorFunc(this, -1);
	if (m_state == State_Stopped)
		return;

	processPendingBindings();

	ASSERT(m_state == State_Starting);
	m_state = State_Running;
}

void
JNC_CDECL
ReactorImpl::stop() {
	if (m_state == State_Stopped)
		return;

	sl::Iterator<Binding> it = m_bindingList.getHead();
	for (; it; it++)
		((rtl::MulticastImpl*)it->m_event)->removeHandler(it->m_handler);

	size_t reactionCount = m_reactionArray.getCount();
	for (size_t i = 0; i < reactionCount; i++) {
		Reaction* reaction = m_reactionArray[i];
		reaction->m_bindingArray.clear();
		reaction->m_flags = 0;
	}

	m_pendingReactionMap.clear();
	m_pendingOnChangedBindingArray.clear();
	m_pendingOnEventBindingArray.clear();
	m_bindingList.clear();
	m_bindingMap.clear();
	m_activeBindingArray.clear();

	m_state = State_Stopped;
}

void
JNC_CDECL
ReactorImpl::addOnChangedBinding(
	size_t reactionIdx,
	Multicast* multicast
) {
	if (m_state == State_Stopped)
		return;

	ASSERT(m_state == State_Starting || m_state == State_Reacting);
	m_pendingOnChangedBindingArray.append(PendingBinding(reactionIdx, multicast));
}

void
JNC_CDECL
ReactorImpl::addOnEventBinding(
	size_t reactionIdx,
	Multicast* multicast
) {
	if (m_state == State_Stopped)
		return;

	ASSERT(m_state == State_Starting || m_state == State_Reacting);
	m_pendingOnEventBindingArray.append(PendingBinding(reactionIdx, multicast));
}

void
JNC_CDECL
ReactorImpl::enterReactiveStmt(
	size_t reactionFromIdx,
	size_t reactionToIdx
) {
	if (m_state == State_Stopped)
		return;

	ASSERT(m_state == State_Starting || m_state == State_Reacting);
	m_pendingReactionMap.clearBitRange(reactionFromIdx, reactionToIdx);

	for (size_t i = reactionFromIdx; i < reactionToIdx; i++)
		activateReaction(i);
}

void
ReactorImpl::onChanged(Binding* binding) {
	m_pendingReactionMap.merge<sl::BitMapOr>(binding->m_reactionMap);

	size_t count = m_reactionArray.getCount();
	for (size_t i = 0; i < count; i++)
		printf("[%d]: 0x%x\n", i, m_reactionArray[i]->m_flags);

	if (m_state != State_Running)
		return;

	m_state = State_Reacting;
	reactionLoop();
	if (m_state == State_Stopped)
		return;

	ASSERT(m_state == State_Reacting);
	m_state = State_Running;
}

void
ReactorImpl::reactionLoop() {
	ASSERT(isClassType(m_box->m_type, ClassTypeKind_Reactor));

	// initialize reaction loop

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_box->m_type;
	ReactorFunc* reactorFunc = (ReactorFunc*)reactorType->getReactor()->getMachineCode();
	size_t reactionCount = m_reactionArray.getCount();
	m_activeReactionArray.clear();
	m_activeBindingArray.clear();

	// the main loop

	size_t reactionIdx = -1;
	for (;;) {
		reactionIdx = m_pendingReactionMap.findBit(reactionIdx + 1); // get the next pending reaction
		if (reactionIdx >= reactionCount) {
			reactionIdx = m_pendingReactionMap.findBit(0); // wrap
			if (reactionIdx >= reactionCount)
				break;
		}

		m_pendingReactionMap.clearBit(reactionIdx);
		bool result = activateReaction(reactionIdx);
		if (!result)
			continue;

		reactorFunc(this, reactionIdx);
		if (m_state == State_Stopped) // reactor stopped itself
			return;
	}

	// finalize

	processPendingBindings();
}

void
ReactorImpl::processPendingBindings() {
	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_box->m_type;
	size_t parentOffset = reactorType->getParentOffset();
	IfaceHdr* parent = parentOffset ? (IfaceHdr*)((char*)this - parentOffset) : NULL;

	// subscribe to 'onchanged' events

	size_t count = m_pendingOnChangedBindingArray.getCount();
	for (size_t i = 0; i < count; i++) {
		PendingBinding pendingBinding = m_pendingOnChangedBindingArray[i];
		sl::HashTableIterator<Multicast*, Binding*> it = m_bindingMap.visit(pendingBinding.m_event);
		Binding* binding;

		if (it->m_value) {
			binding = it->m_value;
		} else {
			binding = subscribe(pendingBinding.m_event);
			binding->m_bindingMapIt = it;
			it->m_value = binding;
		}

		if (!binding->m_reactionMap.getBit(pendingBinding.m_reactionIdx)) {
			binding->m_reactionMap.setBitResize(pendingBinding.m_reactionIdx);

			Reaction* reaction = m_reactionArray[pendingBinding.m_reactionIdx];
			ASSERT(reaction->m_bindingArray.find(binding) == -1);
			reaction->m_bindingArray.append(binding);
		}
	}

	// subscribe to 'onevent' events

	count = m_pendingOnEventBindingArray.getCount();
	for (size_t i = 0; i < count; i++) {
		PendingBinding pendingBinding = m_pendingOnEventBindingArray[i];

		Function* onEvent = reactorType->getOnEventHandler(pendingBinding.m_reactionIdx);
		ASSERT(onEvent);

		AXL_TODO("currently, onevent handlers adjust 'this' internally -- need to clean that up and pass 'parent' directly")

		FunctionPtr onEventPtr = {
			onEvent->getMachineCode(),
			this
		};

		Reaction* reaction = m_reactionArray[pendingBinding.m_reactionIdx];
		Binding* binding = subscribe(pendingBinding.m_event, onEventPtr);
		reaction->m_bindingArray.append(binding);
	}

	// unsubscribe from unused events

	count = m_activeBindingArray.getCount();
	for (size_t i = 0; i < count; i++) {
		Binding* binding = m_activeBindingArray[i];
		if (binding->m_reactionMap.findBit(0) != -1) { // have at least one reaction bound to this event
			binding->m_flags &= Flag_Active;
			continue;
		}

		((rtl::MulticastImpl*)binding->m_event)->removeHandler(binding->m_handler);
		m_bindingMap.erase(binding->m_bindingMapIt);
		m_bindingList.erase(binding);
	}

	// reset all active reactions

	count = m_activeReactionArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_activeReactionArray[i]->m_flags &= ~Flag_Active;

	m_activeBindingArray.clear();
	m_activeReactionArray.clear();
}

bool
ReactorImpl::activateReaction(size_t reactionIdx) {
	Reaction* reaction = m_reactionArray[reactionIdx];
	ASSERT(reaction);

	if (reaction->m_flags & Flag_Active) // a loop detected -- ignore (maybe, throw?)
		return false;

	// add all bindings of this reaction to the active array

	size_t bindingCount = reaction->m_bindingArray.getCount();
	for (size_t j = 0; j < bindingCount; j++) {
		Binding* binding = reaction->m_bindingArray[j];
		if (!(binding->m_flags & Flag_Active)) {
			m_activeBindingArray.append(binding);
			binding->m_reactionMap.clearBit(reactionIdx);
			binding->m_flags |= Flag_Active;
		}
	}

	// add this reaction to the active array

	m_activeReactionArray.append(reaction);
	reaction->m_flags |= Flag_Active;
	reaction->m_bindingArray.clear(); // re-populate it in processPendingBindings
	return true;
}

ReactorImpl::Binding*
ReactorImpl::subscribe(Multicast* event) {
	Binding* binding = new Binding;
	binding->m_event = event;
	m_bindingList.insertTail(binding);

	Runtime* runtime = getCurrentThreadRuntime();
	JNC_BEGIN_CALL_SITE(runtime)
		ClassType* closureType = (ClassType*)runtime->getModule()->m_typeMgr.getStdType(StdType_ReactorClosure);
		ReactorClosure* closure = (ReactorClosure*)runtime->getGcHeap()->allocateClass(closureType);
		closure->m_reactor = this;
		closure->m_binding = binding;

		FunctionPtr functionPtr;
		functionPtr.m_p = jnc_pvoid_cast(onChangedThunk);
		functionPtr.m_closure = &closure->m_ifaceHdr;

		binding->m_handler = ((MulticastImpl*)event)->addHandler(functionPtr);
	JNC_END_CALL_SITE()

	return binding;
}

ReactorImpl::Binding*
ReactorImpl::subscribe(
	Multicast* event,
	FunctionPtr functionPtr
) {
	Binding* binding = new Binding;
	binding->m_event = event;
	binding->m_handler = ((MulticastImpl*)event)->addHandler(functionPtr);
	m_bindingList.insertTail(binding);
	return binding;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
