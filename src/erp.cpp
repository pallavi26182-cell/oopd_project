/*
 * Project ID: 111
 * Authors: Toshi Sinha (MT26198), Pallavi Gupta (MT26182)
 * Course: Object-Oriented Programming and Design (OOPD)
 * Project: Mini-ERP (AcadERP-Lite)
 */

#include "erp.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
#include <fstream>

ProgramConfig::ProgramConfig()
    : totalCredits(160), electiveCredits(24), minGpaHundredths(500),
      maxLoad(24), thesisCredits(100) {}

Course::Course() : code(""), type(REGULAR), credits(0), isCore(true), prereq(-1) {}

Registration::Registration() : course(-1), term(0), grade(""), thesis(NOT_THESIS) {}

Student::Student() : roll(""), name(""), regs(NULL), numRegs(0) {
    regs = new Registration[MAX_REGS];
}

Student::~Student() {
    delete[] regs;
    regs = NULL;
}

ERPSystem::ERPSystem() : courses(NULL), numCourses(0), students(NULL), numStudents(0) {
    courses = new Course[MAX_COURSES];
    students = new Student[MAX_STUDENTS];
}

ERPSystem::~ERPSystem() {
    delete[] courses;
    delete[] students;
    courses = NULL;
    students = NULL;
}

int splitTokens(const std::string& line, std::string tokens[], int maxTokens) {
    int count = 0;
    int i = 0;
    int n = (int)line.length();

    while (i < n) {
        while (i < n && std::isspace((unsigned char)line[i])) {
            i++;
        }

        if (i >= n) break;

        int start = i;

        while (i < n && !std::isspace((unsigned char)line[i])) {
            i++;
        }

        if (count < maxTokens) {
            tokens[count] = line.substr(start, i - start);
        }

        count++;
    }

    return count;
}

bool parseNonNegInt(const std::string& s, int* out) {
    if (s.empty() || s.length() > 9) return false;

    int val = 0;

    for (int i = 0; i < (int)s.length(); i++) {
        if (!std::isdigit((unsigned char)s[i])) return false;
        val = val * 10 + (s[i] - '0');
    }

    *out = val;
    return true;
}

bool parseMinGpa(const std::string& s, int* outHundredths) {
    int dotPos = -1;

    for (int i = 0; i < (int)s.length(); i++) {
        if (s[i] == '.') {
            if (dotPos != -1) return false;
            dotPos = i;
        } else if (!std::isdigit((unsigned char)s[i])) {
            return false;
        }
    }

    if (dotPos <= 0 || dotPos + 3 != (int)s.length()) return false;

    int whole = 0;

    if (!parseNonNegInt(s.substr(0, dotPos), &whole)) return false;

    int frac = (s[dotPos + 1] - '0') * 10 + (s[dotPos + 2] - '0');
    int total = whole * 100 + frac;

    if (total > 1000) return false;

    *outHundredths = total;
    return true;
}

int ERPSystem::gradePoints(const std::string& symbol) {
    if (symbol == "A")  return 10;
    if (symbol == "A-") return 9;
    if (symbol == "B")  return 8;
    if (symbol == "B-") return 7;
    if (symbol == "C")  return 6;
    if (symbol == "C-") return 5;
    if (symbol == "D")  return 4;
    if (symbol == "F")  return 0;

    return -1;
}

const char* ERPSystem::errorToString(ErrorCode err) {
    switch (err) {
        case OK: return "OK";
        case E_PARSE: return "E_PARSE";
        case E_FULL: return "E_FULL";
        case E_DUPLICATE: return "E_DUPLICATE";
        case E_UNKNOWN_STUDENT: return "E_UNKNOWN_STUDENT";
        case E_UNKNOWN_COURSE: return "E_UNKNOWN_COURSE";
        case E_BAD_COURSE: return "E_BAD_COURSE";
        case E_BAD_GRADE: return "E_BAD_GRADE";
        case E_GRADE_TYPE: return "E_GRADE_TYPE";
        case E_NOT_REGISTERED: return "E_NOT_REGISTERED";
        case E_ALREADY_GRADED: return "E_ALREADY_GRADED";
        case E_THESIS_STATE: return "E_THESIS_STATE";
        case E_ALREADY_PASSED: return "E_ALREADY_PASSED";
        case E_ALREADY_REGISTERED: return "E_ALREADY_REGISTERED";
        case E_PREREQ: return "E_PREREQ";
        case E_CREDIT_LIMIT: return "E_CREDIT_LIMIT";
        case E_THESIS_NOT_ALLOWED: return "E_THESIS_NOT_ALLOWED";
        case E_NO_GPA: return "E_NO_GPA";
    }

    return "E_PARSE";
}

