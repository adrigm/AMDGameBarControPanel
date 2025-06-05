// AdlxFeatureController.cpp
// -----------------------------------------------------------------------------
//  AMD Game Bar Control Panel – ADLX backend controller
//
//  Provides a single C++ class that owns the ADLX runtime, queries feature
//  interfaces and exposes simple getter / setter methods for:
//      • AMD Fluid Motion Frames (AFMF)
//      • Radeon Image Sharpening (RIS) + sharpness slider
//      • Radeon Super Resolution (RSR)
//      • Custom Color (Brightness / Contrast / Saturation) toggle + sliders
//
//  All ADLX calls are isolated here so that the UWP front‑end never touches the
//  driver directly.  The class is thread‑safe and fault‑tolerant – any failure
//  is signalled via return codes and a textual error message that can be
//  surfaced to the UI.
// -----------------------------------------------------------------------------

#include <mutex>
#include <string>
#include "ADLXHelper.h"
#include "I3DSettings.h"
#include "I3DSettings1.h"
#include "I3DSettings2.h"
#include "IDisplays.h"
#include "IDisplaySettings.h"

using namespace adlx;

class AdlxFeatureController
{
public:
    AdlxFeatureController();
    ~AdlxFeatureController();

    // ---------------------------------------------------------------------
    //  Initialisation / tear‑down
    // ---------------------------------------------------------------------
    ADLX_RESULT  Initialize();          // Call once on app start
    void         Terminate();           // Call on app shutdown

    // Refresh current state from the driver (useful when other tools modify it)
    ADLX_RESULT  Refresh();

    // Register additional callbacks for 3D settings changes
    ADLX_RESULT  AddSettingsChangedListener(IADLX3DSettingsChangedListener* listener);
    ADLX_RESULT  RemoveSettingsChangedListener(IADLX3DSettingsChangedListener* listener);

    // ---------------------------------------------------------------------
    //  Global error state
    // ---------------------------------------------------------------------
    bool         HasError()  const { return !m_errorMessage.empty(); }
    std::wstring Error()     const { return m_errorMessage; }

    // ---------------------------------------------------------------------
    //  AFMF
    // ---------------------------------------------------------------------
    bool         AFMF_Supported() const { return m_afmf != nullptr; }
    bool         AFMF_Enabled()   const { return m_afmfEnabled; }
    ADLX_RESULT  AFMF_SetEnabled(bool enable);

    // ---------------------------------------------------------------------
    //  RIS (Image Sharpening)
    // ---------------------------------------------------------------------
    bool         RIS_Supported()  const { return m_ris != nullptr; }
    bool         RIS_Enabled()    const { return m_risEnabled; }
    ADLX_RESULT  RIS_SetEnabled(bool enable);

    // Sharpening Slider (valid only if RIS is enabled)
    adlx_int     RIS_Sharpness()  const { return m_risSharpness; }
    adlx_int     RIS_SharpnessMin() const { return m_risRange.minValue; }
    adlx_int     RIS_SharpnessMax() const { return m_risRange.maxValue; }
    ADLX_RESULT  RIS_SetSharpness(adlx_int value);

    // ---------------------------------------------------------------------
    //  RSR
    // ---------------------------------------------------------------------
    bool         RSR_Supported()  const { return m_rsr != nullptr; }
    bool         RSR_Enabled()    const { return m_rsrEnabled; }
    ADLX_RESULT  RSR_SetEnabled(bool enable);

    // ---------------------------------------------------------------------
    //  BOOST  (Resolution‑adaptive Frequency Boost)
    // ---------------------------------------------------------------------
    bool         Boost_Supported() const { return m_boost != nullptr; }             
    bool         Boost_Enabled()   const { return m_boostEnabled; }                 
    ADLX_RESULT  Boost_SetEnabled(bool enable);                                     

