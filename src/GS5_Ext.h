/*! \file GS5_Ext.h
	\brief GS5 Extension Development

	This file is needed to develop your own GS5 extensions such as a license model or GS5 event monitor. 
*/

#ifndef _GS5_EXT_H_
#define _GS5_EXT_H_

#include "GS5.h"

#include <memory>

namespace gs {

typedef class TGSApp *(*f_createApp)(void);

/** 
*  \brief GS5 Application 
*
*  The base class to:
*   - monitor GS events;
*   - query and control application execution; 
*
*  To monitor GS events, a subclass must override interested event handlers. 
*  
*  uses macro \ref DECLARE_APP in class declaration and \ref IMPLEMENT_APP in cpp file to register the subclass. 
*  uses macro \ref GET_APP to retrieve the single application instance.
*
*  \code
       in MyApp.h:    
*        class TMyApp : public TGSApp {
            DECLARE_APP(TMyApp);
			private:
				virtual void OnAppBegin(){ 
				    //App launches, say hello...
				}
		  };

	    in MyApp.cpp:
		 
		   IMPLEMENT_APP(TMyApp);

		   int main(){
		        TMyApp * app = GET_APP(TMyApp);
				...
		   }

*       
*  \endcode
*
*/
class TGSApp {
  private:
    TGSCore *_core; ///< local reference to TGSCore single instance
    static f_createApp s_appCreator;

    static TGSApp *s_app;
    static TGSApp *s_createApp();

    static void s_appEventCB(unsigned int eventId, void *usrData);
    static void s_licEventCB(unsigned int eventId, void *usrData);
    static void s_entityEventCB(unsigned int eventId, TGSEntity *entity, void *usrData);
    static void s_userEventCB(unsigned int eventId, void *eventData, unsigned int eventDataSize, void *usrData);
    //Initialize for every game passes
    void init();
    void registerLicenseModels();

    //[INTERNAL]
    void OnPassBegin(int ring);
    void OnPassEnd(int ring);

  protected:
    /**
	*  \brief Application Initialization
	*    Callback to let the application initialize itself 
	*
	*  \return TRUE if application is initialized correctly, FALSE if something fatal happens and the game will terminate immediately. 
	*
	*  It is called after TGSCore has been initialized. 
	*  
	*/
    virtual bool OnAppInit();

    /** @name Generic Event Handlers */
    ///@{
    /**
	*  \brief Generic Application Events Handler
	*  
	*  \param evtId Application Event Identifier
	*  
	*  The method parses the event id and invokes the corresponding event handlers.
	*  It is recommended that subclass override individual event handlers instead of this one.

	  \see OnAppBegin 
	  \see OnAppRun
	  \see OnAppEnd
	*/
    virtual void OnAppEvent(unsigned int evtId);

    /**
	*  \brief Generic License Events Handler
	*  
	*  \param evtId License Event Identifier
	*  
	*  The method parses the event id and invokes the corresponding event handlers.
	*  It is recommended that subclass override individual event handlers instead of this one.
	*
	*
	*/
    virtual void OnLicenseEvent(unsigned int evtId);
    /**
	*  \brief Generic Entity Events Handler
	*  
	*  \param evtId Entity Event Identifier
	*  \param entity The source entity triggering this event
	*  
	*  The method parses the event id and invokes the corresponding event handlers.
	*  It is recommended that subclass override individual event handlers instead of this one.
	*/
    virtual void OnEntityEvent(unsigned int evtId, TGSEntity *entity);
    /**
	* \brief User Event Handker
	*
	* \param eventId  User defined event id ( >= GS_USER_EVENT)
	* \param eventData Pointer to event data buffer, NULL if no event data assoicated;
	* \param eventDataSize Length of event data buffer
	*
	*  The subclass can override this method to handle user defined event
	*  
	*/
    virtual void OnUserEvent(unsigned int eventId, void *eventData, unsigned int eventDataSize) {}
    ///@}

    /** @name Application Event Handlers */
    ///@{

