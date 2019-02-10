/**
 * @file
 */

#pragma once

#include "ui/turbobadger/Window.h"
#include "core/Common.h"

namespace frontend {

class DisconnectWindow: public ui::turbobadger::Window {
public:
	DisconnectWindow(Window* parent) :
			ui::turbobadger::Window(parent) {
		core_assert_always(loadResourceFile("ui/window/client-disconnect.tb.txt"));
	}

	bool onEvent(const tb::TBWidgetEvent &ev) override {
		if (ev.type == tb::EVENT_TYPE_CLICK && ev.target->getID() == TBIDC("ok")) {
			close();
			return true;
		}
		return ui::turbobadger::Window::onEvent(ev);
	}
};

}
