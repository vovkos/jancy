#include "pch.h"
#include "jnc_ct_Alias.h"
#include "jnc_ct_Module.h"

namespace jnc {
namespace ct {

//.............................................................................

bool
Alias::generateDocumentation (
	const sl::StringRef& outputDir,
	sl::String* itemXml,
	sl::String* indexXml
	)
{
	bool isMulticast = isClassType (m_type, ClassTypeKind_Multicast);

	itemXml->format ("<memberdef kind='alias' id='%s'", getDoxyBlock ()->getRefId ().sz ());

	if (m_accessKind != AccessKind_Public)
		itemXml->appendFormat (" prot='%s'", getAccessKindString (m_accessKind));

	itemXml->appendFormat (">\n<name>%s</name>\n", m_name.sz ());
	itemXml->append (m_type->getDoxyTypeString ());
 
	ASSERT (!m_initializer.isEmpty ());
	itemXml->appendFormat (
		"<initializer>= %s</initializer>\n", 
		getInitializerString ().sz ()
		);

	itemXml->append (getDoxyBlock ()->getDescriptionString ());
	itemXml->append (getDoxyLocationString ());
	itemXml->append ("</memberdef>\n");

	return true;
}

//.............................................................................

} // namespace ct
} // namespace jnc
