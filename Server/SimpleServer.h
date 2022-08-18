#pragma once

class SimpleServer
{
public:
	int main();

private:
	void InputServerName();
	static void RunServer();
	bool CreateServer();
	static void SendMessageToClient();
};