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

#pragma once

#include <VoxelTerrainModule.h>

namespace VoxelTerrainGem
{
    class VoxelTerrainEditorModule
        : public VoxelTerrainModule
    {
    public:
        AZ_RTTI(VoxelTerrainEditorModule, "{461724D7-FA90-4763-B443-61F63A1BEE99}", VoxelTerrainModule);
        AZ_CLASS_ALLOCATOR(VoxelTerrainEditorModule, AZ::SystemAllocator, 0);

        VoxelTerrainEditorModule();

        AZ::ComponentTypeList GetRequiredSystemComponents() const override;

    private:
        size_t m_editorVoxelTerrainId;
    };
}