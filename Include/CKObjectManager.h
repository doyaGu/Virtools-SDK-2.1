#ifndef CKOBJECTRMANAGER_H
#define CKOBJECTRMANAGER_H

#include "CKDefines.h"
#include "CKBaseManager.h"
#include "CKDependencies.h"
#include "XObjectArray.h"

class CKDeferredDeletion;

class CKObjectManager : public CKBaseManager
{
public:
    int ObjectsByClass(CK_CLASSID cid, CKBOOL derived, CK_ID *obj_ids);
    int GetObjectsCount();

    CKERROR DeleteAllObjects();
    CKERROR ClearAllObjects();

    CKBOOL IsObjectSafe(CKObject *iObject);

    CKERROR DeleteObjects(CK_ID *obj_ids, int Count, CK_CLASSID cid, CKDWORD Flags = 0);

    CKERROR GetRootEntities(XObjectPointerArray &array);

    int GetObjectsCountByClassID(CK_CLASSID cid);
    CK_ID *GetObjectsListByClassID(CK_CLASSID cid);

    CK_ID RegisterObject(CKObject *iObject);
    void FinishRegisterObject(CKObject *iObject);
    void UnRegisterObject(CK_ID id);

    CKObject *GetObjectByName(CKSTRING name, CKObject *previous = NULL);
    CKObject *GetObjectByName(CKSTRING name, CK_CLASSID cid, CKObject *previous = NULL);
    CKObject *GetObjectByNameAndParentClass(CKSTRING name, CK_CLASSID pcid, CKObject *previous = NULL);
    CKERROR GetObjectListByType(CK_CLASSID cid, XObjectPointerArray &array, CKBOOL derived);

    CKBOOL InLoadSession();
    void StartLoadSession(int id);
    void EndLoadSession();

    void RegisterLoadObject(CKObject *iObject, int id);

    CK_ID RealId(CK_ID id);

    int CheckIDArray(CK_ID *obj_ids, int Count);
    int CheckIDArrayPredeleted(CK_ID *obj_ids, int Count);

    CKDeferredDeletion *MatchDeletion(CKDependencies *depoptions = NULL, CKDWORD Flags = 0);
    void RegisterDeletion(CKDeferredDeletion *deletion);

    int GetDynamicIDCount();
    CK_ID GetDynamicID(int index);

    void DeleteAllDynamicObjects();

    void SetDynamic(CKObject *iObject);
    void UnSetDynamic(CKObject *iObject);

    //-------------------------------------------------------------------------
    // Internal functions

    virtual CKERROR PreClearAll();
    virtual CKERROR PostProcess();
    virtual CKERROR OnCKReset();
    virtual CKERROR SequenceToBeDeleted(CK_ID *objids, int count);
    virtual CKDWORD GetValidFunctionsMask()
    {
        return CKMANAGER_FUNC_PreClearAll |
               CKMANAGER_FUNC_PostProcess |
               CKMANAGER_FUNC_OnCKReset |
               CKMANAGER_FUNC_OnSequenceToBeDeleted;
    }

    virtual ~CKObjectManager();

    CKObjectManager(CKContext *Context);

public:
    CKDWORD GetGroupGlobalIndex();
    void ReleaseGroupGlobalIndex(CKDWORD);

    int GetSceneGlobalIndex();
    void ReleaseSceneGlobalIndex(int index);

    void *GetObjectAppData(CK_ID id);
    void SetObjectAppData(CK_ID id, void *arg);

    void AddSingleObjectActivity(CKSceneObject *o, CK_ID id);
    int GetSingleObjectActivity(CKSceneObject *o, CK_ID &id);

    int m_ObjectsCount;
    CKObject **m_ObjectPtrs;
    XClassArray<XObjectArray> m_ObjectLists;
    CKDWORD m_LoadSession;
    CKDWORD m_Count;
    CKBOOL m_NeedDeleteAllDynamicObjects;
    CKBOOL m_InLoadSession;
    CKDWORD m_MaxObjectID;
    XHashID m_ObjectAppData;
    XHashID m_SingleObjectActivities;
    XArray<CKGUID> m_Array;
    XArray<CKDeferredDeletion *> m_DeferredDeletions[4];
    XObjectArray m_DynamicObjects;
    XBitArray m_SceneGlobalIndex;
    XBitArray m_GroupGlobalIndex;
};

#endif // CKOBJECTRMANAGER_H
