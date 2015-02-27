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
  else if (rows.size())
    pgmr.set_result_json(static_cast<std::string>(rows[0]));
  else
    pgmr.set_result_json("{}");
}

void memo_query(PGMemoRequest& pgmr, const bson::Document & conf)
{
  std::string redissrv;
  long redisprt;
  conf["redishost"].data(redissrv);
  conf["redisport"].data(redisprt);
  redisContext *c = redisConnect(redissrv.c_str(), (int)redisprt);
  redisReply *reply;
  reply = (redisReply *)redisCommand(c, "GET %s", _h(pgmr.query()).c_str());
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
  lock querylock(pgmr.query(), redissrv, redisprt);
  if (querylock.locked())
  {
    try
    {
      pg_query(pgmr, conf);
      redisCommand(c, "SET %s %s", _h(pgmr.query()).c_str(), pgmr.result_json().c_str());
    }
    catch (...)
    {
      throw;
    }
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
  if (reply)
    freeReplyObject(reply);
  redisCommand(c, "SET %s %s", _lockstr(query).c_str(), "true");
  return true;
}