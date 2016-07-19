/**
 * @file
 */

#pragma once

#include "core/EventBus.h"
#include "core/Trace.h"
#include "network/Network.h"
#include "network/NetworkEvents.h"
#include "voxel/World.h"
#include "backend/spawn/SpawnMgr.h"
#include "core/Input.h"
#include "network/ProtocolHandlerRegistry.h"
#include "backend/entity/EntityStorage.h"
#include "backend/entity/ai/AIRegistry.h"
#include "core/EventBus.h"
#include "backend/entity/ai/AICommon.h"

#include <memory>
#include <thread>
#include <chrono>

namespace backend {

class ServerLoop: public core::IEventBusHandler<network::NewConnectionEvent>, core::IEventBusHandler<network::DisconnectEvent> {
private:
	network::NetworkPtr _network;
	SpawnMgrPtr _spawnMgr;
	voxel::WorldPtr _world;
	ai::Zone _zone;
	ai::Server _aiServer;
	EntityStoragePtr _entityStorage;
	core::EventBusPtr _eventBus;
	AIRegistryPtr _registry;
	attrib::ContainerProviderPtr _containerProvider;
	PoiProviderPtr _poiProvider;
	core::Input _input;

	void readInput();
public:
	ServerLoop(network::NetworkPtr network, SpawnMgrPtr spawnMgr, voxel::WorldPtr world, EntityStoragePtr entityStorage, core::EventBusPtr eventBus, AIRegistryPtr registry,
			attrib::ContainerProviderPtr containerProvider, PoiProviderPtr poiProvider);

	bool init();
	void shutdown();
	void onFrame(long dt);
	void onEvent(const network::DisconnectEvent& event);
	void onEvent(const network::NewConnectionEvent& event);
};

typedef std::shared_ptr<ServerLoop> ServerLoopPtr;

}
