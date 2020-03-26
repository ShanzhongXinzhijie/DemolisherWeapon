/*!
 * @brief	モデルシェーダー。
 */
#ifndef DW_MODEL_FX
#define DW_MODEL_FX

#include"MotionBlurHeader.h"

#if defined(ALL_VS)
#define MOTIONBLUR 1
#define NORMAL_MAP 1
#endif

/////////////////////////////////////////////////////////////
// Shader Resource View
// [SkinModelShaderConst.h:EnSkinModelSRVReg]
/////////////////////////////////////////////////////////////

//テクスチャ
#if ALBEDO_MAP || defined(TEXTURE) || defined(SKY_CUBE)
#if !defined(SKY_CUBE)
//アルベドテクスチャ
Texture2D<float4> albedoTexture : register(t0);	
#else
//スカイボックス用キューブマップ
TextureCube<float4> skyCubeMap : register(t0);
#endif
#endif

#if NORMAL_MAP
//ノーマルマップ
Texture2D<float3> NormalTexture : register(t1);
#endif

#if LIGHTING_MAP
//ライティングパラメータマップ
Texture2D<float4> LightingTexture : register(t2);
#endif

#if TRANSLUCENT_MAP
//トランスルーセントマップ
Texture2D<float> TranslucentTexture : register(t11);
#endif

#if defined(SOFT_PARTICLE) || defined(REV_SOFT_PARTICLE)
//ビュー座標テクスチャ(wが深度値)
Texture2D<float4> ViewPosTexture : register(t12);
#endif

//ストラクチャーバッファ

//ボーン行列
StructuredBuffer<float4x4> boneMatrix : register(t3);
StructuredBuffer<float4x4> boneMatrixOld : register(t4);
#if defined(INSTANCING)
//インスタンシング用ワールド行列
StructuredBuffer<float4x4> InstancingWorldMatrix : register(t5);
StructuredBuffer<float4x4> InstancingWorldMatrixOld : register(t6);
#endif

/////////////////////////////////////////////////////////////
// SamplerState
/////////////////////////////////////////////////////////////
sampler Sampler : register(s0);

/////////////////////////////////////////////////////////////
// 定数バッファ。
// [SkinModelShaderConst.h:EnSkinModelCBReg]
/////////////////////////////////////////////////////////////
/*!
 * @brief	頂点シェーダーとピクセルシェーダー用の定数バッファ。
 * [SkinModel.h:SVSConstantBuffer]
 */
cbuffer VSPSCb : register(b0){
	//行列
	float4x4 mWorld;
	float4x4 mView;
	float4x4 mProj;

	//前回の行列
	float4x4 mWorld_old;
	float4x4 mView_old;
	float4x4 mProj_old;

	//カメラの移動量
	float4 camMoveVec;//w:しきい値≒距離スケール

	//深度値のバイアス
	float4 depthBias;//x:max=(1.0f) y:max=(far-near) z:ブラーの近距離しきい値

	//カメラのワールド座標
	float3 camWorldPos;

	//なんか
    float cb_t;

	//インポスター用
	int2 imposterPartNum;//分割数
	float2 imposterParameter;//x:スケール,y:Y軸回転

    //カメラのNear(x)・Far(y)
	float2 nearFar;

	//ソフトパーティクルが有効になる範囲
    float softParticleArea;
	
	//モーションブラースケール
	float MotionBlurScale;
};

//定数バッファ　[MaterialSetting.h:MaterialParam]
//マテリアルパラメーター
cbuffer MaterialCb : register(b1) {
	float4 albedoScale;	//アルベドにかけるスケール
	float  emissive;	//自己発光
	float  isLighting;	//ライティングするか
	float  metallic;	//メタリック
	float  shininess;	//シャイネス(ラフネスの逆)
	float2 uvOffset;	//UV座標オフセット
	float  triPlanarMapUVScale;//TriPlanarMapping時のUV座標へのスケール
    float  translucent; //トランスルーセント(光の透過具合)
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
	//float3 Binormal : BINORMAL;			//従法線。
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
	//float3 Binormal : BINORMAL;			//従法線。
	uint4  Indices  : BLENDINDICES0;		//この頂点に関連付けされているボーン番号。x,y,z,wの要素に入っている。4ボーンスキニング。
    float4 Weights  : BLENDWEIGHT0;			//この頂点に関連付けされているボーンへのスキンウェイト。x,y,z,wの要素に入っている。4ボーンスキニング。
};

/*!
 * @brief	ピクセルシェーダーの入力。
 */
