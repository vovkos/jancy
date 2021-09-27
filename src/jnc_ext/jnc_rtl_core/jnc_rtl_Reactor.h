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

#include "jnc_RuntimeStructs.h"
#include "jnc_ExtensionLib.h"

namespace jnc {
namespace rtl {

JNC_DECLARE_OPAQUE_CLASS_TYPE(ReactorImpl)

//..............................................................................

class ReactorImpl: public Reactor {
protected:
	enum State {
		State_Stopped,
		State_Running,
		State_Reacting,
	};

	struct Binding: sl::ListLink {
		Multicast* m_multicast;
		handle_t m_handler;
		sl::HashTableIterator<Multicast*, Binding*> m_bindingMapIt;
		sl::BitMap m_reactionMap;
	};

	struct Reaction {
		size_t m_activationCount;
		sl::Array<Binding*> m_bindingArray;
	};

	typedef
	void
	ReactionFunc(
		Reactor* reactor,
		size_t index
	);

protected:
	State m_state;
	sl::AutoPtrArray<Reaction> m_reactionArray;
	sl::BitMap m_pendingReactionMap;
	sl::Array<Multicast*> m_pendingOnChangedBindingArray;
	sl::Array<Multicast*> m_pendingOnEventBindingArray;
	sl::List<Binding> m_bindingList;
	sl::SimpleHashTable<Multicast*, Binding*> m_bindingMap;

public:
	ReactorImpl();

	~ReactorImpl() {
		stop();
	}

	void
	JNC_CDECL
	start();

	void
	JNC_CDECL
	stop();

	void
	JNC_CDECL
	restart() {
		stop();
		start();
	}

	void
	JNC_CDECL
	addOnChangedBinding(Multicast* multicast) {
		ASSERT(m_state == State_Reacting);
		m_pendingOnChangedBindingArray.append(multicast);
	}

	void
	JNC_CDECL
	addOnEventBinding(Multicast* multicast) {
		ASSERT(m_state == State_Reacting);
		m_pendingOnEventBindingArray.append(multicast);
	}

	void
	JNC_CDECL
	resetOnChangedBindings() {
		ASSERT(m_state == State_Reacting);
		m_pendingOnChangedBindingArray.clear();
	}

protected:
	void
	reactionLoop();

	void
	onChanged(Binding* binding);

	static
	void
	onChangedThunk(ReactorClosure* closure) {
		((ReactorImpl*)closure->m_self)->onChanged((Binding*)closure->m_binding);
	}

	Binding*
	subscribe(Multicast* multicast);

	Binding*
	subscribe(
		Multicast* multicast,
		FunctionPtr functionPtr
	);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
