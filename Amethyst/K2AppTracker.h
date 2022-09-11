#pragma once
#include "pch.h"
#include "EigenUtils.h"
#include "LowPassFilter.h"
#include "KalmanFilter.h"

#include <cereal/access.hpp>
#include <cereal/types/vector.hpp>
#include <cereal/types/tuple.hpp>
#include <cereal/types/polymorphic.hpp>
#include <cereal/types/utility.hpp>
#include <cereal/specialize.hpp>
#include <cereal/archives/xml.hpp>

/* Eigen serialization */
namespace cereal
{
	template <class A>
	std::string CEREAL_SAVE_MINIMAL_FUNCTION_NAME(const A&, const std::wstring& in)
	{
		return WStringToString(in);
	}

	template <class A>
	void CEREAL_LOAD_MINIMAL_FUNCTION_NAME(const A&, std::wstring& out, const std::string& in)
	{
		out = StringToWString(in);
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& archive,
	               Eigen::Quaternion<_Scalar>& q)
	{
		archive(
			make_nvp("w", q.w()),
			make_nvp("x", q.x()),
			make_nvp("y", q.y()),
			make_nvp("z", q.z())
		);
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& archive,
	               Eigen::Vector3<_Scalar>& v)
	{
		archive(
			make_nvp("x", v.x()),
			make_nvp("y", v.y()),
			make_nvp("z", v.z())
		);
	}

	template <class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
	void serialize(Archive& archive,
	               Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& t)
	{
		for (size_t i = 0; i < t.size(); i++)
			archive(make_nvp(("m" + std::to_string(i)).c_str(), t.data()[i]));
	}
}

CEREAL_SPECIALIZE_FOR_ALL_ARCHIVES(std::wstring, cereal::specialization::non_member_load_save_minimal);

namespace k2app
{
	// Rotation tracking option enumeration
	// THIS IS FOR THE APP ONLY, NOT USED BY K2APPTRACKER
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

	// Tracking filter option enumeration - position
	enum JointPositionTrackingOption
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

	// Mapping enum to string for eliminating if-else loop
	inline std::map<ktvr::ITrackerType, std::string> ITrackerType_Serial
	{
		{ktvr::ITrackerType::Tracker_Handed, "vive_tracker_handed"},
		{ktvr::ITrackerType::Tracker_LeftFoot, "vive_tracker_left_foot"},
		{ktvr::ITrackerType::Tracker_RightFoot, "vive_tracker_right_foot"},
		{ktvr::ITrackerType::Tracker_LeftShoulder, "vive_tracker_left_Shoulder"},
		{ktvr::ITrackerType::Tracker_RightShoulder, "vive_tracker_right_shoulder"},
		{ktvr::ITrackerType::Tracker_LeftElbow, "vive_tracker_left_elbow"},
		{ktvr::ITrackerType::Tracker_RightElbow, "vive_tracker_right_elbow"},
		{ktvr::ITrackerType::Tracker_LeftKnee, "vive_tracker_left_knee"},
		{ktvr::ITrackerType::Tracker_RightKnee, "vive_tracker_right_knee"},
		{ktvr::ITrackerType::Tracker_Waist, "vive_tracker_waist"},
		{ktvr::ITrackerType::Tracker_Chest, "vive_tracker_chest"},
		{ktvr::ITrackerType::Tracker_Camera, "vive_tracker_camera"},
		{ktvr::ITrackerType::Tracker_Keyboard, "vive_tracker_keyboard"}
	};

	inline std::map<ktvr::ITrackerType, std::string> ITrackerType_Role_Serial
	{
		{ktvr::ITrackerType::Tracker_Handed, "AME-HANDED"},
		{ktvr::ITrackerType::Tracker_LeftFoot, "AME-LFOOT"},
		{ktvr::ITrackerType::Tracker_RightFoot, "AME-RFOOT"},
		{ktvr::ITrackerType::Tracker_LeftShoulder, "AME-LSHOULDER"},
		{ktvr::ITrackerType::Tracker_RightShoulder, "AME-RSHOULDER"},
		{ktvr::ITrackerType::Tracker_LeftElbow, "AME-LELBOW"},
		{ktvr::ITrackerType::Tracker_RightElbow, "AME-RELBOW"},
		{ktvr::ITrackerType::Tracker_LeftKnee, "AME-LKNEE"},
		{ktvr::ITrackerType::Tracker_RightKnee, "AME-RKNEE"},
		{ktvr::ITrackerType::Tracker_Waist, "AME-WAIST"},
		{ktvr::ITrackerType::Tracker_Chest, "AME-CHEST"},
		{ktvr::ITrackerType::Tracker_Camera, "AME-CAMERA"},
		{ktvr::ITrackerType::Tracker_Keyboard, "AME-KEYBOARD"}
	};

