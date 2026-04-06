/*
 * Attendance Manager - "Shortage Predictor"
 * DA2 C++ Mini Project
 * 
 * Student: Daksh Agarwal
 * Registration Number: 25BCE5098
 * 
 * Description: A class advisor tool to track student attendance,
 * identify shortage students, and generate attendance reports.
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <iomanip>
#include <algorithm>
#include <map>
#include <ctime>

using namespace std;

// ==================== Student Class ====================
class Student {
private:
    string regNo;
    string name;
    vector<pair<string, char>> attendanceRecords; // (date, P/A)

public:
    Student() : regNo(""), name("") {}
    
    Student(string reg, string n) : regNo(reg), name(n) {}

    // Getters
    string getRegNo() const { return regNo; }
    string getName() const { return name; }
    vector<pair<string, char>> getAttendanceRecords() const { return attendanceRecords; }

    // Setters
    void setRegNo(string reg) { regNo = reg; }
    void setName(string n) { name = n; }

    // Mark attendance for a specific date
    bool markAttendance(const string& date, char status) {
        // Validate status
        if (status != 'P' && status != 'A') {
            return false;
        }
        
        // Check if attendance already marked for this date
        for (auto& record : attendanceRecords) {
            if (record.first == date) {
                record.second = status; // Update existing record
                return true;
            }
        }
        
        // Add new attendance record
        attendanceRecords.push_back(make_pair(date, status));
        return true;
    }

    // Calculate attendance percentage
    double calculateAttendancePercentage() const {
        if (attendanceRecords.empty()) {
            return 0.0;
        }
        
        int presentCount = 0;
        for (const auto& record : attendanceRecords) {
            if (record.second == 'P') {
                presentCount++;
            }
        }
        
        return (static_cast<double>(presentCount) / attendanceRecords.size()) * 100.0;
    }

    // Get total classes attended
    int getTotalPresent() const {
        int count = 0;
        for (const auto& record : attendanceRecords) {
            if (record.second == 'P') count++;
        }
        return count;
    }

    // Get total classes conducted
    int getTotalClasses() const {
        return attendanceRecords.size();
    }

    // Display student details
    void display() const {
        stringstream ss;
        ss << fixed << setprecision(2) << calculateAttendancePercentage() << "%";
        cout << left << setw(15) << regNo 
             << setw(25) << name 
             << setw(15) << (to_string(getTotalPresent()) + "/" + to_string(getTotalClasses()))
             << setw(15) << ss.str()
             << endl;
    }

    // Display detailed attendance summary
    void displayAttendanceSummary() const {
        cout << "\n========================================\n";
        cout << "         ATTENDANCE SUMMARY\n";
        cout << "========================================\n";
        cout << "Registration No : " << regNo << endl;
        cout << "Name            : " << name << endl;
        cout << "----------------------------------------\n";
        cout << "Total Classes   : " << getTotalClasses() << endl;
        cout << "Classes Attended: " << getTotalPresent() << endl;
        cout << "Classes Absent  : " << (getTotalClasses() - getTotalPresent()) << endl;
        cout << "Attendance %    : " << fixed << setprecision(2) << calculateAttendancePercentage() << "%" << endl;
        cout << "----------------------------------------\n";
        
        if (calculateAttendancePercentage() < 75.0) {
            cout << "*** WARNING: ATTENDANCE SHORTAGE! ***\n";
            int classesNeeded = 0;
            int present = getTotalPresent();
            int total = getTotalClasses();
            // Calculate how many consecutive classes needed to reach 75%
            while ((static_cast<double>(present + classesNeeded) / (total + classesNeeded)) * 100.0 < 75.0 && classesNeeded < 100) {
                classesNeeded++;
            }
            if (classesNeeded < 100) {
                cout << "Need " << classesNeeded << " consecutive present days to reach 75%\n";
            }
        } else {
            cout << "Status: SAFE (Above 75%)\n";
        }
        
        cout << "\nDate-wise Attendance:\n";
        cout << "----------------------------------------\n";
        if (attendanceRecords.empty()) {
            cout << "No attendance records found.\n";
        } else {
            for (const auto& record : attendanceRecords) {
                cout << record.first << " : " << (record.second == 'P' ? "Present" : "Absent") << endl;
            }
        }
        cout << "========================================\n";
    }

    // Serialize to string for file storage
    string serialize() const {
        stringstream ss;
        ss << regNo << "|" << name << "|";
        for (size_t i = 0; i < attendanceRecords.size(); i++) {
            ss << attendanceRecords[i].first << ":" << attendanceRecords[i].second;
            if (i < attendanceRecords.size() - 1) {
                ss << ",";
            }
        }
        return ss.str();
    }

    // Deserialize from string
    void deserialize(const string& data) {
        stringstream ss(data);
        string token;
        
        getline(ss, regNo, '|');
        getline(ss, name, '|');
        
        string attendanceStr;
        getline(ss, attendanceStr);
        
        attendanceRecords.clear();
        if (!attendanceStr.empty()) {
            stringstream attSS(attendanceStr);
            string record;
            while (getline(attSS, record, ',')) {
                size_t colonPos = record.find(':');
                if (colonPos != string::npos) {
                    string date = record.substr(0, colonPos);
                    char status = record[colonPos + 1];
                    attendanceRecords.push_back(make_pair(date, status));
                }
            }
        }
    }
};

// ==================== AttendanceManager Class ====================
class AttendanceManager {
private:
    vector<Student> students;
    string dataFile;

    // Find student index by RegNo
    int findStudentIndex(const string& regNo) const {
        for (size_t i = 0; i < students.size(); i++) {
            if (students[i].getRegNo() == regNo) {
                return i;
            }
        }
        return -1;
    }

    // Check for duplicate RegNo
    bool isDuplicate(const string& regNo) const {
        return findStudentIndex(regNo) != -1;
    }

    // Validate RegNo format: NNAAANNNN (2 digits, 3 letters, 4 digits)
    bool isValidRegNo(const string& regNo) const {
        if (regNo.length() != 9) return false;
        
        // First 2 characters must be digits
        if (!isdigit(regNo[0]) || !isdigit(regNo[1])) return false;
        
        // Next 3 characters must be letters
        if (!isalpha(regNo[2]) || !isalpha(regNo[3]) || !isalpha(regNo[4])) return false;
        
        // Last 4 characters must be digits
        if (!isdigit(regNo[5]) || !isdigit(regNo[6]) || !isdigit(regNo[7]) || !isdigit(regNo[8])) return false;
        
        return true;
    }

public:
    AttendanceManager(const string& filename = "attendance_data.txt") : dataFile(filename) {
        loadFromFile();
    }

    ~AttendanceManager() {
        saveToFile();
    }

    // Save data to file
    void saveToFile() const {
        ofstream file(dataFile);
        if (file.is_open()) {
            for (const auto& student : students) {
                file << student.serialize() << endl;
            }
            file.close();
        }
    }

    // Load data from file
    void loadFromFile() {
        ifstream file(dataFile);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                if (!line.empty()) {
                    Student student;
                    student.deserialize(line);
                    students.push_back(student);
                }
            }
            file.close();
        }
    }

    // Add a new student
    bool addStudent(const string& regNo, const string& name) {
        // Convert regNo to uppercase for validation
        string regNoUpper = regNo;
        transform(regNoUpper.begin(), regNoUpper.end(), regNoUpper.begin(), ::toupper);
        
        if (!isValidRegNo(regNoUpper)) {
            cout << "\n[ERROR] Invalid RegNo format! Format: 2 digits + 3 letters + 4 digits (e.g., 25BCE5098)\n";
            return false;
        }
        
        if (isDuplicate(regNoUpper)) {
            cout << "\n[ERROR] Student with RegNo '" << regNoUpper << "' already exists!\n";
            return false;
        }
        
        students.push_back(Student(regNoUpper, name));
        saveToFile();
        cout << "\n[SUCCESS] Student added successfully!\n";
        return true;
    }

    // Add multiple students
    void addStudentsList() {
        cout << "\n=== ADD STUDENTS LIST ===\n";
        cout << "Enter number of students to add: ";
        int count;
        cin >> count;
        cin.ignore();

        for (int i = 0; i < count; i++) {
            cout << "\nStudent " << (i + 1) << ":\n";
            
            string regNo, name;
            cout << "Enter Registration Number: ";
            getline(cin, regNo);
            
            cout << "Enter Name: ";
            getline(cin, name);
            
            addStudent(regNo, name);
        }
    }

    // Mark attendance for a specific date
    void markAttendanceForDate() {
        if (students.empty()) {
            cout << "\n[ERROR] No students registered. Please add students first.\n";
            return;
        }

        string date;
        cout << "\n=== MARK ATTENDANCE ===\n";
        cout << "Enter date (DD-MM-YYYY): ";
        cin >> date;
        cin.ignore();

        cout << "\nMarking attendance for " << date << "\n";
        cout << "Enter P for Present, A for Absent\n";
        cout << "----------------------------------------\n";

        for (auto& student : students) {
            char status;
            bool validInput = false;
            
            while (!validInput) {
                cout << student.getRegNo() << " - " << student.getName() << ": ";
                cin >> status;
                status = toupper(status);
                
                if (status == 'P' || status == 'A') {
                    student.markAttendance(date, status);
                    validInput = true;
                } else {
                    cout << "[ERROR] Invalid input! Please enter P or A only.\n";
                }
            }
        }
        
        saveToFile();
        cout << "\n[SUCCESS] Attendance marked for " << date << "\n";
    }

    // View a student's attendance summary
    void viewStudentAttendance() const {
        cout << "\n=== VIEW STUDENT ATTENDANCE ===\n";
        cout << "Enter Registration Number: ";
        string regNo;
        cin >> regNo;

        int index = findStudentIndex(regNo);
        if (index == -1) {
            cout << "\n[ERROR] Student not found!\n";
            return;
        }

        students[index].displayAttendanceSummary();
    }

    // Display all students
    void displayAllStudents() const {
        if (students.empty()) {
            cout << "\n[INFO] No students registered.\n";
            return;
        }

        cout << "\n======================================================================\n";
        cout << "                        ALL STUDENTS LIST\n";
        cout << "======================================================================\n";
        cout << left << setw(15) << "Reg No" 
             << setw(25) << "Name" 
             << setw(15) << "Attendance"
             << setw(15) << "Percentage"
             << endl;
        cout << "----------------------------------------------------------------------\n";
        
        for (const auto& student : students) {
            student.display();
        }
        cout << "======================================================================\n";
        cout << "Total Students: " << students.size() << endl;
    }

    // ==================== REPORTS ====================

    // Report: Attendance percentage per student
    void reportAttendancePercentage() const {
        if (students.empty()) {
            cout << "\n[INFO] No students registered.\n";
            return;
        }

        cout << "\n======================================================================\n";
        cout << "               ATTENDANCE PERCENTAGE REPORT\n";
        cout << "======================================================================\n";
        cout << left << setw(15) << "Reg No" 
             << setw(25) << "Name" 
             << setw(15) << "Present/Total"
             << setw(15) << "Percentage"
             << setw(10) << "Status"
             << endl;
        cout << "----------------------------------------------------------------------\n";
        
        for (const auto& student : students) {
            stringstream ss;
            ss << fixed << setprecision(2) << student.calculateAttendancePercentage() << "%";
            
            cout << left << setw(15) << student.getRegNo() 
                 << setw(25) << student.getName() 
                 << setw(15) << (to_string(student.getTotalPresent()) + "/" + to_string(student.getTotalClasses()))
                 << setw(15) << ss.str()
                 << setw(10) << (student.calculateAttendancePercentage() >= 75.0 ? "SAFE" : "SHORTAGE")
                 << endl;
        }
        cout << "======================================================================\n";
    }

    // Report: Students below 75%
    void reportShortageStudents() const {
        if (students.empty()) {
            cout << "\n[INFO] No students registered.\n";
            return;
        }

        cout << "\n======================================================================\n";
        cout << "            SHORTAGE STUDENTS (Below 75%)\n";
        cout << "======================================================================\n";
        
        vector<Student> shortageList;
        for (const auto& student : students) {
            if (student.calculateAttendancePercentage() < 75.0) {
                shortageList.push_back(student);
            }
        }

        if (shortageList.empty()) {
            cout << "\n[INFO] No students with attendance shortage. All students are safe!\n";
            cout << "======================================================================\n";
            return;
        }

        // Sort by percentage (lowest first)
        sort(shortageList.begin(), shortageList.end(), 
            [](const Student& a, const Student& b) {
                return a.calculateAttendancePercentage() < b.calculateAttendancePercentage();
            });

        cout << left << setw(15) << "Reg No" 
             << setw(25) << "Name" 
             << setw(15) << "Present/Total"
             << setw(15) << "Percentage"
             << setw(20) << "Classes to 75%"
             << endl;
        cout << "----------------------------------------------------------------------\n";
        
        for (const auto& student : shortageList) {
            int classesNeeded = 0;
            int present = student.getTotalPresent();
            int total = student.getTotalClasses();
            
            // Calculate consecutive present classes needed
            while ((static_cast<double>(present + classesNeeded) / (total + classesNeeded)) * 100.0 < 75.0 && classesNeeded < 100) {
                classesNeeded++;
            }
            
            stringstream ss;
            ss << fixed << setprecision(2) << student.calculateAttendancePercentage() << "%";
            
            cout << left << setw(15) << student.getRegNo() 
                 << setw(25) << student.getName() 
                 << setw(15) << (to_string(present) + "/" + to_string(total))
                 << setw(15) << ss.str()
                 << setw(10) << classesNeeded
                 << endl;
        }
        
        cout << "----------------------------------------------------------------------\n";
        cout << "Total shortage students: " << shortageList.size() << " out of " << students.size() << endl;
        cout << "======================================================================\n";
    }

    // Report: Class average attendance
    void reportClassAverage() const {
        if (students.empty()) {
            cout << "\n[INFO] No students registered.\n";
            return;
        }

        cout << "\n======================================================================\n";
        cout << "               CLASS ATTENDANCE STATISTICS\n";
        cout << "======================================================================\n";

        double totalPercentage = 0.0;
        double highestPercentage = 0.0;
        double lowestPercentage = 100.0;
        string highestStudent, lowestStudent;
        int safeCount = 0, shortageCount = 0;

        for (const auto& student : students) {
            double percentage = student.calculateAttendancePercentage();
            totalPercentage += percentage;
            
            if (percentage >= 75.0) safeCount++;
            else shortageCount++;
            
            if (percentage > highestPercentage) {
                highestPercentage = percentage;
                highestStudent = student.getName() + " (" + student.getRegNo() + ")";
            }
            if (percentage < lowestPercentage) {
                lowestPercentage = percentage;
                lowestStudent = student.getName() + " (" + student.getRegNo() + ")";
            }
        }

        double avgPercentage = totalPercentage / students.size();

        cout << "\nTotal Students          : " << students.size() << endl;
        cout << "----------------------------------------\n";
        cout << "Class Average Attendance: " << fixed << setprecision(2) << avgPercentage << "%" << endl;
        cout << "----------------------------------------\n";
        cout << "Highest Attendance      : " << fixed << setprecision(2) << highestPercentage << "%" << endl;
        cout << "                          " << highestStudent << endl;
        cout << "Lowest Attendance       : " << fixed << setprecision(2) << lowestPercentage << "%" << endl;
        cout << "                          " << lowestStudent << endl;
        cout << "----------------------------------------\n";
        cout << "Students Above 75%      : " << safeCount << " (" << fixed << setprecision(1) << (safeCount * 100.0 / students.size()) << "%)" << endl;
        cout << "Students Below 75%      : " << shortageCount << " (" << fixed << setprecision(1) << (shortageCount * 100.0 / students.size()) << "%)" << endl;
        cout << "======================================================================\n";
    }

    // Delete a student
    bool deleteStudent() {
        cout << "\n=== DELETE STUDENT ===\n";
        cout << "Enter Registration Number: ";
        string regNo;
        cin >> regNo;

        int index = findStudentIndex(regNo);
        if (index == -1) {
            cout << "\n[ERROR] Student not found!\n";
            return false;
        }

        cout << "\nStudent found:\n";
        cout << "RegNo: " << students[index].getRegNo() << endl;
        cout << "Name: " << students[index].getName() << endl;
        cout << "\nAre you sure you want to delete? (Y/N): ";
        
        char confirm;
        cin >> confirm;
        
        if (toupper(confirm) == 'Y') {
            students.erase(students.begin() + index);
            saveToFile();
            cout << "\n[SUCCESS] Student deleted successfully!\n";
            return true;
        } else {
            cout << "\n[INFO] Deletion cancelled.\n";
            return false;
        }
    }

    // Search student
    void searchStudent() const {
        cout << "\n=== SEARCH STUDENT ===\n";
        cout << "Enter Registration Number or Name keyword: ";
        string keyword;
        cin.ignore();
        getline(cin, keyword);

        // Convert to lowercase for comparison
        string keywordLower = keyword;
        transform(keywordLower.begin(), keywordLower.end(), keywordLower.begin(), ::tolower);

        vector<int> found;
        for (size_t i = 0; i < students.size(); i++) {
            string regNoLower = students[i].getRegNo();
            string nameLower = students[i].getName();
            transform(regNoLower.begin(), regNoLower.end(), regNoLower.begin(), ::tolower);
            transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);

            if (regNoLower.find(keywordLower) != string::npos || 
                nameLower.find(keywordLower) != string::npos) {
                found.push_back(i);
            }
        }

        if (found.empty()) {
            cout << "\n[INFO] No students found matching '" << keyword << "'\n";
            return;
        }

        cout << "\n======================================================================\n";
        cout << "                   SEARCH RESULTS\n";
        cout << "======================================================================\n";
        cout << left << setw(15) << "Reg No" 
             << setw(25) << "Name" 
             << setw(15) << "Attendance"
             << setw(15) << "Percentage"
             << endl;
        cout << "----------------------------------------------------------------------\n";
        
        for (int idx : found) {
            students[idx].display();
        }
        cout << "======================================================================\n";
        cout << "Found " << found.size() << " student(s)\n";
    }

    // Get student count
    int getStudentCount() const {
        return students.size();
    }
};

// ==================== Main Menu Functions ====================
void displayBanner() {
    cout << "\n";
    cout << "======================================================================\n";
    cout << "         ATTENDANCE MANAGER - SHORTAGE PREDICTOR\n";
    cout << "         DA2 Mini Project - C++ OOP\n";
    cout << "         Student: Daksh Agarwal (25BCE5098)\n";
    cout << "======================================================================\n";
}

void displayMenu() {
    cout << "\n";
    cout << "MAIN MENU\n";
    cout << "--------------------------------------\n";
    cout << "  1. Add Student\n";
    cout << "  2. Add Students List\n";
    cout << "  3. Mark Attendance for Date\n";
    cout << "  4. View Student Attendance\n";
    cout << "  5. Display All Students\n";
    cout << "  6. Search Student\n";
    cout << "  7. Delete Student\n";
    cout << "--------------------------------------\n";
    cout << "REPORTS\n";
    cout << "--------------------------------------\n";
    cout << "  8. Attendance % per Student\n";
    cout << "  9. Shortage Students (Below 75%)\n";
    cout << " 10. Class Average Statistics\n";
    cout << "--------------------------------------\n";
    cout << "  0. Exit\n";
    cout << "--------------------------------------\n";
    cout << "\nEnter your choice: ";
}

int main() {
    AttendanceManager manager("attendance_data.txt");
    int choice;

    displayBanner();

    do {
        displayMenu();
        
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(10000, '\n');
            cout << "\n[ERROR] Invalid input! Please enter a number.\n";
            continue;
        }
        cin.ignore(10000, '\n');

        switch (choice) {
            case 1: {
                cout << "\n=== ADD STUDENT ===\n";
                string regNo, name;
                cout << "Enter Registration Number (e.g., 25BCE5098): ";
                getline(cin, regNo);
                cout << "Enter Name: ";
                getline(cin, name);
                manager.addStudent(regNo, name);
                break;
            }
            case 2:
                manager.addStudentsList();
                break;
            case 3:
                manager.markAttendanceForDate();
                break;
            case 4:
                manager.viewStudentAttendance();
                break;
            case 5:
                manager.displayAllStudents();
                break;
            case 6:
                manager.searchStudent();
                break;
            case 7:
                manager.deleteStudent();
                break;
            case 8:
                manager.reportAttendancePercentage();
                break;
            case 9:
                manager.reportShortageStudents();
                break;
            case 10:
                manager.reportClassAverage();
                break;
            case 0:
                cout << "\n======================================================================\n";
                cout << "          Thank you for using Attendance Manager!\n";
                cout << "                    Data saved successfully.\n";
                cout << "======================================================================\n\n";
                break;
            default:
                cout << "\n[ERROR] Invalid choice! Please enter 0-10.\n";
        }
    } while (choice != 0);

    return 0;
}
