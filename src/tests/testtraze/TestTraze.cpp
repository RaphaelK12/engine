/**
 * @file
 */
#include "TestTraze.h"
#include "io/Filesystem.h"
#include "core/command/Command.h"
#include "voxel/MaterialColor.h"
#include "voxel/polyvox/Region.h"

namespace {
const int PlayFieldVolume = 0;
const int FontSize = 24;
}

TestTraze::TestTraze(const metric::MetricPtr& metric, const io::FilesystemPtr& filesystem, const core::EventBusPtr& eventBus, const core::TimeProviderPtr& timeProvider) :
		Super(metric, filesystem, eventBus, timeProvider), _protocol(eventBus), _voxelFontRender(FontSize) {
	init(ORGANISATION, "testtraze");
	setRenderAxis(false);
	_eventBus->subscribe<traze::NewGridEvent>(*this);
	_eventBus->subscribe<traze::NewGamesEvent>(*this);
	_eventBus->subscribe<traze::PlayerListEvent>(*this);
	_eventBus->subscribe<traze::TickerEvent>(*this);
	_eventBus->subscribe<traze::SpawnEvent>(*this);
	_eventBus->subscribe<traze::BikeEvent>(*this);
}

core::AppState TestTraze::onConstruct() {
	core::AppState state = Super::onConstruct();
	core::Var::get("mosquitto_host", "traze.iteratec.de");
	core::Var::get("mosquitto_port", "1883");
	_name = core::Var::get("name", "noname_testtraze");
	core::Command::registerCommand("join", [&] (const core::CmdArgs& args) { _protocol.join(_name->strVal()); });
	core::Command::registerCommand("bail", [&] (const core::CmdArgs& args) { _protocol.bail(); });
	core::Command::registerCommand("left", [&] (const core::CmdArgs& args) { _protocol.steer(traze::BikeDirection::W); });
	core::Command::registerCommand("right", [&] (const core::CmdArgs& args) { _protocol.steer(traze::BikeDirection::E); });
	core::Command::registerCommand("forward", [&] (const core::CmdArgs& args) { _protocol.steer(traze::BikeDirection::N); });
	core::Command::registerCommand("backward", [&] (const core::CmdArgs& args) { _protocol.steer(traze::BikeDirection::S); });
	core::Command::registerCommand("players", [&] (const core::CmdArgs& args) {
		for (const auto& p : _players) {
			Log::info("%s", p.name.c_str());
		}
	});
	return state;
}

core::AppState TestTraze::onInit() {
	core::AppState state = Super::onInit();
	if (state != core::AppState::Running) {
		return state;
	}
	if (!voxel::initDefaultMaterialColors()) {
		Log::error("Failed to initialize the palette data");
		return core::AppState::InitFailure;
	}
	if (!_protocol.init()) {
		Log::error("Failed to init protocol");
		return core::AppState::InitFailure;
	}
	if (!_rawVolumeRenderer.init()) {
		Log::error("Failed to initialize the raw volume renderer");
		return core::AppState::InitFailure;
	}
	if (!_rawVolumeRenderer.onResize(glm::ivec2(0), dimension())) {
		Log::error("Failed to initialize the raw volume renderer");
		return core::AppState::InitFailure;
	}
	if (!_voxelFontRender.init()) {
		Log::error("Failed to init voxel font");
		return core::AppState::InitFailure;
	}

	_camera.setPosition(glm::vec3(0.0f, 50.0f, 84.0f));
	_logLevelVar->setVal(std::to_string(SDL_LOG_PRIORITY_INFO));
	Log::init();

	return state;
}

void TestTraze::onEvent(const traze::NewGamesEvent& event) {
	_games = event.get();
	Log::debug("Got %i games", (int)_games.size());
	// there are some points were we assume a limited amount of games...
	if (_games.size() >= UCHAR_MAX) {
		Log::warn("Too many games found - reducing them");
		_games.resize(UCHAR_MAX - 1);
	}
	// TODO: this doesn't work if the instanceName changed (new game added, old game removed...)
	if (_games.empty() || _currentGameIndex > (int8_t)_games.size()) {
		_protocol.unsubscribe();
		_currentGameIndex = -1;
	} else if (!_games.empty() && _currentGameIndex == -1) {
		Log::info("Select first game");
		_currentGameIndex = 0;
	}
}

