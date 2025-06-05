#include "pch.h"
#include "AdlxFeatureController.h"
#include <iostream>

// ============================================================================
//  Implementation
// ============================================================================

// Forward declaration
class AdlxFeatureController::SettingsChangedCallback : public IADLX3DSettingsChangedListener
{
public:
    SettingsChangedCallback(AdlxFeatureController* owner) : m_owner(owner) {}
    adlx_bool ADLX_STD_CALL On3DSettingsChanged(IADLX3DSettingsChangedEvent* event) override;
private:
    AdlxFeatureController* m_owner;
};

AdlxFeatureController::AdlxFeatureController() {}

AdlxFeatureController::~AdlxFeatureController()
{
    Terminate();
}

// Generic wrapper that locks the public mutex and executes the callable
template<typename Fn>
ADLX_RESULT AdlxFeatureController::CallWithLock(Fn&& fn)
{
    std::lock_guard<std::mutex> guard(m_lock);
    try
    {
        if constexpr (std::is_same_v<std::invoke_result_t<Fn>, void>)
        {
            std::forward<Fn>(fn)();   // callable devuelve void
            return ADLX_OK;           // tratamos como éxito
        }
        else
        {
            return std::forward<Fn>(fn)(); // devuelve ADLX_RESULT
        }
    }
    catch (...)
    {
        return ADLX_FAIL;
    }
}

// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::Initialize()
{
    return CallWithLock([this]()->ADLX_RESULT
        {
            ADLX_RESULT res;
            if (!m_initialized)
            {
                res = m_adlx.Initialize();
                if (!ADLX_SUCCEEDED(res))
                {
                    m_errorMessage = L"ADLX initialization failed";
                    return res;
                }

            }
            

            // -------------------------------------------------------------
            // 3D Settings interfaces
            // -------------------------------------------------------------
            IADLX3DSettingsServicesPtr d3dSrv;
            res = m_adlx.GetSystemServices()->Get3DSettingsServices(&d3dSrv);
            if (!ADLX_SUCCEEDED(res))
            {
                m_errorMessage = L"Failed to obtain 3D settings services";
                return res;
            }

            // AFMF
            {
                IADLX3DSettingsServices1Ptr d3dSrv1(d3dSrv);
                if (d3dSrv1)
                {
                    d3dSrv1->GetAMDFluidMotionFrames(&m_afmf);
                    if (m_afmf)
                        m_afmf->IsEnabled(&m_afmfEnabled);
                }
            }

            // RIS + BOOST (require GPU handle)
            {
                IADLXGPUListPtr gpuList;
                res = m_adlx.GetSystemServices()->GetGPUs(&gpuList);
                if (ADLX_SUCCEEDED(res) && gpuList && !gpuList->Empty())
                {
                    IADLXGPUPtr gpu;
                    // Try to locate the first discrete GPU
                    for (adlx_uint i = 0; i < gpuList->Size(); ++i)
                    {
                        IADLXGPUPtr tmpGpu;
                        if (ADLX_SUCCEEDED(gpuList->At(i, &tmpGpu)) && tmpGpu)
                        {
                            ADLX_GPU_TYPE type = GPUTYPE_UNDEFINED;
                            if (ADLX_SUCCEEDED(tmpGpu->Type(&type)) && type == GPUTYPE_DISCRETE)
                            {
                                gpu = tmpGpu;
                                break;
                            }
                        }
                    }

                    // Fallback to the first GPU if no discrete GPU was found
                    if (!gpu)
                        gpuList->At(0, &gpu);

                    if (gpu)
                    {
                        // RIS ------------------------------------------------
                        d3dSrv->GetImageSharpening(gpu, &m_ris);
                        if (m_ris)
                        {
                            m_ris->IsEnabled(&m_risEnabled);
                            m_ris->GetSharpness(&m_risSharpness);
                            m_ris->GetSharpnessRange(&m_risRange);

                            // output in console status
                            OutputDebugStringW(L"RIS is ");
                            OutputDebugStringW(m_risEnabled ? L"enabled\n" : L"disabled\n");
                        }

                        // BOOST --------------------------------------------- 
                        d3dSrv->GetBoost(gpu, &m_boost);
                        if (m_boost)
                        {
                            m_boost->IsEnabled(&m_boostEnabled);
                            m_boost->GetResolution(&m_boostResolution);
                            m_boost->GetResolutionRange(&m_boostResRange);
                        }
                    }
                }
            }

            // RSR (does not require GPU handle)
            d3dSrv->GetRadeonSuperResolution(&m_rsr);
            if (m_rsr)
                m_rsr->IsEnabled(&m_rsrEnabled);


            // Register for 3D settings change events
            IADLX3DSettingsChangedHandlingPtr changeHdl;
            if (ADLX_SUCCEEDED(d3dSrv->Get3DSettingsChangedHandling(&changeHdl)))
            {
                m_changedHandle = changeHdl;
                m_settingsListener = new SettingsChangedCallback(this);
                m_changedHandle->Add3DSettingsEventListener(m_settingsListener);
            }

            // Success path
            m_refreshPending = false;
            m_initialized = true;
            return ADLX_OK;
        });
}

