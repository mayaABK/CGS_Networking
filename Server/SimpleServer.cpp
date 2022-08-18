#include <enet/enet.h>
#include <stdio.h>
#include <iostream>
#include <thread>
#include <string>

#include "SimpleServer.h"

using namespace std;

ENetAddress address;
ENetHost* server;
ENetEvent event;
string serverName = "";

bool SimpleServer::CreateServer()
{
    /* Bind the server to the default localhost.     */
    /* A specific host address can be specified by   */
    /* enet_address_set_host (& address, "x.x.x.x"); */
    address.host = ENET_HOST_ANY;
    /* Bind the server to port 1234. */
    address.port = 1234;
    server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return server != NULL;
}

void SimpleServer::SendMessageToClient()
{
    string message;
    string input;
    cout << "Enter q to quit" << endl;
    cin.ignore(256, '\n');

    while (input != "q" && input != "Q")
    {
        getline(cin, input);
        message = serverName + ": " + input;

        ENetPacket* packet = enet_packet_create(message.c_str(),
            strlen(message.c_str()) + 1,
            ENET_PACKET_FLAG_RELIABLE);

        enet_host_broadcast(server, 0, packet);

        enet_host_flush(server);
    }

    cout << "Ending chat" << endl;
}

void SimpleServer::RunServer()
{
    while (1)
    {
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                cout << endl << "A new client connected from "
                    << event.peer->address.host
                    << ":" << event.peer->address.port
                    << endl;

                /* Store any relevant client information here. */
                event.peer->data = (void*)("Client information");

                break;
            case ENET_EVENT_TYPE_RECEIVE:
                cout << (char*)event.packet->data << endl;
                //<< "was received from " << (char*)event.peer->data
                //<< " on channel " << event.channelID << endl;
            /* Clean up the packet now that we're done using it. */
                enet_packet_destroy(event.packet);

                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                cout << (char*)event.peer->data << "disconnected." << endl;
                /* Reset the peer's client information. */
                event.peer->data = NULL;
            }
        }
    }
}

void SimpleServer::InputServerName()
{
    cout << "Enter a name for server: ";
    cin >> serverName;
}

int SimpleServer::run()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }

    if (!CreateServer())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server host.\n");
        exit(EXIT_FAILURE);
    }

    cout << "Server is created" << endl;

    InputServerName();
    thread RunServerThread(RunServer);
    thread SendMessageToClientThread(SendMessageToClient);

    RunServerThread.join();
    SendMessageToClientThread.join();

    if (server != NULL)
    {
        enet_host_destroy(server);
    }

    atexit(enet_deinitialize);
}