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
  //set some sane defaults?
  return confd;
}

std::string connstr(const bson::Document & conf )
{
  std::stringstream ss;
  for (const std::string & s: conf.field_names())
    if (s.find("redis") == std::string::npos && s.find("zmq") == std::string::npos)
      ss << s << "=" << conf[s].data<std::string>() << " ";
  return ss.str();
}

std::shared_ptr<pqxx::connection>& getconn(const bson::Document & conf )
{
  static std::shared_ptr<pqxx::connection> conn = std::make_shared<pqxx::connection>(connstr(conf).c_str());
  return conn;
}

void pg_query(PGMemoRequest & pgmr, const bson::Document & conf)
{
  pqxx::work txn(*(getconn(conf)));
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

void memo_query(PGMemoRequest& pgmr, const bson::Document & conf)
{
  std::string redissrv;
  long redisprt;
  conf["redishost"].data(redissrv);
  conf["redisport"].data(redisprt);
  redisContext *c = redisConnect(redissrv.c_str(), (int)redisprt);
  redisReply *reply;
  reply = (redisReply *)redisCommand(c, "GET %s", pgmr.query().c_str());
  if (reply && reply->type == REDIS_REPLY_STRING)
  {
    pgmr.set_result_json(std::string(reply->str, reply->len));
    pgmr.set_cached(true);
  }
  else
  {
    update_cache(pgmr, conf);
  }
  if (reply)
    freeReplyObject(reply);
  if (c)
    redisFree(c);
  return;
}

void update_cache(PGMemoRequest& pgmr, const bson::Document & conf)
{
  std::string redissrv;
  long redisprt;
  conf["redishost"].data(redissrv);
  conf["redisport"].data(redisprt);
  redisContext *c = redisConnect(redissrv.c_str(), (int)redisprt);
  if (!pgmr.cached() && acquire_lock(pgmr.query(), c))
  {
    pg_query(pgmr, conf);
    redisCommand(c, "SET %s %s", pgmr.query().c_str(), pgmr.result_json().c_str());
    release_lock(pgmr.query(), c);
  }
  if (c)
    redisFree(c);
  return;
}

bool acquire_lock(const std::string & query, redisContext * c)
{
  redisReply *reply;
  reply = (redisReply *)redisCommand(c, "GET %s", _lockstr(query).c_str());
  if (reply && reply->type == REDIS_REPLY_STRING)
    return false;
  redisCommand(c, "SET %s %s", _lockstr(query).c_str(), "true");
  return true;
}

void release_lock(const std::string & query, redisContext * c )
{
  redisCommand(c, "DEL %s", _lockstr(query).c_str());
  return;
}