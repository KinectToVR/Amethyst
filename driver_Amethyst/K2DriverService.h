#pragma once
#include "pch.h"
#include "K2Tracker.h"

class K2DriverService final : public ktvr::IK2DriverService::Service
{
public:
	// Public server handler functions
	grpc::Status SetTrackerStateVector(
		grpc::ServerContext* context, grpc::ServerReaderWriter<
			ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream) override;

	grpc::Status UpdateTrackerVector(
		grpc::ServerContext* context, grpc::ServerReaderWriter<
			ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream) override;

	grpc::Status RefreshTrackerPoseVector(
		grpc::ServerContext* context, grpc::ServerReaderWriter<
			ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream) override;

	grpc::Status RequestVRRestart(
		grpc::ServerContext* context, const ktvr::ServiceRequest* request,
		ktvr::Service_TrackerStatePair* response) override;

	grpc::Status PingDriverService(
		grpc::ServerContext* context, const google::protobuf::Empty* request,
		ktvr::PingRequest* response) override;

	void TrackerVector(std::vector<K2Tracker> &vector)
	{
		ptr_tracker_vector_ = std::make_shared<std::vector<K2Tracker>>(vector);
	}
	
private:
	// Tracker vector
	std::shared_ptr<std::vector<K2Tracker>> ptr_tracker_vector_;
};
