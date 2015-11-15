// ======================================================================
//
// InteriorLayoutReaderWriter.cpp
// asommers
//
// copyright 2002, sony online entertainment
//
// ======================================================================

#include "sharedUtility/FirstSharedUtility.h"
#include "sharedUtility/InteriorLayoutReaderWriter.h"

#include "sharedFile/Iff.h"
#include "sharedFoundation/LessPointerComparator.h"
#include "sharedFoundation/PersistentCrcString.h"
#include "sharedFoundation/TemporaryCrcString.h"
#include "sharedFoundation/PointerDeleter.h"
#include "sharedMath/Transform.h"

#include <algorithm>
#include <map>
#include <vector>

// ======================================================================

namespace InteriorLayoutReaderWriterNamespace
{
	const Tag TAG_INLY = TAG(I,N,L,Y);
	const Tag TAG_NODE = TAG(N,O,D,E);
};

using namespace InteriorLayoutReaderWriterNamespace;

// ======================================================================
// InteriorLayoutReaderWriter::Node
// ======================================================================

class InteriorLayoutReaderWriter::Node
{
public:

	explicit Node(char const * cellName);
	~Node();

	CrcString const & getCellName() const;

	int getNumberOfObjects() const;
	CrcString const & getObjectTemplateName(int index) const;
	Transform const & getTransform_o2p(int index) const;

	void addObject(char const * objectTemplateName, Transform const & transform_o2p);

private:

	Node();
	Node(const Node&);
	Node & operator=(const Node&);

private:

	PersistentCrcString const m_cellName;
	typedef std::vector<std::pair<PersistentCrcString, Transform> > ObjectVector;
	ObjectVector m_objectVector;
};

// ======================================================================
// PUBLIC InteriorLayoutReaderWriter::Node
// ======================================================================

InteriorLayoutReaderWriter::Node::Node(char const * const cellName) :
	m_cellName(cellName, true),
	m_objectVector()
{
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter::Node::~Node()
{
}

// ----------------------------------------------------------------------

CrcString const & InteriorLayoutReaderWriter::Node::getCellName() const
{
	return m_cellName;
}

// ----------------------------------------------------------------------

int InteriorLayoutReaderWriter::Node::getNumberOfObjects() const
{
	return static_cast<int>(m_objectVector.size());
}

// ----------------------------------------------------------------------

CrcString const & InteriorLayoutReaderWriter::Node::getObjectTemplateName(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects());

	return m_objectVector[static_cast<size_t>(index)].first;
}

// ----------------------------------------------------------------------

Transform const & InteriorLayoutReaderWriter::Node::getTransform_o2p(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects());

	return m_objectVector[static_cast<size_t>(index)].second;
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::Node::addObject(char const * const objectTemplateName, Transform const & transform_o2p)
{
	FATAL(!objectTemplateName, (""));
	m_objectVector.push_back(std::make_pair(PersistentCrcString(objectTemplateName, true), transform_o2p));
}

// ======================================================================
// PUBLIC InteriorLayoutReaderWriter
// ======================================================================

InteriorLayoutReaderWriter::InteriorLayoutReaderWriter() :
	m_nodeMap(new NodeMap),
	m_numberOfObjects(0)
{
}

// ----------------------------------------------------------------------

InteriorLayoutReaderWriter::~InteriorLayoutReaderWriter()
{
	clear();

	delete m_nodeMap;
}

// ----------------------------------------------------------------------

bool InteriorLayoutReaderWriter::load(const char* const fileName)
{
	clear();

	Iff iff;
	if (iff.open(fileName, true))
	{
		load(iff);

		return true;
	}

	return false;
}

// ----------------------------------------------------------------------

bool InteriorLayoutReaderWriter::save(const char* const fileName) const
{
	Iff iff(1024);
	save(iff);
	return iff.write(fileName, true);
}

// ----------------------------------------------------------------------

CrcString const & InteriorLayoutReaderWriter::getFileName() const
{
	return m_fileName;
}

// ----------------------------------------------------------------------

int InteriorLayoutReaderWriter::getNumberOfCellNames() const
{
	return static_cast<int>(m_nodeMap->size());
}

// ----------------------------------------------------------------------

CrcString const & InteriorLayoutReaderWriter::getCellName(int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfCellNames());

	NodeMap::iterator iter = m_nodeMap->begin();
	for (int i = 0; i < index; ++i, ++iter)
		;

	return *iter->first;
}

// ----------------------------------------------------------------------

bool InteriorLayoutReaderWriter::hasCellName(CrcString const & cellName) const
{
	return m_nodeMap->find(&cellName) != m_nodeMap->end();
}

// ----------------------------------------------------------------------

int InteriorLayoutReaderWriter::getNumberOfObjects(CrcString const & cellName) const
{
	NodeMap::iterator iter = m_nodeMap->find(&cellName);
	if (iter != m_nodeMap->end())
		return static_cast<int>(iter->second->getNumberOfObjects());
	
	return 0;
}

// ----------------------------------------------------------------------

