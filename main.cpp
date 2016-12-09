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
 * File:   main.cpp
 * Author: Heath Leach
 *
 * Created on August 15, 2015, 7:47 AM
 */

#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <string>
#include "FieldOptions.h"

using namespace std;

string fields = "*";
string fileName = "";
string match = "";
string fields94 = "";
string indexFieldName = "";
bool doFieldDump = false;
bool doColumnDump = false;

DBFActor dbf;
FieldOptions fopt;

void do_help() {
    cout << "dbftool - select and dump values from a dbf." << endl;
    cout << endl;
    cout << "    -s <fields>   : Fields to display, comma separated. Defaults to all." << endl;
    cout << "    -f <file.dbf> : Name of DBF file." << endl;
    cout << "    -9 <fields>   : Decode field as base 94, comma separated. Defaults to none." << endl;
    cout << "    -i <name>     : Index field name. Add an index field to the output." << endl;
    cout << "    -d            : Dump fields and exit." << endl;
    cout << "    -c            : Output fields spaced by field length." << endl;
    cout << endl;
    exit(1);
}

void setup(int argc, char* argv[]) {
    if (argc == 1)
        do_help();

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];
        if (arg == "-s") {
            i++;
            if (i < argc)
                fields = argv[i];
            else do_help();
        } else
            if (arg == "-f") {
            i++;
            if (i < argc)
                fileName = argv[i];
            else do_help();
        } else
            if (arg == "-9") {
            i++;
            if (i < argc)
                fields94 = argv[i];
            else do_help();
        } else
            if (arg == "-i") {
            i++;
            if (i < argc)
                indexFieldName = argv[i];
            else do_help();
        } else
            if (arg == "-d") {
            doFieldDump = true;
        } else
            if (arg == "-c") {
            doColumnDump = true;
        } else
            if (arg == "-w") {
            i++;
            if (i < argc)
                match = argv[i];
            else do_help();
        } else
            do_help();

    }

    dbf.open(fileName);

    if (dbf.getStatus().error != dbf.STATUS_READY) {
        cout << "Could not open " << fileName << "." << endl;
        exit(dbf.getStatus().syserror);
    }

    fopt.open(fields, fields94, dbf);
}

string decodeB94(string b94) {
    typedef unsigned long long uintXL;

    const uintXL MAX_UINTXL = numeric_limits<uintXL>::max();
    uintXL count = 0;

    for (uint i = 0; i < b94.length(); i++) {
        uintXL val = (b94[i] - 33);
        if (val > 93)
            return "INVALID";
        if (count > (MAX_UINTXL / 94))
            return "OVERFLOW";
        count *= 94;
        if ((MAX_UINTXL - count) < val)
            return "OVERFLOW";
        count += val;
    }

    return to_string(count);
}

void field_dump() {
    for (uint i = 1; i <= dbf.getFieldCount(); i++) {
        cout << dbf.getField(i).fieldInfo.name << "[";
        uint length = dbf.getField(i).fieldInfo.length;
        cout << length << "]" << endl;
    }
    exit(0);
}

void column_dump() {
    DBFRecord rec = dbf.getRecord();

    if (indexFieldName != "")
        cout << left << setw(indexFieldName.length() + 1) << indexFieldName;

    for (uint i = 1; i <= dbf.getFieldCount(); i++) {
        string f = dbf.getField(i).fieldInfo.name;
        transform(f.begin(), f.end(), f.begin(), ::toupper);
        if (fopt.wants(f)) {
            uint length = dbf.getField(i).fieldInfo.length + 1;
            if (length < 12)
                length = 12;
            if (fopt.wantsB94(f))
                length = 21;
            cout << left << setw(length) << dbf.getField(i).fieldInfo.name;
        }
    }

    cout << endl;

    unsigned long long count = 1;

    while (dbf.getStatus().error == dbf.STATUS_READY) {
        if (indexFieldName != "") {
            cout << left << setw(indexFieldName.length() + 1) << count;
            count++;
        }

        for (uint i = 1; i <= dbf.getFieldCount(); i++) {
            string f = dbf.getField(i).fieldInfo.name;
            uint length = dbf.getField(i).fieldInfo.length + 1;
            if (length < 12)
                length = 12;
            transform(f.begin(), f.end(), f.begin(), ::toupper);
            if (fopt.wants(f)) {
                string v = rec.get(dbf.getField(i).fieldInfo.name);
                v.erase(0, v.find_first_not_of(" "));
                v.erase(v.find_last_not_of(" ") + 1);

                if (fopt.wantsB94(f))
                    cout << left << setw(21) << decodeB94(v);
                else cout << left << setw(length) << v;

            }
        }

        rec = dbf.getRecord();
        cout << endl;
    }
}

void delim_dump(string delim) {
    DBFRecord rec = dbf.getRecord();
    bool first_field = true;

    if (indexFieldName != "")
        cout << indexFieldName << delim;

    for (uint i = 1; i <= dbf.getFieldCount(); i++) {
        string f = dbf.getField(i).fieldInfo.name;
        transform(f.begin(), f.end(), f.begin(), ::toupper);
        if (fopt.wants(f)) {
            if (first_field)
                first_field = false;
            else cout << delim;
            cout << dbf.getField(i).fieldInfo.name;
        }
    }

    cout << endl;

    unsigned long long count = 1;

    while (dbf.getStatus().error == dbf.STATUS_READY) {
        first_field = true;
        if (indexFieldName != "") {
            cout << count << delim;
            count++;
        }
        for (uint i = 1; i <= dbf.getFieldCount(); i++) {
            string f = dbf.getField(i).fieldInfo.name;
            transform(f.begin(), f.end(), f.begin(), ::toupper);
            if (fopt.wants(f)) {
                if (first_field)
                    first_field = false;
                else cout << delim;

                string v = rec.get(dbf.getField(i).fieldInfo.name);
                v.erase(0, v.find_first_not_of(" "));
                v.erase(v.find_last_not_of(" ") + 1);

                bool has_delim = v.find(delim) != string::npos;

                if (has_delim && (v.find("\"") != string::npos)) {
                    for (uint i = 0; i < v.length(); i++) {
                        if (v[i] == '\"') {
                            v.insert(i, "\"");
                            i++;
                        }
                    }
                }

                if (has_delim)
                    cout << "\"";

                if (fopt.wantsB94(f))
                    cout << decodeB94(v);
                else cout << v;

                if (has_delim)
                    cout << "\"";
            }
        }

        rec = dbf.getRecord();
        cout << endl;
    }

}

int main(int argc, char* argv[]) {
    setup(argc, argv);
    if (doFieldDump)
        field_dump();
    else if (doColumnDump)
        column_dump();
    else delim_dump(",");
    return 0;
}


