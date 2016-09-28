#include "pch.h"
#include "jnc_ct_Alias.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

bool
Alias::generateDocumentation (
	const char* outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	bool isMulticast = isClassType (m_type, ClassTypeKind_Multicast);

	itemXml->format ("<memberdef kind='alias' id='%s'", getDoxyBlock ()->getRefId ().cc ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat (" prot='%s'", getAccessKindString (m_accessKind));

	itemXml->appendFormat (">\n<name>%s</name>\n", m_name.cc ());
	itemXml->append (m_type->getDoxyTypeString ());
 
	ASSERT (!m_initializer.isEmpty ());
	itemXml->appendFormat (
		"<initializer>= %s</initializer>\n", 
		getInitializerString ().cc ()
		);

	itemXml->append (getDoxyBlock ()->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</memberdef>\n");

	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
