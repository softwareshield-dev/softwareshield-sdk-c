/**
* \file GS5_Intf.h
* \brief gsCore C API Interface
*
*  These apis are flat-C style handle-based apis, OOP classes are recommended in GS5 extension development.
*
*/
#ifndef _GS5_INTF_H_
#define _GS5_INTF_H_

//VC2008 does not support C99 standard, uses file downloaded from https://code.google.com/p/msinttypes/
#include <cstdint>
#include <ctime>

#ifdef __GNUC__
#define DEPRECATED __attribute__((deprecated))
#elif defined(_MSC_VER)
#define DEPRECATED __declspec(deprecated)
#else
#pragma message("WARNING: You need to implement DEPRECATED for this compiler")
#define DEPRECATED
#endif

/// GameShield name space
namespace gs {

/// Invalid GS5 object handle is NULL
#define INVALID_GS_HANDLE NULL

/** @name Entity Status Attributes */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
/// Entity is currently accessible.
#define ENTITY_ATTRIBUTE_ACCESSIBLE 1
/// Entity's license is fully activated, no expire /trial limits at all.
#define ENTITY_ATTRIBUTE_UNLOCKED 2
/// Entity is active (being accessed via gsBeginAccessEntity())
#define ENTITY_ATTRIBUTE_ACCESSING 4
/// Entity is locked
#define ENTITY_ATTRIBUTE_LOCKED 8
/// Entity is auto-start
#define ENTITY_ATTRIBUTE_AUTOSTART 16
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

/** @name License Model Property Permission */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{
///the param is invisible from SDK apis
#define LM_PARAM_HIDDEN 1
///the param is not persistent (not saved in local license storage)
#define LM_PARAM_TEMP 2
///the param can read via SDK apis
#define LM_PARAM_READ 4
///the param can write via SDK apis
#define LM_PARAM_WRITE 8
///the param is inheritable (new build will inherit value from old build when license upgrading)
#define LM_PARAM_INHERIT 16
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}
//---------- Event IDs -------------
//
/** @name Application Events */
//@{{{{{{{{{{{
/// \anchor eventId
/// Application event id range: [0, 99]
#define EVENT_IDBASE_APPLICATION 0

///\brief Application just gets started, please initialize
///
/// When this event triggers, the local license has been initialized via gsInit().
///
#define EVENT_APP_BEGIN 1
///
///Application is going to terminate, last signal before game exits.
#define EVENT_APP_END 2
///Alarm: Application detects the clock is rolled back
#define EVENT_APP_CLOCK_ROLLBACK 3
///Fatal Error: Application integrity is corrupted.
#define EVENT_APP_INTEGRITY_CORRUPT 4
///Application starts to run, last signal before game code is executing
#define EVENT_APP_RUN 5
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

//ILdrCore [INTERNAL]
#define EVENT_PASS_BEGIN_RING1 20
#define EVENT_PASS_BEGIN_RING2 22

#define EVENT_PASS_END_RING1 21
#define EVENT_PASS_END_RING2 24

#define EVENT_PASS_CHANGE 23

/** @name License Events */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
//ILicenseDoc
/// License event id range: [100, 199]
#define EVENT_IDBASE_LICENSE 100
///Original license is uploaded to license store for the first time.
#define EVENT_LICENSE_NEWINSTALL 101
///The application's license store is connected /initialized successfully (gsCore::gsInit() == 0)
#define EVENT_LICENSE_READY 102

///The application's license store cannot be connected /initialized! (gsCore::gsInit() != 0)
#define EVENT_LICENSE_FAIL 103
///License is loading...
#define EVENT_LICENSE_LOADING 105
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

/** @name Entity Events */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
//IEntityCtl
/// Entity event id range [200, 299]
#define EVENT_IDBASE_ENTITY 200
/**
  * The entity is to be accessed.
  *
  * The listeners might be able to modify the license store here.
  * The internal licenses status are untouched. (inactive if not accessed before)
  */
#define EVENT_ENTITY_TRY_ACCESS 201

/**
  * The entity is being accessed.
  *
  * The listeners can enable any protected resources here. (inject decrypting keys, etc.)
  * The internal licenses status have changed to active mode.
  */
#define EVENT_ENTITY_ACCESS_STARTED 202

/**
  * The entity is leaving now.
  *
  * The listeners can revoke any protected resources here. (remove injected decrypting keys, etc.)
  * Licenses are still in active mode.
  */
#define EVENT_ENTITY_ACCESS_ENDING 203

/**
  * The entity is deactivated now.
  *
  * The listeners can revoke any protected resources here. (remove injected decrypting keys, etc.)
  * Licenses are kept in inactive mode.
  */
#define EVENT_ENTITY_ACCESS_ENDED 204

/// Alarm: Entity access invalid (due to expiration, etc)
#define EVENT_ENTITY_ACCESS_INVALID 205
/// Internal ping event indicating entity is still alive.
#define EVENT_ENTITY_ACCESS_HEARTBEAT 206

/**
  * \brief Action Applied to Entity
  * The status of attached licenses have been modified by applying license action.
  *
  * It is called after the change has been made.
  *
  */
#define EVENT_ENTITY_ACTION_APPLIED 208
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

/** Action IDs **/

/** @name Generic actions **/
//@{

/// Unlock entity / license
#define ACT_UNLOCK 1
/// Lock down entity / license
#define ACT_LOCK 2
//Sets a parameter's value
#define ACT_SET_PARAM 3
// Enables a parameter
#define ACT_ENABLE_PARAM 4
// Disable a parameter
#define ACT_DISABLE_PARAM 5
/// Enable Copy protection feature (NodeLock)
#define ACT_ENABLE_COPYPROTECTION 6
/// Disable Copy protection feature (NodeLock)
#define ACT_DISABLE_COPYPROTECTION 7

#define ACT_ENABLE_ALLEXPIRATION 8
#define ACT_DISABLE_ALLEXPIRATION 9
#define ACT_RESET_ALLEXPIRATION 10

/// Clean up local license storage
#define ACT_CLEAN 11
/// Dummy action, carry only client id
#define ACT_DUMMY 12

#define ACT_PUSH 13
#define ACT_PULL 14

/// Enable Demo Nag UI
#define ACT_NAG_ON 15
/// Disable Demo Nag UI
#define ACT_NAG_OFF 16
/// Activation Code can be used only once
#define ACT_ONE_SHOT 17
/// Activation Code has a shelf time
#define ACT_SHELFTIME 18

/// Error Fix
#define ACT_FP_FIX 19
#define ACT_FIX 19
/// Revoke local license
#define ACT_REVOKE 20
//@}

/** @name LM-specific actions **/
//@{
//LM.expire.accessTime
/// Increase /Decrease access time (LM.expire.accessTime)
#define ACT_ADD_ACCESSTIME 100
/// Sets access time (LM.expire.accessTime)
#define ACT_SET_ACCESSTIME 101

//LM.expire.hardDate
/// Sets start date (LM.expire.hardDate)
#define ACT_SET_STARTDATE 102
/// Sets end date (LM.expire.hardDate)
#define ACT_SET_ENDDATE 103

/// Sets maximum execution session time (LM.expire.sessionTime)
#define ACT_SET_SESSIONTIME 104

//LM.expire.period
/// Sets expire period (LM.expire.period)
#define ACT_SET_EXPIRE_PERIOD 105
/// Increases / Decreases expire period (LM.expire.period)
#define ACT_ADD_EXPIRE_PERIOD 106

//LM.expire.duration
/// Sets expire duration (LM.expire.duration)
#define ACT_SET_EXPIRE_DURATION 107
/// Increases / Decreases expire duration (LM.expire.duration)
#define ACT_ADD_EXPIRE_DURATION 108
//@}

/// GS5 Object handle
typedef void *gs_handle_t;
/// Entity Object handle
typedef gs_handle_t TEntityHandle;
/// License Object Handle
typedef gs_handle_t TLicenseHandle;
/// Variable / Parameter Object Handle
typedef gs_handle_t TVarHandle;
/// GS5 event monitor object handle
typedef gs_handle_t TMonitorHandle;
/// Action Object Handle
typedef gs_handle_t TActionHandle;
/// Request Object Handle
typedef gs_handle_t TRequestHandle;
/// Event Object Handle
typedef gs_handle_t TEventHandle;
/// Event Source Object Handle
typedef gs_handle_t TEventSourceHandle;

typedef gs_handle_t TMPHandle;
typedef gs_handle_t TCodeExchangeHandle;

/// Entity Id is a string
typedef const char *entity_id_t;
/// License Id is a string
typedef const char *license_id_t;
/// Action Id is a unsigned byte
typedef unsigned char action_id_t;
/// Variable TypeId is an int
typedef int var_type_t;

/** @name Virtual Machine Support **/
//@{
/// Virtual Machine Id Mask
typedef unsigned int vm_mask_t;

/// VMware (http://www.vmware.com/)
#define VM_VMware 0x01
/// Virtual PC (http://www.microsoft.com/windows/virtual-pc/)
#define VM_VirtualPC 0x02
/// VirtualBox (https://www.virtualbox.org/)
#define VM_VirtualBox 0x04
/// VMWARE Fusion
#define VM_Fusion 0x08
/// Parallels (http://www.parallels.com)
#define VM_Parallel 0x10
/// QEMU (http://www.qemu.org)
#define VM_QEMU 0x20
/**
  *	Test if the current process is runing inside a virtual machine.
  *
  *	@param vmask	mask of VM types
  *
  *		VM Supported :	VM_VMware (0x01), VM_VirtualPC (0x02),VM_VirtualBox (0x04), VM_Fusion (0x08), VM_Parallel (0x10), VM_QEMU (0x20)
  *
  *	@return	true if any of the VM types (or'ed in the mask) is detected.
  *
  *	It can be called before gsInit().
  *
  */
bool gsRunInsideVM(vm_mask_t vmask);
//@}

/** @name User Defined Variable */
//@{{{{
/// \anchor VarAttrMask

/// Variable is readable
#define VAR_ATTR_READ 0x01
/// Variable is writable
#define VAR_ATTR_WRITE 0x02
/// Variable is persisted to local storage
#define VAR_ATTR_PERSISTENT 0x04
/// Variable is secured in memory
#define VAR_ATTR_SECURE 0x08
/// Variable is persisted at server side
#define VAR_ATTR_REMOTE 0x10
/// Variable cannot be enumerted via apis
#define VAR_ATTR_HIDDEN 0x20
/// Variable is reserved for internal system usage
#define VAR_ATTR_SYSTEM 0x40

/** \brief User Defined Variable TypeId
  *
  * Ref: \ref varType \ref gs::gsAddVariable()
  * \anchor TVarType
  */
typedef enum {
    VAR_TYPE_INT = 7,     ///< 32-bit integer
    VAR_TYPE_INT64 = 8,   ///< 64-bit integer
    VAR_TYPE_FLOAT = 9,   ///< float
    VAR_TYPE_DOUBLE = 10, ///< double
    VAR_TYPE_BOOL = 11,   ///Boolean
    VAR_TYPE_STRING = 20, ///ansi-string
    VAR_TYPE_TIME = 30    /// UTC date time
} TVarType;

//@}}}}}}}}}

/// License Status
typedef enum {
    STATUS_INVALID = 0xFF, ///<The current status value is invalid
    STATUS_LOCKED = 0,   ///<isValid() always return false  the license is disabled permanently.
    STATUS_UNLOCKED = 1, ///<isValid() always return true, it happens when fully purchased.
    STATUS_ACTIVE = 2    ///<isValid() works by its own logic.
} TLicenseStatus;

/// Defines Event Type Base
typedef enum {
    EVENT_TYPE_APP = 0,          ///< Application Event Id Range: [0, 99)
    EVENT_TYPE_LICENSE = 100,    ///< License Event Id Range: [100, 199)
    EVENT_TYPE_ENTITY = 200,     ///< Entity Event Id Range: [200, 299)
    EVENT_TYPE_USER = 0x10000000 ///< User Defined Event Id Range: [0x10000000, 0xFFFFFFFF)
} TEventType;

///User defined event id must >= GS_USER_EVENT
#define GS_USER_EVENT 0x10000000

#ifndef WINAPI
#ifdef _MSC_VER
#define WINAPI __stdcall
#else
#define WINAPI
#endif
#endif

//[ INTERNAL ] Event callback (aka. gs5_monitor_ex in SDK 5.0.10)
typedef void(WINAPI *gs5_monitor_callback)(int evtId, TEventHandle hEvent, void *userData);

// INTERNAL: Custom License Model Callback
typedef bool(WINAPI *lm_isValid_callback)(void *usrData);
typedef void(WINAPI *lm_startAccess_callback)(void *usrData);
typedef void(WINAPI *lm_finishAccess_callback)(void *usrData);
typedef void(WINAPI *lm_onAction_callback)(TActionHandle hAction, void *usrData);

typedef void(WINAPI *lm_destroy_callback)(void *usrData);
typedef gs_handle_t(WINAPI *lm_create_callback)(void *usrData);

// the last api called to explicitly release the internal resources used by SDK
void sdk_finish();

/**
   * \brief One-time Initialization of gsCore
   *
   Runtime Initializer, always update local storage as needed. [Read & Write]

   It tries to search and load license in the following order:
   - Loads from local storage first;
   - Loads from embedded license data;
   - Loads from input license file;

   * \param productId The Product Unique Id.
   * 		Each product has an unique Id that can be used to identity the application on the remote server.
   * \param origLic The full path to the original license document.
   * 		This license file is compiled from the GameShield project IDE and defines the initial license status of the product.
   * 		The file will not be altered and can be deployed in any folder.
   * \param password The string key to decrypt the license document.
   * \param reserved Reserved parameter
   *
   * \return Returns 0 on success, otherwise non-zero, gsGetLastError() to get the error message.

   */

int gsInit(const char *productId, const char *origLic, const char *password, void *reserved);

/** \brief Loads from Local Storage (* Read Only*)

      As a read-only viewer, it tries to load from local storage only and won't update local storage.


      \param productId The Product Unique Id.
      Each product has an unique Id that can be used to identity the application on the remote server.
      \param password The string key to decrypt the license document.

      \return Returns 0 on success, otherwise non-zero, gsGetLastError() to get the error message.
      *
      */
/**
  * \brief Initialization of gsCore with in-memory license data
  *
  Runtime Initializer, always update local storage as needed. [Read & Write]

  It tries to search and load license in the following order:
  - Loads from local storage first;
  - Loads from embedded license data;
  - Loads from input license data;

  * \param productId The Product Unique Id.
  * 		Each product has an unique Id that can be used to identity the application on the remote server.
  * \param origLicData The license data in memory.
  * 		After a license file is compiled from the GameShield project IDE, it can be embedded in the original product's binaries
  *    or stored in encrypted format somewhere, when the app launching, the license data is retrieved in memory before passing to
  *    this api.
  * \param password The string key to decrypt the license document.
  * \param reserved Reserved parameter
  *
  * \return Returns 0 on success, otherwise non-zero, gsGetLastError() to get the error message.

  */

int gsInit(const char *productId, const unsigned char *origLicData, int licSize, const char *password, void *reserved);

/** \brief Finalize the gsCore
   *
   * When the application is terminating, call this api to cleanup the internal data resources.
   */
int gsCleanUp();
/// Get the current SDK version
const char *gsGetVersion();
/**
  *	\brief Close GS5 Object Handle
  *
  *  All handles from gsCore apis must be closed to release internal resources.
  *
  */
void gsCloseHandle(gs_handle_t handle);
/**
  *  \brief Save pending license changes in memory to local storage
  *
  *  Usually the changes in memory is saved to local storage periodically when game running, this api focce a saving immediately.
  */
void gsFlush();

/**
  *  \brief Get the last error message
  *
  *  When gsCore API returns, you can always retrieve the last error message for details if on error.
  */
const char *gsGetLastErrorMessage();
/**
  *  \brief Get the last error code
  *
  *  When gsCore API returns, you can always retrieve the last error code for details if on error.
  */
int gsGetLastErrorCode();

//[INTERNAL]
void gsSetLastErrorInfo(int errCode, const char *errMsg);

/**
  *	\brief Returns the Build Id of the running binary release
  *
  *   See: \ref BuildId
  */
int gsGetBuildId();
/**
  *  \brief Get Product Name
  *  See: \ref ProductName
  */
const char *gsGetProductName();
/**
  *  \brief Get Product Id
  *  See: \ref ProductId
  */
const char *gsGetProductId();

//Environment
/**
  *	\brief Test if the current process is running inside GS5 Ironwrapper runtime
  *
  *	It can be called before gsInit().
  *
  */
bool gsRunInWrappedMode();

/** @name Entity APIs */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
/**
   * \brief Get the total number of entities defined in the application's license file.
   * \return -1 if error occurs.
   */
int gsGetEntityCount();
/**
  *  \brief Get the entity object by index
  *
  * \param index index to the entity, range from 0 to *gsGetEntityCount()-1*
  * \return INVALID_GS_HANDLE if on error, otherwise the non-zero handle to the entity
  *
  * Returned handle must be closed by gsCloseHandle() when not needed.
  */
TEntityHandle gsOpenEntityByIndex(int index);
/**
  *  \brief Get the entity object by entity's unique id
  *
  * \param entityId unique id of the entity, \ref entityId
  * \return INVALID_GS_HANDLE if on error, otherwise the non-zero handle to the entity
  *
  * Returned handle must be closed by gsCloseHandle() when not needed.
  */
TEntityHandle gsOpenEntityById(entity_id_t entityId);
/**
  *  \brief Get the entity's current license status
  *
  *  \param hEntity Handle to the entity object
  *  \return \ref EntityAttr
  */
unsigned int gsGetEntityAttributes(TEntityHandle hEntity);
/**
  * \brief Get the entity's unique id
  *
  *  \param hEntity Handle to the entity object
  *  \return string entity unique id
  */
entity_id_t gsGetEntityId(TEntityHandle hEntity);
/**
  * \brief Get the entity's name
  *
  *  \param hEntity Handle to the entity object
  *  \return entity name
  */
const char *gsGetEntityName(TEntityHandle hEntity);
/**
  * \brief Get the entity's description
  *
  *  \param hEntity Handle to the entity object
  *  \return entity description
  */
const char *gsGetEntityDescription(TEntityHandle hEntity);

/**
   * \brief Try start accessing an entity.
   *
   * If an entity is accessible, all of the associated resources (files, keys, codes, etc.) can be legally used, otherwise
   * they cannot be accessed by the application.
   *
   * The api can be called recursively, and each call must be paired with a gsEndAccessEntity(). When the api is called for the first time
   * the event EVENT_ENTITY_TRY_ACCESS and EVENT_ENTITY_ACCESS_STARTED are posted.
   *
   * \param hEntity The handle to entity to be accessed
   * \return 	returns true if the entity is accessed successfully.
   returns false if:
   - Cannot access any entity when your game is wrapped by a *DEMO* version of GS5/IDE and the its demo license has expired;
   - Entity cannot be accessed due to its negative license feedback;

   */
bool gsBeginAccessEntity(TEntityHandle hEntity);
/**
    \brief Try end accessing an entity

    \param hEntity The handle to entity being accessed
    \return true on success, false if there is unexpected error occurs.

    This api must be paired with gsBeginAccessEntity(), if it is the last calling then event EVENT_ENTITY_ACCESS_ENDING and
    EVENT_ENTITY_ACCESS_ENDED will be posted.

    */
bool gsEndAccessEntity(TEntityHandle hEntity);
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

//License
/** @name License APIs */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
/// Gets the total number of licenses currently attached to an entity
int gsGetLicenseCount(TEntityHandle hEntity);
/** \brief Gets license object handle by its index
  *
  * \param hEntity the entity to be checked
  * \param index the license index of all attached licenses, ranges :[ 0, gsGetLicenseCount()-1 ]
  * \return the license object handle
  *
  * Already deprecated in 5.3, use gsOpenLicense() instead
  */
DEPRECATED TLicenseHandle gsOpenLicenseByIndex(TEntityHandle hEntity, int index);
/** \brief Gets license object handle by its id
  *
  * \param hEntity the entity to be checked
  * \param licenseId the license unique id
  * \return the license object handle
  *
  * Already deprecated in 5.3, use gsOpenLicense() instead
  */
DEPRECATED TLicenseHandle gsOpenLicenseById(TEntityHandle hEntity, license_id_t licenseId);

/// Gets the license object's license id
license_id_t gsGetLicenseId(TLicenseHandle hLicense);
/// Gets the license object's license name
const char *gsGetLicenseName(TLicenseHandle hLicense);
/// Gets the license object's license description
const char *gsGetLicenseDescription(TLicenseHandle hLicense);
/// Gets the license object's license status (ref: \ref LicenseStatus)
TLicenseStatus gsGetLicenseStatus(TLicenseHandle hLicense);
/// Is license currently valid?
bool gsIsLicenseValid(TLicenseHandle hLicense);
/// Get the entity object to which the license is attached.
TEntityHandle gsGetLicensedEntity(TLicenseHandle hLicense);
/*
  * Inspect the license model's parameters
  */
/// Get total number of parameters in a license.
int gsGetLicenseParamCount(TLicenseHandle hLicense);
/// Get the license parameter by its index, ranges [0, gsGetLicenseParamCount()-1 ]
TVarHandle gsGetLicenseParamByIndex(TLicenseHandle hLicense, int index);
/// Get the license parameter by its name.
TVarHandle gsGetLicenseParamByName(TLicenseHandle hLicense, const char *name);

//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

/**@name Action APIs */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
/*
   * Inspect license model's actions
   */
/**
  * \brief Gets total number of actions appliable to a license
  * Ref: \ref ActionInfo
  */
int gsGetActionInfoCount(TLicenseHandle hLicense);
/** \brief Gets action information by index (ref: \ref ActionInfo)

      \param hLicense License handle to be inspected
      \param index index of action information, range [0, getActionInfoCount()-1 ]
      \param[out] actionId output buffer to receive action id
      \return the action name
      */
const char *gsGetActionInfoByIndex(TLicenseHandle hLicense, int index, action_id_t *actionId);

/*
   *	Inspect an action
   */
/// Gets action name
const char *gsGetActionName(TActionHandle hAct);
/// Gets action unique id
action_id_t gsGetActionId(TActionHandle hAct);
/// Gets action description
const char *gsGetActionDescription(TActionHandle hAct);
/// Gets action what-to-do string (ref: \ref WhatToDoActionString "What to do action string")
const char *gsGetActionString(TActionHandle hAct);
/*
   * Inspect action's parameters
   */
/// Gets the total number of action parameters
int gsGetActionParamCount(TActionHandle hAct);
/**
    \brief Gets action parameter by its name

    \param hAct handle to action being inspected;
    \param paramName the string name of a action parameter.
    \return Variable handle on success, INVALID_GS_HANDLE if paramer not found
    */
TVarHandle gsGetActionParamByName(TActionHandle hAct, const char *paramName);
/**
    \brief Gets action parameter by its index

    \param hAct handle to action being inspected;
    \param index the index of parameter, range [0, gsGetActionParamCount()-1 ]
    \return Variable handle on success, INVALID_GS_HANDLE if paramer not found
    */
TVarHandle gsGetActionParamByIndex(TActionHandle hAct, int index);

//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}

//Variables
/** @name Variable / Parameter APIs */
//@{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{{
/**
  * \brief Adds a user defined variable
  *
  * \param varName variable name
  * \param varType \ref TVarType, ref: \ref varType "Variable Type"
  * \param attr variable attribute, ref: \ref varAttr "Variable Attribute"
  It is a combination of \ref VarAttrMask "Variable Attribute Mask".
  * \param initValStr  string representation of initial variable value
  * \return the handle to the created variable, or INVALID_GS_HANDLE on error.
  *
  */
TVarHandle gsAddVariable(const char *varName, TVarType varType, int attr, const char *initValStr);
/**
  * \brief Remove a user defined variable
  *
  * \param varName the name of variable to remove
  * \return true if the variable is removed, false if variable not found.
  */
bool gsRemoveVariable(const char *varName);
/**
  * \brief Gets a user defined variable
  *
  * \param varName the name of variable to retrieve
  * \return the handle to variable on success, INVALID_GS_HANDLE if variable not found.
  */
TVarHandle gsGetVariable(const char *varName);

/// Get total number of user defined variables
int gsGetTotalVariables();
/// Get user defined variable by its index
TVarHandle gsGetVariableByIndex(int index);
/// Get variable's name
const char *gsGetVariableName(TVarHandle hVar);
/// Get variable's type id (ref: \ref varType)
TVarType gsGetVariableType(TVarHandle hVar);
/// Is variable holds a valid value?
bool gsIsVariableValid(TVarHandle hVar);
/// Convert variable's type id to type string (ref: \ref varType)
const char *gsVariableTypeToString(var_type_t paramType);
/// Gets variable's attribute (ref: \ref varAttr)
int gsGetVariableAttr(TVarHandle hVar);
/// Converts from variable's attribute value to its string format (ref: \ref varAttr)
const char *gsVariableAttrToString(int attr, char *buf, int bufSize);
/// Converts from variable's attribute string to attribute value (ref: \ref varAttr)
int gsVariableAttrFromString(const char *attrStr);

//Value Get/Set
/**
  * \brief Returns param value as value string
  *
  *  \param hVar The param handle
  *
  *	@return If the function succeeds, the return value is the pointer to the value string representation.
  *			If the variable is not readable, this function returns empty string ("").
  */
const char *gsGetVariableValueAsString(TVarHandle hVar);
/**
  * \brief Sets the variable's value from a string
  *
  *  \param hVar The param handle
  *  \param valStr The param value in string format
  *	@return true on success, false if the variable is not writable, or there is a value conversion error.
  */
bool gsSetVariableValueFromString(TVarHandle hVar, const char *valStr);

/**
  * \brief Returns param value as a 32 bit integer
  *
  *  \param hVar The param handle
  *  \param[out] val Reference to integer receiving the result
  *	\return True on success, False if the variable is not readable, or there is a value conversion error.
  */
bool gsGetVariableValueAsInt(TVarHandle hVar, int &val);
/// Sets the variable's value from an integer
bool gsSetVariableValueFromInt(TVarHandle hVar, int val);

/**
  * \brief Returns param value as 64 bit integer
  *
  *  \param hVar The param handle
  *  \param[out] val Reference to integer receiving the result
  *	\return True on success, False if the variable is not readable, or there is a value conversion error.
  */
bool gsGetVariableValueAsInt64(TVarHandle hVar, int64_t &val);
/// Sets the variable's value from a 64 bit integer
bool gsSetVariableValueFromInt64(TVarHandle hVar, int64_t val);

/**
  * \brief Returns param value as float
  *
  *  \param hVar The param handle
  *  \param[out] val Reference to integer receiving the result
  *	\return True on success, False if the variable is not readable, or there is a value conversion error.
  */
bool gsGetVariableValueAsFloat(TVarHandle hVar, float &val);
/// Sets the variable's value from a float
bool gsSetVariableValueFromFloat(TVarHandle hVar, float val);

/**
  * \brief Returns param value as double
  *
  *  \param hVar The param handle
  *  \param[out] val Reference to integer receiving the result
  *	\return True on success, False if the variable is not readable, or there is a value conversion error.
  */
bool gsGetVariableValueAsDouble(TVarHandle hVar, double &val);
/// Sets the variable's value from a double
bool gsSetVariableValueFromDouble(TVarHandle hVar, double val);

/** \brief Gets the variable's value as a time_t value
   \param hVar handle to variable object
   \param[out] val reference to time_t data structure receiving the result
   \return true if on success, val holds the number of seconds since 00:00 hours, Jan 1, 1970 UTC (i.e., the current unix timestamp).
   return false if the variable does not hold a valid date time value. (\see gsIsVariableValid())
   */
bool gsGetVariableValueAsTime(TVarHandle hVar, time_t &val);
/// Sets the variable's value from a time_t value
bool gsSetVariableValueFromTime(TVarHandle hVar, time_t val);
//@}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}
/** @name Request APIs */
//@{{{{{{{{
///Create a request object
TRequestHandle gsCreateRequest();
/** \brief Create and add an action to the request.
   *
   *  \param hReq The request handle the action is added to
   *  \param actId The action type id
   *  \param hLic The target license the action is apply to, NULL if a global action (apply to all entities/licenses)
   *
   *  @return The action handle, 0 if the action type is not supported.
   */
TActionHandle gsAddRequestAction(TRequestHandle hReq, action_id_t actId, TLicenseHandle hLic);
/** \brief Create and add an action to the request.
   *
   *  \param hReq The request handle the action is added to
   *  \param actId The action type id
   *  \param (entityId, licenseId) The target license(s) the action is apply to, (NULL, NULL) for a global action, (entityId, NULL) specify action is applied to all licenses associated to the entity
   *
   *  @return The action handle, 0 if the action type is not supported.
   */
TActionHandle gsAddRequestActionEx(TRequestHandle hReq, action_id_t actId, const char *entityId, const char *licenseId);
/// Generates the request code from the request object
const char *gsGetRequestCode(TRequestHandle hReq);
/// Applys license code (aka. Activation Code)
bool gsApplyLicenseCode(const char *activationCode);
//@}}}}}}}}}}
//---------- Time Engine Service ------------
bool gsIsInternalTimerActive();
void gsTurnOnInternalTimer();
void gsTurnOffInternalTimer();

void gsTickFromExternalTimer();
void gsPauseTimeEngine();
void gsResumeTimeEngine();
bool gsIsTimeEngineActive();
//Monitor
TMonitorHandle gsCreateMonitorEx(gs5_monitor_callback cbMonitor, void *usrData, const char *monitorName);
int gsGetEventId(TEventHandle hEvent);
TEventType gsGetEventType(TEventHandle hEvent);
TEventSourceHandle gsGetEventSource(TEventHandle hEvent);

//HTML
/** @name HTML Render APIs */
//@{{{{{{{
/**
    \brief Rendering HTML page in process.

    \param url  URL to html local file or web site page to render;
    \param title The caption of form window rendering the HTML page;
    \param width Pixel width of HTML page;
    \param height Pixel height of HTML page;

    It can be called *before* gsInit() to render generic HTML pages.
    However, gsInit() must be called before to render LMApp HTML pages.

    The default behavior is:
    - Windows Resizable = True;
    - ExitAppAfterUI = False;
    - CleanUpAfterRender = False;

    \see gsRenderHTMLEx
    */

bool gsRenderHTML(const char *url, const char *title, int width, int height);
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
bool gsRenderHTMLEx(const char *url, const char *title, int width, int height, bool resizable, bool exitAppWhenUIClosed, bool cleanUpAfterRendering);
//@}}}}}

/** @name Debug Helpers */
//@{
/// Is the SDK binary a DEBUG version?
bool gsIsDebugVersion();
/**
  *
  * \brief Output debug message
  *
  *  For SDK/Debug version, the message is appended to the current debug log file.
  *  For SDK/Release version, the message is displayed in: (1) DebugViewer (Windows via OutputDebugString); (2) Console (Unix via printf)
  *
  */

void gsTrace(const char *msg);
//@}

/** @name Application Control APIs */
//@{
/// Exit application gracefully
void gsExitApp(int rc);
/// Exit application forcefully
void gsTerminateApp(int rc);
/// Continue running the application
void gsPlayApp();

/// Pauses the application
void gsPauseApp();
/// Resume a paused application and quit immediately
void gsResumeAndExitApp();

/// Restart the current application
void gsRestartApp();
/// Is the current application a restarted session?
bool gsIsRestartedApp();
//@}
/// Gets the root path of the application
const char *gsGetAppRootPath();
/// Gets the startup game exe commandline
const char *gsGetAppCommandLine();
/// Gets the full path to the startup game exe.
const char *gsGetAppMainExe();

/** @name Session Variables APIs */
//@{
/// Sets the value of an application session variable
void gsSetAppVar(const char *name, const char *val);
/// Gets the value of an application session variable
const char *gsGetAppVar(const char *name);
//@}
//Custom LM
TLicenseHandle gsCreateCustomLicense(const char *licId, const char *licName, const char *description, void *usrData,
                                     lm_isValid_callback cbIsValid, lm_startAccess_callback cbStartAccess,
                                     lm_finishAccess_callback cbFinishAccess, lm_onAction_callback cbOnAction,
                                     lm_destroy_callback cbDestroy);

bool gsBindLicense(TEntityHandle hEntity, TLicenseHandle hLic);

TLicenseHandle gsCreateLicense(const char *licId);

void gsRegisterCustomLicense(const char *licId, lm_create_callback createLM, void *usrData);

void gsAddLicenseParamStr(TLicenseHandle hLic, const char *paramName, const char *initValue, int permission);
void gsAddLicenseParamInt(TLicenseHandle hLic, const char *paramName, int initValue, int permission);
void gsAddLicenseParamInt64(TLicenseHandle hLic, const char *paramName, int64_t initValue, int permission);
void gsAddLicenseParamBool(TLicenseHandle hLic, const char *paramName, bool initValue, int permission);
void gsAddLicenseParamFloat(TLicenseHandle hLic, const char *paramName, float initValue, int permission);
void gsAddLicenseParamDouble(TLicenseHandle hLic, const char *paramName, double initValue, int permission);
void gsAddLicenseParamTime(TLicenseHandle hLic, const char *paramName, time_t initValue, int permission);

//-------- Game Execution Context -----------
bool gsIsFirstPass();
bool gsIsGamePass();
bool gsIsLastPass();
bool gsIsFirstGameExe();
bool gsIsLastGameExe();
bool gsIsMainThread();

//----------- User Defined Event ------------
/** @name User Defined Event APIs */
//@{
/**\brief Post User Event
  *
  * \param eventId User defined event id ( must >= GS_USER_EVENT )
  * \param bSync true if event is posted synchronized, the api returns after the event has been parsed by all event handlers.
  otherwise the api returns immediately.
  * \param eventData [Optional] data buffer pointer associated with the event, NULL if no event data
  * \param eventDataSize size of event data buffer, ignored if \a eventData is NULL
  *
  * \return none
  */
void gsPostUserEvent(unsigned int eventId, bool bSync, void *eventData, unsigned int eventDataSize);
/** \brief Gets user defined event data information
  *
  * \param hEvent The handle to user event
  * \param[out] evtDataSize output inter receiving the length of event data
  * \return Pointer to user defined event data
  */
void *gsGetUserEventData(TEventHandle hEvent, unsigned int *evtDataSize);
//@}

/** \brief Node Lock ( Copy Protection )
  */
bool gsIsNodeLocked();
bool gsIsFingerPrintMatched();
const char *gsGetUniqueNodeId();

bool gsIsAppFirstLaunched();

/** @name Online Activation APIs */
//@{

typedef void(WINAPI *ping_cb)(bool serverOk, void *userData);
typedef void(WINAPI *testsn_cb)(bool snValid, void *userData);
typedef void(WINAPI *activate_cb)(const char *sn, bool success, int rc, const char *snRef, void *userData);

/**
    Test if the CheckPoint server is available.
    */
bool gsIsServerAlive(int timeout);
/**
    Test if the CheckPoint server is available (Async version)
    */
void gsIsServerAliveAsync(ping_cb pcb, void *userData, int timeout);

/**
    Test if a serial number is valid (exists and not deleted)
    */
bool gsIsSNValid(const char *sn, int timeout);
/**
    Test if a serial number is valid (exists and not deleted, Async version)
    */
void gsIsSNValidAsync(const char *sn, testsn_cb cb, void *userData, int timeout);

/**
    Apply a serial number to App

    \return true on success, false on failure
    */
bool gsApplySN(const char *sn, int * /* out */ pRetCode, const char ** /* out*/ ppSNRef, int timeout);

/**
    Applys a serial number to App (Async version)
    */
void gsApplySNAsync(const char *sn, activate_cb activateCB, void *userData, int timeout);

//@}

/**
   Test if an entity has a bundled license
   */
bool gsHasLicense(TEntityHandle hEntity);

/**
    Open the Entity's bundled license
    */
TLicenseHandle gsOpenLicense(TEntityHandle hEntity);

/**
    Lock the license
    */
void gsLockLicense(TLicenseHandle hLic);

//@{
/******************************************************
    Transfer
    ******************************************************/
const char *gsGetPreliminarySN();                                 //155
int gsGetTotalUnlockSNs();                                        //17
const char *gsGetUnlockSNByIndex(int index);                      //49
int gsGetTotalEntitiesUnlockedBySN(const char *sn);               //142
const char *gsGetEntityIdUnlockedBySN(const char *sn, int index); //143

const char *gsGetSNByUnlockedEntityId(const char *entityId); //154

//;;;;;;;;;;;;;; REVOKE ;;;;;;;;;;;;;;;;
/**
    Revoke all used SNs of an application
    */
bool gsRevokeApp(int timeout, const char *sn); //135

bool gsRevokeSN(int timeout, const char *sn); //144

//;;;;;;;;;;;;;;;; MOVE ;;;;;;;;;;;;;;;;
TMPHandle gsMPCreate(int reserved);
void gsMPAddEntity(TMPHandle hMP, const char *entityId);
const char *gsMPExport(TMPHandle hMP);
const char *gsMPUpload(TMPHandle hMP, const char *sn, int timeout);
bool gsMPIsTooBigToUpload(TMPHandle hMP);

TMPHandle gsMPOpen(const char *mpStr);

bool gsMPCanPreliminarySNResolved(TMPHandle hMP);                  //156
bool gsMPImportOnline(TMPHandle hMP, const char *sn, int timeout); //141

const char *gsMPGetImportOfflineRequestCode(TMPHandle hMP); // 150
bool gsMPImportOffline(TMPHandle hMP, const char *licenseCode);
const char *gsMPUploadApp(const char *sn, int timeout);
const char *gsMPExportApp();

//new api (158)
bool gsApplyLicenseCodeEx(const char *licenseCode, const char *sn, const char *snRef);

TCodeExchangeHandle gsCodeExchangeBegin();
const char *gsCodeExchangeGetLicenseCode(gs_handle_t hCodeExchange, const char *productId, int buildId, const char *sn, const char *requestCode);
int gsCodeExchangeGetErrorCode(gs_handle_t hCodeExchange);
const char *gsCodeExchangeGetErrorMessage(gs_handle_t hCodeExchange);
//@}

}; // namespace gs
#endif
