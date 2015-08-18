/**
 * @file string.cpp
 * @author Herbert Jones
 *
 * @copyright Copyright 2014 Mediafire
 */
#include "./string.hpp"

#if defined(__MINGW32__)
#       define UNICODE
#       include <windows.h>
#       include <vector>
#else
#       include <locale>
#       include <string>
#       include "boost/locale/encoding_utf.hpp"
#endif

#if defined(__MINGW32__)
uint64_t mf::utils::str_to_uint64(
        const std::string & str,
        int base
    )
{
    // Newer version of MinGW doesn't have _strtoui64, so I've removed that
    // workaround with this one.
    return strtoull(str.c_str(), nullptr, base);
}

// MinGW doesn't have #include <codecvt>
std::string mf::utils::wide_to_bytes(const std::wstring & wide_string)
{
    // Get required size of buffer (in characters!)
    int bytes_required = WideCharToMultiByte(
        CP_UTF8, /* code page */
        0, /* flags */
        wide_string.data(), /* wide char source */
        wide_string.size(), /* wide char size in wchars, not bytes */
        NULL, /* write pointer */
        0, /* write pointer size */
        NULL, /* pointer to char for unrepresentable characters */
        NULL  /* pointer to bool if unrepresentable char used */);

    if ( bytes_required == 0 )
    {
        return std::string();
    }

    auto buffer = std::string(
        std::string::size_type(bytes_required),
        '\0'
        );

    // Convert string
    int chars_written = WideCharToMultiByte(
        CP_UTF8,
        0,
        wide_string.data(),
        wide_string.size(),
        &buffer[0],
        bytes_required,
        NULL,
        NULL);

    assert( chars_written == bytes_required );
    assert( buffer.size() == strlen(buffer.c_str()) );

    if ( chars_written == 0 )
    {
        return std::string();
    }

    // Return the string minus the terminator character as std::string handles
    // that for us
    return buffer;
}

// This function receives a Utf8 std::string and returns a std::wstring
std::wstring mf::utils::bytes_to_wide(const std::string & byte_string)
{
    // Get required size of buffer (in wide characters!)
    int wchars_required = MultiByteToWideChar(
        CP_UTF8,
        0,
        byte_string.data(),
        byte_string.size(),
        NULL,
        0);

    // It is uncertain from the documentation if the return size contains the
    // null terminator.  If the input, passing the size of the input, doesn't
    // have a NULL terminator, then the output shouldn't have one either by
    // existing conventions(input size should equal converted output size, not
    // converted output size plus one) (if -1 were passed to the 4th parameter,
    // then maybe the output size would contain the NULL terminator, but we
    // aren't doing that here)

    if ( wchars_required == 0 )
    {
        return std::wstring();
    }

    auto buffer = std::wstring(
        std::string::size_type(wchars_required),
        L'\0'
        );

    // Convert string
    int wchars_written = MultiByteToWideChar(
        CP_UTF8,
        0,
        byte_string.data(),
        byte_string.size(),
        &buffer[0],
        wchars_required);

    assert( wchars_written == wchars_required );
    assert( buffer.size() == wcslen(buffer.c_str()) );

    if ( wchars_written == 0 )
    {
        return std::wstring();
    }

    return buffer;
}
#else

using boost::locale::conv::utf_to_utf;

uint64_t mf::utils::str_to_uint64(
        const std::string & str,
        int base
    )
{
    return std::stoul(str, 0, base);
}
std::string mf::utils::wide_to_bytes(const std::wstring & str)
{
    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
}

std::wstring mf::utils::bytes_to_wide(const std::string & str)
{
    return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
}
#endif

std::string mf::utils::path_to_utf8(
        const boost::filesystem::path & path
    )
{
#ifdef _WIN32
        return mf::utils::wide_to_bytes(path.wstring());
#else
        return path.string();
#endif
}