    /**
	*  \brief Application Event Handler: Game Starts (EVENT_APP_BEGIN)
	*
	*  Called when the game starts and the license has been initialized. The default method does nothing.
	*
	*  LMApp can check the current license status and pop up UI if necessary. If the product has been fully activated,
	*  you might simply bypass the startup LMApp UI.
	*/
    virtual void OnAppBegin(); //EVENT_APP_BEGIN
                               /**
	*  \brief Application Event Handler: Game Runs (EVENT_APP_RUN)
	*
	*  Called when the game's original code starts to run. The default method does nothing.
	*
	*  You can start timing or initialize whatever logic needed while game is running.
	*/
    virtual void OnAppRun();   //EVENT_APP_RUN

    /**
	*  \brief Application Event Handler: Game Exits (EVENT_APP_END)
	*
	*  Called when the game is terminating. The default method does nothing.
	*
	*  The local license storage is not closed (gsCleanUp is not called) yet and it is a good chance to pops up LMApp exit UI if game not activated.
	*
	*  
	*/
    virtual void OnAppEnd();          //EVENT_APP_END
                                      /**
	*  \brief Application Event Handler: Clock Rollback Detected (EVENT_APP_CLOCK_ROLLBACK)
	*
	*  Called when a clock rollback behavior has been detected. The default method does nothing.
	*
	*  A clock rollback might be detected if the local machine date time is modified backward manually. Some built-in License Models (LM_Expire_HardDate, LM_Expire_Period) use the   
	*  local clock time, for them there is a built-in LM parameter called "rollbackTolerance" to define the maximum time difference tolerable without trigger the clock rollback event.
	*  
	*/
    virtual void OnClockRolledBack(); //EVENT_APP_CLOCK_ROLLBACK

    /**
	*  \brief Application Event Handler: Clock Rollback Detected (EVENT_APP_INTEGRITY_CORRUPT)
	*
	*  Called when a game integrity corruption has been detected. The default method does nothing.
	*
	   Integrity corruption might be:
	   - Content of external files are modified (compared to the version when project wrapping);
	   - Game binaries has been modified in memory;
	*  
	*/
    virtual void OnIntegrityCorrupted(); //EVENT_APP_INTEGRITY_CORRUPT

    ///@}

    /** @name License Event Handlers **/
    ///@{

    /**
	*  \brief License Event Handler:  (EVENT_LICENSE_NEWINSTALL)
	*
	*  Called when a game launches for the first time on the local machine.
	*
	*	This event is triggered before OnAppBegin() when the local license storage is being initialized by gsCore::init(),
	*	It happens only once for the very first launching of the game. The default method does nothing.
	*/
    virtual void OnNewInstall();     //EVENT_LICENSE_NEWINSTALL
                                     /**
	*  \brief License Event Handler:  (EVENT_LICENSE_LOADING)
	*
	*  Called when the game's license is being loaded from local storage.
	*
	*   It gives you a chance to register any used custom license models from external dlls, because the license data might contain
	*   custom LM status and the internal license model factory has to create a LM instance to parse/deserialize the custom LM data.
	*	The default method does nothing.
	*/
    virtual void OnLicenseLoading(); //EVENT_LICENSE_LOADING
                                     /**
	*  \brief License Event Handler:  (EVENT_LICENSE_READY)
	*
	*  Called when the game's license has been loaded successfully from local storage.
	*
	*	The default method does nothing.
	*/
    virtual void OnLicenseLoaded();  //EVENT_LICENSE_READY
                                     /**
	*  \brief License Event Handler:  (EVENT_LICENSE_FAIL)
	*
	*  Called when the game's license cannot be loaded from local storage.
	*
	*  It is a fatal error, and the most of gsCore apis are not usable at this point. 
	*
	*  The reason of a license loading failure might be:
	   - License data corruption due to media error or logic error;
	   - Hard disk permission error;

	   GS5 has tried hard to minimize the chances of license loading failure by using redundunt storage and implementing transaction algorithm. 

	*	The default method does nothing.
	*/
    virtual void OnLicenseFail();    //EVENT_LICENSE_FAIL

