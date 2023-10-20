#ifndef CKCONTEXT_H
#define CKCONTEXT_H

#include "VxDefines.h"
#include "CKDefines.h"
#include "XObjectArray.h"
#include "CKDependencies.h"
#include "CKObjectManager.h"
#include "XHashTable.h"
#include "VxTimeProfiler.h"
#include "VxMemoryPool.h"

//-------------------------------------------------------------------------

typedef XClassArray<CKClassDesc> XClassInfoArray;
typedef XArray<CKBaseManager *> XManagerArray;

//-------- HashTable with CKGUID as Key for Managers and ObjectDeclarations
typedef XHashTable<CKBaseManager *, CKGUID> XManagerHashTable;
typedef XManagerHashTable::Iterator XManagerHashTableIt;
typedef XManagerHashTable::Pair XManagerHashTablePair;

typedef XHashTable<CKObjectDeclaration *, CKGUID> XObjDeclHashTable;
typedef XObjDeclHashTable::Iterator XObjDeclHashTableIt;
typedef XObjDeclHashTable::Pair XObjDeclHashTablePair;

/***************************************************************************
{filename:CKContext}
Summary: Main Interface Object

Remarks:
    + The CKContext object is the heart of all Virtools based applications, It is the first object that should be created in order to
    use Virtools SDK. A CKContext is created with the global function CKCreateContext()

    + The CKContext object act as the central interface to create/destroy objects,to access managers, to load/save files.

    + Several CKContext can be created inside a same process (in multiple threads for example) but objects created
    by a specific CKContext must not be used in other contexts.


See also: CKContext::CreateObject, CKContext::GetObject, CKContext::DestroyObject
*******************************************************************************/
class CKContext
{
    friend class CKBehavior;

public:
    //------------------------------------------------------
    // Objects Management
    CKObject *CreateObject(CK_CLASSID cid, CKSTRING Name = NULL, CK_OBJECTCREATION_OPTIONS Options = CK_OBJECTCREATION_NONAMECHECK, CK_CREATIONMODE *Res = NULL);
    CKObject *CopyObject(CKObject *src, CKDependencies *Dependencies = NULL, CKSTRING AppendName = NULL, CK_OBJECTCREATION_OPTIONS Options = CK_OBJECTCREATION_NONAMECHECK);
    const XObjectArray &CopyObjects(const XObjectArray &SrcObjects, CKDependencies *Dependencies = NULL, CK_OBJECTCREATION_OPTIONS Options = CK_OBJECTCREATION_NONAMECHECK, CKSTRING AppendName = NULL);

    CKObject *GetObject(CK_ID ObjID);
    int GetObjectCount();
    int GetObjectSize(CKObject *obj);
    CKERROR DestroyObject(CKObject *obj, CKDWORD Flags = 0, CKDependencies *depoptions = NULL);
    CKERROR DestroyObject(CK_ID id, CKDWORD Flags = 0, CKDependencies *depoptions = NULL);
    CKERROR DestroyObjects(CK_ID *obj_ids, int Count, CKDWORD Flags = 0, CKDependencies *depoptions = NULL);
    void DestroyAllDynamicObjects();
    void ChangeObjectDynamic(CKObject *iObject, CKBOOL iSetDynamic = TRUE);

    const XObjectPointerArray &CKFillObjectsUnused();

    // Object Access
    CKObject *GetObjectByName(CKSTRING name, CKObject *previous = NULL);
    CKObject *GetObjectByNameAndClass(CKSTRING name, CK_CLASSID cid, CKObject *previous = NULL);
    CKObject *GetObjectByNameAndParentClass(CKSTRING name, CK_CLASSID pcid, CKObject *previous);
    const XObjectPointerArray &GetObjectListByType(CK_CLASSID cid, CKBOOL derived);
    int GetObjectsCountByClassID(CK_CLASSID cid);
    CK_ID *GetObjectsListByClassID(CK_CLASSID cid);

    //-----------------------------------------------------------
    // Engine runtime
    CKERROR Play();
    CKERROR Pause();
    CKERROR Reset();
    CKBOOL IsPlaying();
    CKBOOL IsReseted();
    // Runtime mode
    CKERROR Process();

    CKERROR ClearAll();
    CKBOOL IsInClearAll();

    //----------------------------------------------------------
    // Current Level&Scene Functions
    CKLevel *GetCurrentLevel();
    CKRenderContext *GetPlayerRenderContext();
    CKScene *GetCurrentScene();
    void SetCurrentLevel(CKLevel *level);

