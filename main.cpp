#include <iostream>
#include <string>
#include <deque>
#include <mutex>
#include <future>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <metawear/core/metawearboard.h>
#include <warble/warble.h>
#include <metawear/core/status.h>


// utils
static constexpr size_t UUID_STR_LEN = 37;
static inline char halfByteToChar(size_t val);
static inline void boostUuidToCStr(const boost::uuids::uuid& Uuid, char cstr[UUID_STR_LEN]);
static inline void gattCharToCStr(const MblMwGattChar* characteristic, char cstr[UUID_STR_LEN]);

static MblMwMetaWearBoard* board;
static WarbleGatt* gatt;
static const char* MetaWearMAC;
static MblMwBtleConnection btleConnection;

// reading GATT characteristic
static uint8_t* gattCharVal;
static uint8_t gattCharLen;

// writing GATT characteristic
struct writeElement
{
    MblMwGattCharWriteType writeType;
    WarbleGattChar* gattChar;
    const uint8_t* gattCharVal;
    uint8_t gattCharLen;
};
static std::deque<writeElement*> writeQueue;
static std::mutex writeMutex;

// enabling notification if GATT characteristic changes
struct enableNotifyContext
{
    std::promise<int>* task;
    const void* caller;
    MblMwFnIntVoidPtrArray handler;
    WarbleGattChar* gattChar;
};
static enableNotifyContext* s_enableNotifyContext;
static std::mutex enableNotifyMutex;

// calling handler if disconnect
struct dcContext
{
    std::promise<int32_t>* task;
    const void* caller;
    MblMwFnVoidVoidPtrInt handler;
};
static dcContext* s_dcContext;
static std::mutex dcMutex;

static WarbleGattChar* warbleGattToGattChar(WarbleGatt* gatt, const MblMwGattChar* characteristic)
{
    char gattCharUuid[UUID_STR_LEN];
    gattCharToCStr(characteristic, gattCharUuid);  // this will update gattCharUuid
    
    WarbleGattChar* gattChar = warble_gatt_find_characteristic(gatt, gattCharUuid);
    if (!gattChar)
    {
	throw std::runtime_error("Error in warbleGattToGattChar(): gattChar '" + std::string(gattCharUuid) + "' does not exist");
    }
    return gattChar;
}

static void readCompletedCallback(void* context, WarbleGattChar* caller, const uint8_t* value, uint8_t length, const char* error)
{
    (void) caller;

    auto task = static_cast<std::promise<void>*>(context);
	
    if (error != nullptr)
    {
	task->set_exception(make_exception_ptr(std::runtime_error(error)));
    }
    else
    {
	if (!gattCharVal)
	{
	    delete gattCharVal;
	}
	gattCharVal = new uint8_t[length];
	for (auto it = 0; it < length; ++it)
	{
	    gattCharVal[it] = value[it];
	}
	gattCharLen = length;
	task->set_value();
    }
}

static void readGattCharPrint(WarbleGattChar* gattChar)
{
    std::cout << "Reading characteristic: {" << warble_gattchar_get_uuid(gattChar) << "}: ";

    for (auto i = 0; i < gattCharLen; ++i)
    {
	std::cout << (char)(gattCharVal[i]);
    }
    std::cout << "\n";
}

static void readGattChar(void* context, const void* caller, const MblMwGattChar* characteristic, MblMwFnIntVoidPtrArray handler)
{
    WarbleGatt* gatt = static_cast<WarbleGatt*>(context);
    WarbleGattChar* gattChar = warbleGattToGattChar(gatt, characteristic);

    std::promise<void> readTask;
    warble_gattchar_read_async(gattChar, &readTask, readCompletedCallback);
    readTask.get_future().get();
    
    readGattCharPrint(gattChar);
    handler(caller, gattCharVal, gattCharLen);
}

static void writeCompletedCallback(void* context, WarbleGattChar* caller, const char* error)
{
    (void) caller;
	
    auto task = static_cast<std::promise<void>*>(context);

    if (error != nullptr)
    {
	task->set_exception(make_exception_ptr(std::runtime_error(error)));
    }
    else
    {
	writeElement* elem = writeQueue.front();
	writeQueue.pop_front();
	delete elem;
	task->set_value();
    }
}

static void writeGattCharPrint(WarbleGattChar* gattChar, const uint8_t* value, uint8_t length)
{
    std::cout << "Writing characteristic: {" << warble_gattchar_get_uuid(gattChar) << "}: ";

    for (auto i = 0; i < length; ++i)
    {
	std::cout << (int)(value[i]) << " ";
    }
    std::cout << "\n";
}

