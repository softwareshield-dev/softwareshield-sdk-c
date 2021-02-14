/*! \file GS5.h
  \brief GS5 Core OOP Interface

  This file is needed to access gsCore apis in OOP style
  */
#ifndef _GS5_WRAP_H_
#define _GS5_WRAP_H_

#include <cassert>
#include <exception>
#include <memory>
#include <string>

#include "GS5_Intf.h"

namespace gs {

/// GS5 error code
enum {
    GS_ERROR_GENERIC = -1,        ///< Generic error
    GS_ERROR_INVALID_HANDLE = 1,  /**< null handle */
    GS_ERROR_INVALID_INDEX = 2,   /**< Index out of range */
    GS_ERROR_INVALID_NAME = 3,    /**< Invalid Variable Name */
    GS_ERROR_INVALID_ACTION = 4,  /**< Invalid action for target license */
    GS_ERROR_INVALID_LICENSE = 5, /**< Invalid license for target entity */
    GS_ERROR_INVALID_ENTITY = 6,  /**< Invalid entity for application */
    GS_ERROR_INVALID_VALUE = 7    /**< Invalid variable value */
};

#define TIMEOUT_USE_SERVER_SETTING -1
#define TIMEOUT_WAIT_INFINITE 0

#ifndef NORETURN
#ifdef _MSC_VER
#define NORETURN __declspec(noreturn)
#else
#define NORETURN __attribute__((noreturn))
#endif
#endif

//for c++17 no-discard attribute
#ifndef NODISCARD

#if ((defined(_MSVC_LANG) && _MSVC_LANG >= 201703L) || __cplusplus >= 201703L)
#define NODISCARD [[nodiscard]]
#else
#define NODISCARD
#endif

#endif

//GS5 exception
class gs5_error : public std::exception {
  private:
#ifndef _MSC_VER
    std::string _msg;
#endif
    int _code;

  public:
    gs5_error(const char *msg, int code = GS_ERROR_GENERIC);
    virtual ~gs5_error() {}

    int code() const { return _code; }

#ifndef _MSC_VER
    virtual const char *what() const {
        return _msg.c_str();
    }
#endif

    static NORETURN void raise(int code, const char *message, ...);
};

/** \brief Base of GS5 Objects
  *
  * In C++, the OOP-SDK apis return pointer to an instance of TGSObject subclass, the caller must delete it later to avoid memory leakage.
  *
  * \code

  TGSVariable * pVar = act->ParamByIndex(0);
  int v = pVar->asInt(); //uses pVar
  delete pVar; //release it once done

  or simply uses std::unique_ptr<T>:

  std::unique_ptr<TGSVariable> var(act->getParamByIndex(0));

  * \endcode
  *
  */
class TGSObject {
  protected:
    gs_handle_t _handle;

  public:
    TGSObject(gs_handle_t handle);
    virtual ~TGSObject();
    ///Gets GS5 object handle
    inline gs_handle_t handle() const { return _handle; }
};

///User defined variables or parameters of action / licenses.
class TGSVariable : public TGSObject {
  public:
    TGSVariable(gs_handle_t handle) : TGSObject(handle) {}

    //--- Static helpers ---
    /** \brief Converts variable type id to its type string
    *
    *  Ref: \ref varType
    */
    static const char *getTypeName(var_type_t varType);

    /** @name Attribute conversion helpers */
    //@{
    /// Get attribute value from its string representation. (Ref: \ref varAttr)
    static int AttrFromString(const char *permitStr);
    /// Get string representation from its attribute value. (Ref: \ref varAttr)
    static std::string AttrToString(int permit);
    //@}
    /** @name Value Accessor */
    //@{
    //Setter
    ///set value from a string
    void fromString(const char *v);
    ///set value from a 32-bit integerva
    void fromInt(int v);
    /// set value from a boolean
    void fromBool(bool v);
    /// set value from a 64-bit integer
    void fromInt64(int64_t v);
    /// set value from a float
    void fromFloat(float v);
    /// set value from a double
    void fromDouble(double v);
    /// set value from a time_t (total seconds since 01/01/1970)
    void fromUTCTime(time_t t);

    //Getter
    /// get value as string
    const char *asString();
    /// get valie as 32-bit integer
    int asInt();
    /// get value as boolean
    bool asBool();
    /// get value as 64-bit integer
    int64_t asInt64();
    /// get value as float
    float asFloat();
    /// get value as double
    double asDouble();
    /// get value as a time_t (total seconds since 01/01/1970)
    time_t asUTCTime();
    //@}
    /** @name Properties */
    //@{
    /// get the variable name
    const char *name();
    /// get the variable type id. (ref: \ref varType)
    var_type_t typeId();
    /// get the variable attribute (ref: \ref varAttr)
    std::string attribute();
    //@}
    //template based setter/getter
    template <typename T>
    void get(T &v) { static_assert(false && v, "TGSVariable::get<T> not resolved!"); }

