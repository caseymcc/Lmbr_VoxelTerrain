
#include "StdAfx.h"
#include "voxelTerrain.h"

#include <generic/fileIO.h>
#include <voxigen/equiRectWorldGenerator.h>
#include <voxigen/chunkFunctions.h>

#include <MaterialHelpers.h>

//typedef voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<VoxelWorld::ChunkType>> EquiRectWorldGenerator;

//force generator instantiation
template voxigen::GeneratorTemplate<voxigen::EquiRectWorldGenerator<VoxelWorld>>;

const char *VoxelTerrain::m_name="VoxelTerrain";

namespace generic
{

struct CryPakHandle
{
    AZ::IO::HandleType file;
    ICryPak *cryPak;
};

struct CryPakIO
{
    typedef CryPakHandle Type;
};

template<>
inline typename CryPakIO::Type *open<CryPakIO>(std::string filename, std::string mode, void *userData)
{
    CryPakIO::Type *pakHandle=new CryPakIO::Type();

    std::string ext=generic::getExtension(filename);

    pakHandle->cryPak=(ICryPak *)userData;
    pakHandle->file=pakHandle->cryPak->FOpen(filename.c_str(), "rb");

    return pakHandle;
}

template<>
inline void close<CryPakIO>(typename CryPakIO::Type *type)
{
    type->cryPak->FClose(type->file);
    free(type);
    type=nullptr;
}

template<>
inline size_t read<CryPakIO>(void *ptr, size_t size, size_t count, typename CryPakIO::Type *type)
{
    return type->cryPak->FReadRaw(ptr, size, count, type->file);
}

template<>
inline size_t write<CryPakIO>(void *ptr, size_t size, size_t count, typename CryPakIO::Type *type)
{
    return type->cryPak->FWrite(ptr, size, count, type->file);
}

template<>
size_t size<CryPakIO>(typename CryPakIO::Type *type)
{
    return type->cryPak->FGetSize(type->file);
}

}//namespace generic

VoxelTerrain::VoxelTerrain(const STerrainInfo& TerrainInfo):
    m_activeVolume(&m_world, &m_descriptors,
        std::bind(&VoxelTerrain::getFreeVoxelChunk, this),
        std::bind(&VoxelTerrain::releaseVoxelChunk, this, std::placeholders::_1)),
    m_activeRegionVolume(&m_world, &m_descriptors,
        std::bind(&VoxelTerrain::getFreeVoxelRegion, this),
        std::bind(&VoxelTerrain::releaseVoxelRegion, this, std::placeholders::_1))
{
    setViewRadius(255.0f);

//    m_whiteTextureId=GetRenderer()->EF_LoadTexture("EngineAssets/Textures/white.dds", FT_DONT_STREAM)->GetTextureID();
    m_whiteTextureId=gEnv->pRenderer->EF_LoadTexture("EngineAssets/Textures/white.dds", FT_DONT_STREAM)->GetTextureID();

    {
        SInputShaderResources Res;

        Res.m_LMaterial.m_Opacity=1.0f;
//        m_material=MakeSystemMaterialFromShader("VoxelTerrain", &Res);
//        m_material=Get3DEngine()->GetMaterialManager()->LoadMaterial("Materials/material_voxelterrain_default");
        m_material=gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/material_voxelterrain_default");
    }

//    AZ::Data::AssetManager &assetManager=AZ::Data::AssetManager::Instance();
//    struct _finddata_t fileinfo;
    
//    intptr_t handle=gEnv->pCryPak->FindFirst("texturePacks/*.*", &fileinfo);
//    int count=0;
//
//    if(handle!=-1)
//    {
//        do
//        {
//            count++;
//        } while(gEnv->pCryPak->FindNext(handle, &fileinfo)>=0);
//
//        gEnv->pCryPak->FindClose(handle);
//    }
    voxigen::SharedTexturePack texturePack=voxigen::generateTexturePack<generic::CryPakIO>("TexturePacks/SoA_Default", (void *)gEnv->pCryPak);

    std::vector<std::string> blockNames={"dirt", "grass", "mud", "mud_dry", "sand", "clay", "cobblestone", "stone", "granite", "slate", "snow"};

    m_textureAtlas=voxigen::generateTextureAtlas<generic::CryPakIO>(blockNames, *texturePack, (void *)gEnv->pCryPak);
    const imglib::SimpleImage &image=m_textureAtlas->getImage();

//build material
    m_terrainTexture=gEnv->pRenderer->Create2DTexture("VoxelTerrainTexture", image.width, image.height, 1, FT_STATE_CLAMP|FT_NOMIPS, image.data, eTF_R8G8B8A8);
//    m_voxelMaterial=gEnv->p3DEngine->GetMaterialManager()->CreateMaterial("Terrain.Voxel");
//    m_voxelMaterial->SetShaderName("Terrain.Layer");
//    m_voxelMaterial->SetFlags(MTL_64BIT_SHADERGENMASK|MTL_FLAG_NOSHADOW);
    m_voxelMaterial=gEnv->p3DEngine->GetMaterialManager()->LoadMaterial("Materials/material_terrain_default");
    uint8 diffuseSlot=gEnv->p3DEngine->GetMaterialHelpers().FindTexSlot("Diffuse"); //Bumpmap, Specular

    SShaderItem &shaderItem=m_material->GetShaderItem();
    IRenderShaderResources *shaderResources=shaderItem.m_pShaderResources;
//    SInputShaderResources &shaderResources=m_voxelMaterial->GetShaderResources();
    TexturesResourcesMap *pTextureReourcesMap=shaderResources->GetTexturesResourceMap();
    SEfResTexture *textureResource=&(*pTextureReourcesMap)[diffuseSlot];
    
    textureResource->m_Sampler.m_pITex=m_terrainTexture;
//

    //memory for handling mesh vertex conversion
    size_t vertexes=64*64*16*6*4;//one extra vertex in each dimension
    size_t indexes=64*64*16*6*2*3;//6 faces 2 tris per face 3 indexes per tri

    m_vertexBuffer=allocateAlignedVerticesBuffer<vtx_idx, SVF_P3F_C4B_T2F, TARGET_DEFAULT_ALIGN>(indexes, vertexes);
//    m_vertexBuffer=allocateAlignedVerticesBuffer<vtx_idx, SVF_P3S_N4B_C4B_T2S, TARGET_DEFAULT_ALIGN>(indexes, vertexes);

//    m_updateThreadRun=true;
//    m_updateThread=std::thread(std::bind(&VoxelTerrain::updateThread, this));
    m_meshHandler.start();
}