	inline std::map<ktvr::ITrackerType, ktvr::ITrackedJointType> ITrackerType_Joint
	{
		{ktvr::ITrackerType::Tracker_Handed, ktvr::ITrackedJointType::Joint_HandLeft},
		{ktvr::ITrackerType::Tracker_LeftFoot, ktvr::ITrackedJointType::Joint_AnkleLeft},
		{ktvr::ITrackerType::Tracker_RightFoot, ktvr::ITrackedJointType::Joint_AnkleRight},
		{ktvr::ITrackerType::Tracker_LeftShoulder, ktvr::ITrackedJointType::Joint_ShoulderLeft},
		{ktvr::ITrackerType::Tracker_RightShoulder, ktvr::ITrackedJointType::Joint_ShoulderRight},
		{ktvr::ITrackerType::Tracker_LeftElbow, ktvr::ITrackedJointType::Joint_ElbowLeft},
		{ktvr::ITrackerType::Tracker_RightElbow, ktvr::ITrackedJointType::Joint_ElbowRight},
		{ktvr::ITrackerType::Tracker_LeftKnee, ktvr::ITrackedJointType::Joint_KneeLeft},
		{ktvr::ITrackerType::Tracker_RightKnee, ktvr::ITrackedJointType::Joint_KneeRight},
		{ktvr::ITrackerType::Tracker_Waist, ktvr::ITrackedJointType::Joint_SpineWaist},
		{ktvr::ITrackerType::Tracker_Chest, ktvr::ITrackedJointType::Joint_SpineMiddle},
		{ktvr::ITrackerType::Tracker_Camera, ktvr::ITrackedJointType::Joint_Head},
		{ktvr::ITrackerType::Tracker_Keyboard, ktvr::ITrackedJointType::Joint_HandRight}
	};

	inline std::map<ktvr::ITrackedJointType, ktvr::ITrackerType> Joint_ITrackerType
	{
		{ktvr::ITrackedJointType::Joint_HandLeft, ktvr::ITrackerType::Tracker_Handed},
		{ktvr::ITrackedJointType::Joint_AnkleLeft, ktvr::ITrackerType::Tracker_LeftFoot},
		{ktvr::ITrackedJointType::Joint_AnkleRight, ktvr::ITrackerType::Tracker_RightFoot},
		{ktvr::ITrackedJointType::Joint_ShoulderLeft, ktvr::ITrackerType::Tracker_LeftShoulder},
		{ktvr::ITrackedJointType::Joint_ShoulderRight, ktvr::ITrackerType::Tracker_RightShoulder},
		{ktvr::ITrackedJointType::Joint_ElbowLeft, ktvr::ITrackerType::Tracker_LeftElbow},
		{ktvr::ITrackedJointType::Joint_ElbowRight, ktvr::ITrackerType::Tracker_RightElbow},
		{ktvr::ITrackedJointType::Joint_KneeLeft, ktvr::ITrackerType::Tracker_LeftKnee},
		{ktvr::ITrackedJointType::Joint_KneeRight, ktvr::ITrackerType::Tracker_RightKnee},
		{ktvr::ITrackedJointType::Joint_SpineWaist, ktvr::ITrackerType::Tracker_Waist},
		{ktvr::ITrackedJointType::Joint_SpineMiddle, ktvr::ITrackerType::Tracker_Chest},
		{ktvr::ITrackedJointType::Joint_Head, ktvr::ITrackerType::Tracker_Camera},
		{ktvr::ITrackedJointType::Joint_HandRight, ktvr::ITrackerType::Tracker_Keyboard}
	};

	class K2AppTracker
	{
		friend class cereal::access;

	public:
		// Default constructors
		K2AppTracker()
		{
			// Init the Kalman filter
			for (auto& filter : kalmanFilter)
				filter.init();
		}

		~K2AppTracker() = default;

		// Custom constructor for role+serial
		K2AppTracker(const std::string& _serial, const ktvr::ITrackerType& _role)
		{
			// Copy serial and role
			data_serial = _serial;
			data_role = _role;

			// Init the Kalman filter
			for (auto& filter : kalmanFilter)
				filter.init();
		}

		// Custom constructor for role+serial+filter const
		K2AppTracker(const std::string& _serial, const ktvr::ITrackerType& _role, const double& m_lerp_const)
		{
			// Copy serial and role
			data_serial = _serial;
			data_role = _role;

			// Init the Kalman filter
			for (auto& filter : kalmanFilter)
				filter.init();

			// Overwrite the lerp const
			_lerp_const = m_lerp_const;
		}

		ktvr::K2TrackedJoint getK2TrackedJoint(const bool& _state, const std::string& _name)
		const
		{
			return ktvr::K2TrackedJoint(pose_position, pose_orientation,
			                            _state ? ktvr::State_Tracked : ktvr::State_NotTracked, _name);
		}

