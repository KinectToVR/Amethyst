#ifndef PSMOVEPROTOCOL_INTERFACE_H
#define PSMOVEPROTOCOL_INTERFACE_H

//-- includes -----
#include <memory>
#include "SharedConstants.h"

//-- constants -----
#define MAX_OUTPUT_DATA_FRAME_MESSAGE_SIZE 500
#define MAX_INPUT_DATA_FRAME_MESSAGE_SIZE 64

// See ControllerManager.h in PSMoveService
#define PSMOVESERVICE_MAX_CONTROLLER_COUNT  10

// See TrackerManager.h in PSMoveService
#define PSMOVESERVICE_MAX_TRACKER_COUNT  8

// See HMDManager.h in PSMoveService
#define PSMOVESERVICE_MAX_HMD_COUNT  4
 
//-- pre-declarations -----
namespace PSMoveProtocol
{
    class DeviceOutputDataFrame;
    class DeviceInputDataFrame;
	class Request;
	class Response;
};

typedef std::shared_ptr<PSMoveProtocol::DeviceOutputDataFrame> DeviceOutputDataFramePtr;
typedef std::shared_ptr<PSMoveProtocol::DeviceInputDataFrame> DeviceInputDataFramePtr;
typedef std::shared_ptr<PSMoveProtocol::Request> RequestPtr;
typedef std::shared_ptr<PSMoveProtocol::Response> ResponsePtr;

//-- interface -----
class INotificationListener
{
public:
    virtual void handle_notification(ResponsePtr response) = 0;
};

class IDataFrameListener
{
public:
    virtual void handle_data_frame(const PSMoveProtocol::DeviceOutputDataFrame *data_frame) = 0;
};

class IResponseListener
{
public:
	virtual void handle_request_canceled(RequestPtr request) = 0;
	virtual void handle_response(ResponsePtr response) = 0;
};

#endif  // PSMOVEPROTOCOL_INTERFACE_H
