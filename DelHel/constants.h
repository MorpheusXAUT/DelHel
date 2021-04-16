#pragma once

#include <regex>

#define PLUGIN_NAME    "DelHel"
#define PLUGIN_VERSION "0.0.5"
#define PLUGIN_AUTHOR  "Nick Mueller"
#define PLUGIN_LICENSE "(c) 2020, MIT License"
#define PLUGIN_LATEST_VERSION_URL "https://raw.githubusercontent.com/MorpheusXAUT/DelHel/master/version.txt"
#define PLUGIN_LATEST_DOWNLOAD_URL "https://github.com/MorpheusXAUT/DelHel/releases/latest"

const char SETTINGS_DELIMITER = '|';

const int TAG_ITEM_FP_VALIDATION = 1;

const int TAG_FUNC_VALIDATION_MENU = 100;
const int TAG_FUNC_PROCESS_FP = 101;
const int TAG_FUNC_PROCESS_FP_NAP = 102;
const int TAG_FUNC_PROCESS_FP_NON_NAP = 103;

const COLORREF TAG_COLOR_NONE = 0;
const COLORREF TAG_COLOR_RED = RGB(200, 0, 0);
const COLORREF TAG_COLOR_ORANGE = RGB(255, 165, 0);
const COLORREF TAG_COLOR_GREEN = RGB(0, 200, 0);

const std::regex REGEX_SPEED_LEVEL_GROUP = std::regex("(((?:N|K)\\d{4})((?:F\\d{3})|(?:S\\d{4})|(?:A\\d{3})|(?:M\\d{4})|(?:VFR)))", std::regex_constants::ECMAScript);

const int AUTO_ASSIGN_MIN_FL = 5000;
const int VFR_TRAFFIC_PATTERN_ALTITUDE = 1000;