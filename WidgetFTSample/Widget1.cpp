#include "pch.h"
#include "Widget1.h"
#include "Widget1.g.cpp"

using namespace winrt;
using namespace winrt::Windows::Foundation;
using namespace winrt::Windows::UI::Xaml;
using namespace winrt::Windows::UI::Xaml::Navigation;

namespace winrt::WidgetFTSample::implementation
{
    Widget1::Widget1()
    {
        InitializeComponent();

        WidgetFTFactory(winrt::WidgetFTSample::GetWidgetFTFactory());
        if (auto ftFactory{ WidgetFTFactory() })
        {
            SampleComponent(ftFactory.CreateSampleComponent());

            if (auto sampleComponent{ SampleComponent() })

            {
                sampleComponent.Init();

                //if (sampleComponent.AFMF_Supported()) {
                //}
			}
		}
    }

    void Widget1::OnNavigatedTo(winrt::Windows::UI::Xaml::Navigation::NavigationEventArgs const& e)
    {
        

        m_timer.Interval(std::chrono::seconds(3));
        m_timer.Tick({ this, &Widget1::Refresh });
        m_timer.Start();

    }

    winrt::fire_and_forget Widget1::Refresh(IInspectable const&, IInspectable const&)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        if (auto sampleComponent{ SampleComponent() })
        {
            sampleComponent.Refresh();

            bool afmfStatus = sampleComponent.AFMF_Enabled();

            if (afmfStatus != m_isAFMFEnabled) {
                IsAFMFEnabled(afmfStatus);
            }


        }
    }

    winrt::fire_and_forget Widget1::CreateFTFactoryButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        WidgetFTFactory(winrt::WidgetFTSample::GetWidgetFTFactory());
    }

    winrt::fire_and_forget Widget1::ReleaseFTFactoryButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        WidgetFTFactory(nullptr);
    }

    winrt::fire_and_forget Widget1::CreateSampleComponentButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        if (auto ftFactory{ WidgetFTFactory() })
        {
            SampleComponent(ftFactory.CreateSampleComponent());
        }

        // After we create the sample component, register for the bool property change event
        if (auto sampleComponent{ SampleComponent() })
        {
            m_demoBoolPropertyChangedRevoker = 
                sampleComponent.DemoBoolPropertyChanged(winrt::auto_revoke, { this, &Widget1::RaiseDemoBoolPropertyChanged });
        }
    }

    winrt::fire_and_forget Widget1::ReleaseSampleComponentButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();

        if (auto sampleComponent{ SampleComponent() })
        {
            // Unregister the event handler if needed
            m_demoBoolPropertyChangedRevoker.revoke();
        }

        SampleComponent(nullptr);
    }

    winrt::fire_and_forget Widget1::CallDemoSyncButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        winrt::apartment_context ui_thread;

        if (auto sampleComponent{ SampleComponent() })
        {
            co_await winrt::resume_background();
            OutputDebugStringW(L"[Widget] Calling DemoSync\n");
            sampleComponent.DemoSync();
            OutputDebugStringW(L"[Widget] Calling DemoSync completed\n");
        }
    }

    winrt::fire_and_forget Widget1::CallDemoAsyncButton_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        winrt::apartment_context ui_thread;

        if (auto sampleComponent{ SampleComponent() })
        {
            co_await winrt::resume_background();
            OutputDebugStringW(L"[Widget] Calling DemoAsync\n");
            co_await sampleComponent.DemoAsync();
            OutputDebugStringW(L"[Widget] Calling DemoAsync completed\n");
        }
    }

    winrt::fire_and_forget Widget1::DemoBoolPropertyCheckBox_Click(
        winrt::Windows::Foundation::IInspectable,
        winrt::Windows::UI::Xaml::RoutedEventArgs)
    {
        auto strongThis{ get_strong() };
        winrt::apartment_context ui_thread;

        if (auto sampleComponent{ SampleComponent() })
        {
            co_await winrt::resume_background();
            auto currentValue{ sampleComponent.DemoBoolProperty() };
            sampleComponent.DemoBoolProperty(!currentValue);

            RaisePropertyChanged(L"DemoBoolProperty");
        }
    }

    winrt::event_token Widget1::PropertyChanged(Windows::UI::Xaml::Data::PropertyChangedEventHandler const& handler)
    {
        return m_propertyChanged.add(handler);
    }

    void Widget1::PropertyChanged(winrt::event_token const& token) noexcept
    {
        m_propertyChanged.remove(token);
    }

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

    winrt::fire_and_forget Widget1::btnAfmfClick(winrt::Windows::Foundation::IInspectable const& sender, winrt::Windows::UI::Xaml::RoutedEventArgs const& e)
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_background();
        IsAFMFEnabled(!IsAFMFEnabled());
    }

    winrt::WidgetFT::WidgetFTFactory Widget1::WidgetFTFactory()
    {
        auto lock{ m_ftFactoryLock.lock_shared() };
        return m_ftFactory;
    }

    void Widget1::WidgetFTFactory(winrt::WidgetFT::WidgetFTFactory const& value)
    {
        auto lock{ m_ftFactoryLock.lock_exclusive() };
        m_ftFactory = value;
        RaiseObjectStatesChanged();
    }

    winrt::WidgetFT::SampleComponent Widget1::SampleComponent()
    {
        auto lock{ m_sampleComponentLock.lock_shared() };
        return m_sampleComponent;
    }

    void Widget1::SampleComponent(winrt::WidgetFT::SampleComponent const& value)
    {
        auto lock{ m_sampleComponentLock.lock_exclusive() };
        m_sampleComponent = value;
        RaiseObjectStatesChanged();
    }

    void Widget1::RaiseDemoBoolPropertyChanged(bool newValue)
    {
        // newValue is not used in this sample. It has the new value, but when the 
        // property changed is raised, we just go back to the full trust server to read the
        // value. It's probably more optimal to cache the value on the client side here, but this
        // is just demonstrating one approach.
        if (newValue)
        {
            OutputDebugStringW(L"DemoBoolPropertyChanged: newValue(true)\n");
        }
        else
        {
            OutputDebugStringW(L"DemoBoolPropertyChanged: newValue(false)\n");
        }

        RaisePropertyChanged(L"IsDemoBoolPropertyChecked");
    }

    void Widget1::RaiseObjectStatesChanged()
    {
        RaisePropertyChanged(L"CreateFTFactoryButtonEnabled");
        RaisePropertyChanged(L"ReleaseFTFactoryButtonEnabled");
        RaisePropertyChanged(L"CreateSampleComponentButtonEnabled");
        RaisePropertyChanged(L"ReleaseSampleComponentButtonEnabled");
        RaisePropertyChanged(L"CallDemoSyncButtonEnabled");
        RaisePropertyChanged(L"CallDemoAsyncButtonEnabled");
        RaisePropertyChanged(L"DemoBoolPropertyCheckBoxEnabled");
    }

    winrt::fire_and_forget Widget1::RaisePropertyChanged(winrt::hstring propertyName) try
    {
        auto strongThis{ get_strong() };
        co_await winrt::resume_foreground(Dispatcher());
        m_propertyChanged(*this, Windows::UI::Xaml::Data::PropertyChangedEventArgs{ propertyName });
    }
    catch (...)
    {
        // Handle exception if you care that delegates threw
    }
}
