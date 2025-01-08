/*
    PWD
        pwd=/home/user/Desktop/myTest
    
    Compile command: 
        g++ -I/usr/local/include/opencv4 -I/usr/local/include/opencv4/opencv2 -L/usr/local/lib/ dead.cpp -o dead -lpcap -lboost_system -lopencv_viz -lopencv_core

*/


#include <pcap.h>
#include <iostream>
#include <time.h>
#include <queue>
#include <vector>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2/viz.hpp>
#include <byteswap.h>


struct VizuLaser
{
    double azimuth;
    double vertical;
    float distance;
    unsigned char intensity;
    unsigned char id;
    long long time;

    const bool operator < ( const struct VizuLaser& laser ){
        if( azimuth == laser.azimuth ){
            return id < laser.id;
        }
        else{
            return azimuth < laser.azimuth;
        }
    }
};

class VLP16
{
    public:
        // Boost socket configuration
        boost::asio::io_service ioservice;
        boost::asio::ip::udp::socket* socket = nullptr;
        boost::asio::ip::address address;
        unsigned short port;
        boost::asio::ip::udp::endpoint sender;
        
        // Pcap data config
        // Length of pcap package
        const unsigned int package_size = 1248;
        // Header (42) for .PCAP file. IP source, dest, type ... + Data (1206)
        unsigned char pcap_data[1248] = {
            0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x08, 0x00, 0x45, 0x00, /*  ..............E. */
            0x04, 0xd2, 0x00, 0x00, 0x40, 0x00, 0xff, 0x11, 0xff, 0xff, 0xc0, 0xa8, 0x00, 0xc8, 0xff, 0xff, /*  ....@........... */
            0xff, 0xff, 0x09, 0x40, 0x09, 0x40, 0x04, 0xbe, 0x00, 0x00                                      /* .. */
        }; 

        // Init pcap dummy device
        pcap_t *handle = nullptr;
        pcap_dumper_t *dumper = nullptr;

        int lasers_number = 16;
        // Laser angle 
        std::vector<double> laser_angle = { -10.0, 0.67, -8.67, 2.00, -7.33, 3.33, -6.00, 4.67, 
                                            -4.67, 6.00, -3.33, 7.33, -2.0, 8.67, -0.67, 10.0 };

        double last_azimuth = 0.0;
        std::queue<std::vector<VizuLaser>> queue;
        std::vector<VizuLaser> lasers;
        cv::viz::Viz3d viewer;

        void visualize();
        void init_velodyne_config();
        void pcap_file(std::string name);
        void save_data();
        size_t get_data();
        void close_file();
        void parseDataPackage(unsigned char* pcap_data, std::vector<VizuLaser>& lasers, double& last_azimuth);

        VLP16( std::string _address, unsigned short _port )
        {
            this->address = boost::asio::ip::address::from_string( _address );
            this->port = _port;
            try{
                this->socket = new boost::asio::ip::udp::socket( ioservice, boost::asio::ip::udp::endpoint( address, port ) );
            }
            catch( ... ){
                delete this->socket;
                std::cout << "Can't create socket...\n";
                this->socket = new boost::asio::ip::udp::socket( this->ioservice, boost::asio::ip::udp::endpoint( boost::asio::ip::address_v4::any(), port ) );
            }
            try{
                this->ioservice.run();
            }
            catch( const std::exception& e ){
                std::cerr << e.what() << std::endl;
            }
            // pcap handler init      
            this->handle = pcap_open_dead(DLT_EN10MB, 1 << 16);

            this->viewer.registerKeyboardCallback([]( const cv::viz::KeyboardEvent& event, void* cookie )
            {
                // Close Viewer
                if( event.code == 'q' && event.action == cv::viz::KeyboardEvent::Action::KEY_DOWN ){
                    static_cast<cv::viz::Viz3d*>( cookie )->close();
                }
            }, &viewer);
        }
};

void VLP16::pcap_file(std::string name = "example.pcap")
{
    this->dumper = pcap_dump_open(this->handle, name.c_str());
}

void VLP16::save_data()
{
    // Save to .pcap file
    pcap_pkthdr* pcap_hdr = new pcap_pkthdr();
    pcap_hdr->caplen = 1248;
    // Set timestamp of frame (since 1970)
    gettimeofday(&pcap_hdr->ts, NULL);
    pcap_hdr->len = pcap_hdr->caplen;
    pcap_dump((u_char *)this->dumper, pcap_hdr, this->pcap_data);
} 

size_t VLP16::get_data()
{
    boost::system::error_code error;
    // Hardcoded 1206 -> size of data in packet
    size_t length = this->socket->receive_from( boost::asio::buffer( this->pcap_data+42, 1206 ), this->sender, 1, error );
}

void VLP16::close_file()
{
    pcap_dump_close(this->dumper);
}

