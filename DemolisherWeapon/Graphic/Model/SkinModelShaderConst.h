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
};
/*enum EnSkinModelCBReg{
	enSkinModelCBReg_VSPS,		//!<頂点シェーダーとピクセルシェーダー共通の定数バッファ。
	enSkinModelCBReg_Light,		//!<ライト用の定数バッファ。
	enSkinModelCBReg_Material,	//!<マテリアルパラメータ。
	enSkinModelCBReg_Shadow,	//!<影。
	enSkinModelCBReg_GBuffer,	//!<G-Buffer。
	
};*/

/*!
 *@brief	SRVのレジスタ番号。
 */
enum EnSkinModelSRVReg{
	enSkinModelSRVReg_AlbedoTexture,		//!<アルベドテクスチャ。
	enSkinModelSRVReg_BoneMatrixArray,		//!<ボーン行列の配列。
	enSkinModelSRVReg_BoneMatrixArrayOld,
};
