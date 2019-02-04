/*!
 * @brief	モデルシェーダー。
 */


/////////////////////////////////////////////////////////////
// Shader Resource View
/////////////////////////////////////////////////////////////
//アルベドテクスチャ。
Texture2D<float4> albedoTexture : register(t0);	
//ボーン行列
StructuredBuffer<float4x4> boneMatrix : register(t1);
StructuredBuffer<float4x4> boneMatrixOld : register(t2);

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

	float4 depthBias;
	//float depthBiasScaleFromNearToFar;
};

//マテリアルパラメーター
cbuffer MaterialCb : register(b1) {
	float4 albedoScale;	//アルベドにかけるスケール
	float4 emissive;	//エミッシブ(自己発光) wがライティングするか
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
	float2 TexCoord 	: TEXCOORD0;
	float3 Viewpos		: TEXCOORD1;

	float4	curPos		: CUR_POSITION;//現在座標
	float4	lastPos		: LAST_POSITION;//過去座標
	bool isWorldMove	: IS_WORLD_BLUR;//ワールド空間で移動しているか?
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
PSInput VSMain( VSInputNmTxVcTangent In ) 
{
	PSInput psInput = (PSInput)0;
	float4 pos = mul(mWorld, In.Position); float3 posW = pos.xyz;
	pos = mul(mView, pos); psInput.Viewpos = pos.xyz;
	pos = mul(mProj, pos);
	psInput.Position = pos;
	psInput.TexCoord = In.TexCoord;
	psInput.Normal = normalize(mul(mWorld, In.Normal));
	psInput.Tangent = normalize(mul(mWorld, In.Tangent));

	psInput.curPos = pos;

	//ベロシティマップ用情報
#if MOTIONBLUR
		float4 oldpos = mul(mWorld_old, In.Position);

		if (distance((mView[3].xyz - mView_old[3].xyz), (mWorld[3].xyz - mWorld_old[3].xyz)) > 0.0f //distance(posW, oldpos.xyz)
			|| distance(mWorld[0].xyz, mWorld_old[0].xyz) > 0.0f
			|| distance(mWorld[1].xyz, mWorld_old[1].xyz) > 0.0f
			|| distance(mWorld[2].xyz, mWorld_old[2].xyz) > 0.0f){
			psInput.isWorldMove = true;
		}

		oldpos = mul(mView_old, oldpos);
		oldpos = mul(mProj_old, oldpos);
		
		if (oldpos.z < 0.0f) {
			psInput.lastPos = pos;
		}
		else {
			psInput.lastPos = oldpos;
		}
#endif

	return psInput;
}
//Z値書き込み用
ZPSInput VSMain_RenderZ(VSInputNmTxVcTangent In)
{
	ZPSInput psInput = (ZPSInput)0;
	float4 pos = mul(mWorld, In.Position);
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
PSInput VSMainSkin( VSInputNmTxWeights In ) 
{
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
	  	//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
		//mulは乗算命令。
	    pos = mul(skinning, In.Position);
	}
	float3 posW = pos.xyz;
	psInput.Normal = normalize( mul(skinning, In.Normal) );
	psInput.Tangent = normalize( mul(skinning, In.Tangent) );
	
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
			oldpos = mul(oldskinning, In.Position);
		}

		if (distance((mView[3].xyz - mView_old[3].xyz), (skinning[3].xyz - oldskinning[3].xyz)) > 0.0f
			|| distance(skinning[0].xyz, oldskinning[0].xyz) > 0.0f
			|| distance(skinning[1].xyz, oldskinning[1].xyz) > 0.0f
			|| distance(skinning[2].xyz, oldskinning[2].xyz) > 0.0f) {
			psInput.isWorldMove = true;
		}

		oldpos = mul(mView_old, oldpos);
		oldpos = mul(mProj_old, oldpos);

		if (oldpos.z < 0.0f){
			psInput.lastPos = pos;
		}
		else {
			psInput.lastPos = oldpos;
		}
#endif

    return psInput;
}
//Z値書き込み用
ZPSInput VSMainSkin_RenderZ(VSInputNmTxWeights In)
{
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
		//頂点座標にスキン行列を乗算して、頂点をワールド空間に変換。
		//mulは乗算命令。
		pos = mul(skinning, In.Position);
	}

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
	Out.albedo = albedoTexture.Sample(Sampler, In.TexCoord) * albedoScale;

	//αテスト
	if (Out.albedo.a > 0.5f) { 
		Out.albedo.a = 1.0f;//半透明無効
	}
	else {
		discard;
	}

	//法線
	Out.normal = In.Normal;

	//ビュー座標
	Out.viewpos = float4(In.Viewpos, In.curPos.z / In.curPos.w + depthBias.x);

	//ライティング用パラメーター
	Out.lightingParam = emissive;

	//速度
#if MOTIONBLUR

		float3	current = In.curPos.xyz / In.curPos.w;
		float3	last = In.lastPos.xyz / In.lastPos.w;

		if (In.lastPos.z < 0.0f) {// || last.z < 0.0f || last.z > 1.0f) {			
			current *= 0.0f; last *= 0.0f;
		}

		current.xy *= float2(0.5f, -0.5f); current.xy += 0.5f;
		last.xy *= float2(0.5f, -0.5f); last.xy += 0.5f;

		Out.velocity.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
		Out.velocity.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;

		if (In.isWorldMove) {
			Out.velocity.xy = current.xy - last.xy;
			Out.velocityPS.z = -1.0f;
			Out.velocityPS.w = -1.0f;
		}
		else {
			Out.velocityPS.xy = current.xy - last.xy;
			Out.velocityPS.z = min(In.curPos.z, In.lastPos.z) + depthBias.y;
			Out.velocityPS.w = max(In.curPos.z, In.lastPos.z) + depthBias.y;
		}
#else
		Out.velocity.z = In.curPos.z + depthBias.y;
		Out.velocity.w = In.curPos.z + depthBias.y;
		Out.velocityPS.z = In.curPos.z + depthBias.y;
		Out.velocityPS.w = In.curPos.z + depthBias.y;
#endif

	return Out;
}

//Z値出力
float4 PSMain_RenderZ(ZPSInput In) : SV_Target0
{
	//アルベド
	float alpha = albedoTexture.Sample(Sampler, In.TexCoord).a * albedoScale.a;

	//αテスト
	if (alpha > 0.5f) {
	}
	else {
		discard;
	}

	return In.posInProj.z / In.posInProj.w + depthBias.x;
}