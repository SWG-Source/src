// ======================================================================
//
// XmlTreeNode.h
// Copyright 2004 Sony Online Entertainment, Inc.
// All Rights Reserved.
//
// ======================================================================

#ifndef INCLUDED_XmlTreeNode_H
#define INCLUDED_XmlTreeNode_H

#include<iostream>
#include<sstream>
// ======================================================================

struct _xmlNode;
typedef struct _xmlNode xmlNode;

// ======================================================================

class XmlTreeNode
{
public:

	explicit XmlTreeNode(xmlNode *treeNode);
	XmlTreeNode(XmlTreeNode const &rhs);

	explicit XmlTreeNode(const std::string &name);
	explicit XmlTreeNode(const std::string &name, const std::string &content);

	XmlTreeNode &operator =(const XmlTreeNode &rhs);

	bool  isNull() const;
	bool  isElement() const;
	bool  isAttribute() const;
	bool  isText() const;

	void  assertIsElement(char const *const elementName) const;

	char const  *getName() const;

	XmlTreeNode  getNextSiblingElementNode() const;
	XmlTreeNode  getFirstChildNode() const;
	XmlTreeNode  getFirstChildElementNode() const;
	XmlTreeNode  getFirstChildTextNode() const;

	//-- Functions to call on element nodes.
	XmlTreeNode  getElementAttributeValueNode(char const *attributeName, bool optional) const;
	XmlTreeNode  getChildNodeNamed(const char * name);
	XmlTreeNode  getChildElementNodeNamed(const char * name);
	XmlTreeNode  getTextChildNodeNamed(const char * name);
	
	XmlTreeNode  addChildNode(char const * name);
	template <class T> XmlTreeNode addChildTextNode(char const * name, const T &value);

	template <class T> void setElementAttribute(char const *attributeName, const T &value);

	void         getElementAttributeAsBool(char const *attributeName, bool &value) const;
	void         getElementAttributeAsFloat(char const *attributeName, float &value) const;
	void         getElementAttributeAsInt(char const *attributeName, int &value) const;
	void         getElementAttributeAsString(char const *attributeName, std::string &value) const;

	bool         getElementAttributeAsBool(char const *attributeName, bool &value, bool optional) const;
	bool         getElementAttributeAsFloat(char const *attributeName, float &value, bool optional) const;
	bool         getElementAttributeAsInt(char const *attributeName, int &value, bool optional) const;
	bool         getElementAttributeAsString(char const *attributeName, std::string &value, bool optional) const;

	//-- Functions to call on text nodes.
	char const *getTextValue() const;

private:

	// Disabled.
	XmlTreeNode();

	void addProperty( char const *attributeName, const std::string& value);
	void addChild(const XmlTreeNode &node);
private:

	xmlNode *m_treeNode;

};

// ======================================================================

inline XmlTreeNode::XmlTreeNode(xmlNode *treeNode) :
	m_treeNode(treeNode)
{
}

// ----------------------------------------------------------------------

inline XmlTreeNode::XmlTreeNode(XmlTreeNode const &rhs) :
	m_treeNode(rhs.m_treeNode) //lint !e1554 // Direct pointer copy.  This is okay, this class is simply a wrapper for one of these pointers.
{
}

// ----------------------------------------------------------------------

inline XmlTreeNode &XmlTreeNode::operator =(const XmlTreeNode &rhs)
{
	m_treeNode = rhs.m_treeNode;
	return *this;
}

// ----------------------------------------------------------------------

inline void XmlTreeNode::getElementAttributeAsBool(char const *attributeName, bool &value) const
{
	IGNORE_RETURN(getElementAttributeAsBool(attributeName, value, false));
}

// ----------------------------------------------------------------------

inline void XmlTreeNode::getElementAttributeAsFloat(char const *attributeName, float &value) const
{
	IGNORE_RETURN(getElementAttributeAsFloat(attributeName, value, false));
}

// ----------------------------------------------------------------------

inline void XmlTreeNode::getElementAttributeAsInt(char const *attributeName, int &value) const
{
	IGNORE_RETURN(getElementAttributeAsInt(attributeName, value, false));
}

// ----------------------------------------------------------------------

inline void XmlTreeNode::getElementAttributeAsString(char const *attributeName, std::string &value) const
{
	IGNORE_RETURN(getElementAttributeAsString(attributeName, value, false));
}

// ======================================================================

template<class T> void XmlTreeNode::setElementAttribute( char const *attributeName, const T &value)
{
	std::ostringstream oss;
	oss << value;
	addProperty(attributeName, oss.str());
}

// ----------------------------------------------------------------------

template <class T> XmlTreeNode XmlTreeNode::addChildTextNode(const char * name, const T &value)
{
	std::ostringstream oss;
	oss << value;
	XmlTreeNode node(name, oss.str());
	addChild( node );

	return node;
}

#endif