int ERPSystem::findCourse(const std::string& code) const {
    for (int i = 0; i < numCourses; i++) {
        if (courses[i].code == code) return i;
    }

    return -1;
}

int ERPSystem::findStudent(const std::string& roll) const {
    for (int i = 0; i < numStudents; i++) {
        if (students[i].roll == roll) return i;
    }

    return -1;
}

int ERPSystem::getLatestGradedIndex(int s, int c) const {
    int latest = -1;

    for (int i = 0; i < students[s].numRegs; i++) {
        if (students[s].regs[i].course == c &&
            !students[s].regs[i].grade.empty()) {
            latest = i;
        }
    }

    return latest;
}

bool ERPSystem::isCoursePassed(int s, int c) const {
    int idx = getLatestGradedIndex(s, c);

    if (idx == -1) return false;

    const Registration& r = students[s].regs[idx];

    if (courses[c].type == REGULAR) {
        return gradePoints(r.grade) >= 4;
    } else if (courses[c].type == SU) {
        return r.grade == "S";
    } else {
        return r.thesis == FINAL && gradePoints(r.grade) >= 4;
    }
}

int ERPSystem::earnedCredits(int s) const {
    int sum = 0;

    for (int c = 0; c < numCourses; c++) {
        if (isCoursePassed(s, c)) {
            sum += courses[c].credits;
        }
    }

    return sum;
}

int ERPSystem::termLoad(int s, int term) const {
    int sum = 0;

    for (int i = 0; i < students[s].numRegs; i++) {
        if (students[s].regs[i].term == term) {
            sum += courses[students[s].regs[i].course].credits;
        }
    }

    return sum;
}

ErrorCode ERPSystem::setProgram(const std::string tokens[], int count) {
    if (count != 6) return E_PARSE;

    int tot, elec, mgpa, mload, tcred;

    if (!parseNonNegInt(tokens[1], &tot) || tot <= 0) return E_PARSE;
    if (!parseNonNegInt(tokens[2], &elec) || elec < 0 || elec > tot) return E_PARSE;
    if (!parseMinGpa(tokens[3], &mgpa)) return E_PARSE;
    if (!parseNonNegInt(tokens[4], &mload) || mload <= 0) return E_PARSE;
    if (!parseNonNegInt(tokens[5], &tcred) || tcred < 0) return E_PARSE;

    config.totalCredits = tot;
    config.electiveCredits = elec;
    config.minGpaHundredths = mgpa;
    config.maxLoad = mload;
    config.thesisCredits = tcred;

    return OK;
}

ErrorCode ERPSystem::addCourse(const std::string tokens[], int count) {
    if (count != 6) return E_PARSE;

    const std::string& code = tokens[1];

    if (code.empty() || code.length() > 10) return E_PARSE;

    CourseType ctype;

    if (tokens[2] == "REGULAR") ctype = REGULAR;
    else if (tokens[2] == "SU") ctype = SU;
    else if (tokens[2] == "THESIS") ctype = THESIS;
    else return E_PARSE;

    int cred = 0;

    if (!parseNonNegInt(tokens[3], &cred)) return E_PARSE;

    bool core = false;

    if (tokens[4] == "CORE") core = true;
    else if (tokens[4] == "ELECTIVE") core = false;
    else return E_PARSE;

    const std::string& prereqCode = tokens[5];

    if (prereqCode.empty() || prereqCode.length() > 10) return E_PARSE;

    if (findCourse(code) != -1) return E_DUPLICATE;
    if (cred > 24 || (cred == 0 && ctype != SU)) return E_BAD_COURSE;

    int prereqIdx = -1;

    if (prereqCode != "-") {
        if (prereqCode == code) return E_BAD_COURSE;

        prereqIdx = findCourse(prereqCode);

        if (prereqIdx == -1) return E_BAD_COURSE;
    }

    if (numCourses >= MAX_COURSES) return E_FULL;

    courses[numCourses].code = code;
    courses[numCourses].type = ctype;
    courses[numCourses].credits = cred;
    courses[numCourses].isCore = core;
    courses[numCourses].prereq = prereqIdx;

    numCourses++;

    return OK;
}