VoxelTerrain::~VoxelTerrain()
{
    {
        std::unique_lock<std::mutex> lock(m_updateMutex);
        m_updateThreadRun=false;
    }

    m_meshHandler.stop();

    m_updateEvent.notify_all();
    m_updateThread.join();

    free(m_vertexBuffer);
}

bool VoxelTerrain::SetCompiledData(byte* pData, int nDataSize, std::vector<IStatObj*>** ppStatObjTable, std::vector<_smart_ptr<IMaterial>>** ppMatTable, bool bHotUpdate, SHotUpdateInfo* pExportInfo)
{
//    STerrainChunkHeader* pTerrainChunkHeader=(STerrainChunkHeader*)pData;
//    STerrainInfo* pTerrainInfo=(STerrainInfo*)(pData+sizeof(STerrainChunkHeader));
//    SwapEndian(*pTerrainChunkHeader, eLittleEndian);
//
//    pData+=(sizeof(STerrainChunkHeader)+sizeof(STerrainInfo));
//    nDataSize-=(sizeof(STerrainChunkHeader)+sizeof(STerrainInfo));
//    return Load_T(pData, nDataSize, pTerrainChunkHeader, pTerrainInfo, ppStatObjTable, ppMatTable, bHotUpdate, pExportInfo);
    return true;
}

