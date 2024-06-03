using System;
using System.Numerics;
using MathNet.Numerics.LinearAlgebra;

namespace Amethyst.Utils;

public class KalmanFilter
{
    private Vector3 _position;

    private SingularKalmanFilter[] Filters { get; } = new[]
        { new SingularKalmanFilter(), new SingularKalmanFilter(), new SingularKalmanFilter() };

    public Vector3 Update(Vector3 input)
    {
        _position.X = Filters[0].Update(input.X);
        _position.Y = Filters[1].Update(input.Y);
        _position.Z = Filters[2].Update(input.Z);
        return _position; // Don't create 'new' vectors
    }
}

internal class SingularKalmanFilter
{
    private int L, m;
    private float alpha, ki, beta, lambda, c, q, r;
    private Matrix<float> Wm, Wc, x, P, Q, R;

    /// <summary>
    /// Constructor of Unscented Kalman Filter
    /// </summary>
    /// <param name="L">States number</param>
    /// <param name="m">Measurements number</param>
    public SingularKalmanFilter(int L = 0)
    {
        this.L = L;
    }

    private void Init()
    {
        q = 0.05f;
        r = 0.3f;

        x = q * Matrix<float>.Build.Random(L, 1); //initial state with noise
        P = Matrix<float>.Build.Diagonal(L, L, 1); //initial state covraiance

        Q = Matrix<float>.Build.Diagonal(L, L, q * q); //covariance of process
        R = Matrix<float>.Build.Dense(m, m, r * r); //covariance of measurement  

        alpha = 1e-3f;
        ki = 0;
        beta = 2f;
        lambda = alpha * alpha * (L + ki) - L;
        c = L + lambda;

        //weights for means
        Wm = Matrix<float>.Build.Dense(1, 2 * L + 1, 0.5f / c);
        Wm[0, 0] = lambda / c;

        //weights for covariance
        Wc = Matrix<float>.Build.Dense(1, 2 * L + 1);
        Wm.CopyTo(Wc);
        Wc[0, 0] = Wm[0, 0] + 1 - alpha * alpha + beta;

        c = MathF.Sqrt(c);
    }

    public float Update(float measurement)
    {
        if (m == 0)
        {
            m = 1;
            if (L == 0) L = 1;
            Init();
        }

        var z = Matrix<float>.Build.Dense(m, 1, 0);
        z[0, 0] = measurement;

        //sigma points around x
        var X = GetSigmaPoints(x, P, c);

        //unscented transformation of process
        // X1=sigmas(x1,P1,c) - sigma points around x1
        //X2=X1-x1(:,ones(1,size(X1,2))) - deviation of X1
        var ut_f_matrices = UnscentedTransform(X, Wm, Wc, L, Q);
        var x1 = ut_f_matrices[0];
        var X1 = ut_f_matrices[1];
        var P1 = ut_f_matrices[2];
        var X2 = ut_f_matrices[3];

        //unscented transformation of measurments
        var ut_h_matrices = UnscentedTransform(X1, Wm, Wc, m, R);
        var z1 = ut_h_matrices[0];
        var Z1 = ut_h_matrices[1];
        var P2 = ut_h_matrices[2];
        var Z2 = ut_h_matrices[3];

        //transformed cross-covariance
        var P12 = X2.Multiply(Matrix<float>.Build.Diagonal(Wc.Row(0).ToArray())).Multiply(Z2.Transpose());

        var K = P12.Multiply(P2.Inverse());

        //state update
        x = x1.Add(K.Multiply(z.Subtract(z1)));
        //covariance update 
        P = P1.Subtract(K.Multiply(P12.Transpose()));
        return GetState()[0];
    }

    public float[] GetState()
    {
        return x.ToColumnArrays()[0];
    }

    public float[,] GetCovariance()
    {
        return P.ToArray();
    }

    /// <summary>
    /// Unscented Transformation
    /// </summary>
    /// <param name="f">nonlinear map</param>
    /// <param name="X">sigma points</param>
    /// <param name="Wm">Weights for means</param>
    /// <param name="Wc">Weights for covariance</param>
    /// <param name="n">numer of outputs of f</param>
    /// <param name="R">additive covariance</param>
    /// <returns>[transformed mean, transformed smapling points, transformed covariance, transformed deviations</returns>
    private Matrix<float>[] UnscentedTransform(Matrix<float> X, Matrix<float> Wm, Matrix<float> Wc, int n,
        Matrix<float> R)
    {
        var L = X.ColumnCount;
        var y = Matrix<float>.Build.Dense(n, 1, 0);
        var Y = Matrix<float>.Build.Dense(n, L, 0);

        Matrix<float> row_in_X;
        for (var k = 0; k < L; k++)
        {
            row_in_X = X.SubMatrix(0, X.RowCount, k, 1);
            Y.SetSubMatrix(0, Y.RowCount, k, 1, row_in_X);
            y = y.Add(Y.SubMatrix(0, Y.RowCount, k, 1).Multiply(Wm[0, k]));
        }

        var Y1 = Y.Subtract(y.Multiply(Matrix<float>.Build.Dense(1, L, 1)));
        var P = Y1.Multiply(Matrix<float>.Build.Diagonal(Wc.Row(0).ToArray()));
        P = P.Multiply(Y1.Transpose());
        P = P.Add(R);

        Matrix<float>[] output = { y, Y, P, Y1 };
        return output;
    }

    /// <summary>
    /// Sigma points around reference point
    /// </summary>
    /// <param name="x">reference point</param>
    /// <param name="P">covariance</param>
    /// <param name="c">coefficient</param>
    /// <returns>Sigma points</returns>
    private Matrix<float> GetSigmaPoints(Matrix<float> x, Matrix<float> P, float c)
    {
        var A = P.Cholesky().Factor;

        A = A.Multiply(c);
        A = A.Transpose();

        var n = x.RowCount;

        var Y = Matrix<float>.Build.Dense(n, n, 1);
        for (var j = 0; j < n; j++) Y.SetSubMatrix(0, n, j, 1, x);

        var X = Matrix<float>.Build.Dense(n, 2 * n + 1);
        X.SetSubMatrix(0, n, 0, 1, x);

        var Y_plus_A = Y.Add(A);
        X.SetSubMatrix(0, n, 1, n, Y_plus_A);

        var Y_minus_A = Y.Subtract(A);
        X.SetSubMatrix(0, n, n + 1, n, Y_minus_A);

        return X;
    }
}