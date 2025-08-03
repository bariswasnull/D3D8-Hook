#include "PlayerFunctions.h"
#include "FeatureSettings.h"
#include "SharedMem.h"
#include "Offsets.h"
#define min1(a,b)            (((a) < (b)) ? (a) : (b))
struct EntityData {
    uint32_t vid;
    uint32_t address;
    Vector3 position;
    float distance; 
};
std::vector<EntityData> targetEntities;  
std::vector<uint32_t> targetVIDs;       
void PlayerFunctions::SendAttackPacket(const uint8_t mode, const uint32_t vid)
{
    typedef bool(__thiscall* tSendAttackPacket)(int, const uint8_t, const uint32_t);
    const auto fSendAttackPacket = reinterpret_cast<tSendAttackPacket>(offsets->baseModule + offsets->CFuncBattleCall); /* SendAttackPacket Function Call */

    if (fSendAttackPacket)
        fSendAttackPacket(*reinterpret_cast<uintptr_t*>(offsets->baseModule + offsets->CFuncNETPTR), mode, vid); /* CNetworkStream Class Pointer Instance */
}
void PlayerFunctions::SendFlyingAttackPacket(const uint32_t vid, float* mobPos)
{
    typedef bool(__thiscall* tSendAttackPacket)(int, const uint32_t, const float* mobPos);
    const auto fSendAttackPacket = reinterpret_cast<tSendAttackPacket>(offsets->baseModule + offsets->SendFlyCall); /* SendAttackPacket Function Call */

    if (fSendAttackPacket)
        fSendAttackPacket(*reinterpret_cast<uintptr_t*>(offsets->baseModule + offsets->CFuncNETPTR), vid, mobPos); /* CNetworkStream Class Pointer Instance */
}
void PlayerFunctions::NetworkStreamSendShootPacket(UINT uSkill)
{
    typedef bool(__thiscall* tSendAttackPacket)(int, UINT uSkill);
    const auto fSendAttackPacket = reinterpret_cast<tSendAttackPacket>(offsets->baseModule + offsets->SendShootPacketCall); /* SendAttackPacket Function Call */

    if (fSendAttackPacket)
        fSendAttackPacket(*reinterpret_cast<uintptr_t*>(offsets->baseModule + offsets->CFuncNETPTR), uSkill); /* CNetworkStream Class Pointer Instance */
}
void PlayerFunctions::SendCharacterState(float* mobPos, float myRot, uint8_t efunc, uint8_t args, uint8_t unk)
{
    typedef bool(__thiscall* TeleportPacket)(int, const float*, float, uint8_t, uint8_t);
    const auto wPacket = reinterpret_cast<TeleportPacket>(offsets->baseModule + offsets->CFuncSendCharStateCall); /* SendCharacterState function call */

    if (wPacket) {
        wPacket(*reinterpret_cast<uintptr_t*>(offsets->baseModule + offsets->CFuncNETPTR), mobPos, myRot, efunc, args);
    }

}

