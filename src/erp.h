/*
 * Project ID: 111
 * Authors: Toshi Sinha (MT26198), Pallavi Gupta (MT26182)
 * Course: Object-Oriented Programming and Design (OOPD)
 * Project: Mini-ERP (AcadERP-Lite)
 */

#ifndef ERP_H
#define ERP_H

#include <iostream>
#include <string>

const int MAX_STUDENTS = 100;
const int MAX_COURSES = 50;
const int MAX_REGS = 40;

enum ErrorCode {
    OK = 0,
    E_PARSE,
    E_FULL,
    E_DUPLICATE,
    E_UNKNOWN_STUDENT,
    E_UNKNOWN_COURSE,
    E_BAD_COURSE,
    E_BAD_GRADE,
    E_GRADE_TYPE,
    E_NOT_REGISTERED,
    E_ALREADY_GRADED,
    E_THESIS_STATE,
    E_ALREADY_PASSED,
    E_ALREADY_REGISTERED,
    E_PREREQ,
    E_CREDIT_LIMIT,
    E_THESIS_NOT_ALLOWED,
    E_NO_GPA
};

enum CourseType { REGULAR, SU, THESIS };
enum ThesisState { NOT_THESIS, REGISTERED, IN_PROGRESS, FINAL, FAILED };

struct ProgramConfig {
    int totalCredits;
    int electiveCredits;
    int minGpaHundredths;
    int maxLoad;
    int thesisCredits;
    ProgramConfig();
};

class Course {
public:
    std::string code;
    CourseType type;
    int credits;
    bool isCore;
    int prereq;
    Course();
};

class Registration {
public:
    int course;
    int term;
    std::string grade;
    ThesisState thesis;
    Registration();
};

class Student {
public:
    std::string roll;
    std::string name;
    Registration* regs;
    int numRegs;
    Student();
    ~Student();
};

class ERPSystem {
private:
    ProgramConfig config;
    Course* courses;
    int numCourses;
    Student* students;
    int numStudents;

    int findCourse(const std::string& code) const;
    int findStudent(const std::string& roll) const;
    int getLatestGradedIndex(int s, int c) const;
    bool isCoursePassed(int s, int c) const;
    int earnedCredits(int s) const;
    int termLoad(int s, int term) const;

public:
    ERPSystem();
    ~ERPSystem();

    ErrorCode setProgram(const std::string tokens[], int count);
    ErrorCode addCourse(const std::string tokens[], int count);
    ErrorCode addStudent(const std::string tokens[], int count);
    ErrorCode registerCourse(const std::string tokens[], int count);
    ErrorCode submitGrade(const std::string tokens[], int count);
    ErrorCode convertThesis(const std::string tokens[], int count);

    ErrorCode gpaHundredths(int s, int* result) const;
    int checkEligibility(int s, int c, int term) const;
    static int gradePoints(const std::string& symbol);
    static const char* errorToString(ErrorCode err);

    ErrorCode queryGpa(const std::string tokens[], int count) const;
    ErrorCode queryAvg(const std::string tokens[], int count) const;
    ErrorCode queryGrad(const std::string tokens[], int count) const;
    ErrorCode queryEligible(const std::string tokens[], int count) const;

    // Bonus commands
    ErrorCode queryTranscript(const std::string tokens[], int count) const;
    ErrorCode saveState(const std::string tokens[], int count) const;
    ErrorCode loadState(const std::string tokens[], int count);
};

int splitTokens(const std::string& line, std::string tokens[], int maxTokens);
bool parseNonNegInt(const std::string& s, int* out);
bool parseMinGpa(const std::string& s, int* outHundredths);

#endif
