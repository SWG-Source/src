//======================================================================
//
// CustomizationManager_MorphParameter.h
// copyright (c) 2002-2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_CustomizationManager_MorphParameter_H
#define INCLUDED_CustomizationManager_MorphParameter_H

//======================================================================

#include "sharedGame/CustomizationManager.h"
//#include "Unicode.h"

//----------------------------------------------------------------------

class CustomizationData;
class NetworkId;
class Object;
class RangedIntCustomizationVariable;
class SharedObjectTemplate;

//----------------------------------------------------------------------

struct CustomizationManager::MorphParameter
{
public:
	struct Messages
	{
		struct ScaleFactorChanged
		{
			typedef std::pair<NetworkId, float> Payload;
		};
	};

public:
	static float normalizeScale (Object const & obj, SharedObjectTemplate const * tmpl);
	static float denormalizeScale (SharedObjectTemplate const * tmpl, float normalizedValue);

public:
	Unicode::String name;
	std::string varStartName;
	std::string varEndName;
	RangedIntCustomizationVariable * varStart;
	RangedIntCustomizationVariable * varEnd;
	float target;
	float current;
	float theDefault;
	std::string pictureIconName;
	float cameraYaw;
	bool cameraYawValid;
	bool isScale;
	bool isDiscrete;
	int discreteRange;
	bool reverse;
	bool randomizable;
	bool randomizableGroup;

public:
	MorphParameter (CustomizationData & cdata, CustomizationManager::Customization const & data);
	void computeDefaults (Object const & obj, SharedObjectTemplate const * tmpl);
	void update (Object const & obj, float value);
	bool incrementTarget (Object & obj, float increment);
	void resetDefaults (Object & obj, bool interpolate);
	void setTarget (float target);
	void debugPrint (std::string & result) const;
};

//======================================================================

#endif