    ///@}

    /** @name Entity Event Handlers **/
    ///@{

    /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_TRY_ACCESS)
	*
	*  Called when an entity is going to be accessed. (@see gsBeginAccess) 
	*
	*   \param entity The entity to be accessed
	*
	*   It is a great chance to prepare all resources needed by the entity logic. If currently the entity is not activated yet,  
	*   you can pop up reminder UI to prompt user activate it. 
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityAccessStarting(TGSEntity *entity); //EVENT_ENTITY_TRY_ACCESS
                                                            /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACCESS_STARTED)
	*
	*  Called when an entity is accessed successfully. (@see gsBeginAccess) 
	*
	*   \param entity The entity to be accessed
	*
	*   If an entity is accessible, the gsBeginAccess() will initialize internal data structures for this entity, and before returning triggers 
	*   this event telling you that the entity enters "Active" status.
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityAccessStarted(TGSEntity *entity);  //EVENT_ENTITY_ACCESS_STARTED
                                                            /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACCESS_ENDING)
	*
	*  Called when an entity accessing is going to an end. (@see gsEndAccess) 
	*
	*   \param entity The entity to be accessed
	*
	*   If an entity will not be used any more, the gsEndAccess() will destroy its internal data structures, and before api returning it triggers 
	*   this event telling you that the entity is leaving the "Active" status.
	*
	*   It is a great chance to release any resources used by the entity. For example, the encryption keys are destroyed so that the files associated 
	*   with the entity cannot be accessed by game.
	*
	* 
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityAccessEnding(TGSEntity *entity);   //EVENT_ENTITY_ACCESS_ENDING
                                                            /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACCESS_ENDED)
	*
	*  Called when an entity accessing is over. (@see gsEndAccess) 
	*
	*   \param entity The entity to be accessed
	*
	*   If an entity will not be used any more, the gsEndAccess() will destroy its internal data structures, and before api returning it triggers 
	*   this event telling you that the entity has left the "Active" status and becomes "In-Active" now.
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityAccessEnded(TGSEntity *entity);    //EVENT_ENTITY_ACCESS_ENDED

    /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACCESS_INVALID)
	*
	*  Called when an entity accessing becomes invalid.
	*
	*   \param entity The entity to be accessed
	*   \param inGame If the event is triggered while game running.
	*
	*  For some reason, if the license attached to an active entity (entity being accessed) becomes invalid (TGSLicense::isValid() returns false), 
	*  the entity triggers this error event telling you that the entity should not be accessed any more. 
	*
	*  It is just like the car driving, the green traffic light stands for the positive license status, when license status changes to invalid at run time,   
	*  you will see a red traffic light keeps flashing ahead, so the correct response is braking the car and stops as quick as possible.
	*
	*  The default GS5 behavior is terminating the game immediately and pops up the LMApp exit UI optionally in the last game pass (Execution mode: P1S2, P1S3).
	*  You can change the default behavior by setting one of the built-in LM's parameter "exitAppOnExpire" to <b>false</b>, as a result the game won't be stopped
	*  automatically by GS5 kernel.
	*  
	*  This event handler gives you a chance to prompt the user to activate the game. However, be aware that when game running the event might be triggered from a non-main thread,
	*  rendering a UI in non-main thread is not a easy job, that is why the method has a *inGame* parameter, if you want to activate an entity inside the game,
	*  just do it properly, if it is ok for you to activate an entity after game terminates, inGame can make your job easier:
	*
	*  \code
	*       void TMyApp::OnEntityAccessInvalid(TGSEntity* entity, bool inGame)
		    {
			    if(inGame){
				     //In-Game UI Rendering...
					 if(isMainThread()){
						//Render in main thread
					 }else{
						//in non-main thread, be careful!!!
					 }
				}else{
				     //outside game, rendering UI in main thread
				}
			}
		\endcode
	*      
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityAccessInvalid(TGSEntity *entity, bool inGame); //EVENT_ENTITY_ACCESS_INVALID
                                                                        /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACCESS_HEARTBEAT)
	*
	*  Called periodically while an entity is being accessed (in Active status). 
	*
	*   \param entity The entity to be accessed
	*
	*   This event can be used as a timer.
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityHeartBeat(TGSEntity *entity);                  //EVENT_ENTITY_ACCESS_HEARTBEAT
                                                                        /**
	*  \brief Entity Event Handler:  (EVENT_ENTITY_ACTION_APPLIED)
	*
	*  Called when an entity's license status is modified by an action. 
	*
	*   \param entity The entity to be modified
	*
	*  When applying activation code, the gsApplyLicenseCode() triggers this event after the action is applied to an entity.
	*
	*  You can use this event handler to update the LMApp UI to reflect the latest license status. 
	*
	*	The default method does nothing.
	*/
    virtual void OnEntityActionApplied(TGSEntity *entity);              //EVENT_ENTITY_ACTION_APPLIED
    ///@}

    /**
	*  \brief Constructor
	*    Protected constructor to avoid creating TGSApp instance from user code directly. 
	*/
    TGSApp();
    virtual ~TGSApp();

  public:
    static TGSApp *getInstance();

    static void registerApp(f_createApp appCreator);

    //------- App Control --------
    /** @name Application Control **/
    //@{
    /**
	* \brief Exits application gracefully
	* \param rc Exit Code
	*  
	* Stops the game gracefully, the LMApp will get a chance (optionally in the last different game pass for P1S2, P1S3 mode) to render its Exit-UI later.   
	*/
    void exitApp(int rc);
    /**
	* \brief Terminates application forcefully
	* \param rc Exit Code
	*  
	* Stops the game forcefully; the game terminates immediately without giving a chance to pop up LMApp's Exit-UI, 
	*/
    void terminateApp(int rc);
    /**
	* \brief Allow the game code starts to play
	* 
	* Indicates to the GS5 kernel that the game code can be executed right now. 
	*/
    void playApp();
    /**
	* \brief Replay the game
	* 
	* Restarts the game manually from your code, usually after the game has been activated in your LMApp Exit-UI.
	*
	* When game restarting, you can improve game player's user experience by bypassing LMApp startup UI and goes directly to the game pass.
	* \see isRestartedApp()
	*/
    void restartApp();
    /**
	* \brief Pause the game [ Experimental, Windows Only ]
	* 
	* Pauses game's thread and hide all its top windows.
	*
	* It provides the License model developer a chance to pop up UI while game is running / terminating.
	*
	* \see resumeAndExitApp()
	*/
    void pauseApp();
    /**
	* \brief Resume a paused game and terminate.  [ Experimental, Windows Only ]
	* 
	* 
	*
	* 
	* \see pauseApp()
	*/
    void resumeAndExitApp();

    //@}

    /** @name Application Running Context Query **/
    //@{

    /**
	* \brief Is First Game Pass?
	*
	* \return true if the current pass is the first pass.
	*
	* Ref: \ref ExecutionMode
	*/
    bool isFirstPass();
    /**
	* \brief Is First Game Pass?
	*
	* \return true if the current pass is running the game code
	*
	* Ref: \ref ExecutionMode
	*/
    bool isGamePass();
    /**
	* \brief Is Last Pass?
	*
	* \return true if the current pass is the last game pass.
	*
	* Ref: \ref ExecutionMode
	*/
    bool isLastPass();

    /**
	*  \brief Is the current process exe is the first game exe?
	*  
	*  OnAppBegin() is called only when isFirstGameExe() returns true.
	*
	*  Ref: \ref Exe-Hopping
	*/
    bool isFirstGameExe();
    /**
	*  \brief Is the current process exe is the last game exe?
	*  
	*  OnAppEnd() is called only when isLastGameExe() returns true.
	*  
	*
	*  Ref: \ref Exe-Hopping
	*/
    bool isLastGameExe();

    /**
	*  \brief Is the current thread the main thread?
	*  
	*  \see OnEntityAccessInvalid
	*/
    bool isMainThread();
    /**
	*  \brief Is the current process a restarted one?
	*  
	*  \see restartApp()
	*/
    bool isRestartedApp();

    /**
  * \brief Is the application is launched for the first time ? (Virginal Running)
  *
  *  Returns true only if the application is launched for the very first time after 
  *  installation.
  *
  */
    bool isFirstLaunched() { return _core->isAppFirstLaunched(); }
    //@}

    /**
	* \brief Gets application root directory
	*/
    const char *getAppRootPath();
    /**
	* \brief Gets application startup commandline
	*
	*  The startup exe of a \ref Exe-Hopping game might be different from the current exe; this function returns the 
	*  original command line launching the game.
	*/
    const char *getAppCommandLine();
    /**
	* \brief Gets full path to the application startup Exe
	*
	*  The startup exe of a \ref Exe-Hopping game might be different from the current exe. it is the first exe started when game launching.
	* 
	*  Instead of hard-coding an icon, your custom LMApp may want to extract icon from game's main exe and display it in UI, which make your LM 
	*  more reusable.
	*
	*  \code
		HICON hAppIcon = ExtractIcon(GetModuleHandle(NULL), getAppMainExe(), 0);
	*  \endcode
	*/
    const char *getAppMainExe();

    /** @name App Session Variables **/
    //@{
    /**
	* \brief Write session variable
	*
	* Ref: \ref AppVar
	*/
    void setSessionVar(const char *name, const char *val);
    /**
	* \brief Read session variable
	*
	* Ref: \ref AppVar
	*/
    const char *getSessionVar(const char *name);
    //@}

    /**
	* \brief Gets Application Title
	*
	* \return The game title defined in license project file.  
	*
	* This is a simple helper to return TGSCore::productName().
	* The game title can be used in LMApp UI. 
	*/
    const char *getGameTitle(); //Game Iitle

    /** \brief Send User Defined Event (Synchronized event posting)
	*
	* \param eventId User defined event id ( must >= GS_USER_EVENT )  
	* \param eventData [Optional] data buffer pointer associated with the event, NULL if no event data   
	* \param eventDataSize size of event data buffer, ignored if \a eventData is NULL
	*
	* \return none
	*/
    void sendUserEvent(unsigned int eventId, void *eventData = NULL, unsigned int eventDataSize = 0);

    /**
	* \brief Gets pointer to TGSCore instance 
	* 
	*  It is a helper function retrieving the private class member \a _core, it is the same as TGSCore::getInstance()
	*/
    TGSCore *core() { return _core; }
};
/** @name APP Macros
*  
*/
///@{
/**\def DECLARE_APP(clsName)
*  \brief Declare a TGSApp subclass
*  
*  It defines needed class members for an app subclass and must be put in the class declaration as following:

	\code
		class TMyApp : public gs::TGSApp{
		  DECLARE_APP(TMyApp);
		};
	\endcode

	\see IMPLEMENT_APP \see GET_APP
*/
#define DECLARE_APP(clsName)                                  \
  private:                                                    \
    static TGSApp *createInstance() { return new clsName(); } \
                                                              \
  public:                                                     \
    static void initClass() {                                 \
        gs::TGSApp::registerApp(clsName::createInstance);     \
    }

