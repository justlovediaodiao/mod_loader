// Generated from proxylist.txt; do not edit.
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <stdint.h>

static FARPROC __GetFileVersionInfoA__;
static FARPROC __GetFileVersionInfoByHandle__;
static FARPROC __GetFileVersionInfoExA__;
static FARPROC __GetFileVersionInfoExW__;
static FARPROC __GetFileVersionInfoSizeA__;
static FARPROC __GetFileVersionInfoSizeExA__;
static FARPROC __GetFileVersionInfoSizeExW__;
static FARPROC __GetFileVersionInfoSizeW__;
static FARPROC __GetFileVersionInfoW__;
static FARPROC __VerFindFileA__;
static FARPROC __VerFindFileW__;
static FARPROC __VerInstallFileA__;
static FARPROC __VerInstallFileW__;
static FARPROC __VerLanguageNameA__;
static FARPROC __VerLanguageNameW__;
static FARPROC __VerQueryValueA__;
static FARPROC __VerQueryValueW__;
static FARPROC __Private1__;
static FARPROC __SvchostPushServiceGlobals__;
static FARPROC __WinHttpAddRequestHeaders__;
static FARPROC __WinHttpAutoProxySvcMain__;
static FARPROC __WinHttpCheckPlatform__;
static FARPROC __WinHttpCloseHandle__;
static FARPROC __WinHttpConnect__;
static FARPROC __WinHttpConnectionDeletePolicyEntries__;
static FARPROC __WinHttpConnectionDeleteProxyInfo__;
static FARPROC __WinHttpConnectionFreeNameList__;
static FARPROC __WinHttpConnectionFreeProxyInfo__;
static FARPROC __WinHttpConnectionFreeProxyList__;
static FARPROC __WinHttpConnectionGetNameList__;
static FARPROC __WinHttpConnectionGetProxyInfo__;
static FARPROC __WinHttpConnectionGetProxyList__;
static FARPROC __WinHttpConnectionSetPolicyEntries__;
static FARPROC __WinHttpConnectionSetProxyInfo__;
static FARPROC __WinHttpConnectionUpdateIfIndexTable__;
static FARPROC __WinHttpCrackUrl__;
static FARPROC __WinHttpCreateProxyResolver__;
static FARPROC __WinHttpCreateUrl__;
static FARPROC __WinHttpDetectAutoProxyConfigUrl__;
static FARPROC __WinHttpFreeProxyResult__;
static FARPROC __WinHttpFreeProxyResultEx__;
static FARPROC __WinHttpFreeProxySettings__;
static FARPROC __WinHttpGetDefaultProxyConfiguration__;
static FARPROC __WinHttpGetIEProxyConfigForCurrentUser__;
static FARPROC __WinHttpGetProxyForUrl__;
static FARPROC __WinHttpGetProxyForUrlEx__;
static FARPROC __WinHttpGetProxyForUrlEx2__;
static FARPROC __WinHttpGetProxyForUrlHvsi__;
static FARPROC __WinHttpGetProxyResult__;
static FARPROC __WinHttpGetProxyResultEx__;
static FARPROC __WinHttpGetProxySettingsVersion__;
static FARPROC __WinHttpGetTunnelSocket__;
static FARPROC __WinHttpOpen__;
static FARPROC __WinHttpOpenRequest__;
static FARPROC __WinHttpPacJsWorkerMain__;
static FARPROC __WinHttpProbeConnectivity__;
static FARPROC __WinHttpQueryAuthSchemes__;
static FARPROC __WinHttpQueryDataAvailable__;
static FARPROC __WinHttpQueryHeaders__;
static FARPROC __WinHttpQueryOption__;
static FARPROC __WinHttpReadData__;
static FARPROC __WinHttpReadProxySettings__;
static FARPROC __WinHttpReadProxySettingsHvsi__;
static FARPROC __WinHttpReceiveResponse__;
static FARPROC __WinHttpResetAutoProxy__;
static FARPROC __WinHttpSaveProxyCredentials__;
static FARPROC __WinHttpSendRequest__;
static FARPROC __WinHttpSetCredentials__;
static FARPROC __WinHttpSetDefaultProxyConfiguration__;
static FARPROC __WinHttpSetOption__;
static FARPROC __WinHttpSetStatusCallback__;
static FARPROC __WinHttpSetTimeouts__;
static FARPROC __WinHttpTimeFromSystemTime__;
static FARPROC __WinHttpTimeToSystemTime__;
static FARPROC __WinHttpWebSocketClose__;
static FARPROC __WinHttpWebSocketCompleteUpgrade__;
static FARPROC __WinHttpWebSocketQueryCloseStatus__;
static FARPROC __WinHttpWebSocketReceive__;
static FARPROC __WinHttpWebSocketSend__;
static FARPROC __WinHttpWebSocketShutdown__;
static FARPROC __WinHttpWriteData__;
static FARPROC __WinHttpWriteProxySettings__;
static FARPROC __CreateDXGIFactory__;
static FARPROC __CreateDXGIFactory1__;
static FARPROC __CreateDXGIFactory2__;
static FARPROC __DXGIDeclareAdapterRemovalSupport__;
static FARPROC __DXGIGetDebugInterface1__;

