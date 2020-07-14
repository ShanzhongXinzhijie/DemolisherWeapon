#pragma once

namespace DemolisherWeapon {
	struct InitEngineParameter;

	class IGraphicsAPI {
	public:
		virtual ~IGraphicsAPI() {
			Release();
		}

		virtual bool Init(HWND hWnd, const InitEngineParameter& initParam) = 0;
		virtual void Release() {}

		virtual void ChangeFrameBufferSize() = 0;

		virtual void ClearBackBuffer() = 0;
		virtual void SetViewport(float topLeftX, float topLeftY, float width, float height) = 0;
	};

}