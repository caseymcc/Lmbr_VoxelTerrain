
#include <VoxelTerrainModule.h>
#include <VoxelTerrainSystemComponent.h>

#include <voxelTerrain.h>

namespace VoxelTerrainGem
{
    VoxelTerrainModule::VoxelTerrainModule()
        : AZ::Module()
    {
        //force linker to include registration of VoxelTerrain
        m_voxelTerrainId=VoxelTerrain::m_terrainTypeId;

        // Push results of [MyComponent]::CreateDescriptor() into m_descriptors here.
        m_descriptors.insert(m_descriptors.end(), {
            VoxelTerrainSystemComponent::CreateDescriptor(),
        });
    }

    AZ::ComponentTypeList VoxelTerrainModule::GetRequiredSystemComponents() const
    {
        return AZ::ComponentTypeList{
            azrtti_typeid<VoxelTerrainSystemComponent>(),
        };
    }
}

#ifndef EDITOR_MODULE
AZ_DECLARE_MODULE_CLASS(VoxelTerrain_00310bf422bd4e0b8d298690636640b2, VoxelTerrainGem::VoxelTerrainModule)
#endif