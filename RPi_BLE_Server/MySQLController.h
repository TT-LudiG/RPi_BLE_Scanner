#ifndef MYSQLCONTROLLER_H
#define MYSQLCONTROLLER_H

#include "mysql_connection.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

class MySQLController
{
private:
    sql::Driver* _sqlDriverPtr;
    sql::Connection* _sqlConnectionPtr;
    sql::Statement* _sqlStatementPtr;
    sql::ResultSet* _sqlResultsPtr;

public:
    MySQLController(void);
    ~MySQLController(void);

    void insertValuesRaw(const std::string id, const unsigned long int timestamp, const std::string data, const std::string station, const float latitude, const float longitude, const float noiseAverage, const float rssi, const unsigned long int sequenceNo);
};
#endif