void AdlxFeatureController::Terminate()
{
    CallWithLock([this]()
        {
            if (!m_initialized)
                return;

            ClearFeaturePointers();
            m_adlx.Terminate();
            m_initialized = false;
            m_refreshPending = false;
        });
}

void AdlxFeatureController::ClearFeaturePointers()
{
    m_afmf = nullptr;
    m_ris = nullptr;
    m_rsr = nullptr;
    m_customColor = nullptr;
    m_displaySrv = nullptr;
    if (m_changedHandle && m_settingsListener)
    {
        m_changedHandle->Remove3DSettingsEventListener(m_settingsListener);
    }
    m_changedHandle = nullptr;
    delete m_settingsListener;
    m_settingsListener = nullptr;
}

adlx_bool AdlxFeatureController::SettingsChangedCallback::On3DSettingsChanged(IADLX3DSettingsChangedEvent* event)
{
    if (!m_owner || !event)
        return true;

    std::lock_guard<std::mutex> guard(m_owner->m_lock);

    IADLX3DSettingsChangedEvent1Ptr ev1(event);

    if (ev1 && ev1->IsAMDFluidMotionFramesChanged() && m_owner->m_afmf)
        m_owner->m_afmf->IsEnabled(&m_owner->m_afmfEnabled);

    if (event->IsImageSharpeningChanged() && m_owner->m_ris)
    {
        m_owner->m_ris->IsEnabled(&m_owner->m_risEnabled);
        m_owner->m_ris->GetSharpness(&m_owner->m_risSharpness);
    }

    if (event->IsBoostChanged() && m_owner->m_boost)
    {
        m_owner->m_boost->IsEnabled(&m_owner->m_boostEnabled);
        m_owner->m_boost->GetResolution(&m_owner->m_boostResolution);
    }

    if (ev1 && ev1->IsRadeonSuperResolutionChanged() && m_owner->m_rsr)
        m_owner->m_rsr->IsEnabled(&m_owner->m_rsrEnabled);

    m_owner->m_refreshPending = true;
    return true;
}

// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::Refresh()
{
    return CallWithLock([this]() -> ADLX_RESULT
        {
            if (!m_initialized)
                return ADLX_FAIL;

            // If an event already updated state, just clear the flag
            if (m_refreshPending)
            {
                m_refreshPending = false;
                return ADLX_OK;
            }

            if (m_afmf)
                m_afmf->IsEnabled(&m_afmfEnabled);
            if (m_ris)
            {
                m_ris->IsEnabled(&m_risEnabled);
                m_ris->GetSharpness(&m_risSharpness);
            }
            if (m_rsr)
                m_rsr->IsEnabled(&m_rsrEnabled);
            if (m_boost)
            {
                m_boost->IsEnabled(&m_boostEnabled);
                m_boost->GetResolution(&m_boostResolution);
            }
            return ADLX_OK;
        });
}

// ---------------------------------------------------------------------------
//  AFMF
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::AFMF_SetEnabled(bool enable)
{
    return CallWithLock([this, enable]() -> ADLX_RESULT
        {
            if (!m_afmf)
                return ADLX_FAIL;
            ADLX_RESULT res = m_afmf->SetEnabled(enable);
            if (ADLX_SUCCEEDED(res))
                m_afmfEnabled = enable;
            return res;
        });
}

// ---------------------------------------------------------------------------
//  RIS
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::RIS_SetEnabled(bool enable)
{
    return CallWithLock([this, enable]() -> ADLX_RESULT
        {
            if (!m_ris)
                return ADLX_FAIL;
            ADLX_RESULT res = m_ris->SetEnabled(enable);
            if (ADLX_SUCCEEDED(res))
                m_risEnabled = enable;
            return res;
        });
}

ADLX_RESULT AdlxFeatureController::RIS_SetSharpness(adlx_int value)
{
    return CallWithLock([this, value]() -> ADLX_RESULT
        {
            if (!m_ris || !m_risEnabled)
                return ADLX_FAIL;
            if (value < m_risRange.minValue || value > m_risRange.maxValue)
                return ADLX_INVALID_ARGS;
            ADLX_RESULT res = m_ris->SetSharpness(value);
            if (ADLX_SUCCEEDED(res))
                m_risSharpness = value;
            return res;
        });
}

// ---------------------------------------------------------------------------
//  RSR
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::RSR_SetEnabled(bool enable)
{
    return CallWithLock([this, enable]() -> ADLX_RESULT
        {
            if (!m_rsr)
                return ADLX_FAIL;
            ADLX_RESULT res = m_rsr->SetEnabled(enable);
            if (ADLX_SUCCEEDED(res))
                m_rsrEnabled = enable;
            return res;
        });
}

// ---------------------------------------------------------------------------
//  BOOST (new)
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::Boost_SetEnabled(bool enable)                 
{
    return CallWithLock([this, enable]() -> ADLX_RESULT                          
        {
            if (!m_boost)                                                        
                return ADLX_FAIL;                                               
            ADLX_RESULT res = m_boost->SetEnabled(enable);                      
            if (ADLX_SUCCEEDED(res))
            {
                m_boostEnabled = enable;                                        
            }
            return res;                                                         
        });
}

