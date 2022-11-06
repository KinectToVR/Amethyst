#pragma once
#include <Eigen/Dense>

namespace EigenUtils
{
	/// @brief Returns a lerp'ed quaternion
	/// @see glm::lerp
	template <typename _Scalar>
	Eigen::Vector3<_Scalar> lerp(const Eigen::Vector3<_Scalar>& to, const Eigen::Vector3<_Scalar>& from, float t)
	{
		return from * t + to * (static_cast<_Scalar>(1.f) - t);
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
	Eigen::Matrix<typename Derived::Scalar, 4, 4> lookAt(const Derived& eye, const Derived& center, const Derived& up)
	{
		using Matrix4 = Eigen::Matrix<typename Derived::Scalar, 4, 4>;
		using Vector3 = Eigen::Matrix<typename Derived::Scalar, 3, 1>;
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
	Eigen::Matrix<Scalar, 4, 4> ortho(const Scalar& left,
		const Scalar& right,
		const Scalar& bottom,
		const Scalar& top,
		const Scalar& zNear,
		const Scalar& zFar)
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
	Eigen::Quaternion<typename Derived::Scalar> EulersToQuat(const Derived& eulers)
	{
		using Vector3 = Eigen::Matrix<typename Derived::Scalar, 3, 1>;

		return Eigen::Quaternion<typename Derived::Scalar>(
			Eigen::AngleAxis<typename Derived::Scalar>(eulers(0), Vector3::UnitX())
			* Eigen::AngleAxis<typename Derived::Scalar>(eulers(1), Vector3::UnitY())
			* Eigen::AngleAxis<typename Derived::Scalar>(eulers(2), Vector3::UnitZ()));
	}

	/// @brief Returns Eulers in Vector3 from Quat
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1AngleAxis.html
	/// @see https://stackoverflow.com/questions/31589901/euler-to-quaternion-quaternion-to-euler-using-eigen
	template <typename Derived>
	Eigen::Matrix<typename Derived::Scalar, 3, 1> QuatToEulers(const Derived& quat)
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
	Eigen::Quaternion<typename Derived::Scalar> DirectionQuat(const Derived& from, const Derived& to,
		const Derived& base)
	{
		return Eigen::Quaternion<typename Derived::Scalar>::FromTwoVectors(base, to - from);
	}

	template <typename Scalar>
	Eigen::Quaternion<Scalar> QuaternionFromYaw(const Scalar& yaw)
	{
		return EulersToQuat(Eigen::Vector3<Scalar>(0, yaw, 0));
	}

	/// <summary>
	/// Construct a projected euler angles from a 3d full quaternion / rotation matrix
	/// </summary>
	/// <typeparam name="Derived">The return type, float/double/...</typeparam>
	/// <param name="rotation">The rotation we're projecting</param>
	/// <returns>Projected yaw from the provided rotation object</returns>
	/// @see https://answers.ros.org/question/31006/how-can-a-vector3-axis-be-used-to-produce-a-quaternion/
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1Quaternion.html
	template <typename Derived>
	Eigen::Vector3<typename Derived::Scalar> RotationProjectedEulerAngles(const Derived& rotation)
	{
		// Get current yaw angle
		Eigen::Vector3<typename Derived::Scalar> projected_orientation_forward_vector =
			rotation * Eigen::Vector3<typename Derived::Scalar>(0, 0, 1);

		// Nullify [y] to orto-project the vector
		projected_orientation_forward_vector.y() = 0;

		return QuatToEulers(
			Eigen::Quaternion<typename Derived::Scalar>::FromTwoVectors(
				Eigen::Vector3<typename Derived::Scalar>(0, 0, 1), // To-Front
				projected_orientation_forward_vector // To-Base
			));
	}

	/// <summary>
	/// Construct a projected 2d yaw from a 3d full quaternion / rotation matrix
	/// </summary>
	/// <typeparam name="Derived">The return type, float/double/...</typeparam>
	/// <param name="rotation">The rotation we're projecting</param>
	/// <returns>Projected yaw from the provided rotation object</returns>
	/// @see https://answers.ros.org/question/31006/how-can-a-vector3-axis-be-used-to-produce-a-quaternion/
	/// @see https://eigen.tuxfamily.org/dox/classEigen_1_1Quaternion.html
	template <typename Derived>
	typename Derived::Scalar RotationProjectedYaw(const Derived& rotation)
	{
		// Get current yaw angle
		return RotationProjectedEulerAngles(rotation).y(); // Yaw angle
	}

	/// <summary>
	/// Construct a projected 2d dot product full quaternion / rotation matrix
	/// </summary>
	/// <typeparam name="Derived">The return type, float/double/...</typeparam>
	/// <param name="rotation_from">The base rotation</param>
	/// <param name="rotation_to">The child rotation</param>
	/// <returns>
	/// Dot product (from 2 normalized vectors) from 2 provided rotations
	///	You can use basically any Eigen rotation data type
	/// </returns>
	template <typename Derived_R1, typename Derived_R2>
	typename Derived_R1::Scalar NormalizedRotationVectorDot(
		const Derived_R1& rotation_from, const Derived_R2& rotation_to)
	{
		// Create a typedef outta the container scalar type
		using vector3 = Eigen::Vector3<typename Derived_R1::Scalar>;

		/* Convert from matrices to directional vectors */

		// Probably the orientation of the VR HMD -> vector
		vector3 from_vector = rotation_from * vector3(0, 0, 1);
		// Probably the calibration rotation -> vector
		vector3 to_vector = rotation_to * vector3(0, 0, 1);

		// Cancel the y component
		// Ignore the pitch and roll components of the R,
		// as we only care about the yaw (+y) component
		from_vector.y() = 0.;
		to_vector.y() = 0.;

		// Since we removed the entire y component
		// The vectors won't be unit length,
		// so we must normalize them to unit length
		// as those have some really nice properties
		// (we can take some advantages of, ofc)
		from_vector.normalize();
		to_vector.normalize();

		/*
		 * Note:
		 *
		 * A dot product's properties are as follows
		 *     (when using two unit vectors, which is our case):
		 *
		 * Whenever the two vectors are pointing at the same direction
		 *     (i.e. ↑ ↑ ) the dot product is +1
		 *
		 * Whenever the two vectors are pointing away from each other
		 *     (i.e. ↑ ↓ ) the dot product is -1
		 *
		 * Whenever the two vectors are perpendicular to one another
		 *     (i.e. → ↑ ) the dot product is 0
		 */

		 // Now we have transformed the rotations ->
		 // we can compute the dot product outta them
		return from_vector.dot(to_vector); // return
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
