#pragma once

class SimpleClient
{
public:
	int run();
private:
	bool CreateSimpleClient();
	void ConnectToServer();
	static void SendMessageToServer();
	static void RunSimpleClient();
	void InputSimpleClientName();

};