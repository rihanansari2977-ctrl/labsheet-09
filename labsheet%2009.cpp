/*
    Project Title: Smart Expense Management System
    File Name: labsheet 09.cpp

    Objective:
    To record daily expenses, categorize spending, monitor a monthly budget,
    warn users before the budget is exceeded, and estimate month-end spending.

    Concepts Used:
    Variables, loops, arrays, structures, functions, strings, searching,
    sorting, and file handling.

    Compile (g++):
    g++ "labsheet 09.cpp" -o expense_manager

    Run:
    Windows: expense_manager.exe
    Linux/macOS: ./expense_manager

    Note:
    This is a simple educational project. User credentials and expense data
    are stored in local text files in the same folder as this program.
*/

#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <sstream>
#include <ctime>
#include <limits>

using namespace std;

const int MAX_EXPENSES = 1000;
const string USER_FILE = "users.txt";
const string EXPENSE_FILE = "expenses.txt";
const string BUDGET_FILE = "budget.txt";

struct Expense {
    int id;
    string username;
    string date;       // Format: YYYY-MM-DD
    string category;
    string description;
    double amount;
};

Expense expenses[MAX_EXPENSES];
int expenseCount = 0;
string currentUser;
double monthlyBudget = 0.0;

// Read an integer safely.
int readInt(const string& prompt) {
    int value;
    while (true) {
        cout << prompt;
        if (cin >> value) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Invalid input. Please enter a number.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Read a positive amount safely.
double readAmount(const string& prompt) {
    double value;
    while (true) {
        cout << prompt;
        if (cin >> value && value > 0) {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
        cout << "Please enter an amount greater than zero.\n";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
}

// Return today's date in YYYY-MM-DD format.
string getTodayDate() {
    time_t now = time(0);
    tm* localTime = localtime(&now);
    char dateBuffer[11];
    strftime(dateBuffer, sizeof(dateBuffer), "%Y-%m-%d", localTime);
    return string(dateBuffer);
}

// Check whether a date has the YYYY-MM format at the beginning.
bool isValidMonth(const string& month) {
    if (month.length() != 7 || month[4] != '-') return false;
    for (int i = 0; i < 7; ++i) {
        if (i == 4) continue;
        if (month[i] < '0' || month[i] > '9') return false;
    }
    int m = (month[5] - '0') * 10 + (month[6] - '0');
    return m >= 1 && m <= 12;
}

string monthFromDate(const string& date) {
    if (date.length() >= 7) return date.substr(0, 7);
    return "";
}

// Register a new user.
void registerUser() {
    string username, password;

    cout << "\n--- User Registration ---\n";
    cout << "Create username: ";
    getline(cin, username);
    cout << "Create password: ";
    getline(cin, password);

    if (username.empty() || password.empty() ||
        username.find('|') != string::npos ||
        password.find('|') != string::npos) {
        cout << "Username/password cannot be empty or contain '|'.\n";
        return;
    }

    ifstream input(USER_FILE.c_str());
    string savedUser, savedPassword;
    while (getline(input, savedUser, '|') && getline(input, savedPassword)) {
        if (savedUser == username) {
            cout << "This username already exists. Please log in.\n";
            return;
        }
    }
    input.close();

    ofstream output(USER_FILE.c_str(), ios::app);
    if (!output) {
        cout << "Unable to open user file.\n";
        return;
    }
    output << username << "|" << password << "\n";
    output.close();

    cout << "Registration successful. You can now log in.\n";
}

// Login using a registered username and password.
bool loginUser() {
    string username, password, savedUser, savedPassword;

    cout << "\n--- Login ---\n";
    cout << "Username: ";
    getline(cin, username);
    cout << "Password: ";
    getline(cin, password);

    ifstream input(USER_FILE.c_str());
    while (getline(input, savedUser, '|') && getline(input, savedPassword)) {
        if (savedUser == username && savedPassword == password) {
            currentUser = username;
            input.close();
            cout << "Login successful. Welcome, " << currentUser << "!\n";
            return true;
        }
    }
    input.close();

    cout << "Incorrect username or password, or account not registered.\n";
    return false;
}

// Load all saved expenses from the file.
void loadExpenses() {
    expenseCount = 0;
    ifstream input(EXPENSE_FILE.c_str());
    string line;

    while (getline(input, line) && expenseCount < MAX_EXPENSES) {
        stringstream ss(line);
        string idText, amountText;
        Expense item;

        if (!getline(ss, idText, '|') ||
            !getline(ss, item.username, '|') ||
            !getline(ss, item.date, '|') ||
            !getline(ss, item.category, '|') ||
            !getline(ss, item.description, '|') ||
            !getline(ss, amountText)) {
            continue;
        }

        try {
            item.id = stoi(idText);
            item.amount = stod(amountText);
            expenses[expenseCount++] = item;
        } catch (...) {
            // Ignore an incomplete or invalid record.
        }
    }
    input.close();
}

// Save all expenses to the file.
void saveExpenses() {
    ofstream output(EXPENSE_FILE.c_str());
    if (!output) {
        cout << "Error: Could not save expense data.\n";
        return;
    }

    for (int i = 0; i < expenseCount; ++i) {
        output << expenses[i].id << "|"
               << expenses[i].username << "|"
               << expenses[i].date << "|"
               << expenses[i].category << "|"
               << expenses[i].description << "|"
               << fixed << setprecision(2) << expenses[i].amount << "\n";
    }
    output.close();
}

// Find the next available expense ID.
int getNextExpenseId() {
    int highestId = 0;
    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].id > highestId) highestId = expenses[i].id;
    }
    return highestId + 1;
}

// Load the current user's monthly budget.
void loadBudget() {
    monthlyBudget = 0.0;
    ifstream input(BUDGET_FILE.c_str());
    string savedUser;
    double savedBudget;

    while (input >> savedUser >> savedBudget) {
        if (savedUser == currentUser) {
            monthlyBudget = savedBudget;
            break;
        }
    }
    input.close();
}

// Save or update the current user's budget.
void saveBudget() {
    ifstream input(BUDGET_FILE.c_str());
    string users[500];
    double budgets[500];
    int count = 0;
    string savedUser;
    double savedBudget;

    while (input >> savedUser >> savedBudget && count < 500) {
        if (savedUser != currentUser) {
            users[count] = savedUser;
            budgets[count] = savedBudget;
            ++count;
        }
    }
    input.close();

    ofstream output(BUDGET_FILE.c_str());
    if (!output) {
        cout << "Error: Could not save budget.\n";
        return;
    }

    for (int i = 0; i < count; ++i) {
        output << users[i] << " " << budgets[i] << "\n";
    }
    output << currentUser << " " << monthlyBudget << "\n";
    output.close();
}

// Calculate the current user's expenses for a selected month.
double totalForMonth(const string& month) {
    double total = 0.0;
    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].username == currentUser &&
            monthFromDate(expenses[i].date) == month) {
            total += expenses[i].amount;
        }
    }
    return total;
}

