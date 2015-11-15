//======================================================================
//
// ShipComponentDescriptorWritable.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_ShipComponentDescriptorWritable_H
#define INCLUDED_ShipComponentDescriptorWritable_H

//======================================================================

#include "sharedGame/ShipComponentDescriptor.h"

class ShipComponentDescriptorWritable : public ShipComponentDescriptor
{
public:

	class Messages
	{
	public:
		struct ComponentListChanged;
		struct ComponentChanged
		{
			typedef ShipComponentDescriptorWritable Payload;
		};
	};

public:
	ShipComponentDescriptorWritable (CrcString const & name, ShipComponentType::Type type, CrcString const & compatibility, std::string const & objectTemplateName, std::string const & sharedTemplateName);
	~ShipComponentDescriptorWritable ();

	virtual std::string const & getObjectTemplateName() const;
	virtual std::string const & getSharedTemplateName() const;

	bool setObjectTemplateByName(std::string const & objectTemplateName, std::string const & sharedObjectTemplateName);
//	void setSharedTemplateName(std::string const & sharedTemplateName);

	virtual bool setName(std::string const & name);
	virtual void setCompatibility(std::string const & compat);
	virtual void removeShipComponentDescriptor();
	virtual bool addShipComponentDescriptor(bool checkValidity, bool strict);
	virtual void setComponentType(ShipComponentType::Type type);

	void notifyChanged();

private:

	ShipComponentDescriptorWritable(ShipComponentDescriptorWritable const & rhs);
	ShipComponentDescriptorWritable & operator=(ShipComponentDescriptorWritable const & rhs);

private:

	std::string m_objectTemplateName;
	std::string m_sharedTemplateName;
};

//======================================================================

#endif
