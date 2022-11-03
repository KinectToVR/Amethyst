// pch.h: This is a precompiled header file.
// Files listed below are compiled only once, improving build performance for future builds.
// This also affects IntelliSense performance, including code completion and many code browsing features.
// However, files listed here are ALL re-compiled if any one of them is updated between builds.
// Do not add files here that you will be updating frequently as this negates the performance advantage.

#ifndef PCH_H
#define PCH_H

// add headers that you want to pre-compile here
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

#include <Amethyst_API.pb.h>
#include <Amethyst_API.grpc.pb.h>

#define GLOG_NO_ABBREVIATED_SEVERITIES
#include <glog/logging.h>

#endif //PCH_H
