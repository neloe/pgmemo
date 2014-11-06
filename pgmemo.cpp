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
  PGMemoRequest pgmr, pgmr1;
  bson::Document pgmconf = parse_config(argv[1]);
  pgmr.set_dbname("postgres");
  pgmr.set_query("select clock_timestamp();");
  memo_query(pgmr, pgmconf);
  std::cout << pgmr.result_json() << std::endl;
  pgmr1.set_dbname("postgres");
  pgmr1.set_query("select clock_timestamp();");
  memo_query(pgmr1, pgmconf);
  std::cout << pgmr1.result_json() << std::endl;
  return 0;
}