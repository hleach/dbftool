//
// The MIT License
//
// Copyright (c) 2015 Heath Leach
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

/* 
 * File:   DBFActor.h
 * Author: heath
 *
 * Created on March 1, 2015, 6:13 PM
 */

#ifndef DBFACTOR_H
#define	DBFACTOR_H

#include <fstream>
#include <map>
#include <string>
#include <vector>
#include <stdint.h>

struct __attribute__((__packed__)) DBFHeader {
    uint8_t fileType; // DBF Type
    uint8_t lastUpdated[3]; // Date of last update; in YYMMDD format.
    uint32_t numRecords; // Number of records in file
    uint16_t posFirstRecord; // Position of the first record
    uint16_t recordLength; // Length of each record
    uint8_t reservedBytes[20]; // Reserved for other uses
};

struct __attribute__((__packed__)) DBFFieldInfo {
    char name[11]; // Field name
    char type; // Field Type
    uint32_t fieldDisplacement; // Position of field in record (not reliable, sometimes 0)
    uint8_t length; // Field length in bytes
    uint8_t decimalCount; // Number of decimal places
    uint8_t fieldFlags; // Field flags
    uint32_t autoincNext; // Value of next auto increment
    uint8_t autoincStep; // Step size of auto increment
    uint8_t reserved[8]; // Reserved
};

struct DBFField {
    DBFFieldInfo fieldInfo; // Field information from the dbf
    uint16_t fieldOffset; // Calculated offset of field into the raw record data
    uint16_t fieldNumber; // The position of the field in the header/record data
};

struct DBFStatus {
    int error; // Error number as defined vy STATUS_ constants
    int syserror; // Last system error
    int line; // Line the error happened on
    std::string srcFile; // Source file the error happened in
};

typedef std::map<std::string, DBFField> FieldMap;
typedef std::vector<char> RecordVec;

class DBFRecord {
    friend class DBFActor;
public:

    class FieldProxy {
        friend std::ostream& operator<<(std::ostream &out, const DBFRecord::FieldProxy &fp);
    private:
        DBFRecord *parent;
        std::string name;
    public:
        FieldProxy(DBFRecord *parent_rec, std::string fieldName);
        std::string operator=(std::string value);
    private:
        std::string get() const;
    };

private:
    RecordVec record;
    FieldMap fields;
public:
    DBFRecord(FieldMap fmap, RecordVec rvec);
    DBFRecord(const DBFRecord& orig);
    std::string operator()(std::string fieldName);
    FieldProxy operator[](std::string fieldName);
    std::string get(std::string fieldName);
    void set(std::string fieldName, std::string value);
};

class DBFActor {
public:
    static const int STATUS_CLOSED = -1;
    static const int STATUS_READY = 0;
    static const int STATUS_FAILED_TO_OPEN = 1;
    static const int STATUS_FAILED_TO_READ = 2;
    static const int STATUS_FAILED_TO_SEEK = 3;
    static const int STATUS_FAILED_TO_WRITE = 4;
private:
    std::fstream file; // File Stream
    DBFHeader header; // DBF file header information
    FieldMap fields; // Holds information on the field structure
    DBFStatus status; // Holds status and error information
    bool throwErrors; // Throw errors if true
    uint16_t fieldCount; // Number of fields in file
    static const unsigned char HEADER_RECORD_TERMINATOR = 0x0d;
public:
    DBFActor(std::string fileName, bool throwErrors);
    DBFActor(std::string fileName);
    DBFActor(bool throwErrors);
    DBFActor();
    void open(std::string fileName);
    void close();
    void reset();
    void seekRecord(uint32_t record);
    void readRawRecord(char *buf);
    DBFRecord getRecord();
    DBFRecord getRecord(uint32_t record);
    DBFRecord operator[](uint32_t record);
    uint32_t length();
    DBFStatus getStatus();
    void writeRawRecord(char *buf);
    void writeRecord(DBFRecord record);
    void writeRecord(uint32_t recnum, DBFRecord record);
    DBFField getField(std::string fieldName);
    DBFField getField(uint16_t fieldNumber);
    uint16_t getFieldCount();
private:
    void setStatus(int error, int line, const char *file);
};

std::ostream& operator<<(std::ostream &out, const DBFRecord::FieldProxy &fp);

#endif	/* DBFACTOR_H */
