#ifndef FRAMESIMULATOR_PCAPFRAMESIMULATORPLUGIN_H
#define FRAMESIMULATOR_PCAPFRAMESIMULATORPLUGIN_H

#include <log4cxx/logger.h>
#include <log4cxx/basicconfigurator.h>
#include <log4cxx/propertyconfigurator.h>
#include <log4cxx/helpers/exception.h>
using namespace log4cxx;
using namespace log4cxx::helpers;

#include "FrameSimulatorPlugin.h"

#include <pcap.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <iostream>
#include <sstream>

namespace FrameSimulator {

    class pcapFrameSimulatorPlugin : public FrameSimulatorPlugin {

    public:

        pcapFrameSimulatorPlugin();
        ~pcapFrameSimulatorPlugin();

        virtual bool setup(const po::variables_map& vm);
        virtual void simulate();

        virtual void populate_options(po::options_description& config);

    protected:

        static void pkt_callback(u_char *user, const pcap_pkthdr *hdr, const u_char *buffer);

        virtual void prepare_packets(const struct pcap_pkthdr *header, const u_char *buffer) = 0;
        virtual void replay_packets() = 0;

        std::vector<struct sockaddr_in> m_addrs;

        pcap_t *m_handle;
        char errbuf[PCAP_ERRBUF_SIZE];

        int m_socket;

        boost::optional<int> packet_gap;
        boost::optional<float> drop_frac;
        boost::optional<std::vector<std::string> > drop_packets;

    private:

        /** Pointer to logger **/
        LoggerPtr logger_;

    };

}

#endif //FRAMESIMULATOR_PCAPFRAMESIMULATORPLUGIN_H
