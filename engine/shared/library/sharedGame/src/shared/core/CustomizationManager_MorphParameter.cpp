//======================================================================
//
// CustomizationManager_MorphParameter.cpp
// copyright (c) 2002-2004 Sony Online Entertainment
//
//======================================================================

#include "sharedGame/FirstSharedGame.h"
#include "sharedGame/CustomizationManager_MorphParameter.h"

#include "sharedGame/SharedObjectTemplate.h"
#include "sharedMessageDispatch/Transceiver.h"
#include "sharedObject/CustomizationData.h"
#include "sharedObject/Object.h"
#include "sharedObject/RangedIntCustomizationVariable.h"

#include "Unicode.h"
#include "UnicodeUtils.h"

//======================================================================

namespace CustomizationManagerMorphParameterNamespace
{
	namespace Transceivers
	{
		MessageDispatch::Transceiver<CustomizationManager::MorphParameter::Messages::ScaleFactorChanged::Payload const &, CustomizationManager::MorphParameter::Messages::ScaleFactorChanged> scaleFactorChanged;
	}

	int ParseTwoTokens (Unicode::String const & str, Unicode::String tokens[])
	{
		assert (tokens);
		const size_t comma_pos = str.find_first_of (',');
		tokens [0] = str.substr (0, comma_pos);
		Unicode::trim (tokens [0]);
		if (comma_pos != Unicode::String::npos)
		{
			tokens [1] = str.substr (comma_pos+1);
			Unicode::trim (tokens [1]);
			return 2;
		}
		return 1;
	}
}

using namespace CustomizationManagerMorphParameterNamespace;

//----------------------------------------------------------------------

CustomizationManager::MorphParameter::MorphParameter (CustomizationData & cdata, CustomizationManager::Customization const & data)
: name(),
  varStartName(),
  varEndName(),
  varStart(0),
  varEnd(0),
  target(0.0f),
  current(0.0f),
  theDefault(0.0f),
  pictureIconName(),
  cameraYaw(0.0f),
  cameraYawValid(false),
  isScale(false),
  isDiscrete(false),
  discreteRange(100),
  reverse(false),
  randomizable(true),
  randomizableGroup(false)
{
	name = Unicode::narrowToWide(data.name);

	Unicode::String tokens [2];
	Unicode::String tmpStr = Unicode::narrowToWide(data.variables);
	{
		IGNORE_RETURN (ParseTwoTokens (tmpStr, tokens));

		varStartName = Unicode::wideToNarrow (tokens [0]);
		varEndName   = Unicode::wideToNarrow (tokens [1]);

		if (!varStartName.empty ())
			varStart     = dynamic_cast<RangedIntCustomizationVariable *>(CustomizationManager::findVariable (cdata, varStartName));

		if (!varEndName.empty ())
			varEnd       = dynamic_cast<RangedIntCustomizationVariable *>(CustomizationManager::findVariable (cdata, varEndName));
	}

	cameraYaw = data.cameraYaw;
	cameraYawValid = (cameraYaw != 0.0f) ? true: false;

	isScale = data.isScale;
	isDiscrete = data.discrete;
	reverse = data.reverse;
	randomizable = data.randomizable;
	randomizableGroup = data.randomizableGroup;

	if (isDiscrete && varStart)
	{
		int begin = 0;
		varStart->getRange (begin, discreteRange);
		--discreteRange;
	}

	//-- convert cameraYaw from degrees to radians
	if (cameraYawValid)
		cameraYaw = cameraYaw * PI / 180.0f;
}

//----------------------------------------------------------------------

void CustomizationManager::MorphParameter::debugPrint (std::string & result) const
{
	char buf [256];
	snprintf (buf, sizeof (buf), "%20s    ", Unicode::wideToNarrow (name).c_str ());
	result += buf;
	snprintf (buf, sizeof (buf), "%20s", varStartName.c_str ());
	result += buf;
	if (!varEndName.empty ())
	{
		snprintf (buf, sizeof (buf), ",%20s", varEndName.c_str ());
		result += buf;
	}

	result.append (1, '\n');
}

//----------------------------------------------------------------------

void CustomizationManager::MorphParameter::computeDefaults (Object const & obj, SharedObjectTemplate const * const tmpl)
{
	if (varStart)
	{
		current = varStart->getNormalizedFloatValue ();
		
		if (varEnd)
			current = (1.0f + varEnd->getNormalizedFloatValue () - current) * 0.5f;
	}
	else if (isScale)
	{
		current = normalizeScale (obj, tmpl);
	}
	else
		current = 0.0f;
	if (reverse)
		current = (1.0f - current); 
	theDefault = current;
	target     = current;
}

//----------------------------------------------------------------------

void CustomizationManager::MorphParameter::update (Object const & obj, float const value)
{
	float const localValue = clamp (0.0f, value, 1.0f);

	if (varStart)
	{
		float varValue = localValue;
		if (reverse)
			varValue = (1.0f - varValue);
		if (!varEnd)
		{
			varStart->setNormalizedFloatValue (varValue);
		}
		else
		{
			varStart->setNormalizedFloatValue (std::max (0.0f, (0.5f - varValue) * 2.0f));
			varEnd->setNormalizedFloatValue   (std::max (0.0f, (varValue - 0.5f) * 2.0f));
		}
	}
	else if (isScale)
	{
		Transceivers::scaleFactorChanged.emitMessage (std::make_pair(obj.getNetworkId(), value));
	}
	current = localValue;
}

//----------------------------------------------------------------------

bool CustomizationManager::MorphParameter::incrementTarget (Object & obj, float const increment)
{
	float const old = current;
	if (target > current)
		update (obj, std::min (target, current + increment));
	else if (target < current)
		update (obj, std::max (target, current - increment));
	return current != old || current != target;  //lint !e777 //testing floats
}

//----------------------------------------------------------------------

void CustomizationManager::MorphParameter::resetDefaults   (Object & obj, bool const interpolate)
{
	target = theDefault;
	if (!interpolate)
		update (obj, theDefault);
}

//----------------------------------------------------------------------

void CustomizationManager::MorphParameter::setTarget (float const t)
{
	target = t;
	target = 1.0f - target;
}

//----------------------------------------------------------------------

float CustomizationManager::MorphParameter::normalizeScale (Object const & obj, SharedObjectTemplate const * const tmpl)
{
	if (tmpl)
	{
		float const scaleMax   = tmpl->getScaleMax ();
		float const scaleMin   = tmpl->getScaleMin ();
		float const scaleRange = scaleMax - scaleMin;
		if (scaleRange > 0.0f)
		{
			return (obj.getScale ().y - scaleMin) / scaleRange;
		}
		DEBUG_WARNING (scaleMin == 0.0f || scaleMax == 0.0f, ("SharedObjectTemplate [%s] scalemin/max is set to %f/%f, this is probably not what you want.", tmpl->getName (), scaleMin, scaleMax));
	}
	return 1.0f;
}

//----------------------------------------------------------------------

float CustomizationManager::MorphParameter::denormalizeScale (SharedObjectTemplate const * const tmpl, float const normalizedValue)
{
	if (tmpl && normalizedValue <= 1.0f) // Cekis: Godzilla must die!
	{
		float const scaleMax   = tmpl->getScaleMax ();
		float const scaleMin   = tmpl->getScaleMin ();
		float const scaleRange = scaleMax - scaleMin;
		return scaleMin + (normalizedValue * scaleRange);
	}
	return 1.0f;
}

//======================================================================