CrcString const & InteriorLayoutReaderWriter::getObjectTemplateName(CrcString const & cellName, int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects(cellName));

	NodeMap::iterator iter = m_nodeMap->find(&cellName);
	if (iter != m_nodeMap->end())
		return iter->second->getObjectTemplateName(index);

	DEBUG_FATAL(true, ("InteriorLayoutReaderWriter::getObjectTemplateName: %s does not have any objects in cells named %s", m_fileName.getString(), cellName.getString()));
	return PersistentCrcString::empty;
}

// ----------------------------------------------------------------------

Transform const & InteriorLayoutReaderWriter::getTransform_o2p(CrcString const & cellName, int const index) const
{
	VALIDATE_RANGE_INCLUSIVE_EXCLUSIVE(0, index, getNumberOfObjects(cellName));

	NodeMap::iterator iter = m_nodeMap->find(&cellName);
	if (iter != m_nodeMap->end())
		return iter->second->getTransform_o2p(index);

	DEBUG_FATAL(true, ("InteriorLayoutReaderWriter::getTransform_o2p: %s does not have any objects in cells named %s", m_fileName.getString(), cellName.getString()));
	return Transform::identity;
}

// ----------------------------------------------------------------------

int InteriorLayoutReaderWriter::getNumberOfObjects() const
{
	return m_numberOfObjects;
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::getObjectTemplateNames(CrcStringVector & crcStringVector)
{
	crcStringVector.reserve(static_cast<size_t>(m_numberOfObjects));
	for (NodeMap::iterator iter = m_nodeMap->begin(); iter != m_nodeMap->end(); ++iter)
	{
		Node const * const node = iter->second;
		for (int i = 0; i < node->getNumberOfObjects(); ++i)
		{
			CrcString const & objectTemplateName = node->getObjectTemplateName(i);
			size_t j = 0;
			for (; j < crcStringVector.size(); ++j)
				if (*crcStringVector[j] == objectTemplateName)
					break;

			if (j == crcStringVector.size())
				crcStringVector.push_back(&objectTemplateName);
		}
	}
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::clear()
{
	std::for_each(m_nodeMap->begin(), m_nodeMap->end(), PointerDeleterPairSecond());
	m_nodeMap->clear();
	m_numberOfObjects = 0;
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::addObject(char const * const cellName, char const * const objectTemplateName, Transform const & transform_o2p)
{
	Node * node = 0;
	TemporaryCrcString const cellNameCrcString(cellName, true);
	NodeMap::iterator iter = m_nodeMap->find((const CrcString*)&cellNameCrcString);
	if (iter != m_nodeMap->end())
		node = iter->second;
	else
	{
		node = new Node(cellName);
		m_nodeMap->insert(std::make_pair(&node->getCellName(), node));
	}

	NOT_NULL(node);
	node->addObject(objectTemplateName, transform_o2p);

	++m_numberOfObjects;
}

// ======================================================================
// PUBLIC InteriorLayoutReaderWriter
// ======================================================================

void InteriorLayoutReaderWriter::load(Iff& iff)
{
	if (iff.enterForm(TAG_INLY, true))
	{
		switch(iff.getCurrentName())
		{
		case TAG_0000:
			load_0000(iff);
			break;

		default:
			{
				char tagBuffer [5];
				ConvertTagToString(iff.getCurrentName(), tagBuffer);

				char buffer [128];
				iff.formatLocation(buffer, sizeof(buffer));
				DEBUG_FATAL(true,("InteriorLayoutReaderWriter::load invalid version %s/%s", buffer, tagBuffer));
			}
			break;
		}

		iff.exitForm(TAG_INLY);
	}
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::load_0000(Iff& iff)
{
	iff.enterForm(TAG_0000);

		char objectTemplateName [512];
		char cellName [512];

		while (iff.getNumberOfBlocksLeft())
		{
			iff.enterChunk(TAG_NODE);

				iff.read_string(objectTemplateName, 512);
				iff.read_string(cellName, 512);
				Transform const transform_o2p = iff.read_floatTransform();

				addObject(cellName, objectTemplateName, transform_o2p);

			iff.exitChunk(TAG_NODE);
		}

	iff.exitForm(TAG_0000);
}

// ----------------------------------------------------------------------

void InteriorLayoutReaderWriter::save(Iff& iff) const
{
	iff.insertForm(TAG_INLY);
		iff.insertForm(TAG_0000);

		for (NodeMap::iterator iter = m_nodeMap->begin(); iter != m_nodeMap->end(); ++iter)
		{
			Node const * const node = iter->second;
			for (int i = 0; i < node->getNumberOfObjects(); ++i)
			{
				iff.insertChunk(TAG_NODE);

					iff.insertChunkString(node->getObjectTemplateName(i).getString());
					iff.insertChunkString(node->getCellName().getString());
					iff.insertChunkFloatTransform(node->getTransform_o2p(i));

				iff.exitChunk(TAG_NODE);
			}
		}

		iff.exitForm(TAG_0000);
	iff.exitForm(TAG_INLY);
}

// ======================================================================

