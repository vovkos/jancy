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
		sizeof(ReactorImpl),  // m_size
		NULL,                 // m_markOpaqueGcRootsFunc
		NULL,                 // m_requireOpaqueItemsFunc
		false,                // m_isNonCreatable
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
JNC_END_TYPE_FUNCTION_MAP()

//..............................................................................

ReactorImpl::ReactorImpl() {
	m_activationCountLimit = 1; // disallow loops
	m_state = State_Stopped;

	ASSERT(isClassType(m_ifaceHdr.m_box->m_type, ClassTypeKind_Reactor));

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_ifaceHdr.m_box->m_type;
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

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_ifaceHdr.m_box->m_type;
	ReactionFunc* reactorFunc = (ReactionFunc*)reactorType->getReactor()->getMachineCode();

	m_state = State_Starting;
	reactorFunc(this, -1);
	if (m_state == State_Stopped)
		return;

	m_state = State_Reacting;
	reactionLoop();
	if (m_state == State_Stopped)
		return;

	ASSERT(m_state == State_Reacting);
	m_state = State_Running;
}

void
JNC_CDECL
ReactorImpl::stop() {
	if (m_state == State_Stopped)
		return;

	sl::Iterator<Binding> it = m_bindingList.getHead();
	for (; it; it++)
		((rtl::MulticastImpl*)it->m_multicast)->removeHandler(it->m_handler);

	size_t reactionCount = m_reactionArray.getCount();
	for (size_t i = 0; i < reactionCount; i++) {
		Reaction* reaction = m_reactionArray[i];
		reaction->m_activationCount = 0;
		reaction->m_bindingArray.clear();
	}

	m_pendingReactionMap.clear();
	m_pendingOnChangedBindingArray.clear();
	m_pendingOnEventBindingArray.clear();
	m_bindingList.clear();
	m_bindingMap.clear();

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
ReactorImpl::onChanged(Binding* binding) {
	m_pendingReactionMap.merge<sl::BitMapOr>(binding->m_reactionMap);

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
	ASSERT(isClassType(m_ifaceHdr.m_box->m_type, ClassTypeKind_Reactor));

	ct::ReactorClassType* reactorType = (ct::ReactorClassType*)m_ifaceHdr.m_box->m_type;
	ReactionFunc* reactorFunc = (ReactionFunc*)reactorType->getReactor()->getMachineCode();

	size_t parentOffset = reactorType->getParentOffset();
	IfaceHdr* parent = parentOffset ? (IfaceHdr*)((char*)this - parentOffset) : NULL;

	char buffer[256];
	sl::Array<Binding*> oldBindingArray(rc::BufKind_Stack, buffer, sizeof(buffer));

	// first, reset activation count

	size_t reactionCount = m_reactionArray.getCount();
	for (size_t i = 0; i < reactionCount; i++)
		m_reactionArray[i]->m_activationCount = 0;

	// the main loop

	size_t i = -1;
	for (;;) {
		// get the next pending reaction

		i = m_pendingReactionMap.findBit(i + 1);
		if (i >= reactionCount) {
			i = m_pendingReactionMap.findBit(0); // wrap
			if (i >= reactionCount)
				break;
		}

		m_pendingReactionMap.clearBit(i);

		Reaction* reaction = m_reactionArray[i];
		ASSERT(reaction);

		if (reaction->m_activationCount >= m_activationCountLimit) // too many recursive activations of the reaction -- ignore
			continue;

		reaction->m_activationCount++;

		m_pendingOnChangedBindingArray.clear();
		m_pendingOnEventBindingArray.clear();

		// run reaction #i

		reactorFunc(this, i);

		if (m_state == State_Stopped)
			return;

		// subscribe to new events

		size_t oldBindingCount = reaction->m_bindingArray.getCount();
		oldBindingArray.copy(reaction->m_bindingArray, oldBindingCount);

		size_t bindingCount = m_pendingOnChangedBindingArray.getCount();
		for (size_t j = 0; j < bindingCount; j++) {
			Multicast* multicast = m_pendingOnChangedBindingArray[j].m_multicast;
			sl::HashTableIterator<Multicast*, Binding*> it = m_bindingMap.visit(multicast);
			Binding* binding;

			if (it->m_value) {
				binding = it->m_value;
			} else {
				binding = subscribe(multicast);
				binding->m_bindingMapIt = it;
				it->m_value = binding;
			}

			if (!binding->m_reactionMap.getBit(i)) {
				binding->m_reactionMap.setBitResize(i);

				ASSERT(reaction->m_bindingArray.find(binding) == -1);
				reaction->m_bindingArray.append(binding);
			}
		}

		// unsubscribe from unused events

		for (size_t j = 0; j < oldBindingCount; j++) {
			Binding* binding = oldBindingArray[j];
			if (binding->m_reactionMap.findBit(0) != -1)
				continue;

			((rtl::MulticastImpl*)binding->m_multicast)->removeHandler(binding->m_handler);
			m_bindingMap.erase(binding->m_bindingMapIt);
			m_bindingList.erase(binding);
		}

		// subscribe to 'onevent' events

		if (!m_pendingOnEventBindingArray.isEmpty()) {
			Function* onEvent = reactorType->findOnEventHandler(i);
			ASSERT(onEvent);

			AXL_TODO("currently, onevent handlers adjust 'this' internally -- need to clean that up and pass 'parent' directly")

			FunctionPtr onEventPtr = {
				onEvent->getMachineCode(),
				&m_ifaceHdr
			};

			size_t bindingCount = m_pendingOnEventBindingArray.getCount();
			for (size_t j = 0; j < bindingCount; j++) {
				Multicast* multicast = m_pendingOnEventBindingArray[j].m_multicast;
				Binding* binding = subscribe(multicast, onEventPtr);
				reaction->m_bindingArray.append(binding);
			}
		}
	}
}

ReactorImpl::Binding*
ReactorImpl::subscribe(Multicast* multicast) {
	Binding* binding = new Binding;
	binding->m_multicast = multicast;
	m_bindingList.insertTail(binding);

	Runtime* runtime = getCurrentThreadRuntime();

	JNC_BEGIN_CALL_SITE(runtime)
		ClassType* closureType = (ClassType*)runtime->getModule()->m_typeMgr.getStdType(StdType_ReactorClosure);
		ReactorClosure* closure = (ReactorClosure*)runtime->getGcHeap()->allocateClass(closureType);
		closure->m_self = this;
		closure->m_binding = binding;

		FunctionPtr functionPtr;
		functionPtr.m_p = jnc_pvoid_cast(onChangedThunk);
		functionPtr.m_closure = &closure->m_ifaceHdr;

		binding->m_handler = ((MulticastImpl*)multicast)->addHandler(functionPtr);
	JNC_END_CALL_SITE()

	return binding;
}

ReactorImpl::Binding*
ReactorImpl::subscribe(
	Multicast* multicast,
	FunctionPtr functionPtr
) {
	Binding* binding = new Binding;
	binding->m_multicast = multicast;
	binding->m_handler = ((MulticastImpl*)multicast)->addHandler(functionPtr);
	m_bindingList.insertTail(binding);

	return binding;
}

//..............................................................................

} // namespace rtl
} // namespace jnc
