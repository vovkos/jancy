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

namespace jnc {
namespace rt {

// we do our best to sort destructibles in a topological order

//..............................................................................

struct DestructGraphNode {
	Box* m_box;
	size_t m_index;

	size_t m_tarjanIdx;        // -1 -- not visited yet
	size_t m_tarjanLowlinkIdx;
	size_t m_tarjanSccIdx;     // -1 -- still on the tarjan stack

	DestructGraphNode(
		Box* box,
		size_t index
	);
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DestructGraphNode::DestructGraphNode(
	Box* box,
	size_t index
) {
	m_box = box;
	m_index = index;
	m_tarjanIdx = -1;
	m_tarjanLowlinkIdx = -1;
	m_tarjanSccIdx = -1;
}

//..............................................................................

class DestructGraph {
protected:
	struct Edge {
		size_t m_srcIdx;
		size_t m_dstIdx;

		Edge() {
			m_srcIdx = m_dstIdx = 0;
		}

		Edge(
			size_t srcIdx,
			size_t dstIdx
		) {
			m_srcIdx = srcIdx;
			m_dstIdx = dstIdx;
		}

		operator size_t() const {
			return m_srcIdx; // sort by the source index for tarjan
		}
	};

	// an explicit stack -- dfs depth can be the depth of the whole garbage graph

	struct TarjanDfsFrame {
		size_t m_nodeIdx;
		size_t m_edgeIdx;
		size_t m_edgeEndIdx;
	};

protected:
	// destruct graph (built via marking during State_BuildDestructGraph)

	sl::AutoPtrArray<DestructGraphNode> m_nodeArray;
	sl::SimpleHashTable<Box*, DestructGraphNode*> m_nodeMap; // backup for box index overflow
	sl::Array<Edge> m_unsortedEdgeArray; // edges are added in the order of discovery...
	sl::Array<Edge> m_sortedEdgeArray; // ...then sorted by source idx
	sl::Array<size_t> m_edgeBaseArray; // where the node's edges start in sorted edge array
	size_t m_destructCount;

	// tarjan over destruct graph

	sl::Array<size_t> m_tarjanStack;  // nodes discovered but not yet assigned to a SCC
	sl::Array<TarjanDfsFrame> m_tarjanDfsStack; // explicit DFS stack instead of recursion
	sl::Array<size_t> m_tarjanSccNodeArray; // all nodes, grouped by SCC, in pop order
	sl::Array<size_t> m_tarjanSccBaseArray; // where each SCC starts in sccNodeArray

public:
	DestructGraphNode* m_currentNode;

public:
	DestructGraph() {
		m_currentNode = NULL;
		m_destructCount = 0;
	}

	DestructGraphNode*
	getNode(Box* box);

	void
	build(
		const sl::ArrayRef<IfaceHdr*>& destructArray,
		GcHeap* gcHeap // build via marking
	);

	void
	tarjan();

	void
	emit(sl::Array<IfaceHdr*>* destructArray);

	void
	cleanup();

	DestructGraphNode*
	addEdge(Box* box) {
		return addEdge(m_currentNode, box);
	}

	DestructGraphNode*
	addEdge(
		DestructGraphNode* parent,
		Box* box
	);

	void
	addEdge(
		DestructGraphNode* srcNode,
		DestructGraphNode* dstNode
	) {
		if (srcNode != dstNode) // skip self-references
			m_unsortedEdgeArray.append(Edge(srcNode->m_index, dstNode->m_index));
	}
};

// . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . . .

inline
DestructGraphNode*
DestructGraph::addEdge(
	DestructGraphNode* parent,
	Box* box
) {
	DestructGraphNode* node = getNode(box);
	addEdge(parent, node);
	return node;
}

//..............................................................................

} // namespace rt
} // namespace jnc
