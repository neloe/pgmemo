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
  return 0;
}