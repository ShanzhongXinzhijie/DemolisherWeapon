#pragma once

namespace DemolisherWeapon {
    class DX12Test;

    class DX12Render :
        public IRander
    {
    public:
        DX12Render(DX12Test* dx12ref):m_dx12ref(dx12ref){}
        void Render()override;
    private:
        DX12Test* m_dx12ref;
    };

}