    template <typename T>
    void set(T v) { static_assert(false && v, "TGSVariable::set<T> not resolved!"); }
};

/** @name C++ template based setter/getter [ C++ Only ] */
//@{
/// gets value as 32-bit integer
template <>
void TGSVariable::get<int>(int &v);
/// gets value as 64-bit integer
template <>
void TGSVariable::get<int64_t>(int64_t &v);
/// gets value as float
template <>
void TGSVariable::get<float>(float &v);
/// gets value as double
template <>
void TGSVariable::get<double>(double &v);
/// gets value as string
template <>
void TGSVariable::get<std::string>(std::string &v);

/// sets value from 32-bit integer
template <>
void TGSVariable::set<int>(int v);
/// sets value from 64-bit integer
template <>
void TGSVariable::set<int64_t>(int64_t v);
/// sets value from float
template <>
void TGSVariable::set<float>(float v);
/// sets value from double
template <>
void TGSVariable::set<double>(double v);
/// sets value from string
template <>
void TGSVariable::set<std::string>(std::string v);
//@}

/// GS5 Action Object
class TGSAction : public TGSObject {
  private:
    int _totalParams;

  public:
    TGSAction(gs_handle_t handle);

    /** @name Properties */
    //@{
    /// Gets action name
    const char *name();
    /// Gets action unique id
    action_id_t id();
    /// Gets action description
    const char *description();
    /// Gets action what-to-do string (ref: \ref WhatToDoActionString "What to do action string")
    const char *whatToDo();
    //@}
    /** @name Inspect action's parameters */
    //@{
    /// Gets the total number of action parameters
    int paramCount();

    /**
      \brief Gets action parameter by its index

      \param index the index of parameter, range [0, paramCount()-1 ]
      \return a TGSVariable instance on success, NULL if paramer not found
      */
    TGSVariable *getParamByIndex(int index);
    /**
      \brief Gets action parameter by its name

      \param name the string name of a action parameter.
      \return a TGSVariable instance on success, NULL if paramer not found
      */
    TGSVariable *getParamByName(const char *name);
    //@}
};

class TGSEntity;

/// GS License Object
class TGSLicense : public TGSObject {
  private:
    TGSEntity *_licensedEntity;

  public:
    //Create a license instance by attaching to a license handle (which might be already attached to an entity)
    TGSLicense(gs_handle_t handle, TGSEntity *entity = NULL) : TGSObject(handle), _licensedEntity(entity) {}

    /** \brief Create a license instance by license type id
    *
    *  Usually, the GS5 core creates license instances and bind them to associated entities automatically so there is no need
    * to create a license instance manually by this api; however, if you are trying to do license-entity dynamic-binding at runtime,
    * you can create a license by its type id and calls bindToEntity() to attach the license to a target entity.
    */
    TGSLicense(const char *licId) : TGSObject(gsCreateLicense(licId)), _licensedEntity(NULL) {}

    /** \brief bind a license to an entity
    *
    * Attach this license to a target entity. A license can be attached only to a single entity, while one entity can be attached to
    * multiple licenses.
    */
    bool bindToEntity(TGSEntity *entity);

    /** @name License Parameter APIs */
    //@{
    /// Gets total number of parameters
    int paramCount();
    /// C++: Gets parameter by its index ( \see getParamByIndex)
    TGSVariable *params(int index);
    /// C++: Gets parameter by its name ( \see getParamByName )
    TGSVariable *params(const char *name);
    /// Get the license parameter by its index, ranges [0, paramCount()-1 ]
    TGSVariable *getParamByIndex(int index);
    /// Get the license parameter by its name.
    TGSVariable *getParamByName(const char *name);

    //@}

    //Common Request code helpers
    /// Gets a request code to unlock this license only.
    std::string getUnlockRequestCode();

    /** @name License Properties */
    //@{
    /// Gets the license object's license id
    const char *id();
    /// Gets the license object's license name
    const char *name();
    /// Gets the license object's license description
    const char *description();

    /// Gets the license object's license status (ref: \ref LicenseStatus)
    TLicenseStatus status();
    /** \brief Is license currently valid?
    *
    * In GS5, an entity can be bundled with a license, the entity is accessible if
    * any or all of its bundled licenses are valid.
    *
    */
    bool isValid();

    /** \brief Lock a license
    *
    *  In GS5, we can lock a license from code explicitly, but cannot unlock it without applying an authorized action
    */
    void lock();

    //@}

    /// Gets the entity this license is attached to.
    TGSEntity *licensedEntity();

    /** @name License parameter access helpers
    *
    *  Instead of getting the TGSVariable instance of license parameters and get its value via "asXXX()" functions,
    *  these helpers give you short-cut to retrieve the parameter value directly.
    */
    //@{
    /// Gets parameter value as a string
    std::string getParamStr(const char *name);
    /// Sets parameter value from a string
    void setParamStr(const char *name, const char *v);

    ///Gets parameter value as a 32-bit integer
    int getParamInt(const char *name);
    ///Sets parameter value as a 32-bit integer;
    void setParamInt(const char *name, int v);

    ///Gets parameter value as a 64-bit integer
    int64_t getParamInt64(const char *name);
    ///Sets parameter value as a 64-bit integer;
    void setParamInt64(const char *name, int64_t v);

    ///Gets parameter value as boolean
    bool getParamBool(const char *name);
    ///Sets parameter value from a boolean;
    void setParamBool(const char *name, bool v);

