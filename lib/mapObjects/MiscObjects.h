/*
 * MiscObjects.h, part of VCMI engine
 *
 * Authors: listed in file AUTHORS in main folder
 *
 * License: GNU General Public License v2.0 or later
 * Full text of license available in license.txt file, in main folder
 *
 */
#pragma once

#include "CArmedInstance.h"
#include "../MetaString.h"

VCMI_LIB_NAMESPACE_BEGIN

class CMap;

// This one teleport-specific, but has to be available everywhere in callbacks and netpacks
// For now it's will be there till teleports code refactored and moved into own file
using TTeleportExitsList = std::vector<std::pair<ObjectInstanceID, int3>>;

/// Legacy class, use CRewardableObject instead
class DLL_LINKAGE CTeamVisited: public CGObjectInstance
{
public:
	std::set<PlayerColor> players; //players that visited this object

	bool wasVisited (const CGHeroInstance * h) const override;
	bool wasVisited(PlayerColor player) const override;
	bool wasVisited(const TeamID & team) const;
	void setPropertyDer(ObjProperty what, ObjPropertyID identifier) override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
		h & players;
	}
};

class DLL_LINKAGE CGSignBottle : public CGObjectInstance //signs and ocean bottles
{
public:
	MetaString message;

	void onHeroVisit(const CGHeroInstance * h) const override;
	void initObj(CRandomGenerator & rand) override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
		h & message;
	}
protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGGarrison : public CArmedInstance
{
public:
	bool removableUnits;

	bool passableFor(PlayerColor color) const override;
	void onHeroVisit(const CGHeroInstance * h) const override;
	void battleFinished(const CGHeroInstance *hero, const BattleResult &result) const override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CArmedInstance&>(*this);
		h & removableUnits;
	}
protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGArtifact : public CArmedInstance
{
public:
	CArtifactInstance * storedArtifact = nullptr;
	MetaString message;

	void onHeroVisit(const CGHeroInstance * h) const override;
	void battleFinished(const CGHeroInstance *hero, const BattleResult &result) const override;
	void blockingDialogAnswered(const CGHeroInstance *hero, ui32 answer) const override;

	std::string getObjectName() const override;
	std::string getPopupText(PlayerColor player) const override;
	std::string getPopupText(const CGHeroInstance * hero) const override;
	std::vector<Component> getPopupComponents(PlayerColor player) const override;

	void pick( const CGHeroInstance * h ) const;
	void initObj(CRandomGenerator & rand) override;
	void pickRandomObject(CRandomGenerator & rand) override;

	void afterAddToMap(CMap * map) override;
	BattleField getBattlefield() const override;

	ArtifactID getArtifact() const;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CArmedInstance&>(*this);
		h & message;
		h & storedArtifact;
	}
protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGResource : public CArmedInstance
{
public:
	static constexpr ui32 RANDOM_AMOUNT = 0;
	ui32 amount = RANDOM_AMOUNT; //0 if random
	
	MetaString message;

	void onHeroVisit(const CGHeroInstance * h) const override;
	void initObj(CRandomGenerator & rand) override;
	void pickRandomObject(CRandomGenerator & rand) override;
	void battleFinished(const CGHeroInstance *hero, const BattleResult &result) const override;
	void blockingDialogAnswered(const CGHeroInstance *hero, ui32 answer) const override;
	std::string getHoverText(PlayerColor player) const override;

	void collectRes(const PlayerColor & player) const;
	GameResID resourceID() const;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CArmedInstance&>(*this);
		h & amount;
		h & message;
	}
protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGMine : public CArmedInstance
{
public:
	GameResID producedResource;
	ui32 producedQuantity;
	std::set<GameResID> abandonedMineResources;
	
	bool isAbandoned() const;
	ResourceSet dailyIncome() const;

private:
	void onHeroVisit(const CGHeroInstance * h) const override;
	void battleFinished(const CGHeroInstance *hero, const BattleResult &result) const override;
	void blockingDialogAnswered(const CGHeroInstance *hero, ui32 answer) const override;

	void flagMine(const PlayerColor & player) const;
	void newTurn(CRandomGenerator & rand) const override;
	void initObj(CRandomGenerator & rand) override;

	std::string getObjectName() const override;
	std::string getHoverText(PlayerColor player) const override;

public:
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CArmedInstance&>(*this);
		h & producedResource;
		h & producedQuantity;
		h & abandonedMineResources;
	}
	ui32 defaultResProduction() const;

protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

struct DLL_LINKAGE TeleportChannel
{
	enum EPassability {UNKNOWN, IMPASSABLE, PASSABLE};

	std::vector<ObjectInstanceID> entrances;
	std::vector<ObjectInstanceID> exits;
	EPassability passability = EPassability::UNKNOWN;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & entrances;
		h & exits;
		h & passability;
	}
};

class DLL_LINKAGE CGTeleport : public CGObjectInstance
{
	bool isChannelEntrance(const ObjectInstanceID & id) const;
	bool isChannelExit(const ObjectInstanceID & id) const;

	std::vector<ObjectInstanceID> getAllEntrances(bool excludeCurrent = false) const;

protected:
	enum EType {UNKNOWN, ENTRANCE, EXIT, BOTH};
	EType type = EType::UNKNOWN;

	ObjectInstanceID getRandomExit(const CGHeroInstance * h) const;
	std::vector<ObjectInstanceID> getAllExits(bool excludeCurrent = false) const;

public:
	TeleportChannelID channel;

	bool isEntrance() const;
	bool isExit() const;

	virtual void teleportDialogAnswered(const CGHeroInstance *hero, ui32 answer, TTeleportExitsList exits) const = 0;

