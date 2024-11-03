using System;
using System.Numerics;
using Windows.UI;
using Microsoft.UI.Xaml.Media;

namespace Amethyst.Utils;

public class JointStabilityDetector
{
    public Vector3? Target { get; set; }
    private Vector3 Value { get; set; }
    private DateTime LastUpdate { get; set; } = DateTime.MinValue;
    private double Stability { get; set; } = -0.2;

    public double Update(Vector3 value)
    {
        var length = (Value - value).Length();
        var timeDiff = DateTime.Now - LastUpdate;

        var isStable = (length is not 0.0f ? length / timeDiff.TotalSeconds : 0.0) < 0.3;
        var inRange = Target is null || (Target.Value - value).Length() > 0.5;
        var stabilityChange = (Target is null ? isStable : inRange) ? 0.05 : -0.015;

        if (Target is null && LastUpdate != DateTime.MinValue)
            Stability += stabilityChange * (timeDiff.TotalMilliseconds / 100.0);
        else if (Target is not null)
            Stability = Math.Clamp((Target.Value - value).Length(), 0.0f, 0.6f) * 1.7f;

        Stability = Math.Clamp(Stability, -0.2, 1.0);
        Value = value;
        LastUpdate = DateTime.Now;

        return Math.Clamp(Stability, 0.0, 1.0);
    }
}

public static class MappingExtensions
{
    public static double Map(this double value, double fromSource, double toSource, double fromTarget, double toTarget)
    {
        return Math.Clamp((value - fromSource) / (toSource - fromSource) * (toTarget - fromTarget) + fromTarget, fromTarget, toTarget);
    }

    public static SolidColorBrush Blend(this SolidColorBrush color, SolidColorBrush accent, double ratio)
    {
        ratio = Math.Clamp(ratio.Map(0, 0.8, 0, 1), 0, 1); // Sanity check
        return new SolidColorBrush(Color.FromArgb(255,
            (byte)(color.Color.R * (1.0 - ratio) + accent.Color.R * ratio),
            (byte)(color.Color.G * (1.0 - ratio) + accent.Color.G * ratio),
            (byte)(color.Color.B * (1.0 - ratio) + accent.Color.B * ratio)));
    }
}