bool VoxelTerrain::GetCompiledData(byte* pData, int nDataSize, std::vector<IStatObj*>** ppStatObjTable, std::vector<_smart_ptr<IMaterial>>** ppMatTable, std::vector<struct IStatInstGroup*>** ppStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo)
{
# if !ENGINE_ENABLE_COMPILATION
    CryFatalError("serialization code removed, please enable ENGINE_ENABLE_COMPILATION in Cry3DEngine/StdAfx.h");
    return false;
# else
    bool bHMap(!pExportInfo||pExportInfo->nHeigtmap);
    bool bObjs(!pExportInfo||pExportInfo->nObjTypeMask);

    // write header
    STerrainChunkHeader* pTerrainChunkHeader=(STerrainChunkHeader*)pData;
    pTerrainChunkHeader->nVersion=TERRAIN_CHUNK_VERSION;
    pTerrainChunkHeader->nDummy=0;

    pTerrainChunkHeader->nFlags=(eEndian==eBigEndian)?SERIALIZATION_FLAG_BIG_ENDIAN:0;
    pTerrainChunkHeader->nFlags|=SERIALIZATION_FLAG_SECTOR_PALETTES;

    pTerrainChunkHeader->nFlags2=(gEnv->p3DEngine->IsAreaActivationInUse()?TCH_FLAG2_AREA_ACTIVATION_IN_USE:0);
    pTerrainChunkHeader->nChunkSize=nDataSize;

    STerrainInfo *pTerrainInfo=(STerrainInfo*)(pData+sizeof(STerrainChunkHeader));

    pTerrainInfo->type=m_terrainTypeId;
    pTerrainInfo->nTerrainSizeX_InUnits=m_descriptors.m_size.x;
    pTerrainInfo->nTerrainSizeY_InUnits=m_descriptors.m_size.y;
    pTerrainInfo->nTerrainSizeZ_InUnits=m_descriptors.m_size.z;
    pTerrainInfo->nUnitSize_InMeters=1;
    pTerrainInfo->nSectorSize_InMeters=m_descriptors.m_regionSize.x;
    pTerrainInfo->nSectorSizeY_InMeters=m_descriptors.m_regionSize.y;
    pTerrainInfo->nSectorSizeZ_InMeters=m_descriptors.m_regionSize.z;
    pTerrainInfo->nSectorsTableSize_InSectors=0;
    pTerrainInfo->fHeightmapZRatio=1.0f;
    pTerrainInfo->fOceanWaterLevel=0.0f;// m_descriptors.m_seaLevel;

    SwapEndian(*pTerrainChunkHeader, eEndian);
    SwapEndian(*pTerrainInfo, eEndian);

    UPDATE_PTR_AND_SIZE(pData, nDataSize, sizeof(STerrainChunkHeader)+sizeof(STerrainInfo));

    return (nDataSize==0);
# endif
}

int VoxelTerrain::GetCompiledDataSize(SHotUpdateInfo* pExportInfo)
{
    return sizeof(STerrainChunkHeader)+sizeof(STerrainInfo);
}

void VoxelTerrain::GetStatObjAndMatTables(DynArray<IStatObj*>* pStatObjTable, DynArray<_smart_ptr<IMaterial>>* pMatTable, DynArray<IStatInstGroup*>* pStatInstGroupTable, uint32 nObjTypeMask)
{}

void VoxelTerrain::SetTerrainElevation(int left, int bottom, int areaSize, const float* heightmap, int weightmapSize, const SurfaceWeight* surfaceWeightSet)
{}

void VoxelTerrain::SetOceanWaterLevel(float fOceanWaterLevel)
{}

void VoxelTerrain::ChangeOceanMaterial(_smart_ptr<IMaterial> pMat)
{}

IRenderNode *VoxelTerrain::AddVegetationInstance(int nStaticGroupID, const Vec3 &vPos, const float fScale, uint8 ucBright, uint8 angle, uint8 angleX, uint8 angleY)
{
    return nullptr;
}

bool VoxelTerrain::HasRootNode()
{
    return false;
}

AABB VoxelTerrain::GetRootBBoxVirtual()
{
    return AABB();
}

//CTerrainNode *VoxelTerrain::GetRootNode()
//{
//    return nullptr;
//}

//CTerrainNode *VoxelTerrain::GetLeafNodeAt(Meter x, Meter y)
//{
//    return nullptr;
//}


int VoxelTerrain::GetTerrainSize() const
{
    glm::ivec3 worldSize=m_world.size();

    return std::max(std::max(worldSize.x, worldSize.y), worldSize.z);
}

int VoxelTerrain::GetSectorSize() const
{
    const VoxelWorld::DescriptorType  &descriptors=m_world.getDescriptors();
    glm::ivec3 regionSize=descriptors.getRegionCellSize();

    return std::max(std::max(regionSize.x, regionSize.y), regionSize.z);
}

Vec3i VoxelTerrain::GetSectorSizeVector() const
{
    const VoxelWorld::DescriptorType  &descriptors=m_world.getDescriptors();
    glm::ivec3 regionSize=descriptors.getRegionCellSize();

    return Vec3i(regionSize.x, regionSize.y, regionSize.z);
}

int VoxelTerrain::GetHeightMapUnitSize() const
{
    return 1;
}

int VoxelTerrain::GetTerrainTextureNodeSizeMeters()
{
    return 0;
}

void VoxelTerrain::GetResourceMemoryUsage(ICrySizer* pSizer, const AABB &crstAABB)
{
}

//const int VoxelTerrain::GetUnitToSectorBitShift()
//{
//    return 0;
//}
//
//const int VoxelTerrain::GetMeterToUnitBitShift const
//{
//    return 0;
//}

