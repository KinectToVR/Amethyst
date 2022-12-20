#include "pch.h"
#include "Support.h"

#include "Support.g.cpp"       // NOLINT(bugprone-suspicious-include)
#include "LowPassFilter.g.cpp" // NOLINT(bugprone-suspicious-include)
#include "KalmanFilter.g.cpp"  // NOLINT(bugprone-suspicious-include)

#include "KalmanFilter.h"
#include <numbers>

// The upper bound of the fog volume along the y-axis (height)
// This defines how far up the fog extends from the floor plane
// This is hard coded because all V2 sensors are the same (afaik, I don't know if the fog height changes depending on room conditions though)
constexpr float SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD = 0.4f;

template <typename Derived>
Derived Lerp(Derived from, Derived to, const Derived delta)
{
	return from * (1 - delta) + to * delta;
}

template <typename Derived>
Eigen::Quaternion<typename Derived::Scalar> EulersToQuat(const Derived& eulers)
{
	using Vector3 = Eigen::Matrix<typename Derived::Scalar, 3, 1>;

	return Eigen::Quaternion<typename Derived::Scalar>(
		Eigen::AngleAxis<typename Derived::Scalar>(eulers(0), Vector3::UnitX())
		* Eigen::AngleAxis<typename Derived::Scalar>(eulers(1), Vector3::UnitY())
		* Eigen::AngleAxis<typename Derived::Scalar>(eulers(2), Vector3::UnitZ()));
}

namespace winrt::AmethystSupport::implementation
{
	STransform Support::SVD(
		array_view<const SVector3> head_positions,
		array_view<const SVector3> hmd_positions)
	{
		// MVP Korejan

		// Copy calibration points
		Eigen::Matrix<float, 3, Eigen::Dynamic>
			head_points(3, head_positions.size()),
			hmd_points(3, hmd_positions.size());

		for (int32_t i_point = 0; i_point < head_positions.size(); i_point++)
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
		const Eigen::Matrix3Xf H = Am * Bm.transpose();

		// Find rotation
		const auto svd = Eigen::JacobiSVD(
			H, Eigen::DecompositionOptions::ComputeFullU |
			Eigen::DecompositionOptions::ComputeFullV);

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

		return STransform{
			.Translation = SVector3{
				.X = return_translation.x(),
				.Y = return_translation.y(),
				.Z = return_translation.z()
			},
			.Rotation = SQuaternion{
				.X = rotation_q.x(),
				.Y = rotation_q.y(),
				.Z = rotation_q.z(),
				.W = rotation_q.w()
			}
		};
	}