ADLX_RESULT AdlxFeatureController::Boost_SetResolution(adlx_int value)           
{
    return CallWithLock([this, value]() -> ADLX_RESULT                           
        {
            if (!m_boost || !m_boostEnabled)                                     
                return ADLX_FAIL;                                               
            if (value < m_boostResRange.minValue || value > m_boostResRange.maxValue)
                return ADLX_INVALID_ARGS;                                       
            ADLX_RESULT res = m_boost->SetResolution(value);                    
            if (ADLX_SUCCEEDED(res))
                m_boostResolution = value;                                      
            return res;                                                         
        });
}

// ---------------------------------------------------------------------------
//  Custom Color – master toggle
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::CustomColor_SetEnabled(bool enable)
{
    return CallWithLock([this, enable]() -> ADLX_RESULT
        {
            if (!m_customColor)
                return ADLX_FAIL;

            if (!enable)
            {
                // Reset to defaults
                ADLX_RESULT r1 = m_customColor->SetBrightness(kDefaultBrightness);
                ADLX_RESULT r2 = m_customColor->SetContrast(kDefaultContrast);
                ADLX_RESULT r3 = m_customColor->SetSaturation(kDefaultSaturation);
                if (ADLX_SUCCEEDED(r1) && ADLX_SUCCEEDED(r2) && ADLX_SUCCEEDED(r3))
                {
                    m_brightness = kDefaultBrightness;
                    m_contrast = kDefaultContrast;
                    m_saturation = kDefaultSaturation;
                    m_customColorEnabled = false;
                    return ADLX_OK;
                }
                return ADLX_FAIL;
            }
            else
            {
                // Re‑apply current cached values (they may still be defaults)
                ADLX_RESULT r1 = m_customColor->SetBrightness(m_brightness);
                ADLX_RESULT r2 = m_customColor->SetContrast(m_contrast);
                ADLX_RESULT r3 = m_customColor->SetSaturation(m_saturation);
                if (ADLX_SUCCEEDED(r1) && ADLX_SUCCEEDED(r2) && ADLX_SUCCEEDED(r3))
                {
                    m_customColorEnabled = true;
                    return ADLX_OK;
                }
                return ADLX_FAIL;
            }
        });
}

// ---------------------------------------------------------------------------
//  Custom Color – individual sliders
// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::SetBrightness(adlx_int val)
{
    return CallWithLock([this, val]() -> ADLX_RESULT
        {
            if (!m_customColor)
                return ADLX_FAIL;
            if (val < m_brightnessRange.minValue || val > m_brightnessRange.maxValue)
                return ADLX_INVALID_ARGS;
            ADLX_RESULT res = m_customColor->SetBrightness(val);
            if (ADLX_SUCCEEDED(res))
            {
                m_brightness = val;
                m_customColorEnabled = true; // implicit enable
            }
            return res;
        });
}

ADLX_RESULT AdlxFeatureController::SetContrast(adlx_int val)
{
    return CallWithLock([this, val]() -> ADLX_RESULT
        {
            if (!m_customColor)
                return ADLX_FAIL;
            if (val < m_contrastRange.minValue || val > m_contrastRange.maxValue)
                return ADLX_INVALID_ARGS;
            ADLX_RESULT res = m_customColor->SetContrast(val);
            if (ADLX_SUCCEEDED(res))
            {
                m_contrast = val;
                m_customColorEnabled = true;
            }
            return res;
        });
}

ADLX_RESULT AdlxFeatureController::SetSaturation(adlx_int val)
{
    return CallWithLock([this, val]() -> ADLX_RESULT
        {
            if (!m_customColor)
                return ADLX_FAIL;
            if (val < m_saturationRange.minValue || val > m_saturationRange.maxValue)
                return ADLX_INVALID_ARGS;
            ADLX_RESULT res = m_customColor->SetSaturation(val);
            if (ADLX_SUCCEEDED(res))
            {
                m_saturation = val;
                m_customColorEnabled = true;
            }
            return res;
        });
}

ADLX_RESULT AdlxFeatureController::AddSettingsChangedListener(IADLX3DSettingsChangedListener* listener)
{
    return CallWithLock([this, listener]() -> ADLX_RESULT
        {
            if (!m_changedHandle || !listener)
                return ADLX_FAIL;
            return m_changedHandle->Add3DSettingsEventListener(listener);
        });
}

ADLX_RESULT AdlxFeatureController::RemoveSettingsChangedListener(IADLX3DSettingsChangedListener* listener)
{
    return CallWithLock([this, listener]() -> ADLX_RESULT
        {
            if (!m_changedHandle || !listener)
                return ADLX_FAIL;
            return m_changedHandle->Remove3DSettingsEventListener(listener);
        });
}

// ---------------------------------------------------------------------------
//  End of AdlxFeatureController.cpp
// ---------------------------------------------------------------------------
