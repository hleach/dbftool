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
 * File:   FieldMap.cpp
 * Author: Heath Leach
 * 
 * Created on August 15, 2015, 9:45 AM
 */

#include "FieldOptions.h"
#include <map>
#include <sstream>

using namespace std;

struct fieldRec {
    bool b94;
    bool show;
};

map<string, fieldRec> fieldMap;

FieldOptions::FieldOptions() {

}

FieldOptions::FieldOptions(string fields, string fields94, DBFActor &dbf) {
    this->open(fields, fields94, dbf);
}

void FieldOptions::open(string fields, string fields94, DBFActor &dbf) {

    for (uint i = 0; i <= dbf.getFieldCount(); i++) {
        string f = dbf.getField(i).fieldInfo.name;
        transform(f.begin(), f.end(), f.begin(), ::toupper);
        fieldMap[f].show = fields == "*" ? true : false;
        fieldMap[f].b94 = false;
    }

    if ((fields != "*") && (fields != "")) {
        stringstream strStream(fields);
        string token;

        while (getline(strStream, token, ',')) {
            transform(token.begin(), token.end(), token.begin(), ::toupper);
            if (fieldMap.count(token))
                fieldMap[token].show = true;
        }
    }

    if (fields94 != "") {
        stringstream strStream(fields94);
        string token;

        while (getline(strStream, token, ',')) {
            transform(token.begin(), token.end(), token.begin(), ::toupper);
            if (fieldMap.count(token))
                fieldMap[token].b94 = true;
        }
    }

}

bool FieldOptions::wants(string fieldName) {
    if (fieldMap.count(fieldName))
        return fieldMap[fieldName].show;
    else return false;
}

bool FieldOptions::wantsB94(string fieldName) {
    if (fieldMap.count(fieldName))
        return fieldMap[fieldName].b94;
    else return false;
}
