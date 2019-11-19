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
//#include <Cry3dEngine/StdAfx.h>
//#include <CryCommon/platform_impl.h>
//#include <CryCommon/BaseTypes.h>
//#include <CryCommon/IRenderer.h>
//#include <CryCommon/I3DEngine.h>
//
//#include <QObject>
//#include <QPoint>
//#include <QRect>
//#include <QString>
//
//#include <Editor/IEditor.h>
//#include <Editor/GameEngine.h>
//#include <Editor/Include/EditorCoreAPI.h>
//#include <Editor/Util/EditorUtils.h>
//#include <Editor/Util/XmlArchive.h>
//#include <Editor/Util/Image.h>

#include "editorVoxelTerrain.h"
#include "voxelTerrain.h"

//#include "Editor/GameEngine.h"
//#include "Terrain/TerrainManager.h"

//#include "QtUtil.h"

const char *EditorVoxelTerrain::m_name="VoxelTerrain";

EditorVoxelTerrain::EditorVoxelTerrain():
    m_waterLevel(1024.0),
    m_maxHeight(2048.0)
{

}

EditorVoxelTerrain::EditorVoxelTerrain(const EditorVoxelTerrain &terrain)
{

}

EditorVoxelTerrain::~EditorVoxelTerrain()
{

}

size_t EditorVoxelTerrain::GetTerrainTypeId()
{
    return GetIEditor()->Get3DEngine()->GetTerrainId(m_name);
}

const char *EditorVoxelTerrain::GetTerrainTypeName()
{
    return m_name;
}

void EditorVoxelTerrain::Init()
{
    // construct terrain in 3dengine if was not loaded during SerializeTerrain call
    bool bCreateTerrain=false;
    ITerrain* terrain=gEnv->p3DEngine->GetITerrain();

    if(terrain)
    {
        //check if terran in use is different from current
//        if((gEnv->p3DEngine->GetTerrainSize()!=(si.sectorSize * si.numSectors))||
//            (gEnv->p3DEngine->GetHeightMapUnitSize()!=si.unitSize))
//        {
//            bCreateTerrain=true;
//        }
    }
    else
        bCreateTerrain=true;

    //if terrain needs to be altered, create new one
    if(bCreateTerrain)
    {
        STerrainInfo terrainInfo;

        terrainInfo.type=TerrainFactory::getTerrainId("VoxelTerrain");
        terrainInfo.nTerrainSizeX_InUnits=m_dimX;
        terrainInfo.nTerrainSizeY_InUnits=m_dimY;
        terrainInfo.nTerrainSizeZ_InUnits=m_dimZ;
        terrainInfo.nUnitSize_InMeters=m_unitSize;
        terrainInfo.nSectorSize_InMeters=16;
        terrainInfo.nSectorSizeY_InMeters=16;
        terrainInfo.nSectorSizeZ_InMeters=16;
        terrainInfo.nSectorsTableSize_InSectors=0;
        terrainInfo.fHeightmapZRatio=1.0;
        terrainInfo.fOceanWaterLevel=m_waterLevel;

        terrain=gEnv->p3DEngine->CreateTerrain(terrainInfo);

        //create terrain, load will create the terrain when the directory does not exist
        IEditor *editor=GetIEditor();

        terrain->Load(editor->GetLevelFolder().toStdString().c_str(), &terrainInfo);
    }
}

void EditorVoxelTerrain::Update()
{
}

void EditorVoxelTerrain::GetSectorsInfo(SSectorInfo &si)
{
    ZeroStruct(si);

    si.type=GetType();
    si.unitSize=m_unitSize;
    si.sectorSize=m_dimX;
    si.sectorSizeY=m_dimY;
    si.sectorSizeZ=m_dimZ;
    si.numSectors=1;
    si.sectorTexSize=0;
    si.surfaceTextureSize=0;
}

void EditorVoxelTerrain::InitSectorGrid(int numSectors)
{

}

int EditorVoxelTerrain::GetNumSectors() const
{
    return 0;
}

Vec3 EditorVoxelTerrain::SectorToWorld(const QPoint& sector) const
{
    Vec3 worldPoint(0, 0, 0);

    return worldPoint;
}

uint64 EditorVoxelTerrain::GetWidth() const
{
    return m_dimX;
}

uint64 EditorVoxelTerrain::GetHeight() const
{
    return m_dimY;
}

uint64 EditorVoxelTerrain::GetDepth() const
{
    return m_dimZ;
}

float EditorVoxelTerrain::GetMaxHeight() const
{
    return m_maxHeight;
}

void EditorVoxelTerrain::SetMaxHeight(float maxHeight, bool scale)
{
    m_maxHeight=maxHeight;
}

float EditorVoxelTerrain::GetOceanLevel() const
{
    return m_waterLevel;
}

void EditorVoxelTerrain::SetOceanLevel(float waterLevel)
{
    m_waterLevel=waterLevel;
}

int EditorVoxelTerrain::GetUnitSize() const
{
    return m_unitSize;
}

void EditorVoxelTerrain::SetUnitSize(int unitSize)
{
    m_unitSize=unitSize;
}

Vec3i EditorVoxelTerrain::GetSectorSizeVector() const
{
    glm::ivec3 regionSize=VoxelWorld::regionCellSize();

    return Vec3i(regionSize.x, regionSize.y, regionSize.z);
}

QPoint EditorVoxelTerrain::FromWorld(const Vec3& wp) const
{
    return QPoint(0.0f, 0.0f);
}

