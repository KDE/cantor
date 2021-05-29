/*
    SPDX-License-Identifier: GPL-2.0-or-later
    SPDX-FileCopyrightText: 2015 Minh Ngo <minh@fedoraproject.org>
*/

#include <iostream>
#include <csignal>
#include <vector>
#include <cstring>

#include "pythonserver.h"

using namespace std;

const char messageEnd = 29;
const char recordSep = 30;
const char unitSep = 31;

PythonServer server;
bool isInterrupted = false;

string LOGIN("login");
string EXIT("exit");
string CODE("code");
string FILEPATH("setFilePath");
string MODEL("model");

void signal_handler(int signal)
{
    if (signal == SIGINT)
    {
        isInterrupted = true;
        server.interrupt();
    }
}

vector<string> split(string s, char delimiter)
{
    vector<string> results;

    size_t pos = 0;
    std::string token;
    while ((pos = s.find(delimiter)) != std::string::npos) {
        token = s.substr(0, pos);
        results.push_back(token);
        s.erase(0, pos + 1);
    }
    results.push_back(s);

    return results;
}

int main()
{
    std::signal(SIGINT, signal_handler);

    std::cout << "ready" << std::endl;

    std::string input;
    while (getline(std::cin, input, messageEnd))
    {
        const vector<string>& records = split(input, recordSep);

        if (records.size() == 2)
        {
            if (records[0] == EXIT)
            {
                //Exit from cycle and finish program
                break;
            }
            else if (records[0] == LOGIN)
            {
                server.login();
            }
            if (records[0] == FILEPATH)
            {
                vector<string> args = split(records[1], unitSep);
                if (args.size() == 2)
                    server.setFilePath(args[0], args[1]);
            }
            else if (records[0] == CODE)
            {
                server.runPythonCommand(records[1]);

                if (!isInterrupted)
                {
                    const string& result =
                        server.getOutput()
                        + unitSep
                        + server.getError()
                        + unitSep
                        + to_string((int)server.isError())
                        + messageEnd;

                    std::cout << result.c_str();
                }
                else
                {
                    // No replay when interrupted
                    isInterrupted = false;
                }
            }
            else if (records[0] == MODEL)
            {
                bool ok, val;
                try {
                    val = (bool)stoi(records[1]);
                    ok = true;
                } catch (std::invalid_argument e) {
                    ok = false;
                };

                string result;
                if (ok)
                    result = server.variables(val) + unitSep;
                else
                    result = unitSep + string("Invalid argument for 'model' command");
                result += messageEnd;

                std::cout << result.c_str();
            }
            std::cout.flush();
        }
    }

    return 0;
}
