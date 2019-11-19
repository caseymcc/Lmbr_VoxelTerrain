#ifndef _Cry3DEngine_VoxelTerrainRegion_h_
#define _Cry3DEngine_VoxelTerrainRegion_h_
#pragma once

#include "VoxelTerrainChunk.h"

#include <unordered_map>

class VoxelTerrain;
class VoxelTerrainRegion:
//public Cry3DEngineBase,
public IShadowCaster
{
public:
    typedef std::unordered_map<voxigen::ChunkHash, VoxelTerrainChunk *> ChunkRendererMap;
    friend class VoxelTerrain;

    VoxelTerrainRegion();// VoxelTerrain *parent, voxigen::RegionHash hash, const Vec3i &index, const Vec3 &offset);
    ~VoxelTerrainRegion();

    virtual void Render(const SRendParams& RendParams, const SRenderingPassInfo& passInfo);
    virtual const AABB GetBBoxVirtual();
    virtual void FillBBox(AABB& aabb);
    virtual struct ICharacterInstance* GetEntityCharacter(unsigned int nSlot, Matrix34A* pMatrix=NULL, bool bReturnOnlyVisible=false);
    virtual bool IsRenderNode();
    virtual EERType GetRenderNodeType();

    VoxelTerrain *GetTerrain() { return m_parent; }

    ChunkRendererMap &ChunkRenderers() { return m_chunkRenderers; }

    Vec3i Index() const{ return m_index; }

    Vec3 GetOffset() const { return m_offset; }
    void SetOffset(const Vec3 &value) { m_offset=value; }

//active volume support
    voxigen::RenderAction getAction() { return m_action; }
    void setAction(voxigen::RenderAction action) { m_action=action; }
    static glm::ivec3 getSize() { return glm::ivec3(VoxelRegion::sizeX::value, VoxelRegion::sizeY::value, VoxelRegion::sizeZ::value); }

    void setHandle(VoxelRegionHandle handle) { m_regionHandle=handle; }
    VoxelRegionHandle getHandle() { return m_regionHandle; }

    void setTextureAtlas(voxigen::SharedTextureAtlas textureAtlas) { m_textureAtlas=textureAtlas; }
    voxigen::SharedTextureAtlas getTextureAtlas() { return m_textureAtlas; }

    void build() {}
    void clear() {}

private:
    VoxelTerrain *m_parent;
//    VoxelRegionHandle m_regionHandle;

    ChunkRendererMap m_chunkRenderers;

    AABB m_boundingBox;

    voxigen::RegionHash m_hash;
    Vec3i m_index;
    Vec3 m_offset;

    float m_distanceToCamera;

    VoxelRegionHandle m_regionHandle;

    voxigen::RenderAction m_action;
    voxigen::SharedTextureAtlas m_textureAtlas;
};

#endif // _Cry3DEngine_VoxelTerrainRegion_h_