float VoxelTerrain::GetBilinearZ(MeterF xWS, MeterF yWS) const
{
    return 0.0f;
}

float VoxelTerrain::GetZ(Meter x, Meter y) const
{
    return 0.0f;
}

bool VoxelTerrain::IsHole(Meter x, Meter y) const
{
    return false;
}

bool VoxelTerrain::Recompile_Modified_Incrementaly_RoadRenderNodes()
{
    return false;
}

void VoxelTerrain::RemoveAllStaticObjects()
{}

bool VoxelTerrain::RemoveObjectsInArea(Vec3 vExploPos, float fExploRadius)
{
    return false;
}

void VoxelTerrain::MarkAllSectorsAsUncompiled()
{}


void VoxelTerrain::CheckVis(const SRenderingPassInfo &passInfo)
{}

void VoxelTerrain::AddVisSector(CTerrainNode* newsec)
{

}

void VoxelTerrain::ClearVisSectors()
{}

void VoxelTerrain::UpdateNodesIncrementaly(const SRenderingPassInfo &passInfo)
{}

void VoxelTerrain::DrawVisibleSectors(const SRenderingPassInfo &passInfo)
{
    AZ_TRACE_METHOD();

    SRendParams renderParams;
    
    renderParams.AmbientColor=ColorF(1, 1, 1, 1);

    if(!passInfo.RenderTerrain()||!gEnv->p3DEngine->GetShowTerrainSurface())
        return;

    glm::ivec3 regionIndex=m_activeVolume.relativeCameraIndex();
    const auto &volume=m_activeVolume.getVolume();

    for(auto renderer:volume)
    {
        if(renderer)
        {
            if(renderer->IsMeshReady())
            {
                renderParams.pMatrix=&renderer->GetTransform();

                renderer->RenderMesh(renderParams, passInfo);
            }
        }
    }
}

void VoxelTerrain::UpdateSectorMeshes(const SRenderingPassInfo &passInfo)
{
    const CCamera &camera=passInfo.GetCamera();
    Vec3i sector=camera.GetSector();
    Vec3 position=camera.GetPosition();

    glm::ivec3 cameraRegion(sector.x, sector.y, sector.z);
    glm::ivec3 cameraChunk=m_world.getChunkIndex({position.x, position.y, position.z});

    
//    //if region has changed we need to update our chunk search rings
    if(m_currentRegion!=cameraRegion || m_currentChunk!=cameraChunk || m_updateSearch)
    {
        RegionChunkIndexType cameraIndex;

        cameraIndex.region=cameraRegion;
        cameraIndex.chunk=cameraChunk;

        m_activeVolume.updateCamera(cameraIndex);

        std::vector<VoxelTerrainChunk *> loadRenderer;
        std::vector<VoxelTerrainChunk *> releaseRenderer;

        m_activeVolume.update(cameraIndex, loadRenderer, releaseRenderer);

        for(auto renderer:loadRenderer)
        {
            VoxelChunkHandle handle=renderer->getHandle();

            if(handle->state()!=voxigen::HandleState::Memory)
            {
                //make sure we are not already requesting something from it
                if(handle->action()==voxigen::HandleAction::Idle)
                    m_world.loadChunk(handle, 0);
            }
            else
            {
                if(!renderer->IsMeshReady())
                    requestMesh(renderer);
            }
        }
        loadRenderer.clear();

        for(auto renderer:releaseRenderer)
        {
            m_activeVolume.releaseInfo(renderer);
        }
        releaseRenderer.clear();

//        m_activeRegionVolume.updateCamera(cameraIndex);
//        m_activeRegionVolume.update(cameraIndex);

        //updating the active volume would have cached chunk 
        //update requests, this will push them to the process thread
        m_world.updateProcessQueue();
    }

    //check if any chunks are ready from read thread
    UpdateLoadedChunks(passInfo);

    //check if any chunks are ready to be meshed

}

void VoxelTerrain::updateThread()
{
    glm::ivec3 cameraRegion;
    glm::ivec3 cameraChunk;

    while(m_updateThreadRun)
    {
        {
            std::unique_lock<std::mutex> lock(m_updateMutex);

            cameraRegion=m_cameraRegion;
            cameraChunk=m_cameraChunk;

            if(m_currentRegion==cameraRegion && m_currentChunk==cameraChunk)
            {
                m_updateEvent.wait(lock);
                continue;
            }
        }

        if(m_currentRegion!=cameraRegion||m_currentChunk!=cameraChunk||m_updateSearch)
            updateChunkSearch(cameraRegion, cameraChunk);
    }
}

