#include "stdafx.h"
#include "ToastHelpers.h"

using namespace Microsoft::WRL;
using namespace ABI::Windows::UI::Notifications;
using namespace ABI::Windows::Data::Xml::Dom;
using namespace Windows::Foundation;
using namespace ABI::Windows::UI::Notifications;

const std::basic_string<wchar_t> AppId = _T("Dimitar Toeaster Reminder");
// Display the toast using classic COM. Note that is also possible to create and display the toast using the new C++ /ZW options (using handles,
// COM wrappers, etc.) 
HRESULT DisplayToast(const ToastParams &params)
{
	ComPtr<IToastNotificationManagerStatics> toastStatics;
	HRESULT hr = GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotificationManager).Get(), &toastStatics);

	if (SUCCEEDED(hr))
	{
		ComPtr<IXmlDocument> toastXml;
		hr = CreateToastXml(toastStatics.Get(), &toastXml, params);
		if (SUCCEEDED(hr))
		{
			hr = CreateToast(toastStatics.Get(), toastXml.Get(), params);
		}
	}
	return hr;
}

// Create the toast XML from a template
HRESULT CreateToastXml(_In_ IToastNotificationManagerStatics *toastManager, _Outptr_ IXmlDocument** inputXml, const ToastParams &params)
{
	// Retrieve the template XML
	HRESULT hr = toastManager->GetTemplateContent(ToastTemplateType_ToastImageAndText04, inputXml);
	if (SUCCEEDED(hr))
	{
		wchar_t *imagePath = _wfullpath(nullptr, L"toastImageAndText.png", MAX_PATH);

		hr = imagePath != nullptr ? S_OK : HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		if (SUCCEEDED(hr))
		{
			hr = SetImageSrc(imagePath, *inputXml);
			if (SUCCEEDED(hr))
			{
				hr = SetTextValues(params.vectLines, *inputXml);
			}
		}
	}
	return hr;
}

// Set the value of the "src" attribute of the "image" node
HRESULT SetImageSrc(_In_z_ wchar_t *imagePath, _In_ IXmlDocument *toastXml)
{
	wchar_t imageSrc[MAX_PATH] = L"file:///";
	HRESULT hr = StringCchCat(imageSrc, ARRAYSIZE(imageSrc), imagePath);
	if (SUCCEEDED(hr))
	{
		ComPtr<IXmlNodeList> nodeList;
		hr = toastXml->GetElementsByTagName(StringReferenceWrapper(L"image").Get(), &nodeList);
		if (SUCCEEDED(hr))
		{
			ComPtr<IXmlNode> imageNode;
			hr = nodeList->Item(0, &imageNode);
			if (SUCCEEDED(hr))
			{
				ComPtr<IXmlNamedNodeMap> attributes;

				hr = imageNode->get_Attributes(&attributes);
				if (SUCCEEDED(hr))
				{
					ComPtr<IXmlNode> srcAttribute;

					hr = attributes->GetNamedItem(StringReferenceWrapper(L"src").Get(), &srcAttribute);
					if (SUCCEEDED(hr))
					{
						hr = SetNodeValueString(StringReferenceWrapper(imageSrc).Get(), srcAttribute.Get(), toastXml);
					}
				}
			}
		}
	}
	return hr;
}

// Set the values of each of the text nodes
HRESULT SetTextValues(const std::vector<std::basic_string<wchar_t>>& vectLines, _In_ IXmlDocument *toastXml)
{
	HRESULT hr = !vectLines.empty() ? S_OK : E_INVALIDARG;
	if (SUCCEEDED(hr))
	{
		ComPtr<IXmlNodeList> nodeList;
		hr = toastXml->GetElementsByTagName(StringReferenceWrapper(L"text").Get(), &nodeList);
		if (SUCCEEDED(hr))
		{
			UINT32 nodeListLength;
			hr = nodeList->get_Length(&nodeListLength);
			if (SUCCEEDED(hr))
			{
				hr = vectLines.size() <= nodeListLength ? S_OK : E_INVALIDARG;
				if (SUCCEEDED(hr))
				{
					for (UINT32 i = 0; i < vectLines.size(); i++)
					{
						ComPtr<IXmlNode> textNode;
						hr = nodeList->Item(i, &textNode);
						if (SUCCEEDED(hr))
						{
							hr = SetNodeValueString(StringReferenceWrapper(vectLines[i].c_str(), vectLines[i].length()).Get(), textNode.Get(), toastXml);
						}
					}
				}
			}
		}
	}
	return hr;
}