struct PSInput{
	//基本情報
	float4 Position 	: SV_POSITION;
	float3 Normal		: NORMAL;
	float3 Tangent		: TANGENT;
	float3 Binormal		: BINORMAL;
	float2 TexCoord 	: TEXCOORD0;

	//座標
	float3 Viewpos		: TEXCOORD1;
	float3 Localpos		: TEXCOORD2;
    float3 Worldpos		: TEXCOORD3;

	float4 curPos		: CUR_POSITION;//現在座標
	float4 lastPos		: LAST_POSITION;//過去座標
	bool isWorldMove	: IS_WORLD_BLUR;//ワールド空間で移動しているか?

	uint instanceID		: InstanceID;//インスタンスID

	float3 cubemapPos	: CUBE_POS;//スカイボックスのテクスチャ取得用

	int2 imposterIndex	: IMPOSTER_INDEX;//インポスター用インデックス
};

//Z値書き込みピクセルシェーダーの入力
struct ZPSInput {
	//基本情報
	float4 Position 	: SV_POSITION;
	float2 TexCoord 	: TEXCOORD0;
	float4 posInProj	: TEXCOORD1;

	uint instanceID		: InstanceID;//インスタンスID

	int2 imposterIndex	: IMPOSTER_INDEX;//インポスター用インデックス
};

//G-Buffer出力
struct PSOutput_RenderGBuffer {
	float4 albedo		: SV_Target0;		//アルベド
	float4 normal		: SV_Target1;		//法線
	float4 viewpos		: SV_Target2;		//ビュー座標
	float4 velocity		: SV_Target3;		//速度
	float4 velocityPS	: SV_Target4;		//速度(ピクセルシェーダ)
	float4 lightingParam: SV_Target5;		//ライティング用パラメーター
};

/////////////////////////////////////////////////////////////
// 関数
/////////////////////////////////////////////////////////////
//スキンなしモデルの頂点シェーダ用関数
PSInput VSModel( VSInputNmTxVcTangent In, float4x4 worldMat, float4x4 worldMatOld
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
){
	PSInput psInput = (PSInput)0;

#if defined(INSTANCING)
	//インスタンスID記録
	psInput.instanceID = instanceID;
#endif

	//ワールド行列適応
	float4 pos = mul(worldMat, In.Position);
	float3 posW = pos.xyz;
    psInput.Localpos = In.Position;
    psInput.Worldpos = posW;

	//スカイボックス用情報
#if defined(SKY_CUBE)
	psInput.cubemapPos = normalize(posW - camWorldPos);
#endif

	//ビュープロジェクション行列適応
	pos = mul(mView, pos); psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);

	//設定
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;

	//法線情報
	psInput.Normal = normalize(mul(worldMat, In.Normal));
#if NORMAL_MAP
	psInput.Tangent = normalize(mul(worldMat, In.Tangent));
	psInput.Binormal = normalize(cross(psInput.Normal, psInput.Tangent));// normalize(mul(worldMat, In.Binormal));
#endif

	//変換後座標
	psInput.curPos = pos;

	//ベロシティマップ用情報