ErrorCode ERPSystem::addStudent(const std::string tokens[], int count) {
    if (count != 3) return E_PARSE;

    if (tokens[1].empty() || tokens[1].length() > 10) return E_PARSE;
    if (tokens[2].empty() || tokens[2].length() > 30) return E_PARSE;

    if (findStudent(tokens[1]) != -1) return E_DUPLICATE;
    if (numStudents >= MAX_STUDENTS) return E_FULL;

    students[numStudents].roll = tokens[1];
    students[numStudents].name = tokens[2];
    students[numStudents].numRegs = 0;

    numStudents++;

    return OK;
}

int ERPSystem::checkEligibility(int s, int c, int term) const {
    if (isCoursePassed(s, c)) return 2;

    for (int i = 0; i < students[s].numRegs; i++) {
        const Registration& r = students[s].regs[i];

        if (r.course == c) {
            if (courses[c].type == THESIS) {
                if (r.thesis == REGISTERED || r.thesis == IN_PROGRESS) return 3;
            } else {
                if (r.grade.empty()) return 3;
            }
        }
    }

    if (courses[c].prereq != -1 &&
        !isCoursePassed(s, courses[c].prereq)) {
        return 4;
    }

    if (termLoad(s, term) + courses[c].credits > config.maxLoad) {
        return 5;
    }

    if (courses[c].type == THESIS &&
        earnedCredits(s) < config.thesisCredits) {
        return 6;
    }

    return 0;
}

ErrorCode ERPSystem::registerCourse(const std::string tokens[], int count) {
    if (count != 4) return E_PARSE;

    if (tokens[1].length() > 10 || tokens[2].length() > 10) return E_PARSE;

    int term = 0;

    if (!parseNonNegInt(tokens[3], &term) || term < 1 || term > 12) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int c = findCourse(tokens[2]);

    if (c == -1) return E_UNKNOWN_COURSE;

    int check = checkEligibility(s, c, term);

    if (check == 2) return E_ALREADY_PASSED;
    if (check == 3) return E_ALREADY_REGISTERED;
    if (check == 4) return E_PREREQ;
    if (check == 5) return E_CREDIT_LIMIT;
    if (check == 6) return E_THESIS_NOT_ALLOWED;

    if (students[s].numRegs >= MAX_REGS) return E_FULL;

    int idx = students[s].numRegs++;

    students[s].regs[idx].course = c;
    students[s].regs[idx].term = term;
    students[s].regs[idx].grade = "";
    students[s].regs[idx].thesis =
        (courses[c].type == THESIS) ? REGISTERED : NOT_THESIS;

    return OK;
}

