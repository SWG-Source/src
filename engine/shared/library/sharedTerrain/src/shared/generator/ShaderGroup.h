//===================================================================
//
// ShaderGroup.h
// asommers 9-17-2000
//
// copyright 2000, verant interactive
//
//--
//
// A ShaderGroup contains lists of families of textures organized by id. 
// A shader family is a list of valid shaders for a specified id. The shader
// family also stores the name of the shader, its shaderTemplate, and 
// the actual shader. The shader group will return const shaders for use
// with the terrain system. In the event that requested shader does not
// exist, the shader group will return a [clearly visible] default shader.
// Families will only create shaders when they are chosen.
//
//===================================================================

#ifndef SHADERGROUP_H
#define SHADERGROUP_H

//===================================================================

#include "sharedFoundation/ArrayList.h"
#include "sharedMath/PackedRgb.h"

class Iff;
class ObjectTemplate;

//===================================================================

class ShaderGroup
{
public:

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// Info holds info about a shader and its priority
	//
	struct Info
	{
	public:

		Info ();
		~Info ();

		int   getPriority () const;
		void  setPriority (int priority);
		int   getFamilyId () const;
		void  setFamilyId (int familyId);
		float getChildChoice () const;
		void  setChildChoice (float childChoice);

	private:

		uint8 m_priority;
		uint8 m_familyId;
		uint8 m_childChoice;
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	//
	// FamilyChildData allows creation of ShaderGroups
	//
	struct FamilyChildData
	{
	public:

		int         familyId;
		const char* shaderTemplateName;
		float       weight;

	public:

		FamilyChildData ();
		~FamilyChildData ();
	};

	//- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

private:

	class Family;

private:

	//-- list of families
	ArrayList<Family*> familyList;

	//--
	PackedRgb errorFamilyColor;

private:

	int                   findFamilyIndex (int familyId) const;
	int                   findFamilyIndex (const PackedRgb& desiredColor) const;

private:

	void                  load_0000 (Iff& iff);
	void                  load_0001 (Iff& iff);
	void                  load_0002 (Iff& iff);
	void                  load_0003 (Iff& iff);
	void                  load_0004 (Iff& iff);
	void                  load_0005 (Iff& iff);
	void                  load_0006 (Iff& iff);

private:

	ShaderGroup (const ShaderGroup& rhs);
	ShaderGroup& operator= (const ShaderGroup& rhs);

public:

	ShaderGroup ();
	~ShaderGroup ();

	const Info            getDefaultShader () const;
	const Info            chooseShader (int familyId) const;
	const Info            chooseShader (const PackedRgb& desiredColor) const;

	int                   createShader (const ShaderGroup::Info& sgi) const;

	//-- creation routines (for editor)
	void                  load (Iff& iff);
	void                  save (Iff& iff) const;
	void                  removeAllFamilies ();
	void                  loadSurfaceProperties ();

	//-- family id routines
	void                  addFamily (int familyId, const char* name, const PackedRgb& color);
	void                  removeFamily (int familyId);
	void                  addChild (const FamilyChildData& familyChildData);
	void                  removeChild (const FamilyChildData& familyChildData);
	bool                  hasFamily (int familyId) const;
	bool                  hasFamily (const char* name) const;

	int                   getFamilyId (const PackedRgb& desiredColor) const;
	int                   getFamilyId (const char* name) const;
	const char*           getFamilyName (int familyId) const;
	void                  setFamilyName (int familyId, const char* name);
	const PackedRgb&      getFamilyColor (int familyId) const;
	void                  setFamilyColor (int familyId, const PackedRgb& prgb);
	float                 getFamilyShaderSize (int familyId) const;
	void                  setFamilyShaderSize (int familyId, float meters);
	int                   getFamilyNumberOfChildren (int familyId) const;
	void                  renameChild (int familyId, const char* oldName, const char* newName);
	void                  promoteFamily (int familyId);
	void                  demoteFamily (int familyId);
	void                  promoteChild (int familyId, const char* childName);
	void                  demoteChild (int familyId, const char* childName);
	const FamilyChildData getFamilyChild (int familyId, const char* childName) const;
	const FamilyChildData getFamilyChild (int familyId, int childIndex) const;
	void                  setFamilyChild (int familyId, const FamilyChildData& familyChildData);
	float                 getFamilyFeatherClamp (int familyId) const;
	void                  setFamilyFeatherClamp (int familyId, float featherClamp);
	const char*           getFamilySurfacePropertiesName (int familyId) const;
	void                  setFamilySurfacePropertiesName (int familyId, const char* familySurfacePropertiesName);
	const ObjectTemplate* getFamilySurfaceProperties (int familyId) const;

	//-- family index routines
	int                   getNumberOfFamilies () const;
	int                   getNumberOfChildren (int familyIndex) const;
	const FamilyChildData getChild (int familyIndex, int childIndex) const;
	int                   getFamilyId (int familyIndex) const;
	void                  setFamilyId (int familyIndex, int newId);
};

//===================================================================

#endif