    // Resolution limit controls (valid only if Boost is enabled)                 
    adlx_int     Boost_Resolution() const { return m_boostResolution; }             
    adlx_int     Boost_ResolutionMin() const { return m_boostResRange.minValue; }   
    adlx_int     Boost_ResolutionMax() const { return m_boostResRange.maxValue; }   
    ADLX_RESULT  Boost_SetResolution(adlx_int value);                               

    // ---------------------------------------------------------------------
    //  Custom Color – master toggle + three sliders
    // ---------------------------------------------------------------------
    bool         CustomColor_Supported() const { return m_customColor != nullptr; }
    bool         CustomColor_Enabled()   const { return m_customColorEnabled; }
    ADLX_RESULT  CustomColor_SetEnabled(bool enable);

    // Individual sliders – range helpers
    adlx_int     Brightness()  const { return m_brightness; }
    adlx_int     BrightnessMin() const { return m_brightnessRange.minValue; }
    adlx_int     BrightnessMax() const { return m_brightnessRange.maxValue; }
    ADLX_RESULT  SetBrightness(adlx_int val);

    adlx_int     Contrast()    const { return m_contrast; }
    adlx_int     ContrastMin() const { return m_contrastRange.minValue; }
    adlx_int     ContrastMax() const { return m_contrastRange.maxValue; }
    ADLX_RESULT  SetContrast(adlx_int val);

    adlx_int     Saturation()  const { return m_saturation; }
    adlx_int     SaturationMin() const { return m_saturationRange.minValue; }
    adlx_int     SaturationMax() const { return m_saturationRange.maxValue; }
    ADLX_RESULT  SetSaturation(adlx_int val);

private:
    // Helper utilities
    void  CacheInitialValues();
    void  ClearFeaturePointers();

    // Ranges might be {0, 100, 1} or similar – depends on driver / GPU
    template<typename Fn>
    ADLX_RESULT  CallWithLock(Fn&& fn);

    // Members ----------------------------------------------------------------
    mutable std::mutex m_lock;

    ADLXHelper                           m_adlx;         // RAII helper

    // Feature interfaces ------------------------------------------------------
    IADLX3DAMDFluidMotionFramesPtr       m_afmf = nullptr;
    IADLX3DImageSharpeningPtr            m_ris = nullptr;
    IADLX3DRadeonSuperResolutionPtr      m_rsr = nullptr;
    IADLX3DBoostPtr                      m_boost = nullptr;  

    IADLXDisplayServicesPtr              m_displaySrv = nullptr;
    IADLXDisplayCustomColorPtr           m_customColor = nullptr;

    // Cached state ------------------------------------------------------------
    bool         m_initialized = false;
    std::wstring m_errorMessage;

    // AFMF
    bool         m_afmfEnabled = false;

    // RIS
    bool         m_risEnabled = false;
    adlx_int     m_risSharpness = 0;
    ADLX_IntRange  m_risRange{ 0 };

    // RSR
    bool         m_rsrEnabled = false;

    // Boost                                                                 
    bool         m_boostEnabled = false;                                     
    adlx_int     m_boostResolution = 0;                                      
    ADLX_IntRange  m_boostResRange{ 0 };                                     

    // Custom Color
    bool         m_customColorEnabled = false;
    adlx_int     m_brightness = 0;
    adlx_int     m_contrast = 0;
    adlx_int     m_saturation = 0;
    ADLX_IntRange  m_brightnessRange{ 0 };
    ADLX_IntRange  m_contrastRange{ 0 };
    ADLX_IntRange  m_saturationRange{ 0 };

    // Defaults so we can restore when CustomColor off
    const adlx_int kDefaultBrightness = 0;
    const adlx_int kDefaultContrast = 0;
    const adlx_int kDefaultSaturation = 0;

    // Event handling for 3D settings
    IADLX3DSettingsChangedHandlingPtr    m_changedHandle = nullptr;
    class SettingsChangedCallback;
    SettingsChangedCallback*             m_settingsListener = nullptr;
    bool                                 m_refreshPending = false;
};