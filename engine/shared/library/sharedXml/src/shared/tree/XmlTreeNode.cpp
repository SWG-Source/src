// ======================================================================
//
// XmlTreeNode.cpp
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#include "sharedXml/FirstSharedXml.h"
#include "sharedXml/XmlTreeNode.h"

#include "libxml/tree.h"

#include <string>
// ======================================================================

namespace XmlTreeNodeNamespace
{
	std::string  s_attributeValue;
}

using namespace XmlTreeNodeNamespace;

// ======================================================================

XmlTreeNode::XmlTreeNode(const std::string &name, const std::string &content) :
	m_treeNode(0)
{
	std::string nameCopy = name;
	std::string contentCopy = content;
	// verify parameters
	DEBUG_FATAL(!name.length(), ("Attempt to construct new XmlTreeNode but no name given"));
	if (!name.length())
	{
		nameCopy = "garbage";
	}

	DEBUG_FATAL(!content.length(), ("Attempt to construct new XmlTreeNode with text, but no text sent"));
	if (!content.length())
	{
		contentCopy = "garbage";
	}

	xmlNode *textNode = xmlNewText(BAD_CAST contentCopy.c_str() );
	m_treeNode = xmlNewNode(nullptr, BAD_CAST nameCopy.c_str());
	
	// verify new nodes
	DEBUG_FATAL(!textNode, ("Failed to create xml text node"));
	DEBUG_FATAL(!m_treeNode, ("Failed to create new xml tree node"));
	if ((!m_treeNode) || (!textNode))
	{
		m_treeNode = 0;
		return;
	}

	// add text node to the "real" one.
	xmlAddChild(m_treeNode,textNode);
}

// ----------------------------------------------------------------------

