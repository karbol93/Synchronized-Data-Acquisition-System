#ifndef __KESPP_PY_HPP__
#define __KESPP_PY_HPP__

#include <pybind11/pybind11.h>
#include "../include/dead_tmp.hpp"

namespace py = pybind11;

PYBIND11_MODULE(lidar, m)
{
    using namespace pybind11::literals;
    py::class_<VLP16>(m, "VLP16")
        .def(
            py::init<const char *, const uint16_t>(),
            "address"_a,
            "port"_a
        )
        .def("get_data", &VLP16::get_data, py::call_guard<py::gil_scoped_release>())
        .def("pcap_file", &VLP16::pcap_file, "name"_a, py::call_guard<py::gil_scoped_release>())
        .def("close_file", &VLP16::close_file, py::call_guard<py::gil_scoped_release>())
        .def("save_data", &VLP16::save_data, py::call_guard<py::gil_scoped_release>())
        .def("visualize", &VLP16::visualize, py::call_guard<py::gil_scoped_release>())

        // .def("connected", &KESPPClient::connected, py::call_guard<py::gil_scoped_release>())
        // .def("realtime_diff_nanosec", &KESPPClient::realtime_diff_nanosec, py::call_guard<py::gil_scoped_release>())
        // .def("timeout_nanosec", &KESPPClient::timeout_nanosec, py::call_guard<py::gil_scoped_release>())
        // .def("error_msg", &KESPPClient::error_msg, py::call_guard<py::gil_scoped_release>())
        // .def("try_connect", &KESPPClient::try_connect, py::call_guard<py::gil_scoped_release>())
        // .def("wait_for_start_info", &KESPPClient::wait_for_start_info, py::call_guard<py::gil_scoped_release>())
        // .def("get_info", &KESPPClient::get_info, py::call_guard<py::gil_scoped_release>())
        // .def_static("now_nanosec", &KESPPClient::now_nanosec, py::call_guard<py::gil_scoped_release>())
        // .def("recording_timestamp", &KESPPClient::recording_timestamp, "chunk_nanosec"_a, py::call_guard<py::gil_scoped_release>())
    ;
    
    // py::class_<ControlFrame>(m, "ControlFrame")
    //     .def(
    //         py::init<const int64_t, const uint8_t &>(),
    //         "chunk_nanosec"_a = 0,
    //         "stop_flag"_a = 1
    //     )
    //     .def("chunk_nanosec", py::overload_cast<>(&ControlFrame::chunk_nanosec, py::const_), py::return_value_policy::copy)
    //     .def("stop_flag", py::overload_cast<>(&ControlFrame::stop_flag, py::const_), py::return_value_policy::copy)
    // ;
}

#endif //__KESPP_PY_HPP__
