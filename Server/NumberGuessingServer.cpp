#include "NumberGuessingServer.h"
#include "../CGS_Networking/PacketTypes.h"

#include <enet/enet.h>
#include <string>
#include <iostream>
#include <thread>

using namespace std;

ENetHost* Server = nullptr;
ENetPeer* Peer = nullptr;
thread* PacketThread = nullptr;

//can pass in a peer connection if wanting to limit
bool NumberGuessingServer::CreateServer()
{
    ENetAddress address;
    address.host = ENET_HOST_ANY;
    address.port = 1234;
    Server = enet_host_create(&address /* the address to bind the server host to */,
        32      /* allow up to 32 clients and/or outgoing connections */,
        2      /* allow up to 2 channels to be used, 0 and 1 */,
        0      /* assume any amount of incoming bandwidth */,
        0      /* assume any amount of outgoing bandwidth */);

    return Server != nullptr;
}

void NumberGuessingServer::HandleReceivePacket(const ENetEvent& event)
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
        enet_host_flush(Server);
    }
}

void NumberGuessingServer::BroadcastIsDeadPacket()
{
    IsDeadPacket* DeadPacket = new IsDeadPacket();
    DeadPacket->IsDead = true;
    ENetPacket* packet = enet_packet_create(DeadPacket,
        sizeof(IsDeadPacket),
        ENET_PACKET_FLAG_RELIABLE);

    /* One could also broadcast the packet by         */
    enet_host_broadcast(Server, 0, packet);
    //enet_peer_send(event.peer, 0, packet);

    /* One could just use enet_host_service() instead. */
    //enet_host_service();
    enet_host_flush(Server);
    delete DeadPacket;
}

void NumberGuessingServer::ServerProcessPackets()
{
    while (1)
    {
        ENetEvent event;
        while (enet_host_service(Server, &event, 1000) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT:
                cout << "A new client connected from "
                    << event.peer->address.host
                    << ":" << event.peer->address.port
                    << endl;
                /* Store any relevant client information here. */
                event.peer->data = (void*)("Client information");
                BroadcastIsDeadPacket();
                break;
            case ENET_EVENT_TYPE_RECEIVE:
                HandleReceivePacket(event);
                break;

            case ENET_EVENT_TYPE_DISCONNECT:
                cout << (char*)event.peer->data << "disconnected." << endl;
                /* Reset the peer's client information. */
                event.peer->data = NULL;
                //notify remaining player that the game is done due to player leaving
            }
        }
    }
}

int NumberGuessingServer::run()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        cout << "An error occurred while initializing ENet." << endl;
        return EXIT_FAILURE;
    }
    atexit(enet_deinitialize);

    //How many players?

    if (!CreateServer())
    {
        fprintf(stderr,
            "An error occurred while trying to create an ENet server.\n");
        exit(EXIT_FAILURE);
    }

    cout << "waiting for players to join..." << endl;
    PacketThread = new thread(ServerProcessPackets);

    if (PacketThread)
    {
        PacketThread->join();
    }
    delete PacketThread;
    if (Server != nullptr)
    {
        enet_host_destroy(Server);
    }

    return EXIT_SUCCESS;
}