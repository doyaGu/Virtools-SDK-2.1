#ifndef CKSCENEOBJECTDESC_H
#define CKSCENEOBJECTDESC_H

#include "CKObject.h"

/////////////////////////////////////////////////////
// All private

class CKSceneObjectDesc
{
public:
    CKSceneObjectDesc()
    {
        m_Object = 0;
        m_InitialValue = NULL;
        m_Global = 0;
    };

    CKSceneObjectDesc(CKObject *obj, CKStateChunk *initialValue = NULL, CKDWORD flags = 0)
    {
        m_Object = obj ? obj->GetID() : 0;
        m_InitialValue = initialValue;
        m_Flags = flags;
    }

    CKSceneObjectDesc(const CKSceneObjectDesc &desc)
    {
        m_Object = desc.m_Object;
        m_InitialValue = desc.m_InitialValue;
        m_Global = desc.m_Global;
    }

    friend bool operator==(const CKSceneObjectDesc &lhs, const CKSceneObjectDesc &rhs) {
        return lhs.m_Object == rhs.m_Object
            && lhs.m_InitialValue == rhs.m_InitialValue
            && lhs.m_Global == rhs.m_Global;
    }

    friend bool operator!=(const CKSceneObjectDesc &lhs, const CKSceneObjectDesc &rhs) {
        return !(lhs == rhs);
    }

    CKERROR ReadState(CKStateChunk *chunk);
    void Clear();
    void Init(CKObject *obj = NULL);

    CKDWORD ActiveAtStart() { return m_Flags & CK_SCENEOBJECT_START_ACTIVATE; }
    CKDWORD DeActiveAtStart() { return m_Flags & CK_SCENEOBJECT_START_DEACTIVATE; }
    CKDWORD NothingAtStart() { return m_Flags & CK_SCENEOBJECT_START_LEAVE; }
    CKDWORD ResetAtStart() { return m_Flags & CK_SCENEOBJECT_START_RESET; }
    CKDWORD IsActive() { return m_Flags & CK_SCENEOBJECT_ACTIVE; }

    CK_ID m_Object;
    CKStateChunk *m_InitialValue;
    union
    {
        CKDWORD m_Global;
        CKDWORD m_Flags;
    };
};

#endif // CKSCENEOBJECTDESC_H