extern "C" void load_functions(HMODULE dll) {
    __GetFileVersionInfoA__ = GetProcAddress(dll, "GetFileVersionInfoA");
    __GetFileVersionInfoByHandle__ = GetProcAddress(dll, "GetFileVersionInfoByHandle");
    __GetFileVersionInfoExA__ = GetProcAddress(dll, "GetFileVersionInfoExA");
    __GetFileVersionInfoExW__ = GetProcAddress(dll, "GetFileVersionInfoExW");
    __GetFileVersionInfoSizeA__ = GetProcAddress(dll, "GetFileVersionInfoSizeA");
    __GetFileVersionInfoSizeExA__ = GetProcAddress(dll, "GetFileVersionInfoSizeExA");
    __GetFileVersionInfoSizeExW__ = GetProcAddress(dll, "GetFileVersionInfoSizeExW");
    __GetFileVersionInfoSizeW__ = GetProcAddress(dll, "GetFileVersionInfoSizeW");
    __GetFileVersionInfoW__ = GetProcAddress(dll, "GetFileVersionInfoW");
    __VerFindFileA__ = GetProcAddress(dll, "VerFindFileA");
    __VerFindFileW__ = GetProcAddress(dll, "VerFindFileW");
    __VerInstallFileA__ = GetProcAddress(dll, "VerInstallFileA");
    __VerInstallFileW__ = GetProcAddress(dll, "VerInstallFileW");
    __VerLanguageNameA__ = GetProcAddress(dll, "VerLanguageNameA");
    __VerLanguageNameW__ = GetProcAddress(dll, "VerLanguageNameW");
    __VerQueryValueA__ = GetProcAddress(dll, "VerQueryValueA");
    __VerQueryValueW__ = GetProcAddress(dll, "VerQueryValueW");
    __Private1__ = GetProcAddress(dll, "Private1");
    __SvchostPushServiceGlobals__ = GetProcAddress(dll, "SvchostPushServiceGlobals");
    __WinHttpAddRequestHeaders__ = GetProcAddress(dll, "WinHttpAddRequestHeaders");
    __WinHttpAutoProxySvcMain__ = GetProcAddress(dll, "WinHttpAutoProxySvcMain");
    __WinHttpCheckPlatform__ = GetProcAddress(dll, "WinHttpCheckPlatform");
    __WinHttpCloseHandle__ = GetProcAddress(dll, "WinHttpCloseHandle");
    __WinHttpConnect__ = GetProcAddress(dll, "WinHttpConnect");
    __WinHttpConnectionDeletePolicyEntries__ = GetProcAddress(dll, "WinHttpConnectionDeletePolicyEntries");
    __WinHttpConnectionDeleteProxyInfo__ = GetProcAddress(dll, "WinHttpConnectionDeleteProxyInfo");
    __WinHttpConnectionFreeNameList__ = GetProcAddress(dll, "WinHttpConnectionFreeNameList");
    __WinHttpConnectionFreeProxyInfo__ = GetProcAddress(dll, "WinHttpConnectionFreeProxyInfo");
    __WinHttpConnectionFreeProxyList__ = GetProcAddress(dll, "WinHttpConnectionFreeProxyList");
    __WinHttpConnectionGetNameList__ = GetProcAddress(dll, "WinHttpConnectionGetNameList");
    __WinHttpConnectionGetProxyInfo__ = GetProcAddress(dll, "WinHttpConnectionGetProxyInfo");
    __WinHttpConnectionGetProxyList__ = GetProcAddress(dll, "WinHttpConnectionGetProxyList");
    __WinHttpConnectionSetPolicyEntries__ = GetProcAddress(dll, "WinHttpConnectionSetPolicyEntries");
    __WinHttpConnectionSetProxyInfo__ = GetProcAddress(dll, "WinHttpConnectionSetProxyInfo");
    __WinHttpConnectionUpdateIfIndexTable__ = GetProcAddress(dll, "WinHttpConnectionUpdateIfIndexTable");
    __WinHttpCrackUrl__ = GetProcAddress(dll, "WinHttpCrackUrl");
    __WinHttpCreateProxyResolver__ = GetProcAddress(dll, "WinHttpCreateProxyResolver");
    __WinHttpCreateUrl__ = GetProcAddress(dll, "WinHttpCreateUrl");
    __WinHttpDetectAutoProxyConfigUrl__ = GetProcAddress(dll, "WinHttpDetectAutoProxyConfigUrl");
    __WinHttpFreeProxyResult__ = GetProcAddress(dll, "WinHttpFreeProxyResult");
    __WinHttpFreeProxyResultEx__ = GetProcAddress(dll, "WinHttpFreeProxyResultEx");
    __WinHttpFreeProxySettings__ = GetProcAddress(dll, "WinHttpFreeProxySettings");
    __WinHttpGetDefaultProxyConfiguration__ = GetProcAddress(dll, "WinHttpGetDefaultProxyConfiguration");
    __WinHttpGetIEProxyConfigForCurrentUser__ = GetProcAddress(dll, "WinHttpGetIEProxyConfigForCurrentUser");
    __WinHttpGetProxyForUrl__ = GetProcAddress(dll, "WinHttpGetProxyForUrl");
    __WinHttpGetProxyForUrlEx__ = GetProcAddress(dll, "WinHttpGetProxyForUrlEx");
    __WinHttpGetProxyForUrlEx2__ = GetProcAddress(dll, "WinHttpGetProxyForUrlEx2");
    __WinHttpGetProxyForUrlHvsi__ = GetProcAddress(dll, "WinHttpGetProxyForUrlHvsi");
    __WinHttpGetProxyResult__ = GetProcAddress(dll, "WinHttpGetProxyResult");
    __WinHttpGetProxyResultEx__ = GetProcAddress(dll, "WinHttpGetProxyResultEx");
    __WinHttpGetProxySettingsVersion__ = GetProcAddress(dll, "WinHttpGetProxySettingsVersion");
    __WinHttpGetTunnelSocket__ = GetProcAddress(dll, "WinHttpGetTunnelSocket");
    __WinHttpOpen__ = GetProcAddress(dll, "WinHttpOpen");
    __WinHttpOpenRequest__ = GetProcAddress(dll, "WinHttpOpenRequest");
    __WinHttpPacJsWorkerMain__ = GetProcAddress(dll, "WinHttpPacJsWorkerMain");
    __WinHttpProbeConnectivity__ = GetProcAddress(dll, "WinHttpProbeConnectivity");
    __WinHttpQueryAuthSchemes__ = GetProcAddress(dll, "WinHttpQueryAuthSchemes");
    __WinHttpQueryDataAvailable__ = GetProcAddress(dll, "WinHttpQueryDataAvailable");
    __WinHttpQueryHeaders__ = GetProcAddress(dll, "WinHttpQueryHeaders");
    __WinHttpQueryOption__ = GetProcAddress(dll, "WinHttpQueryOption");
    __WinHttpReadData__ = GetProcAddress(dll, "WinHttpReadData");
    __WinHttpReadProxySettings__ = GetProcAddress(dll, "WinHttpReadProxySettings");
    __WinHttpReadProxySettingsHvsi__ = GetProcAddress(dll, "WinHttpReadProxySettingsHvsi");
    __WinHttpReceiveResponse__ = GetProcAddress(dll, "WinHttpReceiveResponse");
    __WinHttpResetAutoProxy__ = GetProcAddress(dll, "WinHttpResetAutoProxy");
    __WinHttpSaveProxyCredentials__ = GetProcAddress(dll, "WinHttpSaveProxyCredentials");
    __WinHttpSendRequest__ = GetProcAddress(dll, "WinHttpSendRequest");
    __WinHttpSetCredentials__ = GetProcAddress(dll, "WinHttpSetCredentials");
    __WinHttpSetDefaultProxyConfiguration__ = GetProcAddress(dll, "WinHttpSetDefaultProxyConfiguration");
    __WinHttpSetOption__ = GetProcAddress(dll, "WinHttpSetOption");
    __WinHttpSetStatusCallback__ = GetProcAddress(dll, "WinHttpSetStatusCallback");
    __WinHttpSetTimeouts__ = GetProcAddress(dll, "WinHttpSetTimeouts");
    __WinHttpTimeFromSystemTime__ = GetProcAddress(dll, "WinHttpTimeFromSystemTime");
    __WinHttpTimeToSystemTime__ = GetProcAddress(dll, "WinHttpTimeToSystemTime");
    __WinHttpWebSocketClose__ = GetProcAddress(dll, "WinHttpWebSocketClose");
    __WinHttpWebSocketCompleteUpgrade__ = GetProcAddress(dll, "WinHttpWebSocketCompleteUpgrade");
    __WinHttpWebSocketQueryCloseStatus__ = GetProcAddress(dll, "WinHttpWebSocketQueryCloseStatus");
    __WinHttpWebSocketReceive__ = GetProcAddress(dll, "WinHttpWebSocketReceive");
    __WinHttpWebSocketSend__ = GetProcAddress(dll, "WinHttpWebSocketSend");
    __WinHttpWebSocketShutdown__ = GetProcAddress(dll, "WinHttpWebSocketShutdown");
    __WinHttpWriteData__ = GetProcAddress(dll, "WinHttpWriteData");
    __WinHttpWriteProxySettings__ = GetProcAddress(dll, "WinHttpWriteProxySettings");
    __CreateDXGIFactory__ = GetProcAddress(dll, "CreateDXGIFactory");
    __CreateDXGIFactory1__ = GetProcAddress(dll, "CreateDXGIFactory1");
    __CreateDXGIFactory2__ = GetProcAddress(dll, "CreateDXGIFactory2");
    __DXGIDeclareAdapterRemovalSupport__ = GetProcAddress(dll, "DXGIDeclareAdapterRemovalSupport");
    __DXGIGetDebugInterface1__ = GetProcAddress(dll, "DXGIGetDebugInterface1");
}