ErrorCode ERPSystem::submitGrade(const std::string tokens[], int count) {
    if (count != 5) return E_PARSE;

    if (tokens[1].length() > 10 || tokens[2].length() > 10) return E_PARSE;

    int term = 0;

    if (!parseNonNegInt(tokens[3], &term) || term < 1 || term > 12) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int c = findCourse(tokens[2]);

    if (c == -1) return E_UNKNOWN_COURSE;

    const std::string& sym = tokens[4];

    bool isLetter = (gradePoints(sym) != -1);
    bool isSX = (sym == "S" || sym == "X");

    if (!isLetter && !isSX) return E_BAD_GRADE;

    int regIdx = -1;

    for (int i = 0; i < students[s].numRegs; i++) {
        if (students[s].regs[i].course == c &&
            students[s].regs[i].term == term) {
            regIdx = i;
            break;
        }
    }

    if (regIdx == -1) return E_NOT_REGISTERED;

    Registration& r = students[s].regs[regIdx];

    if (!r.grade.empty()) return E_ALREADY_GRADED;

    if (courses[c].type == REGULAR) {
        if (!isLetter) return E_GRADE_TYPE;
        r.grade = sym;
    } else if (courses[c].type == SU) {
        if (!isSX) return E_GRADE_TYPE;
        r.grade = sym;
    } else {
        if (!isSX) return E_GRADE_TYPE;

        r.grade = sym;
        r.thesis = (sym == "S") ? IN_PROGRESS : FAILED;
    }

    return OK;
}

ErrorCode ERPSystem::convertThesis(const std::string tokens[], int count) {
    if (count != 4) return E_PARSE;

    if (tokens[1].length() > 10 || tokens[2].length() > 10) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int c = findCourse(tokens[2]);

    if (c == -1) return E_UNKNOWN_COURSE;

    const std::string& sym = tokens[3];

    bool isLetter = (gradePoints(sym) != -1);
    bool isSX = (sym == "S" || sym == "X");

    if (!isLetter && !isSX) return E_BAD_GRADE;
    if (courses[c].type != THESIS || !isLetter) return E_GRADE_TYPE;

    int latest = -1;

    for (int i = 0; i < students[s].numRegs; i++) {
        if (students[s].regs[i].course == c) latest = i;
    }

    if (latest == -1) return E_NOT_REGISTERED;

    Registration& r = students[s].regs[latest];

    if (r.thesis == FINAL) return E_ALREADY_GRADED;
    if (r.thesis != IN_PROGRESS) return E_THESIS_STATE;

    r.grade = sym;
    r.thesis = FINAL;

    return OK;
}

ErrorCode ERPSystem::gpaHundredths(int s, int* result) const {
    long long n = 0;
    long long d = 0;

    for (int c = 0; c < numCourses; c++) {
        int idx = getLatestGradedIndex(s, c);

        if (idx == -1) continue;

        const Registration& r = students[s].regs[idx];

        if (courses[c].type == REGULAR) {
            int pts = gradePoints(r.grade);

            n += (long long)courses[c].credits * pts;
            d += courses[c].credits;
        } else if (courses[c].type == THESIS && r.thesis == FINAL) {
            int pts = gradePoints(r.grade);

            n += (long long)courses[c].credits * pts;
            d += courses[c].credits;
        }
    }

    if (d == 0) return E_NO_GPA;

    *result = (int)((200 * n + d) / (2 * d));

    return OK;
}

ErrorCode ERPSystem::queryGpa(const std::string tokens[], int count) const {
    if (count != 2) return E_PARSE;
    if (tokens[1].length() > 10) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int h = 0;
    ErrorCode err = gpaHundredths(s, &h);

    if (err != OK) return err;

    std::printf("GPA %s %d.%02d\n",
                students[s].roll.c_str(), h / 100, h % 100);

    return OK;
}

ErrorCode ERPSystem::queryAvg(const std::string tokens[], int count) const {
    if (count != 2) return E_PARSE;
    if (tokens[1].length() > 10) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int h = 0;
    ErrorCode err = gpaHundredths(s, &h);

    if (err != OK) return err;

    const char* letter = "F";

    if (h >= 950) letter = "A";
    else if (h >= 850) letter = "A-";
    else if (h >= 750) letter = "B";
    else if (h >= 650) letter = "B-";
    else if (h >= 550) letter = "C";
    else if (h >= 450) letter = "C-";
    else if (h >= 200) letter = "D";

    std::printf("AVG %s %s\n", students[s].roll.c_str(), letter);

    return OK;
}

