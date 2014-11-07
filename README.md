# pgmemo
This package strives to be a way to memoize compilcated postgres queries in redis assuming one knows the output from postgresql will not change (frequently).

## Configuration

Configuration of the pgmemo server is done through a configuration file; this file is expressed in the JSON  format.  Allowable keys in the JSON file are any expressed in the [libpq Connection String](http://www.postgresql.org/docs/9.3/static/libpq-connect.html) Parameter keywords.  Values must be expressed as strings.

There are three other directives that are required in the configuration: `redishost`, `redisport`, and either `zmq_bind` or `zmq_connect`.  `redisport` is the only value not represented as a string in the json configuration; it is an integer.  `redishost/port` are the host and port that the redis server is listening on.  

If you specify `zmq_bind`, the server will bind the zmq socket to the specified string.  This is useful when you desire the pgmemo server to be the "non-moving part" in your architecture.  When specifying `zmq_connect`, the socket is connected to the specified port.  This is useful for systems where you want to have multiple pgmemo workers connecting to a central router (perhaps through MDP, etc).

Sample configuration file:
```json
{
  "host": "8.8.8.8",
  "user" : "my_db_role",
  "password" : "my_password",
  "port" : "1234",
  "redishost" : "127.0.0.1",
  "redisport" : 6379,
  "zmq_bind" : "tcp://*:5555"
}

```

## Building
This uses a cmake based build system; from the source directory: `mkdir build; cd build; cmake ..; make`

## Querying
pgmemo uses a protocol buffer to perform it's communication.  The required fields are the query field and the dbname field.  If you want to force a refresh of the cache (after sending the cached value), you may set the refresh field to true.  Example (C++):
```cpp
  PGMemoRequest input;
  PGMemoRequest output;
  input.set_dbname("postgres");
  input.set_query("select clock_timestamp();");
  //send the input PGMemoRequest here
  //recieve into the output PGMemoRequest here
  std::cout << output.get_result_json() << std::endl; //JSON string of output
  //force a refresh
  input.set_refresh(true);
  //send the input PGMemoRequest here
  //recieve into the output PGMemoRequest here
  std::cout << output.get_result_json() << std::endl; //should be same as last result
  input.set_refresh(false);
  //send the input PGMemoRequest here
  //recieve into the output PGMemoRequest here
  std::cout << output.get_result_json() << std::endl; // Different result; refresh has been forced
```
## Output
If the query returns a single row, it is returned as a json string of a single document with col_name:value values.  If there are multiple rows returned, the result is a json dictionary containing `{rows: [{<row 1>}, {<row 2>},...]}`.

## Prerequisites
This program requires the hiredis C bindings for redis and libpqxx, a protocol buffer library, as well as the [FastBSON-Cpp](github.com/neloe/FastBSON-Cpp) and [EasyZMQ-Cpp](github.com/neloe/EasyZMQ-Cpp) library.  The prerequisites on a Debian/Ubuntu system may be installed with
```bash
sudo apt-get install libpqxx-dev libhiredis-dev libprotobuf-dev
```