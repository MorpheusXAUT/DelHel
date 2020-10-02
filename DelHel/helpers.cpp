#include "pch.h"

#include "helpers.h"

std::string FetchLatestVersion()
{
	std::ostringstream agent;
	agent << PLUGIN_NAME << '/' << PLUGIN_VERSION;

	HINTERNET init = InternetOpen(agent.str().c_str(), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
	if (!init) {
		throw error{ "Connection failed. Error: " + std::to_string(GetLastError()) };
	}

	HINTERNET open = InternetOpenUrl(init, PLUGIN_LATEST_VERSION_URL, NULL, 0, INTERNET_FLAG_PRAGMA_NOCACHE | INTERNET_FLAG_RELOAD, 0);
	if (!open) {
		InternetCloseHandle(init);
		throw error{ "Failed to load URL. Error: " + std::to_string(GetLastError()) };
	}

	char data[256];
	DWORD read;
	std::string version;

	while (InternetReadFile(open, data, 256, &read) && read) {
		version.append(data, read);
	}

	InternetCloseHandle(init);
	InternetCloseHandle(open);

	return version;
}