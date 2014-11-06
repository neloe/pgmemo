/*!
 * \file pgmemo.h
 * \author Nathan Eloe
 * \brief function defs for pgmemo
 */

#ifndef PGMEMO_H
#define PGMEMO_H

#include <fstream>
#include <hiredis/hiredis.h>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <pqxx/pqxx>
#include <vector>
#include "bson/json/jsonloader.h"
#include "pgmemo.pb.h"

/*!
 * \brief parses the configuration file specified, setting defaults if something not specified
 * \pre file contains JSON configuration parameters
 * \post Connects to file specified by filename
 * \returns the configuration
 */
bson::Document parse_config (const std::string & fname);

/*!
 * \brief builds the connection string based on the config and m_dbname
 * \pre None
 * \post None
 * \returns the connection string
 */

std::string connstr(const bson::Document & c, const std::string & dbn);

/*!
 * \brief Gets a connection based on the supplied configuration and database name
 * \pre None
 * \post the configuration is set to the connection in the pool
 * \post if a connection does not exist in the pool to the dbname, creates a new one
 */
std::shared_ptr< pqxx::connection >& getconn(const bson::Document& conf, const std::string& dbn);

/*!
 * \brief checks to see if the query has been cached, returns the value
 * \pre pgmr does not have a chached flag or a result_json value
 * \post Connects to database if a refresh is requested or the query has not been cached
 * \post Modifies the passed in protobuf to contain a json string with the query result
 */
void memo_query(PGMemoRequest& pgmr, const bson::Document & conf);

/*!
 * \brief connects to the postgres database and performs the query
 * \pre None
 * \post Connects to the database, performs query
 */
void pg_query(PGMemoRequest& pgmr, const bson::Document & conf); 

/*!
 * \brief refreshes the cached result
 * \pre None
 * \post the postgres box is hit, cache is updated
 */
void update_cache(PGMemoRequest& pgmr, const bson::Document & conf); 

#endif