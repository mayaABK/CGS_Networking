#include <enet/enet.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <string>

using namespace std;

ENetHost* simpleClient;
string simpleClientName = "";

bool CreateSimpleClient()
{
    simpleClient = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    return simpleClient != NULL;
}

void ConnectToServer()
{
    ENetAddress address;
    ENetEvent event;
    ENetPeer* peer;
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    peer = enet_host_connect(simpleClient, &address, 2, 0);
    if (peer == NULL)
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }
    /* Wait up to 5 seconds for the connection attempt to succeed. */
    if (enet_host_service(simpleClient, &event, 5000) > 0 &&
        event.type == ENET_EVENT_TYPE_CONNECT)
    {
        cout << "Connection to 127.0.0.1:1234 succeeded." << endl;
    }
    else
    {
        /* Either the 5 seconds are up or a disconnect event was */
        /* received. Reset the peer in the event the 5 seconds   */
        /* had run out without any significant event.            */
        enet_peer_reset(peer);
        cout << "Connection to 127.0.0.1:1234 failed." << endl;
    }
}

void SendMessageToServer()
{
    string message;
    cout << "Enter q to quit" << endl;
    cin.ignore(256, '\n');

    while (message != "q" && message != "Q")
    {
        getline(cin, message);
        message = simpleClientName + ": " + message;

        ENetPacket* packet = enet_packet_create(message.c_str(),
            strlen(message.c_str()) + 1,
            ENET_PACKET_FLAG_RELIABLE);

        enet_host_broadcast(simpleClient, 0, packet);

        /* One could just use enet_host_service() instead. */
        //enet_host_service();
        enet_host_flush(simpleClient);
    }

    cout << "Ending chat" << endl;
}

void RunSimpleClient()
{
    while (1)
    {
        ENetEvent event;
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(simpleClient, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_RECEIVE:
                cout << (char*)event.packet->data << endl;
                /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);
            }
        }
    }
}

void InputSimpleClientName()
{
    cout << "Enter a name for client: ";
    cin >> simpleClientName;
}

int main(int argc, char** argv)
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    if (!CreateSimpleClient())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    cout << "Client is created" << endl;
    InputSimpleClientName();

    ConnectToServer();
    thread RunSimpleClientThread(RunSimpleClient);
    thread SendMessageThread(SendMessageToServer);

    RunSimpleClientThread.join();
    SendMessageThread.join();

    if (simpleClient != NULL)
    {
        enet_host_destroy(simpleClient);
    }

    atexit(enet_deinitialize);
}