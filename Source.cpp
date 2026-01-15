#include <windows.h>
#include <winhttp.h>
#include <iostream>
#include <string>
#include <vector>

#pragma comment(lib, "winhttp.lib")
#pragma warning(disable:6385)
#pragma warning(disable:6284)

std::wstring s2ws(const std::string& s) { return std::wstring(s.begin(), s.end());}
std::vector<std::string> ExtractRepoNames(const std::string& json) {
    std::vector<std::string> repos;
    size_t pos = 0;
    while ((pos = json.find("\"name\":", pos)) != std::string::npos) {
        pos = json.find("\"", pos + 7);
        size_t start = pos + 1;
        size_t end = json.find("\"", start);
        repos.push_back(json.substr(start, end - start));
        pos = end;
    }
    return repos;
}

int main() {
    SetConsoleTitle(L"ListerDepotsGit_Basique");
    std::string username;
    std::cout << "ListerDepotsGit_Basique\tv:1.0\t(C)Patrice Waechter-Ebling 2025\nBase sur la documentation Git\n\nEntrez votre nom d'utilisateur GitHub : ";
    std::cin >> username;
    WCHAR buf[80];
    wsprintf(buf, L"ListerDepotsGit_Basique\t%s", username.c_str());
    SetConsoleTitle(buf);
    std::wstring host = L"api.github.com";
    std::wstring path = L"/users/" + s2ws(username) + L"/repos";
    HINTERNET hSession = WinHttpOpen(L"Win32 GitHub Client/1.0",WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,WINHTTP_NO_PROXY_NAME,WINHTTP_NO_PROXY_BYPASS, 0);
    if (!hSession) {std::cout << "Erreur WinHttpOpen\n";return 1;}
    HINTERNET hConnect = WinHttpConnect(hSession, host.c_str(), INTERNET_DEFAULT_HTTPS_PORT, 0);
    if (!hConnect) {
        std::cout << "Erreur WinHttpConnect\n";
        WinHttpCloseHandle(hSession);
        return 1;
    }
    HINTERNET hRequest = WinHttpOpenRequest(hConnect, L"GET", path.c_str(),NULL, WINHTTP_NO_REFERER,WINHTTP_DEFAULT_ACCEPT_TYPES,WINHTTP_FLAG_SECURE);
    if (!hRequest) {
        std::cout << "Erreur WinHttpOpenRequest\n";
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }
    BOOL sent = WinHttpSendRequest(hRequest,L"User-Agent: Win32App\r\n",-1L,WINHTTP_NO_REQUEST_DATA,0,0,0);
    if (!sent) {
        std::cout << "Erreur WinHttpSendRequest\n";
        WinHttpCloseHandle(hRequest);
        WinHttpCloseHandle(hConnect);
        WinHttpCloseHandle(hSession);
        return 1;
    }
    WinHttpReceiveResponse(hRequest, NULL);
    DWORD size = 0;
    std::string response;
    do {
        WinHttpQueryDataAvailable(hRequest, &size);
        if (size == 0) break;
        std::vector<char> buffer(size + 1);
        DWORD downloaded = 0;
        WinHttpReadData(hRequest, buffer.data(), size, &downloaded);
        buffer[downloaded] = 0;
        response += buffer.data();
    } while (size > 0);
    WinHttpCloseHandle(hRequest);
    WinHttpCloseHandle(hConnect);
    WinHttpCloseHandle(hSession);
    auto repos = ExtractRepoNames(response);
    std::cout << "\n=== Repositories GitHub de " << username << " ===\n";
    for (const auto& r : repos) {std::cout << " - " << r << "\n";}
    if (repos.empty()) {std::cout << "Aucun repository trouvé ou utilisateur inexistant.\n";}
    return 0;
}