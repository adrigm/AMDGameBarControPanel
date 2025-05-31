#include "pch.h"
#include "SampleComponent.h"
#include "WidgetFT.SampleComponent.g.cpp"

using namespace adlx; // ADLX namespace

namespace winrt::WidgetFT::implementation
{
    winrt::Windows::Foundation::IAsyncAction SampleComponent::DemoAsync()
    {
        // Best practice to grab strong ref to self before you co_await
        auto strongThis{ get_strong() };

        OutputDebugStringW(L"[WidgetFTServer] DemoAsync\n");

        // Get off callers thread (COM Neutral Apartment thread in the case of cross-proc)
        co_await winrt::resume_background();

        // Do some async work
        //co_await winrt::resume_after(std::chrono::milliseconds(500));

        OutputDebugStringW(L"[WidgetFTServer] DemoAsync completed\n");
    }

    void SampleComponent::Init() {
        OutputDebugStringW(L"-----------------------------------\n");
        ADLX_RESULT r = m_adlxFeatureController.Initialize();
        if (ADLX_SUCCEEDED(r))
        {
            OutputDebugStringW(L"[WidgetFTServer] ADLX Feature Controller initialized\n");
            /*m_adlxFeatureController.AFMF_SetEnabled(true);
			m_adlxFeatureController.RIS_SetEnabled(true);
            m_adlxFeatureController.RIS_SetSharpness(20);*/

        }
        else
        {
            OutputDebugStringW(L"[WidgetFTServer] ADLX Feature Controller initialization failed\n");
        }
    }

    void SampleComponent::Refresh()
    {
		m_adlxFeatureController.Refresh();
    }

    // ---------------------------------------------------------------------
//  Estado de error global
// ---------------------------------------------------------------------
    bool SampleComponent::HasError() const
    {
        return m_adlxFeatureController.HasError();
    }

    winrt::hstring SampleComponent::Error() const
    {
        // Convertimos std::wstring → hstring
        return winrt::hstring{ m_adlxFeatureController.Error() };
    }

    // ---------------------------------------------------------------------
    //  AFMF
    // ---------------------------------------------------------------------
    bool SampleComponent::AFMF_Supported() const
    {
        return m_adlxFeatureController.AFMF_Supported();
    }

    bool SampleComponent::AFMF_Enabled() const
    {
        return m_adlxFeatureController.AFMF_Enabled();
    }

    bool SampleComponent::AFMF_SetEnabled(bool enable)
    {
        ADLX_RESULT r = m_adlxFeatureController.AFMF_SetEnabled(enable);
        if (ADLX_SUCCEEDED(r))
        {
            m_afmfEnabled = enable;   // Reflejamos el nuevo estado interno
            return true;
        }
        return false;
    }

    // ---------------------------------------------------------------------
    //  RIS (Image Sharpening)
    // ---------------------------------------------------------------------
    bool SampleComponent::RIS_Supported() const
    {
        return m_adlxFeatureController.RIS_Supported();
    }

    bool SampleComponent::RIS_Enabled() const
    {
        return m_adlxFeatureController.RIS_Enabled();
    }

    bool SampleComponent::RIS_SetEnabled(bool enable)
    {
        ADLX_RESULT r = m_adlxFeatureController.RIS_SetEnabled(enable);
        return ADLX_SUCCEEDED(r);
    }

    //  Slider de nitidez (solo válido cuando RIS está activo)
    int SampleComponent::RIS_Sharpness() const
    {
        return static_cast<int>(m_adlxFeatureController.RIS_Sharpness());
    }

    int SampleComponent::RIS_SharpnessMin() const
    {
        return static_cast<int>(m_adlxFeatureController.RIS_SharpnessMin());
    }

    int SampleComponent::RIS_SharpnessMax() const
    {
        return static_cast<int>(m_adlxFeatureController.RIS_SharpnessMax());
    }

    bool SampleComponent::RIS_SetSharpness(int value)
    {
        ADLX_RESULT r = m_adlxFeatureController.RIS_SetSharpness(static_cast<adlx_int>(value));
        return ADLX_SUCCEEDED(r);
    }

    // ---------------------------------------------------------------------
    //  RSR (Radeon Super Resolution)
    // ---------------------------------------------------------------------
    bool SampleComponent::RSR_Supported() const
    {
        return m_adlxFeatureController.RSR_Supported();
    }

    bool SampleComponent::RSR_Enabled() const
    {
        return m_adlxFeatureController.RSR_Enabled();
    }

    bool SampleComponent::RSR_SetEnabled(bool enable)
    {
        ADLX_RESULT r = m_adlxFeatureController.RSR_SetEnabled(enable);
        return ADLX_SUCCEEDED(r);
    }



    void SampleComponent::DemoSync()
    {
        OutputDebugStringW(L"[WidgetFTServer] DemoSync\n");
        Sleep(500); // 500ms
        OutputDebugStringW(L"[WidgetFTServer] DemoSync completed\n");
    }

    bool SampleComponent::DemoBoolProperty()
    {
        return m_demoBoolProperty;
    }

    void SampleComponent::DemoBoolProperty(bool value)
    {
        m_demoBoolProperty = value;
        RaiseDemoBoolPropertyChanged(value); // fire and forget
    }

    winrt::event_token SampleComponent::DemoBoolPropertyChanged(winrt::WidgetFT::DemoBoolPropertyChangedDelegate const& handler)
    {
        // Register the handler the client passed in
        return m_demoBoolPropertyChangedEvent.add(handler);
    }

    void SampleComponent::DemoBoolPropertyChanged(winrt::event_token const& token) noexcept
    {
        // Unregister the handler for the token the client passed in
        m_demoBoolPropertyChangedEvent.remove(token);
    }

    int SampleComponent::DemoSyncCounter() const noexcept
    {
		return m_demoSyncCounter;
    }

    winrt::fire_and_forget SampleComponent::RaiseDemoBoolPropertyChanged(bool value) try
    {
        // Best practice to grab strong ref to self before you co_await
        auto strongThis{ get_strong() };

        // Raise event on a background thread
        co_await winrt::resume_background();

        // Raise the event
        m_demoBoolPropertyChangedEvent(value);
		m_adlxFeatureController.AFMF_SetEnabled(value);
    }
    catch (...)
    {
        // A delegate threw. Log it and move on.
    }   
}
