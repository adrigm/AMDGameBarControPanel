#include "pch.h"
#include "SampleComponent.h"
#include "WidgetFT.SampleComponent.g.cpp"

using namespace adlx; // ADLX namespace

namespace winrt::WidgetFT::implementation
{
    bool SampleComponent::Init() {
        OutputDebugStringW(L"-----------------------------------\n");
        ADLX_RESULT r = m_adlxFeatureController.Initialize();
        if (ADLX_SUCCEEDED(r))
        {
            OutputDebugStringW(L"[WidgetFTServer] ADLX Feature Controller initialized\n");
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
 
}
