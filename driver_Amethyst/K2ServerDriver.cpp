#include "K2ServerDriver.h"
#include <thread>

int K2ServerDriver::init_ServerDriver(
	const std::wstring& ame_api_to_pipe,
	const std::wstring& ame_api_from_pipe,
	const std::wstring& ame_api_to_sem,
	const std::wstring& ame_api_from_sem,
	const std::wstring& ame_api_start_sem)
{
	using namespace std::chrono_literals;
	_isActive = true;

	// initialize the semaphores
	try
	{
		// Create the *to* semaphore
		k2api_to_Semaphore = CreateSemaphoreW(
			nullptr, //Security Attributes
			0, //Initial State i.e.Non Signaled
			1, //No. of Resources
			ame_api_to_sem.c_str()); //Semaphore Name

		if (nullptr == k2api_to_Semaphore)
		{
			LOG(ERROR) << "Semaphore Creation Failed\n";
			LOG(ERROR) << "Error No - " << GetLastError() << '\n';
			return -1;
		}
		LOG(INFO) << "*TO* Semaphore Creation Success\n";

		// Automatically release the sem after creation
		ReleaseSemaphore(k2api_to_Semaphore, 1, nullptr);

		// Create the *from* semaphore
		k2api_from_Semaphore = CreateSemaphoreW(
			nullptr, //Security Attributes
			0, //Initial State i.e.Non Signaled
			1, //No. of Resources
			ame_api_from_sem.c_str()); //Semaphore Name

		if (nullptr == k2api_from_Semaphore)
		{
			LOG(ERROR) << "Semaphore Creation Failed\n";
			LOG(ERROR) << "Error No - " << GetLastError() << '\n';
			return -1;
		}
		LOG(INFO) << "*FROM* Semaphore Creation Success\n";

		// Create the *start* semaphore
		k2api_start_Semaphore = CreateSemaphoreW(
			nullptr, //Security Attributes
			0, //Initial State i.e.Non Signaled
			1, //No. of Resources
			ame_api_start_sem.c_str()); //Semaphore Name

		if (nullptr == k2api_start_Semaphore)
		{
			LOG(ERROR) << "Semaphore Creation Failed\n";
			LOG(ERROR) << "Error No - " << GetLastError() << '\n';
			return -1;
		}
		LOG(INFO) << "*START* Semaphore Creation Success\n";
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << "Setting up server failed: " << e.what();
		return -1;
	}

	// Add 1 tracker for each role
	for (uint32_t role = 0;
	     role <= static_cast<int>(ktvr::ITrackerType::Tracker_Keyboard); role++)
	{
		auto tracker_base = ktvr::K2TrackerBase();
		tracker_base.mutable_data()->set_serial(
			ITrackerType_Role_Serial.at(static_cast<ktvr::ITrackerType>(role)));
		tracker_base.mutable_data()->set_role(static_cast<ktvr::ITrackerType>(role));
		tracker_base.mutable_data()->set_isactive(false); // AutoAdd: false

		trackerVector.push_back(K2Tracker(tracker_base));
	}

	// Log the prepended trackers
	for (auto& _tracker : trackerVector)
		LOG(INFO) << "Registered a tracker: " << _tracker.get_serial();

	std::thread([&, ame_api_to_pipe]
	{
		LOG(INFO) << "Server thread started";

		// Errors' case
		bool server_giveUp = false;
		int server_tries = 0;

		while (!server_giveUp)
		{
			try
			{
				// run until termination
				while (true)
				{
					while (_isActive)
					{
						// Wait for the loop start
						smphFrameUpdate.acquire();

						// Wait for the client to request a read
						while (WaitForSingleObject(k2api_start_Semaphore, 15000L) != WAIT_OBJECT_0)
						{
							LOG(INFO) << "Releasing the *TO* semaphore\n";
							// Release the semaphore in case something hangs,
							// no request would take as long as 15 seconds anyway
							ReleaseSemaphore(k2api_to_Semaphore, 1, nullptr);
						}

						// Here, read from the *TO* pipe
						// Create the pipe file
						std::optional ReaderPipe = CreateFileW(
							ame_api_to_pipe.c_str(),
							GENERIC_READ | GENERIC_WRITE,
							0, nullptr, OPEN_EXISTING, 0, nullptr);

						// Create the buffer
						char read_buffer[2048]{};
						DWORD read = DWORD();

						// Check if we're good
						if (ReaderPipe.has_value())
							// Read the pipe
							ReadFile(ReaderPipe.value(),
							         read_buffer, 2048,
							         &read, nullptr);

						else
							LOG(ERROR) << "Error: Pipe object was not initialized.";

						// Close the pipe
						CloseHandle(ReaderPipe.value());

						// parse request, send reply and return
						try
						{
							// Deserialize now
							ktvr::K2Message response;
							response.ParseFromString(std::string(read_buffer, 2048));

							parse_message(response);
						}
						catch (const std::exception& e)
						{
							LOG(ERROR) << "Global parsing error: " << e.what();
						}
					}
					// if we're currently not running, just wait
					std::this_thread::sleep_for(std::chrono::milliseconds(1000));
				}
			}
			catch (...) // Catch everything
			{
				LOG(ERROR) << "Server loop has crashed! Restarting it now...";
				server_tries++; // One more?
				if (server_tries > 3)
				{
					// We've crashed the third time now. Somethin's off.. really...
					LOG(ERROR) << "Server loop has already crashed 3 times. Giving up...";
					server_giveUp = true;
				}
			}
		}
	}).detach();

	// if everything went ok, return 0
	return 0;
}

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