    //----------------------------------------------------------
    // Object Management functions
    CKParameterIn *CreateCKParameterIn(CKSTRING Name, CKParameterType type, CKBOOL Dynamic = FALSE);
    CKParameterIn *CreateCKParameterIn(CKSTRING Name, CKGUID guid, CKBOOL Dynamic = FALSE);
    CKParameterIn *CreateCKParameterIn(CKSTRING Name, CKSTRING TypeName, CKBOOL Dynamic = FALSE);
    CKParameterOut *CreateCKParameterOut(CKSTRING Name, CKParameterType type, CKBOOL Dynamic = FALSE);
    CKParameterOut *CreateCKParameterOut(CKSTRING Name, CKGUID guid, CKBOOL Dynamic = FALSE);
    CKParameterOut *CreateCKParameterOut(CKSTRING Name, CKSTRING TypeName, CKBOOL Dynamic = FALSE);
    CKParameterLocal *CreateCKParameterLocal(CKSTRING Name, CKParameterType type, CKBOOL Dynamic = FALSE);
    CKParameterLocal *CreateCKParameterLocal(CKSTRING Name, CKGUID guid, CKBOOL Dynamic = FALSE);
    CKParameterLocal *CreateCKParameterLocal(CKSTRING Name, CKSTRING TypeName, CKBOOL Dynamic = FALSE);
    CKParameterOperation *CreateCKParameterOperation(CKSTRING Name, CKGUID opguid, CKGUID ResGuid, CKGUID p1Guid, CKGUID p2Guid);

    CKFile *CreateCKFile();
    CKERROR DeleteCKFile(CKFile *);

    //-----------------------------------------------------
    // IHM

    void SetInterfaceMode(CKBOOL mode = TRUE, CKUICALLBACKFCT CallBackFct = NULL, void *data = NULL);
    CKBOOL IsInInterfaceMode();

    // Behaviors and CK can send messages to the console if errors occur
    CKERROR OutputToConsole(CKSTRING str, CKBOOL bBeep = TRUE);
    CKERROR OutputToConsoleEx(CKSTRING format, ...);
    CKERROR OutputToConsoleExBeep(CKSTRING format, ...);
    CKERROR OutputToInfo(CKSTRING format, ...);
    CKERROR RefreshBuildingBlocks(const XArray<CKGUID> &iGuids);

    CKERROR ShowSetup(CK_ID);
    CK_ID ChooseObject(void *dialogParentWnd);
    CKERROR Select(const XObjectArray &o, CKBOOL clearSelection = TRUE);
    CKDWORD SendInterfaceMessage(CKDWORD reason, CKDWORD param1, CKDWORD param2);

    CKERROR UICopyObjects(const XObjectArray &iObjects, CKBOOL iClearClipboard = TRUE);
    CKERROR UIPasteObjects(const XObjectArray &oObjects);

    //------------------------------------------------------
    // Common Managers functions
    CKRenderManager *GetRenderManager();
    CKBehaviorManager *GetBehaviorManager();
    CKParameterManager *GetParameterManager();
    CKMessageManager *GetMessageManager();
    CKTimeManager *GetTimeManager();
    CKAttributeManager *GetAttributeManager();
    CKPathManager *GetPathManager();
    XManagerHashTableIt GetManagers();
    CKBaseManager *GetManagerByGuid(CKGUID guid);
    CKBaseManager *GetManagerByName(CKSTRING ManagerName);

    int GetManagerCount();
    CKBaseManager *GetManager(int index);

    CKBOOL IsManagerActive(CKBaseManager *bm);
    CKBOOL HasManagerDuplicates(CKBaseManager *bm);
    void ActivateManager(CKBaseManager *bm, CKBOOL active);
    int GetInactiveManagerCount();
    CKBaseManager *GetInactiveManager(int index);

    CKERROR RegisterNewManager(CKBaseManager *manager);

    //---------------------------------------------------------------
    // Profiling functions
    void EnableProfiling(CKBOOL enable);
    CKBOOL IsProfilingEnable();
    void GetProfileStats(CKStats *stats);
    void UserProfileStart(CKDWORD UserSlot);
    float UserProfileEnd(CKDWORD UserSlot);
    float GetLastUserProfileTime(CKDWORD UserSlot);

    //-------------------------------------------------------
    // Utils
    CKSTRING GetStringBuffer(int size);
    CKGUID GetSecureGuid();
    CKDWORD GetStartOptions();
    WIN_HANDLE GetMainWindow();
    int GetSelectedRenderEngine();

    //----------------------------------------------------------------
    // File Save/Load Options
    void SetCompressionLevel(int level);
    int GetCompressionLevel();

    void SetFileWriteMode(CK_FILE_WRITEMODE mode);
    CK_FILE_WRITEMODE GetFileWriteMode();

