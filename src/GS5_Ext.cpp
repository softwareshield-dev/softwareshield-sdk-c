#include "GS5_Ext.h"
#include "GS5.h"
#include "GS5_Intf.h"

#include <vector>

namespace gs {

//************* Static Data ***************
typedef struct TLMInfo {
    f_createLM _createLM;
    const char *_id, *_name, *_description;

    TLMInfo(f_createLM createLM, const char *id, const char *name, const char *description) : _createLM(createLM), _id(id), _name(name), _description(description) {}
} * PLMInfo;

std::vector<TLMInfo *> s_lms;

void registerLM(f_createLM createLM, const char *licId, const char *licName, const char *description) {
    s_lms.push_back(new TLMInfo(createLM, licId, licName, description));
}
//********** TGSApp **************
TGSApp *TGSApp::s_createApp() {
    return new TGSApp();
}

TGSApp *TGSApp::s_app = NULL;
f_createApp TGSApp::s_appCreator = TGSApp::s_createApp;

void TGSApp::registerApp(f_createApp appCreator) {
    s_appCreator = appCreator;
}

TGSApp *TGSApp::getInstance() {
    if (s_app == NULL) {
        s_app = s_appCreator();
    }
    return s_app;
}

void TGSApp::init() {
    if (!OnAppInit()) {
        LOG0("TGSApp.init >> terminating...");
        terminateApp(-1);
    }
}

void TGSApp::OnPassBegin(int ring) {
    if (ring == 2)
        init();
}
void TGSApp::OnPassEnd(int ring) {}

void TGSApp::registerLicenseModels() {
    LOG0(">>");
    if (!s_lms.empty()) {
        for (std::vector<TLMInfo *>::iterator it = s_lms.begin(); it != s_lms.end(); it++) {
            TLMInfo *p = *it;
            gsRegisterCustomLicense(p->_id, TGSDynamicLM::s_createLM, p);
        }
    }
    LOG0("<<");
}

//Event Category handlers
void TGSApp::OnAppEvent(unsigned int evtId) {
    LOG("OnAppEvent >> Event [%s]", TGSCore::getEventName(evtId));
    switch (evtId) {
    case EVENT_PASS_BEGIN_RING1:
        OnPassBegin(1);
        break;
    case EVENT_PASS_BEGIN_RING2:
        OnPassBegin(2);
        break;
    case EVENT_PASS_END_RING1:
        OnPassEnd(1);
        break;
    case EVENT_PASS_END_RING2:
        OnPassEnd(2);
        break;

    case EVENT_APP_BEGIN:
        OnAppBegin();
        break;
    case EVENT_APP_END:
        OnAppEnd();
        break;
    case EVENT_APP_RUN:
        OnAppRun();
        break;
    case EVENT_APP_CLOCK_ROLLBACK:
        OnClockRolledBack();
        break;
    case EVENT_APP_INTEGRITY_CORRUPT:
        OnIntegrityCorrupted();
        break;
    }
    LOG("OnAppEvent << Event [%s]", TGSCore::getEventName(evtId));
}

void TGSApp::OnLicenseEvent(unsigned int evtId) {
    switch (evtId) {
    case EVENT_LICENSE_NEWINSTALL:
        OnNewInstall();
        break;
    case EVENT_LICENSE_LOADING: {
        registerLicenseModels();
        OnLicenseLoading();
    } break;

    case EVENT_LICENSE_READY:
        OnLicenseLoaded();
        break;
    case EVENT_LICENSE_FAIL:
        OnLicenseFail();
        break;
    }
}

void TGSApp::OnEntityEvent(unsigned int evtId, TGSEntity *entity) {
    LOG("OnEntityEvent >> Event [%s] entity [%s]", TGSCore::getEventName(evtId), entity->name());
    switch (evtId) {
    case EVENT_ENTITY_TRY_ACCESS:
        OnEntityAccessStarting(entity);
        break;
    case EVENT_ENTITY_ACCESS_STARTED:
        OnEntityAccessStarted(entity);
        break;
    case EVENT_ENTITY_ACCESS_ENDING:
        OnEntityAccessEnding(entity);
        break;
    case EVENT_ENTITY_ACCESS_ENDED:
        OnEntityAccessEnded(entity);
        break;
    case EVENT_ENTITY_ACCESS_INVALID:
        OnEntityAccessInvalid(entity, isGamePass() && !isLastPass());
        break;
    case EVENT_ENTITY_ACCESS_HEARTBEAT:
        OnEntityHeartBeat(entity);
        break;
    case EVENT_ENTITY_ACTION_APPLIED:
        OnEntityActionApplied(entity);
        break;
    }
}

//--- App Specific Event Handlers
bool TGSApp::OnAppInit() {
    return true;
}
void TGSApp::OnAppBegin() {}                              //EVENT_APP_BEGIN
void TGSApp::OnAppRun() {}                                //EVENT_APP_RUN
void TGSApp::OnAppEnd() {}                                //EVENT_APP_END
void TGSApp::OnClockRolledBack() {}                       //EVENT_APP_CLOCK_ROLLBACK
void TGSApp::OnIntegrityCorrupted(){};                    //EVENT_APP_INTEGRITY_CORRUPT
                                                          //---- License Specific Event Handlers
void TGSApp::OnNewInstall() {}                            //EVENT_LICENSE_NEWINSTALL
void TGSApp::OnLicenseLoading(){};                        //EVENT_LICENSE_LOADING
void TGSApp::OnLicenseLoaded(){};                         //EVENT_LICENSE_READY
void TGSApp::OnLicenseFail(){};                           //EVENT_LICENSE_FAIL
                                                          //----- Entity Specific Event Handlers
void TGSApp::OnEntityAccessStarting(TGSEntity *entity){}; //EVENT_ENTITY_TRY_ACCESS
void TGSApp::OnEntityAccessStarted(TGSEntity *entity){};  //EVENT_ENTITY_ACCESS_STARTED
void TGSApp::OnEntityAccessEnding(TGSEntity *entity){};   //EVENT_ENTITY_ACCESS_ENDING
void TGSApp::OnEntityAccessEnded(TGSEntity *entity){};    //EVENT_ENTITY_ACCESS_ENDED

void TGSApp::OnEntityAccessInvalid(TGSEntity *entity, bool inGame){}; //EVENT_ENTITY_ACCESS_INVALID
void TGSApp::OnEntityHeartBeat(TGSEntity *entity){};                  //EVENT_ENTITY_ACCESS_HEARTBEAT
                                                                      //------ Action Applied Event ------
void TGSApp::OnEntityActionApplied(TGSEntity *entity){};              //EVENT_ENTITY_ACTION_APPLIED

//
void TGSApp::s_appEventCB(unsigned int eventId, void *usrData) {
    ((TGSApp *)usrData)->OnAppEvent(eventId);
}
void TGSApp::s_licEventCB(unsigned int eventId, void *usrData) {
    ((TGSApp *)usrData)->OnLicenseEvent(eventId);
}
void TGSApp::s_entityEventCB(unsigned int eventId, TGSEntity *entity, void *usrData) {
    ((TGSApp *)usrData)->OnEntityEvent(eventId, entity);
}
void TGSApp::s_userEventCB(unsigned int eventId, void *eventData, unsigned int eventDataSize, void *usrData) {
    ((TGSApp *)usrData)->OnUserEvent(eventId, eventData, eventDataSize);
}

TGSApp::TGSApp() {
    //Initializes the gsCore
    _core = TGSCore::getInstance();
    //register my event handlers
    _core->setAppEventHandler(s_appEventCB, this);
    _core->setLicenseEventHandler(s_licEventCB, this);
    _core->setEntityEventHandler(s_entityEventCB, this);
    _core->setUserEventHandler(s_userEventCB, this);
}
TGSApp::~TGSApp() {
    _core->cleanUp();
}

const char *TGSApp::getGameTitle() { //Game Iitle
    return _core->productName();
}
void TGSApp::sendUserEvent(unsigned int eventId, void *eventData, unsigned int eventDataSize) {
    gsPostUserEvent(eventId, true, eventData, eventDataSize);
}
//------- App Control --------
void TGSApp::exitApp(int rc) {
    gsExitApp(rc);
}
void TGSApp::terminateApp(int rc) {
    gsTerminateApp(rc);
}
void TGSApp::playApp() {
    gsPlayApp();
}
void TGSApp::restartApp() {
    gsRestartApp();
}
void TGSApp::pauseApp() {
    gsPauseApp();
}
void TGSApp::resumeAndExitApp() {
    gsResumeAndExitApp();
}

bool TGSApp::isRestartedApp() {
    return gsIsRestartedApp();
}
//----- App Running Context -----
bool TGSApp::isFirstPass() {
    return gsIsFirstPass();
}
bool TGSApp::isGamePass() {
    return gsIsGamePass();
}
bool TGSApp::isLastPass() {
    return gsIsLastPass();
}

bool TGSApp::isFirstGameExe() {
    return gsIsFirstGameExe();
}
bool TGSApp::isLastGameExe() {
    return gsIsLastGameExe();
}

bool TGSApp::isMainThread() {
    TAG_SCOPE;
    return gsIsMainThread();
}

const char *TGSApp::getAppRootPath() {
    return gsGetAppRootPath();
}
const char *TGSApp::getAppCommandLine() {
    return gsGetAppCommandLine();
}
const char *TGSApp::getAppMainExe() {
    return gsGetAppMainExe();
}
//App Session Variables
void TGSApp::setSessionVar(const char *name, const char *val) {
    gsSetAppVar(name, val);
}
const char *TGSApp::getSessionVar(const char *name) {
    return gsGetAppVar(name);
}

//***************** TGSDynamicLM *********************
bool WINAPI TGSDynamicLM::fcb_isValid(void *usrData) {
    return ((TGSDynamicLM *)usrData)->isValid_();
}

void WINAPI TGSDynamicLM::fcb_startAccess(void *usrData) {
    ((TGSDynamicLM *)usrData)->startAccess_();
}

void WINAPI TGSDynamicLM::fcb_finishAccess(void *usrData) {
    ((TGSDynamicLM *)usrData)->finishAccess_();
}

void WINAPI TGSDynamicLM::fcb_onAction(TActionHandle hAct, void *usrData) {
    ((TGSDynamicLM *)usrData)->onAction_(hAct);
}

void WINAPI TGSDynamicLM::fcb_onDestroy(void *usrData) {
    delete (TGSDynamicLM *)usrData;
}

TLicenseHandle WINAPI TGSDynamicLM::s_createLM(void *usrData) {
    TLMInfo *p = (TLMInfo *)usrData;
    TGSDynamicLM *lm = p->_createLM(); //freed on fcb_onDestroy

    TLicenseHandle hLic = gsCreateCustomLicense(p->_id, p->_name, p->_description, lm,
                                                fcb_isValid, fcb_startAccess, fcb_finishAccess, fcb_onAction, fcb_onDestroy);

    lm->_lic.reset(new TGSLicense(hLic, NULL));
    lm->init();

    return hLic;
};

bool TGSDynamicLM::isValid_() {
    return isValid();
}

void TGSDynamicLM::startAccess_() {
    startAccess();
}

void TGSDynamicLM::finishAccess_() {
    finishAccess();
}

void TGSDynamicLM::onAction_(TActionHandle hAct) {
    std::auto_ptr<TGSAction> act(new TGSAction(hAct));
    onAction(act.get());
}
//Initialize the instance (init properties, etc.)
void TGSDynamicLM::init() {}

void TGSDynamicLM::defineParamStr(const char *paramName, const char *paramInitValue, unsigned int permission) {
    gsAddLicenseParamStr(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamInt(const char *paramName, int paramInitValue, unsigned int permission) {
    gsAddLicenseParamInt(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamInt64(const char *paramName, int64_t paramInitValue, unsigned int permission) {
    gsAddLicenseParamInt64(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamBool(const char *paramName, bool paramInitValue, unsigned int permission) {
    gsAddLicenseParamBool(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamFloat(const char *paramName, float paramInitValue, unsigned int permission) {
    gsAddLicenseParamFloat(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamDouble(const char *paramName, double paramInitValue, unsigned int permission) {
    gsAddLicenseParamDouble(_lic->handle(), paramName, paramInitValue, permission);
}
void TGSDynamicLM::defineParamTime(const char *paramName, time_t paramInitValue, unsigned int permission) {
    gsAddLicenseParamTime(_lic->handle(), paramName, paramInitValue, permission);
}

//LM handlers
//Sub-class should override these handlers or uses event properties
bool TGSDynamicLM::isValid() {
    return false;
}
void TGSDynamicLM::startAccess() {}
void TGSDynamicLM::finishAccess() {}
void TGSDynamicLM::onAction(TGSAction *act) {}

TGSDynamicLM::TGSDynamicLM() {}
TGSDynamicLM::~TGSDynamicLM() {}

//init app api
void initApp() {
    //Application launching, initializes my LM class instance.
    gs::TGSApp::getInstance();
}

}; // namespace gs

/*
  Deprecated by calling INIT_GS_APP manually

#ifndef EXPORT_API
#ifdef _MSC_VER
#define EXPORT_API __declspec(dllexport)
#else
#define EXPORT_API
#endif
#endif

extern "C" {
  EXPORT_API void WINAPI GS5_Entry(void * srv)
  {
    LOG("GS5_Entry: Initialize GSApp...");
    gs::initApp();
  }
}

*/