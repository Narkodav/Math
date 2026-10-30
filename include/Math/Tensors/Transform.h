#pragma once
#include "Math/Tensors/TensorFunctions.h"

namespace Math {

	template<Mat4Expr M, Vec3Expr V>
	constexpr decltype(auto) translate(const TensorExpression<M>& m, const TensorExpression<V>& v) {
		auto result = m.eval();
		decltype(auto) vec = v.eval();
		result[3] += result[0] * vec[0] + result[1] * vec[1] + result[2] * vec[2];
		return result;
	}

	template<Mat4Expr M, Vec3Expr V, typename T>
	constexpr decltype(auto) rotate(const TensorExpression<M>& m, const TensorExpression<V>& v, T&& angle) {
		using ValueType = std::remove_cvref_t<T>;
		auto c = std::cos(std::forward<T>(angle));
		auto s = std::sin(std::forward<T>(angle));

		decltype(auto) vec = v.eval();

		auto axis = normalize(v).eval();
		auto temp = ((ValueType{1} - c) * axis).eval();

		TensorBase<Shape<typename M::ValueType, 4, 4>> rotate;
		rotate[0][0] = c + temp[0] * axis[0];
		rotate[0][1] = temp[0] * axis[1] + s * axis[2];
		rotate[0][2] = temp[0] * axis[2] - s * axis[1];

		rotate[1][0] = temp[1] * axis[0] - s * axis[2];
		rotate[1][1] = c + temp[1] * axis[1];
		rotate[1][2] = temp[1] * axis[2] + s * axis[0];

		rotate[2][0] = temp[2] * axis[0] + s * axis[1];
		rotate[2][1] = temp[2] * axis[1] - s * axis[0];
		rotate[2][2] = c + temp[2] * axis[2];

		decltype(auto) mat = m.eval();

		TensorBase<Shape<typename M::ValueType, 4, 4>> result;
		result[0] = mat[0] * rotate[0][0] + mat[1] * rotate[0][1] + mat[2] * rotate[0][2];
		result[1] = mat[0] * rotate[1][0] + mat[1] * rotate[1][1] + mat[2] * rotate[1][2];
		result[2] = mat[0] * rotate[2][0] + mat[1] * rotate[2][1] + mat[2] * rotate[2][2];
		result[3] = mat[3];
		return result;
	}

	template<Mat4Expr M, Vec3Expr V>
	constexpr decltype(auto) scale(const TensorExpression<M>& m, const TensorExpression<V>& v) {
		decltype(auto) mat = m.eval();
		decltype(auto) vec = v.eval();
		TensorBase<Shape<typename M::ValueType, 4, 4>> result;
		result[0] = mat[0] * vec[0];
		result[1] = mat[1] * vec[1];
		result[2] = mat[2] * vec[2];
		result[3] = mat[3];
		return result;
	}

    template<Mat4Expr M, Vec3Expr V, Vec2Expr VX, Vec2Expr VY, Vec2Expr VZ>
    constexpr decltype(auto) shear(const TensorExpression<M>& m, const TensorExpression<V>& v, 
		const TensorExpression<VX>& vx, const TensorExpression<VY>& vy, const TensorExpression<VZ>& vz) {
		using ValueType = typename V::ValueType;

		decltype(auto) vec = v.eval();
		decltype(auto) vecx = vx.eval();
		decltype(auto) vecy = vy.eval();
		decltype(auto) vecz = vz.eval();

        auto& lambda_xy = vecx[0];
        auto& lambda_xz = vecx[1];
        auto& lambda_yx = vecy[0];
        auto& lambda_yz = vecy[1];
        auto& lambda_zx = vecz[0];
        auto& lambda_zy = vecz[1];

        TensorBase<Shape<ValueType, 3>> point_lambda(lambda_xy + lambda_xz, lambda_yx + lambda_yz, lambda_zx + lambda_zy);

        TensorBase<Shape<ValueType, 4, 4>> shear(
            1                      , lambda_yx              , lambda_zx              , 0,
            lambda_xy              , 1                      , lambda_zy              , 0,
            lambda_xz              , lambda_yz              , 1                      , 0,
            -point_lambda[0] * vec[0], -point_lambda[1] * vec[1], -point_lambda[2] * vec[2], 1
        );

		decltype(auto) mat = m.eval();

        TensorBase<Shape<ValueType, 4, 4>> result;
		result[0] = mat[0] * shear[0][0] + mat[1] * shear[0][1] + mat[2] * shear[0][2] + mat[3] * shear[0][3];
		result[1] = mat[0] * shear[1][0] + mat[1] * shear[1][1] + mat[2] * shear[1][2] + mat[3] * shear[1][3];
		result[2] = mat[0] * shear[2][0] + mat[1] * shear[2][1] + mat[2] * shear[2][2] + mat[3] * shear[2][3];
		result[3] = mat[0] * shear[3][0] + mat[1] * shear[3][1] + mat[2] * shear[3][2] + mat[3] * shear[3][3];
        return result;
    }

