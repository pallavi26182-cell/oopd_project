/*
 * Project ID: 111
 * Authors: Toshi Sinha (MT26198), Pallavi Gupta (MT26182)
 * Course: Object-Oriented Programming and Design (OOPD)
 * Project: Mini-ERP (AcadERP-Lite)
 */

#include "erp.h"
#include <cstdio>
#include <cctype>

int main() {
    ERPSystem erp;
    std::string line;
    int lineNum = 0;
    bool hasCommand = false;

    while (std::getline(std::cin, line)) {
        lineNum++;
        if (!line.empty() && line[line.length() - 1] == '\r') {
            line.erase(line.length() - 1);
        }

        int i = 0;
        while (i < (int)line.length() && std::isspace((unsigned char)line[i])) {
            i++;
        }
        if (i == (int)line.length() || line[i] == '#') {
            continue;
        }

        std::string tokens[8];
        int count = splitTokens(line, tokens, 8);
        if (count == 0) continue;

        ErrorCode err = OK;
        if (count > 7) {
            err = E_PARSE;
        } else if (tokens[0] == "PROGRAM") {
            if (hasCommand) err = E_PARSE;
            else err = erp.setProgram(tokens, count);
        } else if (tokens[0] == "COURSE") {
            err = erp.addCourse(tokens, count);
        } else if (tokens[0] == "STUDENT") {
            err = erp.addStudent(tokens, count);
        } else if (tokens[0] == "REGISTER") {
            err = erp.registerCourse(tokens, count);
        } else if (tokens[0] == "GRADE") {
            err = erp.submitGrade(tokens, count);
        } else if (tokens[0] == "CONVERT") {
            err = erp.convertThesis(tokens, count);
        } else if (tokens[0] == "GPA") {
            err = erp.queryGpa(tokens, count);
        } else if (tokens[0] == "AVG") {
            err = erp.queryAvg(tokens, count);
        } else if (tokens[0] == "GRAD") {
            err = erp.queryGrad(tokens, count);
        } else if (tokens[0] == "ELIGIBLE") {
            err = erp.queryEligible(tokens, count);
        } else {
            err = E_PARSE;
        }

        hasCommand = true;

        if (err != OK) {
            std::printf("ERROR %d %s\n", lineNum, ERPSystem::errorToString(err));
        }
    }

    return 0;
}
