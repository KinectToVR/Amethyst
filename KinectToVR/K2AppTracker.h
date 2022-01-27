#pragma once
#include "pch.h"
#include "EigenUtils.h"
#include "LowPassFilter.h"
#include "KalmanFilter.h"

namespace k2app
{
	// Rotation tracking option enumeration
	enum JointRotationTrackingOption
	{
		// Default - internal
		k2_DeviceInferredRotation,
		// Calculated rotation - Optional and feet-only
		k2_SoftwareCalculatedRotation,
		// Copy rotation from HMD
		k2_FollowHMDRotation,
		// Completely disable rotation
		k2_DisableJointRotation
	};

	// Tracking filter option enumeration - position
	enum PositionTrackingFilterOption
	{
		// Interpolation
		k2_PositionTrackingFilter_LERP,
		// Low pass filter
		k2_PositionTrackingFilter_Lowpass,
		// Extended Kalman
		k2_PositionTrackingFilter_Kalman,
		// Filter Off
		k2_NoPositionTrackingFilter 
	};

	// Tracking filter option enumeration - rotation
	enum RotationTrackingFilterOption
	{
		// Spherical interpolation
		k2_OrientationTrackingFilter_SLERP,
		// Spherical interpolation, but slower
		k2_OrientationTrackingFilter_SLERP_Slow,
		// Filter Off
		k2_NoOrientationTrackingFilter 
	};

	class K2AppTracker : public ktvr::K2TrackerBase
	{
	public:
		// Default constructors
		K2AppTracker() :
			lastLERPPosition{ pose.position },
			lastSLERPOrientation{ pose.orientation },
			lastSLERPSlowOrientation{ pose.orientation }
		{
			// Init the Kalman filter
			for (auto& filter : kalmanFilter)
				filter.init();
		}
		~K2AppTracker() = default;

		// Custom constructor for role+serial
		K2AppTracker(std::string const& _serial, ktvr::ITrackerType const& _role) :
			lastLERPPosition{ pose.position },
			lastSLERPOrientation{ pose.orientation },
			lastSLERPSlowOrientation{ pose.orientation }
		{
			// Copy serial and role
			data.serial = _serial;
			data.role = _role;

			// Init the Kalman filter
			for (auto& filter : kalmanFilter)
				filter.init();
		}

		// Position filter update option
		int positionTrackingFilterOption = k2_NoPositionTrackingFilter,
		    orientationTrackingFilterOption = k2_NoOrientationTrackingFilter;

		// Should values be overwritten (for default ones)
		bool overwriteDefaultSerial = false;

		// Internal data offset
		Eigen::Vector3f positionOffset = Eigen::Vector3f(0, 0, 0);
		Eigen::Quaternionf orientationOffset = Eigen::Quaternionf(1, 0, 0, 0);

		// For internal filters
		void updatePositionFilters()
		{
			/* Update the Kalman filter */
			Eigen::VectorXd y[3] = {
					Eigen::VectorXd(1), Eigen::VectorXd(1), Eigen::VectorXd(1)
			}, u(1); // c == 1

			y[0] << pose.position.x();
			y[1] << pose.position.y();
			y[2] << pose.position.z();
			u << 0; // zero control input

			for (int i = 0; i < 3; i++) {
				kalmanFilter[i].predict(u);
				kalmanFilter[i].update(y[i]);
			}

			kalmanPosition = Eigen::Vector3f(
				kalmanFilter[0].state().x(),
				kalmanFilter[1].state().x(),
				kalmanFilter[2].state().x());

			/* Update the LowPass filter */
			lowPassPosition = Eigen::Vector3f(
				lowPassFilter[0].update(pose.position.x()),
				lowPassFilter[1].update(pose.position.y()),
				lowPassFilter[2].update(pose.position.z()));

			/* Update the LERP (mix) filter */
			LERPPosition = EigenUtils::lerp(lastLERPPosition, pose.position, 0.15);
			lastLERPPosition = LERPPosition; // Backup the position
		}