void VoxelTerrain::CheckNodesGeomUnload(const SRenderingPassInfo &passInfo)
{}

bool VoxelTerrain::RenderArea(Vec3 vPos, float fRadius, _smart_ptr<IRenderMesh> &pRenderMesh,
    CRenderObject *pObj, _smart_ptr<IMaterial> pMaterial, const char *szComment, float *pCustomData,
    Plane *planes, const SRenderingPassInfo &passInfo)
{
    return false;
}

void VoxelTerrain::IntersectWithShadowFrustum(PodArray<CTerrainNode*>* plstResult, ShadowMapFrustum* pFrustum, const SRenderingPassInfo& passInfo)
{}

void VoxelTerrain::IntersectWithBox(const AABB& aabbBox, PodArray<CTerrainNode*>* plstResult)
{

}

CTerrainNode *VoxelTerrain::FindMinNodeContainingBox(const AABB &someBox)
{
    return nullptr;
}

bool VoxelTerrain::RayTrace(Vec3 const& vStart, Vec3 const& vEnd, SRayTrace* prt)
{
    return false;
}

bool VoxelTerrain::IsOceanVisible() const
{
    return false;
}

COcean *VoxelTerrain::GetOcean()
{
    return nullptr;
}

float VoxelTerrain::GetWaterLevel()
{
    return 0.0f;
}

float VoxelTerrain::GetDistanceToSectorWithWater() const
{
    return 0.0f;
}

int VoxelTerrain::UpdateOcean(const SRenderingPassInfo &passInfo)
{
    return 0;
}

int VoxelTerrain::RenderOcean(const SRenderingPassInfo &passInfo)
{
    return 0;
}

void VoxelTerrain::InitTerrainWater(_smart_ptr<IMaterial> pTerrainWaterShader)
{}


void VoxelTerrain::ClearTextureSets()
{}

bool VoxelTerrain::IsTextureStreamingInProgress() const
{
    return false;
}

bool VoxelTerrain::TryGetTextureStatistics(MacroTexture::TileStatistics &statistics) const
{
    return false;
}

//MacroTexture *VoxelTerrain::GetMacroTexture()
//{
//    return nullptr;
//}
//
//int VoxelTerrain::GetWhiteTexId() const
//{
//    return 0;
//}

void VoxelTerrain::CloseTerrainTextureFile()
{}

void VoxelTerrain::SetTerrainSectorTexture(int nTexSectorX, int nTexSectorY, unsigned int textureId, bool bMergeNotAllowed)
{}

Vec3 VoxelTerrain::GetTerrainSurfaceNormal(Vec3 vPos, float fRange)
{
    return Vec3();
}

bool VoxelTerrain::IsMeshQuadFlipped(const Meter x, const Meter y, const Meter nUnitSize) const
{
    return false;
}

void VoxelTerrain::GetTerrainAlignmentMatrix(const Vec3& vPos, const float amount, Matrix33& matrix33)
{

}

ITerrain::SurfaceWeight VoxelTerrain::GetSurfaceWeight(Meter x, Meter y) const
{
    return ITerrain::SurfaceWeight();
}

//CTerrainUpdateDispatcher *VoxelTerrain::GetTerrainUpdateDispatcher() const
//{
//    return nullptr;
//}

int VoxelTerrain::GetActiveProcObjNodesCount()
{
    return 0;
}

//void VoxelTerrain::ActivateNodeProcObj(CTerrainNode* pNode)
//{
//}

//void VoxelTerrain::SetNodePyramid(int treeLevel, int x, int y, CTerrainNode *node)
//{
//}

void VoxelTerrain::LoadSurfaceTypesFromXML(XmlNodeRef pDoc)
{}

void VoxelTerrain::UpdateSurfaceTypes()
{}

void VoxelTerrain::InitHeightfieldPhysics()
{}

void VoxelTerrain::ResetTerrainVertBuffers()
{}

bool VoxelTerrain::LoadHandle(AZ::IO::HandleType fileHandle, int dataSize, STerrainChunkHeader* chunkHeader, STerrainInfo* terrainInfo, std::vector<struct IStatObj*>** staticObjectTable, std::vector<_smart_ptr<IMaterial> >** materialTable)
{
    return Load(gEnv->p3DEngine->GetLevelFilePath(""), terrainInfo);
}