Vec3 EditorVoxelTerrain::ToWorld(const QPoint& hpos) const
{
    return Vec3(0.0f, 0.0f, 0.0f);
}

QRect EditorVoxelTerrain::WorldBoundsToRect(const AABB& worldBounds) const
{
    return QRect(0.0f, 0.0f, 0.0f, 0.0f);
}

void EditorVoxelTerrain::SetSurfaceTextureSize(int width, int height)
{
}

void EditorVoxelTerrain::EraseLayerID(uint8 id, uint8 replacementId)
{

}

//bool EditorVoxelTerrain::GetDataEx(t_hmap* pData, UINT iDestWidth, bool bSmooth, bool bNoise) const
//{
//    return false;
//}
//
//bool EditorVoxelTerrain::GetData(const QRect& trgRect, const int resolution, const QPoint& vTexOffset, CFloatImage& hmap, bool bSmooth, bool bNoise)
//{
//    return false;
//}

bool EditorVoxelTerrain::IsAllocated()
{
    return false;
}

void EditorVoxelTerrain::GetMemoryUsage(ICrySizer* pSizer)
{

}

void EditorVoxelTerrain::Resize(int width, int height, int unitSize, bool cleanOld, bool forceKeepVegetation)
{
    assert(false);
}

void EditorVoxelTerrain::Resize(int width, int height, int depth, int unitSize, bool cleanOld, bool forceKeepVegetation)
{
    m_dimX=width;
    m_dimY=height;
    m_dimZ=depth;
    m_unitSize=unitSize;
}

void EditorVoxelTerrain::CleanUp()
{
}

void EditorVoxelTerrain::Serialize(CXmlArchive& xmlAr)
{
    if(xmlAr.bLoading)
    {
        // Loading
        XmlNodeRef root=xmlAr.root;

        root->getAttr("DimX", m_dimX);
        root->getAttr("DimY", m_dimY);
        root->getAttr("DimZ", m_dimZ);
        root->getAttr("WaterLevel", m_waterLevel);
        root->getAttr("UnitSize", m_unitSize);
        root->getAttr("MaxHeight", m_maxHeight);
    }
    else
    {
        // Storing
        XmlNodeRef root=xmlAr.root;

        root->setAttr("DimX", (uint32)m_dimX);
        root->setAttr("DimY", (uint32)m_dimY);
        root->setAttr("DimZ", (uint32)m_dimZ);
        root->setAttr("WaterLevel", m_waterLevel);
        root->setAttr("UnitSize", m_unitSize);
        root->setAttr("MaxHeight", m_maxHeight);
    }
}

void EditorVoxelTerrain::SerializeTerrain(CXmlArchive& xmlAr)
{
    if(xmlAr.bLoading)
    {
        IEditor *editor=GetIEditor();
        I3DEngine *renderEngine=editor->Get3DEngine();
        CGameEngine *gameEngine=editor->GetGameEngine();

        STerrainInfo terrainInfo;

        terrainInfo.type=TerrainFactory::getTerrainId("VoxelTerrain");
        terrainInfo.nTerrainSizeX_InUnits=m_dimX;
        terrainInfo.nTerrainSizeY_InUnits=m_dimY;
        terrainInfo.nTerrainSizeZ_InUnits=m_dimZ;
        terrainInfo.nUnitSize_InMeters=m_unitSize;
        terrainInfo.nSectorSize_InMeters=16;
        terrainInfo.nSectorSizeY_InMeters=16;
        terrainInfo.nSectorSizeZ_InMeters=16;
        terrainInfo.nSectorsTableSize_InSectors=0;
        terrainInfo.fHeightmapZRatio=1.0;
        terrainInfo.fOceanWaterLevel=m_waterLevel;

        //lets tell the VoxelTerrain to load up whatever is avaiable in the level folder
        ITerrain* terrain=renderEngine->GetITerrain();
        QString levelPath=gameEngine->GetLevelPath();

        if((terrain!=nullptr)&&(terrain->GetType()!=TerrainFactory::getTerrainId("VoxelTerrain")))
        {
            renderEngine->DeleteTerrain();
            terrain=nullptr;
        }

        if(terrain == nullptr)
            terrain=renderEngine->CreateTerrain(terrainInfo);
        
        //hackish, if it does not exist it will be created
        terrain->Load(editor->GetLevelFolder().toStdString().c_str(), &terrainInfo);
    }
    else
    {
        IEditor *editor=GetIEditor();
        I3DEngine *renderEngine=editor->Get3DEngine();
        ITerrain* terrain=renderEngine->GetITerrain();
        
        if((terrain==nullptr)||(terrain->GetType()!=TerrainFactory::getTerrainId("VoxelTerrain")))
            return;
        
        VoxelTerrain *voxelTerrain=(VoxelTerrain *)terrain;

        voxelTerrain->Save(editor->GetLevelFolder().toStdString().c_str());

    }
}

//void EditorVoxelTerrain::ExportBlock(const QRect& rect, CXmlArchive& ar, bool bIsExportVegetation, std::set<int>* pLayerIds, std::set<int>* pSurfaceIds) {}
//QPoint EditorVoxelTerrain::ImportBlock(CXmlArchive& ar, const QPoint& newPos, bool bUseNewPos, float heightOffset, bool bOnlyVegetation, ImageRotationDegrees rotation)
//{
//    return QPoint();
//}
