#pragma once
#include "K2DriverService.h"

class K2ServerDriver
{
public:
	// Initialize the service object
	[[nodiscard]] int init_server_driver(const int& port = 7135);

	// Shutdown the service object
	void kill_server_driver() const;
	
	// Tracker vector
	std::vector<K2Tracker> tracker_vector;

private:
	// The gRPC handler service
	K2DriverService service_;

	// The gRPC handler server
	std::unique_ptr<grpc::Server> server_;
};
