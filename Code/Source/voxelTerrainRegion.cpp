/*
* All or portions of this file Copyright (c) Amazon.com, Inc. or its affiliates or
* its licensors.
*
* For complete copyright and license terms please see the LICENSE at the root of this
* distribution (the "License"). All use of this software is governed by the License,
* or, if provided, by the license below or the license accompanying this file. Do not
* remove or modify any license notices. This file is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*
*/
// Original file Copyright Crytek GMBH or its affiliates, used under license.

// Description : terrain node


#include "StdAfx.h"
#include "voxelTerrainRegion.h"

VoxelTerrainRegion::VoxelTerrainRegion()//VoxelTerrain *parent, voxigen::RegionHash hash, const Vec3i &index, const Vec3 &offset):
//m_parent(parent),
//m_hash(hash),
////m_regionHandle(regionHandle),
//m_index(index),
//m_offset(offset)
{

}

VoxelTerrainRegion::~VoxelTerrainRegion()
{

}

void VoxelTerrainRegion::Render(const SRendParams& RendParams, const SRenderingPassInfo& passInfo)
{

}

const AABB VoxelTerrainRegion::GetBBoxVirtual()
{ 
    return m_boundingBox;
}

void VoxelTerrainRegion::FillBBox(AABB& aabb)
{

}

struct ICharacterInstance *VoxelTerrainRegion::GetEntityCharacter(unsigned int nSlot, Matrix34A* pMatrix, bool bReturnOnlyVisible)
{ 
    return NULL;
}

bool VoxelTerrainRegion::IsRenderNode()
{ 
    return false;
}

EERType VoxelTerrainRegion::GetRenderNodeType()
{
    return eERType_NotRenderNode;
}

//bool VoxelTerrainRegion::CheckVis(const SRenderingPassInfo& passInfo)
//{
//    FUNCTION_PROFILER_3DENGINE;
//
//    const CCamera& camera=passInfo.GetCamera();
//
//    if(!camera.IsAABBVisible_EM(m_boundingBox))
//        return false;
//
//    const float distanceToCamera=GetPointToBoxDistance(camera.GetPosition(), m_boundingBox);
//    m_distanceToCamera[passInfo.GetRecursiveLevel()]=distanceToCamera;
//
//    if(distanceToCamera > camera.GetFarPlane())
//        return false; // too far
//
////    if(m_bHasHoles==2)
////    {
////        return false; // has no visible mesh
////    }
//    Get3DEngine()->CheckCreateRNTmpData(&m_pRNTmpData, NULL, passInfo);
//
//    // occlusion test (affects only static objects)
//    if(m_Parent && GetObjManager()->IsBoxOccluded(worldBox, distanceToCamera, &m_pRNTmpData->userData.m_OcclState, false, eoot_TERRAIN_NODE, passInfo))
//    {
//        return false;
//    }
//
//    // find LOD of this sector
//    SetLOD(passInfo);
//
//    m_nSetLodFrameId=passInfo.GetMainFrameID();
//
//    int nSectorSize=CTerrain::GetSectorSize()<<m_nTreeLevel;
//    bool bCameraInsideSector=distanceToCamera < nSectorSize;
//
//    // The Geometry / Texture LODs have nothing to do with the tree level. But this is roughly saying that
//    // we should traverse deeper into the tree if we have a certain threshold of geometry or texture detail,
//    // these exact heuristics are just empirical.
//    //
//    // There are two things to consider when mucking with these heuristics:
//    //   1) This doesn't actually control the geometry LOD, it only controls the render mesh draw call granularity.
//    //      This means the farther up the tree you stop, the bigger the mesh you're going to build, which is
//    //      a tradeoff between draw call count and dynamic mesh size. As an example, if we are at LOD 2, that equates to
//    //      a step size of 4 units. No matter where we stop the recursion, the geometry is still built at 1 vertex per 4 units.
//    //      This heuristic controls how big the sector is, so if, theoretically, we stopped at the root node, we would build single giant
//    //      render mesh at 1 vertex per 4 units.
//    //
//    //   2) The texture tiles are setup such that the maximum resolution at a node is 256x256 pixels. This means if you don't refine the
//    //      quadtree enough will a super high-res texture, you can artificially limit the texture LOD.
//    //
//    bool bHasMoreGeometryDetail=(m_QueuedLOD+GetTerrain()->m_MeterToUnitBitShift)<=m_nTreeLevel;
//
//    // (TODO bethelz): I will probably remove this in the next pass. It places a strange dependency on the texture
//    // LOD but doesn't use any metric to judge how much detail we're preserving. It mostly just forces render mesh updates for no
//    // good reason.
//    // bool bHasMoreTextureDetail = (m_TextureLOD + GetTerrain()->m_nBitShift) < m_nTreeLevel;
//
//    bool bNeedsMoreDetail=(bCameraInsideSector||bHasMoreGeometryDetail|| /*bHasMoreTextureDetail ||*/ m_bForceHighDetail);
//    bool bContinueRecursion=m_Children && bNeedsMoreDetail;
//
//    if(bContinueRecursion)
//    {
//        Vec3 boxCenter=worldBox.GetCenter();
//        Vec3 cameraPos=camera.GetPosition();
//
//        bool bGreaterThanCenterX=(cameraPos.x > boxCenter.x);
//        bool bGreaterThanCenterY=(cameraPos.y > boxCenter.y);
//
//        uint32 firstIndex=(bGreaterThanCenterX ? 2 : 0)|(bGreaterThanCenterY ? 1 : 0);
//        m_Children[firstIndex].CheckVis(bAllInside, bAllowRenderIntoCBuffer, passInfo);
//        m_Children[firstIndex^1].CheckVis(bAllInside, bAllowRenderIntoCBuffer, passInfo);
//        m_Children[firstIndex^2].CheckVis(bAllInside, bAllowRenderIntoCBuffer, passInfo);
//        m_Children[firstIndex^3].CheckVis(bAllInside, bAllowRenderIntoCBuffer, passInfo);
//    }
//    else
//    {
//        if(GetCVars()->e_StatObjBufferRenderTasks==1&&passInfo.IsGeneralPass()&&JobManager::InvokeAsJob("CheckOcclusion"))
//        {
//            GetObjManager()->PushIntoCullQueue(SCheckOcclusionJobData::CreateTerrainJobData(this, GetBBox(), distanceToCamera));
//        }
//        else
//        {
//            GetTerrain()->AddVisSector(this);
//        }
//
//        if(passInfo.IsGeneralPass())
//        {
//            if(distanceToCamera < GetTerrain()->m_fDistanceToSectorWithWater)
//            {
//                GetTerrain()->m_fDistanceToSectorWithWater=distanceToCamera;
//            }
//        }
//
//        if(m_Children)
//        {
//            for(int i=0; i < 4; i++)
//            {
//                m_Children[i].SetChildsLod(m_QueuedLOD, passInfo);
//            }
//        }
//
//        RequestTextures(passInfo);
//    }
//
//    // update procedural vegetation
//    if(GetCVars()->e_ProcVegetation)
//    {
//        if(passInfo.IsGeneralPass())
//        {
//            if(distanceToCamera < GetCVars()->e_ProcVegetationMaxViewDistance)
//            {
//                GetTerrain()->ActivateNodeProcObj(this);
//            }
//        }
//    }
//
//    return true;
//}