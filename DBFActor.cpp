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
 * File:   DBFActor.cpp
 * Author: heath
 * 
 * Created on March 1, 2015, 6:13 PM
 */

#include <cerrno>
#include "DBFActor.h"

using namespace std;

#define DO_FAIL(err) setStatus(err, __LINE__, __FILE__)

DBFActor::DBFActor(string fileName, bool throwErrors) {
    this->throwErrors = throwErrors;
    open(fileName);
}

DBFActor::DBFActor(string fileName) {
    throwErrors = false;
    open(fileName);
}

DBFActor::DBFActor() {
    throwErrors = false;
    setStatus(STATUS_CLOSED, 0, "");
}

DBFActor::DBFActor(bool throwErrors) {
    this->throwErrors = throwErrors;
    setStatus(STATUS_CLOSED, 0, "");
}

void DBFActor::open(string fileName) {
    setStatus(STATUS_READY, 0, "");

    file.open(fileName.c_str(), ios::in | ios::binary | ios::out);
    file.read((char *) &header, sizeof (DBFHeader));

    if (file.fail()) {
        DO_FAIL(STATUS_FAILED_TO_OPEN);
        if (throwErrors)
            throw status;
        return;
    }

    bool loop = true;
    uint16_t offset = 0;
    uint16_t number = 0;

    while (loop) {
        DBFField field;
        file.read((char *) &(field.fieldInfo), sizeof (DBFFieldInfo));

        bool endOfFields = (field.fieldInfo.name[0] == HEADER_RECORD_TERMINATOR);

        if ((file.fail() && !file.eof()) || (file.eof() && !endOfFields)) {
            DO_FAIL(STATUS_FAILED_TO_READ);
            if (throwErrors)
                throw status;
            return;
        }

        if (endOfFields) {
            loop = false;
            fieldCount = number;
        } else {
            number++;
            field.fieldNumber = number;
            field.fieldOffset = offset;
            offset += field.fieldInfo.length;
            fields[field.fieldInfo.name] = field;
        }
    }

    reset();
}

void DBFActor::setStatus(int err, int line, const char *file) {
    status.error = err;
    status.syserror = errno;
    status.line = line;
    status.srcFile = file;
}

void DBFActor::reset() {
    if (status.error != STATUS_READY)
        return;

    file.clear();
    file.seekg(header.posFirstRecord + 1);

    if ((file.tellg() != streampos(header.posFirstRecord + 1)) || (file.fail())) {
        DO_FAIL(STATUS_FAILED_TO_SEEK);
        if (throwErrors)
            throw status;
        return;
    }

}

void DBFActor::seekRecord(uint32_t record) {
    if (status.error != STATUS_READY)
        return;

    uint64_t pos = header.posFirstRecord + 1 + (record * header.recordLength);

    file.clear();
    file.seekg(pos);

    if ((file.tellg() != streampos(pos)) || (file.fail())) {
        DO_FAIL(STATUS_FAILED_TO_SEEK);
        if (throwErrors)
            throw status;
        return;
    }

}

// Responsibility of the caller to make sure their buffer is >= record length

void DBFActor::readRawRecord(char *buf) {
    if (status.error != STATUS_READY)
        return;

    file.read(buf, header.recordLength);

    if (file.fail()) {
        DO_FAIL(STATUS_FAILED_TO_READ);
        if (throwErrors)
            throw status;
        return;
    }
}

DBFRecord DBFActor::getRecord() {
    RecordVec rvec(header.recordLength);
    readRawRecord(rvec.data());

    DBFRecord rec(fields, rvec);

    return rec;
}

DBFRecord DBFActor::getRecord(uint32_t record) {
    seekRecord(record);
    return getRecord();
}

DBFRecord DBFActor::operator[](uint32_t record) {
    return getRecord(record);
}

// Returns the number of records in the dbf

uint32_t DBFActor::length() {
    return header.numRecords;
}

// Returns the status of the DBFActor. This is the same data that gets
// thrown if throwErrors == true

DBFStatus DBFActor::getStatus() {
    return status;
}

// Closes the file and does any needed cleanup

void DBFActor::close() {
    setStatus(STATUS_CLOSED, 0, "");
    file.close();
}

void DBFActor::writeRawRecord(char *buf) {
    if (status.error != STATUS_READY)
        return;

    file.write(buf, header.recordLength);

    if (file.fail()) {
        DO_FAIL(STATUS_FAILED_TO_WRITE);
        if (throwErrors)
            throw status;
        return;
    }
}

void DBFActor::writeRecord(DBFRecord record) {
    writeRawRecord(record.record.data());
}

void DBFActor::writeRecord(uint32_t recnum, DBFRecord record) {
    seekRecord(recnum);
    writeRecord(record);
}

// Returns the field record for field fieldName or returns
// an empty field with number 0 if not in a ready state

DBFField DBFActor::getField(string fieldName) {
    if (status.error == STATUS_READY) {
        return fields[fieldName];
    }

    DBFField nullField;
    nullField.fieldNumber = 0;
    return nullField;
}

// Returns the field record for field # fieldNumber
// Field numbers start at 1 and go up.
// Returns an empty field with number 0 if it doesn't find the field in question
// or if the DBFActor is not in the ready state

DBFField DBFActor::getField(uint16_t fieldNumber) {
    if (status.error == STATUS_READY) {
        for (auto f : fields) {
            if (f.second.fieldNumber == fieldNumber)
                return f.second;
        }

    }

    DBFField nullField;
    nullField.fieldNumber = 0;
    return nullField;
}

uint16_t DBFActor::getFieldCount() {
    return fieldCount;
}