    CK_TEXTURE_SAVEOPTIONS GetGlobalImagesSaveOptions();
    void SetGlobalImagesSaveOptions(CK_TEXTURE_SAVEOPTIONS Options);

    CKBitmapProperties *GetGlobalImagesSaveFormat();
    void SetGlobalImagesSaveFormat(CKBitmapProperties *Format);

    CK_SOUND_SAVEOPTIONS GetGlobalSoundsSaveOptions();
    void SetGlobalSoundsSaveOptions(CK_SOUND_SAVEOPTIONS Options);

    //---------------------------------------------------------------
    // Save/Load functions
    CKERROR Load(CKSTRING FileName, CKObjectArray *liste, CK_LOAD_FLAGS LoadFlags = CK_LOAD_DEFAULT, CKGUID *ReaderGuid = NULL);
    CKERROR Load(int BufferSize, void *MemoryBuffer, CKObjectArray *ckarray, CK_LOAD_FLAGS LoadFlags = CK_LOAD_DEFAULT);
    CKSTRING GetLastFileLoaded();

    CKSTRING GetLastCmoLoaded();
    void SetLastCmoLoaded(CKSTRING str);

    CKERROR GetFileInfo(CKSTRING FileName, CKFileInfo *FileInfo);
    CKERROR GetFileInfo(int BufferSize, void *MemoryBuffer, CKFileInfo *FileInfo);
    CKERROR Save(CKSTRING FileName, CKObjectArray *liste, CKDWORD SaveFlags, CKDependencies *dependencies = NULL, CKGUID *ReaderGuid = NULL);
    CKERROR LoadAnimationOnCharacter(CKSTRING FileName, CKObjectArray *liste, CKCharacter *carac, CKGUID *ReaderGuid = NULL, CKBOOL AsDynamicObjects = FALSE);
    CKERROR LoadAnimationOnCharacter(int BufferSize, void *MemoryBuffer, CKObjectArray *ckarray, CKCharacter *carac, CKBOOL AsDynamicObjects = FALSE);

    void SetAutomaticLoadMode(CK_LOADMODE GeneralMode, CK_LOADMODE _3DObjectsMode, CK_LOADMODE MeshMode, CK_LOADMODE MatTexturesMode);
    void SetUserLoadCallback(CK_USERLOADCALLBACK fct, void *Arg);
    CK_LOADMODE LoadVerifyObjectUnicity(CKSTRING OldName, CK_CLASSID Cid, const CKSTRING NewName, CKObject **newobj);

    CKBOOL IsInLoad();
    CKBOOL IsInSave();
    CKBOOL IsRunTime();

    //----------------------------------------------------
    //	Render Engine Implementation Specific

    void ExecuteManagersOnPreRender(CKRenderContext *dev);
    void ExecuteManagersOnPostRender(CKRenderContext *dev);
    void ExecuteManagersOnPostSpriteRender(CKRenderContext *dev);

    void AddProfileTime(CK_PROFILE_CATEGORY cat, float time);

    //------- Runtime Debug Mode

    CKERROR ProcessDebugStart(float delta_time = 20.0f);
    CKBOOL ProcessDebugStep();
    CKERROR ProcessDebugEnd();
    CKDebugContext *GetDebugContext();

    void SetVirtoolsVersion(CK_VIRTOOLS_VERSION ver, CKDWORD Build);
    int GetPVInformation();
    CKBOOL IsInDynamicCreationMode();

