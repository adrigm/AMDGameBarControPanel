﻿#include "pch.h"
#include "AdlxFeatureController.h"

// ============================================================================
//  Implementation
// ============================================================================

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
    return CallWithLock([this]() -> ADLX_RESULT
        {
            if (m_initialized)
                return ADLX_OK;

            ADLX_RESULT res = m_adlx.Initialize();
            if (!ADLX_SUCCEEDED(res))
            {
                m_errorMessage = L"ADLX initialization failed";
                return res;
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

            // RIS
            {
                // Need a GPU object for RIS
                IADLXGPUListPtr gpuList;
                res = m_adlx.GetSystemServices()->GetGPUs(&gpuList);
                if (ADLX_SUCCEEDED(res) && gpuList && !gpuList->Empty())
                {
                    IADLXGPUPtr gpu;
                    gpuList->At(0, &gpu);
                    if (gpu)
                    {
                        d3dSrv->GetImageSharpening(gpu, &m_ris);
                        if (m_ris)
                        {
                            m_ris->IsEnabled(&m_risEnabled);
                            m_ris->GetSharpness(&m_risSharpness);
                            m_ris->GetSharpnessRange(&m_risRange);
                        }
                    }
                }
            }

            // RSR (does not require GPU handle)
            d3dSrv->GetRadeonSuperResolution(&m_rsr);
            if (m_rsr)
                m_rsr->IsEnabled(&m_rsrEnabled);

            // -------------------------------------------------------------
            // Display / Custom Color
            // -------------------------------------------------------------
            res = m_adlx.GetSystemServices()->GetDisplaysServices(&m_displaySrv);
            if (ADLX_SUCCEEDED(res) && m_displaySrv)
            {
                IADLXDisplayListPtr displayList;
                if (ADLX_SUCCEEDED(m_displaySrv->GetDisplays(&displayList)) && displayList && !displayList->Empty())
                {
                    IADLXDisplayPtr display;
                    displayList->At(0, &display);
                    if (display)
                    {
                        m_displaySrv->GetCustomColor(display, &m_customColor);
                        if (m_customColor)
                        {
                            m_customColor->GetBrightness(&m_brightness);
                            m_customColor->GetContrast(&m_contrast);
                            m_customColor->GetSaturation(&m_saturation);
                            m_customColor->GetBrightnessRange(&m_brightnessRange);
                            m_customColor->GetContrastRange(&m_contrastRange);
                            m_customColor->GetSaturationRange(&m_saturationRange);
                            // Assume enabled if any value differs from default
                            m_customColorEnabled = (m_brightness != kDefaultBrightness ||
                                m_contrast != kDefaultContrast ||
                                m_saturation != kDefaultSaturation);
                        }
                    }
                }
            }

            // Success path
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
        });
}

void AdlxFeatureController::ClearFeaturePointers()
{
    m_afmf = nullptr;
    m_ris = nullptr;
    m_rsr = nullptr;
    m_customColor = nullptr;
    m_displaySrv = nullptr;
}

// ---------------------------------------------------------------------------
ADLX_RESULT AdlxFeatureController::Refresh()
{
    return CallWithLock([this]() -> ADLX_RESULT
        {
            if (!m_initialized)
                return ADLX_FAIL;

            if (m_afmf)
                m_afmf->IsEnabled(&m_afmfEnabled);
            if (m_ris)
            {
                m_ris->IsEnabled(&m_risEnabled);
                m_ris->GetSharpness(&m_risSharpness);
            }
            if (m_rsr)
                m_rsr->IsEnabled(&m_rsrEnabled);
            if (m_customColor)
            {
                m_customColor->GetBrightness(&m_brightness);
                m_customColor->GetContrast(&m_contrast);
                m_customColor->GetSaturation(&m_saturation);
                m_customColorEnabled = (m_brightness != kDefaultBrightness ||
                    m_contrast != kDefaultContrast ||
                    m_saturation != kDefaultSaturation);
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

// ---------------------------------------------------------------------------
//  End of AdlxFeatureController.cpp
// ---------------------------------------------------------------------------
