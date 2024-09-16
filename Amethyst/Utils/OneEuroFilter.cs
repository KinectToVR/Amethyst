using System;
using System.Numerics;
using System.Runtime.CompilerServices;

namespace Amethyst.Utils;

public class OneEuroFilter
{
    private float _beta;
    private float _dcutoff;
    private readonly LowPassFilter _dx;
    private float _freq;
    private float _lasttime;
    private float _mincutoff;
    private readonly LowPassFilter _x;

    public OneEuroFilter(float freq, float mincutoff = 1.0f, float beta = 0.0f, float dcutoff = 1.0f)
    {
        SetFrequency(freq);
        SetMinCutoff(mincutoff);
        SetBeta(beta);
        SetDerivateCutoff(dcutoff);
        _x = new LowPassFilter(Alpha(_mincutoff));
        _dx = new LowPassFilter(Alpha(_dcutoff));
        _lasttime = -1.0f;

        CurrValue = 0.0f;
        PrevValue = CurrValue;
    }

    // currValue contains the latest value which have been succesfully filtered
    // prevValue contains the previous filtered value
    public float CurrValue { get; protected set; }
    public float PrevValue { get; protected set; }

    private float Alpha(float cutoff)
    {
        var te = 1.0f / _freq;
        var tau = 1.0f / (2.0f * MathF.PI * cutoff);
        return 1.0f / (1.0f + tau / te);
    }

    private void SetFrequency(float f)
    {
        if (f <= 0.0f)
        {
            Logger.Error("Freq should be > 0");
            return;
        }

        _freq = f;
    }

    private void SetMinCutoff(float mc)
    {
        if (mc <= 0.0f)
        {
            Logger.Error("Mincutoff should be > 0");
            return;
        }

        _mincutoff = mc;
    }

    private void SetBeta(float b)
    {
        _beta = b;
    }

    private void SetDerivateCutoff(float dc)
    {
        if (dc <= 0.0f)
        {
            Logger.Error("Dcutoff should be > 0");
            return;
        }

        _dcutoff = dc;
    }

    public void UpdateParams(float freq, float mincutoff = 1.0f, float beta = 0.0f, float dcutoff = 1.0f)
    {
        SetFrequency(freq);
        SetMinCutoff(mincutoff);
        SetBeta(beta);
        SetDerivateCutoff(dcutoff);
        _x.SetAlpha(Alpha(_mincutoff));
        _dx.SetAlpha(Alpha(_dcutoff));
    }

    public float Filter(float value, float timestamp = -1.0f)
    {
        PrevValue = CurrValue;

        // update the sampling frequency based on timestamps
        if (_lasttime != -1.0f && timestamp != -1.0f)
            _freq = 1.0f / (timestamp - _lasttime);
        _lasttime = timestamp;
        // estimate the current variation per second 
        var dvalue = _x.HasLastRawValue() ? (value - _x.LastRawValue()) * _freq : 0.0f; // FIXME: 0.0 or value? 
        var edvalue = _dx.FilterWithAlpha(dvalue, Alpha(_dcutoff));
        // use it to update the cutoff frequency
        var cutoff = _mincutoff + _beta * MathF.Abs(edvalue);
        // filter the given value
        CurrValue = _x.FilterWithAlpha(value, Alpha(cutoff));

        return CurrValue;
    }

    internal class LowPassFilter
    {
        private bool _initialized;
        private float _y, _a, _s;

        public LowPassFilter(float alpha, float initval = 0.0f)
        {
            _y = _s = initval;
            SetAlpha(alpha);
            _initialized = false;
        }

        public void SetAlpha(float alpha)
        {
            if (alpha is <= 0.0f or > 1.0f)
            {
                Logger.Error("Alpha should be in (0.0., 1.0]");
                return;
            }

            _a = alpha;
        }

        public float Filter(float value)
        {
            float result;
            if (_initialized)
            {
                result = _a * value + (1.0f - _a) * _s;
            }
            else
            {
                result = value;
                _initialized = true;
            }

            _y = value;
            _s = result;
            return result;
        }

        public float FilterWithAlpha(float value, float alpha)
        {
            SetAlpha(alpha);
            return Filter(value);
        }

        public bool HasLastRawValue()
        {
            return _initialized;
        }

        public float LastRawValue()
        {
            return _y;
        }
    }
}

// this class instantiates an array of OneEuroFilter objects to filter each component of Vector2, Vector3, Vector4 or Quaternion types
public class OneEuroFilter<T> where T : struct
{
    // the array of filters
    private readonly OneEuroFilter[] _oneEuroFilters;

    // containst the type of T
    private readonly Type _type;