XmlTreeNode::XmlTreeNode(const std::string &name) :
	m_treeNode(0)
{
	std::string nameCopy = name;
	// verify parameters
	DEBUG_FATAL(!name.length(), ("Attempt to construct new XmlTreeNode but no name given"));
	if(!name.length())
	{
		nameCopy = "garbage";
	}

	m_treeNode = xmlNewNode(nullptr, BAD_CAST nameCopy.c_str());

	// verify new nodes
	DEBUG_FATAL(!m_treeNode, ("Failed to create new xml tree node"));
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::addChildNode(const char * name)
{
	DEBUG_FATAL( !m_treeNode, ("Attempted to add child to nullptr xml node"));

	XmlTreeNode node(name);
	if (m_treeNode)
	{
		addChild(node);
	}
	
	return node;
}

// ----------------------------------------------------------------------

void XmlTreeNode::addProperty(char const *attributeName, const std::string& value)
{
	xmlNewProp(m_treeNode, BAD_CAST attributeName, BAD_CAST value.c_str());
}

// ----------------------------------------------------------------------

void XmlTreeNode::addChild( const XmlTreeNode& node )
{
	xmlAddChild(m_treeNode, node.m_treeNode);
}

// ----------------------------------------------------------------------

bool XmlTreeNode::isNull() const
{
	return (m_treeNode == nullptr);
}

// ----------------------------------------------------------------------

bool XmlTreeNode::isElement() const
{
	return (m_treeNode && (m_treeNode->type == XML_ELEMENT_NODE));
}

// ----------------------------------------------------------------------

bool XmlTreeNode::isAttribute() const
{
	return (m_treeNode && (m_treeNode->type == XML_ATTRIBUTE_NODE));
}

// ----------------------------------------------------------------------

bool XmlTreeNode::isText() const
{
	return (m_treeNode && (m_treeNode->type == XML_TEXT_NODE));
}

// ----------------------------------------------------------------------

void XmlTreeNode::assertIsElement(char const *const elementName) const
{
	char const *const nodeName = getName();
	UNREF(elementName);
	UNREF(nodeName);
	DEBUG_FATAL(!isElement(), ("expecting element named [%s], found non-element entity named [%s].", elementName ? elementName : "<nullptr element name>", nodeName));
	DEBUG_FATAL(_stricmp(elementName, nodeName), ("expecting element named [%s], found element named [%s] instead.", nodeName));
}

// ----------------------------------------------------------------------

char const *XmlTreeNode::getName() const
{
	return (m_treeNode ? reinterpret_cast<char const*>(m_treeNode->name) : "<nullptr tree node>");
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getNextSiblingElementNode() const
{
	xmlNode *siblingNode = m_treeNode ? m_treeNode->next : nullptr;
	while (siblingNode && (siblingNode->type != XML_ELEMENT_NODE))
		siblingNode = siblingNode->next;

	return XmlTreeNode(siblingNode);
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getFirstChildNode() const
{
	return XmlTreeNode(m_treeNode ? m_treeNode->children : nullptr);
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getFirstChildElementNode() const
{
	//-- Check if we have a nullptr node.
	if (!m_treeNode)
		return XmlTreeNode(nullptr);

	//-- Look for the first child node that is an element.
	xmlNode *childNode = m_treeNode->children;
	while (childNode && (childNode->type != XML_ELEMENT_NODE))
		childNode = childNode->next;

	// Either we found a child that is an element node or we don't have a child element node.
	return XmlTreeNode(childNode);
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getFirstChildTextNode() const
{
	//-- Check if we have a nullptr node.
	if (!m_treeNode)
		return XmlTreeNode(nullptr);

	//-- Look for the first child node that is a text node.
	xmlNode *childNode = m_treeNode->children;
	while (childNode && (childNode->type != XML_TEXT_NODE))
		childNode = childNode->next;

	// Either we found a child that is an element node or we don't have a child element node.
	return XmlTreeNode(childNode);
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getElementAttributeValueNode(char const *attributeName, bool optional) const
{
	UNREF(optional);

	//-- Validate parameters and preconditions.
	if (!isElement())
	{
        DEBUG_FATAL(!optional, ("getElementAttributeValueNode(): node [%s] is not an element.", m_treeNode ? reinterpret_cast<char const*>(m_treeNode->name) : "<nullptr node>"));
		return XmlTreeNode(nullptr);
	}

	if (!attributeName)
	{
		DEBUG_FATAL(!optional, ("getElementAttributeValueNode(): attributeName is nullptr."));
		return XmlTreeNode(nullptr);
	}

	//-- Check the attribute nodes for a match on the given name.  Ignore case.
	for (xmlAttr *attributeNode = m_treeNode ? m_treeNode->properties : nullptr; attributeNode; attributeNode = attributeNode->next)
	{
		if (!_stricmp(attributeName, reinterpret_cast<char const*>(attributeNode->name)))
		{
			// Return the first child node of the matching attribute.  This should be the first (and probably only) value node.
			return XmlTreeNode(attributeNode->children);
		}
	}

	//-- No attribute node matched the attribute name.
	DEBUG_FATAL(!optional, ("getElementAttributeValueNode(): failed to find attribute [%s] on element node [%s].", attributeName, getName()));
	return XmlTreeNode(nullptr);
}

// ----------------------------------------------------------------------

bool XmlTreeNode::getElementAttributeAsBool(char const *attributeName, bool &value, bool optional) const
{
	//-- Get the attribute node.
	XmlTreeNode const attributeValueNode(getElementAttributeValueNode(attributeName, optional));
	if (attributeValueNode.isNull())
		return false;

	//-- Get the value as text.
	char const *const contents = attributeValueNode.getTextValue();
	if (!contents)
	{
		DEBUG_FATAL(!contents, ("contents of attribute [%s] is nullptr.", attributeName));
		return false; //lint !e527 // unreachable // reachable in release.
	}

	//-- Consider "true" or "1" as true, anything else as false.
	value = (contents && (!_stricmp(contents, "true") || !strcmp(contents, "1")));
	return true;
}

// ----------------------------------------------------------------------

bool XmlTreeNode::getElementAttributeAsFloat(char const *attributeName, float &value, bool optional) const
{
	//-- Get the attribute node.
	XmlTreeNode const attributeValueNode(getElementAttributeValueNode(attributeName, optional));
	if (attributeValueNode.isNull())
	{
		DEBUG_FATAL(!optional, ("Attribute [%s] does not exist.", attributeName));
		return false;
	}

	//-- Get the value as text.
	char const *const contents = attributeValueNode.getTextValue();
	if (!contents)
	{
		DEBUG_FATAL(!contents, ("contents of attribute [%s] is nullptr.", attributeName));
		return false; //lint !e527 // unreachable // reachable in release.
	}

	//-- Consider "true" or "1" as true, anything else as false.
	int const scanCount = sscanf(contents, "%f", &value);
	if (scanCount != 1)
	{
		DEBUG_FATAL(!optional, ("could not convert [%s] into a float", contents));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool XmlTreeNode::getElementAttributeAsInt(char const *attributeName, int &value, bool optional) const
{
	//-- Get the attribute node.
	XmlTreeNode const attributeValueNode(getElementAttributeValueNode(attributeName, optional));
	if (attributeValueNode.isNull())
	{
		DEBUG_FATAL(!optional, ("Attribute [%s] does not exist.", attributeName));
		return false;
	}

	//-- Get the value as text.
	char const *const contents = attributeValueNode.getTextValue();
	if (!contents)
	{
		DEBUG_FATAL(!contents, ("contents of attribute [%s] is nullptr.", attributeName));
		return false; //lint !e527 // unreachable // reachable in release.
	}

	//-- Consider "true" or "1" as true, anything else as false.
	int const scanCount = sscanf(contents, "%d", &value);
	if (scanCount != 1)
	{
		DEBUG_FATAL(!optional, ("could not convert [%s] into an integer", contents));
		return false;
	}

	return true;
}

// ----------------------------------------------------------------------

bool XmlTreeNode::getElementAttributeAsString(char const *attributeName, std::string &value, bool optional) const
{
	//-- Get the attribute node.
	XmlTreeNode const attributeValueNode(getElementAttributeValueNode(attributeName, optional));
	if (attributeValueNode.isNull())
		return false;

	//-- Get the value as text.
	value = attributeValueNode.getTextValue();
	return true;
}

// ----------------------------------------------------------------------

// Note that this method is somewhat limited.  It's not going to handle embedded elements
// within a text node well.  Specifically, a node that looks like:
// <parent>this is<embedded/> some text</parent>
// would most likely return the text "this is", if called on the parent of the text.
// If this becomes an issue, you'll either need to manually iterate
// over the nodes contained by the parent, calling getTextValue() on
// each one that's a text node, or expand this method to do so for you, in cases
// where it is called on a parent of a text node.
char const *XmlTreeNode::getTextValue() const
{
	XmlTreeNode node = *this;

	// if we're not a text node, see if one of our children is.  If so, it's pretty clear
	// what was intended, and we should Do The Right Thing.
	if(!isText())
	{
		// to compile, we have to call this off of our constructed node.
		// we're a const method, and getChildNodeNamed() is not.  However,
		// in reality it's the xmlNode contained by us that does the changing,
		// not ourself.
		//
		// getChildNodeNamed() should probably be turned to const, but to be safe I'm
		// leaving it so that nothing breaks.
		node = node.getChildNodeNamed("text");
	}
	
	//-- Ensure we're a text node.
	if(!node.isText())
	{
		DEBUG_FATAL(true, ("node [%s] is not a text node", m_treeNode ? reinterpret_cast<char const*>(m_treeNode->name) : "<nullptr>"));
		return nullptr; //lint !e527 // unreachable // reachable in release.
	}

	//-- Return contents.
	return node.m_treeNode ? reinterpret_cast<char const*>(node.m_treeNode->content) : nullptr;
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getChildNodeNamed(const char * name)
{
	XmlTreeNode node = getFirstChildNode();

	while(!node.isNull() && _stricmp(node.getName(), name))
		node = node.getNextSiblingElementNode();

	return node;
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getChildElementNodeNamed(const char * name)
{
	XmlTreeNode node = getFirstChildNode();

	while(!node.isNull() && (!node.isElement() || _stricmp(node.getName(), name)))
		node = node.getNextSiblingElementNode();

	return node;
}

// ----------------------------------------------------------------------

XmlTreeNode XmlTreeNode::getTextChildNodeNamed(const char * name)
{
	XmlTreeNode node = getChildNodeNamed(name);

	if(!node.isNull())
		node = node.getChildNodeNamed("text");

	return node;
}

// ======================================================================