void K2ServerDriver::parse_message(const ktvr::K2Message& message)
{
	std::string _reply; // Reply that will be sent to client
	ktvr::K2ResponseMessage _response; // Response message to be sent

	// Add the timestamp: parsing
	_response.set_messagemanualtimestamp(AME_API_GET_TIMESTAMP_NOW);

	// Parse the message if it's not invalid
	if (message.messagetype() != ktvr::K2MessageType::K2Message_Invalid)
	{
		// Switch based on the message type
		switch (message.messagetype())
		{
		case ktvr::K2MessageType::K2Message_SetTrackerState:
			{
				// Check if desired tracker exists
				// Set tracker's state to one gathered from argument
				if (!trackerVector.at(message.tracker()).is_added())
					if (!trackerVector.at(message.tracker()).spawn())
					{
						// spawn if needed
						LOG(INFO) << "Tracker autospawn exception! Serial: " +
							trackerVector.at(message.tracker()).get_serial();

						_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_SpawnFailed);
					}

				// Set the state
				trackerVector.at(message.tracker()).set_state(message.state());
				LOG(INFO) << "Tracker role: " << message.tracker() <<
					" state has been set to: " + std::to_string(message.state());

				// Update the tracker
				trackerVector.at(message.tracker()).update();

				// Compose the response
				_response.set_success(true);
				_response.set_tracker(message.tracker()); // ID
				_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Role);
			}
			break;

		case ktvr::K2MessageType::K2Message_SetStateAll:
			{
				// Set all trackers' state
				for (K2Tracker& k2_tracker : trackerVector)
				{
					if (!k2_tracker.is_added())
						if (!k2_tracker.spawn())
						{
							// spawn if needed
							LOG(INFO) << "Tracker autospawn exception! Serial: " + k2_tracker.get_serial();
							_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_SpawnFailed);
						}
					k2_tracker.set_state(message.state()); // set state
					k2_tracker.update(); // Update the tracker
				}
				LOG(INFO) << "All trackers' state has been set to: " + std::to_string(message.state());

				// Compose the response
				_response.set_success(true);
				_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Success);
			}
			break;

		case ktvr::K2MessageType::K2Message_RefreshTracker:
			{
				// Update the tracker
				trackerVector.at(message.tracker()).update();

				// Compose the response
				_response.set_success(true);
				_response.set_tracker(message.tracker()); // ID
				_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Role);
			}
			break;

		case ktvr::K2MessageType::K2Message_RequestRestart:
			{
				// Request the reboot
				if (!message.message_string().empty())
				{
					LOG(INFO) << "Requesting OpenVR restart with reason: " + message.message_string();

					// Perform the request
					vr::VRServerDriverHost()->RequestRestart(
						message.message_string().c_str(), "vrstartup.exe", "", "");

					// Compose the response
					_response.set_success(true);
					_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Success);
				}
				else
				{
					LOG(ERROR) << "Couldn't request a reboot. Reason string is empty.";
					_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_BadRequest);
				}
			}
			break;

		case ktvr::K2MessageType::K2Message_Ping:
			{
				// Compose the response
				_response.set_success(true);
				_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Tracker);
			}
			break;

		case ktvr::K2MessageType::K2Message_UpdateTrackerPoseVector:
			{
				// Check if the pose exists
				if (message.trackerbasevector_size() > 0)
				{
					for (const auto& _tracker : message.trackerbasevector())
					{
						/* Pose */

						// Check if desired tracker exists
						// Update tracker pose (with time offset)
						trackerVector.at(_tracker.tracker()).set_pose(_tracker.pose());

						// Compose the response
						_response.set_success(true);
						_response.set_tracker(_tracker.tracker()); // ID
						_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Role);
					}
				}
				else
				{
					LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
					_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_BadRequest);
				}
			}
			break;

		case ktvr::K2MessageType::K2Message_SetTrackerStateVector:
			{
				// Check if the pose exists
				if (message.trackerstatusesvector_size() > 0)
				{
					for (const auto& _tracker : message.trackerstatusesvector())
					{
						// Assume success
						_response.set_success(true);

						/* State */

						if (!trackerVector.at(_tracker.tracker()).is_added())
							if (!trackerVector.at(_tracker.tracker()).spawn())
							{
								// spawn if needed
								LOG(INFO) << "Tracker autospawn exception! Serial: " +
									trackerVector.at(_tracker.tracker()).get_serial();
								_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_SpawnFailed);
								_response.set_success(false); // Oof we didn't make it
							}

						// Set the state
						trackerVector.at(_tracker.tracker()).set_state(_tracker.status());
						LOG(INFO) << "Tracker role: " << _tracker.tracker() <<
							" state has been set to: " + std::to_string(_tracker.status());

						// Update the tracker
						trackerVector.at(_tracker.tracker()).update();

						// Compose the response
						_response.set_tracker(_tracker.tracker()); // ID
						_response.set_messagetype(ktvr::K2ResponseMessageType::K2ResponseMessage_Role);
					}
				}
				else
				{
					LOG(ERROR) << "Couldn't update multiple trackers, bases are empty.";
					_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_BadRequest);
				}
			}
			break;

		default:
			LOG(ERROR) << "Couldn't process message. The message type was not set. (Type invalid)";
			_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_BadRequest);
			break;
		}
	}
	else
	{
		LOG(ERROR) << "Couldn't process message. Message had had invalid type.";
		_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_ParsingError);

		// We're done, screw em if the type was bad
		ReleaseSemaphore(k2api_to_Semaphore, 1, nullptr);
		return;
	}

	// Check the return code
	if (_response.success()) // If succeed, let's assume it's okay
		_response.set_result(ktvr::K2ResponseMessageCode::K2ResponseMessageCode_OK);

	// Set the manual timestamp
	_response.set_messagetimestamp(AME_API_GET_TIMESTAMP_NOW);

	// Serialize the response
	_reply = _response.SerializeAsString();

	// Check if the client wants a response and eventually send it
	if (message.want_reply())
	{
		// Here, write to the *from* pipe
		HANDLE WriterPipe = CreateNamedPipeW(
			ktvr::ame_api_from_pipe_address.c_str(),
			PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			1, 2048, 2048, 1000L, nullptr);
		DWORD Written;

		// Let the client know that we'll be writing soon
		ReleaseSemaphore(k2api_from_Semaphore, 1, nullptr);

		// Read from the pipe
		ConnectNamedPipe(WriterPipe, nullptr);
		WriteFile(WriterPipe,
		          _reply.c_str(),
		          strlen(_reply.c_str()),
		          &Written, nullptr);
		FlushFileBuffers(WriterPipe);

		// Disconnect and close the pipe
		DisconnectNamedPipe(WriterPipe);
		CloseHandle(WriterPipe);
	}
}

/*
 * FROM-TO
 * BEGIN-STRING_END: data.substr(data.rfind(begin) + begin.length())
 * STRING_BEGIN-END: data.substr(0, data.rfind(end))
 * BEGIN_END: data.substr(data.find(begin) + begin.length(), data.rfind(end) - end.length())
 */
