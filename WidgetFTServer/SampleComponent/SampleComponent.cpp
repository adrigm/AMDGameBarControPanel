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

    void SampleComponent::InitializeADLX()
    {
        if (m_adlxReady)
            return;

        ADLX_RESULT res = m_adlxHelper.Initialize();
        if (ADLX_FAILED(res))
        {
			OutputDebugStringW(L"[WidgetFTServer] ADLX initialization failed with error: ");
            return;
        }

        auto sysServices = m_adlxHelper.GetSystemServices();
        if (!sysServices)
        {
            OutputDebugStringW(L"GetSystemServices returned nullptr");
            return;
        }

        // GPU list
        if (ADLX_FAILED(sysServices->GetGPUs(&m_gpuList)) || !m_gpuList || m_gpuList->Empty())
        {
            OutputDebugStringW(L"No GPUs detected via ADLX");
            return;
        }
        m_gpuList->At(0, &m_gpu);
        if (!m_gpu)
        {
            OutputDebugStringW(L"Could not acquire first GPU");
            return;
        }

        // 3D service layers
        if (ADLX_FAILED(sysServices->Get3DSettingsServices(&m_3DServices)))
        {
            OutputDebugStringW(L"Failed to get 3DSettingsServices");
            return;
        }
        m_3DServices1 = IADLX3DSettingsServices1Ptr(m_3DServices);
        m_3DServices2 = IADLX3DSettingsServices2Ptr(m_3DServices);

        // AFMF
        if (m_3DServices1)
        {
            m_3DServices1->GetAMDFluidMotionFrames(&m_afmf);
            if (m_afmf)
            {
                m_afmf->SetEnabled(true);
                adlx_bool supported = false;
                if (ADLX_SUCCEEDED(m_afmf->IsSupported(&supported)) && !supported)
                    m_afmf.Release();
            }
        }

        // RIS
        if (m_3DServices)
            m_3DServices->GetImageSharpening(m_gpu, &m_imageSharpen);
        if (m_3DServices2)
            m_3DServices2->GetImageSharpenDesktop(m_gpu, &m_imageSharpenDesktop);

        if (m_imageSharpen)
            m_imageSharpen->GetSharpnessRange(&m_sharpRange);

        m_adlxReady = true;
        OutputDebugStringW(L"ADLX initialized OK");
    }

    void SampleComponent::DemoSync()
    {
        OutputDebugStringW(L"[WidgetFTServer] DemoSync\n");
        Sleep(500); // 500ms
        OutputDebugStringW(L"[WidgetFTServer] DemoSync completed\n");

        OutputDebugStringW(L"-----------------------------------\n");
        InitializeADLX(); // Ensure ADLX is initialized
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
    }
    catch (...)
    {
        // A delegate threw. Log it and move on.
    }   
}