    XManagerHashTable m_ManagerTable;
    XManagerArray m_InactiveManagers;
    XArray<CKBaseManager*> m_ManagersOnSequenceToBeDeleted;
    XArray<CKBaseManager*> m_ManagersOnSequenceDeleted;
    XArray<CKBaseManager*> m_ManagersPreProcess;
    XArray<CKBaseManager*> m_ManagersPostProcess;
    XArray<CKBaseManager*> m_ManagersPreClearAll;
    XArray<CKBaseManager*> m_ManagersPostClearAll;
    XArray<CKBaseManager*> m_ManagersOnCKInit;
    XArray<CKBaseManager*> m_ManagersOnCKEnd;
    XArray<CKBaseManager*> m_ManagersOnCKPlay;
    XArray<CKBaseManager*> m_ManagersOnCKPause;
    XArray<CKBaseManager*> m_ManagersPreLoad;
    XArray<CKBaseManager*> m_ManagersPreSave;
    XArray<CKBaseManager*> m_ManagersPreLaunchScene;
    XArray<CKBaseManager*> m_ManagersPostLaunchScene;
    XArray<CKBaseManager*> m_ManagersOnCKReset;
    XArray<CKBaseManager*> m_ManagersPostLoad;
    XArray<CKBaseManager*> m_ManagersPostSave;
    XArray<CKBaseManager*> m_ManagersOnCKPostReset;
    XArray<CKBaseManager*> m_ManagersOnSequenceAddedToScene;
    XArray<CKBaseManager*> m_ManagersOnSequenceRemovedFromScene;
    XArray<CKBaseManager*> m_ManagersOnPreCopy;
    XArray<CKBaseManager*> m_ManagersOnPostCopy;
    XArray<CKBaseManager*> m_ManagersOnPreRender;
    XArray<CKBaseManager*> m_ManagersOnPostRender;
    XArray<CKBaseManager*> m_ManagersOnPostSpriteRender;
    CKDWORD field_150;
    CKDWORD field_154;
    CKDWORD field_158;
    CKDWORD field_15C;
    CKDWORD field_160;
    CKDWORD field_164;
    CKDWORD field_168;
    CKDWORD field_16C;
    CKDWORD field_170;
    CKDWORD field_174;
    CKDWORD field_178;
    CKDWORD field_17C;
    CKDWORD field_180;
    CKDWORD field_184;
    CKDWORD field_188;
    CKDWORD field_18C;
    CKDWORD field_190;
    CKDWORD field_194;
    CKDWORD field_198;
    CKDWORD field_19C;
    CKDWORD field_1A0;
    CKObjectManager *m_ObjectManager;
    CKParameterManager *m_ParameterManager;
    CKAttributeManager *m_AttributeManager;
    CKTimeManager *m_TimeManager;
    CKMessageManager *m_MessageManager;
    CKBehaviorManager *m_BehaviorManager;
    CKPathManager *m_PathManager;
    CKBehaviorContext m_BehaviorContext;
    CKRenderManager *m_RenderManager;
    CKStats m_Stats;
    CKStats m_ProfileStats;
    CKBOOL m_ProfilingEnabled;
    CK_TEXTURE_SAVEOPTIONS m_GlobalImagesSaveOptions;
    CK_SOUND_SAVEOPTIONS m_GlobalSoundsSaveOptions;
    CKBitmapProperties *m_GlobalImagesSaveFormat;
    CKDWORD m_FileWriteMode;
    XString m_LastFileLoaded;
    XString m_LastCmoLoaded;
    void *field_2CC;
    CKDWORD field_2D0;
    CKDWORD field_2D4;
    CKDebugContext *m_DebugContext;
    WIN_HANDLE m_MainWindow;
    CKDWORD m_InterfaceMode;
    int m_VirtoolsVersion;
    int m_VirtoolsBuild;
    CKUICALLBACKFCT m_UICallBackFct;
    void *m_InterfaceModeData;
    CKDWORD m_Playing;
    CKDWORD m_Reseted;
    CKDWORD m_DeferDestroyObjects;
    CK_ID m_CurrentLevel;
    CKBOOL m_InLoad;
    CKBOOL m_Saving;
    CKBOOL m_Init;
    CKBOOL m_InitManagerOnRegister;
    CKBOOL m_InClearAll;
    CKBOOL m_RunTime;
    VxTimeProfiler m_UserProfileTimers[8];
    float m_UserProfile[8];
    XString m_StringBuffer;
    CKDWORD m_StartOptions;
    CKDWORD field_3C8;
    CKDWORD field_3CC;
    CK_LOADMODE m_GeneralLoadMode;
    CK_LOADMODE m_3DObjectsLoadMode;
    CK_LOADMODE m_MeshLoadMode;
    CK_LOADMODE m_MatTexturesLoadMode;
    CK_USERLOADCALLBACK m_UserLoadCallBack;
    void *m_UserLoadCallBackArgs;
    CKDWORD m_SelectedRenderEngine;
    CKBaseManager *m_CurrentManager;
    CKDependenciesContext m_DependenciesContext;
    CKDWORD m_DestroyObjectFlag;
    CKDWORD field_45C;
    CKDWORD field_460;
    int m_CompressionLevel;
    XArray<VxMemoryPool *> m_MemoryPools;
    XBitArray m_BitArray;
    XObjectPointerArray m_GlobalAttributeList;
    XArray<CKGUID> field_488;
    void *field_494;
    CKDWORD field_498;
    void *field_49C;
    CKDWORD field_4A0;
    CKDWORD m_PVInformation;
    CKBOOL m_InDynamicCreationMode;
    XObjectPointerArray m_CopyObjects;
    XObjectPointerArray m_ObjectList;
};

#endif // CKCONTEXT_H
