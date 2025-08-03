#pragma once
#include "MemoryFunctions.h"
#include <mutex>
#define RESET   "\033[0m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
struct TargetInfo {
    uint32_t vid;
    float distance;
    uint32_t entityPtr;

    bool operator<(const TargetInfo& other) const {
        return distance < other.distance; 
    }
};

class PlayerFunctions : MemoryFunctions
{
private:
    std::vector<uint32_t> targetVIDs;

    std::atomic<bool> entityScanActive{ false };
    std::atomic<bool> mainAttackActive{ false };
    std::atomic<bool> dummyAttackActive{ false };
    SharedMemSync* globalSharedMem = nullptr;
    std::unique_ptr<std::thread> scanThread;
    std::unique_ptr<std::thread> attackThread;
    std::unique_ptr<std::thread> dummyThread;

public:

    void EntityScanThread();
    void MainAttackThread();
    void DummyAttackThread();
    void StartMobberSystem();
    void StopMobberSystem();

    
    void SendAttackPacket(const uint8_t mode, const uint32_t vid);
    void SendFlyingAttackPacket(const uint32_t vid, float* mobPos);
    void NetworkStreamSendShootPacket(UINT uSkill);
    void SendCharacterState(float* mobPos, float myRot, uint8_t efunc, uint8_t args, uint8_t unk);

    bool IsSystemRunning() const { return entityScanActive || mainAttackActive || dummyAttackActive; }
    size_t GetTargetCount() const { return targetVIDs.size(); }

    enum instance {
        Mob,
        Npc,
        Tas,
        Tunel,
        Kapi,
        Yapi,
        Oyuncu,
        TYPE_POLY,
        TYPE_HORSE,
        TYPE_GOTO,
        TYPE_OBJECT,
    };
};