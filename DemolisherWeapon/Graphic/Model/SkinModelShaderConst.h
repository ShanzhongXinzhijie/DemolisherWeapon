/*!
 *@brief	スキンモデル関係関係のシェーダーパラメータ。
 */

#pragma once

/*!
 *@brief	定数バッファのレジスタ番号
 */
enum EnSkinModelCBReg {
	enSkinModelCBReg_VSPS,		//!<頂点シェーダーとピクセルシェーダー共通の定数バッファ。
	enSkinModelCBReg_Material,	//!<マテリアルパラメータ。

	enSkinModelCBReg_Free,		//ここから使ってない
};

/*!
 *@brief	SRVのレジスタ番号。
 */
enum EnSkinModelSRVReg{
	enSkinModelSRVReg_AlbedoTexture,			//!<アルベドテクスチャ。
	enSkinModelSRVReg_NormalTexture,			//!<ノーマルマップ。
	enSkinModelSRVReg_LightngTexture,			//!<ライティングパラメータマップ。
	
	enSkinModelSRVReg_BoneMatrixArray,			//!<ボーン行列の配列。
	enSkinModelSRVReg_BoneMatrixArrayOld,
	enSkinModelSRVReg_InstancingWorldMatrix,	//インスタンシング用ワールド行列
	enSkinModelSRVReg_InstancingWorldMatrixOld, 
	
	enSkinModelSRVReg_Free = 7,								//ここから使ってない
	enSkinModelSRVReg_InstancingImposterTextureIndex = 7,	//ただし、インポスターのインスタンシング描画で使用する場合アリ

	enSkinModelSRVReg_Free2,								//ここからマジで使ってない
};
