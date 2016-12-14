//========================================================================
//
// ServerUberObjectTemplate.h
//
//IMPORTANT: Any code between //@BEGIN TFD... and //@END TFD... will be
//overwritten the next time the template definition is compiled. Do not
//make changes to code inside these blocks.
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef _INCLUDED_ServerUberObjectTemplate_H
#define _INCLUDED_ServerUberObjectTemplate_H

#include "sharedTemplateDefinition/TpfTemplate.h"
#include "sharedFoundation/DynamicVariable.h"
#include "sharedTemplateDefinition/TpfTemplate.h"


class Vector;
typedef StructParam<ObjectTemplate> StructParamOT;


class ServerUberObjectTemplate : public TpfTemplate
{
public:
//@BEGIN TFD ID
	enum
	{
		ServerUberObjectTemplate_tag = TAG(S,W,U,T)
	};
//@END TFD ID
public:
	         ServerUberObjectTemplate(const std::string & filename);
	virtual ~ServerUberObjectTemplate();

	virtual Tag getId(void) const;
	virtual Tag getTemplateVersion(void) const;
	virtual Tag getHighestTemplateVersion(void) const;
	static void install(void);

//@BEGIN TFD
public:
	enum UpdateRanges
	{
		UR_near,
		UR_normal,
		UR_far,
		UpdateRanges_Last = UR_far,
	};

public:
	struct Foo
	{
		int                    item1;
		float                  item2;
		std::string            item3;
	};

protected:
	class _Foo : public TpfTemplate
	{
		friend class ServerUberObjectTemplate;
	public:
		enum
		{
			_Foo_tag = TAG(W,O,N,F)
		};

