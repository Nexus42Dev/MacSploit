#include <ixwebsocket/IXNetSystem.h>
#include <ixwebsocket/IXWebSocket.h>

int sendData(uint64_t rl);
int closeSocket(uint64_t rl);

class websocketClass {
public:
	uint64_t th = 0;
	bool connected = false;
	ix::WebSocket* webSocket;
	int onMessageRef;
    int threadRef;
	int onCloseRef;

	void fireMessage(std::string msg) {
		if (!settings::get_boolean("debugLibrary")) return;
		if (!roblox_thread || teleport_event) {
			connected = false;
			return;
		}

		rbx_getref(th, onMessageRef);
		(*rbx_getfield)(th, -1, "Fire");

        (*rbx_insert)(th, -2);
        rbx_pop(th, 1);

		if (!roblox_thread || teleport_event) return;

		rbx_getref(th, onMessageRef);
		rbx_pushstring(th, msg);
		(*rbx_pcall)(th);
		(*rbx_settop)(th, 0);
	}

	void fireClose() {
		connected = false;
		if (!roblox_thread || teleport_event) return;
		if (!settings::get_boolean("debugLibrary")) return;
		rbx_getref(th, onCloseRef);

		(*rbx_getfield)(th, -1, "Fire");
        (*rbx_insert)(th, -2);
        rbx_pop(th, 1);

		rbx_getref(th, onCloseRef);
		(*rbx_pcall)(th);
		(*rbx_settop)(th, 0);

		if (!roblox_thread || teleport_event) return;

		rbx_unref(th, onMessageRef);
		rbx_unref(th, onCloseRef);
		rbx_unref(th, threadRef);
	}

	int handleIndex(uint64_t rl) {
		if (!roblox_thread || teleport_event) connected = false;
		if (!connected)
			return 0;

        (*rbx_checkany)(rl, 2);
		if (!settings::get_boolean("debugLibrary")) {
			(*rbx_error)("Websockets Disabled.");
		};

        if (rbx_gettype(rl, 1) != LUA_TUSERDATA || rbx_gettype(rl, 2) != LUA_TSTRING) {
            (*rbx_error)("Invalid arguments to index hook.");
        }

		std::string idx = rbx_tolstring(rl, 2);

		if (idx == "OnMessage") {
			rbx_getref(rl, this->onMessageRef);
			(*rbx_getfield)(rl, -1, "Event");
			return 1;
		}
		else if (idx == "OnClose") {
			rbx_getref(rl, this->onCloseRef);
			(*rbx_getfield)(rl, -1, "Event");
			return 1;
		}
		else if (idx == "Send") {
			(*rbx_pushvalue)(rl, lua_upvalueindex(1));
			(*rbx_pushcclosure)(rl, sendData, "WebSocket.Send", 1, 0);
			return 1;
		}
		else if (idx == "Close") {
			(*rbx_pushvalue)(rl, lua_upvalueindex(1));
			(*rbx_pushcclosure)(rl, closeSocket, "WebSocket.Close", 1, 0);
			return 1;
		}
		else if (idx == "Connected") {
			(*rbx_pushboolean)(rl, this->connected);
			return 1;
		}

		return 0;
	};

	int handleNewIndex(uint64_t rl) {
		(*rbx_error)("Unable to set property!");
		return 0;
	};
};

int sendData(uint64_t rl) {
    (*rbx_checkany)(rl, 2);
    if (rbx_gettype(rl, 1) != LUA_TUSERDATA || rbx_gettype(rl, 2) != LUA_TSTRING) {
        (*rbx_error)("Invalid arguments to Websocket.Send.");
    }

    std::string data = rbx_tolstring(rl, -1);
    websocketClass* webSocket = reinterpret_cast<websocketClass*>(rbx_touserdata(rl, lua_upvalueindex(1)));
    webSocket->webSocket->send(data, true);
    return 0;
}

void closeThread(uint64_t rl, websocketClass* webSocket) {
	usleep(50000); webSocket->webSocket->stop();
	rbx_getref(webSocket->th, webSocket->onCloseRef);
	(*rbx_getfield)(webSocket->th, -1, "Fire");

	(*rbx_insert)(webSocket->th, -2);
	rbx_pop(webSocket->th, 1);

	rbx_getref(webSocket->th, webSocket->onCloseRef);
	(*rbx_pcall)(webSocket->th);
	(*rbx_settop)(webSocket->th, 0);

	rbx_unref(webSocket->th, webSocket->onMessageRef);
	rbx_unref(webSocket->th, webSocket->onCloseRef);
	rbx_unref(webSocket->th, webSocket->threadRef);
	rbx_continue(rl, 0);
}

