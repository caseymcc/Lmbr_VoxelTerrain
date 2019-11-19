
#include <AzCore/Memory/SystemAllocator.h>
#include <AzCore/Module/Module.h>

namespace VoxelTerrainGem
{
    class VoxelTerrainModule
        : public AZ::Module
    {
    public:
        AZ_RTTI(VoxelTerrainModule, "{42E31C31-7AE5-44D5-8832-4F17263CC6B1}", AZ::Module);
        AZ_CLASS_ALLOCATOR(VoxelTerrainModule, AZ::SystemAllocator, 0);

        VoxelTerrainModule();

        AZ::ComponentTypeList GetRequiredSystemComponents() const override;

    private:
        size_t m_voxelTerrainId;
    };
}

