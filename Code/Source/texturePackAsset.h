#ifndef _voxelTerrain_texturePackAsset_h_
#define _voxelTerrain_texturePackAsset_h_
#pragma once

#include <AzCore/RTTI/TypeInfo.h>
#include <AzFramework/Asset/SimpleAsset.h>

namespace VoxelTerrainGem
{

class TexturePackAsset
{
public:
    AZ_TYPE_INFO(TexturePackAsset, "{6A1A308A-356C-424B-AFC1-694930F8B45F}")
        static const char* GetFileFilter()
    {
        return "*.json";
    }
};

}//namespace VoxelTerrainGem

namespace AZ
{
AZ_TYPE_INFO_SPECIALIZE(AzFramework::SimpleAssetReference<VoxelTerrainGem::TexturePackAsset>, "{2DD4619E-8E0C-456A-9588-35D53CBF8B85}")
}

#endif//_voxelTerrain_texturePackAsset_h_