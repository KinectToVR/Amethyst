#pragma once
#include "pch.h"

namespace EigenUtils
{
	/// @brief Returns a lerp'ed quaternion
	/// @see glm::lerp
	template <typename _Scalar>
	Eigen::Vector3<_Scalar> lerp(const Eigen::Vector3<_Scalar>& to, const Eigen::Vector3<_Scalar>& from, float t)
	{
		return from * t + to * ((_Scalar)1.f - t);
	}

	/// @brief Returns a perspective transformation matrix like the one from gluPerspective
	/// @see http://www.opengl.org/sdk/docs/man2/xhtml/gluPerspective.xml
	/// @see glm::perspective
	template <typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> perspective(Scalar fovy, Scalar aspect, Scalar zNear, Scalar zFar)
	{
		Eigen::Transform<Scalar, 3, Eigen::Projective> tr;
		tr.matrix().setZero();
		assert(aspect > 0);
		assert(zFar > zNear);
		assert(zNear > 0);
		Scalar radf = 3.14159265358979323846 * fovy / 180.0;
		Scalar tan_half_fovy = std::tan(radf / 2.0);
		tr(0, 0) = 1.0 / (aspect * tan_half_fovy);
		tr(1, 1) = 1.0 / (tan_half_fovy);
		tr(2, 2) = -(zFar + zNear) / (zFar - zNear);
		tr(3, 2) = -1.0;
		tr(2, 3) = -(2.0 * zFar * zNear) / (zFar - zNear);
		return tr.matrix();
	}

	template <typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> scale(Scalar x, Scalar y, Scalar z)
	{
		Eigen::Transform<Scalar, 3, Eigen::Affine> tr;
		tr.matrix().setZero();
		tr(0, 0) = x;
		tr(1, 1) = y;
		tr(2, 2) = z;
		tr(3, 3) = 1;
		return tr.matrix();
	}

	template <typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> translate(Scalar x, Scalar y, Scalar z)
	{
		Eigen::Transform<Scalar, 3, Eigen::Affine> tr;
		tr.matrix().setIdentity();
		tr(0, 3) = x;
		tr(1, 3) = y;
		tr(2, 3) = z;
		return tr.matrix();
	}

	/// @brief Returns a view transformation matrix like the one from glu's lookAt
	/// @see http://www.opengl.org/sdk/docs/man2/xhtml/gluLookAt.xml
	/// @see glm::lookAt
	template <typename Derived>
	Eigen::Matrix<typename Derived::Scalar, 4, 4> lookAt(Derived const& eye, Derived const& center, Derived const& up)
	{
		typedef Eigen::Matrix<typename Derived::Scalar, 4, 4> Matrix4;
		typedef Eigen::Matrix<typename Derived::Scalar, 3, 1> Vector3;
		Vector3 f = (center - eye).normalized();
		Vector3 u = up.normalized();
		Vector3 s = f.cross(u).normalized();
		u = s.cross(f);
		Matrix4 mat = Matrix4::Zero();
		mat(0, 0) = s.x();
		mat(0, 1) = s.y();
		mat(0, 2) = s.z();
		mat(0, 3) = -s.dot(eye);
		mat(1, 0) = u.x();
		mat(1, 1) = u.y();
		mat(1, 2) = u.z();
		mat(1, 3) = -u.dot(eye);
		mat(2, 0) = -f.x();
		mat(2, 1) = -f.y();
		mat(2, 2) = -f.z();
		mat(2, 3) = f.dot(eye);
		mat.row(3) << 0, 0, 0, 1;
		return mat;
	}

	/// @see glm::ortho
	template <typename Scalar>
	Eigen::Matrix<Scalar, 4, 4> ortho(Scalar const& left,
	                                  Scalar const& right,
	                                  Scalar const& bottom,
	                                  Scalar const& top,
	                                  Scalar const& zNear,
	                                  Scalar const& zFar)
	{
		Eigen::Matrix<Scalar, 4, 4> mat = Eigen::Matrix<Scalar, 4, 4>::Identity();
		mat(0, 0) = Scalar(2) / (right - left);
		mat(1, 1) = Scalar(2) / (top - bottom);
		mat(2, 2) = -Scalar(2) / (zFar - zNear);
		mat(3, 0) = -(right + left) / (right - left);
		mat(3, 1) = -(top + bottom) / (top - bottom);
		mat(3, 2) = -(zFar + zNear) / (zFar - zNear);
		return mat;
	}

	/// @brief Returns a Quaternion from Euler angles in Vector3
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1AngleAxis.html
	/// @see https://stackoverflow.com/questions/31589901/euler-to-quaternion-quaternion-to-euler-using-eigen
	template <typename Derived>
	Eigen::Quaternion<typename Derived::Scalar> EulersToQuat(Derived const& eulers)
	{
		typedef Eigen::Matrix<typename Derived::Scalar, 3, 1> Vector3;

		return Eigen::Quaternionf(
			Eigen::AngleAxisf(eulers(0), Vector3::UnitX())
			* Eigen::AngleAxisf(eulers(1), Vector3::UnitY())
			* Eigen::AngleAxisf(eulers(2), Vector3::UnitZ()));
	}

	/// @brief Returns Eulers in Vector3 from Quat
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1AngleAxis.html
	/// @see https://stackoverflow.com/questions/31589901/euler-to-quaternion-quaternion-to-euler-using-eigen
	template <typename Derived>
	Eigen::Matrix<typename Derived::Scalar, 3, 1> QuatToEulers(Derived const& quat)
	{
		return quat.toRotationMatrix().eulerAngles(0, 1, 2);
	}

