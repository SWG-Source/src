//======================================================================
//
// NebulaManager.h
// copyright (c) 2004 Sony Online Entertainment
//
//======================================================================

#ifndef INCLUDED_NebulaManager_H
#define INCLUDED_NebulaManager_H

//======================================================================

class Nebula;
class Vector;

//----------------------------------------------------------------------

class NebulaManager
{
public:

	typedef void (*ImplementationClearFunction) (void);

	typedef std::vector<Nebula const *> NebulaVector;

	static void install();
	static void remove();
	static void setClearFunction(ImplementationClearFunction func);

	static void clear();
	static void loadSceneData(std::string const & sceneId);
	static bool hasSceneData(std::string const & sceneId);
	static void loadScene(std::string const & sceneId);

	static NebulaVector const & getNebulaVector(std::string const & sceneId);
	static NebulaVector const & getNebulaVector();

	static void getNebulasAtPosition(Vector const & pos, NebulaVector & nebulaVector);
	static void getNebulasInSphere(Vector const & pos, float radius, NebulaVector & nebulaVector);
	static Nebula const * getClosestNebula(Vector const & pos, float maxDistance, float & outMinDistance, float & outMaxDistance);
	static Nebula const * getNebulaById(int id);

private:

	NebulaManager(NebulaManager const & rhs);
	NebulaManager & operator=(NebulaManager const & rhs);

	static void addNebula(Nebula const & nebula);
};

//======================================================================

#endif
