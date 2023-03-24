#pragma once

#include "WinRT.h"

namespace impl
{
    template <typename T>
    constexpr uint8_t hex_to_uint(T const c)
    {
        if (c >= '0' && c <= '9')
        {
            return static_cast<uint8_t>(c - '0');
        }
        else if (c >= 'A' && c <= 'F')
        {
            return static_cast<uint8_t>(10 + c - 'A');
        }
        else if (c >= 'a' && c <= 'f')
        {
            return static_cast<uint8_t>(10 + c - 'a');
        }
        else
        {
            throw std::invalid_argument("Character is not a hexadecimal digit");
        }
    }

    template <typename T>
    constexpr uint8_t hex_to_uint8(T const a, T const b)
    {
        return (hex_to_uint(a) << 4) | hex_to_uint(b);
    }

    constexpr uint16_t uint8_to_uint16(uint8_t a, uint8_t b)
    {
        return (static_cast<uint16_t>(a) << 8) | static_cast<uint16_t>(b);
    }

    constexpr uint32_t uint8_to_uint32(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
    {
        return (static_cast<uint32_t>(uint8_to_uint16(a, b)) << 16) |
            static_cast<uint32_t>(uint8_to_uint16(c, d));
    }

    static winrt::guid parse(TStringView<char> const value)
    {
        if (value.Len() != 36 || value[8] != '-' || value[13] != '-' || value[18] != '-' || value[23] != '-')
        {
            throw std::invalid_argument("value is not a valid GUID string");
        }

        return
        {
            impl::uint8_to_uint32
            (
                impl::hex_to_uint8(value[0], value[1]),
                impl::hex_to_uint8(value[2], value[3]),
                impl::hex_to_uint8(value[4], value[5]),
                impl::hex_to_uint8(value[6], value[7])
            ),
            impl::uint8_to_uint16
            (
                impl::hex_to_uint8(value[9], value[10]),
                impl::hex_to_uint8(value[11], value[12])
            ),
            impl::uint8_to_uint16
            (
                impl::hex_to_uint8(value[14], value[15]),
                impl::hex_to_uint8(value[16], value[17])
            ),
            {
                impl::hex_to_uint8(value[19], value[20]),
                impl::hex_to_uint8(value[21], value[22]),
                impl::hex_to_uint8(value[24], value[25]),
                impl::hex_to_uint8(value[26], value[27]),
                impl::hex_to_uint8(value[28], value[29]),
                impl::hex_to_uint8(value[30], value[31]),
                impl::hex_to_uint8(value[32], value[33]),
                impl::hex_to_uint8(value[34], value[35]),
            }
        };
    }
};

struct BMBTGUID
{
	static inline const winrt::guid DeviceInformationServiceUUID = winrt::Windows::Devices::Bluetooth::BluetoothUuidHelper::FromShortId(0x180A);
	static inline const winrt::guid DeviceInformationService_CameraManufacturer = winrt::Windows::Devices::Bluetooth::BluetoothUuidHelper::FromShortId(0x2A29);
	static inline const winrt::guid DeviceInformationService_CameraModel = winrt::Windows::Devices::Bluetooth::BluetoothUuidHelper::FromShortId(0x2A24);

    static inline const winrt::guid BlackMagicServiceUUID = impl::parse("291D567A-6D75-11E6-8B77-86F30CA893D3");
	static inline const winrt::guid BlackMagicService_OutgoingCameraControl = impl::parse("5DD3465F-1AEE-4299-8493-D2ECA2F8E1BB");
	static inline const winrt::guid BlackMagicService_IncomingCameraControl = impl::parse("B864E140-76A0-416A-BF30-5876504537D9");
};