#pragma once
#include <Windows.h>
#include <iostream>
#include <vector>
#include <mutex>

extern uintptr_t localactualplayer;
extern uint32_t fakevid;
extern uint32_t closesttarget;
extern std::vector<uint32_t> playerList;
extern std::vector<uint32_t> mobList;
extern std::vector<uint32_t> metinmadenList;
extern std::vector<std::string> foundEntities;
extern std::mutex entityMutex;

struct offsets_t
{
    DWORD NetPointer;
    DWORD BattleCall;
    DWORD SendCharStateCall;
    DWORD SendPickItemCall;
    DWORD EntityList;
    DWORD TargetVID;
    DWORD EntityPOS;
    DWORD ActorInstance;
    DWORD InstancePOS;
    DWORD EntityName;
    DWORD InstanceType;
    DWORD PlayerIfVisible;
    DWORD isAlive;
    DWORD LocalBase;
    DWORD LocalVID;
    DWORD PremHuntBase;
    DWORD PremHuntStartOffset;
    DWORD ItemListBase;
    DWORD ItemVNUM;
    DWORD CFuncNETPTR;
    DWORD SendFlyCall;
    DWORD SendShootPacketCall;
    DWORD CFuncBattleCall;
    DWORD CFuncSendCharStateCall;
    DWORD CFuncSendPickItemCall;
    DWORD DEVICE_OFFSET;
    DWORD baseModule;
    bool running;

	void init();
};
extern offsets_t* offsets;
typedef void (*init_offsets_t)();
typedef offsets_t* (*get_offsets_t)();
extern offsets_t* gethelper();
extern void inithelper();