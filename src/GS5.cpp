#include "GS5.h"

#ifdef _MSC_VER
#include <Windows.h>
#endif

#include <cstdarg>
#include <cstdio>
#include <stdexcept>

namespace gs {

//************** gs5_error *******************
#ifdef _MSC_VER
gs5_error::gs5_error(const char *msg, int code) : std::exception(msg), _code(code) {}
#else
gs5_error::gs5_error(const char *msg, int code) : _msg(msg), _code(code) {
}
#endif

NORETURN void gs5_error::raise(int code, const char *message, ...) {
    va_list list;
    va_start(list, message);
    char buf[1024];
#ifdef _MSC_VER
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, message, list);
#else
    vsnprintf(buf, sizeof(buf) - 1, message, list);
#endif
    va_end(list);

    throw gs5_error(buf, code);
}
//***************** TGSObject *******************

TGSObject::TGSObject(gs_handle_t handle) : _handle(handle) {
    assert(handle != INVALID_GS_HANDLE);
};
TGSObject::~TGSObject() { gsCloseHandle(_handle); }

//***************** TGSVariable *****************
//--- Static helpers ---
//
const char *TGSVariable::getTypeName(var_type_t varType) {
    return gsVariableTypeToString(varType);
};

//Permission conversion helpers
int TGSVariable::AttrFromString(const char *permitStr) {
    return gsVariableAttrFromString(permitStr);
};
std::string TGSVariable::AttrToString(int permit) {
    char Result[32];
    return gsVariableAttrToString(permit, Result, 32);
};

bool TGSVariable::hasValue() const {
    return gsIsVariableValid(_handle);
}

//Setter
void TGSVariable::fromString(const char *v) {
    if (!gsSetVariableValueFromString(_handle, v))
        throw gs5_error("String conversion error", GS_ERROR_INVALID_VALUE);
}
void TGSVariable::fromInt(int v) {
    if (!gsSetVariableValueFromInt(_handle, v))
        throw gs5_error("Int conversion error", GS_ERROR_INVALID_VALUE);
}
void TGSVariable::fromInt64(int64_t v) {
    if (!gsSetVariableValueFromInt64(_handle, v))
        throw gs5_error("Int64 conversion error", GS_ERROR_INVALID_VALUE);
}
void TGSVariable::fromFloat(float v) {
    if (!gsSetVariableValueFromFloat(_handle, v))
        throw gs5_error("Float conversion error", GS_ERROR_INVALID_VALUE);
}
void TGSVariable::fromDouble(double v) {
    if (!gsSetVariableValueFromDouble(_handle, v))
        throw gs5_error("Double conversion error", GS_ERROR_INVALID_VALUE);
}
void TGSVariable::fromUTCTime(time_t t) {
    if (!gsSetVariableValueFromTime(_handle, t))
        throw gs5_error("Time conversion error", GS_ERROR_INVALID_VALUE);
}

//Getter
const char *TGSVariable::asString() {
    return gsGetVariableValueAsString(_handle);
}

int TGSVariable::asInt() {
    int Result;
    if (!gsGetVariableValueAsInt(_handle, Result))
        throw gs5_error("Int conversion error", GS_ERROR_INVALID_VALUE);
    return Result;
}

bool TGSVariable::asBool() {
    return asInt() != 0;
}

void TGSVariable::fromBool(bool v) {
    this->fromInt(v ? 1 : 0);
}

int64_t TGSVariable::asInt64() {
    int64_t Result;
    if (!gsGetVariableValueAsInt64(_handle, Result))
        throw gs5_error("Int64 conversion error", GS_ERROR_INVALID_VALUE);
    return Result;
}

float TGSVariable::asFloat() {
    float Result;
    if (!gsGetVariableValueAsFloat(_handle, Result))
        throw gs5_error("Float conversion error", GS_ERROR_INVALID_VALUE);
    return Result;
}

double TGSVariable::asDouble() {
    double Result;
    if (!gsGetVariableValueAsDouble(_handle, Result))
        throw gs5_error("Double conversion error", GS_ERROR_INVALID_VALUE);
    return Result;
}
time_t TGSVariable::asUTCTime() {
    if (!gsIsVariableValid(_handle))
        throw gs5_error("variable does not hold a value", GS_ERROR_NO_VALUE);

    time_t Result;
    if (!gsGetVariableValueAsTime(_handle, Result))
        throw gs5_error("Time conversion error", GS_ERROR_INVALID_VALUE);
    return Result;
}

