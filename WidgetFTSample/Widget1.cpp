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
        WidgetFTFactory(winrt::WidgetFTSample::GetWidgetFTFactory());
        if (auto ftFactory{ WidgetFTFactory() })
        {
            SampleComponent(ftFactory.CreateSampleComponent());

            if (auto sampleComponent{ SampleComponent() })
            {
                sampleComponent.Init();

                // Ejemplo: comprobar soporte AFMF
                // if (sampleComponent.AFMF_Supported()) { }
            }
        }
    }

    //----------------------------------------------------------------------------
    //  Navegación
    //----------------------------------------------------------------------------
    void Widget1::OnNavigatedTo(
        winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& /*e*/)
    {
        // Arranca un temporizador que refresca cada 2 segundos
        m_timer.Interval(std::chrono::seconds(2));
        m_timer.Tick({ this, &Widget1::Refresh });
        m_timer.Start();
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

    void Widget1::IsRISEnabled(bool value)
    {
        m_isRISEnabled = value;
        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.RIS_SetEnabled(value);
        }
        RaisePropertyChanged(L"IsRISEnabled");
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
} // namespace winrt::WidgetFTSample::implementation
