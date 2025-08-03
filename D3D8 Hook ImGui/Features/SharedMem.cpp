#include "SharedMem.h"
#include <thread>
void SharedMem::InitializeSharedMemory()
{
    CleanupSharedMemory();

    hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        1024,
        "Local\\mt2shared");

    if (hMapFile == NULL)
    {
        return;
    }

    pBuf = (LPTSTR)MapViewOfFile(
        hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        1024);

    if (pBuf == NULL)
    {
        CloseHandle(hMapFile);
        hMapFile = NULL; 
        return;
    }
}
void SharedMem::WriteToSharedMemory(uint32_t target_vid)
{
    if (pBuf)
    {
        memcpy((void*)pBuf, &target_vid, sizeof(target_vid));
    }
}
uint32_t SharedMem::ReadFromSharedMemory()
{
    uint32_t target_vid = 0;
    if (pBuf)
    {
        memcpy(&target_vid, pBuf, sizeof(target_vid));
    }
    return target_vid;
}
void SharedMem::WriteMultipleToSharedMemory(const std::vector<int>& targetVids) {
    if (pBuf) {

        uint32_t count = targetVids.size();

        memcpy((void*)pBuf, &count, sizeof(count));

        for (size_t i = 0; i < targetVids.size(); ++i) {
            memcpy((void*)((char*)pBuf + sizeof(count) + i * sizeof(targetVids[i])), &targetVids[i], sizeof(targetVids[i]));
        }
    }
}
std::vector<int> SharedMem::ReadMultipleFromSharedMemory() {
    std::vector<int> targetVids;

    if (pBuf) {
        uint32_t count = 0;


        memcpy(&count, pBuf, sizeof(count));


        for (size_t i = 0; i < count; ++i) {
            int target_vid = 0;
            memcpy(&target_vid, (void*)((char*)pBuf + sizeof(count) + i * sizeof(target_vid)), sizeof(target_vid));
            targetVids.push_back(target_vid);
        }
    }

    return targetVids;
}
void SharedMem::InitializeSharedMemory2()
{
    hMapFile2 = CreateFileMapping(
        INVALID_HANDLE_VALUE,
        NULL,
        PAGE_READWRITE,
        0,
        256,
        "Local\\mt2shared");

    if (hMapFile2 == NULL)
    {
    
        return;
    }

    pBuf2 = (LPTSTR)MapViewOfFile(
        hMapFile2,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        256);

    if (pBuf2 == NULL)
    {
      
        CloseHandle(hMapFile2);
        hMapFile2 = NULL; 
        return;
    }
}
void SharedMem::WriteToSharedMemory2(uint32_t testvid2)
{
    if (pBuf2)
    {
        memcpy((void*)pBuf2, &testvid2, sizeof(testvid2));
    }
}
uint32_t SharedMem::ReadFromSharedMemory2()
{
    uint32_t testvid2 = 0;
    if (pBuf2)
    {
        memcpy(&testvid2, pBuf2, sizeof(testvid2));
    }
    return testvid2;
}
void SharedMem::CleanupSharedMemory()
{
    if (pBuf)
        UnmapViewOfFile(pBuf);
    if (hMapFile)
        CloseHandle(hMapFile);

    if (pBuf2)
        UnmapViewOfFile(pBuf2);
    if (hMapFile2)
        CloseHandle(hMapFile2);
}


SharedMemSync::SharedMemSync() : hMapFile(NULL), pData(nullptr), isInitialized(false) {}

SharedMemSync::~SharedMemSync() {
    if (pData) {
        UnmapViewOfFile(pData);
        pData = nullptr;
    }
    if (hMapFile) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }
    isInitialized = false;
}

bool SharedMemSync::Initialize() {
    if (isInitialized && pData != nullptr) {
        __try {
            volatile LONG test = pData->syncCounter;
            return true;
        }
        __except (EXCEPTION_EXECUTE_HANDLER) {
            isInitialized = false;
            pData = nullptr;
        }
    }

    if (pData) {
        UnmapViewOfFile(pData);
        pData = nullptr;
    }
    if (hMapFile) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
    }

    hMapFile = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
        0, sizeof(SyncSharedData), "Global\\metin2sync_multi");

    if (hMapFile == NULL) {
        return false;
    }

    bool wasCreated = (GetLastError() != ERROR_ALREADY_EXISTS);
    pData = static_cast<SyncSharedData*>(MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(SyncSharedData)));

    if (pData == nullptr) {
        CloseHandle(hMapFile);
        hMapFile = NULL;
        return false;
    }

    if (wasCreated) {
        InterlockedExchange(&pData->targetCount, 0);
        InterlockedExchange(&pData->dummyFakeVID, 0);
        InterlockedExchange(&pData->mainActive, 0);
        InterlockedExchange(&pData->dummyActive, 0);
        InterlockedExchange(&pData->syncCounter, 0);

        for (int i = 0; i < MAX_TARGETS; i++) {
            InterlockedExchange(&pData->targetVIDs[i], 0);
        }
    }

    isInitialized = true;
    return true;
}