//Properties
const char *TGSVariable::name() { return gsGetVariableName(_handle); }
var_type_t TGSVariable::typeId() { return gsGetVariableType(_handle); }
std::string TGSVariable::attribute() { return AttrToString(gsGetVariableAttr(_handle)); }

template <>
void TGSVariable::get<int>(int &v) { v = this->asInt(); }
/// gets value as 64-bit integer
template <>
void TGSVariable::get<int64_t>(int64_t &v) { v = this->asInt64(); }
/// gets value as float
template <>
void TGSVariable::get<float>(float &v) { v = this->asFloat(); }
/// gets value as double
template <>
void TGSVariable::get<double>(double &v) { v = this->asDouble(); }
/// gets value as string
template <>
void TGSVariable::get<std::string>(std::string &v) { v = this->asString(); }

/// sets value from 32-bit integer
template <>
void TGSVariable::set<int>(int v) { this->fromInt(v); }
/// sets value from 64-bit integer
template <>
void TGSVariable::set<int64_t>(int64_t v) { this->fromInt64(v); }
/// sets value from float
template <>
void TGSVariable::set<float>(float v) { this->fromFloat(v); }
/// sets value from double
template <>
void TGSVariable::set<double>(double v) { this->fromDouble(v); }
/// sets value from string
template <>
void TGSVariable::set<std::string>(std::string v) { this->fromString(v.c_str()); }
//************************ TGSAction ***************************

TGSAction::TGSAction(gs_handle_t handle) : TGSObject(handle) {
    _totalParams = gsGetActionParamCount(_handle);
}

TGSVariable *TGSAction::getParamByIndex(int index) {
    if ((index < 0) || (index >= _totalParams)) {
        gs5_error::raise(GS_ERROR_INVALID_INDEX, "Index [%d] out of range [0, %d)", index, _totalParams);
    };
    return new TGSVariable(gsGetActionParamByIndex(_handle, index));
}

TGSVariable *TGSAction::getParamByName(const char *name) {
    gs_handle_t h = gsGetActionParamByName(_handle, name);
    if (h == INVALID_GS_HANDLE)
        gs5_error::raise(GS_ERROR_INVALID_NAME, "Invalid Param Name [%s]", name);

    return new TGSVariable(h);
}
//Properties
const char *TGSAction::name() {
    return gsGetActionName(_handle);
}
action_id_t TGSAction::id() {
    return gsGetActionId(_handle);
}
const char *TGSAction::description() {
    return gsGetActionDescription(_handle);
}
const char *TGSAction::whatToDo() {
    return gsGetActionString(_handle);
}

int TGSAction::paramCount() {
    return _totalParams;
}

//**************** TGSLicense **********************

bool TGSLicense::bindToEntity(TGSEntity *entity) {
    if (gsBindLicense(entity->handle(), this->handle())) {
        _licensedEntity = entity;
        return true;
    } else
        return false;
}

TGSVariable *TGSLicense::getParamByIndex(int index) const {
    if ((index >= 0) && (index < paramCount())) {
        return new TGSVariable(gsGetLicenseParamByIndex(_handle, index));
    }
    gs5_error::raise(GS_ERROR_INVALID_INDEX, "Index [%d] out of range [0, %d)", index, paramCount());
}

TGSVariable *TGSLicense::getParamByName(const char *name) const {
    gs_handle_t h = gsGetLicenseParamByName(_handle, name);
    if (h != INVALID_GS_HANDLE)
        return new TGSVariable(h);

    gs5_error::raise(GS_ERROR_INVALID_NAME, "Invalid Param Name [%s]", name);
}

std::string TGSLicense::getParamStr(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asString();
}
void TGSLicense::setParamStr(const char *name, const char *v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromString(v);
}

int TGSLicense::getParamInt(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asInt();
}

void TGSLicense::setParamInt(const char *name, int v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromInt(v);
}

int64_t TGSLicense::getParamInt64(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asInt64();
}
void TGSLicense::setParamInt64(const char *name, int64_t v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromInt64(v);
}

bool TGSLicense::getParamBool(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asBool();
}
void TGSLicense::setParamBool(const char *name, bool v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromBool(v);
}