	static bool isTeleport(const CGObjectInstance * dst);
	static bool isConnected(const CGTeleport * src, const CGTeleport * dst);
	static bool isConnected(const CGObjectInstance * src, const CGObjectInstance * dst);
	static void addToChannel(std::map<TeleportChannelID, std::shared_ptr<TeleportChannel> > &channelsList, const CGTeleport * obj);
	static std::vector<ObjectInstanceID> getPassableExits(CGameState * gs, const CGHeroInstance * h, std::vector<ObjectInstanceID> exits);
	static bool isExitPassable(CGameState * gs, const CGHeroInstance * h, const CGObjectInstance * obj);

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & type;
		h & channel;
		h & static_cast<CGObjectInstance&>(*this);
	}
};

class DLL_LINKAGE CGMonolith : public CGTeleport
{
	TeleportChannelID findMeChannel(const std::vector<Obj> & IDs, MapObjectSubID SubID) const;

protected:
	void onHeroVisit(const CGHeroInstance * h) const override;
	void teleportDialogAnswered(const CGHeroInstance *hero, ui32 answer, TTeleportExitsList exits) const override;
	void initObj(CRandomGenerator & rand) override;

public:
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGTeleport&>(*this);
	}
};

class DLL_LINKAGE CGSubterraneanGate : public CGMonolith
{
	void onHeroVisit(const CGHeroInstance * h) const override;
	void initObj(CRandomGenerator & rand) override;

public:
	static void postInit();

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGMonolith&>(*this);
	}
};

class DLL_LINKAGE CGWhirlpool : public CGMonolith
{
	void onHeroVisit(const CGHeroInstance * h) const override;
	void teleportDialogAnswered(const CGHeroInstance *hero, ui32 answer, TTeleportExitsList exits) const override;
	static bool isProtected( const CGHeroInstance * h );

public:
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGMonolith&>(*this);
	}
};

class DLL_LINKAGE CGSirens : public CGObjectInstance
{
public:
	void onHeroVisit(const CGHeroInstance * h) const override;
	std::string getHoverText(const CGHeroInstance * hero) const override;
	void initObj(CRandomGenerator & rand) override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
	}
};

class DLL_LINKAGE CGBoat : public CGObjectInstance, public CBonusSystemNode
{
public:
	ui8 direction;
	const CGHeroInstance *hero;  //hero on board
	bool onboardAssaultAllowed; //if true, hero can attack units from transport
	bool onboardVisitAllowed; //if true, hero can visit objects from transport
	EPathfindingLayer layer;
	
	//animation filenames. If empty - animations won't be used
	AnimationPath actualAnimation; //for OH3 boats those have actual animations
	AnimationPath overlayAnimation; //waves animations
	std::array<AnimationPath, PlayerColor::PLAYER_LIMIT_I> flagAnimations;

	CGBoat();
	bool isCoastVisitable() const override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
		h & static_cast<CBonusSystemNode&>(*this);
		h & direction;
		h & hero;
		h & layer;
		h & onboardAssaultAllowed;
		h & onboardVisitAllowed;
		h & actualAnimation;
		h & overlayAnimation;
		h & flagAnimations;
	}
};

class DLL_LINKAGE CGShipyard : public CGObjectInstance, public IShipyard
{
	friend class ShipyardInstanceConstructor;

	BoatId createdBoat;

protected:
	void getOutOffsets(std::vector<int3> & offsets) const override;
	void onHeroVisit(const CGHeroInstance * h) const override;
	const IObjectInterface * getObject() const override;
	BoatId getBoatType() const override;

public:
	template<typename Handler> void serialize(Handler & h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
		h & createdBoat;
	}

protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGMagi : public CGObjectInstance
{
public:
	void initObj(CRandomGenerator & rand) override;
	void onHeroVisit(const CGHeroInstance * h) const override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
	}
};

class DLL_LINKAGE CGDenOfthieves : public CGObjectInstance
{
	void onHeroVisit(const CGHeroInstance * h) const override;
};

class DLL_LINKAGE CGObelisk : public CTeamVisited
{
public:
	static ui8 obeliskCount; //how many obelisks are on map
	static std::map<TeamID, ui8> visited; //map: team_id => how many obelisks has been visited

	void onHeroVisit(const CGHeroInstance * h) const override;
	void initObj(CRandomGenerator & rand) override;
	std::string getHoverText(PlayerColor player) const override;
	static void reset();

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CTeamVisited&>(*this);
	}
protected:
	void setPropertyDer(ObjProperty what, ObjPropertyID identifier) override;
};

class DLL_LINKAGE CGLighthouse : public CGObjectInstance
{
public:
	void onHeroVisit(const CGHeroInstance * h) const override;
	void initObj(CRandomGenerator & rand) override;

	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
	}
	void giveBonusTo(const PlayerColor & player, bool onInit = false) const;

protected:
	void serializeJsonOptions(JsonSerializeFormat & handler) override;
};

class DLL_LINKAGE CGTerrainPatch : public CGObjectInstance
{
public:
	CGTerrainPatch() = default;

	virtual bool isTile2Terrain() const override
	{
		return true;
	}
};

class DLL_LINKAGE HillFort : public CGObjectInstance, public ICreatureUpgrader
{
	friend class HillFortInstanceConstructor;

	std::vector<int> upgradeCostPercentage;

protected:
	void onHeroVisit(const CGHeroInstance * h) const override;
	void fillUpgradeInfo(UpgradeInfo & info, const CStackInstance &stack) const override;

public:
	template <typename Handler> void serialize(Handler &h, const int version)
	{
		h & static_cast<CGObjectInstance&>(*this);
		h & upgradeCostPercentage;
	}
};

VCMI_LIB_NAMESPACE_END