#if MOTIONBLUR
	float4 oldpos = mul(worldMatOld, In.Position);
	oldpos.xyz = lerp(posW, oldpos.xyz, MotionBlurScale);

	float3 trans = float3(worldMat._m03, worldMat._m13, worldMat._m23);
	float3 transOld = float3(worldMatOld._m03, worldMatOld._m13, worldMatOld._m23);
	transOld = lerp(trans, transOld, MotionBlurScale);
	trans -= transOld;

	if (length(trans) > camMoveVec.w
		&& distance(camMoveVec.xyz, trans) > camMoveVec.w
		|| distance(float3(worldMat._m00, worldMat._m10, worldMat._m20), float3(worldMatOld._m00, worldMatOld._m10, worldMatOld._m20)) > 0.0f
		|| distance(float3(worldMat._m01, worldMat._m11, worldMat._m21), float3(worldMatOld._m01, worldMatOld._m11, worldMatOld._m21)) > 0.0f
		|| distance(float3(worldMat._m02, worldMat._m12, worldMat._m22), float3(worldMatOld._m02, worldMatOld._m12, worldMatOld._m22)) > 0.0f
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

/*!--------------------------------------------------------------------------------------
 * @brief	スキンなしモデル用の頂点シェーダー。
-------------------------------------------------------------------------------------- */
PSInput VSMain(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
#if defined(INSTANCING)
	return VSModel(In, InstancingWorldMatrix[instanceID], InstancingWorldMatrixOld[instanceID], instanceID);
#else
	return VSModel(In, mWorld, mWorld_old);
#endif 
}

//Z値書き込み用スキンなしモデル頂点シェーダ用関数
ZPSInput VSModel_RenderZ(VSInputNmTxVcTangent In, float4x4 worldMat
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
	ZPSInput psInput = (ZPSInput)0;

#if defined(INSTANCING)
	//インスタンスID記録
	psInput.instanceID = instanceID;
#endif

	float4 pos = mul(worldMat, In.Position);
	pos = mul(mView, pos);
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.posInProj = pos;
	psInput.TexCoord = In.TexCoord;

	return psInput;
}

//Z値書き込み用スキンなしモデル頂点シェーダー
ZPSInput VSMain_RenderZ(VSInputNmTxVcTangent In
#if defined(INSTANCING)
	, uint instanceID : SV_InstanceID
#endif 
) {
#if defined(INSTANCING)
	return VSModel_RenderZ(In, InstancingWorldMatrix[instanceID], instanceID);
#else
	return VSModel_RenderZ(In, mWorld);
#endif 
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
	//インスタンスID記録
	psInput.instanceID = instanceID;
	//スキン行列とワールド行列を乗算
	skinning = mul(InstancingWorldMatrix[instanceID], skinning);
#endif
	
	//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
	//mulは乗算命令。
	pos = mul(skinning, In.Position);
	psInput.Normal = normalize( mul(skinning, In.Normal) );
#if NORMAL_MAP
	psInput.Tangent = normalize( mul(skinning, In.Tangent) );
	psInput.Binormal = normalize(cross(psInput.Normal, psInput.Tangent));//normalize( mul(skinning, In.Binormal) );
#endif

	float3 posW = pos.xyz; 
#if defined(SKY_CUBE)
	psInput.cubemapPos = normalize(posW - camWorldPos);
#endif
    psInput.Localpos = In.Position;
    psInput.Worldpos = posW;

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
	//インスタンスID記録
	psInput.instanceID = instanceID;
	//スキン行列とワールド行列を乗算
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

void AlbedoRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//アルベド
#if ALBEDO_MAP || defined(SKY_CUBE)
#if !defined(SKY_CUBE)
	//通常
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset);
#else
	//スカイボックス
	Out.albedo = skyCubeMap.SampleLevel(Sampler, In.cubemapPos, 0);
#endif
	Out.albedo.xyz = pow(Out.albedo.xyz, 2.2f);//リニア空間に変換
	Out.albedo *= albedoScale;//スケールをかける
#else
	//アルベドテクスチャがない場合はスケールをそのまま使う
	Out.albedo = albedoScale;
#endif
}
void NormalRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//法線
#if NORMAL_MAP
	Out.normal.xyz = NormalTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.normal.xyz = Out.normal.xyz * 2.0f - 1.0f;
	Out.normal.xyz = Out.normal.x * In.Tangent + Out.normal.y * In.Binormal + Out.normal.z * In.Normal;
	Out.normal.xyz = normalize(Out.normal.xyz);
#else
	Out.normal.xyz = In.Normal;
#endif
}
void PosRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//ビュー座標
	Out.viewpos = float4(In.Viewpos.x, In.Viewpos.y, In.Viewpos.z + depthBias.y, In.curPos.z / In.curPos.w + depthBias.x);
}
void ParamRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//ライティング用パラメーター
#if LIGHTING_MAP
	//x:エミッシブ y:メタリック z:シャイニネス w:ライティングするか?
	float4 lightMap = LightingTexture.Sample(Sampler, In.TexCoord + uvOffset);
	Out.lightingParam.x = lightMap.x * emissive;	//エミッシブ
	Out.lightingParam.y = lightMap.w * isLighting;	//ライティングするか?
	Out.lightingParam.z = lightMap.y * metallic;	//メタリック
	Out.lightingParam.w = lightMap.z * shininess;	//シャイニネス
#else
	Out.lightingParam.x = emissive;		//エミッシブ
	Out.lightingParam.y = isLighting;	//ライティングするか?
	Out.lightingParam.z = metallic;		//メタリック
	Out.lightingParam.w = shininess;	//シャイニネス
