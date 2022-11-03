#include "pch.h"
#include "Amethyst_API.h"

#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>

void replace_all(std::string& str, const std::string& from, const std::string& to)
{
	if (from.empty())
		return;
	size_t start_pos = 0;
	while ((start_pos = str.find(from, start_pos)) != std::string::npos)
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length();
	}
}

namespace ktvr
{
	// 0: OK, -1: Exception
	// -2: Channel fail
	// -3: Stub failure
	int init_ame_api(const int& port) noexcept
	{
		try
		{
			// Compose the channel arguments
			grpc::ChannelArguments channel_arguments;
			channel_arguments.SetInt(GRPC_ARG_KEEPALIVE_TIME_MS, 7200000);
			channel_arguments.SetInt(GRPC_ARG_KEEPALIVE_TIMEOUT_MS, 20000);
			channel_arguments.SetInt(GRPC_ARG_KEEPALIVE_PERMIT_WITHOUT_CALLS, 1);
			channel_arguments.SetInt(GRPC_ARG_HTTP2_MIN_RECV_PING_INTERVAL_WITHOUT_DATA_MS, 300000);
			channel_arguments.SetInt(GRPC_ARG_HTTP2_MIN_SENT_PING_INTERVAL_WITHOUT_DATA_MS, 10000);

			// Create the RPC channel
			if (channel = CreateCustomChannel(
					std::format("localhost:{}", port),
					grpc::InsecureChannelCredentials(), channel_arguments);
				channel == nullptr)
				return -2;

			// Create the RPC messaging stub
			if (stub = IK2DriverService::NewStub(channel);
				stub == nullptr)
				return -3;
		}
		catch (const std::exception& e)
		{
			return -1;
		}
		return 0;
	}

	std::vector<std::pair<ITrackerType, bool>>
	update_tracker_state_vector_r(const std::vector<std::pair<ITrackerType, bool>>& status_pairs) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->SetTrackerStateVector(&context);

			for (const auto& [tracker, status] : status_pairs)
			{
				auto request = ServiceRequest();
				request.mutable_trackerstatetuple()->set_trackertype(tracker);
				request.mutable_trackerstatetuple()->set_state(status);
				request.set_want_reply(true); // We WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "SetTrackerStateVector failed!";

			return response;
		}
		catch (const std::exception& e)
		{
			return {{Tracker_Handed, false}};
		}
	}

	std::monostate
	update_tracker_state_vector_n(const std::vector<std::pair<ITrackerType, bool>>& status_pairs) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->SetTrackerStateVector(&context);

			for (const auto& [tracker, status] : status_pairs)
			{
				auto request = ServiceRequest();
				request.mutable_trackerstatetuple()->set_trackertype(tracker);
				request.mutable_trackerstatetuple()->set_state(status);
				request.set_want_reply(false); // We DO NOT WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "SetTrackerStateVector failed!";

			return {};
		}
		catch (const std::exception& e)
		{
			return {};
		}
	}

	std::vector<std::pair<ITrackerType, bool>>
	update_tracker_vector_r(const std::vector<K2TrackerBase>& tracker_bases) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->UpdateTrackerVector(&context);

			for (const auto& tracker : tracker_bases)
			{
				auto request = ServiceRequest();
				request.mutable_trackerbase()->CopyFrom(tracker);
				request.set_want_reply(true); // We WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "UpdateTrackerVector failed!";

			return response;
		}
		catch (const std::exception& e)
		{
			return {{Tracker_Handed, false}};
		}
	}

	std::monostate
	update_tracker_vector_n(const std::vector<K2TrackerBase>& tracker_bases) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->UpdateTrackerVector(&context);

			for (const auto& tracker : tracker_bases)
			{
				auto request = ServiceRequest();
				request.mutable_trackerbase()->CopyFrom(tracker);
				request.set_want_reply(false); // We DO NOT WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "UpdateTrackerVector failed!";

			return {};
		}
		catch (const std::exception& e)
		{
			return {};
		}
	}

	std::vector<std::pair<ITrackerType, bool>>
	refresh_tracker_pose_vector_r(const std::vector<ITrackerType>& trackers) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->RefreshTrackerPoseVector(&context);

			for (const auto& tracker : trackers)
			{
				auto request = ServiceRequest();
				request.mutable_trackerstatetuple()->set_trackertype(tracker);
				request.set_want_reply(true); // We WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "RefreshTrackerPoseVector failed!";

			return response;
		}
		catch (const std::exception& e)
		{
			return {{Tracker_Handed, false}};
		}
	}

	std::monostate
	refresh_tracker_pose_vector_n(const std::vector<ITrackerType>& trackers) noexcept
	{
		try
		{
			grpc::ClientContext context;
			const auto stream =
				stub->RefreshTrackerPoseVector(&context);

			for (const auto& tracker : trackers)
			{
				auto request = ServiceRequest();
				request.mutable_trackerstatetuple()->set_trackertype(tracker);
				request.set_want_reply(false); // We DO NOT WANT a reply

				// Give up on any failures/breaks
				if (!stream->Write(request)) break;
			}
			stream->WritesDone(); // Mark as done

			Service_TrackerStatePair reply;
			std::vector<std::pair<ITrackerType, bool>> response;
			while (stream->Read(&reply)) // Collect all replies
				response.emplace_back(reply.trackertype(), reply.state());

			if (!stream->Finish().ok())
				LOG(INFO) << "RefreshTrackerPoseVector failed!";

			return {};
		}
		catch (const std::exception& e)
		{
			return {};
		}
	}

	std::pair<ITrackerType, bool>
	request_vr_restart_r(const std::string& reason) noexcept
	{
		try
		{
			grpc::ClientContext context;
			Service_TrackerStatePair reply;

			auto request = ServiceRequest();
			request.set_message(reason);
			request.set_want_reply(true); // We WANT a reply

			if (!stub->RequestVRRestart(&context, request, &reply).ok())
				LOG(INFO) << "RequestVRRestart failed!";

			return {reply.trackertype(), reply.state()};
		}
		catch (const std::exception& e)
		{
			return {Tracker_Handed, false};
		}
	}

	std::monostate
	request_vr_restart_n(const std::string& reason) noexcept
	{
		try
		{
			grpc::ClientContext context;
			Service_TrackerStatePair reply;

			auto request = ServiceRequest();
			request.set_message(reason);
			request.set_want_reply(false); // We DO NOT WANT a reply

			if (!stub->RequestVRRestart(&context, request, &reply).ok())
				LOG(INFO) << "RequestVRRestart failed!";

			return {};
		}
		catch (const std::exception& e)
		{
			return {};
		}
	}

	std::tuple<bool, long long, long long, long long> test_connection() noexcept
	{
		try
		{
			grpc::ClientContext context;
			PingRequest reply;

			// Grab the current time and send the message
			const long long send_time = AME_API_GET_TIMESTAMP_NOW;
			const bool success = stub->PingDriverService(
				&context, google::protobuf::Empty(), &reply).ok();

			// Return tuple with response and elapsed time
			const auto elapsed_time = // Always >= 0
				std::clamp(AME_API_GET_TIMESTAMP_NOW - send_time,
				           static_cast<long long>(0), LLONG_MAX);

			// If failed for some reason
			if (!success)
			{
				LOG(INFO) << "PingDriverService failed!";
				return {false, send_time, reply.received_timestamp(), elapsed_time};
			}

			return {true, send_time, reply.received_timestamp(), elapsed_time};
		}
		catch (const std::exception& e)
		{
			return {false, static_cast<long long>(0), static_cast<long long>(0), static_cast<long long>(0)};
		}
	}
}
