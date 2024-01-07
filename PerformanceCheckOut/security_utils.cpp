#include "security_utils.h"

PTOKEN_USER getCurrentUserToken() {
    HANDLE hToken;
    CHECK(OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken), NULL, "Failed to open process token");
    DWORD dwBufferSize = 0;
    GetTokenInformation(hToken, TokenUser, NULL, 0, &dwBufferSize);
    PTOKEN_USER pTokenUser = (PTOKEN_USER) new BYTE[dwBufferSize];
    CHECK(GetTokenInformation(hToken, TokenUser, pTokenUser, dwBufferSize, &dwBufferSize), NULL,
        "Failed GetTokenInformation", CLOSE_HANDLES(hToken); delete[] pTokenUser);
    CLOSE_HANDLES(hToken);
    return pTokenUser;
}

DWORD getStringCurrentUserSid(std::string& output) {
    PTOKEN_USER pTokenUser = getCurrentUserToken();
    CHECK(pTokenUser, -1, "Failed to extract the current user token", delete[] pTokenUser);

    LPSTR szUserSid;
    CHECK(ConvertSidToStringSid(pTokenUser->User.Sid, &szUserSid), -1, "Could not obtain the string representation of a SID", delete[] pTokenUser);

    output += std::format("Current user SID: {}\n", szUserSid);

    LocalFree(szUserSid);
    delete[] pTokenUser;
    return 0;
}

PSID getWellKnownSid(WELL_KNOWN_SID_TYPE sidType) {
    DWORD sizeOfPSID = SECURITY_MAX_SID_SIZE;
    PSID pSID = (PSID) new BYTE[sizeOfPSID];
    CHECK(CreateWellKnownSid(sidType, NULL, pSID, &sizeOfPSID),
        NULL, "Error at creating a wellknown SID", delete[] pSID);
    return pSID;
}

DWORD getStringWellKnownSid(WELL_KNOWN_SID_TYPE sidType, LPCSTR label, std::string& output) {
    PSID pSid = getWellKnownSid(sidType);
    CHECK(pSid, -1, "Failed to extract the everyone group sid");

    LPSTR szSid;
    CHECK(ConvertSidToStringSid(pSid, &szSid), -1, "Could not obtain the string representation of a SID", delete[] pSid);

    output += std::format("{} SID: {}\n", label, szSid);

    LocalFree(szSid);
    delete[] pSid;
    return 0;
}

DWORD setFileReadPermissionOnly(LPSTR filename) {
    PTOKEN_USER pTokenUser = getCurrentUserToken();
    CHECK(pTokenUser, -1, "Failed to extract the current user token");

    EXPLICIT_ACCESS eaAllow;
    memset(&eaAllow, 0, sizeof(EXPLICIT_ACCESS));
    eaAllow.grfAccessPermissions = GENERIC_READ;
    eaAllow.grfAccessMode = SET_ACCESS;
    eaAllow.grfInheritance = NO_INHERITANCE;
    eaAllow.Trustee.TrusteeForm = TRUSTEE_IS_SID;
    eaAllow.Trustee.TrusteeType = TRUSTEE_IS_USER;
    eaAllow.Trustee.ptstrName = (LPSTR)pTokenUser->User.Sid;

    PACL pACL;
    CHECK(SetEntriesInAcl(1, &eaAllow, NULL, &pACL) == ERROR_SUCCESS, -1, "Fail to set entries in ACL",
        LocalFree(pACL), delete[] pTokenUser);

    CHECK(SetNamedSecurityInfo(filename, SE_FILE_OBJECT, DACL_SECURITY_INFORMATION | PROTECTED_DACL_SECURITY_INFORMATION, NULL, NULL, pACL, NULL) == ERROR_SUCCESS,
        -1, "Error in function SetNamedSecurityInfo", LocalFree(pACL), delete[] pTokenUser);

    delete[] pTokenUser;
    LocalFree(pACL);

    return 0;
}