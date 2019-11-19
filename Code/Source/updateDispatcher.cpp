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

#include "StdAfx.h"
#include "updateDispatcher.h"

//DECLARE_JOB("Update_Job", UpdateJob, cryengine::DispatchHandler::update);

namespace cryengine
{

void StartJob(const SRenderingPassInfo &passInfo, IGeneralMemoryHeap *heap, DispatchHandler *handler, Job::State *state)
{
    UpdateJob job(passInfo, heap);

    job.SetClassInstance(handler);
    job.RegisterJobState(state);
    job.Run();
}

}
