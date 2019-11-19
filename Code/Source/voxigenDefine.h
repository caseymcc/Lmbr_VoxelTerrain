#ifndef _Cry3DEngine_voxigenDefine_h_
#define _Cry3DEngine_voxigenDefine_h_
#pragma once

//voxigen uses rapidjson, it has been set to use the lumberyard version
#define RAPIDJSON_SKIP_AZCORE_ERROR

#include <voxigen/cell.h>
#include <voxigen/regularGrid.h>

typedef voxigen::RegularGrid<voxigen::Cell, 64, 64, 16> VoxelWorld;

typedef VoxelWorld::SharedRegionHandle VoxelRegionHandle;
typedef VoxelWorld::RegionType VoxelRegion;

typedef VoxelWorld::SharedChunkHandle VoxelChunkHandle;
typedef VoxelWorld::ChunkType VoxelChunk;

#endif // _Cry3DEngine_voxigenDefine_h_