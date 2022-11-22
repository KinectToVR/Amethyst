#pragma once

#pragma unmanaged
#include <Eigen/Dense>
#pragma managed

using namespace System;
using namespace Numerics;
using namespace Collections::Generic;

using namespace Amethyst::Plugins::Contract;

namespace AmethystSupport
{
	public ref class Calibration sealed
	{
	public:
		static Tuple<Vector3, Quaternion>^ SVD(
			List<Vector3>^ head_positions, List<Vector3>^ hmd_positions)
		{
			// MVP Korejan

			// Copy calibration points
			Eigen::Matrix<float, 3, Eigen::Dynamic>
				head_points(3, head_positions->Count),
				hmd_points(3, hmd_positions->Count);

			for (int32_t i_point = 0; i_point < head_positions->Count; i_point++)
			{
				head_points(0, i_point) = head_positions[i_point].X;
				head_points(1, i_point) = head_positions[i_point].Y;
				head_points(2, i_point) = head_positions[i_point].Z;

				hmd_points(0, i_point) = hmd_positions[i_point].X;
				hmd_points(1, i_point) = hmd_positions[i_point].Y;
				hmd_points(2, i_point) = hmd_positions[i_point].Z;
			}

			// Assert we're still okay here
			static_assert(Eigen::Matrix<float, 3, Eigen::Dynamic>::RowsAtCompileTime == 3);
			assert(head_points.cols() == hmd_points.cols());

			// Find mean column wise
			const Eigen::Vector3f centroid_a = head_points.rowwise().mean();
			const Eigen::Vector3f centroid_b = hmd_points.rowwise().mean();

			// Subtract mean
			const Eigen::Matrix<float, 3, Eigen::Dynamic> Am = head_points.colwise() - centroid_a;
			const Eigen::Matrix<float, 3, Eigen::Dynamic> Bm = hmd_points.colwise() - centroid_b;

			Eigen::Matrix<float, 3, Eigen::Dynamic> H = Am * Bm.transpose();

			// Find rotation
			const auto svd = H.jacobiSvd<
				Eigen::DecompositionOptions::ComputeFullU |
				Eigen::DecompositionOptions::ComputeFullV>();

			const Eigen::Matrix3f& result_u = svd.matrixU();
			Eigen::MatrixXf result_v = svd.matrixV();
			Eigen::Matrix3f return_rotation = result_v * result_u.transpose();

			// Special reflection case
			if (return_rotation.determinant() < 0.0f)
			{
				result_v.col(2) *= -1.0f;
				return_rotation = result_v * result_u.transpose();
			}

			const Eigen::Vector3f return_translation = -return_rotation * centroid_a + centroid_b;
			const auto rotation_q = Eigen::Quaternionf(return_rotation);

			return gcnew Tuple<Vector3, Quaternion>(
				Vector3(return_translation.x(), return_translation.y(), return_translation.z()),
				Quaternion(rotation_q.x(), rotation_q.y(), rotation_q.z(), rotation_q.w()));
		}

		static double OrientationDot(Quaternion from, Quaternion to)
		{
			/* Convert from matrices to directional vectors */

			// Probably the orientation of the VR HMD -> vector
			Eigen::Vector3f from_vector = Eigen::Quaternionf(
				from.W, from.X, from.Y, from.Z) * Eigen::Vector3f(0, 0, 1);

			// Probably the calibration rotation -> vector
			Eigen::Vector3f to_vector = Eigen::Quaternionf(
				to.W, to.X, to.Y, to.Z) * Eigen::Vector3f(0, 0, 1);

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

		static double QuaternionYaw(Quaternion q)
		{
			// Get current yaw angle
			Eigen::Vector3f projected_orientation_forward_vector =
				Eigen::Quaternionf(q.W, q.X, q.Y, q.Z) * Eigen::Vector3f(0, 0, 1);

			// Nullify [y] to ort-project the vector
			projected_orientation_forward_vector.y() = 0;

			// Get current yaw angle (.y)
			return Eigen::Quaternionf::FromTwoVectors(
				Eigen::Vector3f(0, 0, 1), // To-Front
				projected_orientation_forward_vector // To-Base
			).toRotationMatrix().eulerAngles(0, 1, 2).y();
		}

		static Quaternion FixFlippedOrientation(Quaternion base)
		{
			// Prepare an eigen'd version
			Eigen::Quaternionf orientation(base.W, base.X, base.Y, base.Z);

			// Remove the pitch angle
			// Grab original orientations and make them euler angles
			Eigen::Vector3f tracker_ori_with_yaw =
				orientation.toRotationMatrix().eulerAngles(0, 1, 2);

			// Remove pitch from eulers and apply to the parent
			orientation = Eigen::Quaternionf(
				Eigen::AngleAxisf(tracker_ori_with_yaw.x(), Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(-tracker_ori_with_yaw.y(), Eigen::Vector3f::UnitY())
				* Eigen::AngleAxisf(-tracker_ori_with_yaw.z(), Eigen::Vector3f::UnitZ()));

			orientation = Eigen::Quaternionf( // Apply the turn-around flip quaternion
				Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitX())
				* Eigen::AngleAxisf(static_cast<float>(Math::PI), Eigen::Vector3f::UnitY())
				* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ())) * orientation;

			// Compose and return
			return Quaternion(
				orientation.x(), orientation.y(),
				orientation.z(), orientation.w());
		}

		static Quaternion FeetSoftwareOrientation(
			TrackedJoint^ ankle, TrackedJoint^ foot, TrackedJoint^ knee)
		{
			return Quaternion::Identity;
		}

		static Quaternion FeetSoftwareOrientationV2(
			TrackedJoint^ ankle, TrackedJoint^ foot, TrackedJoint^ knee)
		{
			return Quaternion::Identity;
		}
	};
}
