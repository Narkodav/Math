#pragma once
#include "Math/Tensors/TensorFunctions.h"

namespace Math {

	template<Mat4Expr M, Vec3Expr V>
	constexpr decltype(auto) translate(const TensorExpression<M>& m, const TensorExpression<V>& v) {
		auto result = m.eval();
		decltype(auto) vec = v.eval();
		result[3] += result[0] * vec[0] + result[1] * vec[1] + result[2] * vec[2] ;
		return result;
	}

	template<Mat4Expr M, Vec3Expr V, typename T>
	constexpr decltype(auto) rotate(const TensorExpression<M>& m, const TensorExpression<V>& v, T&& angle) {
		auto c = std::cos(std::forward<T>(angle));
		auto s = std::sin(std::forward<T>(angle));

		decltype(auto) vec = v.eval();

		auto axis = normalize(v).eval();
		auto temp = ((T{1} - c) * axis).eval();

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

	template<Mat4Expr M, Vec3Expr V, typename T>
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
			s.x, 			up2.x, 			-f.x, 			1,
			s.y, 			up2.y, 			-f.y, 			1,
			s.z, 			up2.z, 			-f.z, 			1,
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
			s.x, 			up2.x, 			f.x, 			1,
			s.y, 			up2.y, 			f.y, 			1,
			s.z, 			up2.z, 			f.z, 			1,
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
}//namespace glm
