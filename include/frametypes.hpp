#ifndef __frametypes_hpp__
#define __frametypes_hpp__

// Protocol KESP-Pv1 frames

#include <sstream>
#include <cstdint>

class ByteFrame
{
protected:
    uint8_t _byte;

public:
    static constexpr uint8_t kSize = 1;

    ByteFrame(const uint8_t &byte = 0) : _byte(byte)
    {
    }

    uint8_t &byte() { return _byte; }
    const uint8_t &byte() const { return _byte; }

    void clear()
    {
        _byte = 0;
    }

    void deserialize(const uint8_t (&buf)[kSize])
    {
        _byte = *buf;
    }

    void serialize(uint8_t (&buf)[kSize]) const
    {
        *buf = _byte;
    }

    bool operator==(const ByteFrame &frame) const
    {
        return _byte == frame._byte;
    }

    friend std::ostream &operator<<(std::ostream &out, const ByteFrame &frame);
    friend std::istream &operator>>(std::istream &in, ByteFrame &frame);
};

std::ostream &operator<<(std::ostream &out, const ByteFrame &frame)
{
    out.write(reinterpret_cast<const char *>(&frame._byte), sizeof(frame._byte));
    return out;
}

std::istream &operator>>(std::istream &in, ByteFrame &frame)
{
    in.read(reinterpret_cast<char *>(&frame._byte), sizeof(frame._byte));
    return in;
}

class ProtocolFrame : public ByteFrame
{
public:
    ProtocolFrame(const uint8_t &byte_ = 0) : ByteFrame(byte_)
    {
    }

    uint8_t &protocol() { return _byte; }
    const uint8_t &protocol() const { return _byte; }
};

class WelcomeFrame
{
private:
    uint8_t _protocol_version;
    uint64_t _realtime_diff_nanosec;
    uint64_t _timeout_nanosec;

public:
    static constexpr uint8_t kSize = 17;

    WelcomeFrame(const uint8_t &protocol_version = 0, const uint64_t &realtime_diff_nanosec = 0, const uint64_t timeout_nanosec = 0)
        : _protocol_version(protocol_version), _realtime_diff_nanosec(realtime_diff_nanosec), _timeout_nanosec(timeout_nanosec)
    {
    }

    uint8_t &protocol_version() { return _protocol_version; }
    const uint8_t &protocol_version() const { return _protocol_version; }
    uint64_t &realtime_diff_nanosec() { return _realtime_diff_nanosec; }
    const uint64_t &realtime_diff_nanosec() const { return _realtime_diff_nanosec; }
    uint64_t &timeout_nanosec() { return _timeout_nanosec; }
    const uint64_t &timeout_nanosec() const { return _timeout_nanosec; }

    void clear()
    {
        _protocol_version = 0;
        _realtime_diff_nanosec = 0;
        _timeout_nanosec = 0;
    }

    void deserialize(const uint8_t (&buf)[kSize])
    {
        _protocol_version = *buf;
        _realtime_diff_nanosec = *reinterpret_cast<const uint64_t *>(buf + sizeof(_protocol_version));
        _timeout_nanosec = *reinterpret_cast<const uint64_t *>(buf + sizeof(_protocol_version) + sizeof(_realtime_diff_nanosec));
    }

    void serialize(uint8_t (&buf)[kSize]) const
    {
        *buf = _protocol_version;
        *reinterpret_cast<uint64_t *>(buf + sizeof(_protocol_version)) = _realtime_diff_nanosec;
        *reinterpret_cast<uint64_t *>(buf + sizeof(_protocol_version) + sizeof(_realtime_diff_nanosec)) = _timeout_nanosec;
    }

    friend std::ostream &operator<<(std::ostream &out, const WelcomeFrame &frame);
    friend std::istream &operator>>(std::istream &in, WelcomeFrame &frame);
};

