#include <iostream>
#include <thread>
#include <zmq.hpp>
#include <sstream>

static int within(int range) 
{
    return rand() % range;
}

class Client 
{
public:
    Client() : _context(1), _clientSocket(_context, zmq::socket_type::dealer) 
    {
        _clientSocket.connect("tcp://localhost:5555");

        std::ostringstream oss;
        oss << std::hex << within(0x10000) << "-" << within(0x10000);
        std::string identity = oss.str();
        _clientSocket.set(zmq::sockopt::routing_id, identity);
    }

    void Run()
    {
        int request_num = 0;

        while (true) 
        {
            std::string request_str;
            std::ostringstream oss;
            oss << "Request #" << request_num++;
            request_str = oss.str();

            zmq::message_t request(request_str.size());
            std::memcpy(request.data(), request_str.data(), request_str.size());

            _clientSocket.send(request, zmq::send_flags::none);

            zmq::message_t reply;
            _clientSocket.recv(reply);

            std::cout << "Received: " << reply.to_string() << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

private:
    zmq::context_t _context;
    zmq::socket_t _clientSocket;
};

int main() 
{
    srand(time(0));

    Client client;
    client.Run();

    return 0;
}