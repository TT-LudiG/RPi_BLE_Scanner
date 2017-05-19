#include "stdafx.h"

#include <fstream>
#include <sstream>

#include "MySQLController.h"

#include <iostream>

MySQLController::MySQLController()
{
    _sqlDriverPtr = nullptr;
    _sqlConnectionPtr = nullptr;
    _sqlStatementPtr = nullptr;
    _sqlResultsPtr = nullptr;

    try
    {
        _sqlDriverPtr = get_driver_instance();

        _sqlConnectionPtr = _sqlDriverPtr->connect("tcp://www.thermotrack.co.za:3306", "LudiG1601", "jankeandbird4LIFE!");

        _sqlConnectionPtr->setSchema("db_sigfox");

        _sqlStatementPtr = _sqlConnectionPtr->createStatement();
    }

    catch (const sql::SQLException& e)
    {
        std::cout << "# ERR: SQLException in " << __FILE__;
        std::cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << std::endl;
        std::cout << "# ERR: " << e.what();
        std::cout << " (MySQL error code: " << e.getErrorCode();
        std::cout << ", SQLState: " << e.getSQLState() << " )" << std::endl;
    }
}

MySQLController::~MySQLController()
{
    if (_sqlResultsPtr != nullptr)
        delete _sqlResultsPtr;

    if (_sqlStatementPtr != nullptr)
        delete _sqlStatementPtr;

    if (_sqlConnectionPtr != nullptr)
        delete _sqlConnectionPtr;
}

void MySQLController::insertValuesRaw(const std::string id, const unsigned long int timestamp, const std::string data, const std::string station, const float latitude, const float longitude, const float noiseAverage, const float rssi, const unsigned long int sequenceNo)
{
    std::stringstream sqlQueryStream;

    sqlQueryStream << "INSERT INTO `db_sigfox`.`raw_data`"
                   << "("
                   << "`raw_data`.`idevice_id`,"
                   << "`raw_data`.`dtlogtime`,"
                   << "`raw_data`.`szdata`,"
                   << "`raw_data`.`szstation`,"
                   << "`raw_data`.`dlat`,"
                   << "`raw_data`.`dlong`,"
                   << "`raw_data`.`isignal_noise_avg`,"
                   << "`raw_data`.`irssi_signal`,"
                   << "`raw_data`.`iseq_number`"
                   << ") "
                   << "VALUES "
                   << "('" << id << "',FROM_UNIXTIME(" << timestamp << "),'" << data << "','" << station << "'," << latitude << "," << longitude << "," << noiseAverage << "," << rssi << "," << sequenceNo << ")";

    try
    {
        _sqlResultsPtr = _sqlStatementPtr->executeQuery(sqlQueryStream.str());
    }

    catch (const sql::SQLException& e)
    {
        // Do nothing.
    }
}