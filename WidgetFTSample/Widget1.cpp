 #include "pch.h"
#include "Widget1.h"
#include "Widget1.g.cpp"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;

namespace winrt::WidgetFTSample::implementation
{
    //----------------------------------------------------------------------------
    //  Constructor
    //----------------------------------------------------------------------------
    Widget1::Widget1()
    {
        InitializeComponent();


        // Inicializar la fábrica y el componente de ejemplo
        try
        {
            WidgetFTFactory(winrt::WidgetFTSample::GetWidgetFTFactory());
        }
        catch (...)
        {
            WidgetFTFactory(nullptr);
        }
        if (auto ftFactory{ WidgetFTFactory() })
        {
            SampleComponent(ftFactory.CreateSampleComponent());

            if (auto sampleComponent{ SampleComponent() })
            {
                m_isInitialized = sampleComponent.Init();

                if (m_isInitialized)
                {
					SetVisibilitySharpness();
                    m_sharpnessValue = sampleComponent.RIS_Sharpness();
                    m_sharpnessMin = sampleComponent.RIS_SharpnessMin();
                    m_sharpnessMax = sampleComponent.RIS_SharpnessMax();
                    UpdateSharpnessRange(m_sharpnessMin, m_sharpnessMax);

                    Refresh(nullptr, nullptr); // Llamar al refresco inicial
                }

                // Ejemplo: comprobar soporte AFMF
                // if (sampleComponent.AFMF_Supported()) { }
            }
        }

        UpdateInitializationState();
    }

