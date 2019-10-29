/*!
 * @brief	�x�N�g��
 * @details
 * DirectMath���g���₷�������x�N�g���N���X�B
 */

#pragma once

#include <Effekseer.h>

#include "kMath.h"

namespace DemolisherWeapon{

class CMatrix;
class CVector3;

class CVector2 {
public:
	CVector2()
	{
		x = y = 0.0f;
	}
	CVector2(float n)
	{
		this->x = n;
		this->y = n;
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

	operator CVector3() const;

	/*!
	*@brief	������Z�q�B
	*/
	CVector2& operator=(const CVector2& _v)
	{
		vec = _v.vec;
		return *this;
	}
	/*!
	 * @brief	�x�N�g�������Z�B
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
	 * @brief	�x�N�g�������Z�B
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
	* @brief	�g��B
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat2(&vec, xmv);
	}
	//�x�N�g�����m�̏�Z�B
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
	* @brief	���Z�B
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}

	/*!
	 * @brief	�������擾
	 */
	float Length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVector2Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	�����̓����擾
	 */
	float LengthSq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat2(&vec);
		return DirectX::XMVector2LengthSq(xmv).m128_f32[0];
	}

	/*!
	* @brief	�@���𐳋K���B
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

	float Cross(const CVector2& _v)const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat2(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat2(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector2Cross(xmv0, xmv1);
		float re;
		DirectX::XMStoreFloat(&re, xmvr);
		return re;
	}

	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector2& operator+=(const CVector2& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief�@��Z������Z�q�B
	*/
	const CVector2& operator*=(float s)
	{
		Scale(s);
		return *this;
	}
	//��Z������Z�q(�x�N�g�����m)�B
	const CVector2& operator*=(const CVector2& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	���Z������Z�q�B
	*/
	const CVector2& operator-=(const CVector2& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector2& operator/=(const float s)
	{
		Div(s);
		return *this;
	}

	/*!
	* @brief	���`��ԁB
	*@details
	* this = v0 + (v1-v0) * t;
	*/
	void Lerp(float t, const CVector2& v0, const CVector2& v1)
	{
		x = v0.x + (v1.x - v0.x) * t;
		y = v0.y + (v1.y - v0.y) * t;
	}
	static const CVector2& Zero()
	{
		static const CVector2 zero = { 0.0f,  0.0f };
		return zero;
	}
	static const CVector2& One()
	{
		static const CVector2 one = { 1.0f,  1.0f };
		return one;
	}

	//�\���������̂��
	template <std::size_t N>
	float get() const {
		if constexpr (N == 0)
			return x;
		else
			return y;
	}
};
/*!
 * @brief	�x�N�g���B
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
	//XMVECTOR�ւ̈Öق̕ϊ��B
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
	*@brief	������Z�q�B
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

	/*!
	* @brief	�R���X�g���N�^�B
	*/
	CVector3() { x = y = z = 0.0f; }
	CVector3(float x, float y, float z)
	{
		Set(x, y, z);
	}
	CVector3(float s)
	{
		Set(s, s, s);
	}

	//�R�s�[�R���X�g���N�^
	CVector3(const btVector3& _v) {
		Set(_v);
	}
	CVector3(const DirectX::XMFLOAT3& _v) {
		vec = _v;
	}

	/*!
	* @brief	���`��ԁB
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
	/// �x�N�g���̊e�v�f���Βl�ɂ���B
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
	* @brief	�x�N�g���̊e�v�f��ݒ�B
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
	 * @brief	�x�N�g�������Z�B
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
	 * @brief	�x�N�g�������Z�B
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
	 * @brief	���ρB
	 */
	float Dot( const CVector3& _v ) const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		return DirectX::XMVector3Dot(xmv0, xmv1).m128_f32[0];
	}
	/*!
	 * @brief	�O�ρB
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
	CVector3 GetCross(const CVector3& _v)const
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&_v.vec);
		DirectX::XMVECTOR xmvr = DirectX::XMVector3Cross(xmv0, xmv1);
		CVector3 re;
		DirectX::XMStoreFloat3(&re.vec, xmvr);
		return re;
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
	 * @brief	�������擾
	 */
	float Length() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	�����̓����擾
	 */
	float LengthSq() const
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		return DirectX::XMVector3LengthSq(xmv).m128_f32[0];
	}
	/*!
	* @brief	�g��B
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat3(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat3(&vec, xmv);
	}
	//�x�N�g�����m�̏�Z�B
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
	* @brief	�@���𐳋K���B
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

	//�Q�̐��K���x�N�g��AB�̂Ȃ��p�x�Ƃ����߂�
	static inline float AngleOf2NormalizeVector(const CVector3& A, const CVector3& B)
	{
		return acos(CMath::ClampFromNegOneToPosOne(A.Dot(B)));
	}

	//�ő�l���擾
	float GetMax()const {
		return max(max(vec.x, vec.y), vec.z);
	}
	//�ŏ��l���擾
	float GetMin()const {
		return min(min(vec.x, vec.y), vec.z);
	}

	/*!
	* @brief	���Z�B
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}
	/*!
	* @brief	�ő�l��ݒ�B
	*/
	void Max(const CVector3& vMax)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&vMax.vec);
		DirectX::XMStoreFloat3(&vec,  DirectX::XMVectorMax(xmv0, xmv1));
	}
	/*!
	* @brief	�ŏ��l��ݒ�B
	*/
	void Min(const CVector3& vMin)
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat3(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat3(&vMin.vec);
		DirectX::XMStoreFloat3(&vec, DirectX::XMVectorMin(xmv0, xmv1));
	}
	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector3& operator+=(const CVector3& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief�@��Z������Z�q�B
	*/
	const CVector3& operator*=(float s) 
	{
		Scale(s);
		return *this;
	}
	//��Z������Z�q(�x�N�g�����m)�B
	const CVector3& operator*=(const CVector3& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	���Z������Z�q�B
	*/
	const CVector3& operator-=(const CVector3& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector3& operator/=(const float s)
	{
		Div(s);
		return *this;
	}
public:
	static const CVector3& Zero()
	{
		static const CVector3 zero = { 0.0f,  0.0f,  0.0f };
		return zero;
	}
	static const CVector3& Right()
	{
		static const CVector3 right = { 1.0f,  0.0f,  0.0f };
		return right;
	}
	static const CVector3& Left()
	{
		static const CVector3 left = { -1.0f,  0.0f,  0.0f };
		return left;
	}
	static const CVector3& Up()
	{
		static const CVector3 up = { 0.0f,  1.0f,  0.0f };
		return up;
	}
	static const CVector3& Down()
	{
		static const CVector3 down = { 0.0f, -1.0f,  0.0f };
		return down;
	}
	static const CVector3& Front()
	{
		static const CVector3 front = { 0.0f,   0.0f,  1.0f };
		return front;
	}
	static const CVector3& Back()
	{
		static const CVector3 back = { 0.0f,   0.0f, -1.0f };
		return back;
	}
	static const CVector3& AxisX()
	{
		static const CVector3 axisX = { 1.0f,  0.0f,  0.0f };
		return axisX;
	}
	static const CVector3& AxisY()
	{
		static const CVector3 axisY = { 0.0f,  1.0f,  0.0f };
		return axisY;
	}
	static const CVector3& AxisZ()
	{
		static const CVector3 axisZ = { 0.0f,  0.0f,  1.0f };
		return axisZ;
	}
	static const CVector3& One()
	{
		static const CVector3 one = { 1.0f, 1.0f, 1.0f };
		return one;
	}
};
/*!
 *@brief	4�v�f�̃x�N�g���N���X�B
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
	*@brief	������Z�q�B
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
	 *@brief	�R���X�g���N�^
	 */
	CVector4(float x, float y, float z, float w)
	{
		Set(x, y, z, w);
	}
	CVector4(float f)
	{
		Set(f, f, f, f);
	}
	/*!
	*@brief	�R���X�g���N�^
	*@details
	* w�ɂ�1.0���i�[����܂��B
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
	 *@brief	�x�N�g���̊e�v�f��ݒ�B
	 */
	void Set(float _x, float _y, float _z, float _w)
	{
		this->x = _x;
		this->y = _y;
		this->z = _z;
		this->w = _w;
	}
	/*!
	* @brief	�@���𐳋K���B
	*/
	void Normalize()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVector4Normalize(xmv);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	/*!
	*@brief	�x�N�g����ݒ�B
	*/
	void Set(const CVector4& _v)
	{
		*this = _v;
	}
	/*!
	*@brief	�x�N�g����ݒ�B
	*@details
	* w�ɂ�1.0���i�[����܂��B
	*/
	
	void Set(const CVector3& _v)
	{
		this->x = _v.x;
		this->y = _v.y;
		this->z = _v.z;
		this->w = 1.0f;
	}
	/*!
	 *@brief	�x�N�g�������Z�B
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
	 *@brief	�x�N�g�������Z�B
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
	 *@brief	����
	 */
	float Dot( const CVector4& _v )
	{
		DirectX::XMVECTOR xmv0 = DirectX::XMLoadFloat4(&vec);
		DirectX::XMVECTOR xmv1 = DirectX::XMLoadFloat4(&_v.vec);
		return DirectX::XMVector4Dot(xmv0, xmv1).m128_f32[0];
	}
	/*!
	 * @brief	�������擾
	 */
	float Length()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4Length(xmv).m128_f32[0];
	}
	/*!
	 * @brief	�����̓����擾
	 */
	float LengthSq()
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		return DirectX::XMVector4LengthSq(xmv).m128_f32[0];
	}
	/*!
	* @brief	�g��B
	*/
	void Scale(float s)
	{
		DirectX::XMVECTOR xmv = DirectX::XMLoadFloat4(&vec);
		xmv = DirectX::XMVectorScale(xmv, s);
		DirectX::XMStoreFloat4(&vec, xmv);
	}
	//�x�N�g�����m�̏�Z�B
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
	* @brief	���Z�B
	*/
	void Div(float d)
	{
		float scale = 1.0f / d;
		Scale(scale);
	}

	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector4& operator+=(const CVector4& _v)
	{
		Add(_v);
		return *this;
	}
	/*!
	*@brief�@��Z������Z�q�B
	*/
	const CVector4& operator*=(float s)
	{
		Scale(s);
		return *this;
	}
	//��Z������Z�q(�x�N�g�����m)�B
	const CVector4& operator*=(const CVector4& _v)
	{
		Multiply(_v);
		return *this;
	}
	/*!
	*@brief	���Z������Z�q�B
	*/
	const CVector4& operator-=(const CVector4& _v)
	{
		Subtract(_v);
		return *this;
	}
	/*!
	 *@brief	���Z������Z�q�B
	 */
	const CVector4& operator/=(const float s)
	{
		Div(s);
		return *this;
	}

	static const CVector4& Black()
	{
		static const CVector4 black = { 0.0f, 0.0f, 0.0f, 1.0f };
		return black;
	}
	static const CVector4& White()
	{
		static const CVector4 white = { 1.0f, 1.0f, 1.0f, 1.0f };
		return white;
	}
	static inline const CVector4& One() {
		return White();
	}
	static const CVector4& Zero()
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
	
	//�t�N�H�[�^�j�I���ɂ���
	void Inverse() {
		DirectX::XMVECTOR xmv = DirectX::XMQuaternionInverse(DirectX::XMLoadFloat4(&vec));
		DirectX::XMStoreFloat4(&vec, xmv);
	}

	/*!
	 *@brief	�C�ӂ̎�����̉�]�N�H�[�^�j�I�����쐬�B
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

	//�C�ӂ̎�����̉�]�N�H�[�^�j�I�����擾
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
	*@brief	�s�񂩂�N�H�[�^�j�I�����쐬�B
	*/
	void SetRotation(const CMatrix& m);

	//�����Ə��������Z�����w��̕����֌�����N�H�[�^�j�I�����쐬	
	void MakeLookTo(const CVector3& direction, const CVector3& up = CVector3::Up());

	//X�EY����]���g���āAZ�����w��̕����Ɍ�����N�H�[�^�j�I�����쐬
	void MakeLookToUseXYAxis(const CVector3& direction);

	/*!
	 *@brief	���ʐ��`�⊮�B
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
	*@brief	�N�H�[�^�j�I�����m�̐ρB
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
	 *@brief	�N�H�[�^�j�I�����m�̏�Z�B
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
	/// �N�H�[�^�j�I����A������
	/// ���̃N�H�[�^�j�I���̉�]�̌��rot�̉�]������N�H�[�^�j�I�����쐬
	/// </summary>
	void Concatenate(const CQuaternion& rot) {
		Multiply(rot, *this);
	}
	/// <summary>
	/// �N�H�[�^�j�I����A������
	/// rot0�̉�]�̌��rot1�̉�]������N�H�[�^�j�I�����쐬
	/// this = rot1 * rot0;
	/// </summary>
	void Concatenate(const CQuaternion& rot0, const CQuaternion& rot1) {
		Multiply(rot1,rot0);
	}

	/*!
	*@brief	�x�N�g���ɃN�H�[�^�j�I����K�p����B
	*@param[in,out] v	�x�N�g���B
	*/
	void Multiply(CVector4& _v)const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat4(&_v.vec, xmv);
	}
	void Multiply(CVector3& _v)const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3Rotate(_v, *this);
		DirectX::XMStoreFloat3(&_v.vec, xmv);
	}

	//�x�N�g�����t��]
	void InverseMultiply(CVector4& _v)const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3InverseRotate(_v, *this);
		DirectX::XMStoreFloat4(&_v.vec, xmv);
	}
	void InverseMultiply(CVector3& _v)const
	{
		DirectX::XMVECTOR xmv = DirectX::XMVector3InverseRotate(_v, *this);
		DirectX::XMStoreFloat3(&_v.vec, xmv);
	}

	//��]���Ɗp�x���v�Z
	void ToAngleAxis(CVector3& return_axis, float& return_angle) {
		DirectX::XMVECTOR axis;
		DirectX::XMQuaternionToAxisAngle(&axis, &return_angle, DirectX::XMLoadFloat4(&vec));
		DirectX::XMStoreFloat3(&return_axis.vec, axis);
	}

	//��Z������Z�q
	const CQuaternion& operator*=(const CQuaternion& _v)
	{
		Multiply(_v);
		return *this;
	}

	static const CQuaternion& Identity()
	{
		static const CQuaternion identity = { 0.0f,  0.0f, 0.0f, 1.0f };
		return identity;
	}
};
//�����^�̃x�N�g���N���X�B
__declspec(align(16)) class CVector4i {
public:
	union {
		struct { int x, y, z, w; };
		int v[4];
	};
};
/*!
*@brief	�x�N�g�����m�̉��Z�B
*/
template<class TVector>
static inline TVector operator+(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Add(v0, v1);
	return result;
}
/*!
*@brief	�x�N�g���̃X�P�[���{�B
*/
template<class TVector>
static inline TVector operator*(const TVector& v, float s)
{
	TVector result;
	result = v;
	result.Scale(s);
	return result;
}
//�x�N�g�����m�̏�Z�B
template<class TVector>
static inline TVector operator*(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Multiply(v0, v1);
	return result;
}
/*!
*@brief	�x�N�g���̏��Z�B
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
*@brief	�x�N�g�����m�̌��Z�B
*/
template<class TVector>
static inline TVector operator-(const TVector& v0, const TVector& v1)
{
	TVector result;
	result.Subtract(v0, v1);
	return result;
}

}


//�\���������̂��
namespace std {
	template <>
	struct tuple_size<DemolisherWeapon::CVector2> : integral_constant<size_t, 2> {}; //CVector2��2�v�f

	template <size_t N>
	struct tuple_element<N, DemolisherWeapon::CVector2> {
		using type = float; // �v�f�̌^�͂��ׂ� float
	};
}