	public:
		         _Foo(const std::string & filename);
		virtual ~_Foo();

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
	CompilerIntegerParam m_item1;
	FloatParam m_item2;
	StringParam m_item3;
	private:
		static void registerMe(void);
		static ObjectTemplate * create(const std::string & filename);
	private:
		_Foo(const _Foo &);
		_Foo & operator =(const _Foo &);
	};
	friend class ServerUberObjectTemplate::_Foo;

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
	// Integer
	CompilerIntegerParam m_intAtDerived;
	CompilerIntegerParam m_intSimple;
	CompilerIntegerParam m_intSimpleDeltaPositive;
	CompilerIntegerParam m_intSimpleDeltaNegative;
	CompilerIntegerParam m_intSimpleDeltaPositivePercent;
	CompilerIntegerParam m_intSimpleDeltaNegativePercent;
	CompilerIntegerParam m_intWeightedList;
	CompilerIntegerParam m_intWeightedListDeltaPositive;
	CompilerIntegerParam m_intWeightedListDeltaNegative;
	CompilerIntegerParam m_intWeightedListDeltaPositivePercent;
	CompilerIntegerParam m_intWeightedListDeltaNegativePercent;
	CompilerIntegerParam m_intRandomRange1;
	CompilerIntegerParam m_intRandomRange2;
	CompilerIntegerParam m_intRandomRange3;
	CompilerIntegerParam m_intRandomRange4;
	CompilerIntegerParam m_intDiceRoll1;
	CompilerIntegerParam m_intDiceRoll2;
	std::vector<CompilerIntegerParam *> m_intListSimple;
	bool m_intListSimpleLoaded;
	bool m_intListSimpleAppend;
	std::vector<CompilerIntegerParam *> m_intListWeightedList;
	bool m_intListWeightedListLoaded;
	bool m_intListWeightedListAppend;
	std::vector<CompilerIntegerParam *> m_intListRandomRange;
	bool m_intListRandomRangeLoaded;
	bool m_intListRandomRangeAppend;
	std::vector<CompilerIntegerParam *> m_intListDiceRoll;
	bool m_intListDiceRollLoaded;
	bool m_intListDiceRollAppend;
	// Float
	FloatParam m_floatAtDerived;
	FloatParam m_floatSimple;
	FloatParam m_floatSimpleDeltaPositive;
	FloatParam m_floatSimpleDeltaNegative;
	FloatParam m_floatSimpleDeltaPositivePercent;
	FloatParam m_floatSimpleDeltaNegativePercent;
	FloatParam m_floatWeightedList;
	FloatParam m_floatRandomRange1;
	FloatParam m_floatRandomRange2;
	FloatParam m_floatRandomRange3;
	FloatParam m_floatRandomRange4;
	std::vector<FloatParam *> m_floatListSimple;
	bool m_floatListSimpleLoaded;
	bool m_floatListSimpleAppend;
	std::vector<FloatParam *> m_floatListWeightedList;
	bool m_floatListWeightedListLoaded;
	bool m_floatListWeightedListAppend;
	std::vector<FloatParam *> m_floatListRandomRange;
	bool m_floatListRandomRangeLoaded;
	bool m_floatListRandomRangeAppend;
	// Enum
	CompilerIntegerParam m_enumIndexedByEnumSingle[3];
	CompilerIntegerParam m_enumIndexedByEnumWeightedList[3];
	std::vector<CompilerIntegerParam *> m_enumListIndexed;
	bool m_enumListIndexedLoaded;
	bool m_enumListIndexedAppend;
	std::vector<CompilerIntegerParam *> m_enumListWeightedList;
	bool m_enumListWeightedListLoaded;
	bool m_enumListWeightedListAppend;
	// StringId
	StringIdParam m_stringIdDerived;
	StringIdParam m_stringIdSimple;
	StringIdParam m_stringIdWeightedList;
	std::vector<StringIdParam *> m_stringIdListSimple;
	bool m_stringIdListSimpleLoaded;
	bool m_stringIdListSimpleAppend;
	std::vector<StringIdParam *> m_stringIdListWeightedList;
	bool m_stringIdListWeightedListLoaded;
	bool m_stringIdListWeightedListAppend;
	// String
	StringParam m_stringDerived;
	StringParam m_stringSimple;
	StringParam m_stringWeightedList;
	std::vector<StringParam *> m_stringListSimple;
	bool m_stringListSimpleLoaded;
	bool m_stringListSimpleAppend;
	std::vector<StringParam *> m_stringListWeightedList;
	bool m_stringListWeightedListLoaded;
	bool m_stringListWeightedListAppend;
	// TRIGGER VOLUME
	TriggerVolumeParam m_triggerVolumeDerived;
	TriggerVolumeParam m_triggerVolumeSimple;
	TriggerVolumeParam m_triggerVolumeWeightedList;
	std::vector<TriggerVolumeParam *> m_triggerVolumeList;
	bool m_triggerVolumeListLoaded;
	bool m_triggerVolumeListAppend;
	std::vector<TriggerVolumeParam *> m_triggerVolumesListWeightedList;
	bool m_triggerVolumesListWeightedListLoaded;
	bool m_triggerVolumesListWeightedListAppend;
	// BOOL
	BoolParam m_boolDerived;
	BoolParam m_boolSimple;
	BoolParam m_boolWeightedList;
	std::vector<BoolParam *> m_boolListDerived;
	bool m_boolListDerivedLoaded;
	bool m_boolListDerivedAppend;
	std::vector<BoolParam *> m_boolListSimple;
	bool m_boolListSimpleLoaded;
	bool m_boolListSimpleAppend;
	std::vector<BoolParam *> m_boolListWeightedList;
	bool m_boolListWeightedListLoaded;
	bool m_boolListWeightedListAppend;
	///******
	//SAJ - I would suggest dumping the vector type, since no code uses it. It was created
	//to be used with spawning, but that's being done entirely in script.
	//
	//-Dan Ignoring this, if they break, whack em
	//******/
	// Vector
	VectorParam m_vectorAtDerived;
	VectorParam m_vectorSimple;
	// vector vectorSimpleDeltaPositive
	// vector vectorSimpleDeltaNegative
	// vector vectorSimpleDeltaPositivePercent
	// vector vectorSimpleDeltaNegativePercent
	// vector vectorWeightedList
	// vector vectorRandomRange
	std::vector<VectorParam *> m_vectorListSimple;
	bool m_vectorListSimpleLoaded;
	bool m_vectorListSimpleAppend;
	// Filename
	StringParam m_filenameAtDerived;
	StringParam m_filenameSimple;
	StringParam m_filenameWeightedList;
	std::vector<StringParam *> m_filenameListSimple;
	bool m_filenameListSimpleLoaded;
	bool m_filenameListSimpleAppend;
	// Objvar
	DynamicVariableParam m_objvarDerived;
	DynamicVariableParam m_objvarSimple;
	// objvar objvarsListAppend
	// Template
	StringParam m_templateDerived;
	StringParam m_templateSimple;
	StringParam m_templateWeightedList;
	std::vector<StringParam *> m_templateListSimple;
	bool m_templateListSimpleLoaded;
	bool m_templateListSimpleAppend;
	// Structs
	StructParamOT m_structAtDerived;
	StructParamOT m_structSimple;
	std::vector<StructParamOT *> m_structListSimple;
	bool m_structListSimpleLoaded;
	bool m_structListSimpleAppend;
	StructParamOT m_structArrayEnum[3];
	StructParamOT m_structArrayInteger[2];
	// Arrays
	CompilerIntegerParam m_integerArray[2];
	FloatParam m_floatArray[2];
	StringParam m_stringArray[2];
	BoolParam m_boolArray[2];
	StringIdParam m_stringIdArray[2];
	TriggerVolumeParam m_triggerArray[2];
	VectorParam m_vectorArray[2];
	StringParam m_fileNameArray[2];
//@END TFD

private:
	Tag  m_templateVersion;	// the template version
	bool m_versionOk;	// flag that the template version loaded is the one we expect

	static void registerMe(void);
	static ObjectTemplate * create(const std::string & filename);

	// no copying
	ServerUberObjectTemplate(const ServerUberObjectTemplate &);
	ServerUberObjectTemplate & operator =(const ServerUberObjectTemplate &);
};


inline void ServerUberObjectTemplate::install(void)
{
//@BEGIN TFD INSTALL
	ServerUberObjectTemplate::registerMe();
	ServerUberObjectTemplate::_Foo::registerMe();
//@END TFD INSTALL
}


#endif	// _INCLUDED_ServerUberObjectTemplate_H