/**
*  \brief Implements a TGSApp subclass
* 
*  It implements needed class members for an app subclass and must be put in a cpp file as following:
*  
	\code
	 in myapp.cpp:

		IMPLEMENT_APP(TMyApp);

		void TMyApp::OnAppBegin(){ ... }
	\endcode

	\see DECLARE_APP, \see GET_APP
*/
#define IMPLEMENT_APP(clsName)                        \
    namespace clsName_ {                              \
    struct clsName##Registor {                        \
        clsName##Registor() { clsName::initClass(); } \
    } clsName##Registor_inst;                         \
    }

/**
* \brief Get TGSApp instance
*
*  Get the single instance of registered TGSApp class.
*
*  Because it is recommended that the constructor of TGSApp and its subclass is protected, this macro is the official way of creating / retrieving a pointer to TGSApp (or its subclass) 
*
*  The reason is for code portability: The same binary can be executed without wrapping or embedded in protected game. 

  \see DECLARE_APP 
  \see IMPLEMENT_APP
*/
#define GET_APP(clsName) ((clsName *)TGSApp::getInstance())

/**
* \brief Initialize TGSApp Instance
* 
*  For compatibility it must be called once before other GSApp-apis.
*
*  If the SDK is built as part of head exe / dll, the GS5_Entry() in GS5_Ext.cpp won't be called automatically by GS5 loader (gsLoader)
* so we must call this api manually during application initialization (within DLLMain or WinMain/main, etc.).
*
*  If the SDK is built as part of non-head dll, the GS5_Entry() in GS5_Ext.cpp will be called automatically by GS5 loader (gsLoader) so
*  calling this api is optional (do nothing if called in this case).
*
*  For all senerios, calling initApp() to initialize the TGSApp (and its derived app class) is recommended.
*
*/
void initApp();

