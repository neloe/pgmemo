/*!
 * \file pgmemo.cpp
 * \author Nathan Eloe
 * \brief Driver for PGMemo
 */

#include <iostream>
#include "pgmemo.h"
#include "pgmemo.pb.h"

int main(int argc, char* argv[])
{
  PGMemoRequest pgmr;
  bson::Document pgmconf = parse_config(argv[1]);
  pgmr.set_dbname("postgres");
  pgmr.set_query("select clock_timestamp();");
  pg_query(pgmr, pgmconf);
  std::cout << pgmr.result_json() << std::endl;
  return 0;
}