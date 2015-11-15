//========================================================================
//
// CreatureController.h
//
// copyright 2001 Sony Online Entertainment
//
//========================================================================

#ifndef	INCLUDED_CreatureController_H
#define	INCLUDED_CreatureController_H

//-----------------------------------------------------------------------

#include "serverGame/TangibleController.h"
#include "swgSharedUtility/Behaviors.def"
#include "swgSharedUtility/MentalStates.def"
#include "swgSharedUtility/Locomotions.def"
#include "swgSharedUtility/Postures.def"
#include "swgSharedUtility/States.def"

//-----------------------------------------------------------------------

class AICreatureController;
class CellObject;
class CreatureObject;
class MessageQueueSecureTrade;
class MessageQueueCommandQueueEnqueue;
class MessageQueueCommandQueueEnqueueFwd;
class PlayerCreatureController;
class ServerSecureTrade;
class WeaponObject;

//-----------------------------------------------------------------------

class CreatureController : public TangibleController
{
public:
	explicit        CreatureController (CreatureObject * newOwner);
	               ~CreatureController ();

	static CreatureController * getCreatureController(NetworkId const & networkId);
	static CreatureController * getCreatureController(Object * object);

	static CreatureController const * asCreatureController(Controller const * controller);
	static CreatureController * asCreatureController(Controller * controller);

	virtual CreatureController * asCreatureController();
	virtual CreatureController const * asCreatureController() const;
	virtual PlayerCreatureController * asPlayerCreatureController();
	virtual PlayerCreatureController const * asPlayerCreatureController() const;
	virtual AICreatureController * asAiCreatureController();
	virtual AICreatureController const * asAiCreatureController() const;

	virtual void    conclude           ();

	/** object modification functions that should only be called on authoritative
	 *  objects
	 */
	bool            setSlopeModAngle(float angle);
	bool            setSlopeModPercent(float percent);
	bool            setWaterModPercent(float percent);
	bool            setMovementScale(float scale);
	bool            setMovementPercent(float percent);
	bool            setAccelScale(float scale);
	bool            setAccelPercent(float percent);

	void            setAppearanceFromObjectTemplate(std::string const &serverObjectTemplateFilename);

	virtual void    setObjectStopWalkRun();
	virtual void    setOwner(Object *newOwner);

	CreatureObject       * getCreature();
	CreatureObject const * getCreature() const;

	virtual void    endBaselines();
	        bool    getHibernate() const;
	virtual void    setHibernate(bool hibernate);
	virtual void    updateHibernate();

private:

	//-----------------------------------------------------------
	// Secure Trade methods
public:
	void                setSecureTrade(ServerSecureTrade *);
	ServerSecureTrade*  getSecureTrade() const;
	void                handleSecureTradeMessage(const MessageQueueSecureTrade * data);
	bool                tradeRequested(CreatureObject&);

private:
	ServerSecureTrade*          m_secureTrade;
	Watcher<CreatureObject>     m_secureTradeInitiator;

protected:
	virtual float realAlter(real time);
	virtual void handleMessage (int message, float value, const MessageQueue::Data* data, uint32 flags);

	virtual void steeringChanged();

	virtual bool    shouldHibernate() const;

protected:
	void         sendGenericResponse (int responseType, int requestType, bool success, uint8 sequenceId);

private:

	void            calculateWaterState(bool& isSwimming,bool& isBurning,float& lavaResistance, float& waterHeight) const;
	void            onEnterSwimming();
	void            onExitSwimming();

	// Disabled.
	CreatureController            (void);
	CreatureController            (const CreatureController & other);
	CreatureController&	operator= (const CreatureController & other);
};

//--------------------------------------------------------------------

#endif	// INCLUDED_CreatureController_H