    ///Gets parameter value as double;
    double getParamDouble(const char *name);
    ///Sets parameter value from a double
    void setParamDouble(const char *name, double v);

    ///Gets parameter value as float
    float getParamFloat(const char *name);
    ///Sets parameter value from float
    void setParamFloat(const char *name, float v);

    ///Gets parameter value as time_t (seconds since 01/01/1970)
    time_t getParamUTCTime(const char *name);
    ///Sets parameter value from time_t (seconds since 01/01/1970)
    void setParamUTCTime(const char *name, time_t v);
    //@}

    /**@name Inspect license model's actions */

    /**
    * \brief Gets total number of actions appliable to a license
    * Ref: \ref ActionInfo
    */
    int actionCount();
    /// Gets action id by index (ref: \ref ActionInfo)
    action_id_t actionIds(int index);
    /// Gets action name by index (ref: \ref ActionInfo)
    const char *actionNames(int index);
};

/** \brief Request Object
  *
  * Request object hosts all desired actions to be applied to local license.
  *
  * When a request (its request code) is sent to server its content is double-checked by server logic,
  * and once approved, a corresponding license code (also known as *activation code*)
  * is returned to authorize the action(s) applying to modify local license status.
  *
  * The server (or gsKeyGenerator) might modify the contained action(s) to enforce license requirement;
  *
  * Request object also contains client side fingerprint information, so even it contains _no_ action, the request code
  * can also be used to *sampling* client machine's hardware information, and if necessary, the server side can generate
  * license code targeting the client's machine without a request code.
  *
  */
class TGSRequest : public TGSObject {
  public:
    TGSRequest(gs_handle_t handle) : TGSObject(handle) {}

    /** \brief adds a global action targeting all entities
    *
    * Adds an action targeting the whole license storage (ACT_CLEAN), or can be applied to all entities( ACT_LOCK, ACT_UNLOCK, etc.)
    */
    NODISCARD TGSAction *addAction(action_id_t actId);
    /** \brief adds an action targeting all licenses of an entity
    *
    * \param actId Action type id;
    * \param entity the target entity, the action will be applied to all licenses attached to the entity.
    * \return the pointer to action object, NULL if the action type id is not supported.
    */
    NODISCARD TGSAction *addAction(action_id_t actId, TGSEntity *entity);
    /** \brief adds an action targeting a single license object
    *
    * \param actId Action type id;
    * \param entityId the target entity id to which the target license is attached;
    * \return the pointer to action object, NULL if the action type id is not supported.
    */
    NODISCARD TGSAction *addAction(action_id_t actId, const char *entityId);

    /** \brief gets the request string code
    *
    *  Ref: \ref requestCode "Request Code"
    */
    const char *code();
};

/// Entity Object
class TGSEntity : public TGSObject {
  public:
    TGSEntity(gs_handle_t handle) : TGSObject(handle) {}
    virtual ~TGSEntity() = default;

    //****** Licensing Status *****
    ///Is Entity accessible? (Passed attached license(s) verfication)
    bool isAccessible() { return (attribute() & ENTITY_ATTRIBUTE_ACCESSIBLE) != 0; }
    ///Is Entity being accessed? ( between beginAccess & endAccess )
    bool isAccessing() { return (attribute() & ENTITY_ATTRIBUTE_ACCESSING) != 0; };
    ///Is Entity unlocked? (Fully Purchased, etc.)
    bool isUnlocked() { return (attribute() & ENTITY_ATTRIBUTE_UNLOCKED) != 0; };
    ///Is Entity locked? (Expired, obsoleted, etc.)
    bool isLocked() { return (attribute() & ENTITY_ATTRIBUTE_LOCKED) != 0; };

    /** \brief Try start accessing an entity.
     *
     * If an entity is accessible, all of the associated resources (files, keys, codes, etc.) can be legally used, otherwise
     * they cannot be accessed by the application.
     *
     * This api can be called recursively, and each call must be paired with an endAccess().

     * When the api is called an event EVENT_ENTITY_TRY_ACCESS is triggered to give the GS5 extension developer a chance to change the entity license
     * status, if after the EVENT_ENTITY_TRY_ACCESS posting the entity is still not accessible, then EVENT_ENTITY_ACCESS_INVALID is posted, otherwise,
     * if the entity is being accessed _for the very first time_, the EVENT_ENTITY_ACCESS_STARTED is posted. the developer can then initialize
     * needed resources for this entity in the event handler.
     *
     * \return 	returns true if the entity is accessed successfully.
     returns false if:
     - Cannot access any entity when your game is wrapped by a *DEMO* version of GS5/IDE and the its demo license has expired;
     - Entity cannot be accessed due to its negative license feedback;

     \see gs::gsBeginAccessEntity()

     */
    bool beginAccess();

    /** \brief Try end accessing an entity

      \return true on success, false if there is unexpected error occurs.

      This api must be paired with beginAccess(), if it is the last calling then event EVENT_ENTITY_ACCESS_ENDING and
      EVENT_ENTITY_ACCESS_ENDED will be posted.

      \see gs::gsEndAccessEntity()

      */
    bool endAccess();

