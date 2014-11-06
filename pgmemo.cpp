/*!
 * \file pgmemo.cpp
 * \author Nathan Eloe
 * \brief Driver for PGMemo
 */

#include <iostream>
#include "pgmemo.h"
#include "pgmemo.pb.h"
#include "zmq/zmqcpp.h"

int main(int argc, char* argv[])
{
  PGMemoRequest pgmr;
  zmqcpp::Socket socket(ZMQ_REP);
  bson::Document pgmconf = parse_config(argv[1]);
  zmqcpp::Message msg;
  std::string reply;
  if (pgmconf.field_names().count("zmq_bind"))
    socket.bind(pgmconf["zmq_bind"].data<std::string>());
  else if (pgmconf.field_names().count("zmq_connect"))
    socket.connect(pgmconf["zmq_connect"].data<std::string>());
  else
  {
    std::cerr << "No valid zmq_connect or zmq_bind configuration directive found; bailing" << std::endl;
    exit(1);
  }
  
  while (true)
  {
    socket.recv(msg);
    pgmr.ParseFromString(msg.last());
    msg.clear();
    memo_query(pgmr, pgmconf);
    pgmr.SerializeToString(&reply);
    socket.send(zmqcpp::Message(reply));
    if(pgmr.cached() && pgmr.refresh())
      update_cache(pgmr, pgmconf);
  }
  /*
  pgmr.set_dbname("postgres");
  pgmr.set_query("select clock_timestamp();");
  std::cout << pgmr.cached() << std::endl;
  memo_query(pgmr, pgmconf);
  std::cout << pgmr.result_json() << " " << pgmr.cached() << std::endl;
  pgmr1.set_dbname("postgres");
  pgmr1.set_query("select clock_timestamp();");
  pgmr1.set_refresh(true);
  std::cout << pgmr1.cached() << std::endl;
  memo_query(pgmr1, pgmconf);
  std::cout << pgmr1.result_json() << " " << pgmr1.cached() << std::endl;
  if (pgmr1.cached() && pgmr1.refresh())
    update_cache(pgmr1, pgmconf);
  std::cout << pgmr1.result_json() << " " << pgmr1.cached() << std::endl;
  */
  return 0;
}