#pragma once

namespace DemolisherWeapon {

    class DX12Render :
        public IRander
    {
    public:
        void Render()override;

        //�O�t���[���̕`�抮����҂�
        bool WaitForPreviousFrame();

        //GPU�̂��ׂĂ̏����̏I����҂�
        bool WaitForGpu();

    };

}