#endif
}
void MotionRender(in PSInput In, inout PSOutput_RenderGBuffer Out) {
	//速度
#if MOTIONBLUR
	float2	current = In.curPos.xy / In.curPos.w;
	float2	last = In.lastPos.xy / In.lastPos.w;

	//無効
	if (In.lastPos.z < 0.0f) {
		//if (last.z < 0.0f || last.z > 1.0f) {
		Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.z = -1.0f;
		Out.velocityPS.w = -1.0f;
		return;//return Out;
		//discard; 
	}

	current.xy *= float2(0.5f, -0.5f); current.xy += 0.5f;
	last.xy *= float2(0.5f, -0.5f); last.xy += 0.5f;

	Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
	Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

	//オブジェクトが動いている or カメラに近い
	if (In.isWorldMove || In.curPos.z + depthBias.y < depthBias.z) {
		Out.velocity.xy = current.xy - last.xy;

		Out.velocityPS.z = max(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.w = -1.0f;//PSブラーしない

		//Out.albedo.r = 1.0f; Out.albedo.b = 0.0f; Out.albedo.g = 0.0f;
	}
	else {
		Out.velocityPS.xy = current.xy - last.xy;

		Out.velocityPS.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocityPS.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

		//動きが小さい
		if (abs(Out.velocityPS.x) < BUNBO*0.5f && abs(Out.velocityPS.y) < BUNBO*0.5f) {
			Out.velocityPS.z = Out.velocityPS.w;
			Out.velocityPS.w = -1.0f;//PSブラーしない
		}

		//Out.albedo.r *= 0.1f; Out.albedo.b = 1.0f; Out.albedo.g *= 0.1f;
	}
#else
	Out.velocity.z = In.curPos.z + depthBias.y;
	Out.velocity.w = In.curPos.z + depthBias.y;
	Out.velocityPS.z = -1.0f;// In.curPos.z + depthBias.y;
	Out.velocityPS.w = -1.0f;// In.curPos.z + depthBias.y;
#endif
}
void TranslucentRender(in PSInput In, inout PSOutput_RenderGBuffer Out)
{
#if TRANSLUCENT_MAP
    Out.normal.a = TranslucentTexture.Sample(Sampler, In.TexCoord + uvOffset) * translucent;
#else
    Out.normal.a = translucent;
#endif
}

//GBuffer出力
PSOutput_RenderGBuffer PSMain_RenderGBuffer(PSInput In)
{
	PSOutput_RenderGBuffer Out = (PSOutput_RenderGBuffer)0;

	AlbedoRender(In,Out);

	//αテスト
	if (Out.albedo.a > 0.5f) { 
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;//描画しない
	}

	NormalRender(In, Out);

	PosRender(In, Out);

	ParamRender(In, Out);

	MotionRender(In, Out);

    TranslucentRender(In, Out);

//#if defined(SKY_CUBE)
//	if(In.cubemapPos.y<0.0f){Out.lightingParam.y=1.0f;}
//#endif

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

//デプス値を線形に変換
float LinearizeDepth(float depth, float near, float far)
{
    return (2.0 * near) / (far + near - depth * (far - near));
}

//素材の味シェーダ
float4 SozaiNoAziInner(PSInput In)
{
    float4 Out;

#if defined(TEXTURE)
	Out = albedoTexture.Sample(Sampler, In.TexCoord + uvOffset) * albedoScale;
#else
    Out = albedoScale;
#endif
	
    //ソフトパーティクル    
#if defined(SOFT_PARTICLE)
	//モデル深度値算出
    float depth = In.Viewpos.z;
	//書き込み先深度値算出
    float2 coord = (In.curPos.xy / In.curPos.w) * float2(0.5f, -0.5f) + 0.5f;
    float screenDepth = ViewPosTexture.Sample(Sampler, coord).z;
	//深度の差算出
    depth = distance(depth, screenDepth);
	//深度の差がsoftParticleArea以下なら透明化
    if (depth <= softParticleArea)
    {
        depth /= softParticleArea;
        Out.a *= depth;
    }
#endif
    
    //逆ソフトパーティクル
#if defined(REV_SOFT_PARTICLE)
	//モデル深度値算出
    float depth = In.Viewpos.z;
	//書き込み先深度値算出
    float2 coord = (In.curPos.xy / In.curPos.w) * float2(0.5f, -0.5f) + 0.5f;
    float screenDepth = ViewPosTexture.Sample(Sampler, coord).z;
	//深度の差算出
    depth = distance(depth, screenDepth);
    //透明化
	depth = max(0.0f,1.0f - (depth / softParticleArea));
    Out.a *= depth;
#endif

    return Out;
}
//モデルのアルベドをそのまま出すシェーダ
float4 PSMain_SozaiNoAzi(PSInput In) : SV_Target0{
    return SozaiNoAziInner(In);
}
//乗算済みアルファに変換する版
float4 PSMain_SozaiNoAzi_ConvertToPMA(PSInput In) : SV_Target0{
    float4 color = SozaiNoAziInner(In);
	color.rgb *= color.a;
	return color;
}

#endif //DW_MODEL_FX