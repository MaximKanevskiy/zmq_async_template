#include <iostream>
#include <thread>
#include <vector>
#include <zmq.hpp>

static int within(int range) 
{
    return rand() % range;
}

class ServerWorker 
{
public:
    ServerWorker(zmq::context_t& context) : _workerSocket(context, zmq::socket_type::dealer) 
    {
        _workerSocket.connect("inproc://backend");
    }

    void operator()() 
    {
        while (true) 
        {
            zmq::message_t identity;
            zmq::message_t request;

            _workerSocket.recv(identity);
            _workerSocket.recv(request);

            std::string reply_str = "Reply: " + request.to_string();
            zmq::message_t reply(reply_str.size());
            memcpy(reply.data(), reply_str.data(), reply_str.size());

            _workerSocket.send(identity, zmq::send_flags::sndmore);
            _workerSocket.send(reply, zmq::send_flags::none);
        }
    }

private:
    zmq::socket_t _workerSocket;
};

class Server 
{
public:
    Server() : _context(1), _frontendSocket(_context, zmq::socket_type::router), _backendSocket(_context, zmq::socket_type::dealer) 
    {
        _frontendSocket.bind("tcp://*:5555");
        _backendSocket.bind("inproc://backend");
    }

    void Run(const uint16_t MAX_THREADS) 
    {
        std::vector<std::thread> workers;

        for (uint16_t i = 0; i < MAX_THREADS; i++)
        {
            workers.emplace_back(ServerWorker(_context));
        }

        zmq::proxy(_frontendSocket, _backendSocket);

        for (auto& worker : workers) 
        {
            worker.join();
        }
    }

private:
    zmq::context_t _context;
    zmq::socket_t _frontendSocket;
    zmq::socket_t _backendSocket;
};

int main(int argc, char** argv) 
{
    if (argc != 2)
    {
        std::cerr << "Usage:\n\tserver <threads_count>" << std::endl;
        return 1;
    }

    srand(time(0));

    Server server;
    server.Run(atoi(argv[1]));
    return 0;
}