    // initialization of our filter(s)
    public OneEuroFilter(float freq, float mincutoff = 1.0f, float beta = 0.0f, float dcutoff = 1.0f)
    {
        _type = typeof(T);
        CurrValue = new T();
        PrevValue = new T();

        Freq = freq;
        Mincutoff = mincutoff;
        Beta = beta;
        Dcutoff = dcutoff;

        if (_type == typeof(Vector2))
        {
            _oneEuroFilters = new OneEuroFilter[2];
        }

        else if (_type == typeof(Vector3))
        {
            _oneEuroFilters = new OneEuroFilter[3];
        }

        else if (_type == typeof(Vector4) || _type == typeof(Quaternion))
        {
            _oneEuroFilters = new OneEuroFilter[4];
        }
        else
        {
            Logger.Error(_type + " is not a supported type");
            return;
        }

        for (var i = 0; i < _oneEuroFilters.Length; i++)
            _oneEuroFilters[i] = new OneEuroFilter(Freq, Mincutoff, Beta, Dcutoff);
    }

    // filter parameters
    public float Freq { get; protected set; }
    public float Mincutoff { get; protected set; }
    public float Beta { get; protected set; }
    public float Dcutoff { get; protected set; }

    // currValue contains the latest value which have been succesfully filtered
    // prevValue contains the previous filtered value
    public T CurrValue { get; protected set; }
    public T PrevValue { get; protected set; }

    // updates the filter parameters
    public void UpdateParams(float freq, float mincutoff = 1.0f, float beta = 0.0f, float dcutoff = 1.0f)
    {
        Freq = freq;
        Mincutoff = mincutoff;
        Beta = beta;
        Dcutoff = dcutoff;

        foreach (var t in _oneEuroFilters)
            t.UpdateParams(Freq, Mincutoff, Beta, Dcutoff);
    }


    // filters the provided _value and returns the result.
    // Note: a timestamp can also be provided - will override filter frequency.
    public T Filter<TU>(TU value, float timestamp = -1.0f) where TU : struct
    {
        PrevValue = CurrValue;

        if (typeof(TU) != _type)
        {
            Logger.Error("WARNING! " + typeof(TU) + " when " + _type + " is expected!\nReturning previous filtered value");
            CurrValue = PrevValue;

            return (T)Convert.ChangeType(CurrValue, typeof(T));
        }

        if (_type == typeof(Vector2))
        {
            var output = Vector2.Zero;
            var input = (Vector2)Convert.ChangeType(value, typeof(Vector2));

            for (var i = 0; i < _oneEuroFilters.Length; i++)
                output[i] = _oneEuroFilters[i].Filter(input[i], timestamp);

            CurrValue = (T)Convert.ChangeType(output, typeof(T));
        }

        else if (_type == typeof(Vector3))
        {
            var output = Vector3.Zero;
            var input = (Vector3)Convert.ChangeType(value, typeof(Vector3));

            for (var i = 0; i < _oneEuroFilters.Length; i++)
                output[i] = _oneEuroFilters[i].Filter(input[i], timestamp);

            CurrValue = (T)Convert.ChangeType(output, typeof(T));
        }

        else if (_type == typeof(Vector4))
        {
            var output = Vector4.Zero;
            var input = (Vector4)Convert.ChangeType(value, typeof(Vector4));

            for (var i = 0; i < _oneEuroFilters.Length; i++)
                output[i] = _oneEuroFilters[i].Filter(input[i], timestamp);

            CurrValue = (T)Convert.ChangeType(output, typeof(T));
        }

        else
        {
            var output = Quaternion.Identity;
            var input = (Quaternion)Convert.ChangeType(value, typeof(Quaternion));

            // Workaround that take into account that some input device sends
            // quaternion that represent only a half of all possible values.
            // this piece of code does not affect normal behaviour (when the
            // input use the full range of possible values).
            if (Extensions.SqrMagnitude(
                    new Vector4(_oneEuroFilters[0].CurrValue, _oneEuroFilters[1].CurrValue, _oneEuroFilters[2].CurrValue, _oneEuroFilters[3].CurrValue)
                        .Normalized()
                    - new Vector4(input[0], input[1], input[2], input[3]).Normalized()) > 2)
                input = new Quaternion(-input.X, -input.Y, -input.Z, -input.W);

            for (var i = 0; i < _oneEuroFilters.Length; i++)
                output[i] = _oneEuroFilters[i].Filter(input[i], timestamp);

            CurrValue = (T)Convert.ChangeType(output, typeof(T));
        }

        return (T)Convert.ChangeType(CurrValue, typeof(T));
    }
}

public static class Extensions
{
    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float SqrMagnitude(Vector4 v)
    {
        return Dot(v, v);
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Dot(Vector4 a, Vector4 b)
    {
        return a.X * b.X + a.Y * b.Y + a.Z * b.Z + a.W * b.W;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static float Magnitude(Vector4 a)
    {
        return (float)Math.Sqrt(Dot(a, a));
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 Normalize(Vector4 a)
    {
        var mag = Magnitude(a);
        return mag > 0.00001F ? a / mag : Vector4.Zero;
    }

    [MethodImpl(MethodImplOptions.AggressiveInlining)]
    public static Vector4 Normalized(this Vector4 v)
    {
        return Vector4.Normalize(v);
    }
}