ErrorCode ERPSystem::queryGrad(const std::string tokens[], int count) const {
    if (count != 2) return E_PARSE;
    if (tokens[1].length() > 10) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    bool r1 = (earnedCredits(s) >= config.totalCredits);

    bool r2 = true;

    for (int c = 0; c < numCourses; c++) {
        if (courses[c].isCore && !isCoursePassed(s, c)) {
            r2 = false;
            break;
        }
    }

    int elecEarned = 0;

    for (int c = 0; c < numCourses; c++) {
        if (!courses[c].isCore && isCoursePassed(s, c)) {
            elecEarned += courses[c].credits;
        }
    }

    bool r3 = (elecEarned >= config.electiveCredits);

    bool r4 = false;

    for (int c = 0; c < numCourses; c++) {
        if (courses[c].type == THESIS) {
            int idx = getLatestGradedIndex(s, c);

            if (idx != -1 && students[s].regs[idx].thesis == FINAL &&
                gradePoints(students[s].regs[idx].grade) >= 6) {
                r4 = true;
                break;
            }
        }
    }

    int h = 0;

    bool r5 = (gpaHundredths(s, &h) == OK &&
               h >= config.minGpaHundredths);

    if (r1 && r2 && r3 && r4 && r5) {
        std::printf("GRAD %s YES\n", students[s].roll.c_str());
    } else {
        std::printf("GRAD %s NO ", students[s].roll.c_str());

        bool first = true;

        if (!r1) {
            std::printf("R1");
            first = false;
        }

        if (!r2) {
            std::printf("%sR2", first ? "" : ", ");
            first = false;
        }

        if (!r3) {
            std::printf("%sR3", first ? "" : ", ");
            first = false;
        }

        if (!r4) {
            std::printf("%sR4", first ? "" : ", ");
            first = false;
        }

        if (!r5) {
            std::printf("%sR5", first ? "" : ", ");
            first = false;
        }

        std::printf("\n");
    }

    return OK;
}

ErrorCode ERPSystem::queryEligible(const std::string tokens[], int count) const {
    if (count != 4) return E_PARSE;
    if (tokens[1].length() > 10 || tokens[2].length() > 10) return E_PARSE;

    int term = 0;

    if (!parseNonNegInt(tokens[3], &term) || term < 1 || term > 12) {
        return E_PARSE;
    }

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int c = findCourse(tokens[2]);

    // Unknown course: report it as an eligibility failure (E1).
    if (c == -1) {
        std::printf("ELIGIBLE %s %s NO E1\n",
                    students[s].roll.c_str(), tokens[2].c_str());
        return OK;
    }

    int check = checkEligibility(s, c, term);

    if (check == 0) {
        std::printf("ELIGIBLE %s %s YES\n",
                    students[s].roll.c_str(), courses[c].code.c_str());
    } else {
        std::printf("ELIGIBLE %s %s NO E%d\n",
                    students[s].roll.c_str(), courses[c].code.c_str(), check);
    }

    return OK;
}

ErrorCode ERPSystem::queryTranscript(const std::string tokens[], int count) const {
    if (count != 2) return E_PARSE;
    if (tokens[1].length() > 10) return E_PARSE;

    int s = findStudent(tokens[1]);

    if (s == -1) return E_UNKNOWN_STUDENT;

    int n = students[s].numRegs;

    std::printf("TRANSCRIPT %s %d\n", students[s].roll.c_str(), n);

    for (int t = 1; t <= 12; t++) {
        for (int i = 0; i < n; i++) {
            const Registration& r = students[s].regs[i];

            if (r.term != t) continue;

            const Course& crs = courses[r.course];

            const char* gstr = r.grade.empty() ? "-" : r.grade.c_str();

            if (crs.type == THESIS && r.thesis == IN_PROGRESS) {
                gstr = "IN_PROGRESS";
            }

            std::printf("%d %s %s %d\n",
                        r.term, crs.code.c_str(), gstr, crs.credits);
        }
    }

    return OK;
}

