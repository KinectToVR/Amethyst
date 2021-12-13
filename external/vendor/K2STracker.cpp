#include "K2STracker.h"

template <typename _Scalar>
Eigen::Vector3<_Scalar> lerp(const Eigen::Vector3<_Scalar>& to, const Eigen::Vector3<_Scalar>& from, float t) {
	return from * t + to * ((_Scalar)1.f - t);
}

void K2STracker::updatePositionFilters()
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
	LERPPosition = lerp(lastLERPPosition, pose.position, 0.15);
	lastLERPPosition = LERPPosition; // Backup the position
}

void K2STracker::updateOrientationFilters()
{
	/* Update the SLERP filter */
	SLERPOrientation = lastSLERPOrientation.slerp(0.6, pose.orientation);
	lastSLERPOrientation = pose.orientation; // Backup the orientation

	/* Update the Slower SLERP filter */
	SLERPSlowOrientation = lastSLERPSlowOrientation.slerp(0.3, pose.orientation);
	lastSLERPSlowOrientation = pose.orientation; // Backup the orientation
}

void K2STracker::initAllFilters()
{
	/* Position filters */
	for (auto& filter : kalmanFilter)
		filter.init();
}
