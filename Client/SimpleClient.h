#pragma once

class SimpleClient
{
public:
	int main();
private:
	bool CreateSimpleClient();
	void ConnectToServer();
	static void SendMessageToServer();
	static void RunSimpleClient();
	void InputSimpleClientName();

};