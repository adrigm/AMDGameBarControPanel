#pragma once
#include "WidgetFT.SampleComponent.g.h"

// ADLX headers
#include "ADLXHelper.h"
#include "I3DSettings.h"
#include "I3DSettings1.h"
#include "I3DSettings2.h"
#include <AdlxFeatureController.h>

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

        void Init();

        void Refresh();

        // ---------------------------------------------------------------------
        //  Global error state
        // ---------------------------------------------------------------------
        bool HasError() const;
        winrt::hstring Error() const;

        // ---------------------------------------------------------------------
        //  AFMF
        // ---------------------------------------------------------------------
        bool AFMF_Supported() const;
        bool AFMF_Enabled() const;
        bool AFMF_SetEnabled(bool enable);

        // ---------------------------------------------------------------------
        //  RIS (Image Sharpening)
        // ---------------------------------------------------------------------
        bool RIS_Supported()  const;
        bool RIS_Enabled()    const;
        bool RIS_SetEnabled(bool enable);

        // Sharpening Slider (valid only if RIS is enabled)
        int RIS_Sharpness()  const;
        int RIS_SharpnessMin() const;
        int RIS_SharpnessMax() const;
        bool RIS_SetSharpness(int value);

    private:

        winrt::fire_and_forget RaiseDemoBoolPropertyChanged(bool value);
		int m_demoSyncCounter{ 5 };

    private:
		AdlxFeatureController m_adlxFeatureController;

        bool m_demoBoolProperty{ false };
        winrt::event<winrt::WidgetFT::DemoBoolPropertyChangedDelegate> m_demoBoolPropertyChangedEvent;

		// bool for AFMF State
		bool m_afmfEnabled{ false };
		winrt::event<winrt::WidgetFT::DemoBoolPropertyChangedDelegate> m_afmfStateChangedEvent;
    };
}
