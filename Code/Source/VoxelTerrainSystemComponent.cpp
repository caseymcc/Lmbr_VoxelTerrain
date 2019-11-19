
#include <VoxelTerrainSystemComponent.h>

#include <platform_impl.h> // Many CryCommon files require that this is included first.

#include <AzCore/Serialization/SerializeContext.h>
#include <AzCore/Serialization/EditContext.h>
#include <AzCore/Serialization/EditContextConstants.inl>

namespace VoxelTerrainGem
{
    void VoxelTerrainSystemComponent::Reflect(AZ::ReflectContext* context)
    {
        if (AZ::SerializeContext* serialize = azrtti_cast<AZ::SerializeContext*>(context))
        {
            serialize->Class<VoxelTerrainSystemComponent, AZ::Component>()
                ->Version(0);

            if (AZ::EditContext* ec = serialize->GetEditContext())
            {
                ec->Class<VoxelTerrainSystemComponent>("VoxelTerrain", "[Description of functionality provided by this System Component]")
                    ->ClassElement(AZ::Edit::ClassElements::EditorData, "")
                        ->Attribute(AZ::Edit::Attributes::AppearsInAddComponentMenu, AZ_CRC("System"))
                        ->Attribute(AZ::Edit::Attributes::AutoExpand, true)
                    ;
            }
        }
    }

    void VoxelTerrainSystemComponent::GetProvidedServices(AZ::ComponentDescriptor::DependencyArrayType& provided)
    {
        provided.push_back(AZ_CRC("VoxelTerrainService"));
    }

    void VoxelTerrainSystemComponent::GetIncompatibleServices(AZ::ComponentDescriptor::DependencyArrayType& incompatible)
    {
        incompatible.push_back(AZ_CRC("VoxelTerrainService"));
    }

    void VoxelTerrainSystemComponent::GetRequiredServices(AZ::ComponentDescriptor::DependencyArrayType& required)
    {
        AZ_UNUSED(required);
    }

    void VoxelTerrainSystemComponent::GetDependentServices(AZ::ComponentDescriptor::DependencyArrayType& dependent)
    {
        AZ_UNUSED(dependent);
    }

    void VoxelTerrainSystemComponent::Init()
    {
    }

    void VoxelTerrainSystemComponent::Activate()
    {
        CrySystemEventBus::Handler::BusConnect();
        VoxelTerrainRequestBus::Handler::BusConnect();
    }

    void VoxelTerrainSystemComponent::Deactivate()
    {
        VoxelTerrainRequestBus::Handler::BusDisconnect();
        CrySystemEventBus::Handler::BusDisconnect();
    }

    void VoxelTerrainSystemComponent::OnCrySystemInitialized(ISystem& system, const SSystemInitParams&)
    {
#if !defined(AZ_MONOLITHIC_BUILD)
        // When module is linked dynamically, we must set our gEnv pointer.
        // When module is linked statically, we'll share the application's gEnv pointer.
        gEnv=system.GetGlobalEnvironment();
#endif
    }

    void VoxelTerrainSystemComponent::OnCrySystemShutdown(ISystem&)
    {

    }

    void VoxelTerrainSystemComponent::OnCryEditorInitialized()
    {

    }
}