#define GS_INIT_APP \
    { gs::initApp(); }

///@}

// ---------------------------- Dynamic LM ----------------------------------

/**
* \brief Base class of Dynamic License Model
*
*  Privodes all built-in facilities for DLM development
*
*  All dynamic license model should subclass this class.
*
*  Subclass of TGSDynamicLM should use \ref DECLARE_LM to define its unique identifier in class declaration, 
*  and \ref IMPLEMENT_LM in *cpp* file to register the DLM class to GS5 kernel.
*
*  Example:
*
*  \code

	in MyLM.h:    

       class TMyLM : public gs::TGSDynamicLM {
			DECLARE_LM(TMyLM, "CAC9EE30-A394-4609-B6BA-3B1FA3F0C60B", "My First LM", "User must log in to play game");
	    protected:
			virtual void init(){
			  defineParamStr("UserName", "", LM_PARAM_READ | LM_PARAM_WRITE);
			  defineParamStr("Password", "", LM_PARAM_READ | LM_PARAM_WRITE);
			}

			bool isValidLogIn(const char* usr, const char* pwd){
			  LOG("usr [%s] pwd [%s]", usr, pwd);
				return (0 == strcmp(usr, "Randy")) && (0 == strcmp(pwd, "abcd1234"));
			}

			virtual bool isValid(){
				return isValidLogIn(license()->getParamStr("UserName").c_str(), license()->getParamStr("Password").c_str());
			}
	   };

	 in MyLM.cpp:

	   #include "MyLM.h"

	   IMPLEMENT_LM(TMyLM);

*  \endcode
*
*
* Ref: \ref DLM
*/
class TGSDynamicLM {
  private:
    friend class TGSApp; //access to static callbacks
    static TLicenseHandle WINAPI s_createLM(void *usrData);
    static bool WINAPI fcb_isValid(void *usrData);
    static void WINAPI fcb_startAccess(void *usrData);
    static void WINAPI fcb_finishAccess(void *usrData);
    static void WINAPI fcb_onAction(TActionHandle hAct, void *usrData);
    static void WINAPI fcb_onDestroy(void *usrData);

