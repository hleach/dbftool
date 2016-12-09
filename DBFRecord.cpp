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
 * File:   DBFRecord.cpp
 * Author: heath
 * 
 * Created on March 8, 2015, 8:31 AM
 */

#include "DBFActor.h"

using namespace std;

DBFRecord::FieldProxy::FieldProxy(DBFRecord *parent_rec, std::string fieldName) {
    this->name = fieldName;
    this->parent = parent_rec;
}

string DBFRecord::FieldProxy::operator=(string value) {
    parent->set(this->name, value);
    return parent->get(this->name);
}

ostream& operator<<(ostream &out, const DBFRecord::FieldProxy &fp) {
    string s = fp.get();
    out << s;
    return out;
}

string DBFRecord::FieldProxy::get() const {
    return parent->get(name);
}

DBFRecord::DBFRecord(FieldMap fmap, RecordVec rvec) {
    this->fields = fmap;
    this->record = rvec;
}

DBFRecord::DBFRecord(const DBFRecord& orig) {
    this->fields = orig.fields;
    this->record = orig.record;
}

string DBFRecord::operator()(string fieldName) {
    return DBFRecord::get(fieldName);
}

DBFRecord::FieldProxy DBFRecord::operator[](std::string fieldName) {
    DBFRecord::FieldProxy proxy(this, fieldName);
    return proxy;
}

string DBFRecord::get(string fieldName) {
    string data(record.data() + fields[fieldName].fieldOffset, fields[fieldName].fieldInfo.length);
    return data;
}

void DBFRecord::set(string fieldName, string value) {
    int offset = fields[fieldName].fieldOffset;
    int length = fields[fieldName].fieldInfo.length > value.size() ?
            value.size() : fields[fieldName].fieldInfo.length;

    for (int i = 0; i < fields[fieldName].fieldInfo.length; i++) {
        if (i < length)
            record[offset + i] = value[i];
        else
            record[offset + i] = 0x20;
    }
}