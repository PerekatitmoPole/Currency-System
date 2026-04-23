#include "providers/HttpClient.hpp"

#include "common/Exceptions.hpp"

#include <windows.h>
#include <winhttp.h>

#include <memory>
#include <string>
#include <type_traits>

namespace currency::providers {

namespace {

struct HandleCloser {
    void operator()(HINTERNET handle) const {
        if (handle != nullptr) {
            WinHttpCloseHandle(handle);
        }
    }
};

using UniqueHandle = std::unique_ptr<std::remove_pointer_t<HINTERNET>, HandleCloser>;

std::wstring utf8ToWide(const std::string& value) {
    if (value.empty()) {
        return {};
    }

    const auto size = MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), nullptr, 0);
    std::wstring result(size, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, value.c_str(), static_cast<int>(value.size()), result.data(), size);
    return result;
}

std::string formatWindowsError(const std::string& operation) {
    return operation + " failed with code " + std::to_string(GetLastError());
}

}

std::string HttpClient::get(const std::string& url, const std::chrono::milliseconds timeout) const {
    const auto wideUrl = utf8ToWide(url);

    URL_COMPONENTS components{};
    components.dwStructSize = sizeof(components);
    components.dwSchemeLength = static_cast<DWORD>(-1);
    components.dwHostNameLength = static_cast<DWORD>(-1);
    components.dwUrlPathLength = static_cast<DWORD>(-1);
    components.dwExtraInfoLength = static_cast<DWORD>(-1);

    if (!WinHttpCrackUrl(wideUrl.c_str(), 0, 0, &components)) {
        throw common::ExternalApiError(formatWindowsError("Unable to parse URL"));
    }

    const std::wstring host(components.lpszHostName, components.dwHostNameLength);
    std::wstring target(components.lpszUrlPath, components.dwUrlPathLength);
    if (components.dwExtraInfoLength > 0) {
        target.append(components.lpszExtraInfo, components.dwExtraInfoLength);
    }

    const auto timeoutValue = static_cast<int>(timeout.count());
    UniqueHandle session(WinHttpOpen(L"CurrencySystem/2.0", WINHTTP_ACCESS_TYPE_AUTOMATIC_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0));
    if (session == nullptr) {
        throw common::ExternalApiError(formatWindowsError("Unable to open WinHTTP session"));
    }

    WinHttpSetTimeouts(session.get(), timeoutValue, timeoutValue, timeoutValue, timeoutValue);

    UniqueHandle connection(WinHttpConnect(session.get(), host.c_str(), components.nPort, 0));
    if (connection == nullptr) {
        throw common::ExternalApiError(formatWindowsError("Unable to open WinHTTP connection"));
    }

    const auto flags = components.nScheme == INTERNET_SCHEME_HTTPS ? WINHTTP_FLAG_SECURE : 0;
    UniqueHandle request(WinHttpOpenRequest(
        connection.get(),
        L"GET",
        target.c_str(),
        nullptr,
        WINHTTP_NO_REFERER,
        WINHTTP_DEFAULT_ACCEPT_TYPES,
        flags));
    if (request == nullptr) {
        throw common::ExternalApiError(formatWindowsError("Unable to open WinHTTP request"));
    }

    if (!WinHttpSendRequest(request.get(), WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0)) {
        throw common::ExternalApiError(formatWindowsError("Unable to send HTTP request"));
    }

    if (!WinHttpReceiveResponse(request.get(), nullptr)) {
        throw common::ExternalApiError(formatWindowsError("Unable to receive HTTP response"));
    }

    DWORD statusCode = 0;
    DWORD statusCodeSize = sizeof(statusCode);
    if (!WinHttpQueryHeaders(request.get(), WINHTTP_QUERY_STATUS_CODE | WINHTTP_QUERY_FLAG_NUMBER, WINHTTP_HEADER_NAME_BY_INDEX, &statusCode, &statusCodeSize, WINHTTP_NO_HEADER_INDEX)) {
        throw common::ExternalApiError(formatWindowsError("Unable to read HTTP status code"));
    }

    if (statusCode >= 400) {
        throw common::ExternalApiError("HTTP provider request failed with status " + std::to_string(statusCode));
    }

    std::string body;
    for (;;) {
        DWORD availableBytes = 0;
        if (!WinHttpQueryDataAvailable(request.get(), &availableBytes)) {
            throw common::ExternalApiError(formatWindowsError("Unable to inspect HTTP response body"));
        }
        if (availableBytes == 0) {
            break;
        }

        std::string chunk(availableBytes, '\0');
        DWORD bytesRead = 0;
        if (!WinHttpReadData(request.get(), chunk.data(), availableBytes, &bytesRead)) {
            throw common::ExternalApiError(formatWindowsError("Unable to read HTTP response body"));
        }

        chunk.resize(bytesRead);
        body.append(chunk);
    }

    return body;
}

}
