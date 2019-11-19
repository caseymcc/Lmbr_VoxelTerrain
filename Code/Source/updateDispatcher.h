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

#ifndef CRYINCLUDE_CRY3DENGINE_UPDATEDISPATCHER_H
#define CRYINCLUDE_CRY3DENGINE_UPDATEDISPATCHER_H
#pragma once


#include <AzCore/Jobs/Job.h>
#include <AzCore/Jobs/JobManager.h>

#include <memory>

namespace cryengine
{

//polymorphic template voodoo to get a type for the JobManager at compile time that can call a type we don't know yet
struct DispatchHandler
{
    template<typename _Type>
    DispatchHandler(_Type &value):m_self(new DispatchModel<_Type>(value)) {}

    void update(const SRenderingPassInfo &passInfo, IGeneralMemoryHeap *heap) { return m_self->update(passInfo, heap); }

    struct DispatchConcept
    {
        virtual ~DispatchConcept()=default;

        virtual void update(const SRenderingPassInfo &passInfo, IGeneralMemoryHeap *heap)=0;
    };

    template<typename _Type>
    struct DispatchModel:DispatchConcept
    {
        DispatchModel(_Type &value):node(value) {}

        virtual void update(const SRenderingPassInfo &passInfo, IGeneralMemoryHeap *heap){ node->update(passInfo, heap); };

        _Type &node;
    };

    std::shared_ptr<DispatchConcept> m_self;
};

}//namespace cryengine

namespace cryengine
{

template<typename _NodeType>
struct Dispatch
{
    _NodeType *node;
    std::shared_ptr<DispatchHandler> handler;
    AZ::Job::State state;
};
// Container to manager temp memory as well as running update jobs
template<typename _NodeType, size_t _PoolSize=4U<<20 >
class UpdateDispatcher:public Cry3DEngineBase
{
public:
    UpdateDispatcher();
    ~UpdateDispatcher();

    void QueueJob(_NodeType *node, const SRenderingPassInfo &passInfo);
    std::vector<_NodeType *> SyncAllJobs(bool bForceAll, const SRenderingPassInfo &passInfo);
    bool Contains(_NodeType *node)
    {
        auto iter=std::find_if(m_runningJobs.begin(), m_runningJobs.end(), [&](Dispatch<_NodeType> &value){return (value.node==node); });

        return (m_queuedJobs.Find(node)!=-1 || iter!=m_runningJobs.end());
    };

    void GetMemoryUsage(ICrySizer *sizer) const;

    void RemoveJob(_NodeType *node);

private:
    bool AddJob(_NodeType *node, bool executeAsJob, const SRenderingPassInfo &passInfo);

    void *m_heapStorage;
    _smart_ptr<IGeneralMemoryHeap> m_heap;

    PodArray<_NodeType *> m_queuedJobs;
    PodArray<Dispatch<_NodeType>> m_runningJobs;
    std::vector<_NodeType *> m_completedJobs;
#ifdef _DEBUG
    threadID m_threadID;
#endif
};

}//namespace cryengine

//implementation
#include "updateDispatcher.inl"

#endif //CRYINCLUDE_CRY3DENGINE_UPDATEDISPATCHER_H
