#include "pch.h"
#include "KinectToVR_API.h"

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

template <class V>
std::type_info const& var_type(V const& v)
{
	return std::visit([](auto&& x)-> decltype(auto) { return typeid(x); }, v);
}

namespace ktvr
{
	int init_k2api(
		const std::string& k2_to_pipe,
		const std::string& k2_from_pipe,
		const std::string& k2_to_sem,
		const std::string& k2_from_sem,
		const std::string& k2_start_sem) noexcept
	{
		try
		{
			// Copy pipe addresses
			k2api_to_pipe_address = k2_to_pipe;
			k2api_from_pipe_address = k2_from_pipe;

			// Open existing *to* semaphore
			k2api_to_Semaphore = OpenSemaphoreA(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_to_sem.c_str()); //Semaphore Name

			// Open existing *from* semaphore
			k2api_from_Semaphore = OpenSemaphoreA(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_from_sem.c_str()); //Semaphore Name

			// Open existing *start* semaphore
			k2api_start_Semaphore = OpenSemaphoreA(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_start_sem.c_str()); //Semaphore Name

			if (k2api_to_Semaphore == nullptr ||
				k2api_from_Semaphore == nullptr ||
				k2api_start_Semaphore == nullptr)
				return -1;
		}
		catch (std::exception const& e)
		{
			return -1;
		}
		return 0;
	}

	// May be blocking
	int close_k2api() noexcept
	{
		try
		{
			CloseHandle(k2api_to_Semaphore);
			CloseHandle(k2api_from_Semaphore);
			CloseHandle(k2api_start_Semaphore);
		}
		catch (std::exception const& e)
		{
			return -1;
		}
		return 0;
	}
	
	std::string send_message(std::string const& data, const bool want_reply) noexcept(false)
	{
		// change the string to hex format
		std::string msg_data = hexString(data);

		///// Send the message via named pipe /////

		// Wait for the semaphore if it's locked
		WaitForSingleObject(k2api_to_Semaphore, INFINITE);

		// Here, write to the *to* pipe
		HANDLE API_WriterPipe = CreateNamedPipeA(
			k2api_to_pipe_address.c_str(),
			PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			1, 4096, 4096, 1000L, nullptr);
		DWORD Written;

		// Let the server know we'll be writing soon
		ReleaseSemaphore(k2api_start_Semaphore, 1, 0);

		// Read from the pipe
		ConnectNamedPipe(API_WriterPipe, nullptr);
		WriteFile(API_WriterPipe,
		          msg_data.c_str(),
		          strlen(msg_data.c_str()),
		          &Written, nullptr);
		FlushFileBuffers(API_WriterPipe);

		// Close the pipe
		DisconnectNamedPipe(API_WriterPipe);
		CloseHandle(API_WriterPipe);

		///// Send the message via named pipe /////

		///// Receive the response via named pipe /////

		if (want_reply)
		{
			// Wait for the server to request a response, max 1s
			if (WaitForSingleObject(k2api_from_Semaphore, 1000L) != WAIT_OBJECT_0)
			{
				k2api_last_error = "Server didn't respond after 1 second.\n";
				return "";
				//LOG(ERROR) << "Server didn't respond after 1 second.\n";
			}

			// Here, read from the *from* pipe
			// Create the pipe file
			std::optional<HANDLE> API_ReaderPipe = CreateFile(
				TEXT("\\\\.\\pipe\\k2api_from_pipe"),
				GENERIC_READ | GENERIC_WRITE,
				0, nullptr, OPEN_EXISTING, 0, nullptr);

			// Create the buffer
			char API_read_buffer[4096];
			DWORD Read = DWORD();

			// Check if we're good
			if (API_ReaderPipe.has_value())
			{
				// Read the pipe
				ReadFile(API_ReaderPipe.value(),
				         API_read_buffer, 4096,
				         &Read, nullptr);
			}
			else
			{
				k2api_last_error = "Error: Pipe object was not initialized.";
				return "";
				//LOG(ERROR) << "Error: Pipe object was not initialized.";
			}

			CloseHandle(API_ReaderPipe.value());

			// Unlock the semaphore after job done
			ReleaseSemaphore(k2api_to_Semaphore, 1, 0);

			///// Receive the response via named pipe /////

			// I don't know why, when and how,
			// but somehow K2API inserts this shit at index 62
			std::string _s = API_read_buffer;
			if (_s.find("3120300a") == 62)_s.erase(62, 8);

			// decode hex reply
			return asciiString(_s); // Return only the reply
		}
		else
		{
			// Unlock the semaphore after job done
			ReleaseSemaphore(k2api_to_Semaphore, 1, 0);
			return ""; // No reply
		}
	}