bool VoxelTerrain::Load(const char *levelPath, STerrainInfo* terrainInfo)
{
    std::string terrainPath=std::string(levelPath)+"/terrain";
    
    ICryPak &cryPak=*gEnv->pCryPak;
    AZ::IO::FileIOBase *fileIO=AZ::IO::FileIOBase::GetDirectInstance(); 

    if(!fileIO->Exists(terrainPath.c_str()))
    {
        fileIO->CreatePath(terrainPath.c_str());

        glm::ivec3 size;

        size.x=terrainInfo->nTerrainSizeX_InUnits;
        size.y=terrainInfo->nTerrainSizeY_InUnits;
        size.z=terrainInfo->nTerrainSizeZ_InUnits;

        m_world.create(terrainPath, "test", size, "EquiRectWorldGenerator");
    }
    else
        m_world.load(terrainPath);

    m_descriptors=m_world.getDescriptors();
    return true;
}

void VoxelTerrain::Save(const char *path)
{
    std::string terrainPath=std::string(path)+"/terrain";

    m_world.saveTo(terrainPath);
}

void VoxelTerrain::setViewRadius(float radius)
{
    buildChunkSearchRings(radius);
    m_updateSearch=true; //force search update
}

void VoxelTerrain::buildChunkSearchRings(float radius)
{
    //this build a search area around center based on 3D rings of the player position
    //this are in relative cell positions from player
    m_viewRadius=radius;
    m_viewRadiusMax=radius*1.5f;

    glm::ivec3 vRadius(radius, radius, radius);

    m_activeVolume.setViewRadius(vRadius);
    m_activeRegionVolume.setViewRadius(vRadius);

    size_t chunkCount=m_activeVolume.getContainerCount();
//    m_freeVoxelChunks.setMaxSize((chunkCount/2)*3);
    m_freeVoxelChunks.setMaxSize(chunkCount*3);

    size_t regionCount=m_activeRegionVolume.getContainerCount();
    m_freeVoxelRegions.setMaxSize((regionCount/2)*3);
    

//    size_t maxChunkRing=(size_t)std::ceil(radius/std::max(std::max(VoxelChunk::sizeX::value, VoxelChunk::sizeY::value), VoxelChunk::sizeZ::value));

//    m_chunkSearchRings.resize(maxChunkRing);
//    m_chunkQueryOrder.resize(maxChunkRing);

//    for(size_t i=0; i<maxChunkRing; ++i)
//        voxigen::ringCube<VoxelChunk>(m_chunkSearchRings[i], i);
}

VoxelTerrainChunk *VoxelTerrain::getFreeVoxelChunk()
{
    VoxelTerrainChunk *chunk=m_freeVoxelChunks.get();

    assert(chunk);
    chunk->SetTerrain(this);
    return chunk;
}

void VoxelTerrain::releaseVoxelChunk(VoxelTerrainChunk *chunk)
{
    m_freeVoxelChunks.release(chunk);
}

VoxelTerrainRegion *VoxelTerrain::getFreeVoxelRegion()
{
    VoxelTerrainRegion *region=m_freeVoxelRegions.get();

    assert(region);
    return region;
}

void VoxelTerrain::releaseVoxelRegion(VoxelTerrainRegion *region)
{
    m_freeVoxelRegions.release(region);
}


struct ChunkQueryOffset
{
    ChunkQueryOffset(size_t queryRing, glm::vec3 &offset):queryRing(queryRing), offset(offset) {}

    size_t queryRing;
    glm::vec3 offset;
};