	template<Vec3Expr E, Vec3Expr C, Vec3Expr U>
	constexpr decltype(auto) lookAtRH(const TensorExpression<E>& e, const TensorExpression<C>& c, const TensorExpression<U>& u)
	{
		decltype(auto) eye = e.eval();
		decltype(auto) center = c.eval();
		decltype(auto) up = u.eval();
		auto f = normalize(center - eye);
		auto s = normalize(cross(f, up));
		auto up2 = cross(s, f);

		using ValueType = typename E::ValueType;

		TensorBase<Shape<ValueType, 4, 4>> result(
			s[0], 			up2[0], 			-f[0], 			1,
			s[1], 			up2[1], 			-f[1], 			1,
			s[2], 			up2[2], 			-f[2], 			1,
			-dot(s, eye),	-dot(up2, eye),	dot(f, eye), 	1
		);
		return result;
	}

	template<Vec3Expr E, Vec3Expr C, Vec3Expr U>
	constexpr decltype(auto) lookAtLH(const TensorExpression<E>& e, const TensorExpression<C>& c, const TensorExpression<U>& u)
	{
		decltype(auto) eye = e.eval();
		decltype(auto) center = c.eval();
		decltype(auto) up = u.eval();
		auto f = normalize(center - eye);
		auto s = normalize(cross(up, f));
		auto up2 = cross(f, s);

		using ValueType = typename E::ValueType;

		TensorBase<Shape<ValueType, 4, 4>> result(
			s[0], 			up2[0], 			f[0], 			1,
			s[1], 			up2[1], 			f[1], 			1,
			s[2], 			up2[2], 			f[2], 			1,
			-dot(s, eye),	-dot(up2, eye),	-dot(f, eye), 	1
		);
		return result;
	}

	enum class Handedness {
		Right, Left
	};

	template<Handedness hand = Handedness::Right, Vec3Expr E, Vec3Expr C, Vec3Expr U>
	constexpr decltype(auto) lookAt(const TensorExpression<E>& e, const TensorExpression<C>& c, const TensorExpression<U>& u)
	{
		if constexpr (hand == Handedness::Right) return lookAtRH(e, c, u);
		else return lookAtLH(e, c, u);
	}

	template<typename T>
	constexpr decltype(auto) perspectiveRH(T fovy, T aspect, T zNear, T zFar, T NDCmin, T NDCmax) {
		assert(std::abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

		T tanHalfFovy = std::tan(fovy / static_cast<T>(2));

		TensorBase<Shape<T, 4, 4>> result(static_cast<T>(0));
		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][2] = - (NDCmax * zNear - NDCmin * zFar) / (zFar - zNear);
		result[2][3] = - static_cast<T>(1);
		result[3][2] = - (zNear * zFar * (NDCmax - NDCmin)) / (zFar - zNear);
		return result;
	}

	template<typename T>
	constexpr decltype(auto) perspectiveLH(T fovy, T aspect, T zNear, T zFar, T NDCmin, T NDCmax) {
		assert(std::abs(aspect - std::numeric_limits<T>::epsilon()) > static_cast<T>(0));

		T tanHalfFovy = std::tan(fovy / static_cast<T>(2));

		TensorBase<Shape<T, 4, 4>> result(static_cast<T>(0));
		result[0][0] = static_cast<T>(1) / (aspect * tanHalfFovy);
		result[1][1] = static_cast<T>(1) / (tanHalfFovy);
		result[2][2] = (NDCmax * zNear - NDCmin * zFar) / (zFar - zNear);
		result[2][3] = static_cast<T>(1);
		result[3][2] = - (zNear * zFar * (NDCmax - NDCmin)) / (zFar - zNear);
		return result;
	}

	template<Handedness hand = Handedness::Right, typename T>
	constexpr decltype(auto) perspective(T fovy, T aspect, T zNear, T zFar, T NDCmin = 0, T NDCmax = 1) {
		if constexpr (hand == Handedness::Right) return perspectiveRH(fovy, aspect, zNear, zFar, NDCmin, NDCmax);
		else return perspectiveLH(fovy, aspect, zNear, zFar, NDCmin, NDCmax);
	}


	template<typename T>
	constexpr decltype(auto) orthoRH(T left, T right, T bottom, T top, T zNear, T zFar, T NDCmin, T NDCmax) {
		TensorBase<Shape<T,4,4>> result(static_cast<T>(1));
		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[2][2] = -(NDCmax - NDCmin) / (zFar - zNear);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		result[3][2] = (NDCmin * zFar - NDCmax * zNear) / (zFar - zNear);
		return result;
	}

	template<typename T>
	constexpr decltype(auto) orthoLH(T left, T right, T bottom, T top, T zNear, T zFar, T NDCmin, T NDCmax) {
		TensorBase<Shape<T,4,4>> result(static_cast<T>(1));
		result[0][0] = static_cast<T>(2) / (right - left);
		result[1][1] = static_cast<T>(2) / (top - bottom);
		result[2][2] = (NDCmax - NDCmin) / (zFar - zNear);
		result[3][0] = -(right + left) / (right - left);
		result[3][1] = -(top + bottom) / (top - bottom);
		result[3][2] = (NDCmin * zFar - NDCmax * zNear) / (zFar - zNear);
		return result;
	}

	template<Handedness hand = Handedness::Right, typename T>
	constexpr decltype(auto) ortho(T left, T right, T bottom, T top, T zNear, T zFar, T NDCmin = 0, T NDCmax = 1) {
		if constexpr (hand == Handedness::Right) return orthoRH(left, right, bottom, top, zNear, zFar, NDCmin, NDCmax);
		else return orthoLH(left, right, bottom, top, zNear, zFar, NDCmin, NDCmax);
	}

}//namespace glm