float TGSLicense::getParamFloat(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asFloat();
}
void TGSLicense::setParamFloat(const char *name, float v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromFloat(v);
}

double TGSLicense::getParamDouble(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asDouble();
}
void TGSLicense::setParamDouble(const char *name, double v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromDouble(v);
}

time_t TGSLicense::getParamUTCTime(const char *name) const {
    std::unique_ptr<TGSVariable> var(params(name));
    return var->asUTCTime();
}
void TGSLicense::setParamUTCTime(const char *name, time_t v) {
    std::unique_ptr<TGSVariable> var(params(name));
    var->fromUTCTime(v);
}

//Properties
const char *TGSLicense::id() const {
    return gsGetLicenseId(_handle);
}

const char *TGSLicense::name() const {
    return gsGetLicenseName(_handle);
}

const char *TGSLicense::description() const {
    return gsGetLicenseDescription(_handle);
}

TLicenseStatus TGSLicense::status() const {
    return gsGetLicenseStatus(_handle);
}

bool TGSLicense::isValid() const {
    return gsIsLicenseValid(_handle);
}

void TGSLicense::lock() {
    gsLockLicense(_handle);
}

TGSEntity *TGSLicense::licensedEntity() const {
    return _licensedEntity;
}

std::string TGSLicense::getUnlockRequestCode() const {
    std::unique_ptr<TGSRequest> req(TGSCore::getInstance()->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_UNLOCK, this->_licensedEntity));
    return req->code();
}

int TGSLicense::paramCount() const {
    return gsGetLicenseParamCount(_handle);
}

TGSVariable *TGSLicense::params(int index) const {
    return getParamByIndex(index);
}

TGSVariable *TGSLicense::params(const char *name) const {
    return getParamByName(name);
}

int TGSLicense::actionCount() const {
    return gsGetActionInfoCount(_handle);
}

action_id_t TGSLicense::actionIds(int index) const {
    action_id_t Result;
    gsGetActionInfoByIndex(_handle, index, &Result);
    return Result;
}

const char *TGSLicense::actionNames(int index) const {
    action_id_t dummy;
    return gsGetActionInfoByIndex(_handle, index, &dummy);
}

//TAction
void TAction::addTo(TGSRequest *req) const {
    std::unique_ptr<TGSAction> act(req->addAction(id()));
    this->prepare(act.get());
}

void TAction::addTo(TGSRequest *req, const char *target_entityid) const {
    std::unique_ptr<TGSAction> act(req->addAction(id(), target_entityid));
    this->prepare(act.get());
}

void TAction::addTo(TGSRequest *req, TGSEntity* target_entity) const {
    std::unique_ptr<TGSAction> act(req->addAction(id(), target_entity));
    this->prepare(act.get());
}
// *************** TGSRequest ***************************
//Global action targeting all entities
TGSAction *TGSRequest::addAction(action_id_t actId) {
    return this->addAction(actId, (const char *)NULL);
}
//Action targeting all licenses of an entity
TGSAction *TGSRequest::addAction(action_id_t actId, TGSEntity *entity) {
    return this->addAction(actId, entity->id());
}

TGSAction *TGSRequest::addAction(action_id_t actId, const char *entityId) {
    gs_handle_t h = gsAddRequestActionEx(_handle, actId, entityId, NULL);
    if (h != INVALID_GS_HANDLE)
        return new TGSAction(h);

    gs5_error::raise(GS_ERROR_INVALID_ACTION, "Invalid action (actId = %d)", actId);
}

const char *TGSRequest::code() {
    return gsGetRequestCode(_handle);
}
//******************* TGSEntity ***************************

bool TGSEntity::beginAccess() {
    return gsBeginAccessEntity(_handle);
}
bool TGSEntity::endAccess() {
    return gsEndAccessEntity(_handle);
}

std::string TGSEntity::getUnlockRequestCode() {
    std::unique_ptr<TGSRequest> req(TGSCore::getInstance()->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_UNLOCK, this));
    return req->code();
}

bool TGSEntity::hasLicense() {
    return gsHasLicense(_handle);
}

