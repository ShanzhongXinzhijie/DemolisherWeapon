#pragma once

#ifdef DW_DX12
namespace DemolisherWeapon {
    class DX12Test;

    class DX12Render :
        public IRander
    {
    public:
        void Init(DX12Test* dx12ref) {
            m_dx12ref = dx12ref;
        }
        void Render()override;
    private:
        DX12Test* m_dx12ref = nullptr;
    };

}
#endif