void PlayerFunctions::StopMobberSystem() {
   
    entityScanActive = false;
    mainAttackActive = false;
    dummyAttackActive = false;

    if (scanThread && scanThread->joinable()) {
        scanThread->join();
    }
    if (attackThread && attackThread->joinable()) {
        attackThread->join();
    }
    if (dummyThread && dummyThread->joinable()) {
        dummyThread->join();
    }

    if (globalSharedMem) {
        globalSharedMem->SetMainActive(false);
        globalSharedMem->SetDummyActive(false);
        delete globalSharedMem;
        globalSharedMem = nullptr;
    }

}
//#define _DEBUG
void PlayerFunctions::EntityScanThread()
{
#ifdef _DEBUG
    sdk::utilities::setup_console();
#endif
    offsets->baseModule = reinterpret_cast<DWORD>(GetModuleHandleA("metin2client.exe"));
   
    while (entityScanActive.load())
    {
        if (FeatureSettings::_7xActivate && !FeatureSettings::isDummy) {
            try {
                uint32_t entityList = read<uint32_t>(offsets->baseModule + offsets->EntityList);
                uint32_t localPlayer = read<uint32_t>(offsets->baseModule + offsets->LocalBase);
                if (!localPlayer) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    continue;
                }
                int localvid = read<int>(localPlayer + offsets->LocalVID);

                if (localactualplayer == 0) {
                    for (size_t i = 0; i < 300; i++) {
                        uint32_t entities = read<uint32_t>(entityList + (0x4 * i));
                        if (!entities) continue;

                        int vid = read<int>(entities + offsets->TargetVID);
                        if (vid == localvid) {
                            localactualplayer = entities;
                            break;
                        }
                    }
                }

                Vector3 LocalPos = read<Vector3>(localactualplayer + offsets->EntityPOS);

                std::vector<EntityData> allValidEntities;

                for (size_t i = 0; i < 300; i++)
                {
                    uint32_t entities = read<uint32_t>(entityList + (0x4 * i));
                    if (!entities || IsBadReadPtr((void*)entities, sizeof(uint32_t))) continue;

                    uint32_t entityType = entities & 0xF0000000;
                    uint32_t entitySubType = entities & 0xF;

                    if (entityType != 0x20000000 && entityType != 0x30000000 && entityType != 0x40000000) continue;
                    if (entitySubType != 0x0 && entitySubType != 0x8) continue;
                    if (read<bool>(entities + offsets->isAlive)) continue;

                    int targetvid = read<int>(entities + offsets->TargetVID);
                    if (targetvid == localvid) { localactualplayer = entities; continue; }
                    if (targetvid <= 1 || targetvid == localvid) continue;
                  
                    uint32_t mobinstance = read<uint32_t>(entities + offsets->InstanceType);
                    bool isValidInstance = (mobinstance == instance::Mob && FeatureSettings::targetMob) ||
                        (mobinstance == instance::Tas && FeatureSettings::targetMetin) ||
                        (mobinstance == instance::Oyuncu && FeatureSettings::targetPlayer);
              
                    if (isValidInstance) {
                        Vector3 EntPos = read<Vector3>(entities + offsets->EntityPOS);
                        float dist = distance(LocalPos, EntPos);
         
                        if (dist < FeatureSettings::attackDistance) {
#ifdef _DEBUG
                            printf("\nTarget Addr %p", entities);
#endif
                            EntityData entityData;
                            entityData.vid = static_cast<uint32_t>(targetvid);
                            entityData.address = entities;
                            entityData.position = EntPos;
                            entityData.distance = dist;

                            allValidEntities.push_back(entityData);
                        }
                    }
                    else continue;
                }

                
                if (allValidEntities.size() > FeatureSettings::mobSize) {
                   
                    std::sort(allValidEntities.begin(), allValidEntities.end(),
                        [](const EntityData& a, const EntityData& b) {
                            return a.distance < b.distance;
                        });

                 
                    allValidEntities.resize(FeatureSettings::mobSize);
                }

                targetEntities.clear();
                targetVIDs.clear();

                for (const EntityData& entity : allValidEntities) {
                    targetEntities.push_back(entity);
                    targetVIDs.push_back(entity.vid);
                }

                if (globalSharedMem && !targetVIDs.empty()) {
                    globalSharedMem->SendTargetList(targetVIDs);
                }

                static int scanCycle = 0;
                if (++scanCycle % 20 == 0) {
                 
                }
            }
            catch (...) {
                
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
   
}

void PlayerFunctions::MainAttackThread()
{

    while (mainAttackActive.load())
    {
        if (FeatureSettings::_7xActivate && !FeatureSettings::isDummy) {
            if (!targetEntities.empty()) {
                
                for (auto it = targetEntities.begin(); it != targetEntities.end();) {
                    try {
                      
                        bool isAlive = read<bool>(it->address + isAlive);
                        if (isAlive) {
                          
                            it = targetEntities.erase(it);  
                            continue;
                        }

                      
                        int currentVID = read<int>(it->address + offsets->TargetVID);
                        if (currentVID <= 1) {
                            
                            it = targetEntities.erase(it);
                            continue;
                        }

                       
                        Vector3 currentPos = read<Vector3>(it->address + offsets->EntityPOS);
                        Vector3 LocalPos = read<Vector3>(localactualplayer + offsets->EntityPOS);

                        float mobPosF[2] = { currentPos.x, abs(currentPos.y) };
                        float myPos[2] = { LocalPos.x, abs(LocalPos.y) };
#ifdef _DEBUG
                        printf("Main Hesap: Saldirilan VID: %d\n", currentVID);
#endif
                        SendCharacterState(mobPosF, 0, 0, 0, 0);
                        SendAttackPacket(0, currentVID);
                        SendCharacterState(myPos, 0, 0, 0, 0);

                        ++it; 
                    }
                    catch (...) {
                       
                        it = targetEntities.erase(it);  
                    }
                }

                
                targetVIDs.clear();
                for (const EntityData& entity : targetEntities) {
                    targetVIDs.push_back(entity.vid);
                }

              
                if (globalSharedMem) {
                    uint32_t dummyFakeVID = globalSharedMem->ReadFakeVID();
                    if (dummyFakeVID > 0) {
                        SendAttackPacket(0, dummyFakeVID);
                    }
                }

                static int attackCycle = 0;
                if (++attackCycle % 50 == 0) {

                }
            }
            else {
                static int noTargetCount = 0;
                if (++noTargetCount % 100 == 0) {
               
                }
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(FeatureSettings::attackDelay));
    }
   
}

void PlayerFunctions::DummyAttackThread()
{
  
    offsets->baseModule = reinterpret_cast<DWORD>(GetModuleHandleA("metin2client.exe"));

    std::this_thread::sleep_for(std::chrono::milliseconds(3000));

    while (dummyAttackActive.load())
    {
        if (FeatureSettings::isDummy && globalSharedMem) {
            try {
                uint32_t localPlayer = read<uint32_t>(offsets->baseModule + offsets->LocalBase);
                if (localPlayer) {
                    int localvid = read<int>(localPlayer + offsets->LocalVID);
                    if (localvid > 0) {
                        globalSharedMem->SendFakeVID(static_cast<uint32_t>(localvid));
                    }
                }

              
                std::vector<uint32_t> targetVIDs = globalSharedMem->ReadTargetList();
                if (!targetVIDs.empty()) {
                    for (uint32_t targetVID : targetVIDs) {
                        SendAttackPacket(0, targetVID);
                    }

                    static int dummyAttackCount = 0;
                    if (++dummyAttackCount % 30 == 0) {
                        
                    }
                }
            }
            catch (...) {
              
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(FeatureSettings::attackDelay));
    }
 
}

void PlayerFunctions::StartMobberSystem()
{
    if (IsSystemRunning()) {
       
        return;
    }

    globalSharedMem = new SharedMemSync();
    if (!globalSharedMem->Initialize()) {
        delete globalSharedMem;
        globalSharedMem = nullptr;
        return;
    }

    entityScanActive = true;
    mainAttackActive = true;
    dummyAttackActive = true;

    scanThread = std::make_unique<std::thread>(&PlayerFunctions::EntityScanThread, this);
    attackThread = std::make_unique<std::thread>(&PlayerFunctions::MainAttackThread, this);
    dummyThread = std::make_unique<std::thread>(&PlayerFunctions::DummyAttackThread, this);

    while (entityScanActive.load() || mainAttackActive.load() || dummyAttackActive.load()) {
        if (GetAsyncKeyState(VK_END)) {
            break;
        }
        if (FeatureSettings::isDummy) {
            mainAttackActive = false;
            if (globalSharedMem) {
                globalSharedMem->SetDummyActive(true);
                globalSharedMem->SetMainActive(false);
            }
        }
        else {
            mainAttackActive = true;
            if (globalSharedMem) {
                globalSharedMem->SetMainActive(true);
                globalSharedMem->SetDummyActive(false);
            }
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    StopMobberSystem();
}
