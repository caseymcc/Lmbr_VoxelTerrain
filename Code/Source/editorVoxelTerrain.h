#ifndef _voxelTerrain_editorVoxelTerrain_h_
#define _voxelTerrain_editorVoxelTerrain_h_
#pragma once

//#include <CryCommon/ITerrain.h>
//#include <CryCommon/TerrainFactory.h>
//#include <CryCommon/IRenderer.h>
//#include <CryCommon/I3DEngine.h>

#include <CryCommon/platform.h>
#include <CryCommon/BaseTypes.h>
#include <CryCommon/IRenderer.h>
#include <CryCommon/I3DEngine.h>

#include <QObject>
#include <QPoint>
#include <QRect>
#include <QString>

#include <Editor/IEditor.h>
#include <Editor/GameEngine.h>
#include <Editor/Include/EditorCoreAPI.h>
#include <Editor/Util/EditorUtils.h>
#include <Editor/Util/XmlArchive.h>
#include <Editor/Util/Image.h>

#include <Editor/Terrain/IEditorTerrain.h>
#include <Editor/Terrain/EditorTerrainFactory.h>

class EditorVoxelTerrain:
    public RegisterEditorTerrain<EditorVoxelTerrain, IEditorTerrain>
{
public:
    EditorVoxelTerrain();
    EditorVoxelTerrain(const EditorVoxelTerrain &);
    virtual ~EditorVoxelTerrain();

    static const char *Name() { return m_name; }

    size_t GetType() override { return m_terrainTypeId; }
    const char *GetTypeName() override { return EditorTerrainFactory::getTerrainName(m_terrainTypeId); }
    size_t GetTerrainTypeId() override;
    const char *GetTerrainTypeName() override;
    void GetSectorsInfo(SSectorInfo &si) override;

//IEditorTerrain
    void Init() override;
    void Update() override;

    bool SupportEditing() override { return false; }
    bool SupportLayers() override { return false; }
    bool SupportSerialize() override { return true; }
    bool SupportSerializeTexture() override { return false; }
    bool SupportHeightMap() override { return false; }

    Vec3i GetSectorSizeVector() const override;
    void InitSectorGrid(int numSectors) override;
    int GetNumSectors() const override;
    Vec3 SectorToWorld(const QPoint& sector) const override;
    
//    virtual void GetSectorsInfo(SSectorInfo &si);
    uint64 GetWidth() const override;
    uint64 GetHeight() const override;
    uint64 GetDepth() const override;

    float GetMaxHeight() const override;
    void SetMaxHeight(float maxHeight, bool scale=false) override;

    float GetOceanLevel() const override;
    void SetOceanLevel(float waterLevel) override;

    int GetUnitSize() const override;
    void SetUnitSize(int unitSize) override;

    QPoint FromWorld(const Vec3& wp) const override;
    Vec3 ToWorld(const QPoint& pos) const override;

    QRect WorldBoundsToRect(const AABB& worldBounds) const override;

    void SetSurfaceTextureSize(int width, int height) override;
    void EraseLayerID(uint8 id, uint8 replacementId) override;

    bool IsAllocated() override;
    void GetMemoryUsage(ICrySizer* pSizer) override;

    void Resize(int width, int height, int unitSize, bool cleanOld=true, bool forceKeepVegetation=false) override;
    void Resize(int width, int height, int depth, int unitSize, bool cleanOld=true, bool forceKeepVegetation=false) override;
    void CleanUp() override;

    void Update(bool bOnlyElevation, bool boUpdateReloadSurfacertypes) override {}
    void UpdateSectors() override {}

    void Serialize(CXmlArchive& xmlAr) override;
    void SerializeTerrain(CXmlArchive& xmlAr) override;

    void ClearTerrain() override {}
//    virtual void ExportBlock(const QRect& rect, CXmlArchive& ar, bool bIsExportVegetation=true, std::set<int>* pLayerIds=0, std::set<int>* pSurfaceIds=0)=0;
//    virtual QPoint ImportBlock(CXmlArchive& ar, const QPoint& newPos, bool bUseNewPos=true, float heightOffset=0.0f, bool bOnlyVegetation=false, ImageRotationDegrees rotation=ImageRotationDegrees::Rotate0)=0;

//local
private:
    static const char *m_name;

    size_t m_dimX;
    size_t m_dimY;
    size_t m_dimZ;

    float m_maxHeight;
    int m_unitSize;
    float m_waterLevel;

};

#endif // _voxelTerrain_editorVoxelTerrain_h_
