#ifndef CKINTERFACE_H
#define CKINTERFACE_H

#include "CKObject.h"

class CKInterfaceObjectManager : public CKObject
{
public:
    void SetGuid(CKGUID guid);
    CKGUID GetGuid();

    ////////////////////////////////////////
    // Datas
    void AddStateChunk(CKStateChunk *chunk);
    void RemoveStateChunk(CKStateChunk *chunk);
    int GetChunkCount();
    CKStateChunk *GetChunk(int pos);

    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    // Internal functions
    //-------------------------------------------------------------------
    //-------------------------------------------------------------------
    virtual CK_CLASSID GetClassID();
    virtual ~CKInterfaceObjectManager();

    //--------------------------------------------
    // Class Registering {secret}
    static CKSTRING GetClassName();
    static int GetDependenciesCount(int mode);
    static CKSTRING GetDependencies(int i, int mode);
    static void Register();
    static CKInterfaceObjectManager *CreateInstance(CKContext *Context);
    static CK_ID m_ClassID;

    // Dynamic Cast method (returns NULL if the object can't be cast)
    static CKInterfaceObjectManager *Cast(CKObject *iO)
    {
        return CKIsChildClassOf(iO, CKCID_INTERFACEOBJECTMANAGER) ? (CKInterfaceObjectManager *)iO : NULL;
    }

private:
    int m_Count;
    CKStateChunk *m_Chunks;
    CKGUID m_Guid;
};

#endif // CKINTERFACE_H
