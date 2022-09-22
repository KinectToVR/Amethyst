#include "pch.h"
#include "Amethyst_API.h"

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
	int init_ame_api(
		const std::wstring& k2_to_pipe,
		const std::wstring& k2_from_pipe,
		const std::wstring& k2_to_sem,
		const std::wstring& k2_from_sem,
		const std::wstring& k2_start_sem) noexcept
	{
		try
		{
			// Copy pipe & semaphore addresses
			ame_api_to_pipe_address = k2_to_pipe;
			ame_api_from_pipe_address = k2_from_pipe;
			ame_api_to_semaphore_address = k2_to_sem;
			ame_api_from_semaphore_address = k2_from_sem;
			ame_api_start_semaphore_address = k2_start_sem;

			// Open existing *to* semaphore
			ame_api_to_semaphore = OpenSemaphoreW(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_to_sem.c_str()); //Semaphore Name

			// Open existing *from* semaphore
			ame_api_from_semaphore = OpenSemaphoreW(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_from_sem.c_str()); //Semaphore Name

			// Open existing *start* semaphore
			ame_api_start_semaphore = OpenSemaphoreW(
				SYNCHRONIZE | SEMAPHORE_MODIFY_STATE,
				FALSE,
				k2_start_sem.c_str()); //Semaphore Name

			if (ame_api_to_semaphore == nullptr ||
				ame_api_from_semaphore == nullptr ||
				ame_api_start_semaphore == nullptr)
				return -1;
		}
		catch (const std::exception& e)
		{
			return -1;
		}
		return 0;
	}

	// May be blocking
	int close_ame_api() noexcept
	{
		try
		{
			CloseHandle(ame_api_to_semaphore);
			CloseHandle(ame_api_from_semaphore);
			CloseHandle(ame_api_start_semaphore);
		}
		catch (const std::exception& e)
		{
			return -1;
		}
		return 0;
	}

	std::string send_message(const std::string& data, const bool want_reply) noexcept(false)
	{
		///// Send the message via named pipe /////
		
		// Wait for the semaphore if it's locked
		WaitForSingleObject(ame_api_to_semaphore, INFINITE);

		// Here, write to the *to* pipe
		const HANDLE API_WriterPipe =
			CreateNamedPipeW(
			ame_api_to_pipe_address.c_str(),
			PIPE_ACCESS_INBOUND | PIPE_ACCESS_OUTBOUND,
			PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE,
			1, 2048, 2048, 1000L, nullptr);
		DWORD Written;

		// Let the server know we'll be writing soon
		ReleaseSemaphore(ame_api_start_semaphore, 1, nullptr);

		// Read from the pipe
		ConnectNamedPipe(API_WriterPipe, nullptr);
		WriteFile(API_WriterPipe,
		          data.c_str(),
		          data.length(),
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
			if (WaitForSingleObject(ame_api_from_semaphore, 1000L) != WAIT_OBJECT_0)
			{
				ame_api_last_error = "Server didn't respond by the max timeout / 1 second.\n";
				return "";
				//LOG(ERROR) << "Server didn't respond after 1 second.\n";
			}

			// Here, read from the *from* pipe
			// Create the pipe file
			const std::optional API_ReaderPipe = 
				CreateFileW(ame_api_from_pipe_address.c_str(),
				GENERIC_READ | GENERIC_WRITE,
				0, nullptr, OPEN_EXISTING, 0, nullptr);

			// Create the buffer
			char API_read_buffer[2048]{};
			DWORD read = DWORD();

			// Check if we're good
			if (API_ReaderPipe.has_value())
			{
				// Read the pipe
				ReadFile(API_ReaderPipe.value(),
				         API_read_buffer, 2048,
				         &read, nullptr);
			}
			else
			{
				ame_api_last_error = "Error: Pipe object was not initialized.";
				return "";
				//LOG(ERROR) << "Error: Pipe object was not initialized.";
			}

			CloseHandle(API_ReaderPipe.value());

			// Unlock the semaphore after job done
			ReleaseSemaphore(ame_api_to_semaphore, 1, nullptr);

			///// Receive the response via named pipe /////

			return { API_read_buffer }; // Return only the reply
		}
		// Unlock the semaphore after job done
		ReleaseSemaphore(ame_api_to_semaphore, 1, nullptr);
		return ""; // No reply
	}

	std::monostate send_message_no_reply(K2Message message)
	{
		// Add timestamp
		message.set_messagetimestamp(AME_API_GET_TIMESTAMP_NOW);
		message.set_want_reply(false);

		// Serialize to string

		// Send the message
		send_message(message.SerializeAsString(), false);
		return {};
	}

	K2ResponseMessage send_message_want_reply(K2Message message)
	{
		// Add timestamp
		message.set_messagetimestamp(AME_API_GET_TIMESTAMP_NOW);
		message.set_want_reply(true);

		// Serialize to string

		// Send the message
		// Deserialize then

		// Compose the response
		const auto reply = send_message(
			message.SerializeAsString(),
			true);

		// Deserialize reply and return
		auto response = K2ResponseMessage();
		response.ParseFromString(reply);

		return response;
	}

	std::tuple<K2ResponseMessage, long long, long long> test_connection() noexcept
	{
		try
		{
			auto message = K2Message();
			message.set_messagetype(K2Message_Ping);

			// Grab the current time and send the message
			const long long send_time = AME_API_GET_TIMESTAMP_NOW;
			const auto response = send_message(message);

			// Return tuple with response and elapsed time
			const auto elapsed_time = // Always >= 0
				std::clamp(AME_API_GET_TIMESTAMP_NOW - send_time,
				           static_cast<long long>(0), LLONG_MAX);

			return std::make_tuple(response, send_time, elapsed_time);
		}
		catch (const std::exception& e)
		{
			return std::make_tuple(K2ResponseMessage(), static_cast<long long>(0), static_cast<long long>(0));
			// Success is set to false by default
		}
	}
}
