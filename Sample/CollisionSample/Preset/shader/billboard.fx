#ifndef DW_BILLBOARD_FX
#define DW_BILLBOARD_FX

#include"model.fx"

//ビルボード頂点シェーダ(通常)
PSInput VSMain_Billboard(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	//ビルボード行列の計算
	float4x4 worldMat, worldMatOld;
	worldMat = mView; worldMat._m03 = 0.0f; worldMat._m13 = 0.0f; worldMat._m23 = 0.0f;
	//worldMatOld = mView_old; worldMatOld._m03 = 0.0f; worldMatOld._m13 = 0.0f; worldMatOld._m23 = 0.0f;
	worldMat = transpose(worldMat);
	worldMatOld = worldMat;//transpose(worldMatOld);

	//行列合成
#if defined(INSTANCING)
	worldMat = mul(worldMat, InstancingWorldMatrix[instanceID]);
	worldMatOld = mul(worldMatOld, InstancingWorldMatrixOld[instanceID]);
#else
	worldMat = mul(worldMat, mWorld);
	worldMatOld = mul(worldMatOld, mWorld_old);
#endif

	//座標合成
#if defined(INSTANCING)
	worldMat._m03 = InstancingWorldMatrix[instanceID]._m03; worldMat._m13 = InstancingWorldMatrix[instanceID]._m13; worldMat._m23 = InstancingWorldMatrix[instanceID]._m23;
	worldMatOld._m03 = InstancingWorldMatrixOld[instanceID]._m03; worldMatOld._m13 = InstancingWorldMatrixOld[instanceID]._m13; worldMatOld._m23 = InstancingWorldMatrixOld[instanceID]._m23;
#else
	worldMat._m03 = mWorld._m03; worldMat._m13 = mWorld._m13; worldMat._m23 = mWorld._m23;
	worldMatOld._m03 = mWorld_old._m03; worldMatOld._m13 = mWorld_old._m13; worldMatOld._m23 = mWorld_old._m23;
#endif	

	//通常処理
	PSInput psInput = VSModel(In, worldMat, worldMatOld
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	return psInput;
}

//ビルボード頂点シェーダ(深度値)
ZPSInput VSMain_RenderZ_Billboard(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	//ビルボード行列の計算
	float4x4 worldMat, viewRotMat;
	viewRotMat = mView; viewRotMat._m03 = 0.0f; viewRotMat._m13 = 0.0f; viewRotMat._m23 = 0.0f;
	worldMat = transpose(viewRotMat);

	//行列合成
#if defined(INSTANCING)
	worldMat = mul(worldMat, InstancingWorldMatrix[instanceID]);
#else
	worldMat = mul(worldMat, mWorld);
#endif	

	//座標合成
#if defined(INSTANCING)
	worldMat._m03 = InstancingWorldMatrix[instanceID]._m03; worldMat._m13 = InstancingWorldMatrix[instanceID]._m13; worldMat._m23 = InstancingWorldMatrix[instanceID]._m23;
#else
	worldMat._m03 = mWorld._m03; worldMat._m13 = mWorld._m13; worldMat._m23 = mWorld._m23;
#endif

	//シャドウ位置ずらす
	float3 camFront = mul(float3(0.0f, 0.0f, 1.0f), viewRotMat);//カメラの前方向
	float maxScale = length(float3(worldMat._m00, worldMat._m01, worldMat._m02));
	maxScale = max(maxScale, length(float3(worldMat._m10, worldMat._m11, worldMat._m12)));
	maxScale = max(maxScale, length(float3(worldMat._m20, worldMat._m21, worldMat._m22)));
	camFront *= maxScale;
	worldMat._m03 += camFront.x; worldMat._m13 += camFront.y; worldMat._m23 += camFront.z;

	//通常処理
	ZPSInput psInput = VSModel_RenderZ(In, worldMat
#if defined(INSTANCING)
		, instanceID
#endif 
	);

	return psInput;
}

#endif //DW_BILLBOARD_FX