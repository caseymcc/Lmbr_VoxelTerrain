#ifndef _voxelTerrain_voxelTerrain_h_
#define _voxelTerrain_voxelTerrain_h_
#pragma once

#include <ISerialize.h>
#include <IRenderer.h>
#include <I3DEngine.h>
#include <IEngineTerrain.h>
#include <TerrainFactory.h>
#include <ITexture.h>

#include "voxigenDefine.h"
#include "voxelTerrainRegion.h"
//#include "updateDispatcher.h"
#include "meshHandler.h"

#include <voxigen/activeVolume.h>
#include <voxigen/textureAtlas.h>
#include <unordered_map>
#include <thread>

class VoxelTerrain:
    public RegisterTerrain<VoxelTerrain, IEngineTerrain>//,
    //public Cry3DEngineBase
{
public:
    typedef std::unordered_map<voxigen::RegionHash, VoxelTerrainRegion> RegionRendererMap;
    typedef std::unique_ptr<VoxelTerrainChunk> UniqueChunkRenderer;
//    using Meter = int;
//    using MeterF = float;
//    using Unit = int;

    VoxelTerrain(const STerrainInfo& TerrainInfo);
    ~VoxelTerrain();

    static const char *Name() { return m_name; }

//ITerrain interface
    virtual int GetType() { return m_terrainTypeId; }
    virtual bool SetCompiledData(byte* pData, int nDataSize, std::vector<IStatObj*>** ppStatObjTable, std::vector<_smart_ptr<IMaterial>>** ppMatTable, bool bHotUpdate=false, SHotUpdateInfo* pExportInfo=nullptr);
    virtual bool GetCompiledData(byte* pData, int nDataSize, std::vector<IStatObj*>** ppStatObjTable, std::vector<_smart_ptr<IMaterial>>** ppMatTable, std::vector<struct IStatInstGroup*>** ppStatInstGroupTable, EEndian eEndian, SHotUpdateInfo* pExportInfo=nullptr);
    virtual int GetCompiledDataSize(SHotUpdateInfo* pExportInfo=nullptr);
    virtual void GetStatObjAndMatTables(DynArray<IStatObj*>* pStatObjTable, DynArray<_smart_ptr<IMaterial>>* pMatTable, DynArray<IStatInstGroup*>* pStatInstGroupTable, uint32 nObjTypeMask);
    virtual void SetTerrainElevation(int left, int bottom, int areaSize, const float* heightmap, int weightmapSize, const SurfaceWeight* surfaceWeightSet);
    virtual void SetOceanWaterLevel(float fOceanWaterLevel);
    virtual void ChangeOceanMaterial(_smart_ptr<IMaterial> pMat);
    virtual IRenderNode* AddVegetationInstance(int nStaticGroupID, const Vec3& vPos, const float fScale, uint8 ucBright, uint8 angle, uint8 angleX=0, uint8 angleY=0);

    bool Load(const char *levelPath, STerrainInfo* pTerrainInfo) override;
    Vec3i GetSectorSizeVector() const override;

//IEngineTerrain
    virtual bool HasRootNode();
    virtual AABB GetRootBBoxVirtual();
//    virtual CTerrainNode* GetRootNode();
//    virtual CTerrainNode *GetLeafNodeAt(Meter x, Meter y);

    //stats
    CTerrainNode* GetRootNode() override { return nullptr; }
    int GetTerrainSize() const override;
    int GetSectorSize() const override;
    int GetHeightMapUnitSize() const override;
    virtual int GetTerrainTextureNodeSizeMeters();
    virtual  void GetResourceMemoryUsage(ICrySizer* pSizer, const AABB &crstAABB);

//    virtual const int GetUnitToSectorBitShift();
//    virtual const int GetMeterToUnitBitShift const;
    //edit
    virtual float GetBilinearZ(MeterF xWS, MeterF yWS) const;
    virtual float GetZ(Meter x, Meter y) const;
    virtual bool IsHole(Meter x, Meter y) const;
    virtual bool Recompile_Modified_Incrementaly_RoadRenderNodes();
    virtual void RemoveAllStaticObjects();
    virtual bool RemoveObjectsInArea(Vec3 vExploPos, float fExploRadius);
    virtual void GetObjectsAround(Vec3 vPos, float fRadius, PodArray<struct SRNInfo>* pEntList, bool bSkip_ERF_NO_DECALNODE_DECALS, bool bSkipDynamicObjects) {}
    virtual void MarkAllSectorsAsUncompiled();
    //vis
    virtual void CheckVis(const SRenderingPassInfo &passInfo);
    virtual void AddVisSector(CTerrainNode* newsec);
    virtual void ClearVisSectors();
    virtual void UpdateNodesIncrementaly(const SRenderingPassInfo &passInfo);

    virtual void DrawVisibleSectors(const SRenderingPassInfo &passInfo);
    virtual void UpdateSectorMeshes(const SRenderingPassInfo &passInfo);
    virtual void CheckNodesGeomUnload(const SRenderingPassInfo &passInfo);

    virtual bool RenderArea(Vec3 vPos, float fRadius, _smart_ptr<IRenderMesh> &pRenderMesh,
        CRenderObject *pObj, _smart_ptr<IMaterial> pMaterial, const char *szComment, float *pCustomData,
        Plane *planes, const SRenderingPassInfo &passInfo);
    virtual void IntersectWithShadowFrustum(PodArray<CTerrainNode*>* plstResult, ShadowMapFrustum* pFrustum, const SRenderingPassInfo& passInfo);
    virtual void IntersectWithBox(const AABB& aabbBox, PodArray<CTerrainNode*>* plstResult);
    virtual CTerrainNode* FindMinNodeContainingBox(const AABB& someBox);

    virtual bool RayTrace(Vec3 const& vStart, Vec3 const& vEnd, SRayTrace* prt);
    //ocean
    virtual bool IsOceanVisible() const;
    virtual COcean* GetOcean();
    virtual float GetWaterLevel();
    virtual float GetDistanceToSectorWithWater() const;
    virtual int UpdateOcean(const SRenderingPassInfo &passInfo);
    virtual int RenderOcean(const SRenderingPassInfo &passInfo);
    virtual void InitTerrainWater(_smart_ptr<IMaterial> pTerrainWaterShader);

    //texture
    virtual void ClearTextureSets();
    virtual bool IsTextureStreamingInProgress() const;
    virtual bool TryGetTextureStatistics(MacroTexture::TileStatistics &statistics) const;
//    virtual MacroTexture *GetMacroTexture();
//    virtual int GetWhiteTexId() const;

    //terrain
    virtual void CloseTerrainTextureFile();
    virtual void SetTerrainSectorTexture(int nTexSectorX, int nTexSectorY, unsigned int textureId, bool bMergeNotAllowed);
    virtual Vec3 GetTerrainSurfaceNormal(Vec3 vPos, float fRange);
    virtual bool IsMeshQuadFlipped(const Meter x, const Meter y, const Meter nUnitSize) const;
    virtual void GetTerrainAlignmentMatrix(const Vec3& vPos, const float amount, Matrix33& matrix33);
    void GetMaterials(AZStd::vector<_smart_ptr<IMaterial>>& materials) override { return; }
    virtual ITerrain::SurfaceWeight GetSurfaceWeight(Meter x, Meter y) const;

//    virtual CTerrainUpdateDispatcher *GetTerrainUpdateDispatcher() const;
    //streams
    virtual int GetActiveProcObjNodesCount();
//    virtual void ActivateNodeProcObj(CTerrainNode* pNode);
//    virtual void SetNodePyramid(int treeLevel, int x, int y, CTerrainNode *node);

    //
    virtual void LoadSurfaceTypesFromXML(XmlNodeRef pDoc);
    virtual void UpdateSurfaceTypes();

    //
    virtual void InitHeightfieldPhysics();
    virtual void ResetTerrainVertBuffers();

    //
    bool LoadHandle(AZ::IO::HandleType fileHandle, int nDataSize, STerrainChunkHeader* pTerrainChunkHeader, STerrainInfo* pTerrainInfo, std::vector<struct IStatObj*>** ppStatObjTable, std::vector<_smart_ptr<IMaterial> >** ppMatTable) override;
//    virtual bool Load(const char *levelPath, STerrainInfo* pTerrainInfo);

    int GetWhiteTextureId() { return m_whiteTextureId; }

    
//Local
    void Save(const char *path);
    _smart_ptr<IMaterial> &GetMaterial(){ return m_material; }
//    _smart_ptr<IMaterial> &GetMaterial() { return m_voxelMaterial; }
    ITexture *getTerrainTexture() { return m_terrainTexture; }

    void setViewRadius(float radius);
    void UpdateLoadedChunks(const SRenderingPassInfo &passInfo);

    VoxelTerrainChunk *getFreeVoxelChunk();
    void releaseVoxelChunk(VoxelTerrainChunk *chunk);

    VoxelTerrainRegion *getFreeVoxelRegion();
    void releaseVoxelRegion(VoxelTerrainRegion *region);

    void buildChunkSearchRings(float radius);
    void updateChunkSearch(const glm::ivec3 &playerRegionIndex, const glm::ivec3 &playerChunkIndex);
//    VoxelTerrainChunk *GetFreeChunkRenderer();

    void updateThread();

private:
    void requestMesh(VoxelTerrainChunk *renderer);

    static const char *m_name;

    voxigen::GridDescriptors<VoxelWorld> m_descriptors;
    VoxelWorld m_world;
//    voxigen::RenderPrepThread m_renderPrepThread;

    glm::ivec3 m_currentRegion;
    glm::ivec3 m_currentChunk;

    bool m_updateSearch;
    float m_viewRadius;
    float m_viewRadiusMax; //distance before renderer is removed

    typedef voxigen::RegionChunkIndex<typename VoxelWorld::RegionType, typename VoxelWorld::ChunkType> RegionChunkIndexType;
    voxigen::ActiveVolume<VoxelWorld, VoxelTerrainChunk, RegionChunkIndexType> m_activeVolume;
    typedef voxigen::RegionIndex<typename VoxelWorld::RegionType> RegionIndexType;
    voxigen::ActiveVolume<VoxelWorld, VoxelTerrainRegion, RegionIndexType> m_activeRegionVolume;

//    //we search for new chunks based on concentric rings around player. This allows us to use occlusion querying before loading
//    std::vector<std::vector<glm::ivec3>> m_chunkSearchRings;

    voxigen::FreeQueue<VoxelTerrainChunk>  m_freeVoxelChunks;
    voxigen::FreeQueue<VoxelTerrainRegion>  m_freeVoxelRegions;

    //Query rings, checks to make sure chunk is in view before loading data
//    int m_currentQueryRing;
//    std::vector<std::vector<VoxelTerrainChunk *>> m_chunkQueryOrder;

    //All renderers current in memory
    RegionRendererMap m_regionRenderers;
    std::vector<UniqueChunkRenderer> m_chunkRenderers; //all allocated  renderers
    std::vector<VoxelTerrainChunk *> m_freeChunkRenderers; //list of un used renderers

    MeshHandler m_meshHandler;
//    cryengine::UpdateDispatcher<VoxelTerrainChunk> m_updateDispatcher;
    bool m_updateThreadRun;
    std::thread m_updateThread;
    std::mutex m_updateMutex;
    std::condition_variable m_updateEvent;
    glm::ivec3 m_cameraRegion;
    glm::ivec3 m_cameraChunk;

    _smart_ptr<IMaterial> m_material;
    _smart_ptr<IMaterial> m_voxelMaterial;

    int m_whiteTextureId;
//    voxigen::TextureAtlas m_textureAtlas;
    voxigen::SharedTextureAtlas m_textureAtlas;
    ITexture *m_terrainTexture;

//    size_t m_heapSize;
//    void *m_heapStorage;
//    IGeneralMemoryHeap *m_heap;
    size_t m_vertexBufferSize;
    AlignedVerticesBuffer<vtx_idx, SVF_P3F_C4B_T2F, TARGET_DEFAULT_ALIGN> *m_vertexBuffer;
//    AlignedVerticesBuffer<vtx_idx, SVF_P3S_N4B_C4B_T2S, TARGET_DEFAULT_ALIGN> *m_vertexBuffer;
};

#endif //_voxelTerrain_voxelTerrain_h_

//    template<>
//    GLM_FUNC_QUALIFIER double next_float(double const& x)
//    {
//    #		if GLM_HAS_CXX11_STL
//        return std::nextafter(x, std::numeric_limits<double>::max());
//    #		elif((GLM_COMPILER & GLM_COMPILER_VC) || ((GLM_COMPILER & GLM_COMPILER_INTEL) && (GLM_PLATFORM & GLM_PLATFORM_WINDOWS)))
//        return detail::nextafter(x, std::numeric_limits<double>::max());
//    #		elif(GLM_PLATFORM & GLM_PLATFORM_ANDROID)
//        return __builtin_nextafter(x, FLT_MAX);
//    #		else
//        return nextafter(x, DBL_MAX);
//    #		endif
//    }