static void writeGattCharAsync()
{
    writeElement* elem = nullptr;

    {
	std::unique_lock<std::mutex> lock(writeMutex);

	if (!writeQueue.empty())
	{
	    elem = writeQueue.front();
	}
    }

    if (elem)
    {
	writeGattCharPrint(elem->gattChar, elem->gattCharVal, elem->gattCharLen);
	
	std::promise<void> writeTask;
	if (elem->writeType == MBL_MW_GATT_CHAR_WRITE_WITH_RESPONSE)
	{
	    warble_gattchar_write_async(elem->gattChar, elem->gattCharVal, elem->gattCharLen, &writeTask, writeCompletedCallback);
	}
	else if (elem->writeType == MBL_MW_GATT_CHAR_WRITE_WITHOUT_RESPONSE)
	{
	    warble_gattchar_write_without_resp_async(elem->gattChar, elem->gattCharVal, elem->gattCharLen, &writeTask, writeCompletedCallback);
	}
	
	writeTask.get_future().get();
	if (!writeQueue.empty())
	{
	    writeGattCharAsync();
	}
    }
}

static void writeGattChar(void* context, const void* caller, MblMwGattCharWriteType write_type, const MblMwGattChar* characteristic, const uint8_t* value, uint8_t length)
{
    if (write_type != MBL_MW_GATT_CHAR_WRITE_WITH_RESPONSE && write_type != MBL_MW_GATT_CHAR_WRITE_WITHOUT_RESPONSE)
    {
	throw std::runtime_error("Error in writeGattChar(): invalid write_type arg.");
    }

    (void) caller;
    
    WarbleGatt* gatt = static_cast<WarbleGatt*>(context);
    WarbleGattChar* gattChar = warbleGattToGattChar(gatt, characteristic);

    bool doGattWrite = false;

    {
	std::unique_lock<std::mutex> lock(writeMutex);

	doGattWrite = writeQueue.empty();
	writeElement* newElement = new writeElement;
	newElement->writeType = write_type;
	newElement->gattChar = gattChar;
	newElement->gattCharVal = value;
	newElement->gattCharLen = length;
	writeQueue.emplace_back(newElement);
    }
    
    if (doGattWrite)
    {
	writeGattCharAsync();
    }
}

static void receiveNotifyCompletedCallback(void* context, WarbleGattChar* caller, const uint8_t* value, uint8_t length)
{
    (void) caller;
    
    auto _context = static_cast<enableNotifyContext*>(context);
    
    _context->handler(_context->caller, value, length);
}

static void enableNotifyCompletedCallback(void* context, WarbleGattChar* caller, const char* error)
{
    (void) caller;
    
    auto _context = static_cast<enableNotifyContext*>(context);

    if (error != nullptr)
    {
	_context->task->set_value(0);
    }
    else
    {
	{
	    std::unique_lock<std::mutex> lock(enableNotifyMutex);

	    std::cout << "Enabling notification of characteristic: {" << warble_gattchar_get_uuid(_context->gattChar) << "}\n";
	    warble_gattchar_on_notification_received(_context->gattChar, _context, receiveNotifyCompletedCallback);
	}

	_context->task->set_value(1);
    }
}

static void enableCharNotify(void* context, const void* caller, const MblMwGattChar* characteristic, MblMwFnIntVoidPtrArray handler, MblMwFnVoidVoidPtrInt ready)
{
    WarbleGatt* gatt = static_cast<WarbleGatt*>(context);
    WarbleGattChar* gattChar = warbleGattToGattChar(gatt, characteristic);

    std::promise<int> enableNotifyTask;
    {
	std::unique_lock<std::mutex> lock(enableNotifyMutex);

	s_enableNotifyContext = new enableNotifyContext;
	s_enableNotifyContext->task = &enableNotifyTask;
	s_enableNotifyContext->caller = caller;
	s_enableNotifyContext->handler = handler;
	s_enableNotifyContext->gattChar = gattChar;
    }

    warble_gattchar_enable_notifications_async(gattChar, s_enableNotifyContext, enableNotifyCompletedCallback);
    int success = s_enableNotifyContext->task->get_future().get();
    if (success == 0)
    {
	std::cerr << "Failed to enable notifications.\n";
	ready(caller, MBL_MW_STATUS_ERROR_ENABLE_NOTIFY);
    }
    else if (success == 1)
    {
	std::cerr << "Succeeded in enabling notifications.\n";
	ready(caller, MBL_MW_STATUS_OK);
    }

    {
	std::unique_lock<std::mutex> lock(enableNotifyMutex);
	
	delete s_enableNotifyContext;
    }
}

static void dcCompletedCallback(void* context, WarbleGatt* caller, int32_t value)
{
    (void) caller;
    
    auto _context = static_cast<dcContext*>(context);

    {
	std::unique_lock<std::mutex> lock(dcMutex);

	_context->handler(_context->caller, value);
    }
}

