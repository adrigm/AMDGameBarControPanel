#include "pch.h"
#include "SampleComponent.h"
#include "WidgetFT.SampleComponent.g.cpp"

using namespace adlx; // ADLX namespace

class SampleComponent::SettingsChangedCallback : public IADLX3DSettingsChangedListener
{
public:
    SettingsChangedCallback(SampleComponent* owner) : m_owner(owner) {}
    adlx_bool ADLX_STD_CALL On3DSettingsChanged(IADLX3DSettingsChangedEvent* ev) override
    {
        if (m_owner)
            m_owner->On3DSettingsChanged(ev);
        return true;
    }
private:
    SampleComponent* m_owner;
};

namespace winrt::WidgetFT::implementation
{
    SampleComponent::~SampleComponent()
    {
        if (m_listener)
        {
            m_adlxFeatureController.RemoveSettingsChangedListener(m_listener);
            delete m_listener;
            m_listener = nullptr;
        }
    }

    winrt::event_token SampleComponent::SettingsChanged(
        Windows::Foundation::TypedEventHandler<winrt::WidgetFT::SampleComponent, winrt::Windows::Foundation::IInspectable> const& handler)
    {
        return m_settingsChanged.add(handler);
    }

    void SampleComponent::SettingsChanged(winrt::event_token const& token) noexcept
    {
        m_settingsChanged.remove(token);
    }

    bool SampleComponent::Init() {
        OutputDebugStringW(L"-----------------------------------\n");
        ADLX_RESULT r = m_adlxFeatureController.Initialize();
        if (ADLX_SUCCEEDED(r))
        {
            OutputDebugStringW(L"[WidgetFTServer] ADLX Feature Controller initialized\n");
            m_listener = new SettingsChangedCallback(this);
            m_adlxFeatureController.AddSettingsChangedListener(m_listener);
            m_afmfEnabled = m_adlxFeatureController.AFMF_Enabled();
        }
        else
        {
            OutputDebugStringW(L"[WidgetFTServer] ADLX Feature Controller initialization failed\n");
        }

        return ADLX_SUCCEEDED(r);
    }

    void SampleComponent::Refresh()
    {
        m_adlxFeatureController.Refresh();
        m_afmfEnabled = m_adlxFeatureController.AFMF_Enabled();
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

    // ---------------------------------------------------------------------
    //  BOOST (AMD Boost)                                                      
    // ---------------------------------------------------------------------
    bool SampleComponent::Boost_Supported() const                              
    {                                                                          
        return m_adlxFeatureController.Boost_Supported();                      
    }                                                                          

    bool SampleComponent::Boost_Enabled() const                                
    {                                                                          
        return m_adlxFeatureController.Boost_Enabled();                        
    }                                                                          

    bool SampleComponent::Boost_SetEnabled(bool enable)                        
    {                                                                          
        ADLX_RESULT r = m_adlxFeatureController.Boost_SetEnabled(enable);      
        return ADLX_SUCCEEDED(r);                                                         
    }                                                                          

    int SampleComponent::Boost_Resolution() const                              
    {                                                                          
        return static_cast<int>(m_adlxFeatureController.Boost_Resolution());    
    }                                                                          

    int SampleComponent::Boost_ResolutionMin() const                           
    {                                                                          
        return static_cast<int>(m_adlxFeatureController.Boost_ResolutionMin()); 
    }                                                                          

    int SampleComponent::Boost_ResolutionMax() const                           
    {                                                                          
        return static_cast<int>(m_adlxFeatureController.Boost_ResolutionMax()); 
    }                                                                          

    bool SampleComponent::Boost_SetResolution(int value)                       
    {                                                                          
        ADLX_RESULT r = m_adlxFeatureController.Boost_SetResolution(static_cast<adlx_int>(value)); 
        return ADLX_SUCCEEDED(r);                                              
    }                                                                          

    bool SampleComponent::Boost_SetPerfMin()                                   
    {                                                                          
        return Boost_SetResolution(Boost_ResolutionMin());                     
    }                                                                          

    bool SampleComponent::Boost_SetPerfMax()
    {
        return Boost_SetResolution(Boost_ResolutionMax());
    }

    void SampleComponent::On3DSettingsChanged(IADLX3DSettingsChangedEvent* /*ev*/)
    {
        // Simply refresh cached values when the driver notifies a change
        Refresh();
        m_settingsChanged(*this, nullptr);
    }

}