//void VoxelTerrain::updateChunkSearch(const glm::ivec3 &playerRegionIndex, const glm::ivec3 &playerChunkIndex)
//{
//    m_updateSearch=false;
//
//    glm::ivec3 chunkOffset;
//    std::unordered_map<voxigen::Key::Type, ChunkQueryOffset> chunks;
//    glm::ivec3 index;
//    glm::ivec3 currentRegionIndex;
//
//    glm::ivec3 &regionCellSize=m_world.getDescriptors().m_regionCellSize;
//
//    Matrix34 transform=Matrix34::CreateIdentity();
//
//    //loop through all seach rings
//    for(size_t i=0; i<m_chunkSearchRings.size(); ++i)
//    {
//        std::vector<glm::ivec3> &chunkSearchRing=m_chunkSearchRings[i];
//
//        for(size_t j=0; j<chunkSearchRing.size(); ++j)
//        {
//            index=playerChunkIndex+chunkSearchRing[j];
//            currentRegionIndex=playerRegionIndex;
//
//            glm::vec3 regionOffset=m_world.getDescriptors().adjustRegion(currentRegionIndex, index);
//            voxigen::Key key=m_world.getHashes(currentRegionIndex, index);
//
//            chunks.insert(std::pair<voxigen::Key::Type, ChunkQueryOffset>(key.hash, ChunkQueryOffset(i, regionOffset)));
//        }
//    }
//
//    //update region renders and chunks
//    for(auto regionIter=m_regionRenderers.begin(); regionIter!=m_regionRenderers.end(); )
//    {
//        VoxelTerrainRegion &regionRenderer=regionIter->second;
//        Vec3i regionOffset=regionRenderer.Index()-sector;
//
//        regionRenderer.SetOffset(regionOffset.CompMul(Vec3i(regionCellSize.x, regionCellSize.y, regionCellSize.z)));
//
//        auto &chunkRendererMap=regionRenderer.ChunkRenderers();
//
//        for(auto chunkIter=chunkRendererMap.begin(); chunkIter!=chunkRendererMap.end(); )
//        {
//            auto *chunkRenderer=chunkIter->second;
//            voxigen::Key key(chunkRenderer->GetRegionHash(), chunkRenderer->GetChunkHash());
//
//            glm::ivec3 regionIndex=m_world.getDescriptors().regionIndex(key.regionHash);
//            glm::ivec3 chunkIndex=m_world.getDescriptors().chunkIndex(key.chunkHash);
//            float chunkDistance=m_world.getDescriptors().distance(playerRegionIndex, playerChunkIndex, regionIndex, chunkIndex);
//
//            //chunk outside of range so invalidate
//            if(chunkDistance > m_viewRadiusMax)
//            {
//                if(!chunkRenderer->IsInUse()) //need to keep if in use in another thread
//                {
//                    chunkRenderer->Invalidate();
//                    chunkIter=chunkRendererMap.erase(chunkIter);
//                    m_freeChunkRenderers.push_back(chunkRenderer);
//                    continue;
//                }
//            }
//            else
//            {
//                auto iter=chunks.find(key.hash);
//
//                if(iter!=chunks.end())
//                {
//                    chunkOffset=((playerRegionIndex-regionIndex)*m_world.getDescriptors().m_regionCellSize)+chunkIndex;
//
//                    transform.SetTranslation(Vec3((float)chunkOffset.x, (float)chunkOffset.y, (float)chunkOffset.z));
//                    chunkRenderer->SetTransform(transform);
//
//                    m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//                    chunks.erase(iter);
//
//                    CryLog("Chunk (%x, %x) erased", chunkRenderer->GetRegionHash(), chunkRenderer->GetChunkHash());
//                }
//            }
//            ++chunkIter;
//        }
//
//        if(chunkRendererMap.empty())
//            regionIter=m_regionRenderers.erase(regionIter);
//        else
//            ++regionIter;
//    }
//
//    for(auto iter=chunks.begin(); iter!=chunks.end(); ++iter)
//    {
//        VoxelTerrainChunk *chunkRenderer=GetFreeChunkRenderer();
//
//        if(chunkRenderer==nullptr)
//            continue;
//
//        voxigen::Key key(iter->first);
//        VoxelRegionHandle regionHandle=m_world.getRegion(key.regionHash);
//        VoxelChunkHandle chunkHandle=m_world.getChunk(key.regionHash, key.chunkHash);
//
//        chunkRenderer->SetChunk(chunkHandle);
//        m_chunkQueryOrder[iter->second.queryRing].push_back(chunkRenderer);
//
//        auto regionIter=m_regionRenderers.find(key.regionHash);
//
//        if(regionIter==m_regionRenderers.end())
//        {
//            glm::ivec3 index=m_world.getRegionIndex(key.regionHash);
//
//            auto interResult=m_regionRenderers.insert(RegionRendererMap::value_type(key.regionHash, VoxelTerrainRegion(this, key.regionHash, Vec3i(index.x, index.y, index.z), Vec3(iter->second.offset.x, iter->second.offset.y, iter->second.offset.z))));
//
//            assert(interResult.second);
//            regionIter=interResult.first;
//        }
//
//        chunkRenderer->SetParent(&regionIter->second);
//        regionIter->second.ChunkRenderers().insert(VoxelTerrainRegion::ChunkRendererMap::value_type(key.chunkHash, chunkRenderer));
//
//        //request the chunk
//        glm::ivec3 &regionIndex=m_world.getDescriptors().regionIndex(key.regionHash);
//        glm::ivec3 &chunkIndex=chunkHandle->regionOffset();
//
//        chunkOffset=((regionIndex-playerRegionIndex)*m_world.getDescriptors().m_regionCellSize)+chunkIndex;
//
//        transform.SetTranslation(Vec3((float)chunkOffset.x, (float)chunkOffset.y, (float)chunkOffset.z));
//
//        chunkRenderer->SetTransform(transform);
//        chunkRenderer->IncrementInUse();//about to send this thing of to a thread for loading, so lets remember someone else is using
//
//        CryLog("Chunk (%x, %x) request load", chunkHandle->regionHash(), chunkHandle->hash());
//
//        m_world.loadChunk(chunkHandle, 0);
//    }
//
//    m_currentRegion=playerRegionIndex;
//    m_currentChunk=playerChunkIndex;
//}

