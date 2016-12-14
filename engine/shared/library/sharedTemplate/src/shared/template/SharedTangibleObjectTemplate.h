//========================================================================
//
// SharedTangibleObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_SharedTangibleObjectTemplate_H
#define _INCLUDED_SharedTangibleObjectTemplate_H

#include "SharedObjectTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class SharedTangibleObjectTemplate : public SharedObjectTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		SharedTangibleObjectTemplate_tag = TAG(S,T,O,T)
	};
//@END TFD ID
public:
	         SharedTangibleObjectTemplate(const std::string & filename);
	virtual ~SharedTangibleObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum ClientVisabilityFlags
	{
		CVF_always,
		CVF_gm_only,
		ClientVisabilityFlags_Last = CVF_gm_only,
	};

public:
	struct RangedIntCustomizationVariable
	{
		std::string            variableName;
		int                    minValueInclusive;
		int                    defaultValue;
		int                    maxValueExclusive;
	};

	struct PaletteColorCustomizationVariable
	{
		std::string            variableName;
		std::string            palettePathName;
		int                    defaultPaletteIndex;
	};

	struct ConstStringCustomizationVariable
	{
		std::string            variableName;
		std::string            constValue;
	};

	struct CustomizationVariableMapping
	{
		std::string            sourceVariable;
		std::string            dependentVariable;
	};

protected:
	class _RangedIntCustomizationVariable : public TpfTemplate
	{
		friend class SharedTangibleObjectTemplate;
	public:
		enum
		{
			_RangedIntCustomizationVariable_tag = TAG(R,I,C,V)
		};

	public:
		         _RangedIntCustomizationVariable(const std::string & filename);
		virtual ~_RangedIntCustomizationVariable();

	virtual Tag getId(void) const;

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	StringParam m_variableName;
	CompilerIntegerParam m_minValueInclusive;
	CompilerIntegerParam m_defaultValue;
	CompilerIntegerParam m_maxValueExclusive;
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_RangedIntCustomizationVariable(const _RangedIntCustomizationVariable &);
		_RangedIntCustomizationVariable & operator =(const _RangedIntCustomizationVariable &);
	};
	friend class SharedTangibleObjectTemplate::_RangedIntCustomizationVariable;

	class _PaletteColorCustomizationVariable : public TpfTemplate
	{
		friend class SharedTangibleObjectTemplate;
	public:
		enum
		{
			_PaletteColorCustomizationVariable_tag = TAG(P,C,C,V)
		};

	public:
		         _PaletteColorCustomizationVariable(const std::string & filename);
		virtual ~_PaletteColorCustomizationVariable();

	virtual Tag getId(void) const;

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	StringParam m_variableName;
	StringParam m_palettePathName;
	CompilerIntegerParam m_defaultPaletteIndex;
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_PaletteColorCustomizationVariable(const _PaletteColorCustomizationVariable &);
		_PaletteColorCustomizationVariable & operator =(const _PaletteColorCustomizationVariable &);
	};
	friend class SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable;

	class _ConstStringCustomizationVariable : public TpfTemplate
	{
		friend class SharedTangibleObjectTemplate;
	public:
		enum
		{
			_ConstStringCustomizationVariable_tag = TAG(C,S,C,V)
		};

	public:
		         _ConstStringCustomizationVariable(const std::string & filename);
		virtual ~_ConstStringCustomizationVariable();

	virtual Tag getId(void) const;

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	StringParam m_variableName;
	StringParam m_constValue;
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_ConstStringCustomizationVariable(const _ConstStringCustomizationVariable &);
		_ConstStringCustomizationVariable & operator =(const _ConstStringCustomizationVariable &);
	};
	friend class SharedTangibleObjectTemplate::_ConstStringCustomizationVariable;

	class _CustomizationVariableMapping : public TpfTemplate
	{
		friend class SharedTangibleObjectTemplate;
	public:
		enum
		{
			_CustomizationVariableMapping_tag = TAG(C,V,M,M)
		};

	public:
		         _CustomizationVariableMapping(const std::string & filename);
		virtual ~_CustomizationVariableMapping();

	virtual Tag getId(void) const;

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	StringParam m_sourceVariable;
	StringParam m_dependentVariable;
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_CustomizationVariableMapping(const _CustomizationVariableMapping &);
		_CustomizationVariableMapping & operator =(const _CustomizationVariableMapping &);
	};
	friend class SharedTangibleObjectTemplate::_CustomizationVariableMapping;

