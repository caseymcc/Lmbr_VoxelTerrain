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

// Description : Create buffer, copy it into var memory, draw


namespace cryengine
{

template<typename _NodeType, size_t _PoolSize>
UpdateDispatcher<_NodeType, _PoolSize>::UpdateDispatcher()
{
#ifdef _DEBUG
    m_threadID=THREADID_NULL;
#endif
    m_heapStorage=CryMemory::AllocPages(_PoolSize);
    m_heap=CryGetIMemoryManager()->CreateGeneralMemoryHeap(m_heapStorage, _PoolSize, "Terrain temp pool");
}

template<typename _NodeType, size_t _PoolSize>
UpdateDispatcher<_NodeType, _PoolSize>::~UpdateDispatcher()
{
    SyncAllJobs(true, SRenderingPassInfo::CreateGeneralPassRenderingInfo(gEnv->p3DEngine->GetRenderingCamera()));
    
    if(m_queuedJobs.size() || m_runningJobs.size())
    {
        CryFatalError(
            "UpdateDispatcher<_NodeType, _PoolSize>::~UpdateDispatcher(): instance still has jobs "
            "queued while being destructed!\n");
    }

    m_heap=NULL;
    CryMemory::FreePages(m_heapStorage, _PoolSize);
}

template<typename _NodeType, size_t _PoolSize>
void UpdateDispatcher<_NodeType, _PoolSize>::QueueJob(_NodeType* node, const SRenderingPassInfo &passInfo)
{
#ifdef _DEBUG
    if(m_threadID == THREADID_NULL)
        m_threadID=CryGetCurrentThreadId();
#endif

    if(node&&!Contains(node))
    {
        if(!RunJob(node, true, passInfo))
        {
            // if job submission was unsuccessful, queue the terrain job
            m_queuedJobs.Add(node);
        }
    }
}

void StartJob(const SRenderingPassInfo &passInfo, IGeneralMemoryHeap *heap, DispatchHandler *handler, AZ::Job::State *state);

template<typename _NodeType, size_t _PoolSize>
bool UpdateDispatcher<_NodeType, _PoolSize>::AddJob(_NodeType* node, bool executeAsJob, const SRenderingPassInfo &passInfo)
{
    // dont run async in case of editor or if we render into shadowmap
    executeAsJob&=!gEnv->IsEditor();
    executeAsJob&=!passInfo.IsShadowPass();

    if(executeAsJob)
    {
//        ScopedSwitchToGlobalHeap useGlobalHeap;
        Dispatch<_NodeType> &dispatch=m_runningJobs.AddNew();

        dispatch.handler.reset(new DispatchHandler(node));
        dispatch.node=node;
        StartJob(passInfo, m_heap, dispatch.handler.get(), &dispatch.state);
    }
    else
    {
        bool retValue=node->update(passInfo, m_heap);

        if(!retValue)
            return false;

        m_completedJobs.push_back(node);
    }
    return true;
}


template<typename _NodeType, size_t _PoolSize>
std::vector<_NodeType *> UpdateDispatcher<_NodeType, _PoolSize>::SyncAllJobs(bool bForceAll, const SRenderingPassInfo &passInfo)
{
#ifdef _DEBUG
    //check if this is the same thread that Queues Jobs, there is no thread protection otherwise
    if(m_threadID!=THREADID_NULL) //if nothing added yet, ignore check
        assert(CryGetCurrentThreadId()==m_threadID);
#endif//_DEBUG

    FUNCTION_PROFILER_3DENGINE;

//    std::vector<_NodeType *> completed;
    uint32 nNothingQueued=0;

    do
    {
        bool bQueued=m_queuedJobs.size()?false:true;

        size_t i=0, nEnd=m_queuedJobs.size();
        while(i<nEnd)
        {
            _NodeType* node=m_queuedJobs[i];
            if(RunJob(node, false, passInfo))
            {
                bQueued=true;
//                completed.push_back(node);
                m_queuedJobs[i]=m_queuedJobs.Last();
                m_queuedJobs.DeleteLast();
                --nEnd;
                continue;
            }
            ++i;
        }

        i=0;
        nEnd=m_runningJobs.size();
        while(i<nEnd)
        {
            Dispatch<_NodeType> &dispatch=m_runningJobs[i];

//            if(!dispatch.state.IsRunning())
            if(dispatch.state != AZ::Job::State::STATE_PROCESSING)
            {
                m_completedJobs.push_back(dispatch.node);
                m_runningJobs[i]=m_runningJobs.Last();
                m_runningJobs.DeleteLast();
                --nEnd;
                continue;
            }
            ++i;
        }

        if(m_runningJobs.size()==0&&!bQueued)
            ++nNothingQueued;
        if(!bForceAll && nNothingQueued>4)
        {
            CryLogAlways("ERROR: not all terrain sector vertex/index update requests could be scheduled");
            break;
        }
    } while(m_queuedJobs.size()!=0||m_runningJobs.size()!=0);

    std::vector<_NodeType *> completed=std::move(m_completedJobs);

    return completed;
}

template<typename _NodeType, size_t _PoolSize>
void UpdateDispatcher<_NodeType, _PoolSize>::GetMemoryUsage(ICrySizer *sizer) const
{
    sizer->AddObject(m_heapStorage, _PoolSize);
    
    sizer->AddObject(m_queuedJobs);
    sizer->AddObject(m_runningJobs);
    sizer->AddObject(m_completedJobs);
}

template<typename _NodeType, size_t _PoolSize>
void UpdateDispatcher<_NodeType, _PoolSize>::RemoveJob(_NodeType* node)
{
    int index=m_runningJobs.Find_If([&](Dispatch<_NodeType> &value){return (value.node==node);});
    if(index>=0)
    {
        m_runningJobs.Delete(index);
        return;
    }

    index=m_queuedJobs.Find(node);
    if(index>=0)
    {
        m_queuedJobs.Delete(index);
        return;
    }
}

}//namespace cryengine
