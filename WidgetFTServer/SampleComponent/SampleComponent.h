#pragma once
#include "WidgetFT.SampleComponent.g.h"

// ADLX headers
#include "ADLXHelper.h"
#include "I3DSettings.h"
#include "I3DSettings1.h"
#include "I3DSettings2.h"

namespace winrt::WidgetFT::implementation
{
    // wrlrt::module_base is required to ensure the ref count of you COM server works properly
    // xblib::singleton_winrt_base adds singleton support to this class. If you want to do initialization on creation
    // see variant inside singleton_base.h.
    struct SampleComponent : SampleComponentT<SampleComponent, wrlrt::module_base>, xblib::singleton_winrt_base<SampleComponent>
    {
        SampleComponent() = default;

        winrt::Windows::Foundation::IAsyncAction DemoAsync();
        void DemoSync();
        bool DemoBoolProperty();
        void DemoBoolProperty(bool value);
        winrt::event_token DemoBoolPropertyChanged(winrt::WidgetFT::DemoBoolPropertyChangedDelegate const& handler);
        void DemoBoolPropertyChanged(winrt::event_token const& token) noexcept;
        int DemoSyncCounter() const noexcept;

    private:

        winrt::fire_and_forget RaiseDemoBoolPropertyChanged(bool value);
		int m_demoSyncCounter{ 5 };

    private:

        bool m_demoBoolProperty{ false };
        winrt::event<winrt::WidgetFT::DemoBoolPropertyChangedDelegate> m_demoBoolPropertyChangedEvent;

        void InitializeADLX();

        // ADLX
        ADLXHelper m_adlxHelper;
        adlx::IADLXGPUListPtr m_gpuList;
        adlx::IADLXGPUPtr m_gpu;
        adlx::IADLX3DSettingsServicesPtr m_3DServices;
        adlx::IADLX3DSettingsServices1Ptr m_3DServices1;
        adlx::IADLX3DSettingsServices2Ptr m_3DServices2;

        // Feature interfaces
        adlx::IADLX3DAMDFluidMotionFramesPtr m_afmf;
        adlx::IADLX3DImageSharpeningPtr m_imageSharpen;
        adlx::IADLX3DImageSharpenDesktopPtr m_imageSharpenDesktop;
        ADLX_IntRange m_sharpRange{ 0 };

        bool m_adlxReady{ false };
    };
}
