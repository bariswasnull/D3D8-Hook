#include "Offsets.h"

uintptr_t localactualplayer;
uint32_t fakevid;
uint32_t closesttarget;
std::vector<uint32_t> playerList;
std::vector<uint32_t> mobList;
std::vector<uint32_t> metinmadenList;
std::vector<std::string> foundEntities;
std::mutex entityMutex;

static offsets_t g_offsets;
offsets_t* offsets = &g_offsets;
offsets_t* gethelper()
{
    return &g_offsets;
}

void inithelper()
{
    g_offsets.init();
}