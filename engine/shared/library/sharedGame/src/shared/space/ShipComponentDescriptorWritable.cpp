//======================================================================
//
// ShipComponentDescriptorWritable.cpp
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/ShipComponentDescriptorWritable.h"

#include "sharedFoundation/Crc.h"
#include "sharedMessageDispatch/Transceiver.h"

//======================================================================

namespace ShipComponentDescriptorWritableNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<bool const &, ShipComponentDescriptorWritable::Messages::ComponentListChanged> componentListChanged;
		MessageDispatch::Transceiver<ShipComponentDescriptorWritable::Messages::ComponentChanged::Payload const &, ShipComponentDescriptorWritable::Messages::ComponentChanged> componentChanged;
	}
}

using namespace ShipComponentDescriptorWritableNamespace;

//----------------------------------------------------------------------

ShipComponentDescriptorWritable::ShipComponentDescriptorWritable (CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, std::string const & objectTemplateName, std::string const & sharedTemplateName) :
ShipComponentDescriptor(name, type, compatibility, objectTemplateName, sharedTemplateName),
m_objectTemplateName(objectTemplateName),
m_sharedTemplateName(sharedTemplateName)
{
}

//----------------------------------------------------------------------

ShipComponentDescriptorWritable::~ShipComponentDescriptorWritable ()
{
}

//----------------------------------------------------------------------

std::string const & ShipComponentDescriptorWritable::getObjectTemplateName() const
{
	return m_objectTemplateName;
}

//----------------------------------------------------------------------

std::string const & ShipComponentDescriptorWritable::getSharedTemplateName() const
{
	return m_sharedTemplateName;
}

//----------------------------------------------------------------------

bool ShipComponentDescriptorWritable::setObjectTemplateByName(std::string const & objectTemplateName, std::string const & sharedObjectTemplateName)
{
	uint32 const crc = Crc::normalizeAndCalculate(objectTemplateName.c_str());
	uint32 const sharedCrc = Crc::normalizeAndCalculate(sharedObjectTemplateName.c_str());

	if (setObjectTemplateCrcs(crc, sharedCrc))
	{
		m_objectTemplateName = objectTemplateName;
		m_sharedTemplateName = sharedObjectTemplateName;
		notifyChanged();
		return true;
	}

	return false;
}

/*
//----------------------------------------------------------------------

void ShipComponentDescriptorWritable::setSharedTemplateName(std::string const & sharedTemplateName)
{
	m_sharedTemplateName = sharedTemplateName;
}
*/
//----------------------------------------------------------------------

bool ShipComponentDescriptorWritable::setName(std::string const & name)
{
	if (ShipComponentDescriptor::setName(name))
	{
		if (nullptr != ShipComponentDescriptor::findShipComponentDescriptorByCrc(Crc::normalizeAndCalculate(name.c_str())))
		{
			notifyChanged();
			Transceivers::componentListChanged.emitMessage(true);
		}

		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void ShipComponentDescriptorWritable::setCompatibility(std::string const & compat)
{
	ShipComponentDescriptor::setCompatibility(compat);
	notifyChanged();
}

//----------------------------------------------------------------------

void ShipComponentDescriptorWritable::notifyChanged()
{
	Transceivers::componentChanged.emitMessage(*this);
}

//----------------------------------------------------------------------

void ShipComponentDescriptorWritable::removeShipComponentDescriptor()
{
	ShipComponentDescriptor::removeShipComponentDescriptor();
	Transceivers::componentListChanged.emitMessage(true);
}

//----------------------------------------------------------------------

bool ShipComponentDescriptorWritable::addShipComponentDescriptor(bool checkValidity, bool strict)
{
	if (ShipComponentDescriptor::addShipComponentDescriptor(checkValidity, strict))
	{
		Transceivers::componentListChanged.emitMessage(true);
		return true;
	}
	return false;
}

//----------------------------------------------------------------------

void ShipComponentDescriptorWritable::setComponentType(ShipComponentType::Type type)
{
	ShipComponentDescriptor::setComponentType(type);
	Transceivers::componentListChanged.emitMessage(true);
}

//======================================================================
