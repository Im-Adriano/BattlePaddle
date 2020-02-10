#include "BPHelper.hpp"

using namespace PacketParse;
using namespace std;

int actionResponse(unique_ptr<info_t> eventInfo){
    switch (eventInfo->bpHeader.header_type){
        case 0x02:
            //execute command then respond
            return 1;
        case 0x04:
            //keep alive for future use
            return 1;
        default:
            return -1;
    }
}

int requestAction(){

}