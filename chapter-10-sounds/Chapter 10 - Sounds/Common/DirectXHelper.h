#pragma once

#include <ppltasks.h>	// Para create_task

namespace DX
{
	inline void ThrowIfFailed(HRESULT hr)
	{
		if (FAILED(hr))
		{
			// Configure un punto de interrupción en esta línea para detectar errores de la API Win32.
			throw Platform::Exception::CreateException(hr);
		}
	}

	// Función que lee desde un archivo binario de forma asincrónica.
	inline Concurrency::task<std::vector<byte>> ReadDataAsync(const std::wstring& filename)
	{
		using namespace Windows::Storage;
		using namespace Concurrency;

		auto folder = Windows::ApplicationModel::Package::Current->InstalledLocation;

		return create_task(folder->GetFileAsync(Platform::StringReference(filename.c_str()))).then([](StorageFile^ file)
		{
			return FileIO::ReadBufferAsync(file);
		}).then([](Streams::IBuffer^ fileBuffer) -> std::vector<byte>
		{
			std::vector<byte> returnBuffer;
			returnBuffer.resize(fileBuffer->Length);
			Streams::DataReader::FromBuffer(fileBuffer)->ReadBytes(Platform::ArrayReference<byte>(returnBuffer.data(), fileBuffer->Length));
			return returnBuffer;
		});
	}

	// Convierte una longitud expresada en píxeles independientes del dispositivo (PID) en una longitud expresada en píxeles físicos.
	inline float ConvertDipsToPixels(float dips, float dpi)
	{
		static const float dipsPerInch = 96.0f;
		return floorf(dips * dpi / dipsPerInch + 0.5f); // Redondear al entero más próximo.
	}

	// Asignar un nombre al objeto para facilitar la depuración.
#if defined(_DEBUG)
	inline void SetName(ID3D12Object* pObject, LPCWSTR name)
	{
		pObject->SetName(name);
	}
#else
	inline void SetName(ID3D12Object*, LPCWSTR)
	{
	}
#endif
}

// Nombrar función del asistente para ComPtr<T>.
// Asigna el nombre de la variable como nombre del objeto.
#define NAME_D3D12_OBJECT(x) DX::SetName(x.Get(), L#x)