	K2ResponseMessage add_tracker(K2TrackerBase& tracker) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			K2ResponseMessage response =
				send_message(K2Message(tracker));

			// Overwrite the current tracker's id
			tracker.id = response.id;

			// Send the message and return
			return response;
		}
		catch (std::exception const& e)
		{
			return K2ResponseMessage(); // Success is set to false by default
		}
	}

	K2ResponseMessage download_tracker(int const& tracker_id) noexcept
	{
		try
		{
			// Send and grab the response
			// Thanks to our constructors,
			// message will set all
			// Send the message and return
			return send_message(K2Message(tracker_id));
		}
		catch (std::exception const& e)
		{
			return K2ResponseMessage(); // Success is set to false by default
		}
	}

	K2ResponseMessage download_tracker(std::string const& tracker_serial) noexcept
	{
		try
		{
			// Send and grab the response
			// Send the message and return
			// Normally, we'd set some id to grab tracker from,
			// although this time it'll be -1,
			// forcing the driver to check if we've provided a serial
			K2Message message = K2Message();
			message.messageType = static_cast<int>(K2MessageType::K2Message_DownloadTracker);
			message.tracker_data.serial = tracker_serial;

			return send_message(message);
		}
		catch (std::exception const& e)
		{
			return K2ResponseMessage(); // Success is set to false by default
		}
	}

	K2ResponseMessage download_tracker(K2TrackerBase const& tracker) noexcept
	{
		try
		{
			// Send the message and return
			return download_tracker(tracker.id);
		}
		catch (std::exception const& e)
		{
			return K2ResponseMessage(); // Success is set to false by default
		}
	}

	K2ResponseMessage download_tracker(const ITrackerType& tracker_role) noexcept
	{
		try
		{
			// Send and grab the response
			// Send the message and return
			// Normally, we'd set some id to grab tracker from,
			// although this time it'll be skipped,
			// the driver will check message_string and download via role
			K2Message message = K2Message();
			message.messageType = static_cast<int>(K2MessageType::K2Message_DownloadTracker);
			message.message_string = "role";
			message.tracker_data.role = static_cast<uint32_t>(tracker_role);

			return send_message(message);
		}
		catch (std::exception const& e)
		{
			return K2ResponseMessage(); // Success is set to false by default
		}
	}

	std::tuple<K2ResponseMessage, long long, long long> test_connection() noexcept
	{
		try
		{
			K2Message message = K2Message();
			message.messageType = static_cast<int>(K2MessageType::K2Message_Ping);

			// Grab the current time and send the message
			const long long send_time = K2API_GET_TIMESTAMP_NOW;
			const auto response = send_message(message);

			// Return tuple with response and elapsed time
			const auto elapsed_time = // Always >= 0
				std::clamp(K2API_GET_TIMESTAMP_NOW - send_time,
				           static_cast<long long>(0), LLONG_MAX);
			return std::make_tuple(response, send_time, elapsed_time);
		}
		catch (std::exception const& e)
		{
			return std::make_tuple(K2ResponseMessage(), (long long)0, (long long)0);
			// Success is set to false by default
		}
	}

	std::string hexString(const std::string& s)
	{
		std::ostringstream ret;

		// Change ascii string to hex string
		for (char c : s)
			ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << (int)c;
		std::string ret_s = ret.str();

		// Erase 0a00 if there is so
		if (ret_s.find("0a00") == 0)ret_s.erase(0, 4);
		return ret_s;;
	}

	std::string asciiString(const std::string& s)
	{
		std::ostringstream ret;

		// Start from the index 2, removing 0A00 occur
		for (std::string::size_type i = 0; i < s.length(); i += 2)
			ret << (char)(int)strtol(s.substr(i, 2).c_str(), nullptr, 16);

		return ret.str();
	}
}
