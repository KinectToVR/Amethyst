#pragma once
#include "pch.h"
#include "K2Tracker.h"

class K2DriverService final : public ktvr::IK2DriverService::Service
{
public:
	// Public server handler functions
	grpc::Status SetTrackerStateVector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return set_tracker_state_vector(context, reader, reply, true);
	}

	grpc::Status SetTrackerStateVectorNoReply(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return set_tracker_state_vector(context, reader, reply, false);
	}

	grpc::Status UpdateTrackerVector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return update_tracker_vector(context, reader, reply, true);
	}

	grpc::Status UpdateTrackerVectorNoReply(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return update_tracker_vector(context, reader, reply, false);
	}

	grpc::Status RefreshTrackerPoseVector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return refresh_tracker_pose_vector(context, reader, reply, true);
	}

	grpc::Status RefreshTrackerPoseVectorNoReply(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply) override
	{
		return refresh_tracker_pose_vector(context, reader, reply, false);
	}

	grpc::Status RequestVRRestart(
		grpc::ServerContext* context, const ktvr::ServiceRequest* request,
		ktvr::K2ResponseMessage* reply) override
	{
		return request_vr_restart(context, request, reply, true);
	}

	grpc::Status RequestVRRestartNoReply(
		grpc::ServerContext* context, const ktvr::ServiceRequest* request,
		ktvr::K2ResponseMessage* reply) override
	{
		return request_vr_restart(context, request, reply, false);
	}

private:
	// Tracker vector
	std::shared_ptr<std::vector<K2Tracker>> ptr_tracker_vector_;

	// Member server functions
	grpc::Status set_tracker_state_vector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply, const bool& want_reply) const;

	grpc::Status update_tracker_vector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply, const bool& want_reply) const;

	grpc::Status refresh_tracker_pose_vector(
		grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
		ktvr::K2ResponseMessage* reply, const bool& want_reply) const;

	grpc::Status request_vr_restart(
		grpc::ServerContext* context, const ktvr::ServiceRequest* request,
		ktvr::K2ResponseMessage* reply, const bool& want_reply) const;
};
