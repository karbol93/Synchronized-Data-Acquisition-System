#ifndef __KESPP_CLIENT_HPP__
#define __KESPP_CLIENT_HPP__

#include <string>
#include <ctime>
#include <cstdint>
#include <unistd.h>
#include <cstring>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "frametypes.hpp"

/// Main KESP-P client class which implements KESP-P protocol.
///
/// Use this class in your plugin for easy interfacing with KESP server,
/// using @ref kespp "KMD-3 Edge Synchronization Protocol for Plugin".
class KESPPClient
{
private:
    static constexpr size_t _kStrerrorBufferSize = 512;
    static constexpr uint8_t _kKESPPLiteral = 1;
    static constexpr uint8_t _kProtocolVersion = 1;

    std::string _hostname;
    uint16_t _port;
    std::atomic_bool _connected;
    int _socket_descr;
    uint64_t _realtime_diff_nanosec;
    uint64_t _timeout_nanosec;
    ControlFrame _control_frame;
    std::string _error_msg;

    std::thread _receiver_thread_handle;
    std::mutex _data_mtx;
    std::mutex _error_message_mtx;
    std::condition_variable _wait_for_start_cv;

    addrinfo *_resolve_address(const std::string &hostname, const uint16_t port)
    {
        addrinfo hints;
        addrinfo *results = nullptr;

        std::memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int error = getaddrinfo(
            hostname.c_str(), std::to_string(port).c_str(),
            &hints, &results);
        if (error != 0)
        {
            _error_msg = std::string("Getaddrinfo error: ") + gai_strerror(error);
            freeaddrinfo(results);
            return nullptr;
        }

        return results;
    }

    bool _connect()
    {
        addrinfo *results = _resolve_address(_hostname.c_str(), _port);
        if (!results)
            return false;

        _socket_descr = socket(results->ai_family, results->ai_socktype, results->ai_protocol);
        if (_socket_descr == -1)
        {
            _error_msg = "Socket error: Could not create socket descriptor";
            freeaddrinfo(results);
            return false;
        }

        int error = connect(_socket_descr, results->ai_addr, results->ai_addrlen);
        if (error == -1)
        {
            char *error_msg_buffer = new char[_kStrerrorBufferSize];
            _error_msg = std::string("Connecting error: ") + strerror_r(errno, error_msg_buffer, _kStrerrorBufferSize);

            delete[] error_msg_buffer;
            close(_socket_descr);
            freeaddrinfo(results);
            return false;
        }
        freeaddrinfo(results);
        return true;
    }

    void _receiver()
    {
        uint8_t r_buf[ControlFrame::kSize];
        while (true)
        {
            ssize_t r_count = recv(_socket_descr, r_buf, ControlFrame::kSize, 0);

            if (r_count == -1 && errno != EWOULDBLOCK)  // If recv gets a timeout (EWOULDBLOCK), nothing wrong happens and recv should wait again
            {
                char *error_msg_buffer = new char[_kStrerrorBufferSize];
                {
                    std::lock_guard<std::mutex> lg(_error_message_mtx);
                    _error_msg = std::string("Receiving error: ") + strerror_r(errno, error_msg_buffer, _kStrerrorBufferSize);
                }
                delete[] error_msg_buffer;
                break;
            }
            else if (r_count == 0)
            {
                {
                    std::lock_guard<std::mutex> lg(_error_message_mtx);
                    _error_msg = "Connection closed";
                }
                break;
            }
            else if (r_count != ControlFrame::kSize)
            {
                {
                std::lock_guard<std::mutex> lg(_error_message_mtx);
                _error_msg = std::string("Partially received ControlFrame (received ") + std::to_string(r_count) +
                             " bytes instead of " + std::to_string(ControlFrame::kSize) + " bytes)";
                }
                break;
            }

            {
                std::lock_guard<std::mutex> lg(_data_mtx);
                _control_frame.deserialize(r_buf);
            }
            _wait_for_start_cv.notify_one();
        }
        {
            std::lock_guard<std::mutex> lg(_data_mtx);
            _connected = false;
        }
        _wait_for_start_cv.notify_one();
        close(_socket_descr);
    }

public:
    static constexpr char kDefaultHost[] = "localhost";
    static constexpr uint16_t kDefaultPort = 19251;

    KESPPClient(const KESPPClient &) = delete;
    KESPPClient &operator=(const KESPPClient &) = delete;

    /// Instantiates KESP-P client object.
    ///
    /// Object is instantiated but not connected to server. Use try_connect() to establish connection.
    /// @param host localhost by default, IPv4? or name can be passed
    /// @param port server is listening at this port number
    KESPPClient(const char *host = kDefaultHost, const uint16_t port = kDefaultPort)
        : _hostname(host),
          _port(port),
          _connected(false),
          _socket_descr(-1),
          _realtime_diff_nanosec(0),
          _control_frame(ControlFrame(0, 1)),
          _error_msg("No error occurred")
    {
    }

    /// Returns connection status.
    ///
    /// @returns true if connected, false if disconnected
    bool connected() const { return _connected; }

    /// Returns difference between monotonic and realtime clock.
    ///
    /// The difference is measured by the server and broadcasted to clients.
    /// It allows for achieving realtime consistency across all the clients,
    /// so generated timestamps in filenames are consistent.
    /// @note You rather want to use recording_timestamp() method for textual timestamp generation.
    /// @returns difference (in nanoseconds) between monotonic and realtime clock
    uint64_t realtime_diff_nanosec() const { return _realtime_diff_nanosec; }
    
