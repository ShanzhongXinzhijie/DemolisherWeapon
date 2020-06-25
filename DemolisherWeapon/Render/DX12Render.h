#pragma once

namespace DemolisherWeapon {

    class DX12Render :
        public IRander
    {
    public:
        void Render()override;

        //前フレームの描画完了を待つ
        bool WaitForPreviousFrame();

        //GPUのすべての処理の終わりを待つ
        bool WaitForGpu();

    };

}