extern "C" intptr_t exp_GetFileVersionInfoA() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoA__)(); }
extern "C" intptr_t exp_GetFileVersionInfoByHandle() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoByHandle__)(); }
extern "C" intptr_t exp_GetFileVersionInfoExA() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoExA__)(); }
extern "C" intptr_t exp_GetFileVersionInfoExW() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoExW__)(); }
extern "C" intptr_t exp_GetFileVersionInfoSizeA() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoSizeA__)(); }
extern "C" intptr_t exp_GetFileVersionInfoSizeExA() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoSizeExA__)(); }
extern "C" intptr_t exp_GetFileVersionInfoSizeExW() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoSizeExW__)(); }
extern "C" intptr_t exp_GetFileVersionInfoSizeW() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoSizeW__)(); }
extern "C" intptr_t exp_GetFileVersionInfoW() { return ((intptr_t (WINAPI*)())__GetFileVersionInfoW__)(); }
extern "C" intptr_t exp_VerFindFileA() { return ((intptr_t (WINAPI*)())__VerFindFileA__)(); }
extern "C" intptr_t exp_VerFindFileW() { return ((intptr_t (WINAPI*)())__VerFindFileW__)(); }
extern "C" intptr_t exp_VerInstallFileA() { return ((intptr_t (WINAPI*)())__VerInstallFileA__)(); }
extern "C" intptr_t exp_VerInstallFileW() { return ((intptr_t (WINAPI*)())__VerInstallFileW__)(); }
extern "C" intptr_t exp_VerLanguageNameA() { return ((intptr_t (WINAPI*)())__VerLanguageNameA__)(); }
extern "C" intptr_t exp_VerLanguageNameW() { return ((intptr_t (WINAPI*)())__VerLanguageNameW__)(); }
extern "C" intptr_t exp_VerQueryValueA() { return ((intptr_t (WINAPI*)())__VerQueryValueA__)(); }
extern "C" intptr_t exp_VerQueryValueW() { return ((intptr_t (WINAPI*)())__VerQueryValueW__)(); }
extern "C" intptr_t exp_Private1() { return ((intptr_t (WINAPI*)())__Private1__)(); }
extern "C" intptr_t exp_SvchostPushServiceGlobals() { return ((intptr_t (WINAPI*)())__SvchostPushServiceGlobals__)(); }
extern "C" intptr_t exp_WinHttpAddRequestHeaders() { return ((intptr_t (WINAPI*)())__WinHttpAddRequestHeaders__)(); }
extern "C" intptr_t exp_WinHttpAutoProxySvcMain() { return ((intptr_t (WINAPI*)())__WinHttpAutoProxySvcMain__)(); }
extern "C" intptr_t exp_WinHttpCheckPlatform() { return ((intptr_t (WINAPI*)())__WinHttpCheckPlatform__)(); }
extern "C" intptr_t exp_WinHttpCloseHandle() { return ((intptr_t (WINAPI*)())__WinHttpCloseHandle__)(); }
extern "C" intptr_t exp_WinHttpConnect() { return ((intptr_t (WINAPI*)())__WinHttpConnect__)(); }
extern "C" intptr_t exp_WinHttpConnectionDeletePolicyEntries() { return ((intptr_t (WINAPI*)())__WinHttpConnectionDeletePolicyEntries__)(); }
extern "C" intptr_t exp_WinHttpConnectionDeleteProxyInfo() { return ((intptr_t (WINAPI*)())__WinHttpConnectionDeleteProxyInfo__)(); }
extern "C" intptr_t exp_WinHttpConnectionFreeNameList() { return ((intptr_t (WINAPI*)())__WinHttpConnectionFreeNameList__)(); }
extern "C" intptr_t exp_WinHttpConnectionFreeProxyInfo() { return ((intptr_t (WINAPI*)())__WinHttpConnectionFreeProxyInfo__)(); }
extern "C" intptr_t exp_WinHttpConnectionFreeProxyList() { return ((intptr_t (WINAPI*)())__WinHttpConnectionFreeProxyList__)(); }
extern "C" intptr_t exp_WinHttpConnectionGetNameList() { return ((intptr_t (WINAPI*)())__WinHttpConnectionGetNameList__)(); }
extern "C" intptr_t exp_WinHttpConnectionGetProxyInfo() { return ((intptr_t (WINAPI*)())__WinHttpConnectionGetProxyInfo__)(); }
extern "C" intptr_t exp_WinHttpConnectionGetProxyList() { return ((intptr_t (WINAPI*)())__WinHttpConnectionGetProxyList__)(); }
extern "C" intptr_t exp_WinHttpConnectionSetPolicyEntries() { return ((intptr_t (WINAPI*)())__WinHttpConnectionSetPolicyEntries__)(); }
extern "C" intptr_t exp_WinHttpConnectionSetProxyInfo() { return ((intptr_t (WINAPI*)())__WinHttpConnectionSetProxyInfo__)(); }
extern "C" intptr_t exp_WinHttpConnectionUpdateIfIndexTable() { return ((intptr_t (WINAPI*)())__WinHttpConnectionUpdateIfIndexTable__)(); }
extern "C" intptr_t exp_WinHttpCrackUrl() { return ((intptr_t (WINAPI*)())__WinHttpCrackUrl__)(); }
extern "C" intptr_t exp_WinHttpCreateProxyResolver() { return ((intptr_t (WINAPI*)())__WinHttpCreateProxyResolver__)(); }
extern "C" intptr_t exp_WinHttpCreateUrl() { return ((intptr_t (WINAPI*)())__WinHttpCreateUrl__)(); }
extern "C" intptr_t exp_WinHttpDetectAutoProxyConfigUrl() { return ((intptr_t (WINAPI*)())__WinHttpDetectAutoProxyConfigUrl__)(); }
extern "C" intptr_t exp_WinHttpFreeProxyResult() { return ((intptr_t (WINAPI*)())__WinHttpFreeProxyResult__)(); }
extern "C" intptr_t exp_WinHttpFreeProxyResultEx() { return ((intptr_t (WINAPI*)())__WinHttpFreeProxyResultEx__)(); }
extern "C" intptr_t exp_WinHttpFreeProxySettings() { return ((intptr_t (WINAPI*)())__WinHttpFreeProxySettings__)(); }
extern "C" intptr_t exp_WinHttpGetDefaultProxyConfiguration() { return ((intptr_t (WINAPI*)())__WinHttpGetDefaultProxyConfiguration__)(); }
extern "C" intptr_t exp_WinHttpGetIEProxyConfigForCurrentUser() { return ((intptr_t (WINAPI*)())__WinHttpGetIEProxyConfigForCurrentUser__)(); }
extern "C" intptr_t exp_WinHttpGetProxyForUrl() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyForUrl__)(); }
extern "C" intptr_t exp_WinHttpGetProxyForUrlEx() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyForUrlEx__)(); }
extern "C" intptr_t exp_WinHttpGetProxyForUrlEx2() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyForUrlEx2__)(); }
extern "C" intptr_t exp_WinHttpGetProxyForUrlHvsi() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyForUrlHvsi__)(); }
extern "C" intptr_t exp_WinHttpGetProxyResult() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyResult__)(); }
extern "C" intptr_t exp_WinHttpGetProxyResultEx() { return ((intptr_t (WINAPI*)())__WinHttpGetProxyResultEx__)(); }
extern "C" intptr_t exp_WinHttpGetProxySettingsVersion() { return ((intptr_t (WINAPI*)())__WinHttpGetProxySettingsVersion__)(); }
extern "C" intptr_t exp_WinHttpGetTunnelSocket() { return ((intptr_t (WINAPI*)())__WinHttpGetTunnelSocket__)(); }
extern "C" intptr_t exp_WinHttpOpen() { return ((intptr_t (WINAPI*)())__WinHttpOpen__)(); }
extern "C" intptr_t exp_WinHttpOpenRequest() { return ((intptr_t (WINAPI*)())__WinHttpOpenRequest__)(); }
extern "C" intptr_t exp_WinHttpPacJsWorkerMain() { return ((intptr_t (WINAPI*)())__WinHttpPacJsWorkerMain__)(); }
extern "C" intptr_t exp_WinHttpProbeConnectivity() { return ((intptr_t (WINAPI*)())__WinHttpProbeConnectivity__)(); }
extern "C" intptr_t exp_WinHttpQueryAuthSchemes() { return ((intptr_t (WINAPI*)())__WinHttpQueryAuthSchemes__)(); }
extern "C" intptr_t exp_WinHttpQueryDataAvailable() { return ((intptr_t (WINAPI*)())__WinHttpQueryDataAvailable__)(); }
extern "C" intptr_t exp_WinHttpQueryHeaders() { return ((intptr_t (WINAPI*)())__WinHttpQueryHeaders__)(); }
extern "C" intptr_t exp_WinHttpQueryOption() { return ((intptr_t (WINAPI*)())__WinHttpQueryOption__)(); }
extern "C" intptr_t exp_WinHttpReadData() { return ((intptr_t (WINAPI*)())__WinHttpReadData__)(); }
extern "C" intptr_t exp_WinHttpReadProxySettings() { return ((intptr_t (WINAPI*)())__WinHttpReadProxySettings__)(); }
extern "C" intptr_t exp_WinHttpReadProxySettingsHvsi() { return ((intptr_t (WINAPI*)())__WinHttpReadProxySettingsHvsi__)(); }
extern "C" intptr_t exp_WinHttpReceiveResponse() { return ((intptr_t (WINAPI*)())__WinHttpReceiveResponse__)(); }
extern "C" intptr_t exp_WinHttpResetAutoProxy() { return ((intptr_t (WINAPI*)())__WinHttpResetAutoProxy__)(); }
extern "C" intptr_t exp_WinHttpSaveProxyCredentials() { return ((intptr_t (WINAPI*)())__WinHttpSaveProxyCredentials__)(); }
extern "C" intptr_t exp_WinHttpSendRequest() { return ((intptr_t (WINAPI*)())__WinHttpSendRequest__)(); }
extern "C" intptr_t exp_WinHttpSetCredentials() { return ((intptr_t (WINAPI*)())__WinHttpSetCredentials__)(); }
extern "C" intptr_t exp_WinHttpSetDefaultProxyConfiguration() { return ((intptr_t (WINAPI*)())__WinHttpSetDefaultProxyConfiguration__)(); }
extern "C" intptr_t exp_WinHttpSetOption() { return ((intptr_t (WINAPI*)())__WinHttpSetOption__)(); }
extern "C" intptr_t exp_WinHttpSetStatusCallback() { return ((intptr_t (WINAPI*)())__WinHttpSetStatusCallback__)(); }
extern "C" intptr_t exp_WinHttpSetTimeouts() { return ((intptr_t (WINAPI*)())__WinHttpSetTimeouts__)(); }
extern "C" intptr_t exp_WinHttpTimeFromSystemTime() { return ((intptr_t (WINAPI*)())__WinHttpTimeFromSystemTime__)(); }
extern "C" intptr_t exp_WinHttpTimeToSystemTime() { return ((intptr_t (WINAPI*)())__WinHttpTimeToSystemTime__)(); }
extern "C" intptr_t exp_WinHttpWebSocketClose() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketClose__)(); }
extern "C" intptr_t exp_WinHttpWebSocketCompleteUpgrade() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketCompleteUpgrade__)(); }
extern "C" intptr_t exp_WinHttpWebSocketQueryCloseStatus() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketQueryCloseStatus__)(); }
extern "C" intptr_t exp_WinHttpWebSocketReceive() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketReceive__)(); }
extern "C" intptr_t exp_WinHttpWebSocketSend() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketSend__)(); }
extern "C" intptr_t exp_WinHttpWebSocketShutdown() { return ((intptr_t (WINAPI*)())__WinHttpWebSocketShutdown__)(); }
extern "C" intptr_t exp_WinHttpWriteData() { return ((intptr_t (WINAPI*)())__WinHttpWriteData__)(); }
extern "C" intptr_t exp_WinHttpWriteProxySettings() { return ((intptr_t (WINAPI*)())__WinHttpWriteProxySettings__)(); }
extern "C" intptr_t exp_CreateDXGIFactory() { return ((intptr_t (WINAPI*)())__CreateDXGIFactory__)(); }
extern "C" intptr_t exp_CreateDXGIFactory1() { return ((intptr_t (WINAPI*)())__CreateDXGIFactory1__)(); }
extern "C" intptr_t exp_CreateDXGIFactory2() { return ((intptr_t (WINAPI*)())__CreateDXGIFactory2__)(); }
extern "C" intptr_t exp_DXGIDeclareAdapterRemovalSupport() { return ((intptr_t (WINAPI*)())__DXGIDeclareAdapterRemovalSupport__)(); }
extern "C" intptr_t exp_DXGIGetDebugInterface1() { return ((intptr_t (WINAPI*)())__DXGIGetDebugInterface1__)(); }