		void updateOrientationFilters()
		{
			/* Update the SLERP filter */
			SLERPOrientation = lastSLERPOrientation.slerp(0.6, pose.orientation);
			lastSLERPOrientation = pose.orientation; // Backup the orientation

			/* Update the Slower SLERP filter */
			SLERPSlowOrientation = lastSLERPSlowOrientation.slerp(0.3, pose.orientation);
			lastSLERPSlowOrientation = pose.orientation; // Backup the orientation
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		[[nodiscard]] Eigen::Vector3f getFilteredPosition(int filter = -1) const
		{
			int m_filter = filter;
			if (filter < 0)
				m_filter = positionTrackingFilterOption;

			switch (m_filter)
			{
			default:
				return pose.position;
			case k2_PositionTrackingFilter_LERP:
				return LERPPosition;
			case k2_PositionTrackingFilter_Lowpass:
				return lowPassPosition;
			case k2_PositionTrackingFilter_Kalman:
				return kalmanPosition;
			case k2_NoPositionTrackingFilter:
				return pose.position;
			}
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		[[nodiscard]] Eigen::Quaternionf getFilteredOrientation(int filter = -1) const
		{
			int m_filter = filter;
			if (filter < 0)
				m_filter = orientationTrackingFilterOption;

			switch (m_filter)
			{
			default:
				return pose.orientation;
			case k2_OrientationTrackingFilter_SLERP:
				return SLERPOrientation;
			case k2_OrientationTrackingFilter_SLERP_Slow:
				return SLERPSlowOrientation;
			case k2_NoOrientationTrackingFilter:
				return pose.orientation;
			}
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		// Additionally, this adds the offsets
		[[nodiscard]] Eigen::Vector3f getFullPosition(int filter = -1) const
		{
			return getFilteredPosition(filter) + positionOffset;
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		// Additionally, this adds the offsets
		[[nodiscard]] Eigen::Quaternionf getFullOrientation(int filter = -1) const
		{
			return getFilteredOrientation(filter) * orientationOffset;
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		// Additionally, this adds the offsets
		// Offset will be added after translation
		[[nodiscard]] Eigen::Vector3f getFullCalibratedPosition
		(
			Eigen::Matrix<float, 3, 3> rotationMatrix,
			Eigen::Matrix<float, 3, 1> translationVector,
			Eigen::Vector3f calibration_origin = Eigen::Vector3d::Zero(),
			int filter = -1
		) const
		{
			// Construct the current pose
			Eigen::Vector3f m_pose(
				getFilteredPosition(filter).x(),
				getFilteredPosition(filter).y(),
				getFilteredPosition(filter).z()
			);

			// Construct the calibrated pose
			Eigen::Matrix<float, 3, Eigen::Dynamic> m_pose_calibrated =
				(rotationMatrix * (m_pose - calibration_origin)).
				colwise() + translationVector + calibration_origin;

			// Construct the calibrated pose in eigen
			Eigen::Vector3f calibrated_pose_gl(
				m_pose_calibrated(0),
				m_pose_calibrated(1),
				m_pose_calibrated(2)
			);

			// Return the calibrated pose with offset
			return calibrated_pose_gl + positionOffset;
		}

		// Get tracker base
		// This is for updating the server with
		// exclusive filtered data from K2AppTracker
		// By default, the saved filter is selected
		// Offsets are added inside called methods
		[[nodiscard]] K2TrackerBase getTrackerBase
		(
			Eigen::Matrix<float, 3, 3> rotationMatrix,
			Eigen::Matrix<float, 3, 1> translationVector,
			Eigen::Vector3f calibration_origin,
			int pos_filter = -1, int ori_filter = -1
		) const
		{
			// Check if matrices are empty
			const bool not_calibrated =
				rotationMatrix.isZero() &&
				translationVector.isZero() &&
				calibration_origin.isZero();

			// Construct the return type
			K2TrackerBase tracker_base(
				ktvr::K2TrackerPose(
					getFullOrientation(ori_filter),
					not_calibrated
						? getFullPosition(pos_filter)
						: getFullCalibratedPosition(
							rotationMatrix, translationVector, calibration_origin, pos_filter)
				),
				ktvr::K2TrackerData(
					data.serial, static_cast<ktvr::ITrackerType>(data.role), data.isActive
				)
			);

			// Add id and return
			tracker_base.id = id;
			return tracker_base;
		}

		// Get tracker base
		// This is for updating the server with
		// exclusive filtered data from K2AppTracker
		// By default, the saved filter is selected
		// Offsets are added inside called methods
		[[nodiscard]] K2TrackerBase getTrackerBase(
			int pos_filter = -1, int ori_filter = -1) const
		{
			// Construct the return type
			K2TrackerBase tracker_base(
				ktvr::K2TrackerPose(
					getFullOrientation(ori_filter),
					getFullPosition(pos_filter)
				),
				ktvr::K2TrackerData(
					data.serial, static_cast<ktvr::ITrackerType>(data.role), data.isActive
				)
			);

			// Add id and return
			tracker_base.id = id;
			return tracker_base;
		}

		// Internal filters' datas
		Eigen::Vector3f kalmanPosition = Eigen::Vector3f(),
		                lowPassPosition = Eigen::Vector3f(), LERPPosition = Eigen::Vector3f();

		Eigen::Quaternionf SLERPOrientation = Eigen::Quaternionf(),
		                   SLERPSlowOrientation = Eigen::Quaternionf();

		// LERP data's backup
		Eigen::Vector3f lastLERPPosition;
		Eigen::Quaternionf lastSLERPOrientation,
		                   lastSLERPSlowOrientation;

		template <class Archive>
		void serialize(Archive& ar, const unsigned int version)
		{
			ar & boost::serialization::make_nvp("K2AppTracker",
			                                    boost::serialization::base_object<K2TrackerBase>(*this))
				& BOOST_SERIALIZATION_NVP(positionTrackingFilterOption)
				& BOOST_SERIALIZATION_NVP(orientationTrackingFilterOption)
				& BOOST_SERIALIZATION_NVP(overwriteDefaultSerial)
				& BOOST_SERIALIZATION_NVP(positionOffset)
				& BOOST_SERIALIZATION_NVP(orientationOffset);
		}

	private:
		/* Position filters */

		// Internal position filters
		LowPassFilter lowPassFilter[3] = {
			LowPassFilter(7.2, .005),
			LowPassFilter(7.2, .005),
			LowPassFilter(7.2, .005)
		};

		// Internal Kalman filter, must be initialized
		KalmanFilter kalmanFilter[3] = {
			KalmanFilter(), KalmanFilter(), KalmanFilter()
		};
	};
}