    /// Lock the bundled license
    void lock();

    //Common used request code
    /// Get the *Unlock* request code to unlock all attached license(s)
    std::string getUnlockRequestCode();

    /** @name Properties **/
    //@{{{{{{{{{{{{{{{{{{{{{{{{
    /// Entity Attributes (ref: \ref EntityAttr)
    unsigned int attribute();
    /// Entity Id
    const char *id();
    /// Entity Name
    const char *name();
    /// Entity Description
    const char *description();
    //@}}}}}}}}}}}}}}}}}}}}}}}}

    /** @name License **/
    //@{{{{{{{{{{{{{{{{{{{{{{{{{{{{
    /// Is a licene attached to this entity?
    bool hasLicense();

    ///Get the attached license
    TGSLicense *getLicense();
    //@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
};

/// License Move
class TMovePackage : public TGSObject {
  public:
    TMovePackage(gs_handle_t handle) : TGSObject(handle) {}

    void addEntityId(const char *entityId) {
        gsMPAddEntity(_handle, entityId);
    }
    ///------- Move License Online --------
    ///Returns a receipt ( actually a SN ) from server on success
    ///
    /// It will be used to activate app on the target machine so
    /// should be saved in a safely place.
    ///
    /// After this api returns, the entities in this move package are locked.
    ///

    const char *upload(const char *preSN = NULL) {
        //make sure we have a valid preliminary SN for online operation
        assert(preSN || canPreliminarySNResolved());

        return gsMPUpload(_handle, preSN, TIMEOUT_WAIT_INFINITE);
    }

    bool isTooBigToUpload() {
        return gsMPIsTooBigToUpload(_handle);
    }
    ///----- Move License Offline ---------
    ///Returns encrypted data string of move package
    /// It will be used to activate app on the target machine so
    /// should be saved in a safely place.
    ///
    /// On Success:
    ///   return non-empty string, and the entities in this move package are locked.
    ///
    std::string exportData() {
        return gsMPExport(_handle);
    }

    const char *getImportOfflineRequestCode() {
        return gsMPGetImportOfflineRequestCode(_handle);
    }

    bool importOffline(const char *licenseCode) {
        return gsMPImportOffline(_handle, licenseCode);
    }

    bool importOnline(const char *preSN = NULL) {
        //make sure we have a valid preliminary SN for online operation
        assert(preSN || canPreliminarySNResolved());

        return gsMPImportOnline(_handle, preSN, TIMEOUT_WAIT_INFINITE);
    }

    //
    bool canPreliminarySNResolved() {
        return gsMPCanPreliminarySNResolved(_handle);
    }
};

class TCodeExchange : public TGSObject {
  public:
    TCodeExchange(TCodeExchangeHandle hCodeExchange) : TGSObject(hCodeExchange) {}

    // buildId: set to -1 for the latest build, otherwise specifies the exact app build-id the requestCode is generated from.
    //
    //Return non-empty string on success, empty string on failure.
    const char *getLicenseCode(const char *productId, int buildId, const char *sn, const char *requestCode) {
        return gsCodeExchangeGetLicenseCode(_handle, productId, buildId, sn, requestCode);
    }

    int getErrorCode() {
        return gsCodeExchangeGetErrorCode(_handle);
    }

    const char *getErrorMessage() {
        return gsCodeExchangeGetErrorMessage(_handle);
    }
};

typedef void (*TGSAppEventHandler)(unsigned int eventId, void *usrData);
typedef void (*TGSLicenseEventHandler)(unsigned int eventId, void *usrData);
typedef void (*TGSEntityEventHandler)(unsigned int eventId, TGSEntity *entity, void *usrData);
typedef void (*TGSUserEventHandler)(unsigned int eventId, void *eventData, unsigned int eventDataSize, void *usrData);

/** \brief GS5 Core Object
  *
  * TGSCore is a singleton class to interact with GS5 runtime.
  *
  * It can be retrieved by TGSCore::getInstance(), or, if you are working on a subclass of TGSApp, just refer to it via TGSApp::core() member function.
  *
  * \anchor whyInit
  *
  * #### Why Core must be initialized before API calling?
  *
  * The core has to be initialized ( TGSCore::init() ) __successfully__ before most core apis can be used, except those apis that do not need access to license
  * information therefore do not need a previous initialization, in which case they are tagged as __static__ member functions.
  *
  * \code
  *      cout << "SDK version: " << TGSCore::SDKVersion() << " is debug version? " << TGSCore::isDebugVersion() ? "yes" : "no" << endl;
  * \endcode
  *
  * However, TGSCore::renderHTML() is a special case, if the Web page is just a generic one ( normal web pages) it can render without
  * calling TGSCore::init() first, otherwise if the Web page has content releated to current license status (via javascript gs5.js, etc.) then
  * the TGSCore must be initialized before page rendering begins.
  *
  * \anchor whenInit
  * #### When Core should be initialized in my source code?
  *
  * When developing GS5 extension with SDK, where should I put gsInit() or TGSCore::init() calling in my source code?
  *
  * Depending on the usage of SDK, there are three senerios:

  + Your extension module is not wrapped at all:
  *  If you are developing a GS5 extension dll (or simply compiled in the game exe) that is not delivered in a wrapped mode, it is the developer's reponsibility to initialize the
  *  core before its apis are called.
  *
  *  Usually it happens if you are trying to develop simple test cases to play with the SDK apis, or develop internal utility to test or view license information, etc.
  *
  + Your extension module is wrapped but in _Ironwrapper Only_ mode:
  *  In this case, the GS5 kernel won't get a chance to initialize itself so the developer also has to initialize the core first before most api callings are mode.
  *
  + Your extension module is wrapped in GS5 IDE's *Built-In* or *Custom* license model mode:
  *
  *  When game loading, a GS5 system component ( gsLoader.dll on Windows ) will be embedded to orchestra the workflow of application execution mode, event broadcasting, etc. The GS5 kernel
  * can listen to application startup events and initialize itself properly before EVENT_APP_BEGIN is fired.
  *
  *  If your code follows the SDK design pattern that is completely event-driven, there is no need to initialize GS5 core manually since it is already done by the system. You can call gsInit() manually
  * in your event handlers but is totally useless and bypassed directly in GS5 core.
  *
  *
  */
class TGSCore {
  private:
    //Installable event handlers
    TGSAppEventHandler _appEventHandler;
    void *_appEventUsrData;

