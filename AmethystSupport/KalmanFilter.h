#pragma once
#include <Eigen/Dense>

class KalmanFilter
{
public:
	/**
	* Create a Kalman filter with the specified matrices.
	*   A - System dynamics matrix
	*   B - Input matrix
	*   C - Output matrix
	*   Q - Process noise covariance
	*   R - Measurement noise covariance
	*   P - Estimate error covariance
	*/

	KalmanFilter()
	{
		constexpr int _n = 3; // Number of states
		constexpr int _m = 1; // Number of measurements
		constexpr int _c = 1; // Number of control inputs

		Eigen::MatrixXd _A(_n, _n); // System dynamics matrix
		Eigen::MatrixXd _B(_n, _c); // Input control matrix
		Eigen::MatrixXd _C(_m, _n); // Output matrix
		Eigen::MatrixXd _Q(_n, _n); // Process noise covariance
		Eigen::MatrixXd _R(_m, _m); // Measurement noise covariance
		Eigen::MatrixXd _P(_n, _n); // Estimate error covariance

		double dt = 1.0 / 100;
		// Discrete LTI projectile motion, measuring position only
		_A << 1, dt, 0, 0, 1, dt, 0, 0, 1;
		_B << 0, 0, 0;
		_C << 1, 0, 0;

		// Reasonable covariance matrices
		_Q << .17, .17, .0, .17, .17, .0, .0, .0, .0;
		_R << 5;
		_P << .3, .3, .3, .3, 30000, 30, .3, 30, 300;

		A = _A;
		B = _B;
		C = _C;
		Q = _Q;
		R = _R;

		P0 = _P;

		m = _C.rows();
		n = _A.rows();
		c = _B.cols();

		I = Eigen::MatrixXd(n, n);
		x_hat = Eigen::VectorXd(n);

		initialized = false;
		I.setIdentity();
	}

	/**
	* Initialize the filter with initial states as zero.
	*/
	void init()
	{
		x_hat.setZero();
		P = P0;
		initialized = true;
	}

	/**
	* Initialize the filter with a guess for initial states.
	*/
	void init(const Eigen::VectorXd& x0)
	{
		x_hat = x0;
		P = P0;
		initialized = true;
	}

	/**
	* Update the prediction based on control input.
	*/
	void predict(const Eigen::VectorXd& u)
	{
		if (!initialized)
			init();

		x_hat = A * x_hat + B * u;
		P = A * P * A.transpose() + Q;
	}

	/**
	* Update the estimated state based on measured values.
	*/
	void update(const Eigen::VectorXd& y)
	{
		K = P * C.transpose() * (C * P * C.transpose() + R).inverse();
		x_hat += K * (y - C * x_hat);
		P = (I - K * C) * P;
	}

	/**
	* Update the dynamics matrix.
	*/
	void update_dynamics(const Eigen::MatrixXd A)
	{
		this->A = A;
	}

	/**
	* Update the output matrix.
	*/
	void update_output(const Eigen::MatrixXd C)
	{
		this->C = C;
	}

	/**
	* Return the current state.
	*/
	Eigen::VectorXd state() { return x_hat; };

private:
	// Matrices for computation
	Eigen::MatrixXd A, B, C, Q, R, P, K, P0;

	// System dimensions
	int m, n, c;

	// Is the filter initialized?
	bool initialized = false;

	// n-size identity
	Eigen::MatrixXd I;

	// Estimated states
	Eigen::VectorXd x_hat;
};