  private:
    std::unique_ptr<TGSLicense> _lic;

    bool isValid_();
    void startAccess_();
    void finishAccess_();
    void onAction_(TActionHandle hAct);

  protected:
    /** Initialize the DLM instance
	*
	*  Subclass should override this method to initialize itself (defines LM parameters, etc.)
	*/
    virtual void init();

    /** @name Define License Model Parameters **/
    //@{
    /** Defines String Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamStr(const char *paramName, const char *paramInitValue, unsigned int permission);
    /** Defines 32bit Integer Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamInt(const char *paramName, int paramInitValue, unsigned int permission);
    /** Defines 64bit Integer Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamInt64(const char *paramName, int64_t paramInitValue, unsigned int permission);
    /** Defines Boolean Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamBool(const char *paramName, bool paramInitValue, unsigned int permission);
    /** Defines Float Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamFloat(const char *paramName, float paramInitValue, unsigned int permission);
    /** Defines Double Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value 
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamDouble(const char *paramName, double paramInitValue, unsigned int permission);
    /** Defines Time Parameter
	*  
	* \param paramName the name of parameter
	* \param paramInitValue the initial parameter value
	*
	*       It is the number of seconds elapsed since 00:00 hours, Jan 1, 1970 UTC (i.e., a unix timestamp).   
	*
	* \param permission the the parameter access control mask (Ref: \ref LMParamAccessCtl)
	*/
    void defineParamTime(const char *paramName, time_t paramInitValue, unsigned int permission);
    //@}