public:
	virtual CompilerIntegerParam *getCompilerIntegerParam(const char *name, bool deepCheck = true, int index = 0);
	virtual FloatParam *getFloatParam(const char *name, bool deepCheck = true, int index = 0);
	virtual BoolParam *getBoolParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringParam *getStringParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StringIdParam *getStringIdParam(const char *name, bool deepCheck = true, int index = 0);
	virtual VectorParam *getVectorParam(const char *name, bool deepCheck = true, int index = 0);
	virtual DynamicVariableParam *getDynamicVariableParam(const char *name, bool deepCheck = true, int index = 0);
	virtual StructParamOT *getStructParamOT(const char *name, bool deepCheck = true, int index = 0);
	virtual TriggerVolumeParam *getTriggerVolumeParam(const char *name, bool deepCheck = true, int index = 0);
	virtual void initStructParamOT(StructParamOT &param, const char *name);
	virtual void setAsEmptyList(const char *name);
	virtual void setAppend(const char *name);
	virtual bool isAppend(const char *name) const;
	virtual int getListLength(const char *name) const;

protected:
	virtual void load(Iff &file);
	virtual void save(Iff &file);

private:
	std::vector<StructParamOT *> m_paletteColorCustomizationVariables;		// // all palette color customization variables exposed by an Object created with this template
	bool m_paletteColorCustomizationVariablesLoaded;
	bool m_paletteColorCustomizationVariablesAppend;
	std::vector<StructParamOT *> m_rangedIntCustomizationVariables;		// // all ranged-int style customization variables exposed by an Object created with this template
	bool m_rangedIntCustomizationVariablesLoaded;
	bool m_rangedIntCustomizationVariablesAppend;
	std::vector<StructParamOT *> m_constStringCustomizationVariables;		// // constant string values added to the Object's customization data, not persisted in the DB
	bool m_constStringCustomizationVariablesLoaded;
	bool m_constStringCustomizationVariablesAppend;
	std::vector<CompilerIntegerParam *> m_socketDestinations;		// // GOTs that this object can be socketed into
	bool m_socketDestinationsLoaded;
	bool m_socketDestinationsAppend;
	StringParam m_structureFootprintFileName;
	BoolParam m_useStructureFootprintOutline;
	BoolParam m_targetable;		// // can the object be targetted by the client
	std::vector<StringParam *> m_certificationsRequired;		// // List of the certifications required to use this item (used in x1 only)
	bool m_certificationsRequiredLoaded;
	bool m_certificationsRequiredAppend;
	std::vector<StructParamOT *> m_customizationVariableMapping;		// // Allows remapping of variables when needed
	bool m_customizationVariableMappingLoaded;
	bool m_customizationVariableMappingAppend;
	CompilerIntegerParam m_clientVisabilityFlag;		// // can the object be viewed on the client
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	SharedTangibleObjectTemplate(const SharedTangibleObjectTemplate &);
	SharedTangibleObjectTemplate & operator =(const SharedTangibleObjectTemplate &);
};


inline void SharedTangibleObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	SharedTangibleObjectTemplate::registerMe();
	SharedTangibleObjectTemplate::_ConstStringCustomizationVariable::registerMe();
	SharedTangibleObjectTemplate::_CustomizationVariableMapping::registerMe();
	SharedTangibleObjectTemplate::_PaletteColorCustomizationVariable::registerMe();
	SharedTangibleObjectTemplate::_RangedIntCustomizationVariable::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_SharedTangibleObjectTemplate_H
