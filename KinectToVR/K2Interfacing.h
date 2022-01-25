#pragma once
#include "pch.h"
#include "K2Shared.h"

namespace k2app
{
	namespace interfacing
	{
		inline void ShowToast(std::string const& header, std::string const& text)
		{
			using namespace winrt::Windows::UI::Notifications;
			using namespace winrt::Windows::Data::Xml::Dom;

			// Construct the XML toast template
			XmlDocument document;
			document.LoadXml(L"\
				<toast>\
					<visual>\
				        <binding template=\"ToastGeneric\">\
				            <text></text>\
				            <text></text>\
				        </binding>\
				    </visual>\
				</toast>");

			// Populate with text and values
			document.SelectSingleNode(L"//text[1]").InnerText(wstring_cast(header));
			document.SelectSingleNode(L"//text[2]").InnerText(wstring_cast(text));

			// Construct the notification
			ToastNotification notification{ document };
			ToastNotifier toastNotifier{ ToastNotificationManager::CreateToastNotifier() };

			// And show it!
			toastNotifier.Show(notification);
		}

		inline bool SpawnDefaultEnabledTrackers()
		{
			return true; // TODO
		}

		inline void CheckK2ServerDriver()
		{
			// TODO
		}
	}
}