	/// <summary>
	/// Construct a rotation quaternion for which would rotate 'from' onto 'to' vector
	/// </summary>
	/// <typeparam name="Derived">Should be Vector3[f || d]</typeparam>
	/// <param name="from">Pose vector from where we're looking from</param>
	/// <param name="to">Pose vector to where we're looking at</param>
	/// <param name="base">Where would object point if we applied an empty quaternion</param>
	/// <returns>Eigen quaternion for translating from -> to OR base -> from - to</returns>
	/// @see https://answers.ros.org/question/31006/how-can-a-vector3-axis-be-used-to-produce-a-quaternion/
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1Quaternion.html
	template <typename Derived>
	Eigen::Quaternion<typename Derived::Scalar> DirectionQuat(Derived const& from, Derived const& to,
	                                                          Derived const& base)
	{
		return Eigen::Quaternionf::FromTwoVectors(base, to - from);
	}

	/**
	* \brief This template will let us convert between different types
	* \tparam Ret What type should be returned from function
	* \tparam T Class of the parameter 'in'
	* \param in Parameter, object which will be 'converted'
	* \return Returns 'in' converted to 'ret' return type
	*/
	template <typename Ret, class T>
	auto p_cast_type(const T& in)
	{
		/* If somehow same */
		if constexpr (std::is_same<Ret, T>::value) return in;

			/* To Eigen Quaternion */
		else if constexpr (std::is_same<Ret, Eigen::Quaternionf>::value && std::is_same<T, vr::HmdQuaternion_t>::value)
			return Eigen::Quaternionf(in.w, in.x, in.y, in.z);

			/* To OpenVR Quaternion */
		else if constexpr (std::is_same<Ret, vr::HmdQuaternion_t>::value && std::is_same<T, Eigen::Quaternionf>::value)
			return vr::HmdQuaternion_t{in.w(), in.x(), in.y(), in.z()};

			/* To Eigen Vector3f */
		else if constexpr (std::is_same<Ret, Eigen::Vector3f>::value && std::is_same<T, vr::HmdVector3d_t>::value)
			return Eigen::Vector3f(in.v[0], in.v[1], in.v[2]);

		else if constexpr (std::is_same<Ret, Eigen::Vector3f>::value && std::is_same<T, vr::HmdMatrix34_t>::value)
			return Eigen::Vector3f(in.m[0][3], in.m[1][3], in.m[2][3]);

			/* To OpenVR HmdVector3d_t */
		else if constexpr (std::is_same<Ret, vr::HmdVector3d_t>::value && std::is_same<T, Eigen::Vector3f>::value)
			return vr::HmdVector3d_t{in.x(), in.y(), in.z()};

		else if constexpr (std::is_same<Ret, vr::HmdVector3d_t>::value && std::is_same<T, vr::HmdMatrix34_t>::value)
			return vr::HmdVector3d_t{in.m[0][3], in.m[1][3], in.m[2][3]};

			/* From OpenVR Matrix to OpenVR Quaternion */
		else if constexpr (std::is_same<Ret, vr::HmdQuaternion_t>::value && std::is_same<T, vr::HmdMatrix34_t>::value)
		{
			auto q = vr::HmdQuaternion_t{1., 0., 0., 0.};
			q.w = sqrt(fmax(0, 1 + in.m[0][0] + in.m[1][1] + in.m[2][2])) / 2;
			q.x = sqrt(fmax(0, 1 + in.m[0][0] - in.m[1][1] - in.m[2][2])) / 2;
			q.y = sqrt(fmax(0, 1 - in.m[0][0] + in.m[1][1] - in.m[2][2])) / 2;
			q.z = sqrt(fmax(0, 1 - in.m[0][0] - in.m[1][1] + in.m[2][2])) / 2;
			q.x = copysign(q.x, in.m[2][1] - in.m[1][2]);
			q.y = copysign(q.y, in.m[0][2] - in.m[2][0]);
			q.z = copysign(q.z, in.m[1][0] - in.m[0][1]);
			return q;
		}

		/* From OpenVR Matrix to Eigen Quaternion */
		else if constexpr (std::is_same<Ret, Eigen::Quaternionf>::value && std::is_same<T, vr::HmdMatrix34_t>::value)
			return p_cast_type<Eigen::Quaternionf>(p_cast_type<vr::HmdQuaternion_t>(in));
	}

	using PointSet = Eigen::Matrix<double, 3, Eigen::Dynamic>;

	inline std::tuple<Eigen::Matrix3d, Eigen::Vector3d>
		rigid_transform_3D(const PointSet& A, const PointSet& B)
	{
		static_assert(PointSet::RowsAtCompileTime == 3);
		assert(A.cols() == B.cols());

		// find mean column wise
		const Eigen::Vector3d centroid_A = A.rowwise().mean();
		const Eigen::Vector3d centroid_B = B.rowwise().mean();

		// subtract mean
		PointSet Am = A.colwise() - centroid_A;
		PointSet Bm = B.colwise() - centroid_B;

		PointSet H = Am * Bm.transpose();

		//
		//# sanity check
		//#if linalg.matrix_rank(H) < 3:
		//	#    raise ValueError("rank of H = {}, expecting 3".format(linalg.matrix_rank(H)))
		//

		// find rotation
		Eigen::JacobiSVD<Eigen::Matrix3Xd> svd = H.jacobiSvd(
			Eigen::DecompositionOptions::ComputeFullU | Eigen::DecompositionOptions::ComputeFullV);
		const Eigen::Matrix3d& U = svd.matrixU();
		Eigen::MatrixXd V = svd.matrixV();
		Eigen::Matrix3d R = V * U.transpose();

		// special reflection case
		if (R.determinant() < 0.0f)
		{
			V.col(2) *= -1.0f;
			R = V * U.transpose();
		}

		const Eigen::Vector3d t = -R * centroid_A + centroid_B;

		return std::make_tuple(R, t);
	}
}
