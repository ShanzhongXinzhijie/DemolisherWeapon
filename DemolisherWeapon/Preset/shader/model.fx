/*!
 * @brief	モデルシェーダー。
 */

#include"MotionBlurHeader.h"

#if defined(ALL)
#define MOTIONBLUR 1
#define NORMAL_MAP 1
#endif

/////////////////////////////////////////////////////////////
// Shader Resource View
/////////////////////////////////////////////////////////////
#if !defined(SKY_CUBE)
//アルベドテクスチャ。
Texture2D<float4> albedoTexture : register(t0);	
#else
//スカイボックス用キューブマップ
TextureCube<float4> skyCubeMap : register(t0);
#endif
#if NORMAL_MAP
//ノーマルマップ
Texture2D<float3> NormalTexture : register(t1);
#endif
//ボーン行列
StructuredBuffer<float4x4> boneMatrix : register(t2);
StructuredBuffer<float4x4> boneMatrixOld : register(t3);
//インスタンシング用ワールド行列
StructuredBuffer<float4x4> InstancingWorldMatrix : register(t4);
StructuredBuffer<float4x4> InstancingWorldMatrixOld : register(t5);

/////////////////////////////////////////////////////////////
// SamplerState
/////////////////////////////////////////////////////////////
sampler Sampler : register(s0);

/////////////////////////////////////////////////////////////
// 定数バッファ。
/////////////////////////////////////////////////////////////
/*!
 * @brief	頂点シェーダーとピクセルシェーダー用の定数バッファ。
 */
cbuffer VSPSCb : register(b0){
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;

	//前回の行列
	float4x4 mWorld_old;
	float4x4 mView_old;
	float4x4 mProj_old;

	//カメラの移動量
	float4 camMoveVec;//w:しきい値≒距離スケール

	float4 depthBias;//x:max=(1.0f) y:max=(far-near) z:ブラーの近距離しきい値

	//カメラのワールド座標
	float3 camWorldPos;
};

//マテリアルパラメーター
cbuffer MaterialCb : register(b1) {
	float4 albedoScale;	//アルベドにかけるスケール
	float  emissive;	//自己発光
	float  isLighting;	//ライティングするか
	float2 uvOffset;
}

/////////////////////////////////////////////////////////////
//各種構造体
/////////////////////////////////////////////////////////////
/*!
 * @brief	スキンなしモデルの頂点構造体。
 */
struct VSInputNmTxVcTangent
{
    float4 Position : SV_Position;			//頂点座標。
    float3 Normal   : NORMAL;				//法線。
    float3 Tangent  : TANGENT;				//接ベクトル。
	float3 Binormal : BINORMAL;				//従法線。
    float2 TexCoord : TEXCOORD0;			//UV座標。
};
/*!
 * @brief	スキンありモデルの頂点構造体。
 */
struct VSInputNmTxWeights
{
    float4 Position : SV_Position;			//頂点座標。
    float3 Normal   : NORMAL;				//法線。
    float2 TexCoord	: TEXCOORD0;			//UV座標。
    float3 Tangent	: TANGENT;				//接ベクトル。
	float3 Binormal : BINORMAL;				//従法線。
	uint4  Indices  : BLENDINDICES0;		//この頂点に関連付けされているボーン番号。x,y,z,wの要素に入っている。4ボーンスキニング。
    float4 Weights  : BLENDWEIGHT0;			//この頂点に関連付けされているボーンへのスキンウェイト。x,y,z,wの要素に入っている。4ボーンスキニング。
};

/*!
 * @brief	ピクセルシェーダーの入力。
 */
struct PSInput{
	float4 Position 	: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float2 TexCoord 	: TEXCOORD0;
	float3 Viewpos		: TEXCOORD1;

	float4	curPos		: CUR_POSITION;//現在座標
	float4	lastPos		: LAST_POSITION;//過去座標
	bool isWorldMove	: IS_WORLD_BLUR;//ワールド空間で移動しているか?

	uint instanceID		: InstanceID;

	float3 cubemapPos	: CUBE_POS;
};

//Z値書き込みピクセルシェーダーの入力
struct ZPSInput {
	float4 Position 	: SV_POSITION;
	float2 TexCoord 	: TEXCOORD0;
	float4 posInProj	: TEXCOORD1;
};


