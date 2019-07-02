//
// Created by link9 on 02.07.19.
//

#include <iostream>
#include <capnzero/CapnZero.h>
#include <zmq.h>

#include "capnzerowrapper.h"

int main(int argc, char** argv) {

    void* ctx = zmq_ctx_new();
    void* socket = zmq_socket(ctx, ZMQ_RADIO);
    zmq_connect(socket, "udp://224.0.0.1");
    sendMessage(socket, capnzero::CommType::UDP, "MCGroup", "Hello World man!");

    std::cout << "sent \"Hello World man!\"" << std::endl;
    return 0;
}