void SharedMemSync::SetMainActive(bool active) {
    if (isInitialized && pData) {
        InterlockedExchange(&pData->mainActive, active ? 1 : 0);
    }
}

void SharedMemSync::SetDummyActive(bool active) {
    if (isInitialized && pData) {
        InterlockedExchange(&pData->dummyActive, active ? 1 : 0);
    }
}

void SharedMemSync::SendTargetList(const std::vector<uint32_t>& targetVIDs) {
    if (!isInitialized || !pData || targetVIDs.empty()) return;

    int count = min((int)targetVIDs.size(), MAX_TARGETS);
    InterlockedExchange(&pData->targetCount, count);
    for (int i = 0; i < count; i++) {
        InterlockedExchange(&pData->targetVIDs[i], (LONG)targetVIDs[i]);
    }
    for (int i = count; i < MAX_TARGETS; i++) {
        InterlockedExchange(&pData->targetVIDs[i], 0);
    }
    InterlockedIncrement(&pData->syncCounter);
}

std::vector<uint32_t> SharedMemSync::ReadTargetList() {
    std::vector<uint32_t> targets;

    if (!isInitialized || !pData) {
        return targets;
    }

    if (InterlockedCompareExchange(&pData->mainActive, 0, 0) != 1) {
        static int inactiveCount = 0;
        if (++inactiveCount % 200 == 0) {}
        return targets;
    }

    int count = InterlockedCompareExchange(&pData->targetCount, 0, 0);
    if (count <= 0) return targets;

    targets.reserve(count);
    for (int i = 0; i < count && i < MAX_TARGETS; i++) {
        uint32_t targetVID = (uint32_t)InterlockedCompareExchange(&pData->targetVIDs[i], 0, 0);
        if (targetVID > 0) {
            targets.push_back(targetVID);
        }
    }

    return targets;
}

void SharedMemSync::SendFakeVID(uint32_t fakeVID) {
    if (isInitialized && pData && fakeVID > 0) {
        InterlockedExchange(&pData->dummyFakeVID, (LONG)fakeVID);
    }
}

uint32_t SharedMemSync::ReadFakeVID() {
    if (isInitialized && pData) {
        if (InterlockedCompareExchange(&pData->dummyActive, 0, 0) == 1) {
            return (uint32_t)InterlockedCompareExchange(&pData->dummyFakeVID, 0, 0);
        }
    }
    return 0;
}

bool SharedMemSync::IsMainActive() const {
    return isInitialized && pData && (InterlockedCompareExchange(&pData->mainActive, 0, 0) == 1);
}

bool SharedMemSync::IsDummyActive() const {
    return isInitialized && pData && (InterlockedCompareExchange(&pData->dummyActive, 0, 0) == 1);
}




PipeServer::PipeServer() :
    hPipeToClient(INVALID_HANDLE_VALUE), hPipeFromClient(INVALID_HANDLE_VALUE),
    connectedToClient(false), connectedFromClient(false) {
}

PipeServer::~PipeServer() {
    if (hPipeToClient != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipeToClient);
        CloseHandle(hPipeToClient);
    }
    if (hPipeFromClient != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipeFromClient);
        CloseHandle(hPipeFromClient);
    }
}

bool PipeServer::Initialize() {
    if (hPipeToClient != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipeToClient);
        hPipeToClient = INVALID_HANDLE_VALUE;
    }
    if (hPipeFromClient != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipeFromClient);
        hPipeFromClient = INVALID_HANDLE_VALUE;
    }

    hPipeToClient = CreateNamedPipe(TEXT("\\\\.\\pipe\\mt2shared"),
        PIPE_ACCESS_OUTBOUND, PIPE_TYPE_MESSAGE | PIPE_NOWAIT,
        1, 1024, 1024, 0, NULL);

    if (hPipeToClient == INVALID_HANDLE_VALUE)
        return false;

    hPipeFromClient = CreateNamedPipe(TEXT("\\\\.\\pipe\\mt2shared"),
        PIPE_ACCESS_INBOUND, PIPE_TYPE_MESSAGE | PIPE_NOWAIT,
        1, 1024, 1024, 0, NULL);

    if (hPipeFromClient == INVALID_HANDLE_VALUE) {
        CloseHandle(hPipeToClient);
        hPipeToClient = INVALID_HANDLE_VALUE;
        return false;
    }

    return true;
}

void PipeServer::SendTargetVID(uint32_t targetVID) {
    if (!connectedToClient && hPipeToClient != INVALID_HANDLE_VALUE) {
        BOOL connectResult = ConnectNamedPipe(hPipeToClient, NULL);
        if (connectResult || GetLastError() == ERROR_PIPE_CONNECTED)
            connectedToClient = true;
    }

    if (!connectedToClient) return;

    DWORD bytesWritten;
    BOOL result = WriteFile(hPipeToClient, &targetVID, sizeof(targetVID), &bytesWritten, NULL);

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_NO_DATA || error == ERROR_PIPE_NOT_CONNECTED)
            connectedToClient = false;
    }
}

