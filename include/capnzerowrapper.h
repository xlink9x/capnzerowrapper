#pragma once

#include <capnzero/CapnZero.h>

extern "C" {

int sendMessage(void *socket, capnzero::CommType commType, const char *c_topic, const char *c_message) {
    std::string message(c_message);
    std::string topic(c_topic);

    // init builder
    ::capnp::MallocMessageBuilder msgBuilder;
    capnzero::String::Builder beaconMsgBuilder = msgBuilder.initRoot<capnzero::String>();

    // set content
    beaconMsgBuilder.setString(message);

    // setup zmq msg
    zmq_msg_t msg;

    kj::Array<capnp::word> wordArray = capnp::messageToFlatArray(msgBuilder);
    kj::Array<capnp::word> *wordArrayPtr = new kj::Array<capnp::word>(kj::mv(wordArray)); // will be delete by zero-mq
    capnzero::check(zmq_msg_init_data(&msg, wordArrayPtr->begin(), wordArrayPtr->size() * sizeof(capnp::word),
                                      &capnzero::Publisher::cleanUpMsgData,
                                      wordArrayPtr), "zmq_msg_init_data");

    // set group
    if (commType == capnzero::CommType::UDP) {
//        std::cout << "Publisher: Sending on Group '" << topic << "'" << std::endl;
        capnzero::check(zmq_msg_set_group(&msg, topic.c_str()), "zmq_msg_set_group");
    }

    // send
    if (commType != capnzero::CommType::UDP) {
        zmq_msg_t topicMsg;
        zmq_msg_init_data(&topicMsg, &topic, topic.size() * sizeof(topic), &capnzero::Publisher::cleanUpMsgData, NULL);
        zmq_msg_send(&topicMsg, socket, ZMQ_SNDMORE);
    }
    int numBytesSend = zmq_msg_send(&msg, socket, 0);
    if (numBytesSend == -1) {
        std::cerr << "zmq_msg_send was unsuccessful: Errno " << errno << " means: " << zmq_strerror(errno) << std::endl;
        capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");
    }

    return numBytesSend;
}

const char *receiveSerializedMessage(void *socket, capnzero::CommType type) {
    zmq_msg_t msg;
    capnzero::check(zmq_msg_init(&msg), "zmq_msg_init");
    if (type != capnzero::CommType::UDP) {
        zmq_msg_t topic;
        capnzero::check(zmq_msg_init(&topic), "zmq_msg_init");
        zmq_msg_recv(&topic, socket, 0);
    }
    int nbytes = zmq_msg_recv(&msg, socket, 0);

    //        std::cout << "Subscriber::receive(): nBytes: " << nbytes << " errno: " << errno << "(EAGAIN: " << EAGAIN << ")" << std::endl;

    // handling for unsuccessful call to zmq_msg_recv
    if (nbytes == -1) {
        if (errno != EAGAIN) // receiving a message was unsuccessful
        {
            std::cerr << "Subscriber::receive(): zmq_msg_recv received no bytes! " << errno << " - zmq_strerror(errno)"
                      << std::endl;
        }
        capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");
    }

    // Received message must contain an integral number of words.
    if (zmq_msg_size(&msg) % capnzero::Subscriber::wordSize != 0) {
        std::cout << "Non-Integral number of words!" << std::endl;
        capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");
    }

    // Check whether message is memory aligned
    assert(reinterpret_cast<uintptr_t>(zmq_msg_data(&msg)) % capnzero::Subscriber::wordSize == 0);

    int numWordsInMsg = zmq_msg_size(&msg);
    auto wordArray = kj::ArrayPtr<capnp::word const>(reinterpret_cast<capnp::word const *>(zmq_msg_data(&msg)),
                                                     numWordsInMsg);

    ::capnp::FlatArrayMessageReader msgReader = ::capnp::FlatArrayMessageReader(wordArray);

    capnzero::check(zmq_msg_close(&msg), "zmq_msg_close");

    return msgReader.getRoot<capnzero::String>().toString().flatten().cStr();
}
}
