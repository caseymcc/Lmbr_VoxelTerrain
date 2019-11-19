#include "meshHandler.h"

#include <voxigen/cubicMeshBuilder.h>
#include <deque>

MeshHandler::MeshHandler()
{
    size_t vertexes=64*64*16*6*6*4; //6 faces 4 vertexes
    size_t indexes=64*64*16*6*2*3; //6 faces 2 triangles per face 3 indexes per triangle 

    for(size_t i=0; i<10; ++i)
    {
        voxigen::ChunkTextureMesh *mesh=new voxigen::ChunkTextureMesh();

        mesh->reserve(vertexes, indexes);

        m_freeMeshes.push_back(mesh);
        m_meshes.emplace_back(mesh);
    }
}

void MeshHandler::addMeshRequest(VoxelTerrainChunk *renderer)
{
    MeshRequest *request=getFreeMeshRequest();

    request->type=MeshRequest::BuildMesh;
    request->renderer=renderer;
    request->mesh=nullptr;

    m_requestCache.push_back(request);
}

void MeshHandler::addReturnMesh(voxigen::ChunkTextureMesh *mesh)
{
    MeshRequest *request=getFreeMeshRequest();

    request->type=MeshRequest::ReturnMesh;
    request->renderer=nullptr;
    request->mesh=mesh;

    m_requestCache.push_back(request);
}

void MeshHandler::returnMeshRequest(MeshRequest *meshRequest)
{
    m_freeMesheRequests.push_back(meshRequest);
}

void MeshHandler::updateQueues(MeshRequests &completedQueue)
{
    bool update=false;

    {
        std::unique_lock<std::mutex> lock(m_queueMutex);

        if(!m_requestCache.empty())
        {
            m_requestQueue.insert(m_requestQueue.end(), m_requestCache.begin(), m_requestCache.end());
            m_requestCache.clear();
            update=true;

        }

        if(!m_completedQueue.empty())
        {
            completedQueue.insert(completedQueue.end(), m_completedQueue.begin(), m_completedQueue.end());
            m_completedQueue.clear();
        }
    }

    if(update)
    {
        m_requestAvail=1;
        m_event.notify_all();
    }
}

void MeshHandler::start()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=true;
    }

    m_thread=std::thread(std::bind(&MeshHandler::processThread, this));
}


void MeshHandler::stop()
{
    {
        std::unique_lock<std::mutex> lock(m_queueMutex);
        m_run=false;
    }

    m_event.notify_all();
    m_thread.join();
}


void MeshHandler::processThread()
{
    bool run=true;
    std::deque<MeshRequest *> requestQueue;
    MeshRequests completedQueue;

    while(run)
    {
        bool completed;

//        if(requestQueue.empty())
        if((m_requestAvail > 0) || requestQueue.empty())
        {
            m_requestAvail=0;

            std::unique_lock<std::mutex> lock(m_queueMutex);

            //update running
            run=m_run;

            //check if any new request have been added.
            if(!m_requestQueue.empty())
            {
                requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
                m_requestQueue.clear();
            }

            //while we have the lock update anything that is complete
            if(!completedQueue.empty())
            {
                m_completedQueue.insert(m_completedQueue.end(), completedQueue.begin(), completedQueue.end());
                completedQueue.clear();
            }

            if(run && requestQueue.empty())
                m_event.wait(lock);

            continue;
        }

        MeshRequest *request=nullptr;
        
        //check for ReturnMesh first
        for(auto iter=requestQueue.begin(); iter!=requestQueue.end(); ++iter)
        {
            if((*iter)->type==MeshRequest::ReturnMesh)
            {
                request=(*iter);
                requestQueue.erase(iter);
                break;
            }
        }
        
        if(!request)
        {
            request=requestQueue.front();
            requestQueue.pop_front();
        }

        completed=false;
        switch(request->type)
        {
        case MeshRequest::BuildMesh:
            completed=buildMesh(request);
            break;
        case MeshRequest::ReturnMesh:
            completed=returnMesh(request);
            break;
        }

        if(completed)
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);
//            completedQueue.push_back(request);
            m_completedQueue.push_back(request);
        }
        else//wasn't able to process, lets try again
        {
            std::unique_lock<std::mutex> lock(m_queueMutex);

            requestQueue.push_back(request);
            
            requestQueue.insert(requestQueue.end(), m_requestQueue.begin(), m_requestQueue.end());
            m_requestQueue.clear();
        }
    }
}

bool MeshHandler::buildMesh(MeshRequest *request)
{
    VoxelTerrainChunk *chunk=request->renderer;
    voxigen::ChunkTextureMesh *mesh=getFreeMesh();

    if(!mesh)
        return false;

    mesh->clear();
    mesh->setTextureAtlas(chunk->getTextureAtlas().get());
    voxigen::buildCubicMesh(*mesh, chunk->GetChunk()->chunk());

    request->mesh=mesh;
    return true;
}

bool MeshHandler::returnMesh(MeshRequest *request)
{
    if(request->mesh)
        m_freeMeshes.push_back(request->mesh);
    return true;
}

MeshRequest *MeshHandler::getFreeMeshRequest()
{
    if(m_freeMesheRequests.empty())
    {
        //no free request, grow size
        for(size_t i=0; i<10; ++i)
        {
            MeshRequest *meshRequest=new MeshRequest();

            m_freeMesheRequests.push_back(meshRequest);
            m_mesheRequest.emplace_back(meshRequest);
        }
    }

    MeshRequest *meshRequest=m_freeMesheRequests.back();

    m_freeMesheRequests.pop_back();
    return meshRequest;
}

voxigen::ChunkTextureMesh *MeshHandler::getFreeMesh()
{
    if(m_freeMeshes.empty())
        return nullptr;

    voxigen::ChunkTextureMesh *mesh=m_freeMeshes.back();

    m_freeMeshes.pop_back();
    return mesh;
}