// Set the monthly spending limit.
void setBudget() {
    cout << "\n--- Set Monthly Budget ---\n";
    monthlyBudget = readAmount("Enter your monthly budget (Rs.): ");
    saveBudget();
    cout << "Monthly budget saved: Rs. "
         << fixed << setprecision(2) << monthlyBudget << "\n";
}

// Display a warning if the budget is close to or over the limit.
void checkBudgetWarning(const string& month) {
    if (monthlyBudget <= 0) {
        cout << "Please set your monthly budget first.\n";
        return;
    }

    double spent = totalForMonth(month);
    double remaining = monthlyBudget - spent;
    double percentage = (spent / monthlyBudget) * 100.0;

    cout << "\nBudget used: " << fixed << setprecision(1)
         << percentage << "%\n";

    if (spent > monthlyBudget) {
        cout << "WARNING: You have exceeded your monthly budget by Rs. "
             << fixed << setprecision(2) << (spent - monthlyBudget) << ".\n";
    } else if (percentage >= 80.0) {
        cout << "WARNING: You have used at least 80% of your budget.\n";
        cout << "Remaining budget: Rs. " << fixed << setprecision(2)
             << remaining << "\n";
    } else {
        cout << "Good! You are within your monthly budget.\n";
        cout << "Remaining budget: Rs. " << fixed << setprecision(2)
             << remaining << "\n";
    }
}

// Add a new daily expense.
void addExpense() {
    if (expenseCount >= MAX_EXPENSES) {
        cout << "Expense limit reached. Cannot add more records.\n";
        return;
    }

    Expense item;
    item.id = getNextExpenseId();
    item.username = currentUser;

    cout << "\n--- Add Daily Expense ---\n";
    cout << "Enter date (YYYY-MM-DD) or press Enter for today: ";
    getline(cin, item.date);
    if (item.date.empty()) item.date = getTodayDate();

    cout << "Category (Food/Travel/Books/Shopping/Bills/Other): ";
    getline(cin, item.category);
    cout << "Description: ";
    getline(cin, item.description);
    item.amount = readAmount("Amount (Rs.): ");

    if (item.date.find('|') != string::npos ||
        item.category.find('|') != string::npos ||
        item.description.find('|') != string::npos ||
        item.category.empty()) {
        cout << "Invalid text. Fields cannot be empty or contain '|'.\n";
        return;
    }

    expenses[expenseCount++] = item;
    saveExpenses();

    cout << "Expense added successfully. Expense ID: " << item.id << "\n";
    checkBudgetWarning(monthFromDate(item.date));
}

