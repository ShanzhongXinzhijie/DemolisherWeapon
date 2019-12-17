#ifndef DW_MODEL_DISOLVE_FX
#define DW_MODEL_DISOLVE_FX

#include"model.fx"

//しきい値テクスチャ
Texture2D<float> DisolveTexture : register(t9);
//ディゾルブシェーダ用しきい値
StructuredBuffer<float> Disolve_t : register(t10);

//ディゾルブシェーダ(GBuffer)
PSOutput_RenderGBuffer PSMain_DisolveGBuffer(PSInput In)
{
	if (DisolveTexture.Sample(Sampler, In.TexCoord) > Disolve_t[In.instanceID]) {
		return PSMain_RenderGBuffer(In);//通常シェーディング
	}
	
	discard;//死	
    return (PSOutput_RenderGBuffer) 0;
}
//ディゾルブシェーダ(Z値出力)
float4 PSMain_DisolveZ(ZPSInput In) : SV_Target0
{
	if (DisolveTexture.Sample(Sampler, In.TexCoord) > Disolve_t[In.instanceID]) {
		return PSMain_RenderZ(In);//通常シェーディング
	}
	
    discard;//死
    return (float4) 0;
}

//乗算済みアルファに変換する版
float4 PSMain_DisolveSozaiNoAzi_ConvertToPMA(PSInput In) : SV_Target0
{
   // if (DisolveTexture.Sample(Sampler, In.TexCoord) > Disolve_t[In.instanceID])
   // {
        float4 re =  PSMain_SozaiNoAzi_ConvertToPMA(In);
        re *= saturate(1.0f - (Disolve_t[In.instanceID] / DisolveTexture.Sample(Sampler, In.TexCoord)));
        return re;
   // }

   // discard; //死
   // return (float4) 0;
}

#endif //DW_MODEL_DISOLVE_FX