	SQuaternion Support::FixFlippedOrientation(const SQuaternion& base)
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
			* Eigen::AngleAxisf(static_cast<float>(std::numbers::pi), Eigen::Vector3f::UnitY())
			* Eigen::AngleAxisf(0.f, Eigen::Vector3f::UnitZ())) * orientation;

		// Compose and return
		return SQuaternion{
			.X = orientation.x(),
			.Y = orientation.y(),
			.Z = orientation.z(),
			.W = orientation.w()
		};
	}

	float Support::OrientationDot(const SQuaternion& from, const SQuaternion& to)
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

	float Support::QuaternionYaw(const SQuaternion& q)
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

	SQuaternion Support::FeetSoftwareOrientation(const SVector3& ankle, const SVector3& foot, const SVector3& knee)
	{
		// Capture needed joints' positions
		const Eigen::Vector3f
			forward(0, 0, 1),
			ankleLeftPose(ankle.X, ankle.Y, ankle.Z),
			footLeftPose(foot.X, foot.Y, foot.Z),
			kneeLeftPose(knee.X, knee.Y, knee.Z);

		// Calculate euler yaw foot orientation, we'll need it later
		Eigen::Vector3f footLeftRawOrientation =
			Eigen::Quaternionf::FromTwoVectors(
				forward, Eigen::Vector3f(footLeftPose.x(), 0.f, footLeftPose.z()) -
				Eigen::Vector3f(ankleLeftPose.x(), 0.f, ankleLeftPose.z()))
			.toRotationMatrix().eulerAngles(0, 1, 2);

		// Flip the yaw around, without reversing it -> we need it basing to 0
		// (what an irony that we actually need to reverse it...)
		footLeftRawOrientation.y() *= -1.f;
		footLeftRawOrientation.y() += static_cast<float>(std::numbers::pi);

		// Make the yaw less sensitive
		// Decided to go for radians for the read-ability
		// (Although my code is shit anyway, and there'll be none in the end)
		float lsFixedYaw = footLeftRawOrientation.y() *
			180.f / static_cast<float>(std::numbers::pi);

		if (lsFixedYaw > 180.f && lsFixedYaw < 360.f)
			lsFixedYaw = 360.f - abs(lsFixedYaw - 360.f) * .5f;
		else if (lsFixedYaw < 180.f && lsFixedYaw > 0.f)
			lsFixedYaw *= .5f;

		// Apply to the base // Back to the RAD format
		footLeftRawOrientation.y() = lsFixedYaw * static_cast<float>(std::numbers::pi) / 180.f;

		// Calculate the knee-ankle orientation, aka "Tibia"
		// We aren't disabling look-thorough yaw, since it'll be 0
		Eigen::Quaternionf knee_ankleLeftOrientationQuaternion =
			Eigen::Quaternionf::FromTwoVectors(forward, ankleLeftPose - kneeLeftPose);

		// Now adjust some values like playspace yaw and pitch, additional rotations
		// -> they're facing purely down and Z / Y are flipped
		// Apply the fine-tuning to global variable
		knee_ankleLeftOrientationQuaternion = EulersToQuat(Eigen::Vector3f(
			static_cast<float>(std::numbers::pi) / 5.f, 0.f, 0.f)) * knee_ankleLeftOrientationQuaternion;

		// Grab original orientations and make them euler angles
		Eigen::Vector3f left_knee_ori_full = knee_ankleLeftOrientationQuaternion
		                                     .toRotationMatrix().eulerAngles(0, 1, 2);

		// Try to fix yaw and roll mismatch, caused by XYZ XZY mismatch
		knee_ankleLeftOrientationQuaternion = EulersToQuat(Eigen::Vector3f(
			left_knee_ori_full.x() - static_cast<float>(std::numbers::pi) / 1.6f,
			0.f, -left_knee_ori_full.y()));

		// All the rotations
		Eigen::Quaternionf calculatedLeftFootOrientation =
			EulersToQuat(footLeftRawOrientation) * knee_ankleLeftOrientationQuaternion;

		// Now adjust some values like playspace yaw and pitch, additional rotations
		calculatedLeftFootOrientation = EulersToQuat(
			Eigen::Vector3f(2.8623399733f, 0.f, 0.f)) * calculatedLeftFootOrientation;

		// Apply fixes

		// Grab original orientations and make them euler angles
		Eigen::Vector3f left_ori_vector = calculatedLeftFootOrientation
		                                  .toRotationMatrix().eulerAngles(0, 1, 2);

		// Kind of a solution for flipping at too big X
		// Found out during testing,
		// no other known mathematical reason (maybe except gimbal lock)
		if (left_ori_vector.y() <= 0.f
			&& left_ori_vector.y() >= -1.f
			&& left_ori_vector.z() <= -1.f
			&& left_ori_vector.z() >= -static_cast<float>(std::numbers::pi))
			left_ori_vector.y() += -static_cast<float>(std::numbers::pi);

		// Apply to the base
		calculatedLeftFootOrientation = EulersToQuat(left_ori_vector);

		// Compose and return
		return SQuaternion{
			.X = calculatedLeftFootOrientation.x(),
			.Y = calculatedLeftFootOrientation.y(),
			.Z = calculatedLeftFootOrientation.z(),
			.W = calculatedLeftFootOrientation.w()
		};
	}

	SQuaternion Support::FeetSoftwareOrientationV2(const SVector3& ankle, const SVector3& knee)
	{
		// Capture needed joints' positions
		const Eigen::Vector3f
			anklePose(ankle.X, ankle.Y, ankle.Z),
			kneePose(knee.X, knee.Y, knee.Z);

		// The improved approach for fixing foot rotation on the Xbox One Kinect
		// Thigh rotation is copied onto the foot, this is due to the fact that more often than not, 
		// the thigh is facing the same direction as your foot.  
		// Given the foot is an unreliable mess due to the fog on the Xbox One Kinect, 
		// The ankle position is stable though, so we can use it.
		Eigen::Vector3f legsDir = kneePose - anklePose;

		// Normalize the direction to have a length of 1
		legsDir.normalize();

		// tend towards 0 below the fog threshold
		// Remove the pitch entirely if within the fog area
		legsDir.y() = // smoothly interpolate between 0 and y^2
			Lerp(legsDir.y(),

			     // from 0 to y^2 (where 1 is fog threshold)
			     Lerp(0.f, legsDir.y() * legsDir.y(),

			          // such that we remap the y direction relative to the threshold between 0 and 1
			          std::max(std::min(SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD,
			                            legsDir.y()), 0.f) / SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD),

			     // from 0.8 * fog threshold to 1.2 * fog threshold
			     std::min(1.f, std::max(0.f, 0.4f * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD
			                            * legsDir.y() - 0.8f * SOFTWARE_CALCULATED_ROTATION_FOG_THRESHOLD)));

		legsDir.normalize(); // Normalize the direction to have a length of 1
		auto calculatedLeftFootOrientation = Eigen::Quaternionf::
			FromTwoVectors(Eigen::Vector3f::UnitX(), legsDir - Eigen::Vector3f::UnitX());

		return SQuaternion{
			.X = calculatedLeftFootOrientation.x(),
			.Y = calculatedLeftFootOrientation.y(),
			.Z = calculatedLeftFootOrientation.z(),
			.W = calculatedLeftFootOrientation.w()
		};
	}

	LowPassFilter::LowPassFilter(float cutOff, float deltaTime) :
		e_pow_(1.0 - exp(-deltaTime * 2.0 * std::numbers::pi * cutOff))
	{
		if (cutOff <= 0 || deltaTime <= 0)e_pow_ = 0.0;
	}

	SVector3 LowPassFilter::Update(const SVector3& input)
	{
		output_ = SVector3{
			.X = (input.X - output_.X) * e_pow_,
			.Y = (input.Y - output_.Y) * e_pow_,
			.Z = (input.Z - output_.Z) * e_pow_,
		};

		return output_; // Update and return
	}

	KalmanFilter::KalmanFilter()
	{
		for (auto& filter : filters_)
			filter.init(); // Setup!
	}

	SVector3 KalmanFilter::Update(const SVector3& input)
	{
		Eigen::VectorXf u(1);
		Eigen::VectorXf y[3] =
		{
			Eigen::VectorXf(1),
			Eigen::VectorXf(1),
			Eigen::VectorXf(1)
		};

		y[0] << input.X;
		y[1] << input.Y;
		y[2] << input.Z;
		u << 0; // zero control input

		for (auto& filter : filters_)
			filter.predict(u);

		filters_[0].update(y[0]);
		filters_[1].update(y[1]);
		filters_[2].update(y[2]);

		return SVector3{
			.X = filters_[0].state().x(),
			.Y = filters_[1].state().x(),
			.Z = filters_[2].state().x()
		};
	}
}
