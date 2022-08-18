#pragma once

class SimpleServer
{
public:
	int run();

private:
	void InputServerName();
	static void RunServer();
	bool CreateServer();
	static void SendMessageToClient();
};