#include "pch.h"

#include "K2DriverService.h"

grpc::Status K2DriverService::set_tracker_state_vector(
	grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
	ktvr::K2ResponseMessage* reply, const bool& want_reply) const
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming state sets
	while (reader->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerstatetuple())
		{
			LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
			if (want_reply) reply->set_success(false);
			continue; // Process the next message (...or at least try to)
		}

		// Check the state
		const auto ptr_tracker = &ptr_tracker_vector_->at(request.trackerstatetuple().trackertype());
		if (!ptr_tracker->is_added() && !ptr_tracker->spawn())
		{
			// Spawn if needed
			LOG(INFO) << "Tracker autospawn exception! Serial: " << ptr_tracker->get_serial();

			// Compose the reply (if applies)
			if (want_reply)
			{
				reply->set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_SpawnFailed);
				reply->set_success(false); // Oof we didn't make it
			}
			continue; // Give up for now, the tracker has failed to spawn (or add to OpenVR...)
		}

		// Set the state
		ptr_tracker->set_state(request.trackerstatetuple().state());
		LOG(INFO) << "Tracker role: " << request.trackerstatetuple().trackertype() <<
			" state has been set to: " << std::to_string(request.trackerstatetuple().state());

		// Update the tracker
		ptr_tracker->update();

		// Compose the reply (if applies)
		if (want_reply)
		{
			reply->set_tracker(request.trackerstatetuple().trackertype());
			reply->set_success(true); // Winning it, yay!
		}
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::update_tracker_vector(
	grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
	ktvr::K2ResponseMessage* reply, const bool& want_reply) const
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming pose sets
	while (reader->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerbase())
		{
			LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
			if (want_reply) reply->set_success(false);
			continue; // Process the next message (...or at least try to)
		}

		// Update pose
		ptr_tracker_vector_->at(request.trackerbase().tracker())
		                   .set_pose(request.trackerbase().pose());

		// Compose the reply (if applies)
		if (want_reply)
		{
			reply->set_tracker(request.trackerstatetuple().trackertype());
			reply->set_success(true); // Winning it, yay!
		}
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::refresh_tracker_pose_vector(
	grpc::ServerContext* context, grpc::ServerReader<ktvr::ServiceRequest>* reader,
	ktvr::K2ResponseMessage* reply, const bool& want_reply) const
{
	// Create a stub message object
	ktvr::ServiceRequest request;

	// Parse all incoming refresh requests
	while (reader->Read(&request))
	{
		// Sanity check
		if (!request.has_trackerstatetuple())
		{
			LOG(ERROR) << "Couldn't refresh multiple trackers, bases are empty.";
			if (want_reply) reply->set_success(false);
			continue; // Process the next message (...or at least try to)
		}

		// Call the VR update handler
		ptr_tracker_vector_->at(request.trackerbase().tracker()).update();

		// Compose the reply (if applies)
		if (want_reply)
		{
			reply->set_tracker(request.trackerstatetuple().trackertype());
			reply->set_success(true); // Winning it, yay!
		}
	}

	return grpc::Status::OK;
}

grpc::Status K2DriverService::request_vr_restart(
	grpc::ServerContext* context, const ktvr::ServiceRequest* request,
	ktvr::K2ResponseMessage* reply, const bool& want_reply) const
{
	// Sanity check
	if (!request->has_message() || request->message().empty())
	{
		LOG(ERROR) << "Couldn't request a reboot. The reason string is empty.";
		if (want_reply) reply->set_success(false);
		return grpc::Status::OK;
	}

	// Perform the request
	LOG(INFO) << "Requesting OpenVR restart with reason: " << request->message();
	vr::VRServerDriverHost()->RequestRestart(
		request->message().c_str(),
		"vrstartup.exe", "", "");

	// Compose the reply (if applies)
	if (want_reply) reply->set_success(true);
	return grpc::Status::OK; // Winning it, yay!
}
