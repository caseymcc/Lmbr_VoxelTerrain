#ifndef _voxigen_meshHandler_h
#define _voxigen_meshHandler_h

#include "voxelTerrainChunk.h"

#include <memory>
#include <vector>

struct MeshRequest
{
    enum
    {
        BuildMesh,
        ReturnMesh
    } type;

    VoxelTerrainChunk *renderer;
    voxigen::ChunkTextureMesh *mesh;
};
typedef std::vector<MeshRequest *> MeshRequests;

class MeshHandler
{
public:
    MeshHandler();

    void start();
    void stop();
    void processThread();

    //add request to cached queue, request do not start until they
    //are pushed to the thread queue with updateQueues
    void addMeshRequest(VoxelTerrainChunk *renderer);

    //
    void addReturnMesh(voxigen::ChunkTextureMesh *mesh);

    void returnMeshRequest(MeshRequest *meshRequest);

    //pushes cached request to thread queue, retrieves completed
    //request at the same time
    void updateQueues(MeshRequests &completedQueue);

private:
    MeshRequest *getFreeMeshRequest();

    bool buildMesh(MeshRequest *request);
    bool returnMesh(MeshRequest *request);

    voxigen::ChunkTextureMesh *getFreeMesh();

    std::atomic<int> m_requestAvail;
    std::thread m_thread;
    std::condition_variable m_event;

    //expected to be accessed from only the from one thread normally the main thread
    MeshRequests m_requestCache;
    
    //can only be accessed under lock
    bool m_run;
    MeshRequests m_requestQueue;
    MeshRequests m_completedQueue;
    std::mutex m_queueMutex;

    //
    typedef std::unique_ptr<MeshRequest> UniqueMeshRequest;
    std::vector<UniqueMeshRequest> m_mesheRequest;
    std::vector<MeshRequest *> m_freeMesheRequests;

    typedef std::unique_ptr<voxigen::ChunkTextureMesh> UniqueChunkTextureMesh;
    std::vector<UniqueChunkTextureMesh> m_meshes;
    std::vector<voxigen::ChunkTextureMesh *> m_freeMeshes;
};

#endif//_voxigen_meshHandler_h