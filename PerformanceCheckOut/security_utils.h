#pragma once
#include "constants.h"

PTOKEN_USER getCurrentUserToken();

DWORD getStringCurrentUserSid(std::string& output);

PSID getWellKnownSid(WELL_KNOWN_SID_TYPE sidType);

DWORD getStringWellKnownSid(WELL_KNOWN_SID_TYPE sidType, LPCSTR label, std::string& output);

DWORD setFileReadPermissionOnly(LPSTR filename);