void VoxelTerrain::updateChunkSearch(const glm::ivec3 &playerRegionIndex, const glm::ivec3 &playerChunkIndex)
{
    
}

void VoxelTerrain::requestMesh(VoxelTerrainChunk *renderer)
{
    //check if chunk is empty
    if(!renderer->empty())
    {
        //make sure to flag chunk as in use, keep it from being deleted
        renderer->setAction(voxigen::RenderAction::Meshing);
        renderer->setTextureAtlas(m_textureAtlas);

        //Chunk ready so lets get a mesh
        m_meshHandler.addMeshRequest(renderer);
    }
}

void VoxelTerrain::UpdateLoadedChunks(const SRenderingPassInfo &passInfo)
{
    std::vector<voxigen::RegionHash> updatedRegions;
    std::vector<voxigen::Key> updatedChunks;

    //has any load request finished
    m_world.getUpdated(updatedRegions, updatedChunks);

    if(!updatedChunks.empty())
    {
        VoxelTerrainChunk *renderer;
        RegionChunkIndexType index;
        const VoxelWorld::DescriptorType &descriptors=m_world.getDescriptors();

        for(size_t i=0; i<updatedChunks.size(); ++i)
        {
            voxigen::Key &key=updatedChunks[i];

            CryLog("Chunk (%x, %x) loaded", key.regionHash, key.chunkHash);

            index.region=descriptors.getRegionIndex(key.regionHash);
            index.chunk=descriptors.getChunkIndex(key.chunkHash);

            renderer=m_activeVolume.getRenderInfo(index);
            
            if(renderer==nullptr)
                continue;

            requestMesh(renderer);

//            //check if chunk is empty
//            if(!renderer->empty())
//            {
//                //make sure to flag chunk as in use, keep it from being deleted
//                renderer->setAction(voxigen::RenderAction::Meshing);
//                renderer->setTextureAtlas(m_textureAtlas);
//
//                //Chunk ready so lets get a mesh
//                m_meshHandler.addMeshRequest(renderer);
//            }
        }
    }

    MeshRequests completedRequests;

    //flush queues and get completed request
    m_meshHandler.updateQueues(completedRequests);

    if(!completedRequests.empty())
    {
        for(MeshRequest *request:completedRequests)
        {
            if(request->type==MeshRequest::BuildMesh)
            {
                VoxelTerrainChunk *renderer=request->renderer;

                renderer->buildMesh(passInfo, m_vertexBuffer, request->mesh);
                renderer->setAction(voxigen::RenderAction::Idle);

                m_meshHandler.addReturnMesh(request->mesh);
            }
            m_meshHandler.returnMeshRequest(request);
        }
    }
}


//VoxelTerrainChunk *VoxelTerrain::GetFreeChunkRenderer()
//{
//    if(m_freeChunkRenderers.empty())
//    {
//        size_t minRendererCount=0;
//
//        for(size_t i=0; i<m_chunkSearchRings.size(); ++i)
//            minRendererCount+=m_chunkSearchRings[i].size();
//        minRendererCount=(minRendererCount*3)/2;
//
//        if(minRendererCount<m_chunkRenderers.size())
//            minRendererCount=m_chunkRenderers.size()+1;
//
//        if(m_chunkRenderers.size()<minRendererCount)
//        {
//            size_t buildIndex=m_chunkRenderers.size();
//            m_chunkRenderers.resize(minRendererCount);
//
//            //need to setup buffers for new chunks
//            for(size_t i=buildIndex; i<m_chunkRenderers.size(); ++i)
//            {
//                VoxelTerrainChunk *chunkRenderer=new VoxelTerrainChunk();
//
//                m_chunkRenderers[i].reset(chunkRenderer);
//
////                chunkRenderer->SetParent(this);
//
//                m_freeChunkRenderers.push_back(chunkRenderer);
//            }
//        }
//    }
//
//    if(m_freeChunkRenderers.empty())
//        return nullptr;
//
//    VoxelTerrainChunk *renderer=m_freeChunkRenderers.back();
//
//    m_freeChunkRenderers.pop_back();
//    return renderer;
//}