    TGSLicenseEventHandler _licEventHandler;
    void *_licEventUsrData;

    TGSEntityEventHandler _entityEventHandler;
    void *_entityEventUsrData;

    TGSUserEventHandler _userEventHandler;
    void *_userEventUsrData;

    static void WINAPI s_monitorCallback(int eventId, TEventHandle hEvent, void *usrData);

    void onEvent(int eventId, TEventHandle hEvent);

    TGSCore();
    ~TGSCore();

  public:
    /// Get the single core object instance
    static TGSCore *getInstance();
    /// Releases shared core instance, should be called before app exiting to avoid memory leakage.
    static void finish();

    /** \brief Clean up the core
     *
     * When the application is terminating, call this api to cleanup the internal data resources.
     *
     * When game runs in wrapped mode, GS5 kernel will detect game exiting and flush all pending data to local license storage
     * automatically on terminating, so this api is optional and GS5 extension developer do not have to call it manually.
     *
     * For a simple non-wrapping SDK usage, this api must be called before game terminates to make sure the in-memory pending license
     * changes are saved to local storage.
     *
     * \see gsCleanUp()
     */
    int cleanUp();

    //Event Handlers (uses TGSApp instead)
    void setAppEventHandler(TGSAppEventHandler handler, void *usrData);
    void setLicenseEventHandler(TGSLicenseEventHandler handler, void *usrData);
    void setEntityEventHandler(TGSEntityEventHandler handler, void *usrData);
    void setUserEventHandler(TGSUserEventHandler handler, void *usrData);

    /** @name License Initialization / Load APIs */
    //@{
    /**
     * \brief One-time Initialization of gsCore from License File
     *
     One time GS5 kernel initialize, always update local storage as needed. [Read & Write]

     It tries to search and load license in the following order:
     - Loads from local storage first;
     - Loads from embedded license data;
     - Loads from input license file;

     * \param productId The Product Unique Id.
     * 		Each product has an unique Id that can be used to identity the application on the remote server.
     * \param productLic The full path to the original license document.
     * 		This license file is compiled from the GameShield project IDE and defines the initial license status of the product.
     * 		The file will not be altered and can be deployed in any folder.
     * \param licPassword The string key to decrypt the license document.
     *
     * \return Returns true on success, false on error, uses lastErrorCode()/lastErrorMessage() to get the error information.


     Ref:
     - \ref whyInit "Why Core must be initialized before API calling?"
     - \ref whenInit "When Core should be initialized in my source code?"
     */

    bool init(const char *productId, const char *productLic, const char *licPassword);
    /**
    * \brief One-time Initialization of gsCore from in-memory license data
    *
    One time GS5 kernel initialize, always update local storage as needed. [Read & Write]

    It tries to search and load license in the following order:
    - Loads from local storage first;
    - Loads from embedded license data;
    - Loads from input license file;

    * \param productId The Product Unique Id.
    * 		Each product has an unique Id that can be used to identity the application on the remote server.
    * \param pLicData The pointer to the original license data.
    * 		A license file is compiled from the GameShield project IDE and defines the initial license status of the product.
    *    It can be embedded as application resource and retrieved in memory at runtime before passing as param to this api.
    * \param licSize The length of license data
    *
    * \param licPassword The string key to decrypt the license document.
    *
    * \return Returns true on success, false on error, uses lastErrorCode()/lastErrorMessage() to get the error information.


    Ref:
    - \ref whyInit "Why Core must be initialized before API calling?"
    - \ref whenInit "When Core should be initialized in my source code?"
    */
    bool init(const char *productId, const unsigned char *pLicData, int licSize, const char *licPassword);

    //@}

    /** \brief Convert event id to human readable string, for debug purpose
    *
    * It can be used to display user-friendly debug message to log view.
    *
    * ref: \ref eventId "Event Id"
    */
    static const char *getEventName(int eventId);

