#include "DWstdafx.h"
#include "ShaderPreloader.h"

namespace DemolisherWeapon::GameObj::Suicider {

	void ShaderPreloader::PreLoadShader() {
		Shader shader;
		SkinModelEffectShader SMEshader;

		//スカイボックス
		{
			D3D_SHADER_MACRO macros[] = { "SKY_CUBE", "1", "MOTIONBLUR", "1", NULL, NULL };
			shader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "SKY_CUBE", macros);
			shader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS, "SKY_CUBE", macros);
		}

		//ビルボード
		{
			D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			shader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
			shader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "INSTANCING", macrosVS);
		}
		{
			D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
			shader.Load("Preset/shader/billboard.fx", "VSMain_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
			shader.Load("Preset/shader/billboard.fx", "VSMain_RenderZ_Billboard", Shader::EnType::VS, "NORMAL", macrosVS);
		}

		//インスタンシング
		{
			D3D_SHADER_MACRO macros[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			shader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS, "INSTANCING", macros);
			shader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
			shader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS, "INSTANCING", macros);
			shader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS, "INSTANCING", macros);
		}

		//モデル通常頂点シェーダ
		{
			SMEshader.Load("Preset/shader/model.fx", "VSMain", Shader::EnType::VS);
			shader.Load("Preset/shader/model.fx", "VSMain_RenderZ", Shader::EnType::VS);
			SMEshader.Load("Preset/shader/model.fx", "VSMainSkin", Shader::EnType::VS);
			shader.Load("Preset/shader/model.fx", "VSMainSkin_RenderZ", Shader::EnType::VS);
		}
		//モデル通常ピクセルシェーダ
		{
			SMEshader.Load("Preset/shader/model.fx", "PSMain_RenderGBuffer", Shader::EnType::PS);
			SMEshader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS);
			D3D_SHADER_MACRO macrosYOnly[] = { "Y_ONLY", "1", NULL, NULL };
			SMEshader.Load("Preset/shader/TriPlanarMapping.fx", "PS_TriPlanarMapping", Shader::EnType::PS, "Y_ONLY", macrosYOnly);
		}
		//モデル通常深度値シェーダ
		{
			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
			shader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS);
			shader.Load("Preset/shader/model.fx", "PSMain_RenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		}

		//ポストドローシェーダ
		{
			//マクロ
			D3D_SHADER_MACRO macros[3] = { NULL,NULL,NULL,NULL,NULL,NULL }, macrosTex[3] = { "TEXTURE","1",NULL,NULL,NULL,NULL };
			char shaderName[64] = "DEFAULT", shaderNameTex[64] = "TEXTURE";
			
			for (int i = 0; i < 2; ++i) {
				//ソフトパーティクル設定
				if (i==0) {
					macros[0].Name = "SOFT_PARTICLE";
					macros[0].Definition = "1";
					strcpy_s(shaderName, "SOFT_PARTICLE");
					macrosTex[1].Name = "SOFT_PARTICLE";
					macrosTex[1].Definition = "1";
					strcpy_s(shaderNameTex, "TEXTURE+SOFT_PARTICLE");
				}
				if (i==1) {
					macros[0].Name = "REV_SOFT_PARTICLE";
					macros[0].Definition = "1";
					strcpy_s(shaderName, "REV_SOFT_PARTICLE");
					macrosTex[1].Name = "REV_SOFT_PARTICLE";
					macrosTex[1].Definition = "1";
					strcpy_s(shaderNameTex, "TEXTURE+REV_SOFT_PARTICLE");
				}
				//乗算済みアルファに変換する
				shader.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderName, macros);
				shader.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi_ConvertToPMA", Shader::EnType::PS, shaderNameTex, macrosTex);
				//通常
				shader.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderName, macros);
				shader.Load("Preset/shader/model.fx", "PSMain_SozaiNoAzi", Shader::EnType::PS, shaderNameTex, macrosTex);
			}
		}

		//インポスター
		{
			//インスタンシング用シェーダ
			D3D_SHADER_MACRO macrosVS[] = { "INSTANCING", "1", "ALL_VS", "1", NULL, NULL };
			shader.Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);
			shader.Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "INSTANCING", macrosVS);

			D3D_SHADER_MACRO macros[] = {
				"INSTANCING", "1",
				NULL, NULL
			};
			SMEshader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS, "INSTANCING", macros);

			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1","INSTANCING", "1", NULL, NULL };
			shader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE_INSTANCING", macrosZ);
		}
		{
			D3D_SHADER_MACRO macrosVS[] = { "ALL_VS", "1", NULL, NULL };
			shader.Load("Preset/shader/Imposter.fx", "VSMain_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);
			shader.Load("Preset/shader/Imposter.fx", "VSMain_RenderZ_Imposter", Shader::EnType::VS, "NORMAL", macrosVS);

			SMEshader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderGBuffer", Shader::EnType::PS);

			D3D_SHADER_MACRO macrosZ[] = { "TEXTURE", "1", NULL, NULL };
			shader.Load("Preset/shader/Imposter.fx", "PSMain_ImposterRenderZ", Shader::EnType::PS, "TEXTURE", macrosZ);
		}
	}

	void ShaderPreloader::PreLoopUpdate() {
		if (m_isDrawed) {//ロード画面描画済みなら
			//シェーダーのロード
			PreLoadShader();
			//死ぬ
			delete this; return;
		}
	}

	//void ShaderPreloader::PostLoopUpdate() {
	//	DrawQuad2D(0.0f, 1.0f, {0.0f,0.3f,0.95f,0.6f});
	//}

	void ShaderPreloader::PostRender() {
		CFont font;
		font.ChangeFont(GetEngine().GetGraphicsEngine().GetSpriteFont());
		font.Draw(
			L"ヴキクォリッルシハーグエを実行中\n"
			L"初回起動時は時間がかかります...(すごく)"
			, 0.5f, 1.0f, 0.5f, 0.5f, 0.0f, DirectX::SpriteEffects_None, 0.0f
		);
		m_isDrawed = true;//描画完了
	}

}