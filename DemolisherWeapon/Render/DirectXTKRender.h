#pragma once
#include "IRander.h"

namespace DemolisherWeapon {
    class GameObjectManager;
    class CFpsCounter;

    class DirectXTKRender :
        public IRander
    {
    public:
        void Init(GameObjectManager*, CFpsCounter*);
        void Render()override;

    private:
        GameObjectManager* m_gameObjectManager_Ptr = nullptr;
        CFpsCounter* m_fpscounter = nullptr;
    };

}
