#pragma once
#include "Widget1.g.h"

namespace winrt::WidgetFTSample::implementation
{
    //------------------------------------------------------------------------------
    //  Clase principal del widget
    //------------------------------------------------------------------------------
    struct Widget1 : Widget1T<Widget1>
    {
        //--------------------------------------------------------------------------
        //  Construcción y navegación
        //--------------------------------------------------------------------------
        Widget1();
        void OnNavigatedTo(
            winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e);

    public: // ------------------------ Interface pública (IDL) --------------------

        // Propiedad: indica si AFMF está habilitado
        bool IsAFMFEnabled();
        void IsAFMFEnabled(bool value);

        // Manejador de clic del botón que conmuta AFMF
        winrt::fire_and_forget btnAfmfClick(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

		// Manejador de clic del botón que muestra el estado de RIS
		bool IsRISEnabled();
        winrt::fire_and_forget IsRISEnabled(bool value);

        // Manejador de clic del botón que conmuta RIS
        winrt::fire_and_forget btnRisClick(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        // Propiedad: indica si RSR está habilitado
        bool IsRSREnabled();
        void IsRSREnabled(bool value);

        // Manejador de clic del botón que conmuta RSR
        winrt::fire_and_forget btnRsrClick(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        // Propiedad: valor de nitidez RIS (0-100)
        int SharpnessValue();
        winrt::fire_and_forget SharpnessValue(int value);

        // Soporte INotifyPropertyChanged
        winrt::event_token PropertyChanged(
            Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler);
        void PropertyChanged(winrt::event_token const& token) noexcept;

        bool IsBoostEnabled();
        winrt::fire_and_forget IsBoostEnabled(bool value);

        winrt::fire_and_forget btnBoostClick(
            winrt::Windows::Foundation::IInspectable const& sender,
            winrt::Windows::UI::Xaml::RoutedEventArgs const& e);

        // 0 = Calidad | 1 = Rendimiento
        int BoostMode();
        winrt::fire_and_forget BoostMode(int value);

    private: // ----------------------- Acceso a propiedades internas --------------

        // WidgetFTFactory (getter / setter)
        winrt::WidgetFT::WidgetFTFactory  WidgetFTFactory();
        void WidgetFTFactory(
            winrt::WidgetFT::WidgetFTFactory const& value);

        // SampleComponent (getter / setter)
        winrt::WidgetFT::SampleComponent  SampleComponent();
        void SampleComponent(
            winrt::WidgetFT::SampleComponent const& value);

        // Notificaciones de cambio de estado/propiedad
        void RaiseObjectStatesChanged();
        winrt::fire_and_forget RaisePropertyChanged(
            winrt::hstring propertyName);

    private: // ----------------------- Estado interno ------------------------------

        // Revoker para la propiedad DemoBool del SampleComponent
        winrt::WidgetFT::SampleComponent::DemoBoolPropertyChanged_revoker
            m_demoBoolPropertyChangedRevoker;

        // Evento PropertyChanged típico de MVVM
        winrt::event<Windows::UI::Xaml::Data::PropertyChangedEventHandler>
            m_propertyChanged;

        // Sincronización y caché de la fábrica
        wil::srwlock                     m_ftFactoryLock;
        winrt::WidgetFT::WidgetFTFactory m_ftFactory{ nullptr };

        // Sincronización y caché del componente de ejemplo
        wil::srwlock                     m_sampleComponentLock;
        winrt::WidgetFT::SampleComponent m_sampleComponent{ nullptr };

        // Temporizador para actualizaciones periódicas
        winrt::Windows::UI::Xaml::DispatcherTimer m_timer;
        winrt::fire_and_forget Refresh(
            winrt::Windows::Foundation::IInspectable const&,
            winrt::Windows::Foundation::IInspectable const&);

        // Init Status
		bool m_isInitialized{ false };

        // Estado actual de AFMF
        bool m_isAFMFEnabled{ false };

		// Estado actual de RIS
		bool m_isRISEnabled{ false };

        // Estado actual de RSR
        bool m_isRSREnabled{ false };
        // Valor de nitidez RIS (0-100)
		int m_sharpnessValue{ 0 }; // Valor predeterminado

        // Rango actual del slider RIS
        int m_sharpnessMin{ 0 };
        int m_sharpnessMax{ 100 };

        bool m_isBoostEnabled{ false };
        int  m_boostMode{ 0 };     // 0-Calidad | 1-Rendimiento
        winrt::fire_and_forget SetVisibilityBoostMode();

        winrt::fire_and_forget SetVisibilitySharpness();

        // Actualiza título y rango del slider en UI-thread
        void UpdateSharpnessRange(int min, int max);

        winrt::fire_and_forget UpdateInitializationState();

    public:
        void TextBlock_SelectionChanged(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e);
};
} // namespace winrt::WidgetFTSample::implementation

namespace winrt::WidgetFTSample::factory_implementation
{
    //------------------------------------------------------------------------------
    //  Plantilla de la fábrica generada por cppwinrt
    //------------------------------------------------------------------------------
    struct Widget1 : Widget1T<Widget1, implementation::Widget1> {};
} // namespace winrt::WidgetFTSample::factory_implementation