static void onDisconnect(void* context, const void* caller, MblMwFnVoidVoidPtrInt handler)
{
    WarbleGatt* gatt = static_cast<WarbleGatt*>(context);

    int32_t connectionStatus = 0;

    {
	std::unique_lock<std::mutex> lock(dcMutex);

        connectionStatus = warble_gatt_is_connected(gatt);
	
	s_dcContext = new dcContext;
	s_dcContext->caller = caller;
	s_dcContext->handler = handler;
    }

    if (connectionStatus == 0)
    {
	warble_gatt_on_disconnect(gatt, s_dcContext, dcCompletedCallback);
    }
    else
    {
	handler(caller, MBL_MW_STATUS_OK);
    }
}

static void initCompleteCallback(void* context, MblMwMetaWearBoard* board, int32_t status)
{
    (void) board;

    auto task = static_cast<std::promise<void>*>(context);

    if (status)
    {
	std::cerr << "Error initialising board: " << status << "\n";
    }
    else
    {
	std::cerr << "Board initialised\n";
    }
    
    task->set_value();
}

static void initBoard(MblMwMetaWearBoard* board)
{
    std::promise<void> initializeTask;
    mbl_mw_metawearboard_initialize(board, &initializeTask, initCompleteCallback);
    initializeTask.get_future().get();
}

// taken from example/connect.cpp in Warble repo
static void connectCompletedCallback(void* context, WarbleGatt* caller, const char* value)
{
    (void) caller;
    
    auto task = static_cast<std::promise<void>*>(context);
	
    if (value != nullptr)
    {
	std::cerr << "Check that MAC address of device is input correctly.\n";
	task->set_exception(make_exception_ptr(std::runtime_error(value)));
    }
    else
    {
	task->set_value();
    }
}

static void attemptConnectToMetaWear()
{
    std::cout << "Connecting to " << MetaWearMAC << "\n";

    /* Ignore any message on Terminal that reads "Error on line: 296 (src/blestatemachine.cc):
     * Operation now in progress"
     * (https://mbientlab.com/community/discussion/2636/cannot-connect-using-the-python-wrapper-for-metawear)
     */
    std::promise<void> connectTask;
    warble_gatt_connect_async(gatt, &connectTask, connectCompletedCallback);
    connectTask.get_future().get();
    if (!warble_gatt_is_connected(gatt))
    {
	throw std::runtime_error("Connection to " + std::string(MetaWearMAC) + " failed.");
    }
    
    std::cerr << "Connected to " << MetaWearMAC << "\n";
}

// taken from: https://stackoverflow.com/questions/52981188/boostuuid-into-char-without-stdstring
static inline char halfByteToChar(size_t val)
{
    if (val <= 9)
    {
	return static_cast<char>('0' + val);
    }
    else
    {
	return static_cast<char>('a' + (val-10));
    }
}

// taken from: https://stackoverflow.com/questions/52981188/boostuuid-into-char-without-stdstring
static inline void boostUuidToCStr(const boost::uuids::uuid& Uuid, char cstr[UUID_STR_LEN])
{
    size_t cstrIdx = 0;
    for (boost::uuids::uuid::const_iterator it = Uuid.begin(); it != Uuid.end(); ++it)
    {
	const size_t higherHalf = ((*it) >> 4) & 0x0F;
	cstr[cstrIdx++] = halfByteToChar(higherHalf);

	const size_t lowerHalf = (*it) & 0x0F;
	cstr[cstrIdx++] = halfByteToChar(lowerHalf);

 	if (cstrIdx == 8 || cstrIdx == 13 || cstrIdx == 18 || cstrIdx == 23)
 	{
 	    cstr[cstrIdx++] = '-';
 	}
     }
     cstr[UUID_STR_LEN - 1] = '\0';
}

static inline void gattCharToCStr(const MblMwGattChar* characteristic, char cstr[UUID_STR_LEN])
{
    uint8_t gattCharUuidFull[16];
    uint8_t* uuidHighPtr = (uint8_t*) &(characteristic->uuid_high);
    uint8_t* uuidLowPtr = (uint8_t*) &(characteristic->uuid_low);

    for (int idx = 0; idx < 8; ++idx)
    {
	gattCharUuidFull[7-idx] = uuidHighPtr[idx];
	gattCharUuidFull[15-idx] = uuidLowPtr[idx];
    }

    boost::uuids::uuid Uuid;
    size_t idx = 0;
    for (auto it = Uuid.begin(); it != Uuid.end(); ++it)
    {
	*it = gattCharUuidFull[idx++];
    }

    boostUuidToCStr(Uuid, cstr);
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
	std::cerr << "main.cpp: error: ./main [MAC address of device]\n";
	return 1;
    }

    MetaWearMAC = argv[1];
    gatt = warble_gatt_create(MetaWearMAC);
    attemptConnectToMetaWear();
    btleConnection = { gatt, writeGattChar, readGattChar, enableCharNotify, onDisconnect };
    board = mbl_mw_metawearboard_create(&btleConnection);
    initBoard(board);
}
