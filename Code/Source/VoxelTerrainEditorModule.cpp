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

#include <VoxelTerrainEditorModule.h>

#include <editorVoxelTerrain.h>

namespace VoxelTerrainGem
{
    VoxelTerrainEditorModule::VoxelTerrainEditorModule()
    {
        //force linker to include registration of EditorVoxelTerrain
        m_editorVoxelTerrainId=EditorVoxelTerrain::m_terrainTypeId;
    }

    AZ::ComponentTypeList VoxelTerrainEditorModule::GetRequiredSystemComponents() const
    {
        AZ::ComponentTypeList requiredComponents = VoxelTerrainModule::GetRequiredSystemComponents();

        return requiredComponents;
    }
}

AZ_DECLARE_MODULE_CLASS(VoxelTerrainEditor, VoxelTerrainGem::VoxelTerrainEditorModule)