    /** @name LM Event handlers 
	
		Sub-class overrides these handlers to handle licensing logic events 
	**/
    //@{

    /** 
	* \brief Is License Valid?
	* 
	* \return true if the license is still valid.
	*
	* This function is called *periodically* by GS5 kernel to query the current license status. 
	*
	* The entity might not be active, i.e., the startAccess() might not have been called, so please make sure
	* the function implementation should not assume that the startAccess() was called before.
	*/
    virtual bool isValid();
    /** 
	* \brief License starts to work
	*
	* Called when the entity the license attached to enters *Active* status. that is, the gsBeginAccess() is called
	* and the entity accessing is approved by its attached license(s).
	*
	* It is a great chance to start any internal timing logic, or initialize local data resources. 
	*/
    virtual void startAccess();
    /** 
	* \brief License stops working
	*
	* Called when the entity the license attached to enters *In-Active* status; the gsEndAccess() triggers this event
	*
	* It is a great chance to release any local resources used in licensing logic. 
	*/
    virtual void finishAccess();
    /**
	* \brief Action Processing
	* \param act action being applied
	*
	* Called when an action is applied to this license.
	* GS5 has processed all commonly used actions (ACT_UNLOCK, ACT_LOCK, ACT_CLEAN, etc.) before this method is called so you
	* will not see these actions passed in, you only need to deal with LM-specific actions that other action processors do not  
	* understand.
	*/
    virtual void onAction(TGSAction *act);
    //@}

    /**
	* @name Constructor and Destructor
	*
	* Must be protected to stop developer from creating /destroying LM instance directly
	*
	* DLM can only be created and initialized internally by SDK framework.  
	*/
    //@{
    ///construcotr
    TGSDynamicLM();
    ///destructor
    virtual ~TGSDynamicLM();
    //@}
  public:
    /**
	* \brief Gets the proxy TGSLicense object pointer
	*
	* The returned TGSLicense object is created automatically when the LM instance is created, it acts as proxy to access LM's parameters.
	*
	*/
    TGSLicense *license() {
        return _lic.get();
    }
};

/** @name Dynamic License Model Macros
*
*/
//@{
/**
*  Declare a License Model subclass
* 
*  \param clsName Sub-class name
*  \param licType Unique string typeId of the LM
*       
*	The typeId can be any string ( a UUID or whatever string meaningful in your software system) 
*
*  \param licName User-friendly string name of the LM
*  \param licDescription String description of the LM
*
*/
#define DECLARE_LM(clsName, licType, licName, licDescription)                      \
  private:                                                                         \
    static TGSDynamicLM *createInstance() { return new clsName(); }                \
                                                                                   \
  public:                                                                          \
    static void initClass() {                                                      \
        gs::registerLM(clsName::createInstance, licType, licName, licDescription); \
    }

/**
*  Implements a License Model subclass
* 
*  \param clsName Sub-class name
*
*   It must be in a CPP file to make sure the LM subclass is registered to the GS5 kernel.
*/
#define IMPLEMENT_LM(clsName)                         \
    namespace clsName_ {                              \
    struct clsName##Registor {                        \
        clsName##Registor() { clsName::initClass(); } \
    } clsName##Registor_inst;                         \
    }

//@}

typedef TGSDynamicLM *(*f_createLM)(void);
void registerLM(f_createLM createLM, const char *licId, const char *licName, const char *description);

}; // namespace gs
#endif