void VLP16::parseDataPackage(unsigned char* pcap_data, std::vector<VizuLaser>& lasers, double& last_azimuth)
{
    struct Laser
    {
        uint16_t distance;
        uint8_t intensity;
    };
    
    struct FiringData
    {
        uint16_t blockIdentifier;
        uint16_t rotationalPosition;
        Laser laser[32];
    };

    struct PacketData
    {
        FiringData firingData[12];
        uint32_t gpsTimestamp;
        uint8_t mode;
        uint8_t sensorType;
    };

    // parsing start
    pcap_data = pcap_data + 42;
    PacketData* packetData = new PacketData();
    for( int i = 0; i < 12; i++)
    {
        packetData->firingData[i].blockIdentifier = (pcap_data[100*i+1] << 8) + pcap_data[100*i];
        packetData->firingData[i].rotationalPosition = (pcap_data[100*i+3] << 8) + pcap_data[100*i+2];
        for( int j = 0; j < 32; j++)
        {
            packetData->firingData[i].laser[j].distance = (pcap_data[(100*i)+(j*3+5)] << 8) + pcap_data[(100*i)+(j*3+4)];
            packetData->firingData[i].laser[j].intensity = pcap_data[(100*i)+(j*3+6)];
        }
    }
    // parsing end

    const std::chrono::time_point<std::chrono::system_clock> now = std::chrono::system_clock::now();
    const std::chrono::microseconds epoch = std::chrono::duration_cast<std::chrono::microseconds>( now.time_since_epoch() );
    const long long unixtime = epoch.count();

    // Azimuth delta is the angle from one firing sequence to the next one
    double azimuth_delta = 0.0;
    if( packetData->firingData[1].rotationalPosition < packetData->firingData[0].rotationalPosition ){
        azimuth_delta = ( ( packetData->firingData[1].rotationalPosition + 36000 ) - packetData->firingData[0].rotationalPosition );
    }
    else{
        azimuth_delta = ( packetData->firingData[1].rotationalPosition - packetData->firingData[0].rotationalPosition );
    }

    //Point processing
    for( int firing_index = 0; firing_index < 12; firing_index++)
    {
        // std::cout << "Loop index: " << firing_index << '\n';
        FiringData firing_data = packetData->firingData[firing_index];
        for( int laser_index = 0; laser_index < 32; laser_index++)
        {
            // std::cout << "Loop laser: " << firing_index << '\n';
            double azimuth = static_cast<double>(firing_data.rotationalPosition);
            double laser_relative_time = laser_index *  2.304 + 18.432*(laser_index / 16);
            azimuth += azimuth_delta * laser_relative_time / 110.592;
            // Reset Rotation Azimuth
            if( azimuth >= 36000 )
            {
                azimuth -= 36000;
            }
            if( last_azimuth > azimuth )
            {
                // Push One Rotation Data to Queue
                // mutex.lock();
                VLP16::queue.push( std::move( lasers ) );
                
                // mutex.unlock();
                lasers.clear();
            }
            
            VizuLaser laser;
            laser.azimuth = azimuth / 100.0f;
            laser.vertical = laser_angle[laser_index % 16];

            laser.distance = static_cast<float>(firing_data.laser[laser_index].distance)*2.0f / 10.0f;
            laser.intensity = firing_data.laser[laser_index].intensity;
            laser.id = static_cast<unsigned char>(laser_index % 16);
            laser.time = unixtime + static_cast<long long>(laser_relative_time);
            lasers.push_back(laser);
            last_azimuth = azimuth;
        }
    }
}

void VLP16::visualize()
{
    this->parseDataPackage(this->pcap_data, this->lasers, this->last_azimuth);
    std::vector<VizuLaser> lasers;
    if(!this->queue.empty())
    {
        // std::cout << "queue " << this->queue.size() << '\n';
        lasers = std::move (this->queue.front());
        this->queue.pop();
    }
    if(!lasers.empty())
    {
        std::vector<cv::Vec3f> buffer( lasers.size() );
        for( const VizuLaser& laser : lasers )
        {
            const double distance = static_cast<double>( laser.distance );
            const double azimuth  = laser.azimuth  * CV_PI / 180.0;
            const double vertical = laser.vertical * CV_PI / 180.0;

            float x = static_cast<float>( ( distance * std::cos( vertical ) ) * std::sin( azimuth ) );
            float y = static_cast<float>( ( distance * std::cos( vertical ) ) * std::cos( azimuth ) );
            float z = static_cast<float>( ( distance * std::sin( vertical ) ) );

            if( x == 0.0f && y == 0.0f && z == 0.0f ){
                x = std::numeric_limits<float>::quiet_NaN();
                y = std::numeric_limits<float>::quiet_NaN();
                z = std::numeric_limits<float>::quiet_NaN();
            }

            buffer.push_back( cv::Vec3f( x, y, z ) );
        }

        // Create Widget
        cv::Mat cloudMat = cv::Mat( static_cast<int>( buffer.size() ), 1, CV_32FC3, &buffer[0] );
        cv::viz::WCloud cloud( cloudMat );

        // Show Point Cloud
        viewer.showWidget( "Cloud", cloud );
        viewer.spinOnce(); 
    }
}