void TestTraze::onEvent(const traze::BikeEvent& event) {
	const traze::Bike& bike = event.get();
	Log::debug("Received bike event for player %u", bike.playerId);
}

void TestTraze::onEvent(const traze::TickerEvent& event) {
	const traze::Ticker& ticker = event.get();
	switch (ticker.type) {
	case traze::TickerType::Frag:
		Log::info("Received frag event");
		break;
	case traze::TickerType::Suicide:
		Log::info("Received suicide event");
		break;
	default:
		break;
	}
}

void TestTraze::onEvent(const traze::SpawnEvent& event) {
	const glm::ivec2& position = event.get();
	Log::info("Spawn at position %i:%i", position.x, position.y);
}

void TestTraze::onEvent(const traze::NewGridEvent& event) {
	voxel::RawVolume* v = event.get();
	delete _rawVolumeRenderer.setVolume(PlayFieldVolume, v);
	const glm::mat4& translate = glm::translate(-v->region().getCentre());
	const glm::mat4& rotateY = glm::rotate(glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	const glm::mat4& rotateX = glm::rotate(glm::radians(25.0f), glm::vec3(1.0f, 0.0f, 0.0f));
	_rawVolumeRenderer.setModelMatrix(PlayFieldVolume, rotateX * rotateY * translate);
	if (!_rawVolumeRenderer.extract(PlayFieldVolume)) {
		Log::error("Failed to extract the volume");
	}
}

void TestTraze::onEvent(const traze::PlayerListEvent& event) {
	_players = event.get();
}

core::AppState TestTraze::onRunning() {
	const int remaining = _eventBus->update(2);
	if (remaining) {
		Log::debug("Remaining events in queue: %i", remaining);
	}
	core::AppState state = Super::onRunning();
	if (_currentGameIndex != -1) {
		_protocol.subscribe(_games[_currentGameIndex]);
	}
	return state;
}

core::AppState TestTraze::onCleanup() {
	core::AppState state = Super::onCleanup();
	_voxelFontRender.shutdown();
	const std::vector<voxel::RawVolume*>& old = _rawVolumeRenderer.shutdown();
	for (voxel::RawVolume* v : old) {
		delete v;
	}
	_protocol.shutdown();
	return state;
}

void TestTraze::onRenderUI() {
	if (ImGui::BeginCombo("GameInfo", _currentGameIndex == -1 ? "" : _games[_currentGameIndex].name.c_str(), 0)) {
		for (size_t i = 0u; i < (size_t)_games.size(); ++i) {
			const traze::GameInfo& game = _games[i];
			const bool selected = _currentGameIndex == (int)i;
			if (ImGui::Selectable(game.name.c_str(), selected)) {
				_currentGameIndex = i;
			}
			if (selected) {
				ImGui::SetItemDefaultFocus();
			}
		}
		ImGui::EndCombo();
	}
	ImGui::InputVarString("Name", _name);
	if (!_protocol.joined() && ImGui::Button("Join")) {
		_protocol.join(_name->strVal());
	}
	if (_protocol.joined() && ImGui::Button("Leave")) {
		_protocol.bail();
	}
	ImGui::Checkbox("Render board", &_renderBoard);
	ImGui::Checkbox("Render player names", &_renderPlayerNames);
	Super::onRenderUI();
}

void TestTraze::doRender() {
	if (_renderBoard) {
		_rawVolumeRenderer.render(_camera);
	}

	if (_renderPlayerNames) {
		int yOffset = 0;
		_voxelFontRender.setViewProjectionMatrix(_camera.viewProjectionMatrix());
		const glm::mat4& namesTranslate = glm::translate(glm::vec3(0.0f, 0.0f, -100.0f));
		glm::mat4 model = glm::scale(namesTranslate, glm::vec3(0.2f));
		_voxelFontRender.setModelMatrix(model);
		for (const traze::Player& p : _players) {
			const std::string& frags = core::string::format("%i", p.frags);
			_voxelFontRender.text(p.name.c_str(), glm::ivec3(0, yOffset, 0), p.color);
			_voxelFontRender.text(frags.c_str(), glm::ivec3(200, yOffset, 0), p.color);
			yOffset += FontSize;
		}
		_voxelFontRender.render();
	}
}

TEST_APP(TestTraze)
