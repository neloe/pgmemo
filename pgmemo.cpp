/*
  Copyright (c) Nathan Eloe, 2014
  This file is part of pgmemo.

  pgmemo is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  pgmemo is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with pgmemo.  If not, see <http://www.gnu.org/licenses/>.
*/

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
  return 0;
}