#include "NumberGuessingClient.h"
#include "../CGS_Networking/PacketTypes.h"

#include <enet/enet.h>
#include <string>
#include <iostream>
#include <thread>

using namespace std;

ENetHost* Client = nullptr;
ENetPeer* Peer = nullptr;
thread* PacketThread = nullptr;

bool NumberGuessingClient::CreateClient()
{
    Client = enet_host_create(NULL /* create a client host */,
        1 /* only allow 1 outgoing connection */,
        2 /* allow up 2 channels to be used, 0 and 1 */,
        0 /* assume any amount of incoming bandwidth */,
        0 /* assume any amount of outgoing bandwidth */);

    return Client != nullptr;
}

bool NumberGuessingClient::AttemptConnectToServer()
{
    ENetAddress address;
    /* Connect to some.server.net:1234. */
    enet_address_set_host(&address, "127.0.0.1");
    address.port = 1234;
    /* Initiate the connection, allocating the two channels 0 and 1. */
    Peer = enet_host_connect(Client, &address, 2, 0);
    return Peer != nullptr;
}

void NumberGuessingClient::HandleReceivePacket(const ENetEvent& event)
{
    GamePacket* RecGamePacket = (GamePacket*)(event.packet->data);
    if (RecGamePacket)
    {
        cout << "Received Game Packet " << endl;

        if (RecGamePacket->Type == PHT_IsDead)
        {
            cout << "u dead?" << endl;
            IsDeadPacket* DeadGamePacket = (IsDeadPacket*)(event.packet->data);
            if (DeadGamePacket)
            {
                string response = (DeadGamePacket->IsDead ? "yeah" : "no");
                cout << response << endl;
            }
        }
    }
    else
    {
        cout << "Invalid Packet " << endl;
    }

    /* Clean up the packet now that we're done using it. */
    enet_packet_destroy(event.packet);
    {
        enet_host_flush(Client);
    }
}


void NumberGuessingClient::ClientProcessPackets()
{
    while (1)
    {
        ENetEvent event;
        /* Wait up to 1000 milliseconds for an event. */
        while (enet_host_service(Client, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case  ENET_EVENT_TYPE_CONNECT:
                cout << "Connection succeeded " << endl;
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                HandleReceivePacket(event);
                break;
            }
        }
    }
}

int NumberGuessingClient::run()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    
    if (!CreateClient())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet client host.\n");
        exit(EXIT_FAILURE);
    }

    if (!AttemptConnectToServer())
    {
        fprintf(stderr,
            "No available peers for initiating an ENet connection.\n");
        exit(EXIT_FAILURE);
    }

    PacketThread = new thread(ClientProcessPackets);

    //handle possible connection failure
    {
        //enet_peer_reset(Peer);
        //cout << "Connection to 127.0.0.1:1234 failed." << endl;
    }


    if (PacketThread)
    {
        PacketThread->join();
    }
    delete PacketThread;
    if (Client != nullptr)
    {
        enet_host_destroy(Client);
    }

    return EXIT_SUCCESS;
}