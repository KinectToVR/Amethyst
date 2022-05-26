#include "pch.h"
#include "Amethyst_API.h"

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/export.hpp>

#include <boost/serialization/vector.hpp>
#include <boost/serialization/array.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <boost/serialization/optional.hpp>

namespace boost::serialization
{
	// Eigen serialization
	template <class Archive, typename _Scalar, int _Rows, int _Cols, int _Options, int _MaxRows, int _MaxCols>
	void serialize(Archive& ar,
	               Eigen::Matrix<_Scalar, _Rows, _Cols, _Options, _MaxRows, _MaxCols>& t,
	               const unsigned int file_version
	)
	{
		for (size_t i = 0; i < t.size(); i++)
			ar & make_nvp(("m" + std::to_string(i)).c_str(), t.data()[i]);
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& ar, Eigen::Quaternion<_Scalar>& q, unsigned)
	{
		ar & make_nvp("w", q.w())
			& make_nvp("x", q.x())
			& make_nvp("y", q.y())
			& make_nvp("z", q.z());
	}

	template <class Archive, typename _Scalar>
	void serialize(Archive& ar, Eigen::Vector3<_Scalar>& v, unsigned)
	{
		ar & make_nvp("x", v.x())
			& make_nvp("y", v.y())
			& make_nvp("z", v.z());
	}
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
		catch (const std::exception& e)
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
		catch (const std::exception& e)
		{
			return -1;
		}
		return 0;
	}

	std::string send_message(const std::string& data, const bool want_reply) noexcept(false)
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
		ReleaseSemaphore(k2api_start_Semaphore, 1, nullptr);

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
				TEXT("\\\\.\\pipe\\k2api_amethyst_from_pipe"),
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
			ReleaseSemaphore(k2api_to_Semaphore, 1, nullptr);

			///// Receive the response via named pipe /////

			// I don't know why, when and how,
			// but somehow K2API inserts this shit at index 62
			std::string _s = API_read_buffer;
			if (_s.find("3120300a") == 62)_s.erase(62, 8);

			// decode hex reply
			return asciiString(_s); // Return only the reply
		}
		// Unlock the semaphore after job done
		ReleaseSemaphore(k2api_to_Semaphore, 1, nullptr);
		return ""; // No reply
	}

	std::monostate send_message_no_reply(K2Message message)
	{
		// Add timestamp
		message.messageTimestamp = K2API_GET_TIMESTAMP_NOW;
		message.want_reply = false;

		// Serialize to string
		std::ostringstream o;
		boost::archive::text_oarchive oa(o);
		oa << message;

		// Send the message

		// Probably void
		send_message(o.str(), false);
		return std::monostate();
	}

	K2ResponseMessage send_message_want_reply(K2Message message)
	{
		// Add timestamp
		message.messageTimestamp = K2API_GET_TIMESTAMP_NOW;
		message.want_reply = true;

		// Serialize to string
		std::ostringstream o;
		boost::archive::text_oarchive oa(o);
		oa << message;

		// Send the message
		// Deserialize then

		// Compose the response
		K2ResponseMessage response;
		auto reply = send_message(o.str(), true);

		std::istringstream i(reply);
		boost::archive::text_iarchive ia(i);
		ia >> response;

		// Deserialize reply and return
		return response;
	}

	std::tuple<K2ResponseMessage, long long, long long> test_connection() noexcept
	{
		try
		{
			auto message = K2Message();
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
		catch (const std::exception& e)
		{
			return std::make_tuple(K2ResponseMessage(), static_cast<long long>(0), static_cast<long long>(0));
			// Success is set to false by default
		}
	}

	std::string hexString(const std::string& s)
	{
		std::ostringstream ret;

		// Change ascii string to hex string
		for (char c : s)
			ret << std::hex << std::setfill('0') << std::setw(2) << std::nouppercase << static_cast<int>(c);
		std::string ret_s = ret.str();

		// Erase 0a00 if there is so
		if (ret_s.find("0a00") == 0)ret_s.erase(0, 4);
		return ret_s;
	}

	std::string asciiString(const std::string& s)
	{
		std::ostringstream ret;

		// Start from the index 2, removing 0A00 occur
		for (std::string::size_type i = 0; i < s.length(); i += 2)
			ret << static_cast<char>(static_cast<int>(strtol(s.substr(i, 2).c_str(), nullptr, 16)));

		return ret.str();
	}

	template <class Archive>
	KTVR_API void K2TrackerPose::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(orientation) & BOOST_SERIALIZATION_NVP(position);
	}

	template <class Archive>
	KTVR_API void K2TrackerData::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(serial) & BOOST_SERIALIZATION_NVP(role) & BOOST_SERIALIZATION_NVP(isActive);
	}

	template <class Archive>
	KTVR_API void K2PosePacket::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(orientation) & BOOST_SERIALIZATION_NVP(position) &
			BOOST_SERIALIZATION_NVP(millisFromNow); // Serialize
	}

	template <class Archive>
	KTVR_API void K2TrackerBase::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(pose) & BOOST_SERIALIZATION_NVP(data) & BOOST_SERIALIZATION_NVP(tracker);
	}

	template <class Archive>
	KTVR_API void K2Message::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(messageType)
			& BOOST_SERIALIZATION_NVP(tracker_base)
			& BOOST_SERIALIZATION_NVP(tracker_pose)
			& BOOST_SERIALIZATION_NVP(tracker_bases_vector)
			& BOOST_SERIALIZATION_NVP(message_string)
			& BOOST_SERIALIZATION_NVP(tracker)
			& BOOST_SERIALIZATION_NVP(state)
			& BOOST_SERIALIZATION_NVP(want_reply)
			& BOOST_SERIALIZATION_NVP(messageTimestamp)
			& BOOST_SERIALIZATION_NVP(messageManualTimestamp);
	}

	std::string K2Message::serializeToString()
	{
		std::ostringstream o;
		boost::archive::text_oarchive oa(o);
		oa << *this;
		return o.str();
	}

	[[nodiscard]] K2Message K2Message::parseFromString(const std::string& str) noexcept
	{
		try
		{
			std::istringstream i(str);
			boost::archive::text_iarchive ia(i);

			K2Message response;
			ia >> response;
			return response;
		}
		catch (const boost::archive::archive_exception& e)
		{
			return K2Message();
		}
	}

	template <class Archive>
	KTVR_API void K2ResponseMessage::serialize(Archive& ar, const unsigned int version)
	{
		ar & BOOST_SERIALIZATION_NVP(messageType)
			& BOOST_SERIALIZATION_NVP(tracker_base)
			& BOOST_SERIALIZATION_NVP(tracker)
			& BOOST_SERIALIZATION_NVP(result)
			& BOOST_SERIALIZATION_NVP(success)
			& BOOST_SERIALIZATION_NVP(messageTimestamp)
			& BOOST_SERIALIZATION_NVP(messageManualTimestamp);
	}

	std::string K2ResponseMessage::serializeToString()
	{
		std::ostringstream o;
		boost::archive::text_oarchive oa(o);
		oa << *this;
		return o.str();
	}

	[[nodiscard]] K2ResponseMessage K2ResponseMessage::parseFromString(const std::string& str) noexcept
	{
		try
		{
			std::istringstream i(str);
			boost::archive::text_iarchive ia(i);

			K2ResponseMessage response;
			ia >> response;
			return response;
		}
		catch (const boost::archive::archive_exception& e)
		{
		}
		return K2ResponseMessage();
	}
}
