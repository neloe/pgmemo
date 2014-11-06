/*!
 * \file pgmemo_util.cpp
 * \author Nathan Eloe
 * \brief Utility function implementation of utility functions for pgmemo_util
 */

#include "pgmemo.h"

bson::Document parse_config(const std::string & fname)
{
  std::ifstream fin(fname.c_str());
  bson::JSON_Loader loader;
  bson::Document confd(loader.parse(fin));
  
  return confd;
}

std::string connstr(const bson::Document & conf, const std::string & dbn)
{
  std::stringstream ss;
  ss << "dbname=" << dbn;
  for (const std::string & s: conf.field_names())
    ss << " " << s << "=" << conf[s].data<std::string>();
  return ss.str();
}

std::shared_ptr<pqxx::connection>& getconn(const bson::Document & conf, const std::string & dbn)
{
  static std::map<std::string, std::shared_ptr<pqxx::connection>> connpool;
  if (!connpool.count(dbn))
    connpool[dbn] = std::make_shared<pqxx::connection>(connstr(conf, dbn).c_str());
  return connpool[dbn];
}

void pg_query(PGMemoRequest & pgmr, const bson::Document & conf)
{
  pqxx::work txn(*(getconn(conf, pgmr.dbname())));
  pqxx::result r = txn.exec(pgmr.query().c_str());
  std::vector<bson::Element> rows;
  bson::Document res;
  for (auto tuple: r)
  {
    bson::Document row;
    for (pqxx::tuple::size_type i=0; i < r.columns(); i++)
    {
      if (!tuple[i].is_null())
      {
	row.add(r.column_name(i), tuple[i].c_str());
      }
    }
    if (row.size())
      rows.push_back(bson::Element(row));
  }
  if (rows.size() > 1)
  {
    res.add("rows", rows);
    pgmr.set_result_json(static_cast<std::string>(bson::Element(res)));
  }
  else
    pgmr.set_result_json(static_cast<std::string>(rows[0]));
}