#pragma once
#include <windows.h>
#include <iostream>
#include <vector>


#pragma once
#include <stdint.h>

class SharedMem 
{
private:
    HANDLE hMapFile;
    LPCTSTR pBuf;
    HANDLE hMapFile2;
    LPCTSTR pBuf2;
protected:

    void InitializeSharedMemory();
    void WriteToSharedMemory(uint32_t target_vid);
    uint32_t ReadFromSharedMemory();
    void WriteMultipleToSharedMemory(const std::vector<int>& targetVids);
    std::vector<int> ReadMultipleFromSharedMemory();
    void InitializeSharedMemory2();
    void WriteToSharedMemory2(uint32_t testvid2);
    uint32_t ReadFromSharedMemory2();
    void CleanupSharedMemory();

};

#pragma pack(push, 1)
#define MAX_TARGETS 50

struct SyncSharedData {
    volatile LONG targetCount;
    volatile LONG targetVIDs[MAX_TARGETS];
    volatile LONG dummyFakeVID;
    volatile LONG mainActive;
    volatile LONG dummyActive;
    volatile LONG syncCounter;
};
#pragma pack(pop)

class SharedMemSync {
private:
    HANDLE hMapFile;
    SyncSharedData* pData;
    bool isInitialized;

public:
    SharedMemSync();
    ~SharedMemSync();

    bool Initialize();

    void SetMainActive(bool active);
    void SetDummyActive(bool active);

    void SendTargetList(const std::vector<uint32_t>& targetVIDs);
    std::vector<uint32_t> ReadTargetList();

    void SendFakeVID(uint32_t fakeVID);
    uint32_t ReadFakeVID();

    bool IsMainActive() const;
    bool IsDummyActive() const;
};

class PipeServer {
private:
    HANDLE hPipeToClient;
    HANDLE hPipeFromClient;
    bool connectedToClient;
    bool connectedFromClient;

public:
    PipeServer();
    ~PipeServer();

    bool Initialize();
    void SendTargetVID(uint32_t targetVID);
    uint32_t ReadFakeVID();
    bool IsConnected() const;
    void Disconnect();
};

class PipeClient {
private:
    HANDLE hPipeFromServer;
    HANDLE hPipeToServer;
    bool connectedFromServer;
    bool connectedToServer;

public:
    PipeClient();
    ~PipeClient();

    bool Initialize();
    bool Reconnect();
    uint32_t ReadTargetVID();
    void SendFakeVID(uint32_t fakeVID);
    bool IsConnected() const;
};



//#define _DEBUG