    ///Save license immediately if dirty
    void flush();

    /** @name Entity Enumeration */
    //@{
    /// Get total entities
    int getTotalEntities() const;

    /// Get entity by index ( 0 <= index < getTotalEntities()-1 )
    TGSEntity *getEntityByIndex(int index) const;
    /// Get entity by its unique entity id
    TGSEntity *getEntityById(entity_id_t entityId) const;
    //@}
    /** @name "User Defined Variables" */
    //@{{{

    /**
    * \brief Adds a user defined variable
    *
    * \param varName variable name
    * \param varType \ref TVarType, ref: \ref varType "Variable Type"
    * \param attr variable attribute, ref: \ref varAttr "Variable Attribute"
    * \param initValStr  string representation of initial variable value
    * \return if the variable already exists, returns the variable object, otherwise returns the the created new variable
    *
    */
    TGSVariable *addVariable(const char *varName, TVarType varType,
                             unsigned int attr, const char *initValStr);

    /** \brief remove user defined variable by its name
    * \param varName variable name
    * \return true if success, false if variable not found
    */
    bool removeVariable(const char *varName);

    /// Get total user defined variables
    int getTotalVariables() const {
        return gsGetTotalVariables();
    }
    /// Get user defined variable by its name
    TGSVariable *getVariableByName(const char *name) const;
    /// Get user defined variable by index ( 0 <= index < getTotalVariables() )
    TGSVariable *getVariableByIndex(int index) const;
    //@}}}
    /// Create a request object
    TGSRequest *createRequest();

    /// Apply license code
    bool applyLicenseCode(const char *code, const char *sn = NULL, const char *snRef = NULL);

    /** @name Time Engine Service */
    //@{
    /**\brief Turn on internal timer
    *
    * After TGSCore::init() is called, the internal timer is automatically turned on.
    */
    void turnOnInternalTimer();
    /** \brief Turn off internaltimer
    *
    * After TGSCore::init() is called, the internal timer is automatically turned on. if you want to use your own timer,
    * you should turn off the internal timer first, then call tickFromExteralTimer() manually from your timer routine.
    */
    void turnOffInternalTimer();
    /// Is internal timer turned on?
    bool isInternalTimerActive();
    /** \brief tick the core time engine from external timer
    *
    * The external timer frequency is not necessarily 1 HZ. Using an external timer you can control the thread context in which
    * GS events are fired.
    */
    void tickFromExternalTimer();
    /** \brief Pause the GS5 core time engine.
    *
    *  When GS5 time engine is paused, no event can be fired.
    *
    */
    void pauseTimeEngine();
    /// Resume time engine if it is previously paused.
    void resumeTimeEngine();
    /// Is time engine currently active? ( firing events )
    bool isTimeEngineActive();
    //@}
    /** @name HTML Render */
    //@{

    /**
      \brief Rendering HTML page in process.

      \param url  URL to html local file or web site page to render;
      \param title The caption of form window rendering the HTML page;
      \param width Pixel width of HTML page;
      \param height Pixel height of HTML page;

      It can be called *before* init() to render generic HTML pages.
      However, init() must be called before to render LMApp HTML pages.

      The default behavior is:
      - Windows Resizable = True;
      - ExitAppAfterUI = False;
      - CleanUpAfterRender = False;
      */
    static bool renderHTML(const char *url, const char *title, int width, int height);
    /**
      \brief Rendering HTML with more control	 (Since SDK 5.0.7)

      \param url  URL to html local file or web site page to render;
      \param title The caption of form window rendering the HTML page;
      \param width Pixel width of HTML page;
      \param height Pixel height of HTML page;
      \param resizable: The HTML windows is resizable.
      \param exitAppWhenUIClosed: Terminate current process when the HTML main windows is manually closed (clicking [x] button on right title for Windows).
      if false, the UI is just closed.
      \param cleanUpAfterRendering: Clean up all of the internal rendering facilities before this API returns.
      If you cleanup rendering, then the possible conflicts with game are minimized, however, next time the render engine has to be re-created again.
      WARNING: If set to true, the Qt rendering engine might CRASH for the second time api calling due to Qt internal issue!!!
      So the best practice is that: Only set cleanUpAfterRendering to true if it is the last time rendering.

      If you do not cleanup rendering, the render engine stays active in memory and is quick for next rendering.
      However, since the Qt/Win stuffs is still alive, it might conflict with game in unexpected way.(Mac: The top main menu bar, about, etc.)

      */
    static bool renderHTML(const char *url, const char *title, int width, int height,
                           bool resizable, bool exitAppWhenUIClosed, bool cleanUpAfterRendering);
    //@}
    //======================================================
    ///Get the last error message
    const char *lastErrorMessage();
    ///Get the last error code
    int lastErrorCode();

    ///Get the current SDK binary version
    static const char *SDKVersion();

    ///Get product name
    const char *productName();
    ///Get product unique id
    const char *productId();
    ///Get product Build Id (ref: \ref BuildId "Build Id")
    int buildId();

    /**
    *	\brief Test if the current process is running inside GS5 Ironwrapper runtime
    *
    *	It can be called before gsInit().
    *
    */
    static bool runInWrappedMode() {
        return gsRunInWrappedMode();
    }