    Widget1::~Widget1()
    {
        m_timer.Stop();
        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.SettingsChanged(m_settingsToken);
        }
    }

    //----------------------------------------------------------------------------
    //  Navegación
    //----------------------------------------------------------------------------
    void Widget1::OnNavigatedTo(
        winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& /*e*/)
    {
        if (auto sampleComponent{ SampleComponent() })
        {
            m_settingsToken = sampleComponent.SettingsChanged({ this, &Widget1::OnSettingsChanged });
        }
    }

    //----------------------------------------------------------------------------
    //  Refresco periódico
    //----------------------------------------------------------------------------
    winrt::fire_and_forget Widget1::Refresh(
        IInspectable const& /*sender*/,
        IInspectable const& /*args*/)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        if (!m_isInitialized)
            co_return;

        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.Refresh();

            bool afmfStatus = sampleComponent.AFMF_Enabled();
            if (afmfStatus != m_isAFMFEnabled)
            {
                IsAFMFEnabled(afmfStatus);
            }

			bool risStatus = sampleComponent.RIS_Enabled();
            if (risStatus != m_isRISEnabled)
            {
                IsRISEnabled(risStatus);
			}

            bool rsrStatus = sampleComponent.RSR_Enabled();
            if (rsrStatus != m_isRSREnabled)
            {
                IsRSREnabled(rsrStatus);
                SetVisibilitySharpness();
            }

            int sharp = sampleComponent.RIS_Sharpness();

            if (sharp != m_sharpnessValue)
            {
                SharpnessValue(sharp);
                
            }

            int newMin = sampleComponent.RIS_SharpnessMin();
            int newMax = sampleComponent.RIS_SharpnessMax();
            if (newMin != m_sharpnessMin || newMax != m_sharpnessMax)
            {
                UpdateSharpnessRange(newMin, newMax);
            }

            bool boostStatus = sampleComponent.Boost_Enabled();
            if (boostStatus != m_isBoostEnabled)
            {
                IsBoostEnabled(boostStatus);
            }

            // --- Sincronización del modo Boost ---------------------------
            int resCur = sampleComponent.Boost_Resolution();
            int resMin = sampleComponent.Boost_ResolutionMin();
            int resMax = sampleComponent.Boost_ResolutionMax();

            int mode = ResolutionToBoostMode(resCur, resMin, resMax);

            if (mode != m_boostMode)
            {
                // Solo actualizamos el backing-field y notificamos;
                // no escribimos de nuevo en el driver.
                m_boostMode = mode;

                co_await winrt::resume_foreground(Dispatcher());
                RaisePropertyChanged(L"BoostMode");
            }
        }
    }

    //----------------------------------------------------------------------------
    //  Implementación de INotifyPropertyChanged
    //----------------------------------------------------------------------------
    winrt::event_token Widget1::PropertyChanged(
        Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void Widget1::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

    //----------------------------------------------------------------------------
    //  Propiedad IsAFMFEnabled
    //----------------------------------------------------------------------------
    bool Widget1::IsAFMFEnabled()
    {
        return m_isAFMFEnabled;
    }

    void Widget1::IsAFMFEnabled(bool value)
    {
        m_isAFMFEnabled = value;

        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.AFMF_SetEnabled(value);
        }

        RaisePropertyChanged(L"IsAFMFEnabled");
    }

    //----------------------------------------------------------------------------
    //  Manejador de clic del botón AFMF
    //----------------------------------------------------------------------------
    winrt::fire_and_forget Widget1::btnAfmfClick(
        winrt::Windows::Foundation::IInspectable const& /*sender*/,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        IsAFMFEnabled(!IsAFMFEnabled());  // Conmutar estado
    }

    //----------------------------------------------------------------------------
    //  Propiedad IsRISEnabled
    //----------------------------------------------------------------------------
    bool Widget1::IsRISEnabled()
    {
        return m_isRISEnabled;
	}

    winrt::fire_and_forget Widget1::IsRISEnabled(bool value)
    {
        if (m_isRISEnabled == value) return;

        auto strongThis{ get_strong() };
        co_await winrt::resume_background();


        m_isRISEnabled = value;
        SetVisibilitySharpness();

        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.RIS_SetEnabled(value);
        }
        RaisePropertyChanged(L"IsRISEnabled");
	}

    void Widget1::UpdateSharpnessRange(int min, int max)
    {
        m_sharpnessMin = min;
        m_sharpnessMax = max;

        // Ajustamos el rango del slider (propiedades son double)
        risSharpnessSlider().Minimum(static_cast<double>(min));
        risSharpnessSlider().Maximum(static_cast<double>(max));

        // Título con números claros
        std::wstring text = L"Sharpness (" + std::to_wstring(min) +
            L" - " + std::to_wstring(max) + L")";
        sharpnessTitle().Text(text);
    }

    //----------------------------------------------------------------------------
    //  Manejador de clic del botón RIS
    //----------------------------------------------------------------------------
    winrt::fire_and_forget Widget1::btnRisClick(
        winrt::Windows::Foundation::IInspectable const& /*sender*/,
        winrt::Windows::UI::Xaml::RoutedEventArgs const& /*e*/)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        IsRISEnabled(!IsRISEnabled());  // Conmutar estado
	}


    // --------------------- IsRSREnabled ---------------------
    bool Widget1::IsRSREnabled()
    {
        return m_isRSREnabled;
    }

    void Widget1::IsRSREnabled(bool value)
    {
        m_isRSREnabled = value;
        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.RSR_SetEnabled(value);
        }
        RaisePropertyChanged(L"IsRSREnabled");
    }

    // --------------------- btnRsrClick ----------------------
    winrt::fire_and_forget Widget1::btnRsrClick(
        IInspectable const&,
        RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        IsRSREnabled(!IsRSREnabled());   // Conmutar
    }

    // -------------------- SharpnessValue --------------------
    int Widget1::SharpnessValue()
    {
        return m_sharpnessValue;
    }

    winrt::fire_and_forget Widget1::SharpnessValue(int value)
    {
        m_sharpnessValue = value;

        if (m_sharpnessValue) {
            co_await winrt::resume_foreground(Dispatcher());
            sharpnessCurrent().Text(L"(Current: " + std::to_wstring(m_sharpnessValue) + L")");
        }
        
        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.RIS_SetSharpness(value);
        }
        RaisePropertyChanged(L"SharpnessValue");
    }


    winrt::fire_and_forget Widget1::UpdateInitializationState()
    {
		auto strongThis{ get_strong() };
		co_await winrt::resume_foreground(Dispatcher());

        // Asegúrate de estar en el hilo UI
        contentPanel().Visibility(
            m_isInitialized
            ? Windows::UI::Xaml::Visibility::Visible
            : Windows::UI::Xaml::Visibility::Collapsed);

        unsupportedText().Visibility(
            m_isInitialized
            ? Windows::UI::Xaml::Visibility::Collapsed
            : Windows::UI::Xaml::Visibility::Visible);
    }

    winrt::fire_and_forget Widget1::SetVisibilitySharpness()
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_foreground(Dispatcher());

        if (m_isRISEnabled)
        {
			risSliderPanel().Visibility(Windows::UI::Xaml::Visibility::Visible);
        }
        else
        {
			risSliderPanel().Visibility(Windows::UI::Xaml::Visibility::Collapsed);
        }
    }




    winrt::fire_and_forget Widget1::SetVisibilityBoostMode()
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_foreground(Dispatcher());

        boostModePanel().Visibility(
            m_isBoostEnabled
            ? Windows::UI::Xaml::Visibility::Visible
            : Windows::UI::Xaml::Visibility::Collapsed);
    }

    bool Widget1::IsBoostEnabled()
    {
        return m_isBoostEnabled;
    }

    winrt::fire_and_forget Widget1::IsBoostEnabled(bool value)
    {
        if (m_isBoostEnabled == value) co_return;

        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        m_isBoostEnabled = value;
        SetVisibilityBoostMode();

        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.Boost_SetEnabled(value);
        }
        RaisePropertyChanged(L"IsBoostEnabled");
    }

    winrt::fire_and_forget Widget1::btnBoostClick(
        IInspectable const&, RoutedEventArgs const&)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        IsBoostEnabled(!IsBoostEnabled());   // Conmutar
    }

    int Widget1::BoostMode()
    {
        return m_boostMode;
    }

    winrt::fire_and_forget Widget1::BoostMode(int value)
    {
        if (value == m_boostMode) co_return;

        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        m_boostMode = value;

        if (auto sampleComponent{ SampleComponent() })
        {
            // 0 = Calidad → PerfMin | 1 = Rendimiento → PerfMax
            (value == 0) ? sampleComponent.Boost_SetPerfMin()
                : sampleComponent.Boost_SetPerfMax();
        }
        RaisePropertyChanged(L"BoostMode");
    }

    // --------------

    int Widget1::ResolutionToBoostMode(int res, int resMin, int resMax)
    {
        // 0 = Calidad (resolución al máximo)
        // 1 = Rendimiento (resolución al mínimo)
        return (res == resMax) ? 1 : 0;
    }


    //----------------------------------------------------------------------------
    //  WidgetFTFactory  (getter / setter)
    //----------------------------------------------------------------------------
    winrt::WidgetFT::WidgetFTFactory Widget1::WidgetFTFactory()
    {
        auto lock{ m_ftFactoryLock.lock_shared() };
        return m_ftFactory;
    }

    void Widget1::WidgetFTFactory(
        winrt::WidgetFT::WidgetFTFactory const& value)
    {
        auto lock{ m_ftFactoryLock.lock_exclusive() };
        m_ftFactory = value;
        RaiseObjectStatesChanged();
    }

    //----------------------------------------------------------------------------
    //  SampleComponent (getter / setter)
    //----------------------------------------------------------------------------
    winrt::WidgetFT::SampleComponent Widget1::SampleComponent()
    {
        auto lock{ m_sampleComponentLock.lock_shared() };
        return m_sampleComponent;
    }

    void Widget1::SampleComponent(
        winrt::WidgetFT::SampleComponent const& value)
    {
        auto lock{ m_sampleComponentLock.lock_exclusive() };
        m_sampleComponent = value;
        RaiseObjectStatesChanged();
    }

    //----------------------------------------------------------------------------
    //  Actualiza el estado de controles vinculados
    //----------------------------------------------------------------------------
    void Widget1::RaiseObjectStatesChanged()
    {
        // Solo notificamos las propiedades realmente expuestas al enlace de datos.
        RaisePropertyChanged(L"WidgetFTFactory");
        RaisePropertyChanged(L"SampleComponent");
    }

    //----------------------------------------------------------------------------
    //  Envía notificación PropertyChanged de forma segura al hilo UI
    //----------------------------------------------------------------------------
    winrt::fire_and_forget Widget1::RaisePropertyChanged(
        winrt::hstring propertyName) try
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_foreground(Dispatcher());
        m_propertyChanged(*this,
            Windows::UI::Xaml::Data::PropertyChangedEventArgs{ propertyName });
    }
    catch (...)
    {
        // Manejar la excepción si los delegados lanzan
    }

    void Widget1::OnSettingsChanged(
        winrt::WidgetFT::SampleComponent const& /*sender*/,
        winrt::Windows::Foundation::IInspectable const& /*args*/)
    {
        Refresh(nullptr, nullptr);
    }
} // namespace winrt::WidgetFTSample::implementation

void winrt::WidgetFTSample::implementation::Widget1::TextBlock_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
{

}