		ktvr::K2TrackedJoint getK2TrackedJoint()
		const
		{
			return ktvr::K2TrackedJoint(pose_position, pose_orientation,
			                            data_isActive ? ktvr::State_Tracked : ktvr::State_NotTracked, data_serial);
		}

		// For internal filters
		void updatePositionFilters()
		{
			/* Update the Kalman filter */
			Eigen::VectorXd y[3] = {
				                Eigen::VectorXd(1), Eigen::VectorXd(1), Eigen::VectorXd(1)
			                }, u(1); // c == 1

			y[0] << pose_position.x();
			y[1] << pose_position.y();
			y[2] << pose_position.z();
			u << 0; // zero control input

			for (int i = 0; i < 3; i++)
			{
				kalmanFilter[i].predict(u);
				kalmanFilter[i].update(y[i]);
			}

			kalmanPosition = Eigen::Vector3f(
				kalmanFilter[0].state().x(),
				kalmanFilter[1].state().x(),
				kalmanFilter[2].state().x());

			/* Update the LowPass filter */
			lowPassPosition = Eigen::Vector3f(
				lowPassFilter[0].update(pose_position.x()),
				lowPassFilter[1].update(pose_position.y()),
				lowPassFilter[2].update(pose_position.z()));

			/* Update the LERP (mix) filter */
			LERPPosition = EigenUtils::lerp(lastLERPPosition, pose_position, _lerp_const);
			lastLERPPosition = LERPPosition; // Backup the position
		}

		void updateOrientationFilters()
		{
			// ik that's a bunch of normalizations but we really em, weird things happen sometimes

			/* Update the SLERP filter */
			SLERPOrientation = lastSLERPOrientation.normalized().slerp(0.25, pose_orientation.normalized());
			lastSLERPOrientation = pose_orientation.normalized(); // Backup the orientation

			/* Update the Slower SLERP filter */
			SLERPSlowOrientation = lastSLERPSlowOrientation.normalized().slerp(0.15, pose_orientation.normalized());
			lastSLERPSlowOrientation = pose_orientation.normalized(); // Backup the orientation
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
				return pose_position;
			case k2_PositionTrackingFilter_LERP:
				return LERPPosition;
			case k2_PositionTrackingFilter_Lowpass:
				return lowPassPosition;
			case k2_PositionTrackingFilter_Kalman:
				return kalmanPosition;
			case k2_NoPositionTrackingFilter:
				return pose_position;
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
				return pose_orientation;
			case k2_OrientationTrackingFilter_SLERP:
				return SLERPOrientation;
			case k2_OrientationTrackingFilter_SLERP_Slow:
				return SLERPSlowOrientation;
			case k2_NoOrientationTrackingFilter:
				return pose_orientation;
			}
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		// Additionally, this adds the offsets
		[[nodiscard]] Eigen::Vector3f getFullPosition(int filter = -1) const
		{
			return getFilteredPosition(filter) + positionOffset.cast<float>();
		}

		// Get filtered data
		// By default, the saved filter is selected,
		// and to select it, the filter number must be < 0
		// Additionally, this adds the offsets
		[[nodiscard]] Eigen::Quaternionf getFullOrientation(int filter = -1) const
		{
			return getFilteredOrientation(filter) * EigenUtils::EulersToQuat(orientationOffset.cast<float>());
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
			return calibrated_pose_gl + positionOffset.cast<float>();
		}

		// Get tracker base
		// This is for updating the server with
		// exclusive filtered data from K2AppTracker
		// By default, the saved filter is selected
		// Offsets are added inside called methods
		[[nodiscard]] ktvr::K2TrackerBase getTrackerBase
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
			ktvr::K2TrackerBase tracker_base;

			auto _full_orientation = getFullOrientation(ori_filter);
			auto _full_position = not_calibrated
				                      ? getFullPosition(pos_filter)
				                      : getFullCalibratedPosition(
					                      rotationMatrix, translationVector, calibration_origin, pos_filter);

			tracker_base.mutable_pose()->mutable_orientation()->set_w(_full_orientation.w());
			tracker_base.mutable_pose()->mutable_orientation()->set_x(_full_orientation.x());
			tracker_base.mutable_pose()->mutable_orientation()->set_y(_full_orientation.y());
			tracker_base.mutable_pose()->mutable_orientation()->set_z(_full_orientation.z());

			tracker_base.mutable_pose()->mutable_position()->set_x(_full_position.x());
			tracker_base.mutable_pose()->mutable_position()->set_y(_full_position.y());
			tracker_base.mutable_pose()->mutable_position()->set_z(_full_position.z());

			tracker_base.mutable_data()->set_serial(data_serial);
			tracker_base.mutable_data()->set_role(data_role);
			tracker_base.mutable_data()->set_isactive(data_isActive);