/*!--------------------------------------------------------------------------------------
 * @brief	スキンなしモデル用の頂点シェーダー。
-------------------------------------------------------------------------------------- */
PSInput VSMain( VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID 
#endif 
){
	PSInput psInput = (PSInput)0;

#if defined(INSTANCING)
	psInput.instanceID = instanceID;
	float4 pos = mul(InstancingWorldMatrix[instanceID], In.Position);
#else
	float4 pos = mul(mWorld, In.Position);
#endif

	float3 posW = pos.xyz; psInput.cubemapPos = normalize(posW - camWorldPos);
	pos = mul(mView, pos); psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

#if defined(INSTANCING)
	psInput.Normal = normalize(mul(InstancingWorldMatrix[instanceID], In.Normal));
#if NORMAL_MAP
	psInput.Tangent = normalize(mul(InstancingWorldMatrix[instanceID], In.Tangent));
	psInput.Binormal = normalize(mul(InstancingWorldMatrix[instanceID], In.Binormal));
#endif
#else
	psInput.Normal = normalize(mul(mWorld, In.Normal));
#if NORMAL_MAP
	psInput.Tangent = normalize(mul(mWorld, In.Tangent));
	psInput.Binormal = normalize(mul(mWorld, In.Binormal));
#endif
#endif

	psInput.curPos = pos;

	//ベロシティマップ用情報
#if MOTIONBLUR

#if defined(INSTANCING)
		float4 oldpos = mul(InstancingWorldMatrixOld[instanceID], In.Position);
#else
		float4 oldpos = mul(mWorld_old, In.Position);
#endif

		oldpos.xyz = lerp(posW, oldpos.xyz, MotionBlurScale);

#if defined(INSTANCING)
		float3 trans = float3(InstancingWorldMatrix[instanceID]._m03, InstancingWorldMatrix[instanceID]._m13, InstancingWorldMatrix[instanceID]._m23);
		float3 transOld = float3(InstancingWorldMatrixOld[instanceID]._m03, InstancingWorldMatrixOld[instanceID]._m13, InstancingWorldMatrixOld[instanceID]._m23);
#else
		float3 trans = float3(mWorld._m03, mWorld._m13, mWorld._m23);
		float3 transOld = float3(mWorld_old._m03, mWorld_old._m13, mWorld_old._m23);
#endif

		transOld = lerp(trans, transOld, MotionBlurScale);
		trans -= transOld;

		if (length(trans) > camMoveVec.w
			&& distance(camMoveVec.xyz, trans) > camMoveVec.w
#if defined(INSTANCING)
			|| distance(float3(InstancingWorldMatrix[instanceID]._m00, InstancingWorldMatrix[instanceID]._m10, InstancingWorldMatrix[instanceID]._m20), float3(InstancingWorldMatrixOld[instanceID]._m00, InstancingWorldMatrixOld[instanceID]._m10, InstancingWorldMatrixOld[instanceID]._m20)) > 0.0f
			|| distance(float3(InstancingWorldMatrix[instanceID]._m01, InstancingWorldMatrix[instanceID]._m11, InstancingWorldMatrix[instanceID]._m21), float3(InstancingWorldMatrixOld[instanceID]._m01, InstancingWorldMatrixOld[instanceID]._m11, InstancingWorldMatrixOld[instanceID]._m21)) > 0.0f
			|| distance(float3(InstancingWorldMatrix[instanceID]._m02, InstancingWorldMatrix[instanceID]._m12, InstancingWorldMatrix[instanceID]._m22), float3(InstancingWorldMatrixOld[instanceID]._m02, InstancingWorldMatrixOld[instanceID]._m12, InstancingWorldMatrixOld[instanceID]._m22)) > 0.0f
#else
			|| distance(float3(mWorld._m00, mWorld._m10, mWorld._m20), float3(mWorld_old._m00, mWorld_old._m10, mWorld_old._m20)) > 0.0f
			|| distance(float3(mWorld._m01, mWorld._m11, mWorld._m21), float3(mWorld_old._m01, mWorld_old._m11, mWorld_old._m21)) > 0.0f
			|| distance(float3(mWorld._m02, mWorld._m12, mWorld._m22), float3(mWorld_old._m02, mWorld_old._m12, mWorld_old._m22)) > 0.0f
#endif
		){
			psInput.isWorldMove = true;
		}

		float4 oldpos2 = mul(mView_old, oldpos);
		oldpos = mul(mView, oldpos);
		oldpos.xyz = lerp(oldpos.xyz, oldpos2.xyz, MotionBlurScale);

		oldpos = mul(mProj_old, oldpos);
		
		psInput.lastPos = oldpos;
#endif

	return psInput;
}
//Z値書き込み用
ZPSInput VSMain_RenderZ(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	ZPSInput psInput = (ZPSInput)0;

#if defined(INSTANCING)
	float4 pos = mul(InstancingWorldMatrix[instanceID], In.Position);
#else
	float4 pos = mul(mWorld, In.Position);
#endif

	pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

/*!--------------------------------------------------------------------------------------
 * @brief	スキンありモデル用の頂点シェーダー。
 * 全ての頂点に対してこのシェーダーが呼ばれる。
 * Inは1つの頂点データ。VSInputNmTxWeightsを見てみよう。
-------------------------------------------------------------------------------------- */
PSInput VSMainSkin( VSInputNmTxWeights In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	PSInput psInput = (PSInput)0;
	///////////////////////////////////////////////////
	//ここからスキニングを行っている箇所。
	//スキン行列を計算。
	///////////////////////////////////////////////////
	float4x4 skinning = 0;	
	float4 pos = 0;
	{	
		float w = 0.0f;
		[unroll]
	    for (int i = 0; i < 3; i++)
	    {
			//boneMatrixにボーン行列が設定されていて、
			//In.indicesは頂点に埋め込まれた、関連しているボーンの番号。
			//In.weightsは頂点に埋め込まれた、関連しているボーンのウェイト。
	        skinning += boneMatrix[In.Indices[i]] * In.Weights[i];
	        w += In.Weights[i];
	    }
	    //最後のボーンを計算する。
	    skinning += boneMatrix[In.Indices[3]] * (1.0f - w);	  	
	}
#if defined(INSTANCING)
	psInput.instanceID = instanceID;
	//インスタンシング
	skinning = mul(InstancingWorldMatrix[instanceID], skinning);
#endif
	
	//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
	//mulは乗算命令。
	pos = mul(skinning, In.Position);
	psInput.Normal = normalize( mul(skinning, In.Normal) );
#if NORMAL_MAP
	psInput.Tangent = normalize( mul(skinning, In.Tangent) );
	psInput.Binormal = normalize( mul(skinning, In.Binormal) );
#endif

	float3 posW = pos.xyz; psInput.cubemapPos = normalize(posW - camWorldPos);

	pos = mul(mView, pos);  psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

	psInput.curPos = pos;

	//ベロシティマップ用情報
#if MOTIONBLUR
		float4x4 oldskinning = 0;
		float4 oldpos = 0;
		{
			float w = 0.0f;
			[unroll]
			for (int i = 0; i < 3; i++)
			{
				oldskinning += boneMatrixOld[In.Indices[i]] * In.Weights[i];
				w += In.Weights[i];
			}
			oldskinning += boneMatrixOld[In.Indices[3]] * (1.0f - w);
		}
#if defined(INSTANCING)
		//インスタンシング
		oldskinning = mul(InstancingWorldMatrixOld[instanceID], oldskinning);
#endif
		oldpos = mul(oldskinning, In.Position);
		oldpos.xyz = lerp(posW, oldpos.xyz, MotionBlurScale);

		float3 trans = float3(skinning._m03, skinning._m13, skinning._m23);
		float3 transOld = float3(oldskinning._m03, oldskinning._m13, oldskinning._m23);
		transOld = lerp(trans, transOld, MotionBlurScale);
		trans -= transOld;

		if (length(trans) > camMoveVec.w
			&& distance(camMoveVec.xyz, trans) > camMoveVec.w
			|| distance(float3(skinning._m00, skinning._m10, skinning._m20), float3(oldskinning._m00, oldskinning._m10, oldskinning._m20)) > 0.0f
			|| distance(float3(skinning._m01, skinning._m11, skinning._m21), float3(oldskinning._m01, oldskinning._m11, oldskinning._m21)) > 0.0f
			|| distance(float3(skinning._m02, skinning._m12, skinning._m22), float3(oldskinning._m02, oldskinning._m12, oldskinning._m22)) > 0.0f
		) {
			psInput.isWorldMove = true;
		}

		float4 oldpos2 = mul(mView_old, oldpos);
		oldpos = mul(mView, oldpos);
		oldpos.xyz = lerp(oldpos.xyz, oldpos2.xyz, MotionBlurScale);

		oldpos = mul(mProj_old, oldpos);
			
		psInput.lastPos = oldpos;
#endif

    return psInput;
}
//Z値書き込み用
ZPSInput VSMainSkin_RenderZ(VSInputNmTxWeights In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	ZPSInput psInput = (ZPSInput)0;
	///////////////////////////////////////////////////
	//ここからスキニングを行っている箇所。
	//スキン行列を計算。
	///////////////////////////////////////////////////
	float4x4 skinning = 0;
	float4 pos = 0;
	{
		float w = 0.0f;
		[unroll]
		for (int i = 0; i < 3; i++)
		{
			//boneMatrixにボーン行列が設定されていて、
			//In.indicesは頂点に埋め込まれた、関連しているボーンの番号。
			//In.weightsは頂点に埋め込まれた、関連しているボーンのウェイト。
			skinning += boneMatrix[In.Indices[i]] * In.Weights[i];
			w += In.Weights[i];
		}
		//最後のボーンを計算する。
		skinning += boneMatrix[In.Indices[3]] * (1.0f - w);		
	}
#if defined(INSTANCING)
	//インスタンシング
	skinning = mul(InstancingWorldMatrix[instanceID], skinning);
#endif

	//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
	//mulは乗算命令。
	pos = mul(skinning, In.Position);	
	pos = mul(mView, pos);
	pos = mul(mProj, pos);

	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

//G-Buffer出力
struct PSOutput_RenderGBuffer {
	float4 albedo		: SV_Target0;		//アルベド
	float3 normal		: SV_Target1;		//法線
	float4 viewpos		: SV_Target2;		//ビュー座標
	float4 velocity		: SV_Target3;		//速度
	float4 velocityPS	: SV_Target4;		//速度(ピクセルシェーダ)
	float4 lightingParam: SV_Target5;		//ライティング用パラメーター
};
PSOutput_RenderGBuffer PSMain_RenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	//アルベド
#if !defined(SKY_CUBE)
	//通常
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);	
#else
	//スカイボックス
	Out.albedo = skyCubeMap.SampleLevel(Sampler, In.cubemapPos, 0);
#endif
	Out.albedo.xyz = pow(Out.albedo.xyz, 2.2f);
	Out.albedo *= albedoScale;

	//αテスト
	if (Out.albedo.a > 0.5f) { 
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;
	}

	//法線
#if NORMAL_MAP
	Out.normal = NormalTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.normal = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
#else
	Out.normal = In.Normal;
#endif

	//ビュー座標
	Out.viewpos = float4(In.Viewpos.x, In.Viewpos.y, In.Viewpos.z + depthBias.y, In.curPos.z / In.curPos.w + depthBias.x);

	//ライティング用パラメーター
	Out.lightingParam.x = emissive;//エミッシブ
	Out.lightingParam.y = isLighting;//ライティングするか?
	Out.lightingParam.z = 0.0f;//メタリック
	Out.lightingParam.w = 0.38f;//シャイニネス

	//速度
#if MOTIONBLUR
		float2	current = In.curPos.xy / In.curPos.w;
		float2	last = In.lastPos.xy / In.lastPos.w;		

		if (In.lastPos.z < 0.0f) {
		//if (last.z < 0.0f || last.z > 1.0f) {
			Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
			Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y; 
			Out.velocityPS.z = -1.0f;
			Out.velocityPS.w = -1.0f;
			return Out;
			//discard; 
		}

		current.xy *= float2(0.5f, -0.5f); current.xy += 0.5f;
		last.xy *= float2(0.5f, -0.5f); last.xy += 0.5f;

		Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

		if (In.isWorldMove || In.curPos.z + depthBias.y < depthBias.z) {
			Out.velocity.xy = current.xy - last.xy;

			Out.velocityPS.z = max(In.curPos.z, In.lastPos.z) + depthBias.y;
			Out.velocityPS.w = -1.0f;

			//Out.albedo.r = 1.0f; Out.albedo.b = 0.0f; Out.albedo.g = 0.0f;
		}
		else {
			Out.velocityPS.xy = current.xy - last.xy;
			
			Out.velocityPS.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
			Out.velocityPS.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

			if (abs(Out.velocityPS.x) < BUNBO*0.5f && abs(Out.velocityPS.y) < BUNBO*0.5f) {
				Out.velocityPS.z = Out.velocityPS.w;
				Out.velocityPS.w = -1.0f;
			}

			//Out.albedo.r *= 0.1f; Out.albedo.b = 1.0f; Out.albedo.g *= 0.1f;
		}
#else
		Out.velocity.z = In.curPos.z + depthBias.y;
		Out.velocity.w = In.curPos.z + depthBias.y;
		Out.velocityPS.z = -1.0f;// In.curPos.z + depthBias.y;
		Out.velocityPS.w = -1.0f;// In.curPos.z + depthBias.y;
#endif

	return Out;
}

#if !defined(SKY_CUBE)
//Z値出力
float4 PSMain_RenderZ(ZPSInput In) : SV_Target0
{
#if defined(TEXTURE)
	//アルベド
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset).a * albedoScale.a;
#else
	float alpha = albedoScale.a;
#endif
	//αテスト
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}
	return In.posInProj.z / In.posInProj.w + depthBias.x ;// +1.0f*max(abs(ddx(In.posInProj.z / In.posInProj.w)), abs(ddy(In.posInProj.z / In.posInProj.w)));
}
#endif