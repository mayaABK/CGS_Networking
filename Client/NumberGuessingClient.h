#pragma once
#include <enet/enet.h>

class NumberGuessingClient
{
public:
	int run();
private:
	static void ClientProcessPackets();
	static void HandleReceivePacket(const ENetEvent& event);
	bool AttemptConnectToServer();
	bool CreateClient();
};