			// Add ID and return
			tracker_base.set_tracker(base_tracker);
			return tracker_base;
		}

		// Get tracker base
		// This is for updating the server with
		// exclusive filtered data from K2AppTracker
		// By default, the saved filter is selected
		// Offsets are added inside called methods
		[[nodiscard]] ktvr::K2TrackerBase getTrackerBase(
			int pos_filter = -1, int ori_filter = -1) const
		{
			// Construct the return type
			ktvr::K2TrackerBase tracker_base;

			auto _full_orientation = getFullOrientation(ori_filter);
			auto _full_position = getFullPosition(pos_filter);

			tracker_base.mutable_pose()->mutable_orientation()->set_w(_full_orientation.w());
			tracker_base.mutable_pose()->mutable_orientation()->set_x(_full_orientation.x());
			tracker_base.mutable_pose()->mutable_orientation()->set_y(_full_orientation.y());
			tracker_base.mutable_pose()->mutable_orientation()->set_z(_full_orientation.z());

			tracker_base.mutable_pose()->mutable_position()->set_x(_full_position.x());
			tracker_base.mutable_pose()->mutable_position()->set_y(_full_position.y());
			tracker_base.mutable_pose()->mutable_position()->set_z(_full_position.z());

			tracker_base.mutable_data()->set_serial(data_serial);
			tracker_base.mutable_data()->set_role(data_role);
			tracker_base.mutable_data()->set_isactive(data_isActive);

			// Add ID and return
			tracker_base.set_tracker(base_tracker);
			return tracker_base;
		}

		// Position filter update option
		RotationTrackingFilterOption orientationTrackingFilterOption = k2_OrientationTrackingFilter_SLERP;
		JointRotationTrackingOption orientationTrackingOption = k2_DeviceInferredRotation;

		// Position and orientation option
		JointPositionTrackingOption positionTrackingFilterOption = k2_PositionTrackingFilter_LERP;

		// Internal data offset
		Eigen::Vector3d positionOffset = Eigen::Vector3d(0, 0, 0),
		                orientationOffset = Eigen::Vector3d(0, 0, 0);

		// If using JointsBasis, the assigned host joint
		uint32_t selectedTrackedJointID = 0;

		// Is this joint overridden?
		bool isPositionOverridden = false,
		     isRotationOverridden = false;

		// If the joint is overridden, overrides' ids
		uint32_t positionOverrideJointID = 0,
		         rotationOverrideJointID = 0;

		// Tracker data (inherited)
		std::string data_serial;
		ktvr::ITrackerType data_role = ktvr::ITrackerType::Tracker_Handed;
		ktvr::ITrackerType base_tracker = ktvr::ITrackerType::Tracker_Handed;
		bool data_isActive = false;

		// Tracker pose (inherited)
		Eigen::Vector3f pose_position{0, 0, 0};
		Eigen::Quaternionf pose_orientation{1, 0, 0, 0};

		template <class Archive>
		void serialize(Archive& archive)
		{
			archive(
				CEREAL_NVP(selectedTrackedJointID),
				CEREAL_NVP(isPositionOverridden),
				CEREAL_NVP(isRotationOverridden),
				CEREAL_NVP(positionOverrideJointID),
				CEREAL_NVP(rotationOverrideJointID),
				CEREAL_NVP(pose_position),
				CEREAL_NVP(pose_orientation),
				CEREAL_NVP(data_serial),
				CEREAL_NVP(data_role),
				CEREAL_NVP(data_isActive),
				CEREAL_NVP(base_tracker),
				CEREAL_NVP(orientationTrackingOption),
				CEREAL_NVP(orientationTrackingFilterOption),
				CEREAL_NVP(positionTrackingFilterOption),
				CEREAL_NVP(positionOffset),
				CEREAL_NVP(orientationOffset)
			);
		}

		// Internal filters' datas
		Eigen::Vector3f kalmanPosition = Eigen::Vector3f(0, 0, 0),
		                lowPassPosition = Eigen::Vector3f(0, 0, 0),
		                LERPPosition = Eigen::Vector3f(0, 0, 0);

		Eigen::Quaternionf SLERPOrientation = Eigen::Quaternionf(1, 0, 0, 0),
		                   SLERPSlowOrientation = Eigen::Quaternionf(1, 0, 0, 0);

		// LERP data's backup
		Eigen::Vector3f lastLERPPosition = Eigen::Vector3f(0, 0, 0);
		Eigen::Quaternionf lastSLERPOrientation = Eigen::Quaternionf(1, 0, 0, 0),
		                   lastSLERPSlowOrientation = Eigen::Quaternionf(1, 0, 0, 0);

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

		// LERP filter const
		double _lerp_const = 0.31;
	};
}
