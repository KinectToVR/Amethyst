#include "pch.h"

#include "K2DriverService.h"

grpc::Status K2DriverService::SetTrackerStateVector(
	grpc::ServerContext* context, grpc::ServerReaderWriter<
		ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream)
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming state sets
	while (stream->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerstatetuple())
		{
			LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
			if (request.want_reply())
				stream->Write(ktvr::Service_TrackerStatePair(0, false));
			continue; // Process the next message (...or at least try to)
		}

		// Check the state
		const auto ptr_tracker = &ptr_tracker_vector_->at(request.trackerstatetuple().trackertype());
		if (!ptr_tracker->is_added() && !ptr_tracker->spawn())
		{
			// Spawn if needed
			LOG(INFO) << "Tracker autospawn exception! Serial: " << ptr_tracker->get_serial();

			// Compose the reply (if applies)
			if (request.want_reply()) // Oof we didn't make it
				stream->Write(ktvr::Service_TrackerStatePair(
					request.trackerstatetuple().trackertype(), false));

			continue; // Give up for now, the tracker has failed to spawn (or add to OpenVR...)
		}

		// Set the state
		ptr_tracker->set_state(request.trackerstatetuple().state());
		LOG(INFO) << "Tracker role: " << request.trackerstatetuple().trackertype() <<
			" state has been set to: " << std::to_string(request.trackerstatetuple().state());

		// Update the tracker
		ptr_tracker->update();

		// Compose the reply (if applies)
		if (request.want_reply()) // Winning it, yay!
			stream->Write(ktvr::Service_TrackerStatePair(
				request.trackerstatetuple().trackertype(), true));
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::UpdateTrackerVector(
	grpc::ServerContext* context, grpc::ServerReaderWriter<
		ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream)
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming pose sets
	while (stream->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerbase())
		{
			LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
			if (request.want_reply())
				stream->Write(ktvr::Service_TrackerStatePair(0, false));
			continue; // Process the next message (...or at least try to)
		}

		// Update pose
		ptr_tracker_vector_->at(request.trackerbase().tracker())
		                   .set_pose(request.trackerbase().pose());

		// Compose the reply (if applies)
		if (request.want_reply()) // Winning it, yay!
			stream->Write(ktvr::Service_TrackerStatePair(
				request.trackerstatetuple().trackertype(), true));
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::RefreshTrackerPoseVector(
	grpc::ServerContext* context, grpc::ServerReaderWriter<
		ktvr::Service_TrackerStatePair, ktvr::ServiceRequest>* stream)
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming refresh requests
	while (stream->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerstatetuple())
		{
			LOG(ERROR) << "Couldn't refresh multiple trackers, bases are empty.";
			if (request.want_reply())
				stream->Write(ktvr::Service_TrackerStatePair(0, false));
			continue; // Process the next message (...or at least try to)
		}

		// Call the VR update handler
		ptr_tracker_vector_->at(request.trackerbase().tracker()).update();

		// Compose the reply (if applies)
		if (request.want_reply()) // Winning it, yay!
			stream->Write(ktvr::Service_TrackerStatePair(
				request.trackerstatetuple().trackertype(), true));
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::RequestVRRestart(
	grpc::ServerContext* context, const ktvr::ServiceRequest* request,
	ktvr::Service_TrackerStatePair* response)
{
	// Sanity check
	if (!request->has_message() || request->message().empty())
	{
		LOG(ERROR) << "Couldn't request a reboot. The reason string is empty.";
		if (request->want_reply()) response->set_state(false);
		return grpc::Status::OK;
	}

	// Perform the request
	LOG(INFO) << "Requesting OpenVR restart with reason: " << request->message();
	vr::VRServerDriverHost()->RequestRestart(
		request->message().c_str(),
		"vrstartup.exe", "", "");

	// Compose the reply (if applies)
	if (request->want_reply())response->set_state(true);
	return grpc::Status::OK; // Winning it, yay!
}

grpc::Status K2DriverService::PingDriverService(
	grpc::ServerContext* context, const google::protobuf::Empty* request,
	ktvr::PingRequest* response)
{
	// Perform the request
	response->set_received_timestamp(AME_API_GET_TIMESTAMP_NOW);
	return grpc::Status::OK; // Winning it, yay!
}
