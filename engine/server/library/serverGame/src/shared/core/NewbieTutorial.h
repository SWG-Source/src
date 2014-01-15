// ======================================================================
//
// NewbieTutorial.h
//
// Copyright 2002 Sony Online Entertainment
//
// ======================================================================

#ifndef _NewbieTutorial_H_
#define _NewbieTutorial_H_

// ======================================================================

// ======================================================================

class Container;
class Vector;
class ServerObject;
class CreatureObject;

// ======================================================================

class NewbieTutorial // static
{
public:
	static std::string const &getSceneId();
	static std::string const &getTutorialTemplateName();
	static ServerObject *createTutorial(Vector const &location);

	static Vector const &getStartCoords();
	static std::string getStartCellName();
	static Vector getTutorialLocation();

	static void setupCharacterForTutorial(ServerObject* character);
	static void setupCharacterToSkipTutorial(ServerObject* character);
	static bool shouldStartTutorial(const ServerObject* character);

	static bool isInTutorial(const ServerObject* character);
	static bool isInTutorialArea(const ServerObject* character);

	static void stripNonFreeAssetsFromPlayerInTutorial(const CreatureObject* character);
private:
	NewbieTutorial();
	NewbieTutorial(NewbieTutorial const &);
	NewbieTutorial const &operator=(NewbieTutorial const &);

	static bool isFreeTrialScene(const std::string &scene);
	static bool isInFreeTrialBuildoutArea(const std::string &scene, const Vector &location);
	static void getNonFreeObjectsForDeletion(const Container* const container, std::vector<ServerObject *>& objectsToDelete, const CreatureObject* character);
};

// ======================================================================

#endif // _NewbieTutorial_H_