// Display one expense record.
void displayExpense(const Expense& item) {
    cout << left << setw(6) << item.id
         << setw(13) << item.date
         << setw(15) << item.category
         << setw(25) << item.description
         << "Rs. " << fixed << setprecision(2) << item.amount << "\n";
}

// View all expense records for the logged-in user.
void viewExpenseHistory() {
    cout << "\n--- Expense History ---\n";
    cout << left << setw(6) << "ID" << setw(13) << "Date"
         << setw(15) << "Category" << setw(25) << "Description"
         << "Amount\n";
    cout << string(75, '-') << "\n";

    bool found = false;
    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].username == currentUser) {
            displayExpense(expenses[i]);
            found = true;
        }
    }
    if (!found) cout << "No expenses found.\n";
}

// Show spending totals grouped by category for a month.
void categoryWiseExpense() {
    string month;
    cout << "\nEnter month (YYYY-MM): ";
    getline(cin, month);
    if (!isValidMonth(month)) {
        cout << "Invalid month format.\n";
        return;
    }

    string categories[MAX_EXPENSES];
    double totals[MAX_EXPENSES] = {0};
    int categoryCount = 0;

    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].username != currentUser ||
            monthFromDate(expenses[i].date) != month) continue;

        int position = -1;
        for (int j = 0; j < categoryCount; ++j) {
            if (categories[j] == expenses[i].category) {
                position = j;
                break;
            }
        }

        if (position == -1) {
            categories[categoryCount] = expenses[i].category;
            totals[categoryCount] = expenses[i].amount;
            ++categoryCount;
        } else {
            totals[position] += expenses[i].amount;
        }
    }

    cout << "\nCategory-wise report for " << month << "\n";
    if (categoryCount == 0) {
        cout << "No expenses found for this month.\n";
        return;
    }

    for (int i = 0; i < categoryCount; ++i) {
        cout << left << setw(18) << categories[i]
             << "Rs. " << fixed << setprecision(2) << totals[i] << "\n";
    }
}

// Display a monthly report and budget status.
void monthlyExpenseReport() {
    string month;
    cout << "\nEnter month (YYYY-MM): ";
    getline(cin, month);
    if (!isValidMonth(month)) {
        cout << "Invalid month format.\n";
        return;
    }

    double total = totalForMonth(month);
    cout << "\n--- Monthly Expense Report: " << month << " ---\n";
    cout << "Total spending: Rs. " << fixed << setprecision(2) << total << "\n";
    cout << "Monthly budget: Rs. " << fixed << setprecision(2)
         << monthlyBudget << "\n";

    if (monthlyBudget > 0) {
        cout << "Budget balance: Rs. " << fixed << setprecision(2)
             << (monthlyBudget - total) << "\n";
        checkBudgetWarning(month);
    } else {
        cout << "No budget has been set yet.\n";
    }
}

// Estimate month-end spending using average daily spending so far.
void predictExpenses() {
    string month;
    cout << "\nEnter current month (YYYY-MM): ";
    getline(cin, month);
    if (!isValidMonth(month)) {
        cout << "Invalid month format.\n";
        return;
    }

    double spent = totalForMonth(month);
    string today = getTodayDate();
    int currentDay = 1;

    if (month == monthFromDate(today)) {
        currentDay = (today[8] - '0') * 10 + (today[9] - '0');
    } else {
        cout << "Prediction uses today's day number for the selected month.\n";
        currentDay = 15;
    }

    // Find the number of days in the selected month.
    int year = stoi(month.substr(0, 4));
    int monthNumber = stoi(month.substr(5, 2));
    int daysInMonth = 31;
    if (monthNumber == 4 || monthNumber == 6 ||
        monthNumber == 9 || monthNumber == 11) {
        daysInMonth = 30;
    } else if (monthNumber == 2) {
        bool leapYear = (year % 400 == 0) ||
                        (year % 4 == 0 && year % 100 != 0);
        daysInMonth = leapYear ? 29 : 28;
    }

    if (currentDay > daysInMonth) currentDay = daysInMonth;
    double predicted = (spent / currentDay) * daysInMonth;

    cout << "\n--- Expense Prediction ---\n";
    cout << "Amount spent so far: Rs. " << fixed << setprecision(2) << spent << "\n";
    cout << "Estimated month-end spending: Rs. " << predicted << "\n";

    if (monthlyBudget > 0) {
        if (predicted > monthlyBudget) {
            cout << "Prediction: Your monthly budget may be exceeded by Rs. "
                 << (predicted - monthlyBudget) << ".\n";
        } else {
            cout << "Prediction: You are likely to stay within your budget.\n";
        }
    } else {
        cout << "Set a monthly budget to compare the prediction with your limit.\n";
    }
}