TGSLicense *TGSEntity::getLicense() {
    gs_handle_t h = gsOpenLicense(_handle);
    if (h == INVALID_GS_HANDLE)
        gs5_error::raise(GS_ERROR_INVALID_LICENSE, "No License Bundled to entity[%s]", name());

    return new TGSLicense(h, this);
}
//Properties
unsigned int TGSEntity::attribute() {
    return gsGetEntityAttributes(_handle);
}
const char *TGSEntity::id() {
    return gsGetEntityId(_handle);
}
const char *TGSEntity::name() {
    return gsGetEntityName(_handle);
}
const char *TGSEntity::description() {
    return gsGetEntityDescription(_handle);
}

void TGSEntity::lock() {
    if (hasLicense()) {
        std::unique_ptr<TGSLicense> lic(getLicense());
        lic->lock();
    }
}

//************** TGSCore *************************

void WINAPI TGSCore::s_monitorCallback(int eventId, TEventHandle hEvent, void *usrData) {
    ((TGSCore *)usrData)->onEvent(eventId, hEvent);
}

void TGSCore::setAppEventHandler(TGSAppEventHandler handler, void *usrData) {
    _appEventHandler = handler;
    _appEventUsrData = usrData;
}
void TGSCore::setLicenseEventHandler(TGSLicenseEventHandler handler, void *usrData) {
    _licEventHandler = handler;
    _licEventUsrData = usrData;
}
void TGSCore::setEntityEventHandler(TGSEntityEventHandler handler, void *usrData) {
    _entityEventHandler = handler;
    _entityEventUsrData = usrData;
}

void TGSCore::setUserEventHandler(TGSUserEventHandler handler, void *usrData) {
    _userEventHandler = handler;
    _userEventUsrData = usrData;
}

void TGSCore::onEvent(int eventId, TEventHandle hEvent) {
    TEventType evtType = gsGetEventType(hEvent);
    switch (evtType) {
    case EVENT_TYPE_APP: {
        if (_appEventHandler)
            _appEventHandler(eventId, _appEventUsrData);
        break;
    }
    case EVENT_TYPE_LICENSE: {
        if (_licEventHandler)
            _licEventHandler(eventId, _licEventUsrData);
        break;
    }
    case EVENT_TYPE_ENTITY: {
        std::unique_ptr<TGSEntity> entity(new TGSEntity(gsGetEventSource(hEvent)));
        if (_entityEventHandler)
            _entityEventHandler(eventId, entity.get(), _entityEventUsrData);
        break;
    }

    case EVENT_TYPE_USER: {
        unsigned int evtDataSize;
        void *evtData = gsGetUserEventData(hEvent, &evtDataSize);
        if (_userEventHandler)
            _userEventHandler(eventId, evtData, evtDataSize, _userEventUsrData);
        break;
    }
    }
}

TGSCore::TGSCore() : _appEventHandler(NULL), _appEventUsrData(NULL),
                     _licEventHandler(NULL), _licEventUsrData(NULL), _entityEventHandler(NULL), _entityEventUsrData(NULL),
                     _userEventHandler(NULL), _userEventUsrData(NULL) {
    gsCreateMonitorEx(s_monitorCallback, this, "$SDK");
}

TGSCore::~TGSCore() {
    cleanUp();
}

static TGSCore *s_core = nullptr;
TGSCore *TGSCore::getInstance() {
    if (s_core == nullptr) {
        s_core = new TGSCore();
    }
    return s_core;
}

void TGSCore::finish() {
    if (s_core) {
        s_core->flush();
        s_core->cleanUp();
        delete s_core;
        s_core = nullptr;
    }
    sdk_finish();
}

int TGSCore::cleanUp() {
    return gsCleanUp();
}

bool TGSCore::init(const char *productId, const char *productLic, const char *licPassword) {
    return (0 == gsInit(productId, productLic, licPassword, NULL));
}

bool TGSCore::init(const char *productId, const unsigned char *pLicData, int licSize, const char *licPassword) {
    return (0 == gsInit(productId, pLicData, licSize, licPassword, NULL));
}

