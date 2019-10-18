/*!
 *@brief	スキンモデル関係関係のシェーダーパラメータ。
 */

#pragma once

/*!
 *@brief	定数バッファのレジスタ番号
 */
enum EnSkinModelCBReg {
	enSkinModelCBReg_VSPS,		//頂点シェーダーとピクセルシェーダー共通の定数バッファ。
	enSkinModelCBReg_Material,	//マテリアルパラメータ。

	enSkinModelCBReg_Free,		//ここから使ってない
};

/*!
 *@brief	SRVのレジスタ番号。
 */
enum EnSkinModelSRVReg{
	enSkinModelSRVReg_AlbedoTexture,			//アルベドテクスチャ。
	enSkinModelSRVReg_NormalTexture,			//ノーマルマップ。
	enSkinModelSRVReg_LightngTexture,			//ライティングパラメータマップ。
	
	enSkinModelSRVReg_BoneMatrixArray,			//ボーン行列の配列。
	enSkinModelSRVReg_BoneMatrixArrayOld,
	enSkinModelSRVReg_InstancingWorldMatrix,	//インスタンシング用ワールド行列
	enSkinModelSRVReg_InstancingWorldMatrixOld, 
	
	enSkinModelSRVReg_Free = 7,					//遺物
	enSkinModelSRVReg_ImposterSizeToCamera = 7,	//インポスターのインデックスごとのカメラ方向へのモデルサイズ
	enSkinModelSRVReg_InstancingImposterScale,	//インスタンシング用インポスター拡大率

	enSkinModelSRVReg_DisolveTexture,			//ディゾルブシェーダ、しきい値テクスチャ
	enSkinModelSRVReg_Disolve_t,				//モデルごとのディゾルブシェーダ用しきい値

	enSkinModelSRVReg_TranslucentTexture,		//トランスルーセントマップ

	enSkinModelSRVReg_ViewPosTexture,			//ビュー座標テクスチャ

	enSkinModelSRVReg_Free2 = 60,				//ここから使ってない
};
