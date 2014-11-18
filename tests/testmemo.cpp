#include "gtest/gtest.h"
#include "pgmemo.pb.h"
#include <string>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <zmq.hpp>
const char PORT[] = "tcp://localhost:5555";

//
/// \brief serializes the pbuff to a string, and sends it over the socket_t
/// \pre pbuff must have the SerializeToString(std::string*) method implemented
/// \post serializes the pbuff to a string, and sends it over the socket_t
/// \returns the zmq::socket_t to allow for chaining
//
template <class T>
zmq::socket_t& operator << (zmq::socket_t & sock, const T& pbuff)
{
  zmq::message_t msg(pbuff.ByteSize());
  pbuff.SerializeToArray(msg.data(), pbuff.GetCachedSize());
  sock.send(msg);
  return sock;
}

//
/// \brief deserializes the pbuff from the socket_t
/// \pre pbuff must have the ParseFromArray(void*, int) method implemented
/// \post parses the response from the socket and stores it in the pbuff
/// \returns the zmq::socket_t to allow for chaining
//
template <class T>
zmq::socket_t& operator >> (zmq::socket_t & sock, T& pbuff)
{
  zmq::message_t msg;
  sock.recv(&msg);
  pbuff.ParseFromArray(msg.data(), msg.size());
  return sock;
}

const int ITERATIONS = 10;

TEST(DeviceInsertion, SameDevSameID)
{
  zmq::context_t context(1);
  zmq::socket_t socket (context, ZMQ_REQ);
  socket.connect(PORT);
  PGMemoRequest input;
  PGMemoRequest output;
  std::string res;
  input.set_query("select clock_timestamp();");
  socket << input;
  socket >> output;
  res = output.result_json();
  //not checking for cached; might be from a previous run
  for (int i=0; i<ITERATIONS; i++)
  {
    socket << input;
    socket >> output;
    ASSERT_EQ(res, output.result_json());
    ASSERT_TRUE(output.cached());
  }
  input.set_refresh(true);
  socket << input;
  socket >> output;
  ASSERT_EQ(res, output.result_json()); // refresh will not happen until the next set_query
  sleep(1);
  input.set_refresh(false);
  socket << input;
  socket >> output;
  ASSERT_NE(res, output.result_json());
  res = output.result_json();
  for (int i=0; i<ITERATIONS; i++)
  {
    socket << input;
    socket >> output;
    ASSERT_EQ(res, output.result_json());
    ASSERT_TRUE(output.cached());
  }
}
