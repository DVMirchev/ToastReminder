#pragma once

struct ToastParams
{
	std::vector<std::basic_string<wchar_t>> vectLines;
	std::basic_string<wchar_t> imagePath;
	std::basic_string<wchar_t> audioPath;
	bool bHandles = false;
	HWND hwnd = nullptr;
	HWND hEdit = nullptr;
};

HRESULT DisplayToast(const ToastParams &params);

HRESULT CreateToastXml(
	_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager,
	_Outptr_ ABI::Windows::Data::Xml::Dom::IXmlDocument **xml,
	_In_ const ToastParams &params
	);

HRESULT CreateToast(
	_In_ ABI::Windows::UI::Notifications::IToastNotificationManagerStatics *toastManager,
	_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml,
	_In_ const ToastParams &params
	);
HRESULT SetImageSrc(
	_In_z_ wchar_t *imagePath,
	_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
	);
HRESULT SetTextValues(
	const std::vector<std::basic_string<wchar_t>>& vectLines,
	_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *toastXml
	);
HRESULT SetNodeValueString(
	_In_ HSTRING onputString,
	_In_ ABI::Windows::Data::Xml::Dom::IXmlNode *node,
	_In_ ABI::Windows::Data::Xml::Dom::IXmlDocument *xml
	);

class StringReferenceWrapper
{
public:

	// Constructor which takes an existing string buffer and its length as the parameters.
	// It fills an HSTRING_HEADER struct with the parameter.      
	// Warning: The caller must ensure the lifetime of the buffer outlives this      
	// object as it does not make a copy of the wide string memory.      

	StringReferenceWrapper(_In_reads_(length) PCWSTR stringRef, _In_ UINT32 length) throw()
	{
		HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);

		if (FAILED(hr))
		{
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
	}

	~StringReferenceWrapper()
	{
		WindowsDeleteString(_hstring);
	}

	template <size_t N>
	StringReferenceWrapper(_In_reads_(N) wchar_t const (&stringRef)[N]) throw()
	{
		UINT32 length = N - 1;
		HRESULT hr = WindowsCreateStringReference(stringRef, length, &_header, &_hstring);

		if (FAILED(hr))
		{
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}
	}

	template <size_t _>
	StringReferenceWrapper(_In_reads_(_) wchar_t(&stringRef)[_]) throw()
	{
		UINT32 length;
		HRESULT hr = SizeTToUInt32(wcslen(stringRef), &length);

		if (FAILED(hr))
		{
			RaiseException(static_cast<DWORD>(STATUS_INVALID_PARAMETER), EXCEPTION_NONCONTINUABLE, 0, nullptr);
		}

		WindowsCreateStringReference(stringRef, length, &_header, &_hstring);
	}

	HSTRING Get() const throw()
	{
		return _hstring;
	}


private:
	HSTRING             _hstring;
	HSTRING_HEADER      _header;
};

typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ::IInspectable *> DesktopToastActivatedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastDismissedEventArgs *> DesktopToastDismissedEventHandler;
typedef ABI::Windows::Foundation::ITypedEventHandler<ABI::Windows::UI::Notifications::ToastNotification *, ABI::Windows::UI::Notifications::ToastFailedEventArgs *> DesktopToastFailedEventHandler;

class ToastEventHandler :
	public Microsoft::WRL::Implements<DesktopToastActivatedEventHandler, DesktopToastDismissedEventHandler, DesktopToastFailedEventHandler>
{
public:
	ToastEventHandler::ToastEventHandler(_In_ HWND hToActivate, _In_ HWND hEdit);
	~ToastEventHandler();

	// DesktopToastActivatedEventHandler 
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification *sender, _In_ IInspectable* args);

	// DesktopToastDismissedEventHandler
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification *sender, _In_ ABI::Windows::UI::Notifications::IToastDismissedEventArgs *e);

	// DesktopToastFailedEventHandler
	IFACEMETHODIMP Invoke(_In_ ABI::Windows::UI::Notifications::IToastNotification *sender, _In_ ABI::Windows::UI::Notifications::IToastFailedEventArgs *e);

	// IUnknown
	IFACEMETHODIMP_(ULONG) AddRef() { return InterlockedIncrement(&_ref); }

	IFACEMETHODIMP_(ULONG) Release() {
		ULONG l = InterlockedDecrement(&_ref);
		if (l == 0) delete this;
		return l;
	}

	IFACEMETHODIMP QueryInterface(_In_ REFIID riid, _COM_Outptr_ void **ppv) {
		if (IsEqualIID(riid, IID_IUnknown))
			*ppv = static_cast<IUnknown*>(static_cast<DesktopToastActivatedEventHandler*>(this));
		else if (IsEqualIID(riid, __uuidof(DesktopToastActivatedEventHandler)))
			*ppv = static_cast<DesktopToastActivatedEventHandler*>(this);
		else if (IsEqualIID(riid, __uuidof(DesktopToastDismissedEventHandler)))
			*ppv = static_cast<DesktopToastDismissedEventHandler*>(this);
		else if (IsEqualIID(riid, __uuidof(DesktopToastFailedEventHandler)))
			*ppv = static_cast<DesktopToastFailedEventHandler*>(this);
		else *ppv = nullptr;

		if (*ppv) {
			reinterpret_cast<IUnknown*>(*ppv)->AddRef();
			return S_OK;
		}

		return E_NOINTERFACE;
	}

private:
	ULONG _ref;
	HWND _hToActivate;
	HWND _hEdit;
};