//Convert event id to human readable string, for debug purpose
const char *TGSCore::getEventName(int eventId) {
    struct TEventIdName {
        int id;
        const char *name;
    };
    static TEventIdName s_id_names[] = {
        {EVENT_APP_BEGIN, "EVENT_APP_BEGIN"},
        {EVENT_APP_RUN, "EVENT_APP_RUN"},
        {EVENT_APP_END, "EVENT_APP_END"},
        {EVENT_APP_CLOCK_ROLLBACK, "EVENT_APP_CLOCK_ROLLBACK"},
        {EVENT_APP_INTEGRITY_CORRUPT, "EVENT_APP_INTEGRITY_CORRUPT"},

        {EVENT_PASS_BEGIN_RING1, "EVENT_PASS_BEGIN_RING1"},
        {EVENT_PASS_BEGIN_RING2, "EVENT_PASS_BEGIN_RING2"},
        {EVENT_PASS_END_RING1, "EVENT_PASS_END_RING1"},
        {EVENT_PASS_END_RING2, "EVENT_PASS_END_RING2"},
        {EVENT_PASS_CHANGE, "EVENT_PASS_CHANGE"},

        {EVENT_LICENSE_NEWINSTALL, "EVENT_LICENSE_NEWINSTALL"},
        {EVENT_LICENSE_READY, "EVENT_LICENSE_READY"},
        {EVENT_LICENSE_FAIL, "EVENT_LICENSE_FAIL"},
        {EVENT_LICENSE_LOADING, "EVENT_LICENSE_LOADING"},

        {EVENT_ENTITY_TRY_ACCESS, "EVENT_ENTITY_TRY_ACCESS"},
        {EVENT_ENTITY_ACCESS_STARTED, "EVENT_ENTITY_ACCESS_STARTED"},
        {EVENT_ENTITY_ACCESS_ENDING, "EVENT_ENTITY_ACCESS_ENDING"},
        {EVENT_ENTITY_ACCESS_ENDED, "EVENT_ENTITY_ACCESS_ENDED"},
        {EVENT_ENTITY_ACCESS_INVALID, "EVENT_ENTITY_ACCESS_INVALID"},
        {EVENT_ENTITY_ACCESS_HEARTBEAT, "EVENT_ENTITY_ACCESS_HEARTBEAT"},
        {EVENT_ENTITY_ACTION_APPLIED, "EVENT_ENTITY_ACTION_APPLIED"}};

    if (eventId == -1)
        return "ALL Events";

    for (unsigned int i = 0; i < sizeof(s_id_names) / sizeof(s_id_names[0]); i++) {
        if (eventId == s_id_names[i].id)
            return s_id_names[i].name;
    }
    return "Unknown Event";
}

void TGSCore::flush() { gsFlush(); }

TGSEntity *TGSCore::getEntityByIndex(int index) const {
    int N = getTotalEntities();
    if ((index >= 0) && (index < N))
        return new TGSEntity(gsOpenEntityByIndex(index));

    gs5_error::raise(GS_ERROR_INVALID_INDEX, "Index [%d] out of range [0, %d)", index, N);
}

TGSEntity *TGSCore::getEntityById(entity_id_t entityId) const {
    gs_handle_t h = gsOpenEntityById(entityId);
    if (h != INVALID_GS_HANDLE)
        return new TGSEntity(h);

    gs5_error::raise(GS_ERROR_INVALID_ENTITY, "Invalid EntityId (%s)", entityId);
}

//Variables
TGSVariable *TGSCore::addVariable(const char *varName, TVarType varType,
                                  unsigned int permission, const char *initValStr) {
    return new TGSVariable(gsAddVariable(varName, varType, permission, initValStr));
}

bool TGSCore::removeVariable(const char *varName) {
    return gsRemoveVariable(varName);
}

TGSVariable *TGSCore::getVariableByIndex(int index) const {
    gs_handle_t h = gsGetVariableByIndex(index);
    if (h != INVALID_GS_HANDLE)
        return new TGSVariable(h);

    gs5_error::raise(GS_ERROR_INVALID_INDEX, "Invalid Variable Index [%s]", index);
}

TGSVariable *TGSCore::getVariableByName(const char *name) const {
    gs_handle_t h = gsGetVariable(name);
    if (h != INVALID_GS_HANDLE)
        return new TGSVariable(h);

    gs5_error::raise(GS_ERROR_INVALID_NAME, "Invalid Variable Name [%s]", name);
}

//Request
TGSRequest *TGSCore::createRequest() {
    return new TGSRequest(gsCreateRequest());
}

bool TGSCore::applyLicenseCode(const char *code, const char *sn, const char *snRef) {
    return gsApplyLicenseCodeEx(code, sn, snRef);
}

