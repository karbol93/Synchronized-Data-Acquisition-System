#ifndef __PERIODIC_ACQUISITION_ENGINE_HPP
#define __PERIODIC_ACQUISITION_ENGINE_HPP

#include <string>

#include "../../kespp/include/kespp_client.hpp"


template <class T>
class KESPPPeriodicSensorBase
{
public:
    virtual std::string get_sensor_type() = 0;
    virtual std::string get_sensor_id() = 0;
    virtual std::string get_file_extension() = 0;
    virtual void init_sensor() = 0;
    virtual void stop_sensor() = 0;
    virtual void start_buffering() = 0;
    virtual void stop_buffering() = 0;
    virtual T get_sample() = 0;
    virtual T interpolate_sample(T succeeding_sample) = 0;
    virtual double sampling_period_s() = 0;
    virtual std::string open_file(std::string full_path) = 0;
    virtual void close_file() = 0;
    virtual void write_sample_to_file(self, sample: T) = 0;
};


class PeriodicAcquisitionEngine
{
private:
    static constexpr double NANO = 1e-9;
    KESPPClient _client = KESPPClient();
};

#endif //__PERIODIC_ACQUISITION_ENGINE_HPP