std::ostream &operator<<(std::ostream &out, const WelcomeFrame &frame)
{
    out.write(reinterpret_cast<const char *>(&frame._protocol_version), sizeof(frame._protocol_version));
    out.write(reinterpret_cast<const char *>(&frame._realtime_diff_nanosec), sizeof(frame._realtime_diff_nanosec));
    out.write(reinterpret_cast<const char *>(&frame._timeout_nanosec), sizeof(frame._timeout_nanosec));
    return out;
}

std::istream &operator>>(std::istream &in, WelcomeFrame &frame)
{
    in.read(reinterpret_cast<char *>(&frame._protocol_version), sizeof(frame._protocol_version));
    in.read(reinterpret_cast<char *>(&frame._realtime_diff_nanosec), sizeof(frame._realtime_diff_nanosec));
    in.read(reinterpret_cast<char *>(&frame._timeout_nanosec), sizeof(frame._timeout_nanosec));
    return in;
}

/// Class for storing data from \ref kespp "KESP-P" CONTROL_01 frame.
class ControlFrame
{
private:
    int64_t _chunk_nanosec;
    uint8_t _stop_flag; // as bool

public:
    /// Mandatory buffer size.
    static constexpr uint8_t kSize = 9;

    /// Instantiates ControlFrame object with passed values.
    ///
    /// @param chunk_nanosec timepoint (in nanoseconds) according to monotonic clock
    /// @param stop_flag in fact integer casted to boolean - 0 is false, any other value is true but always use 1
    ControlFrame(const int64_t chunk_nanosec = 0, const uint8_t stop_flag = 1)
        : _chunk_nanosec(chunk_nanosec), _stop_flag(stop_flag)
    {
    }

    /// Setter for chunk_nanosec field
    int64_t &chunk_nanosec() { return _chunk_nanosec; }

    /// Getter for chunk_nanosec field
    const int64_t &chunk_nanosec() const { return _chunk_nanosec; }
    
    /// Setter for stop_flag field
    uint8_t &stop_flag() { return _stop_flag; }

    /// Getter for stop_flag field
    const uint8_t &stop_flag() const { return _stop_flag; }

    /// Sets default values to it's fields.
    void clear()
    {
        _chunk_nanosec = 0;
        _stop_flag = 1;
    }

    /// Used to deserialize data from received buffer.
    void deserialize(const uint8_t (&buf)[kSize])
    {
        _chunk_nanosec = *reinterpret_cast<const uint64_t *>(buf);
        _stop_flag = *reinterpret_cast<const uint8_t *>(buf + sizeof(_chunk_nanosec));
    }

    /// Used to serialize data from fields.
    ///
    /// Serialized data is returned to buffer passed as the argument,
    /// which must by an array of exactly ControlFrame::kSize size.
    void serialize(uint8_t (&buf)[kSize]) const
    {
        *reinterpret_cast<uint64_t *>(buf) = _chunk_nanosec;
        *reinterpret_cast<uint8_t *>(buf + sizeof(_chunk_nanosec)) = _stop_flag;
    }

    friend std::ostream &operator<<(std::ostream &out, const ControlFrame &frame);
    friend std::istream &operator>>(std::istream &in, ControlFrame &frame);
};

std::ostream &operator<<(std::ostream &out, const ControlFrame &frame)
{
    out.write(reinterpret_cast<const char *>(&frame._chunk_nanosec), sizeof(frame._chunk_nanosec));
    out.write(reinterpret_cast<const char *>(&frame._stop_flag), sizeof(frame._stop_flag));
    return out;
}

std::istream &operator>>(std::istream &in, ControlFrame &frame)
{
    in.read(reinterpret_cast<char *>(&frame._chunk_nanosec), sizeof(frame._chunk_nanosec));
    in.read(reinterpret_cast<char *>(&frame._stop_flag), sizeof(frame._stop_flag));
    return in;
}

#endif // __frametypes_hpp__