    ///	Test if the current process is runing inside any virtual machine.
    static bool runInVM();

    //Debug Helpers (v5.0.14.0+)
    /// Is current SDK binary a debugger version?
    static bool isDebugVersion();

    /**
    *
    * \brief Output debug message
    *
    *  For SDK/Debug version, the message is appended to the current debug log file.
    *  For SDK/Release version, the message is displayed in: (1) DebugViewer (Windows via OutputDebugString); (2) Console (Unix via printf)
    *
    */
    void trace(const char *msg);

    /** @name Commonly used request code helpers */
    //@{
    /// Get request code to fix license error ( all entities/licenses )
    std::string getFixRequestCode();
    /// Get request code to unlock the whole application ( all entities/licenses )
    std::string getUnlockRequestCode();
    /// Get request code to clean up the local license
    std::string getCleanRequestCode();
    /// Get request code to send client information (fingerprint) to server
    std::string getDummyRequestCode();
    //@}

    bool isNodeLocked() {
        return gsIsNodeLocked();
    }
    bool isFingerPrintMatched() {
        return gsIsFingerPrintMatched();
    }
    const char *getUniqueNodeId() {
        return gsGetUniqueNodeId();
    }

    bool isAppFirstLaunched() {
        return gsIsAppFirstLaunched();
    }

    bool isServerAlive(int timeout = TIMEOUT_USE_SERVER_SETTING) const {
        return gsIsServerAlive(timeout);
    }

  private:
    //A copy in heap of a functional object instance / function pointer in stack
    template <typename T>
    struct TParamCopy {
        T _fcb;
        TParamCopy(const T &fcb) : _fcb(fcb) {}
    };

    template <typename T>
    static void WINAPI s_pingCB(bool ok, void *userData) {
        std::unique_ptr<TParamCopy<T>> param((TParamCopy<T> *)userData);
        param->_fcb(ok);
    }

  public:
    template <typename T>
    void isServerAlive(T cb, int timeout = TIMEOUT_USE_SERVER_SETTING) {
        gsIsServerAliveAsync(s_pingCB<T>, new TParamCopy<T>(cb), timeout);
    }

    //void isServerAlive(ping_cb callback, void * userData = NULL, int timeout =TIMEOUT_USE_SERVER_SETTING ){
    //  gsIsServerAliveAsync(callback, userData, timeout);
    //}

    bool applySN(const char *sn, int *pRetCode = NULL, std::string *pSNRef = NULL, int timeout = TIMEOUT_USE_SERVER_SETTING) {
        const char *p = NULL;
        bool ok = gsApplySN(sn, pRetCode, &p, timeout);
        if (p && pSNRef)
            *pSNRef = p;
        return ok;
    }

  private:
    template <typename T>
    static void WINAPI s_activateCB(const char *sn, bool success, int rc, const char *snRef, void *userData) {
        std::unique_ptr<TParamCopy<T>> param((TParamCopy<T> *)userData);
        param->_fcb(success, rc, snRef);
    };

  public:
    template <typename T>
    void applySNAsync(const char *sn, T fcb, int timeout = TIMEOUT_USE_SERVER_SETTING) {
        gsApplySNAsync(sn, s_activateCB<T>, new TParamCopy<T>(fcb), timeout);
    }

    //void applySN(const char* sn, activate_cb activateCB, void * userData = NULL, int timeout = TIMEOUT_USE_SERVER_SETTING ){
    //  gsApplySNAsync(sn, activateCB, userData, timeout);
    //}

    bool isSNValid(const char *sn, int timeout = TIMEOUT_USE_SERVER_SETTING) {
        return gsIsSNValid(sn, timeout);
    }

  private:
    template <typename T>
    static void WINAPI s_isSNValidCB(bool valid, void *userData) {
        std::unique_ptr<TParamCopy<T>> param((TParamCopy<T> *)userData);
        param->_fcb(valid);
    }

  public:
    template <typename T>
    void isSNValid(const char *sn, T fcb, int timeout = TIMEOUT_USE_SERVER_SETTING) {
        gsIsSNValidAsync(sn, s_isSNValidCB<T>, new TParamCopy<T>(fcb), timeout);
    }

    //void isSNValid(const char* sn, testsn_cb callback, void * userData = NULL, int timeout = TIMEOUT_USE_SERVER_SETTING){
    //  gsIsSNValidAsync(sn, callback, userData, timeout);
    //}

    //Deactivate all entities
    void lockAllEntities();
    bool isAllEntitiesLocked() const;

    /**
      Revoke all serial numbers of an application

      IN: 
      
        snCompatible: [optional] serial number for pre-5.3.1 compatible usage.

        Before GS5.3.1, the serial number is not kept in local license data, so you must save it elsewhere and provide it as an external parameter.

        For NEW projects created since GS5.3.1, all applied serial numbers are persisted in local license storage so there is no need for a serial number.
      */
    bool revokeApp(const char *snCompatible = NULL) {
        return gsRevokeApp(TIMEOUT_WAIT_INFINITE, snCompatible);
    }

