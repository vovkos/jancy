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
		State_Starting,
		State_Running,
		State_Reacting,
	};

	enum Flag {
		Flag_Active = 0x01, // active
	};

	struct Binding: sl::ListLink {
		Multicast* m_event;
		handle_t m_handler;
		sl::HashTableIterator<Multicast*, Binding*> m_bindingMapIt;
		sl::BitMap m_reactionMap;
		uint_t m_flags;

		Binding() {
			m_flags = 0;
		}
	};

	struct Reaction {
		sl::Array<Binding*> m_bindingArray;
		uint_t m_flags;

		Reaction() {
			m_flags = 0;
		}
	};

	struct PendingBinding {
		size_t m_reactionIdx;
		Multicast* m_event;

		PendingBinding() {
			m_reactionIdx = 0;
			m_event = NULL;
		}

		PendingBinding(
			size_t reactionIdx,
			Multicast* event
		) {
			m_reactionIdx = reactionIdx;
			m_event = event;
		}
	};

	typedef
	void
	ReactorFunc(
		Reactor* reactor,
		size_t index
	);

protected:
	State m_state;
	sl::AutoPtrArray<Reaction> m_reactionArray;
	sl::BitMap m_pendingReactionMap;
	sl::Array<PendingBinding> m_pendingOnChangedBindingArray;
	sl::Array<PendingBinding> m_pendingOnEventBindingArray;
	sl::List<Binding> m_bindingList;
	sl::Array<Reaction*> m_activeReactionArray;
	sl::Array<Binding*> m_activeBindingArray;
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
	addOnChangedBinding(
		size_t reactionIdx,
		Multicast* event
	);

	void
	JNC_CDECL
	addOnEventBinding(
		size_t reactionIdx,
		Multicast* event
	);

	void
	JNC_CDECL
	enterReactiveStmt(
		size_t reactionFromIdx,
		size_t reactionToIdx
	);

protected:
	void
	reactionLoop();

	void
	processPendingBindings();

	void
	onChanged(Binding* binding);

	static
	void
	onChangedThunk(ReactorClosure* closure) {
		((ReactorImpl*)closure->m_reactor)->onChanged((Binding*)closure->m_binding);
	}

	bool
	activateReaction(size_t reactionIdx);

	Binding*
	subscribe(Multicast* event);

	Binding*
	subscribe(
		Multicast* event,
		FunctionPtr functionPtr
	);

	void
	unsubscribe(Binding* binding);
};

//..............................................................................

} // namespace rtl
} // namespace jnc
