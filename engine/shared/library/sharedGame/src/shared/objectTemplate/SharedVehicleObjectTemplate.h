//========================================================================
//
// SharedVehicleObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedVehicleObjectTemplate_H
#define _INCLUDED_SharedVehicleObjectTemplate_H

#include "SharedTangibleObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedUtility/TemplateParameter.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;
//@BEGIN TFD TEMPLATE REFS
//@END TFD TEMPLATE REFS


class SharedVehicleObjectTemplate : public SharedTangibleObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedVehicleObjectTemplate_tag = TAG(S,V,O,T)
	};
//@END TFD ID
public:
	         SharedVehicleObjectTemplate(const std::string & filename);
	virtual ~SharedVehicleObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(bool allowDefaultTemplateParams = true);

//@BEGIN TFD
public:
	enum MovementTypes
	{
		MT_hover,
		MT_underwater,
		MT_ground,
		MT_swim,
		MT_walker,
		MovementTypes_Last = MT_walker,
	};

public:
	float                  getSpeed(MovementTypes index) const;
	float                  getSpeedMin(MovementTypes index) const;
	float                  getSpeedMax(MovementTypes index) const;
	float                  getSlopeAversion() const;
	float                  getSlopeAversionMin() const;
	float                  getSlopeAversionMax() const;
	float                  getHoverValue() const;
	float                  getHoverValueMin() const;
	float                  getHoverValueMax() const;
	float                  getTurnRate() const;
	float                  getTurnRateMin() const;
	float                  getTurnRateMax() const;
	float                  getMaxVelocity() const;
	float                  getMaxVelocityMin() const;
	float                  getMaxVelocityMax() const;
	float                  getAcceleration() const;
	float                  getAccelerationMin() const;
	float                  getAccelerationMax() const;
	float                  getBraking() const;
	float                  getBrakingMin() const;
	float                  getBrakingMax() const;


protected:
	virtual void load(Iff &file);

private:
	FloatParam m_speed[5];		// how fast the vehicle moves
	FloatParam m_slopeAversion;		// ?
	FloatParam m_hoverValue;		// height vehicle hovers at
	FloatParam m_turnRate;		// how tight the vehicle turns (?)
	FloatParam m_maxVelocity;		// max speed the vehicle can move
	FloatParam m_acceleration;		// vehicle acceleration
	FloatParam m_braking;		// vehicle braking
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect
	static bool ms_allowDefaultTemplateParams;	// flag to allow defaut params instead of fataling

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedVehicleObjectTemplate(const SharedVehicleObjectTemplate &);
	SharedVehicleObjectTemplate & operator =(const SharedVehicleObjectTemplate &);
};


inline void SharedVehicleObjectTemplate::install(bool allowDefaultTemplateParams)
{
	ms_allowDefaultTemplateParams = allowDefaultTemplateParams;
//@BEGIN TFD INSTALL
	SharedVehicleObjectTemplate::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedVehicleObjectTemplate_H