    //Gets the SN used to lock an entity
    const char *getSNByUnlockedEntityId(const char *entityId) {
        return gsGetSNByUnlockedEntityId(entityId);
    }

    //Get total serial numbers used to unlock entities
    int getTotalUnlockSNs() {
        return gsGetTotalUnlockSNs();
    }
    //Get individual unlock serial number by its index  ( 0 <= index < getTotalUnlockSNs() )
    const char *getUnlockSNByIndex(int index) {
        return gsGetUnlockSNByIndex(index);
    }

    //Get total entities unlocked by a sn
    int getTotalEntitiesUnlockedBySN(const char *sn) {
        return gsGetTotalEntitiesUnlockedBySN(sn);
    }

    //Get the entity id unlocked by a sn, ( 0 <= index < getTotalEntitiesUnlockedBySN(sn) )
    const char *getEntityIdUnlockedBySN(const char *sn, int index) {
        return gsGetEntityIdUnlockedBySN(sn, index);
    }

    //Get the prelimitary serial number which is used for license transfer (revoke & move) and error fix
    std::string getPreliminarySN() {
        return gsGetPreliminarySN();
    }

    //Revoke a single serial number, all those entities previously unlocked by this sn are locked
    bool revokeSN(const char *sn) {
        return gsRevokeSN(TIMEOUT_WAIT_INFINITE, sn);
    }

    /** \brief Create a new move package
    *
    *  \param mpDataStr the encrypted data string of a move package.
    *         if mpDataStr == NULL, then an empty move package is created
    */
    TMovePackage *createMovePackage(const char *mpDataStr = NULL) {
        TMPHandle hMP = (mpDataStr == NULL) ? gsMPCreate(0) : gsMPOpen(mpDataStr);
        return (hMP == NULL) ? NULL : new TMovePackage(hMP);
    }

    //Move the whole license via online license server
    //Return: on success, a non-empty receipt (SN) to activate app later on target machine
    std::string uploadApp(const char *preSN = NULL) {
        //make sure we have a valid preliminary serial number for online operation
        assert(preSN || gsMPCanPreliminarySNResolved(NULL));

        return gsMPUploadApp(preSN, TIMEOUT_WAIT_INFINITE);
    }

    //Move the whole license manually / offline
    //Return: on success, a non-empty encrypted string contains the current license data.
    std::string exportApp() {
        return gsMPExportApp();
    }

    //Code Exchange
    static TCodeExchange *beginCodeExchange() {
        TCodeExchangeHandle h = gsCodeExchangeBegin();
        if (h != NULL)
            return new TCodeExchange(h);
        return NULL;
    }
};

/**
  *  Built-In License Model Inspectors
  *
  *  An inspector encapsulates LM-specific functions into a helper class, otherwise you have to access a LM via TGSLicense ( a generic way)
  *
  */
class TLM_Inspector {
  protected:
    TGSLicense *_lic; //License being inspected
  public:
};

/// LM_Expire_Period Inspector
class TLM_Period : public TLM_Inspector {
};

//Debug Helper
//Ouput debug message
#if defined(DEBUG) || defined(_DEBUG)
void DebugMsg(const char *msg);
void DebugMsgFmt(const char *format, ...);

//Auto Debug >> << pairs
class TAutoDbg {
  private:
    std::string _msg;

  public:
    TAutoDbg(const char *format, ...);
    ~TAutoDbg();
};

#endif

}; // namespace gs

#if defined(DEBUG) || defined(_DEBUG)
#if defined(_MSC_VER)

#define LOG(fmt, ...) gs::DebugMsgFmt("%s, %s [Line %d]: " fmt, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#define LOG0(msg) gs::DebugMsgFmt("%s, %s [Line %d]: %s", __FUNCTION__, __FILE__, __LINE__, (msg))

#else

#define LOG(fmt, ...) gs::DebugMsgFmt("%s, %s [Line %d]: " fmt, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#define LOG0(msg) gs::DebugMsgFmt("%s, %s [Line %d]: %s", __PRETTY_FUNCTION__, __FILE__, __LINE__, (msg))

#endif
#else //Release

#ifdef _MSC_VER
#define _PT_SLASH(c) / ##c
//#define TRACE(arg...) false && __dummy
#define LOG _PT_SLASH(/)
#else
#define LOG(arg...)
#endif

#define LOG0(x) ((void)0)

#endif

#if defined(DEBUG) || defined(_DEBUG)

#ifdef _MSC_VER
#define AUTODBG(fmt, ...) gs::TAutoDbg __dbg##__LINE__("%s, %s [Line %d]: " fmt, __FUNCTION__, __FILE__, __LINE__, __VA_ARGS__)
#else
#define AUTODBG(fmt, ...) gs::TAutoDbg __dbg##__LINE__("%s, %s [Line %d]: " fmt, __PRETTY_FUNCTION__, __FILE__, __LINE__, ##__VA_ARGS__)
#endif

#else

#ifdef _MSC_VER
#define AUTODBG _PT_SLASH(/)
#else
#define AUTODBG(arg...) ((void)0)
#endif
#endif

//Tag the code scope
#define TAG_SCOPE AUTODBG("")

#endif //_GS5_H_
