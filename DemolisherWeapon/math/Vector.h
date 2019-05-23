/*!
 * @brief	ベクトル
 * @details
 * DirectMathを使いやすくしたベクトルクラス。
 */

#pragma once

#include <Effekseer.h>

#include "kMath.h"

namespace DemolisherWeapon{

class CMatrix;

class CVector2 {
public:
	CVector2()
	{
		x = y = 0.0f;
	}
	CVector2(float x, float y)
	{
		this->x = x;
		this->y = y;
	}
	union {
		DirectX::XMFLOAT2 vec;
		struct { float x, y; };
		float v[2];
	};

	/*!
	*@brief	代入演算子。
	*/
	CVector2& operator=(const CVector2& _v)
	{
		vec = _v.vec;
		return *this;
	}
	/*!
	 * @brief	ベクトルを加算。
	 */
	void Add(const CVector2& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	void Add(const CVector2& v0, const CVector2& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	/*!
	 * @brief	ベクトルを減算。
	 */
	void Subtract(const CVector2& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	void Subtract(const CVector2& v0, const CVector2& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	/*!
	* @brief	拡大。
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat2(&vec, xmv);
	}
	//ベクトル同士の乗算。
	void Multiply(const CVector2& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	void Multiply(const CVector2& v0, const CVector2& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat2(&vec, xmvr);
	}
	/*!
	* @brief	除算。
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}

	/*!
	 * @brief	長さを取得
	 */
	float Length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVector2Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	長さの二乗を取得
	 */
	float LengthSq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVector2LengthSq(xmv).m128_f32[0];
	}

	/*!
	* @brief	法線を正規化。
	*/
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVector2Normalize(xmv);
		DirectX::XMStoreFloat2(&vec, xmv);
	}
	CVector2 GetNorm() const {
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVector2Normalize(xmv);
		CVector2 re;
		DirectX::XMStoreFloat2(&re.vec, xmv);

		return re;
	}

	float Cross(const CVector2& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector2Cross(xmv0, xmv1);
		float re;
		DirectX::XMStoreFloat(&re, xmvr);
		return re;
	}

	/*!
	 *@brief	加算代入演算子。
	 */
	const CVector2& operator+=(const CVector2& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief　乗算代入演算子。
	*/
	const CVector2& operator*=(float s)
	{
		Scale(s);
		return *this;
	}
	//乗算代入演算子(ベクトル同士)。
	const CVector2& operator*=(const CVector2& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	減算代入演算子。
	*/
	const CVector2& operator-=(const CVector2& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	除算代入演算子。
	 */
	const CVector2& operator/=(const float s)
	{
		Div(s);
		return *this;
	}

	/*!
	* @brief	線形補間。
	*@details
	* this = v0 + (v1-v0) * t;
	*/
	void Lerp(float t, const CVector2& v0, const CVector2& v1)
	{
		x = v0.x + (v1.x - v0.x) * t;
		y = v0.y + (v1.y - v0.y) * t;
	}
	static CVector2 Zero()
	{
		static const CVector2 zero = { 0.0f,  0.0f };
		return zero;
	}
	static CVector2 One()
	{
		static const CVector2 one = { 1.0f,  1.0f };
		return one;
	}

	//構造化束縛のやつ
	template <std::size_t N>
	float get() const {
		if constexpr (N == 0)
			return x;
		else
			return y;
	}
};
/*!
 * @brief	ベクトル。
 */
class CVector3{
public:
	union{
		DirectX::XMFLOAT3 vec;
		float v[3];
		struct { float x, y, z; };
	};
	//static const CVector3 Zero;

public:
	//XMVECTORへの暗黙の変換。
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat3(&vec);
	}
	operator Effekseer::Vector3D() const
	{
		return Effekseer::Vector3D(x,y,z);
	}
	operator CVector2() const
	{
		return CVector2(x, y);
	}
	operator btVector3() const
	{
		return btVector3(x, y, z);
	}
	//operator D3DXVECTOR3(void) { return s_cast<D3DXVECTOR3>(*this); }

	/*!
	*@brief	代入演算子。
	*/
	CVector3& operator=(const CVector3& _v)
	{
		vec = _v.vec;
		return *this;
	}
	template<class TVector>
	CVector3& operator=(const TVector& _v)
	{
		vec = _v.vec;
		return *this;
	}
	template<>
	CVector3& operator=(const CVector2& _v)
	{
		vec.x = _v.vec.x;
		vec.y = _v.vec.y;
		return *this;
	}
	template<>
	CVector3& operator=(const btVector3& _v) 
	{
		vec.x = _v.x();
		vec.y = _v.y();
		vec.z = _v.z();
		return *this;
	}
	template<>
	CVector3& operator=(const DirectX::XMFLOAT3& _v)
	{
		vec.x = _v.x;
		vec.y = _v.y;
		vec.z = _v.z;
		return *this;
	}

	CVector3() { x = y = z = 0.0f; }
	/*!
	* @brief	コンストラクタ。
	*/
	CVector3(float x, float y, float z)
	{
		Set(x, y, z);
	}
	CVector3(float s)
	{
		Set(s, s, s);
	}

	//コピーコンストラクタ
	CVector3(const btVector3& _v) {
		Set(_v);
	}

	/*!
	* @brief	線形補間。
	*@details
	* this = v0 + (v1-v0) * t;
	*/
	void Lerp(float t, const CVector3& v0, const CVector3& v1)
	{
		DirectX::XMVECTOR _v = DirectX::XMVectorLerp(
			DirectX::XMLoadFloat3(&v0.vec),
			DirectX::XMLoadFloat3(&v1.vec),
			t);
		DirectX::XMStoreFloat3(&vec, _v);
	}

	/// <summary>
	/// ベクトルの各要素を絶対値にする。
	/// </summary>
	void Abs() {
		x = abs(x);
		y = abs(y);
		z = abs(z);
	}

	template<class TVector>
	void CopyTo(TVector& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
	}
	template<>
	void CopyTo(btVector3& dst) const
	{
		dst.setX(x);
		dst.setY(y);
		dst.setZ(z);
	}
	/*!
	* @brief	ベクトルの各要素を設定。
	*/
	void Set(float _x, float _y, float _z)
	{
		vec.x = _x;
		vec.y = _y;
		vec.z = _z;
	}
	template<class TVector>
	void Set(const TVector& _v)
	{
		Set(_v.x, _v.y, _v.z);
	}
	template<>
	void Set(const btVector3& _v)
	{
		Set(_v.x(), _v.y(), _v.z());
	}
	
	/*!
	 * @brief	ベクトルを加算。
	 */
	void Add( const CVector3& _v) 
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	void Add( const CVector3& v0, const CVector3& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/*!
	 * @brief	ベクトルを減算。
	 */
	void Subtract( const CVector3& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	void Subtract( const CVector3& v0, const CVector3& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/*!
	 * @brief	内積。
	 */
	float Dot( const CVector3& _v ) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		return DirectX::XMVector3Dot(xmv0, xmv1).m128_f32[0];
	}
	/*!
	 * @brief	外積。
	 */
	void Cross(const CVector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	void Cross(const CVector3& v0, const CVector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	static CVector3 GetCross(const CVector3& v0, const CVector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		CVector3 re;
		DirectX::XMStoreFloat3(&re.vec, xmvr);
		return re;
	}

	/*!
	 * @brief	長さを取得
	 */
	float Length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	長さの二乗を取得
	 */
	float LengthSq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3LengthSq(xmv).m128_f32[0];
	}
	/*!
	* @brief	拡大。
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat3(&vec, xmv);
	}
	//ベクトル同士の乗算。
	void Multiply(const CVector3& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	void Multiply(const CVector3& v0, const CVector3& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat3(&vec, xmvr);
	}
	/*!
	* @brief	法線を正規化。
	*/
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVector3Normalize(xmv);
		DirectX::XMStoreFloat3(&vec, xmv);
	}
	CVector3 GetNorm() const{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVector3Normalize(xmv);
		CVector3 re;
		DirectX::XMStoreFloat3(&re.vec, xmv);

		return re;
	}

	//２つの正規化ベクトルABのなす角度θを求める
	static inline float AngleOf2NormalizeVector(const CVector3& A, const CVector3& B)
	{
		return acos(A.Dot(B));
	}

	/*!
	* @brief	除算。
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}
	/*!
	* @brief	最大値を設定。
	*/
	void Max(const CVector3& vMax)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&vMax.vec);
		DirectX::XMStoreFloat3(&vec,  DirectX::XMVectorMax(xmv0, xmv1));
	}
	/*!
	* @brief	最小値を設定。
	*/
	void Min(const CVector3& vMin)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&vMin.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMin(xmv0, xmv1));
	}
	/*!
	 *@brief	加算代入演算子。
	 */
	const CVector3& operator+=(const CVector3& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief　乗算代入演算子。
	*/
	const CVector3& operator*=(float s) 
	{
		Scale(s);
		return *this;
	}
	//乗算代入演算子(ベクトル同士)。
	const CVector3& operator*=(const CVector3& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	減算代入演算子。
	*/
	const CVector3& operator-=(const CVector3& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	除算代入演算子。
	 */
	const CVector3& operator/=(const float s)
	{
		Div(s);
		return *this;
	}
public:
	static CVector3 Zero()
	{
		static const CVector3 zero = { 0.0f,  0.0f,  0.0f };
		return zero;
	}
	static CVector3 Right()
	{
		static const CVector3 right = { 1.0f,  0.0f,  0.0f };
		return right;
	}
	static CVector3 Left()
	{
		static const CVector3 left = { -1.0f,  0.0f,  0.0f };
		return left;
	}
	static CVector3 Up()
	{
		static const CVector3 up = { 0.0f,  1.0f,  0.0f };
		return up;
	}
	static CVector3 Down()
	{
		static const CVector3 down = { 0.0f, -1.0f,  0.0f };
		return down;
	}
	static CVector3 Front()
	{
		static const CVector3 front = { 0.0f,   0.0f,  1.0f };
		return front;
	}
	static CVector3 Back()
	{
		static const CVector3 back = { 0.0f,   0.0f, -1.0f };
		return back;
	}
	static CVector3 AxisX()
	{
		static const CVector3 axisX = { 1.0f,  0.0f,  0.0f };
		return axisX;
	}
	static CVector3 AxisY()
	{
		static const CVector3 axisY = { 0.0f,  1.0f,  0.0f };
		return axisY;
	}
	static CVector3 AxisZ()
	{
		static const CVector3 axisZ = { 0.0f,  0.0f,  1.0f };
		return axisZ;
	}
	static CVector3 One()
	{
		static const CVector3 one = { 1.0f, 1.0f, 1.0f };
		return one;
	}
};
/*!
 *@brief	4要素のベクトルクラス。
 */
class CVector4{
public:
	union{
		DirectX::XMFLOAT4 vec;
		struct { float x, y, z, w; };
		float v[4];
	};
public:
	operator DirectX::XMVECTOR() const
	{
		return DirectX::XMLoadFloat4(&vec);
	}
	CVector4(){ x = y = z = w = 0.0f; }
	/*!
	*@brief	代入演算子。
	*/
	CVector4& operator=(const CVector4& _v)
	{
		vec = _v.vec;
		return *this;
	}
	CVector4& operator=(const CVector3& _v)
	{
		vec.x = _v.vec.x;
		vec.y = _v.vec.y;
		vec.z = _v.vec.z;
		return *this;
	}
	/*!
	 *@brief	コンストラクタ
	 */
	CVector4(float x, float y, float z, float w)
	{
		Set(x, y, z, w);
	}
	/*!
	*@brief	コンストラクタ
	*@details
	* wには1.0が格納されます。
	*/
	CVector4(const CVector3& v)
	{
		Set(v);
	}

	template<class TVector4>
	void CopyTo(TVector4& dst) const
	{
		dst.x = x;
		dst.y = y;
		dst.z = z;
		dst.w = w;
	}
	template<>
	void CopyTo(btQuaternion& dst) const
	{
		dst.setX(x);
		dst.setY(y);
		dst.setZ(z);
		dst.setW(w);
	}

	/*!
	 *@brief	ベクトルの各要素を設定。
	 */
	void Set(float _x, float _y, float _z, float _w)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->w = _w;
	}
	/*!
	* @brief	法線を正規化。
	*/
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVector4Normalize(xmv);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/*!
	*@brief	ベクトルを設定。
	*/
	void Set(const CVector4& _v)
	{
		*this = _v;
	}
	/*!
	*@brief	ベクトルを設定。
	*@details
	* wには1.0が格納されます。
	*/
	
	void Set(const CVector3& _v)
	{
		this->x = _v.x;
		this->y = _v.y;
		this->z = _v.z;
		this->w = 1.0f;
	}
	/*!
	 *@brief	ベクトルを加算。
	 */
	void Add( const CVector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	void Add( const CVector4& v0, const CVector4& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorAdd(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/*!
	 *@brief	ベクトルを減算。
	 */
	void Subtract( const CVector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	void Subtract( const CVector4& v0, const CVector4& v1 )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorSubtract(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/*!
	 *@brief	内積
	 */
	float Dot( const CVector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		return DirectX::XMVector4Dot(xmv0, xmv1).m128_f32[0];
	}
	/*!
	 * @brief	長さを取得
	 */
	float Length()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	長さの二乗を取得
	 */
	float LengthSq()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4LengthSq(xmv).m128_f32[0];
	}
	/*!
	* @brief	拡大。
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	//ベクトル同士の乗算。
	void Multiply(const CVector4& _v)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	void Multiply(const CVector4& v0, const CVector4& v1)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&v0.vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&v1.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVectorMultiply(xmv0, xmv1);
		DirectX::XMStoreFloat4(&vec, xmvr);
	}
	/*!
	* @brief	除算。
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}

	/*!
	 *@brief	加算代入演算子。
	 */
	const CVector4& operator+=(const CVector4& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief　乗算代入演算子。
	*/
	const CVector4& operator*=(float s)
	{
		Scale(s);
		return *this;
	}
	//乗算代入演算子(ベクトル同士)。
	const CVector4& operator*=(const CVector4& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	減算代入演算子。
	*/
	const CVector4& operator-=(const CVector4& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	除算代入演算子。
	 */
	const CVector4& operator/=(const float s)
	{
		Div(s);
		return *this;
	}

	static CVector4 White()
	{
		static const CVector4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
		return white;
	}
	static inline CVector4 One() {
		return White();
	}
	static CVector4 Zero()
	{
		static const CVector4 zero = { 0.0f,  0.0f,  0.0f ,  0.0f };
		return zero;
	}
};

class CQuaternion : public CVector4{
public:
	CQuaternion() {
		x = y = z = 0.0f;
		w = 1.0f;
	}
	CQuaternion(float x, float y, float z, float w) :
		CVector4(x, y, z, w)
	{
	}
	CQuaternion(const CVector3& axis, float angle)
	{
		SetRotation(axis, angle);
	}
	
	//逆クォータニオンにする
	void Inverse() {
		DirectX::XMVECTOR xmv = DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&vec));
		DirectX::XMStoreFloat4(&vec, xmv);
	}

	/*!
	 *@brief	任意の軸周りの回転クォータニオンを作成。
	 */
	void SetRotation( const CVector3& axis, float angle )
	{
		float s;
		float halfAngle = angle * 0.5f;
		s = sin(halfAngle);
		w = cos(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}
	void SetRotationDeg(const CVector3& axis, float angle)
	{
		float s;
		float halfAngle = CMath::DegToRad(angle) * 0.5f;
		s = sin(halfAngle);
		w = cos(halfAngle);
		x = axis.x * s;
		y = axis.y * s;
		z = axis.z * s;
	}
	void Set(const btQuaternion& rot)
	{
		x = rot.x();
		y = rot.y();
		z = rot.z();
		w = rot.w();
	}

	//任意の軸周りの回転クォータニオンを取得
	static CQuaternion GetRotation(const CVector3& axis, float angle)
	{
		float s;
		float halfAngle = angle * 0.5f;
		s = sin(halfAngle);

		return { axis.x * s , axis.y * s, axis.z * s, cos(halfAngle) };
	}
	static CQuaternion GetRotationDeg(const CVector3& axis, float angle)
	{
		float s;
		float halfAngle = CMath::DegToRad(angle) * 0.5f;
		s = sin(halfAngle);

		return { axis.x * s , axis.y * s, axis.z * s, cos(halfAngle) };
	}

	/*!
	*@brief	行列からクォータニオンを作成。
	*/
	void SetRotation(const CMatrix& m);

	//方向と上方向からZ軸を指定の方向へ向けるクォータニオンを作成	
	void MakeLookTo(const CVector3& direction, const CVector3& up = CVector3::Up());

	/*!
	 *@brief	球面線形補完。
	 */
	void Slerp(float t, CQuaternion q1, CQuaternion q2)
	{
		DirectX::XMVECTOR xmv = DirectX::XMQuaternionSlerp(
			DirectX::XMLoadFloat4(&q1.vec),
			DirectX::XMLoadFloat4(&q2.vec),
			t
		);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/*!
	*@brief	クォータニオン同士の積。
	*/
	void Multiply(const CQuaternion& rot)
	{
		float pw, px, py, pz;
		float qw, qx, qy, qz;

		pw = w; px = x; py = y; pz = z;
		qw = rot.w; qx = rot.x; qy = rot.y; qz = rot.z;

		w = pw * qw - px * qx - py * qy - pz * qz;
		x = pw * qx + px * qw + py * qz - pz * qy;
		y = pw * qy - px * qz + py * qw + pz * qx;
		z = pw * qz + px * qy - py * qx + pz * qw;

	}
	/*!
	 *@brief	クォータニオン同士の乗算。
	 *@details
	 * this = rot0 * rot1;
	 */
	void Multiply(const CQuaternion& rot0, const CQuaternion& rot1)
	{
		float pw, px, py, pz;
		float qw, qx, qy, qz;

		pw = rot0.w; px = rot0.x; py = rot0.y; pz = rot0.z;
		qw = rot1.w; qx = rot1.x; qy = rot1.y; qz = rot1.z;

		w = pw * qw - px * qx - py * qy - pz * qz;
		x = pw * qx + px * qw + py * qz - pz * qy;
		y = pw * qy - px * qz + py * qw + pz * qx;
		z = pw * qz + px * qy - py * qx + pz * qw;
	}

	/// <summary>
	/// クォータニオンを連結する
	/// このクォータニオンの回転の後にrotの回転をするクォータニオンを作成
	/// </summary>
	void Concatenate(const CQuaternion& rot) {
		Multiply(rot, *this);
	}
	/// <summary>
	/// クォータニオンを連結する
	/// rot0の回転の後にrot1の回転をするクォータニオンを作成
	/// this = rot1 * rot0;
	/// </summary>
	void Concatenate(const CQuaternion& rot0, const CQuaternion& rot1) {
		Multiply(rot1,rot0);
	}

	/*!
	*@brief	ベクトルにクォータニオンを適用する。
	*@param[in,out] v	ベクトル。
	*/
	void Multiply(CVector4& _v)
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat4(&_v.vec, xmv);
	}
	void Multiply(CVector3& _v)
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat3(&_v.vec, xmv);
	}

	//ベクトルを逆回転
	void InverseMultiply(CVector4& _v)
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3InverseRotate(_v, *this);
		DirectX::XMStoreFloat4(&_v.vec, xmv);
	}
	void InverseMultiply(CVector3& _v)
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3InverseRotate(_v, *this);
		DirectX::XMStoreFloat3(&_v.vec, xmv);
	}

	//乗算代入演算子
	const CQuaternion& operator*=(const CQuaternion& _v)
	{
		Multiply(_v);
		return *this;
	}

	static CQuaternion Identity()
	{
		static const CQuaternion identity = { 0.0f,  0.0f, 0.0f, 1.0f };
		return identity;
	}
};
//整数型のベクトルクラス。
__declspec(align(16)) class CVector4i {
public:
	union {
		struct { int x, y, z, w; };
		int v[4];
	};
};
/*!
*@brief	ベクトル同士の加算。
*/
template<class TVector>
static inline TVector operator+(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Add(v0, v1);
	return result;
}
/*!
*@brief	ベクトルのスケール倍。
*/
template<class TVector>
static inline TVector operator*(const TVector& v, float s)
{
	TVector result;
	result = v;
	result.Scale(s);
	return result;
}
//ベクトル同士の乗算。
template<class TVector>
static inline TVector operator*(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Multiply(v0, v1);
	return result;
}
/*!
*@brief	ベクトルの除算。
*/
template<class TVector>
static inline TVector operator/(const TVector& v, float s)
{
	TVector result;
	result = v;
	result.Div(s);
	return result;
}
/*!
*@brief	ベクトル同士の減算。
*/
template<class TVector>
static inline TVector operator-(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Subtract(v0, v1);
	return result;
}

}


//構造化束縛のやつ
namespace std {
	template <>
	struct tuple_size<DemolisherWeapon::CVector2> : integral_constant<size_t, 2> {}; //CVector2は2要素

	template <size_t N>
	struct tuple_element<N, DemolisherWeapon::CVector2> {
		using type = float; // 要素の型はすべて float
	};
}