ErrorCode ERPSystem::saveState(const std::string tokens[], int count) const {
    if (count != 2) return E_PARSE;

    std::ofstream out(tokens[1].c_str());

    if (!out.is_open()) return E_PARSE;

    out << "ACADERP_SAVE_V1\n";

    out << config.totalCredits << " " << config.electiveCredits << " "
        << config.minGpaHundredths << " " << config.maxLoad << " "
        << config.thesisCredits << "\n";

    out << numCourses << "\n";

    for (int i = 0; i < numCourses; i++) {
        out << courses[i].code << " " << (int)courses[i].type << " "
            << courses[i].credits << " " << (courses[i].isCore ? 1 : 0) << " "
            << courses[i].prereq << "\n";
    }

    out << numStudents << "\n";

    for (int i = 0; i < numStudents; i++) {
        out << students[i].roll << " " << students[i].name << " "
            << students[i].numRegs << "\n";

        for (int j = 0; j < students[i].numRegs; j++) {
            const Registration& r = students[i].regs[j];

            std::string g = r.grade.empty() ? "-" : r.grade;

            out << r.course << " " << r.term << " " << g << " "
                << (int)r.thesis << "\n";
        }
    }

    return out.good() ? OK : E_PARSE;
}

ErrorCode ERPSystem::loadState(const std::string tokens[], int count) {
    if (count != 2) return E_PARSE;

    std::ifstream in(tokens[1].c_str());

    if (!in.is_open()) return E_PARSE;

    std::string magic;

    if (!(in >> magic) || magic != "ACADERP_SAVE_V1") return E_PARSE;

    ProgramConfig tmpCfg;

    if (!(in >> tmpCfg.totalCredits >> tmpCfg.electiveCredits
             >> tmpCfg.minGpaHundredths >> tmpCfg.maxLoad
             >> tmpCfg.thesisCredits)) {
        return E_PARSE;
    }

    int tmpNumCourses = 0;

    if (!(in >> tmpNumCourses) ||
        tmpNumCourses < 0 || tmpNumCourses > MAX_COURSES) {
        return E_PARSE;
    }

    Course* tmpCourses = new Course[MAX_COURSES];

    for (int i = 0; i < tmpNumCourses; i++) {
        int tInt = 0;
        int coreInt = 0;

        if (!(in >> tmpCourses[i].code >> tInt >> tmpCourses[i].credits
                 >> coreInt >> tmpCourses[i].prereq)) {
            delete[] tmpCourses;
            return E_PARSE;
        }

        if (tInt < 0 || tInt > 2) {
            delete[] tmpCourses;
            return E_PARSE;
        }

        tmpCourses[i].type = (CourseType)tInt;
        tmpCourses[i].isCore = (coreInt != 0);
    }

    int tmpNumStudents = 0;

    if (!(in >> tmpNumStudents) ||
        tmpNumStudents < 0 || tmpNumStudents > MAX_STUDENTS) {
        delete[] tmpCourses;
        return E_PARSE;
    }

    Student* tmpStudents = new Student[MAX_STUDENTS];

    for (int i = 0; i < tmpNumStudents; i++) {
        int nRegs = 0;

        if (!(in >> tmpStudents[i].roll >> tmpStudents[i].name >> nRegs) ||
            nRegs < 0 || nRegs > MAX_REGS) {
            delete[] tmpCourses;
            delete[] tmpStudents;
            return E_PARSE;
        }

        tmpStudents[i].numRegs = nRegs;

        for (int j = 0; j < nRegs; j++) {
            std::string g;
            int thInt = 0;

            if (!(in >> tmpStudents[i].regs[j].course
                     >> tmpStudents[i].regs[j].term >> g >> thInt) ||
                thInt < 0 || thInt > 4) {
                delete[] tmpCourses;
                delete[] tmpStudents;
                return E_PARSE;
            }

            tmpStudents[i].regs[j].grade = (g == "-") ? "" : g;
            tmpStudents[i].regs[j].thesis = (ThesisState)thInt;
        }
    }

    delete[] courses;
    delete[] students;

    config = tmpCfg;
    courses = tmpCourses;
    numCourses = tmpNumCourses;
    students = tmpStudents;
    numStudents = tmpNumStudents;

    return OK;
}