HRESULT SetNodeValueString(_In_ HSTRING inputString, _In_ IXmlNode *node, _In_ IXmlDocument *xml)
{
	ComPtr<IXmlText> inputText;

	HRESULT hr = xml->CreateTextNode(inputString, &inputText);
	if (SUCCEEDED(hr))
	{
		ComPtr<IXmlNode> inputTextNode;

		hr = inputText.As(&inputTextNode);
		if (SUCCEEDED(hr))
		{
			ComPtr<IXmlNode> pAppendedChild;
			hr = node->AppendChild(inputTextNode.Get(), &pAppendedChild);
		}
	}

	return hr;
}

// Create and display the toast
HRESULT CreateToast(_In_ IToastNotificationManagerStatics *toastManager, _In_ IXmlDocument *xml, const ToastParams &params)
{
	ComPtr<IToastNotifier> notifier;
	HRESULT hr = toastManager->CreateToastNotifierWithId(StringReferenceWrapper(AppId.c_str(), AppId.length()).Get(), &notifier);
	if (SUCCEEDED(hr))
	{
		ComPtr<IToastNotificationFactory> factory;
		hr = GetActivationFactory(StringReferenceWrapper(RuntimeClass_Windows_UI_Notifications_ToastNotification).Get(), &factory);
		if (SUCCEEDED(hr))
		{
			ComPtr<IToastNotification> toast;
			hr = factory->CreateToastNotification(xml, &toast);
			if (SUCCEEDED(hr))
			{
				if (params.bHandles)
				{
					// Register the event handlers
					EventRegistrationToken activatedToken, dismissedToken, failedToken;
					ComPtr<ToastEventHandler> eventHandler(new ToastEventHandler(params.hwnd, params.hEdit));

					hr = toast->add_Activated(eventHandler.Get(), &activatedToken);
					if (SUCCEEDED(hr))
					{
						hr = toast->add_Dismissed(eventHandler.Get(), &dismissedToken);
						if (SUCCEEDED(hr))
						{
							hr = toast->add_Failed(eventHandler.Get(), &failedToken);
							if (SUCCEEDED(hr))
							{
								hr = notifier->Show(toast.Get());
							}
						}
					}
				}
				else
					if (SUCCEEDED(hr))
					{
						hr = notifier->Show(toast.Get());
					}
			}
		}
	}
	return hr;
}

ToastEventHandler::ToastEventHandler(_In_ HWND hToActivate, _In_ HWND hEdit) : _ref(1), _hToActivate(hToActivate), _hEdit(hEdit)
{

}

ToastEventHandler::~ToastEventHandler()
{

}

// DesktopToastActivatedEventHandler
IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification* /* sender */, _In_ IInspectable* /* args */)
{
	BOOL succeeded = SetForegroundWindow(_hToActivate);
	if (succeeded)
	{
		LRESULT result = SendMessage(_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(L"The user clicked on the toast."));
		succeeded = result ? TRUE : FALSE;
	}
	return succeeded ? S_OK : E_FAIL;
}

// DesktopToastDismissedEventHandler
IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification* /* sender */, _In_ IToastDismissedEventArgs* e)
{
	ToastDismissalReason tdr;
	HRESULT hr = e->get_Reason(&tdr);
	if (SUCCEEDED(hr))
	{
		wchar_t *outputText;
		switch (tdr)
		{
		case ToastDismissalReason_ApplicationHidden:
			outputText = L"The application hid the toast using ToastNotifier.hide()";
			break;
		case ToastDismissalReason_UserCanceled:
			outputText = L"The user dismissed this toast";
			break;
		case ToastDismissalReason_TimedOut:
			outputText = L"The toast has timed out";
			break;
		default:
			outputText = L"Toast not activated";
			break;
		}

		LRESULT succeeded = SendMessage(_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(outputText));
		hr = succeeded ? S_OK : E_FAIL;
	}
	return hr;
}

// DesktopToastFailedEventHandler
IFACEMETHODIMP ToastEventHandler::Invoke(_In_ IToastNotification* /* sender */, _In_ IToastFailedEventArgs* /* e */)
{
	LRESULT succeeded = SendMessage(_hEdit, WM_SETTEXT, reinterpret_cast<WPARAM>(nullptr), reinterpret_cast<LPARAM>(L"The toast encountered an error."));
	return succeeded ? S_OK : E_FAIL;
}
