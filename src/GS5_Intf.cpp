#include "GS5_Intf.h"

#include <assert.h>
#include <stdexcept>
#include <string.h>
#include <string>

#ifdef _MSC_VER
#include <Windows.h>
#else
#include <dlfcn.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#endif

namespace gs {

#define MIN_API_INDEX 2
#define MAX_API_INDEX 162

static void *apis[MAX_API_INDEX + 1];

//: image base of gsCore
static void *s_core = nullptr;
static bool s_finished = false;

void sdk_finish() {
    if (s_finished)
        return;
    s_finished = true;

    if (s_core) {
#if defined(_WINDOWS_) || defined(_WIN_)
        FreeLibrary((HMODULE)s_core);
#elif defined(_MAC_) || defined(_LINUX_)
        dlclose(s_core);
#endif
        s_core = nullptr;
    }
}

//Resolve all gsCore apis dynamically, must be called before any other apis
static void resolveAPIs(void) {
    static bool inited = false;
    if (inited)
        return;

    memset(apis, 0, sizeof(apis));

#if defined(_WINDOWS_) || defined(_WIN_)
    HMODULE h = nullptr;
    const char *core_dlls[2] = {"gsCore.dll", "gsCore-6.dll"};
    for (int i = 0; !h && i < sizeof(core_dlls) / sizeof(core_dlls[0]); i++) {
        const char* core_dll = core_dlls[i];
        h = LoadLibraryA(core_dll);
        if (!h) {
            //gsCore.dll is not in the dll search path.
            //try searching environment variable "GS_SDK_BIN"
            char *p = getenv("GS_SDK_BIN");
            if (p) {
                char buf[MAX_PATH];
                strncpy(buf, p, sizeof(buf)-1);
                int i = strlen(buf);
                if (i < sizeof(buf) - 1) {
                    if (buf[i - 1] != '/' && buf[i - 1] != '\\') {
                        buf[i] = '\\';
                        buf[i + 1] = 0;
                    }
                }
                //try GS_SDK_BIN/gsCore.dll
                std::string dll_path = std::string(buf) + core_dll;
                h = LoadLibraryA(dll_path.c_str());
                if (!h) {
                    //try GS_SDK_BIN/[win32|win64]/gsCore.dll
                    dll_path = std::string(buf) + (sizeof(p) == 4 ? "win32\\" : "win64\\") + core_dll;
                    h = LoadLibraryA(dll_path.c_str());
                }
            }
        }
    }
    if (h) {
        for (int i = MIN_API_INDEX; i <= MAX_API_INDEX; i++) {
            apis[i] = GetProcAddress(h, (const char *)i);
        }
    }
    s_core = h;
#elif defined(_MAC_)

    void *h = nullptr;
    //Mac wrapped core, if loaded, will set its image base in environment
    char *p = getenv("GS_CORE_BASE");
    if (p) {
        //Already loaded in memory p;
        sscanf(p, "%p", &h);
        printf("GS_CORE_BASE = [%p]", h);
    } else {
        const char *cores[] = {"libgsCore.dylib", "libgsCore.6.dylib"};
        for(int i = 0; !h && i < sizeof(cores)/sizeof(cores[0]); i++){
            const char *core = cores[i];
            char buf[4096];
            //try searching environment variable "GS_SDK_BIN"
            char *p = getenv("GS_SDK_BIN");
            if (p) {
                strncpy(buf, p, sizeof(buf)-1);
                size_t i = strlen(buf);
                if (i < sizeof(buf) - 1) {
                    if (buf[i - 1] != '/') {
                        buf[i] = '/';
                        buf[i + 1] = 0;
                    }
                }
                //try GS_SDK_BIN/libgsCore.dylib
                std::string dll_path = std::string(buf) + core;
                printf("try loading [%s]...", dll_path.c_str());
                h = dlopen(dll_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
            }

            if(!h){
                //Load it from the directory side by side with *this* module (lib or exe)

                dl_info di;
                if (dladdr(&apis, &di)) {
                    std::string this_module = (const char *)realpath(di.dli_fname, buf);
                    size_t i = this_module.find_last_of('/');
                    buf[i + 1] = 0;
                    strncat(buf, core, sizeof(buf)-1);
                } else {
                    strncpy(buf, core, sizeof(buf)-1);
                }

                printf("Loading Core lib [%s]...\n", buf);
                h = dlopen(buf, RTLD_LAZY | RTLD_LOCAL);
            }
        }

        if (h == nullptr) {
            printf("ERROR: cannot load core lib");
            exit(-1);
        }
    }

    s_core = h;
#elif defined(_LINUX_)
    const char *core = "libgsCore.so";
    void *h = dlopen(core, RTLD_LAZY | RTLD_LOCAL);
    if (!h) {
        char buf[4096];
        //try searching environment variable "GS_SDK_BIN"
        char *p = getenv("GS_SDK_BIN");
        if (p) {
            strncpy(buf, p, sizeof(buf)-1);
            size_t i = strlen(buf);
            if (i < sizeof(buf) - 1) {
                if (buf[i - 1] != '/') {
                    buf[i] = '/';
                    buf[i + 1] = 0;
                }
            }
            //try GS_SDK_BIN/libgsCore.so
            std::string dll_path = std::string(buf) + core;
            h = dlopen(dll_path.c_str(), RTLD_LAZY | RTLD_LOCAL);

            if (!h) {
                //try GS_SDK_BIN/[win32|win64]/gsCore.dll
                dll_path = std::string(buf) + (sizeof(p) == 4 ? "linux32/" : "linux64/") + core;
                h = dlopen(dll_path.c_str(), RTLD_LAZY | RTLD_LOCAL);
            }
        }
        //sdk core cannot be loaded in standard way, try searching it side by side
        //with current executable

        if (!h) {
            Dl_info di;
            if (dladdr(&apis, &di)) {
                std::string this_module = (const char *)realpath(di.dli_fname, buf);
                size_t i = this_module.find_last_of('/');
                buf[i + 1] = 0;
                strncat(buf, core, sizeof(buf)-1);
            } else {
                strncpy(buf, core, sizeof(buf)-1);
            }

            printf("Loading Core lib [%s]...\n", buf);
            h = dlopen(buf, RTLD_LAZY | RTLD_LOCAL);
        }
    }
    s_core = h;
#else
#error("Either _WIN_, _LINUX_ or _MAC_ must be defined to build SoftwareShield SDK-C!")
#endif
    inited = true;

    if (s_core == nullptr) {
        fprintf(stderr, "gsCore cannot be loaded!\n");
        exit(-1);
    }
}

#ifdef _WINDOWS_
#define RESOLVE_API(ord, apiName) \
    {                             \
        resolveAPIs();            \
        assert(apis[ord]);        \
    }
#else
//Unix..
void resolveApi(int ord, const char *apiName) {
    if (nullptr == s_core)
        resolveAPIs();

    if (nullptr == apis[ord]) {
        apis[ord] = dlsym(s_core, apiName);
    }
    assert(apis[ord]);
}

#define RESOLVE_API(ord, apiName) \
    { resolveApi(ord, #apiName); }
#endif

#define FUNC_CALL(...) \
    (__VA_ARGS__);     \
    }

#define BIND_FUNC(ord, retType, apiName, ...)       \
    retType apiName(__VA_ARGS__) {                  \
        RESOLVE_API(ord, apiName);                  \
        typedef retType(WINAPI *Fapi)(__VA_ARGS__); \
        return (*(Fapi)apis[ord])FUNC_CALL

#define BIND_PROC(ord, apiName, ...)              \
    void apiName(__VA_ARGS__) {                   \
        RESOLVE_API(ord, apiName);                \
        typedef void(WINAPI * Fapi)(__VA_ARGS__); \
        (*(Fapi)apis[ord]) FUNC_CALL

#define BIND_PROC0(ord, apiName)       \
    void apiName() {                   \
        RESOLVE_API(ord, apiName);     \
        typedef void(WINAPI * Fapi)(); \
        (*(Fapi)apis[ord])();          \
    }

#define BIND_FUNC0(ord, retType, apiName) \
    retType apiName() {                   \
        RESOLVE_API(ord, apiName);        \
        typedef retType(WINAPI *Fapi)();  \
        return (*(Fapi)apis[ord])();      \
    }

BIND_FUNC(3, int, gsInit, const char *productId, const char *origLic, const char *password, void *reserved)
(productId, origLic, password, reserved);

BIND_FUNC(103, int, gsInitEx, const char *productId, const unsigned char *origLicData, int licSize, const char *password, void *reserved)
(productId, origLicData, licSize, password, reserved);

int gsInit(const char *productId, const unsigned char *origLicData, int licSize, const char *password, void *reserved) {
    return gsInitEx(productId, origLicData, licSize, password, reserved);
}

BIND_FUNC0(4, int, gsCleanUp);

BIND_FUNC0(2, const char *, gsGetVersion);

BIND_PROC(5, gsCloseHandle, gs_handle_t handle)
(handle);

BIND_PROC0(6, gsFlush);
BIND_FUNC0(7, const char *, gsGetLastErrorMessage);
BIND_FUNC0(8, int, gsGetLastErrorCode);

BIND_PROC(104, gsSetLastErrorInfo, int errCode, const char *errMsg)
(errCode, errMsg);

BIND_FUNC0(9, int, gsGetBuildId);
BIND_FUNC0(84, const char *, gsGetProductName);
BIND_FUNC0(85, const char *, gsGetProductId);

//Entity
BIND_FUNC0(10, int, gsGetEntityCount);

BIND_FUNC(11, TEntityHandle, gsOpenEntityByIndex, int index)
(index);

BIND_FUNC(12, TEntityHandle, gsOpenEntityById, entity_id_t entityId)
(entityId);

BIND_FUNC(13, unsigned int, gsGetEntityAttributes, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(14, entity_id_t, gsGetEntityId, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(15, const char *, gsGetEntityName, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(16, const char *, gsGetEntityDescription, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(20, bool, gsBeginAccessEntity, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(21, bool, gsEndAccessEntity, TEntityHandle hEntity)
(hEntity);

//License
BIND_FUNC(25, int, gsGetLicenseCount, TEntityHandle hEntity)
(hEntity);

BIND_FUNC(26, TLicenseHandle, gsOpenLicenseByIndex, TEntityHandle hEntity, int index)
(hEntity, index);

BIND_FUNC(27, TLicenseHandle, gsOpenLicenseById, TEntityHandle hEntity, license_id_t licenseId)
(hEntity, licenseId);

/*
* Inspect the license model's status
*/
BIND_FUNC(28, license_id_t, gsGetLicenseId, TLicenseHandle hLicense)
(hLicense);

BIND_FUNC(22, const char *, gsGetLicenseName, TLicenseHandle hLicense)
(hLicense);

BIND_FUNC(23, const char *, gsGetLicenseDescription, TLicenseHandle hLicense)
(hLicense);

BIND_FUNC(24, unsigned char, _gsGetLicenseStatus, TLicenseHandle hLicense)
(hLicense);

TLicenseStatus gsGetLicenseStatus(TLicenseHandle hLicense){
    return (TLicenseStatus)_gsGetLicenseStatus(hLicense);
}

BIND_FUNC(34, bool, gsIsLicenseValid, TLicenseHandle hLicense)
(hLicense);

BIND_FUNC(48, TEntityHandle, gsGetLicensedEntity, TLicenseHandle hLic)
(hLic);

/*
* Inspect the license model's parameters
*/
/// Get total number of parameters in a license.
BIND_FUNC(29, int, gsGetLicenseParamCount, TLicenseHandle hLicense)
(hLicense);

/// Get the index'th parameter info handle
BIND_FUNC(30, TVarHandle, gsGetLicenseParamByIndex, TLicenseHandle hLicense, int index)
(hLicense, index);

BIND_FUNC(31, TVarHandle, gsGetLicenseParamByName, TLicenseHandle hLicense, const char *name)
(hLicense, name);

/**
 * Inspect license model's actions
 */
BIND_FUNC(32, int, gsGetActionInfoCount, TLicenseHandle hLicense)
(hLicense);

BIND_FUNC(33, const char *, gsGetActionInfoByIndex, TLicenseHandle hLicense, int index, action_id_t *actionId)
(hLicense, index, actionId);

/**
 *	Inspect an action
*/
BIND_FUNC(38, const char *, gsGetActionName, TActionHandle hAct)
(hAct);

BIND_FUNC(39, action_id_t, gsGetActionId, TActionHandle hAct)
(hAct);

BIND_FUNC(40, const char *, gsGetActionDescription, TActionHandle hAct)
(hAct);

BIND_FUNC(41, const char *, gsGetActionString, TActionHandle hAct)
(hAct);

/**
 * Inspect action's parameters
 */
BIND_FUNC(42, int, gsGetActionParamCount, TActionHandle hAct)
(hAct);

BIND_FUNC(43, TVarHandle, gsGetActionParamByName, TActionHandle hAct, const char *paramName)
(hAct, paramName);

BIND_FUNC(44, TVarHandle, gsGetActionParamByIndex, TActionHandle hAct, int index)
(hAct, index);

//Variables
BIND_FUNC(50, TVarHandle, gsAddVariable, const char *varName, TVarType varType, int attr, const char *initValStr)
(varName, varType, attr, initValStr);

BIND_FUNC(51, bool, gsRemoveVariable, const char *varName)
(varName);

BIND_FUNC(52, TVarHandle, gsGetVariable, const char *varName)
(varName);

BIND_FUNC(53, const char *, gsGetVariableName, TVarHandle hVar)
(hVar);

BIND_FUNC(54, TVarType, gsGetVariableType, TVarHandle hVar)
(hVar);

BIND_FUNC(55, const char *, gsVariableTypeToString, var_type_t paramType)
(paramType);

BIND_FUNC(56, int, gsGetVariableAttr, TVarHandle hVar)
(hVar);

BIND_FUNC(65, const char *, gsVariableAttrToString, int permit, char *buf, int bufSize)
(permit, buf, bufSize);

BIND_FUNC(66, int, gsVariableAttrFromString, const char *permitStr)
(permitStr);

//Value Get/Set
BIND_FUNC(57, const char *, gsGetVariableValueAsString, TVarHandle hVar)
(hVar);

BIND_FUNC(58, bool, gsSetVariableValueFromString, TVarHandle hVar, const char *valstr)
(hVar, valstr);

BIND_FUNC(59, bool, gsGetVariableValueAsInt, TVarHandle hVar, int &val)
(hVar, val);

BIND_FUNC(60, bool, gsSetVariableValueFromInt, TVarHandle hVar, int val)
(hVar, val);

BIND_FUNC(61, bool, gsGetVariableValueAsInt64, TVarHandle hVar, int64_t &val)
(hVar, val);

BIND_FUNC(62, bool, gsSetVariableValueFromInt64, TVarHandle hVar, int64_t val)
(hVar, val);

BIND_FUNC(63, bool, gsGetVariableValueAsFloat, TVarHandle hVar, float &val)
(hVar, val);

BIND_FUNC(64, bool, gsSetVariableValueFromFloat, TVarHandle hVar, float val)
(hVar, val);

BIND_FUNC(78, bool, gsGetVariableValueAsDouble, TVarHandle hVar, double &val)
(hVar, val);

BIND_FUNC(79, bool, gsSetVariableValueFromDouble, TVarHandle hVar, double val)
(hVar, val);

BIND_FUNC(68, bool, gsGetVariableValueAsTime, TVarHandle hVar, time_t &val)
(hVar, val);

BIND_FUNC(69, bool, gsSetVariableValueFromTime, TVarHandle hVar, time_t val)
(hVar, val);

//Request
BIND_FUNC0(36, TRequestHandle, gsCreateRequest)

BIND_FUNC(37, TActionHandle, gsAddRequestAction, TRequestHandle hReq, action_id_t actId, TLicenseHandle hLic)
(hReq, actId, hLic);

BIND_FUNC(47, TActionHandle, gsAddRequestActionEx, TRequestHandle hReq, action_id_t actId, const char *entityId, const char *licenseId)
(hReq, actId, entityId, licenseId);

BIND_FUNC(45, const char *, gsGetRequestCode, TRequestHandle hReq)
(hReq);

BIND_FUNC(46, bool, gsApplyLicenseCode, const char *licenseCode)
(licenseCode);

//---------- Time Engine Service ------------
BIND_PROC0(70, gsTurnOnInternalTimer);
BIND_PROC0(71, gsTurnOffInternalTimer);
BIND_FUNC0(72, bool, gsIsInternalTimerActive);
BIND_PROC0(73, gsTickFromExternalTimer);
BIND_PROC0(74, gsPauseTimeEngine);
BIND_PROC0(75, gsResumeTimeEngine);
BIND_FUNC0(76, bool, gsIsTimeEngineActive);

//Monitor
BIND_FUNC(90, TMonitorHandle, gsCreateMonitorEx, gs5_monitor_callback cbMonitor, void *usrData, const char *monitorName)
(cbMonitor, usrData, monitorName);

BIND_FUNC(86, int, gsGetEventId, TEventHandle hEvent)
(hEvent);

BIND_FUNC(87, TEventType, gsGetEventType, TEventHandle hEvent)
(hEvent);

BIND_FUNC(88, TEventSourceHandle, gsGetEventSource, TEventHandle hEvent)
(hEvent);

//HTML
BIND_FUNC(80, bool, gsRenderHTML, const char *url, const char *title, int width, int height)
(url, title, width, height);

BIND_FUNC(83, bool, gsRenderHTMLEx, const char *url, const char *title, int width, int height, bool resizable, bool exitAppWhenUIClosed, bool cleanUpAfterRendering)
(url, title, width, height, resizable, exitAppWhenUIClosed, cleanUpAfterRendering);

BIND_FUNC0(81, bool, gsRunInWrappedMode);

BIND_FUNC(82, bool, gsRunInsideVM, vm_mask_t vmask)
(vmask);

BIND_FUNC0(91, bool, gsIsDebugVersion);

BIND_PROC(92, gsTrace, const char *msg)
(msg);

BIND_FUNC(67, bool, gsIsVariableValid, TVarHandle hVar)
(hVar);

//Application Control
BIND_PROC(93, gsExitApp, int rc)
(rc);

BIND_PROC(94, gsTerminateApp, int rc)
(rc);

BIND_PROC0(95, gsPlayApp);
BIND_PROC0(96, gsRestartApp);
BIND_FUNC0(102, bool, gsIsRestartedApp);

BIND_FUNC0(97, const char *, gsGetAppRootPath);
BIND_FUNC0(98, const char *, gsGetAppCommandLine);
BIND_FUNC0(101, const char *, gsGetAppMainExe);

//Session Variables
BIND_PROC(99, gsSetAppVar, const char *name, const char *val)
(name, val);

BIND_FUNC(100, const char *, gsGetAppVar, const char *name)
(name);

//Custom LM
BIND_FUNC(105, TLicenseHandle, gsCreateCustomLicense, const char *licId, const char *licName, const char *description, void *usrData,
          lm_isValid_callback cbIsValid, lm_startAccess_callback cbStartAccess,
          lm_finishAccess_callback cbFinishAccess, lm_onAction_callback cbOnAction,
          lm_destroy_callback cbDestroy)
(licId, licName, description, usrData, cbIsValid, cbStartAccess, cbFinishAccess, cbOnAction, cbDestroy);

BIND_FUNC(106, bool, gsBindLicense, TEntityHandle hEntity, TLicenseHandle hLic)
(hEntity, hLic);

BIND_FUNC(107, TLicenseHandle, gsCreateLicense, const char *licId)
(licId);

BIND_PROC(108, gsRegisterCustomLicense, const char *licId, lm_create_callback createLM, void *usrData)
(licId, createLM, usrData);

BIND_PROC(109, gsAddLicenseParamStr, TLicenseHandle hLic, const char *paramName, const char *initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(110, gsAddLicenseParamInt, TLicenseHandle hLic, const char *paramName, int initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(111, gsAddLicenseParamInt64, TLicenseHandle hLic, const char *paramName, int64_t initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(112, gsAddLicenseParamBool, TLicenseHandle hLic, const char *paramName, bool initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(113, gsAddLicenseParamFloat, TLicenseHandle hLic, const char *paramName, float initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(114, gsAddLicenseParamTime, TLicenseHandle hLic, const char *paramName, time_t initValue, int permission)
(hLic, paramName, initValue, permission);

BIND_PROC(115, gsAddLicenseParamDouble, TLicenseHandle hLic, const char *paramName, double initValue, int permission)
(hLic, paramName, initValue, permission);

//-------- Game Execution Context -----------
BIND_FUNC0(116, bool, gsIsFirstPass);
BIND_FUNC0(117, bool, gsIsGamePass);
BIND_FUNC0(118, bool, gsIsLastPass);
BIND_FUNC0(119, bool, gsIsFirstGameExe);
BIND_FUNC0(120, bool, gsIsLastGameExe);
BIND_FUNC0(121, bool, gsIsMainThread);

BIND_FUNC0(122, int, gsGetTotalVariables);
BIND_FUNC(123, TVarHandle, gsGetVariableByIndex, int index)
(index);

BIND_PROC(89, gsPostUserEvent, unsigned int evtId, bool bSync, void *usrData, unsigned int usrDataSize)
(evtId, bSync, usrData, usrDataSize);

BIND_FUNC(124, void *, gsGetUserEventData, TEventHandle hEvent, unsigned int *usrDataSize)
(hEvent, usrDataSize);

BIND_PROC0(125, gsPauseApp);
BIND_PROC0(126, gsResumeAndExitApp);

BIND_FUNC0(127, bool, gsIsNodeLocked);
BIND_FUNC0(128, bool, gsIsFingerPrintMatched);
BIND_FUNC0(129, const char *, gsGetUniqueNodeId);

BIND_FUNC0(130, bool, gsIsAppFirstLaunched);

BIND_FUNC(131, bool, gsIsServerAlive, int timeout)
(timeout);

BIND_PROC(132, gsIsServerAliveAsync, ping_cb pcb, void *userData, int timeout)
(pcb, userData, timeout);

BIND_FUNC(133, bool, gsApplySN, const char *sn, int *pRetCode, const char **ppSNRef, int timeout)
(sn, pRetCode, ppSNRef, timeout);

BIND_PROC(134, gsApplySNAsync, const char *sn, activate_cb activateCB, void *userData, int timeout)
(sn, activateCB, userData, timeout);

//136
BIND_FUNC(136, bool, gsHasLicense, TEntityHandle hEntity)
(hEntity);

//137
BIND_FUNC(137, TLicenseHandle, gsOpenLicense, TEntityHandle hEntity)
(hEntity);

//138
BIND_PROC(138, gsLockLicense, TLicenseHandle hLic)
(hLic);

//139
BIND_FUNC(139, bool, gsIsSNValid, const char *sn, int timeout)
(sn, timeout);

//140
BIND_PROC(140, gsIsSNValidAsync, const char *sn, testsn_cb cb, void *userData, int timeout)
(sn, cb, userData, timeout);

//135
BIND_FUNC(135, bool, gsRevokeApp, int timeout, const char *sn)
(timeout, sn);

//144
BIND_FUNC(144, bool, gsRevokeSN, int timeout, const char *sn)
(timeout, sn);

BIND_FUNC0(17, int, gsGetTotalUnlockSNs); //17

BIND_FUNC(49, const char *, gsGetUnlockSNByIndex, int index)
(index);

BIND_FUNC(142, int, gsGetTotalEntitiesUnlockedBySN, const char *sn)
(sn);

BIND_FUNC(143, const char *, gsGetEntityIdUnlockedBySN, const char *sn, int index)
(sn, index);

BIND_FUNC(154, const char *, gsGetSNByUnlockedEntityId, const char *entityId)
(entityId);

BIND_FUNC0(155, const char *, gsGetPreliminarySN);
//;;;;;;;;;;;;;;;; MOVE ;;;;;;;;;;;;;;;;
//145
BIND_FUNC(145, TMPHandle, gsMPCreate, int reserved)
(reserved);
//146
BIND_PROC(146, gsMPAddEntity, TMPHandle hMP, const char *entityId)
(hMP, entityId);
//147
BIND_FUNC(147, const char *, gsMPExport, TMPHandle hMP)
(hMP);
//148
BIND_FUNC(148, const char *, gsMPUpload, TMPHandle hMP, const char *sn, int timeout)
(hMP, sn, timeout);

//149
BIND_FUNC(149, TMPHandle, gsMPOpen, const char *mpStr)
(mpStr);

BIND_FUNC(141, bool, gsMPImportOnline, TMPHandle hMP, const char *sn, int timeout)
(hMP, sn, timeout);

//150
BIND_FUNC(150, const char *, gsMPGetImportOfflineRequestCode, TMPHandle hMP)
(hMP);
//151
BIND_FUNC(151, bool, gsMPImportOffline, TMPHandle hMP, const char *licenseCode)
(hMP, licenseCode);
//152
BIND_FUNC(152, const char *, gsMPUploadApp, const char *sn, int timeout)
(sn, timeout);

//153
BIND_FUNC0(153, const char *, gsMPExportApp);

BIND_FUNC(156, bool, gsMPCanPreliminarySNResolved, TMPHandle hMP)
(hMP);

BIND_FUNC(157, bool, gsMPIsTooBigToUpload, TMPHandle hMP)
(hMP);

BIND_FUNC(158, bool, gsApplyLicenseCodeEx, const char *licenseCode, const char *sn, const char *snRef)
(licenseCode, sn, snRef);

BIND_FUNC0(159, TCodeExchangeHandle, gsCodeExchangeBegin);

BIND_FUNC(160, const char *, gsCodeExchangeGetLicenseCode, gs_handle_t hCodeExchange, const char *productId, int buildId, const char *sn, const char *requestCode)
(hCodeExchange, productId, buildId, sn, requestCode);

BIND_FUNC(161, int, gsCodeExchangeGetErrorCode, gs_handle_t hCodeExchange)
(hCodeExchange);

BIND_FUNC(162, const char *, gsCodeExchangeGetErrorMessage, gs_handle_t hCodeExchange)
(hCodeExchange);

}; //gs
