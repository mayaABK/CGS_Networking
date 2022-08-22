#pragma once

#include <enet/enet.h>

class NumberGuessingServer
{
public:
	int run();
private:
	static void ServerProcessPackets();
	static void BroadcastIsDeadPacket();
	static void HandleReceivePacket(const ENetEvent& event);
	bool CreateServer();
};