    /// Returns timeout time period.
    ///
    /// Timeout period is provided by the server while connecting.
    /// In order to calculate timeout timepoint, timeout period should be added to chunk timepoint (`chunk_nanosec`).
    /// Timeout is reached if new ControlFrame isn't provided up to this timepoint.
    /// @returns timeout period in nanoseconds
    uint64_t timeout_nanosec() const { return _timeout_nanosec; }
    
    /// Access to error message generated during connecting or after disconnection.
    ///
    /// @returns text of error message
    std::string error_msg()
    {
        std::lock_guard<std::mutex> lg(_error_message_mtx);
        return _error_msg;
    }

    /// Use for connecting to KESP server.
    ///
    /// Call once for one connection trial. Method can be called multiple times (retrying connection).
    /// If client already connected there is no effect.
    /// @returns true if connected successfully, false if connection failed and leaves error message returned by error_msg()
    bool try_connect()
    {
        {
            std::lock_guard<std::mutex> lg(_data_mtx);
            if (_connected)
                return _connected;
        }
        
        if (_receiver_thread_handle.joinable())
            _receiver_thread_handle.join();
        // From here only main thread can work

        _socket_descr = -1;
        _realtime_diff_nanosec = 0;
        _control_frame = ControlFrame(0, 1);
        _error_msg = "No error occurred";

        if (!_connect())
            return _connected;

        // Send PROTOCOL frame
        uint8_t protocol = _kKESPPLiteral;
        ssize_t s_count = send(_socket_descr, &protocol, ProtocolFrame::kSize, 0);
        if (s_count != ProtocolFrame::kSize)
        {
            _error_msg = "Couldn't send PROTOCOL frame";
            close(_socket_descr);
            return _connected;
        }

        // Receive WELCOME frame
        uint8_t recv_buffer[WelcomeFrame::kSize];
        ssize_t r_count = recv(_socket_descr, recv_buffer, WelcomeFrame::kSize, 0);
        if (r_count != WelcomeFrame::kSize)
        {
            _error_msg = "Wrong WELCOME frame size or error while receiving: " + std::to_string(r_count) + " bytes";
            close(_socket_descr);
            return _connected;
        }
        WelcomeFrame welcome_fr;
        welcome_fr.deserialize(recv_buffer);
        if (welcome_fr.protocol_version() != _kProtocolVersion)
        {
            _error_msg = std::string("Wrong KESP-P protocol version (server version: ") + std::to_string((int)welcome_fr.protocol_version()) +
                         ", client version: " + std::to_string((int)_kProtocolVersion) + ")";
            close(_socket_descr);
            return _connected;
        }
        _realtime_diff_nanosec = welcome_fr.realtime_diff_nanosec();
        _timeout_nanosec = welcome_fr.timeout_nanosec();

        // Set connection status and run receiver thread
        _connected = true; // has to be first, before thread creation
        _receiver_thread_handle = std::thread(&KESPPClient::_receiver, this);
        // From here multiple threads can work
    
        return _connected;
    }

    /// Blocks until receiving starting frame or disconnecting.
    ///
    /// Method returns after the starting frame is received from the KESP server.
    /// It's good practice to immediately use get_info() after unlocking and get ready to start.
    /// @note Unlocking doesn't mean that recording period has just started.
    /// There is still *buffer period* before starting point.
    void wait_for_start_info()
    {
        std::unique_lock<std::mutex> lock(_data_mtx);
        _wait_for_start_cv.wait(
            lock,
            [this]{ return !_connected.load() || !_control_frame.stop_flag(); }  // wake if not connected or stop flag is 0
        );
    }

    /// Returns current ControlFrame from \ref kespp "KESP-P" protocol
    ///
    /// The mechanizm of returning whole ControlFrame is needed for further implementing synchronized plugin.
    /// @returns most recent ControlFrame received by the client. Older frame is always forgotten.
    ControlFrame get_info()
    {
        std::lock_guard<std::mutex> lg(_data_mtx);
        return _control_frame;
    }

    /// Shortcut for retrieving current time from monotonic clock.
    /// @returns current time (in nanoseconds) from monotonic clock
    static int64_t now_nanosec()
    {
        timespec mono_time;
        clock_gettime(CLOCK_MONOTONIC, &mono_time);
        return static_cast<int64_t>(mono_time.tv_sec) * 1000000000LL + static_cast<int64_t>(mono_time.tv_nsec);
    }

    /// Use to generate timestamps for your filenames.
    ///
    /// Returned timestamp is based on passed timepoint and difference between
    /// realtime clock and monotonic clock. Difference is maintained automatically by the protocol.
    /// @param chunk_nanosec timepoint according to monotonic clock for which timestamp is generated.
    /// Probably you will always use ControlFrame::chunk_nanosec() const from ControlFrame returned by get_info().
    std::string recording_timestamp(int64_t chunk_nanosec) const
    {
        char date_c_str[256];
        const int64_t nano_time = _realtime_diff_nanosec + static_cast<uint64_t>(chunk_nanosec);
        const time_t time = nano_time / 1000000000LL;
        std::strftime(date_c_str, sizeof(date_c_str), "%Y%m%d_%H%M%S_", std::localtime(&time));
        std::string date(date_c_str);
        std::string mills = std::to_string(nano_time / 1000000LL - time * 1000LL);

        mills.insert(0, 3U - mills.length(), '0');
        date += mills;
        return std::string(date);
    }

    ~KESPPClient()
    {
        if (_receiver_thread_handle.joinable())
            _receiver_thread_handle.join();
    }
};

#endif //__KESPP_CLIENT_HPP__
