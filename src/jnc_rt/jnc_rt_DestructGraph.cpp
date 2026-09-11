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
#include "jnc_rt_DestructGraph.h"
#include "jnc_rt_GcHeap.h"
#include "jnc_ct_Type.h"

namespace jnc {
namespace rt {

//..............................................................................

DestructGraphNode*
DestructGraph::getNode(Box* box) {
	if (box->m_flags & BoxFlag_Index)
		return m_nodeArray[box->m_index];
	else if (box->m_flags & BoxFlag_Map)
		return m_nodeMap[box];

	size_t i = m_nodeArray.getCount();
	DestructGraphNode* node = new DestructGraphNode(box, i);
	m_nodeArray.append(node);

	if (i < BoxIndexLimit) {
		box->m_flags |= BoxFlag_Index;
		box->m_index = (uint32_t)i;
	} else {
		box->m_flags |= BoxFlag_Map;
		m_nodeMap[box] = node;
	}

	return node;
}

void
DestructGraph::build(
	const sl::ArrayRef<IfaceHdr*>& destructArray,
	rt::GcHeap* gcHeap
) {
	ASSERT(gcHeap->m_state == GcHeap::State_BuildDestructGraph);

	size_t count = destructArray.getCount();
	m_nodeArray.setCount(count);
	m_nodeMap.clear();
	m_unsortedEdgeArray.clear();

	sl::Array<DestructGraphNode*>::Rwi rwi = m_nodeArray.rwi();

	size_t i = 0;
	size_t indexedCount = AXL_MIN(count, BoxIndexLimit);
	for (; i < indexedCount; i++) {
		Box* box = destructArray[i]->m_box;
		rwi[i] = new DestructGraphNode(box, i);
		box->m_flags |= BoxFlag_Index;
		box->m_index = (uint32_t)i;
	}

	for (; i < count; i++) {
		Box* box = destructArray[i]->m_box;
		DestructGraphNode* node = new DestructGraphNode(box, i);
		rwi[i] = node;
		box->m_flags |= BoxFlag_Map;
		m_nodeMap[box] = node;
	}

	for (size_t i = 0; i < count; i++) {
		m_currentNode = m_nodeArray[i];
		gcHeap->markClass(m_currentNode->m_box);
	}

	gcHeap->runMarkCycle();

	size_t nodeCount = m_nodeArray.getCount();
	size_t edgeCount = m_unsortedEdgeArray.getCount();

	m_sortedEdgeArray.setCount(edgeCount);
	m_edgeBaseArray.setCount(nodeCount + 1);

	sl::countSort(
		m_sortedEdgeArray.p(),
		m_unsortedEdgeArray.cp(),
		edgeCount,
		m_edgeBaseArray.p(),
		nodeCount - 1
	);

	m_edgeBaseArray.rwi()[nodeCount] = edgeCount; // sentinel
	m_destructCount = count;
}

void
DestructGraph::cleanup() {
	// clear the flags we set on boxes

	size_t count = m_nodeArray.getCount();
	for (size_t i = 0; i < count; i++)
		m_nodeArray[i]->m_box->m_flags &= ~(BoxFlag_Index | BoxFlag_Map);

	// free nodes

	m_nodeArray.clear();
	m_nodeMap.clear();
	m_currentNode = NULL;

	// the rest is non-essential
}

// tarjan scc over the destruct graph builds a topological order of SCCs

void
DestructGraph::tarjan() {
	size_t nodeCount = m_nodeArray.getCount();
	size_t edgeCount = m_sortedEdgeArray.getCount();

	ASSERT(nodeCount >= m_destructCount);

	m_tarjanStack.clear();
	m_tarjanDfsStack.clear();
	m_tarjanSccNodeArray.clear();
	m_tarjanSccBaseArray.clear();

	size_t dfsCounter = 0;

	// seed from candidates in the original order to preserves the newest-first order we had before sorting

	for (size_t i = 0; i < m_destructCount; i++) {
		DestructGraphNode* rootNode = m_nodeArray[i];
		if (rootNode->m_tarjanIdx != -1)
			continue;

		rootNode->m_tarjanIdx = rootNode->m_tarjanLowlinkIdx = dfsCounter++;
		m_tarjanStack.append(i);

		// DFS

		TarjanDfsFrame rootFrame;
		rootFrame.m_nodeIdx = i;
		rootFrame.m_edgeIdx = m_edgeBaseArray[i];
		rootFrame.m_edgeEndIdx = m_edgeBaseArray[i + 1];
		m_tarjanDfsStack.append(rootFrame);

		while (!m_tarjanDfsStack.isEmpty()) {
			TarjanDfsFrame* frame = &m_tarjanDfsStack.getBack();
			DestructGraphNode* node = m_nodeArray[frame->m_nodeIdx];

			if (frame->m_edgeIdx < frame->m_edgeEndIdx) {
				size_t dstIdx = m_sortedEdgeArray[frame->m_edgeIdx].m_dstIdx;
				frame->m_edgeIdx++;

				DestructGraphNode* dstNode = m_nodeArray[dstIdx];
				if (dstNode->m_tarjanIdx != -1) {
					if (dstNode->m_tarjanSccIdx == -1) // still on the tarjan stack
						node->m_tarjanLowlinkIdx = AXL_MIN(node->m_tarjanLowlinkIdx, dstNode->m_tarjanIdx);

					continue;
				}

				dstNode->m_tarjanIdx = dstNode->m_tarjanLowlinkIdx = dfsCounter++;
				m_tarjanStack.append(dstIdx);

				TarjanDfsFrame nextFrame;
				nextFrame.m_nodeIdx = dstIdx;
				nextFrame.m_edgeIdx = m_edgeBaseArray[dstIdx];
				nextFrame.m_edgeEndIdx = m_edgeBaseArray[dstIdx + 1];
				m_tarjanDfsStack.append(nextFrame); // invalidates frame pointer
				continue;
			}

			// node is done

			if (node->m_tarjanLowlinkIdx == node->m_tarjanIdx) { // scc root -- pop the component
				size_t sccIdx = m_tarjanSccBaseArray.getCount();
				m_tarjanSccBaseArray.append(m_tarjanSccNodeArray.getCount());

				for (;;) {
					size_t j = m_tarjanStack.getBackAndPop();
					m_nodeArray[j]->m_tarjanSccIdx = sccIdx;
					m_tarjanSccNodeArray.append(j);

					if (j == node->m_index)
						break;
				}
			}

			m_tarjanDfsStack.pop();

			if (!m_tarjanDfsStack.isEmpty()) {
				DestructGraphNode* parent = m_nodeArray[m_tarjanDfsStack.getBack().m_nodeIdx];
				parent->m_tarjanLowlinkIdx = AXL_MIN(parent->m_tarjanLowlinkIdx, node->m_tarjanLowlinkIdx);
			}
		}
	}

	m_tarjanSccBaseArray.append(m_tarjanSccNodeArray.getCount()); // sentinel
}

// emits candidates in topological order
// SCC with more than one candidate is a loop -- order is unspecified (destruct LIFO, show warning)

void
DestructGraph::emit(sl::Array<IfaceHdr*>* destructArray) {
	size_t baseCount = destructArray->getCount();
	destructArray->setCount(baseCount + m_destructCount);

	IfaceHdr** dst = destructArray->p() + baseCount;
	for (intptr_t i = m_tarjanSccBaseArray.getCount() - 2; i >= 0; i--) { // -2 due to sentinel
		size_t base = m_tarjanSccBaseArray[i];
		size_t end = m_tarjanSccBaseArray[i + 1];

		// compat candidates

		size_t* p = m_tarjanSccNodeArray.p() + base;
		size_t* p0 = p;

		for (size_t j = base; j < end; j++) {
			size_t k = m_tarjanSccNodeArray[j];
			if (k < m_destructCount) // candidate
				*p++ = k;
		}

		size_t loopLength = p - p0;
		switch (loopLength) {
		case 0: // no candidates in this SCC
			continue;

		case 1: { // single candidate
			Box* box = m_nodeArray[*p0]->m_box;
			*dst++ = (IfaceHdr*)(box + 1);
			break;
			}

		default: // loop
			TRACE("-- WARNING: destruct loop length %d detected (fix topology with weak refs):\n", loopLength);

			std::sort(p0, p, sl::Gt<size_t>());

			for (; p0 < p; p0++) {
				Box* box = m_nodeArray[*p0]->m_box;
				*dst++ = (IfaceHdr*)(box + 1);
				TRACE("    %s\n", box->m_type->getTypeString().sz());
			}

			break;
		}
	}

	ASSERT(dst == destructArray->getEnd());
}

//..............................................................................

} // namespace rt
} // namespace jnc
