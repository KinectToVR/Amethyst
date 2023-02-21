#pragma once
#include "Support.g.h"
#include "KalmanFilter.g.h"
#include "KalmanFilter.h"
#include "LowPassFilter.g.h"

namespace winrt::AmethystSupport::implementation
{
    struct Support : SupportT<Support>
    {
        Support() = default;

        static STransform SVD(array_view<const SVector3> head_positions, array_view<const SVector3> hmd_positions);
        static SQuaternion FixFlippedOrientation(const SQuaternion& base);
        static float OrientationDot(const SQuaternion& from, const SQuaternion& to);
        static float QuaternionYaw(const SQuaternion& q);
        static SQuaternion FeetSoftwareOrientation(const SVector3& ankle, const SVector3& foot, const SVector3& knee);
        static SQuaternion FeetSoftwareOrientationV2(const SVector3& ankle, const SVector3& knee);
    };

    struct LowPassFilter : LowPassFilterT<LowPassFilter>
    {
        LowPassFilter() = default;

        LowPassFilter(float cutOff, float deltaTime);
        SVector3 Update(const SVector3& input);

    private:
        SVector3 output_{0, 0, 0};
        float e_pow_; // Init-only
    };

    struct KalmanFilter : KalmanFilterT<KalmanFilter>
    {
        KalmanFilter();

        SVector3 Update(const SVector3& input);

    private:
        std::array<CKalmanFilter, 3> filters_;
    };
}

namespace winrt::AmethystSupport::factory_implementation
{
    struct Support : SupportT<Support, implementation::Support>
    {
    };

    struct LowPassFilter : LowPassFilterT<LowPassFilter, implementation::LowPassFilter>
    {
    };

    struct KalmanFilter : KalmanFilterT<KalmanFilter, implementation::KalmanFilter>
    {
    };
}
