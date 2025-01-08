from time import sleep
from kespp_python import kespp


def main():
    while True:
        my_kespp_client = kespp.KESPPClient()
        i = 0
        while not my_kespp_client.try_connect():
            print(my_kespp_client.error_msg())
            print('Retrying...')
            sleep(1)
            if i >= 9:
                print('Could not connect\n')
                exit(1)
            i += 1
        print('Connected!\n')

        # synchro_data = kespp_client.ControlFrame()
        synchro_data = my_kespp_client.get_info()

        print('Realtime diff nanosec:', my_kespp_client.realtime_diff_nanosec(), '\n')
        my_kespp_client.recording_timestamp(synchro_data.chunk_nanosec())
        my_kespp_client.now_nanosec()

        while True:
            my_kespp_client.wait_for_start_info()
            synchro_data = my_kespp_client.get_info()
            print('Recording session started')
            print('Error msg:', my_kespp_client.error_msg())
            print('Timestamp:', my_kespp_client.recording_timestamp(synchro_data.chunk_nanosec()))

            while True:
                synchro_data = my_kespp_client.get_info()
                print(
                    'Chunk nanoseconds: ', synchro_data.chunk_nanosec(),
                    ', stop flag: ', bool(synchro_data.stop_flag()),
                    ', timeout nanoseconds: ', my_kespp_client.timeout_nanosec() + synchro_data.chunk_nanosec(),
                    sep='',
                )
                print('Current time:', kespp.KESPPClient.now_nanosec())

                sleep(1)

                if synchro_data.stop_flag() or (not my_kespp_client.connected()):
                    break
            
            print('Recording session finished')
            if not my_kespp_client.connected():
                print(my_kespp_client.error_msg())
                break
    
    return


if __name__ == "__main__":
    main()
