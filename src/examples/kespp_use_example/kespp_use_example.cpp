#include <iostream>
#include <chrono>

#include <kespp_client.hpp>

int main(int argc, char **argv)
{
    while (true)
    {
        KESPPClient my_kespp_client;
        for (uint8_t i=0; !my_kespp_client.try_connect(); i++)
        {
            std::cout << my_kespp_client.error_msg() << std::endl << "Retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
            if (i>=9)
            {
                std::cout << "Could not connect\n" << std::endl;
                return 1;
            }
        }
        std::cout << "Connected!\n" << std::endl;

        ControlFrame synchro_data;
        synchro_data = my_kespp_client.get_info();

        std::cout << "Realtime diff nanosec: " << my_kespp_client.realtime_diff_nanosec() << "\n\n";
        my_kespp_client.recording_timestamp(synchro_data.chunk_nanosec());

        while (true)
        {
            using std::cout, std::endl;
            using std::this_thread::sleep_for;

            // while (my_kespp_client.get_info().stop_flag())
            //     sleep_for(std::chrono::milliseconds(10));
            my_kespp_client.wait_for_start_info();

            synchro_data = my_kespp_client.get_info();
            cout << "Recording session started\n";
            cout << "Error msg: " << my_kespp_client.error_msg() << endl;
            cout << "Timestamp: " << my_kespp_client.recording_timestamp(synchro_data.chunk_nanosec()) << endl;

            while (true)
            {
                synchro_data = my_kespp_client.get_info();
                cout << "Chunk nanoseconds: " << synchro_data.chunk_nanosec()
                    << ", stop flag: " << (bool)synchro_data.stop_flag()
                    << ", timeout nanoseconds: " << (my_kespp_client.timeout_nanosec() + synchro_data.chunk_nanosec()) << endl;
                cout << "Current time: " << KESPPClient::now_nanosec() << endl;

                sleep_for(std::chrono::seconds(1));

                if (synchro_data.stop_flag() || !my_kespp_client.connected())
                    break;
            }

            cout << "Recording session finished\n";
            if (!my_kespp_client.connected())
            {
                cout << my_kespp_client.error_msg() << endl;
                break;
            }
        }
    }

    return 0;
}