//---------- Time Engine Service ------------
void TGSCore::turnOnInternalTimer() { gsTurnOnInternalTimer(); }
void TGSCore::turnOffInternalTimer() { gsTurnOffInternalTimer(); }
bool TGSCore::isInternalTimerActive() { return gsIsInternalTimerActive(); }
void TGSCore::tickFromExternalTimer() { gsTickFromExternalTimer(); }
void TGSCore::pauseTimeEngine() { gsPauseTimeEngine(); }
void TGSCore::resumeTimeEngine() { gsResumeTimeEngine(); }
bool TGSCore::isTimeEngineActive() { return gsIsTimeEngineActive(); }

//-------- HTML Render -----------
bool TGSCore::renderHTML(const char *url, const char *title, int width, int height) {
    return gsRenderHTML(url, title, width, height);
}
bool TGSCore::renderHTML(const char *url, const char *title, int width, int height,
                         bool resizable, bool exitAppWhenUIClosed, bool cleanUpAfterRendering) {
    return gsRenderHTMLEx(url, title, width, height, resizable, exitAppWhenUIClosed, cleanUpAfterRendering);
}

const char *TGSCore::lastErrorMessage() { return gsGetLastErrorMessage(); }
int TGSCore::lastErrorCode() { return gsGetLastErrorCode(); }

const char *TGSCore::SDKVersion() { return gsGetVersion(); }

const char *TGSCore::productName() { return gsGetProductName(); }
const char *TGSCore::productId() { return gsGetProductId(); }
int TGSCore::buildId() { return gsGetBuildId(); }

bool TGSCore::runInVM() { return gsRunInsideVM(0xFFFFFFFF); }

int TGSCore::getTotalEntities() const { return gsGetEntityCount(); }

void TGSCore::lockAllEntities() {
    for (int i = 0; i < getTotalEntities(); i++) {
        std::unique_ptr<TGSEntity> entity(getEntityByIndex(i));
        entity->lock();
    }
}

bool TGSCore::isAllEntitiesLocked() const {
    for (int i = 0; i < getTotalEntities(); i++) {
        std::unique_ptr<TGSEntity> entity(getEntityByIndex(i));
        if (!entity->isLocked())
            return false;
    }
    return true;
}

//Debug Helpers (v5.0.14.0+)
bool TGSCore::isDebugVersion() {
    return gsIsDebugVersion();
}

void TGSCore::trace(const char *msg) {
    gsTrace(msg);
}

std::string TGSCore::getFixRequestCode() {
    std::unique_ptr<TGSRequest> req(this->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_FIX));
    return req->code();
}
std::string TGSCore::getUnlockRequestCode() {
    std::unique_ptr<TGSRequest> req(this->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_UNLOCK));
    return req->code();
}
std::string TGSCore::getCleanRequestCode() {
    std::unique_ptr<TGSRequest> req(this->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_CLEAN));
    return req->code();
}
std::string TGSCore::getDummyRequestCode() {
    std::unique_ptr<TGSRequest> req(this->createRequest());
    std::unique_ptr<TGSAction> act(req->addAction(ACT_DUMMY));
    return req->code();
}

/// Auto destroy TCore shared instance when not used.
struct TCoreDestroy {
    ~TCoreDestroy() {
        TGSCore::finish();
    }
};

static TCoreDestroy s_destroyCoreWhenNotUsed;

#if defined(DEBUG) || defined(_DEBUG)
void DebugMsg(const char *msg) {
#ifdef _WINDOWS
    OutputDebugStringA(msg);
#endif
    TGSCore::getInstance()->trace(msg);
}
void DebugMsgFmt(const char *format, ...) {
    va_list list;
    va_start(list, format);
    char buf[1024];
#ifdef _MSC_VER
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format, list);
#else
    vsnprintf(buf, sizeof(buf) - 1, buf, list);
#endif
    va_end(list);

    DebugMsg(buf);
}

//Auto Debug >> << pairs
TAutoDbg::~TAutoDbg() {
    DebugMsg((_msg + " <<").c_str());
}

TAutoDbg::TAutoDbg(const char *format, ...) {
    va_list list;
    va_start(list, format);
    char buf[1024];
#ifdef _MSC_VER
    vsnprintf_s(buf, sizeof(buf), _TRUNCATE, format, list);
#else
    vsnprintf(buf, sizeof(buf) - 1, format, list);
#endif
    _msg = buf;

    va_end(list);

    DebugMsg((_msg + " >>").c_str());
}

#endif
}; // namespace gs