void forceCloseSocket(websocketClass* webSocket) {
	webSocket->webSocket->stop();
}

int closeSocket(uint64_t rl) {
    if (rbx_gettop(rl) == 0) {
		(*rbx_error)("Expected socket to close.");
		return 0;
	}

	if (rbx_gettop(rl) == 2) {
		websocketClass* webSocket = reinterpret_cast<websocketClass*>(rbx_touserdata(rl, lua_upvalueindex(1)));
		std::thread(forceCloseSocket, webSocket).detach();
		webSocket->connected = false;
		return 0;
	}

    if (rbx_gettype(rl, 1) != LUA_TUSERDATA) {
        (*rbx_error)("Invalid arguments to Websocket.Close.");
		return 0;
    }

    websocketClass* webSocket = reinterpret_cast<websocketClass*>(rbx_touserdata(rl, lua_upvalueindex(1)));
	webSocket->connected = false;

	std::thread(closeThread, rl, webSocket).detach();
    return rbx_yield(rl, 0);
}

void connect_handler(uint64_t rl, websocketClass* webSocket) {
    webSocket->webSocket->connect(5);
	if (webSocket->webSocket->getReadyState() != ix::ReadyState::Open) {
        (*rbx_print)(2, "WebSocket connection failed.");
        rbx_continue(rl, 0);
	}
	
	webSocket->connected = true;
	webSocket->webSocket->start();
    rbx_continue(rl, 1);
}

int websocket_connect(uint64_t rl) {
	(*rbx_checkany)(rl, 3);
	if (!settings::get_boolean("debugLibrary")) {
		(*rbx_error)("Websockets Disabled.");
	}

    if (rbx_gettype(rl, 1) != LUA_TSTRING) {
        (*rbx_error)("Invalid arguments for Websocket.Connect");
    }

    if (rbx_gettype(rl, 2) != LUA_TUSERDATA || rbx_gettype(rl, 3) != LUA_TUSERDATA) {
        (*rbx_error)("Called WebSocket.Connect incorrectly.");
    }

	std::string url = rbx_tolstring(rl, 1);
	websocketClass* webSocket = (websocketClass*)(*rbx_newudata)(rl, sizeof(websocketClass), 0);
	*webSocket = websocketClass{};

	webSocket->th = (*rbx_newthread)(rl);
    webSocket->threadRef = rbx_ref(rl, -1);
	webSocket->webSocket = new ix::WebSocket();
	webSocket->webSocket->setUrl(url);
	rbx_pop(rl, 1);

	webSocket->onMessageRef = rbx_ref(rl, 2);
	webSocket->onCloseRef = rbx_ref(rl, 3);

	webSocket->webSocket->setOnMessageCallback(
		[webSocket](const ix::WebSocketMessagePtr& msg) -> void {
			if (!webSocket->connected) return;

			if (msg->type == ix::WebSocketMessageType::Message) {
                std::cout << "[WebSocket] " << msg->str << "\n";
				webSocket->fireMessage(msg->str);
                return;
			}

			if (msg->type == ix::WebSocketMessageType::Error || msg->type == ix::WebSocketMessageType::Close) {
				webSocket->fireClose();
			}
		}
	);

	(*rbx_createtable)(rl, 0, 0);
	rbx_pushstring(rl, "WebSocket");
	(*rbx_setfield)(rl, -2, "__type");

	(*rbx_pushvalue)(rl, -2);
	(*rbx_pushcclosure)(rl,
		[](uint64_t rl) -> int {
			websocketClass* webSocket = reinterpret_cast<websocketClass*>(rbx_touserdata(rl, lua_upvalueindex(1)));
			return webSocket->handleIndex(rl);
		}, "__index", 1, 0);

	(*rbx_setfield)(rl, -2, "__index");

    (*rbx_pushvalue)(rl, -2);
	(*rbx_pushcclosure)(rl,
		[](uint64_t rl) -> int {
			websocketClass* webSocket = reinterpret_cast<websocketClass*>(rbx_touserdata(rl, lua_upvalueindex(1)));
			return webSocket->handleNewIndex(rl);
		}, "__newindex", 1, 0);

	(*rbx_setfield)(rl, -2, "__newindex");
	(*rbx_setmetatable)(rl, -2);

    std::thread(connect_handler, rl, webSocket).detach();
    return rbx_yield(rl, 1);
}