// Search expenses by ID, category, or description.
void searchExpense() {
    cout << "\n--- Search Expense ---\n";
    cout << "1. Search by Expense ID\n";
    cout << "2. Search by Category\n";
    cout << "3. Search by Description\n";
    int choice = readInt("Enter choice: ");
    bool found = false;

    if (choice == 1) {
        int searchId = readInt("Enter expense ID: ");
        for (int i = 0; i < expenseCount; ++i) {
            if (expenses[i].username == currentUser &&
                expenses[i].id == searchId) {
                displayExpense(expenses[i]);
                found = true;
            }
        }
    } else if (choice == 2 || choice == 3) {
        string keyword;
        cout << "Enter search text: ";
        getline(cin, keyword);

        for (int i = 0; i < expenseCount; ++i) {
            if (expenses[i].username != currentUser) continue;

            string field = (choice == 2) ? expenses[i].category
                                         : expenses[i].description;
            if (field.find(keyword) != string::npos) {
                displayExpense(expenses[i]);
                found = true;
            }
        }
    } else {
        cout << "Invalid choice.\n";
        return;
    }

    if (!found) cout << "No matching expense found.\n";
}

// Delete an expense by its ID.
void deleteExpense() {
    cout << "\n--- Delete Expense ---\n";
    int deleteId = readInt("Enter expense ID to delete: ");

    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].username == currentUser &&
            expenses[i].id == deleteId) {
            for (int j = i; j < expenseCount - 1; ++j) {
                expenses[j] = expenses[j + 1];
            }
            --expenseCount;
            saveExpenses();
            cout << "Expense deleted successfully.\n";
            return;
        }
    }
    cout << "Expense ID not found.\n";
}

// Sort the current user's expense records by amount (highest first).
void sortExpensesByAmount() {
    for (int i = 0; i < expenseCount - 1; ++i) {
        for (int j = 0; j < expenseCount - i - 1; ++j) {
            if (expenses[j].username == currentUser &&
                expenses[j + 1].username == currentUser &&
                expenses[j].amount < expenses[j + 1].amount) {
                Expense temp = expenses[j];
                expenses[j] = expenses[j + 1];
                expenses[j + 1] = temp;
            }
        }
    }

    cout << "\n--- Your Expenses Sorted by Amount (Highest First) ---\n";
    for (int i = 0; i < expenseCount; ++i) {
        if (expenses[i].username == currentUser) displayExpense(expenses[i]);
    }
}

// Main menu shown after a successful login.
void expenseMenu() {
    loadExpenses();
    loadBudget();

    int choice;
    do {
        cout << "\n========== SMART EXPENSE MANAGEMENT SYSTEM ==========\n";
        cout << "Logged in as: " << currentUser << "\n";
        cout << "1. Add Daily Expense\n";
        cout << "2. View Expense History\n";
        cout << "3. Category-wise Expense\n";
        cout << "4. Monthly Expense Report\n";
        cout << "5. Set Monthly Budget\n";
        cout << "6. Budget Warning\n";
        cout << "7. Expense Prediction\n";
        cout << "8. Search Expense\n";
        cout << "9. Delete Expense\n";
        cout << "10. Sort Expenses by Amount\n";
        cout << "11. Save Data\n";
        cout << "0. Logout\n";

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1: addExpense(); break;
            case 2: viewExpenseHistory(); break;
            case 3: categoryWiseExpense(); break;
            case 4: monthlyExpenseReport(); break;
            case 5: setBudget(); break;
            case 6: {
                string month;
                cout << "Enter month (YYYY-MM): ";
                getline(cin, month);
                if (isValidMonth(month)) checkBudgetWarning(month);
                else cout << "Invalid month format.\n";
                break;
            }
            case 7: predictExpenses(); break;
            case 8: searchExpense(); break;
            case 9: deleteExpense(); break;
            case 10: sortExpensesByAmount(); break;
            case 11:
                saveExpenses();
                saveBudget();
                cout << "All data saved successfully.\n";
                break;
            case 0:
                saveExpenses();
                saveBudget();
                cout << "Logged out successfully.\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);
}

int main() {
    int choice;

    cout << "===============================================\n";
    cout << "       SMART EXPENSE MANAGEMENT SYSTEM\n";
    cout << "===============================================\n";

    do {
        cout << "\n1. Register\n";
        cout << "2. Login\n";
        cout << "0. Exit\n";

        choice = readInt("Enter your choice: ");

        switch (choice) {
            case 1:
                registerUser();
                break;
            case 2:
                if (loginUser()) expenseMenu();
                break;
            case 0:
                cout << "Thank you for using Smart Expense Management System.\n";
                break;
            default:
                cout << "Invalid choice. Please try again.\n";
        }
    } while (choice != 0);

    return 0;
}