uint32_t PipeServer::ReadFakeVID() {
    if (!connectedFromClient && hPipeFromClient != INVALID_HANDLE_VALUE) {
        BOOL connectResult = ConnectNamedPipe(hPipeFromClient, NULL);
        if (connectResult || GetLastError() == ERROR_PIPE_CONNECTED)
            connectedFromClient = true;
    }

    if (!connectedFromClient) return 0;

    uint32_t fakeVID = 0;
    DWORD bytesRead;
    BOOL result = ReadFile(hPipeFromClient, &fakeVID, sizeof(fakeVID), &bytesRead, NULL);

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED)
            connectedFromClient = false;
        return 0;
    }

    return (bytesRead == sizeof(fakeVID) && fakeVID > 0) ? fakeVID : 0;
}

bool PipeServer::IsConnected() const {
    bool toClientReady = connectedToClient;
    bool fromClientReady = connectedFromClient;

    if (!toClientReady && hPipeToClient != INVALID_HANDLE_VALUE) {
        BOOL connectResult = ConnectNamedPipe(hPipeToClient, NULL);
        if (connectResult || GetLastError() == ERROR_PIPE_CONNECTED)
            const_cast<bool&>(connectedToClient) = true;
    }

    if (!fromClientReady && hPipeFromClient != INVALID_HANDLE_VALUE) {
        BOOL connectResult = ConnectNamedPipe(hPipeFromClient, NULL);
        if (connectResult || GetLastError() == ERROR_PIPE_CONNECTED)
            const_cast<bool&>(connectedFromClient) = true;
    }

    return connectedToClient || connectedFromClient;
}

void PipeServer::Disconnect() {
    if (hPipeToClient != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipeToClient);
        connectedToClient = false;
    }
    if (hPipeFromClient != INVALID_HANDLE_VALUE) {
        DisconnectNamedPipe(hPipeFromClient);
        connectedFromClient = false;
    }
}


PipeClient::PipeClient() :
    hPipeFromServer(INVALID_HANDLE_VALUE), hPipeToServer(INVALID_HANDLE_VALUE),
    connectedFromServer(false), connectedToServer(false) {
}

PipeClient::~PipeClient() {
    if (hPipeFromServer != INVALID_HANDLE_VALUE)
        CloseHandle(hPipeFromServer);
    if (hPipeToServer != INVALID_HANDLE_VALUE)
        CloseHandle(hPipeToServer);
}

bool PipeClient::Initialize() {
    for (int i = 0; i < 5; i++) {
        hPipeFromServer = CreateFile(TEXT("\\\\.\\pipe\\mt2shared"),
            GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hPipeFromServer != INVALID_HANDLE_VALUE) {
            connectedFromServer = true;
            break;
        }

        if (GetLastError() == ERROR_PIPE_BUSY)
            WaitNamedPipe(TEXT("\\\\.\\pipe\\mt2shared"), 2000);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    for (int i = 0; i < 5; i++) {
        hPipeToServer = CreateFile(TEXT("\\\\.\\pipe\\mt2shared"),
            GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

        if (hPipeToServer != INVALID_HANDLE_VALUE) {
            connectedToServer = true;
            break;
        }

        if (GetLastError() == ERROR_PIPE_BUSY)
            WaitNamedPipe(TEXT("\\\\.\\pipe\\mt2shared"), 2000);

        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return connectedFromServer || connectedToServer;
}

bool PipeClient::Reconnect() {
    if (hPipeFromServer != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipeFromServer);
        hPipeFromServer = INVALID_HANDLE_VALUE;
    }
    if (hPipeToServer != INVALID_HANDLE_VALUE) {
        CloseHandle(hPipeToServer);
        hPipeToServer = INVALID_HANDLE_VALUE;
    }
    connectedFromServer = false;
    connectedToServer = false;
    return Initialize();
}

uint32_t PipeClient::ReadTargetVID() {
    if (!connectedFromServer) return 0;

    uint32_t targetVID = 0;
    DWORD bytesRead;
    BOOL result = ReadFile(hPipeFromServer, &targetVID, sizeof(targetVID), &bytesRead, NULL);

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED)
            connectedFromServer = false;
        return 0;
    }

    return (bytesRead == sizeof(targetVID)) ? targetVID : 0;
}

void PipeClient::SendFakeVID(uint32_t fakeVID) {
    if (!connectedToServer) return;

    DWORD bytesWritten;
    BOOL result = WriteFile(hPipeToServer, &fakeVID, sizeof(fakeVID), &bytesWritten, NULL);

    if (!result) {
        DWORD error = GetLastError();
        if (error == ERROR_BROKEN_PIPE || error == ERROR_PIPE_NOT_CONNECTED)
            connectedToServer = false;
    }
}

bool PipeClient::IsConnected() const {
    return connectedFromServer || connectedToServer;
}