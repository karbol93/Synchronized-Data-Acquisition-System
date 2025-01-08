#ifndef __KESPP_PY_HPP__
#define __KESPP_PY_HPP__

#include <pybind11/pybind11.h>
#include <kespp_client.hpp>
#include <frametypes.hpp>

namespace py = pybind11;

PYBIND11_MODULE(kespp, m)
{
    using namespace pybind11::literals;
    py::class_<KESPPClient>(m, "KESPPClient")
        .def(
            py::init<const char *, const uint16_t>(),
            "hostname"_a = KESPPClient::kDefaultHost,
            "port"_a = KESPPClient::kDefaultPort
        )
        .def("connected", &KESPPClient::connected, py::call_guard<py::gil_scoped_release>())
        .def("realtime_diff_nanosec", &KESPPClient::realtime_diff_nanosec, py::call_guard<py::gil_scoped_release>())
        .def("timeout_nanosec", &KESPPClient::timeout_nanosec, py::call_guard<py::gil_scoped_release>())
        .def("error_msg", &KESPPClient::error_msg, py::call_guard<py::gil_scoped_release>())
        .def("try_connect", &KESPPClient::try_connect, py::call_guard<py::gil_scoped_release>())
        .def("wait_for_start_info", &KESPPClient::wait_for_start_info, py::call_guard<py::gil_scoped_release>())
        .def("get_info", &KESPPClient::get_info, py::call_guard<py::gil_scoped_release>())
        .def_static("now_nanosec", &KESPPClient::now_nanosec, py::call_guard<py::gil_scoped_release>())
        .def("recording_timestamp", &KESPPClient::recording_timestamp, "chunk_nanosec"_a, py::call_guard<py::gil_scoped_release>())
    ;
    
    py::class_<ControlFrame>(m, "ControlFrame")
        .def(
            py::init<const int64_t, const uint8_t &>(),
            "chunk_nanosec"_a = 0,
            "stop_flag"_a = 1
        )
        .def("chunk_nanosec", py::overload_cast<>(&ControlFrame::chunk_nanosec, py::const_), py::return_value_policy::copy)
        .def("stop_flag", py::overload_cast<>(&ControlFrame::stop_flag, py::const_), py::return_value_policy::copy)
    ;
}

#endif //__KESPP_PY_HPP__
