#include <iostream>
#include <string>
#include <unordered_map>
#include <cctype>
#include <iomanip>
#include <random>
#include <sstream>

using namespace std;

// Cryptography/hashing helpers
static string generateSalt() {
    const string chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    random_device rd;
    mt19937 eng(rd());

    uniform_int_distribution<> distr(0, static_cast<int>(chars.size() - 1));

    string salt = "";
    for (int i = 0; i < 8; ++i) {
        salt += chars[distr(eng)];
    }
    return salt;
}

static string hashPassword(const string& password, const string& salt) {
    string combined = password + salt;
    unsigned long long hashValue = 5381;

    for (char c : combined) {
        hashValue = ((hashValue << 5) + hashValue) + c;
    }

    stringstream ss;
    ss << hex << setw(16) << setfill('0') << hashValue;
    return ss.str();
}

// 1. ARRAY: Fixed-size transaction history
const int MAX_HISTORY = 5;

struct Transaction {
    string type;
    double amount = 0.0;
};

struct Account {
    int accountNumber = 0;
    string username;
    string passwordHash;
    string salt;
    double balance = 0.0;

    Transaction history[MAX_HISTORY];
    int historyCount = 0;

    void recordTransaction(string type, double amount) {
        if (historyCount == MAX_HISTORY) {
            for (int i = 1; i < MAX_HISTORY; i++) {
                history[i - 1] = history[i];
            }
            historyCount--;
        }
        history[historyCount].type = type;
        history[historyCount].amount = amount;
        historyCount++;
    }
};

// 2. LINKED LIST: Custom node for data storage
struct Node {
    Account account;
    Node* next;

    Node(Account acc) : account(acc), next(nullptr) {}
};

class BankSystem {
private:
    Node* head;
    Node* tail;

    // 3. HASH TABLES: For instant lookups & uniqueness
    unordered_map<int, Account*> accountIndex;
    unordered_map<string, int> usernameToIndex;

    int nextAccountNumber;

public:
    BankSystem() : head(nullptr), tail(nullptr), nextAccountNumber(1001) {}

    ~BankSystem() {
        Node* current = head;
        while (current != nullptr) {
            Node* nextNode = current->next;
            delete current;
            current = nextNode;
        }
    }

    bool isUsernameTaken(const string& username) {
        return usernameToIndex.find(username) != usernameToIndex.end();
    }

    // --- Authentication ---
    int login(string username, string rawPassword) {
        if (!isUsernameTaken(username)) {
            return -1;
        }

        int accNum = usernameToIndex[username];
        Account* acc = accountIndex[accNum];

        string attemptedHash = hashPassword(rawPassword, acc->salt);

        if (attemptedHash == acc->passwordHash) {
            return accNum;
        }

        return -1;
    }

    //Core banking operations

    void createAccount(string username, string rawPassword, double initialDeposit) {
        if (initialDeposit < 0) {
            cout << "Initial deposit cannot be negative!\n";
            return;
        }

        Account newAcc;
        newAcc.accountNumber = nextAccountNumber++;
        newAcc.username = username;

        newAcc.salt = generateSalt();
        newAcc.passwordHash = hashPassword(rawPassword, newAcc.salt);

        newAcc.balance = initialDeposit;
        newAcc.recordTransaction("Initial Deposit", initialDeposit);

        Node* newNode = new Node(newAcc);

        if (head == nullptr) {
            head = tail = newNode;
        }
        else {
            tail->next = newNode;
            tail = newNode;
        }

        accountIndex[newNode->account.accountNumber] = &(newNode->account);
        usernameToIndex[username] = newNode->account.accountNumber;

        cout << "\nSuccess! Account created securely.\n";
        cout << "Username: " << username << "\n";
        cout << "Account Number: " << newNode->account.accountNumber << "\n";
    }

    void checkBalance(int accNum) {
        Account* acc = accountIndex[accNum];
        cout << "\n--- Account Details ---\n";
        cout << "Username: " << acc->username << "\n";
        cout << "Account #: " << acc->accountNumber << "\n";
        cout << "Balance: $" << fixed << setprecision(2) << acc->balance << "\n";

        cout << "Recent Transactions:\n";
        if (acc->historyCount == 0) cout << "  No transactions yet.\n";
        for (int i = 0; i < acc->historyCount; i++) {
            cout << "  - " << acc->history[i].type << ": $" << acc->history[i].amount << "\n";
        }
    }

    void deposit(int accNum, double amount) {
        if (amount <= 0) {
            cout << "Error: Deposit amount must be greater than zero.\n";
            return;
        }

        Account* acc = accountIndex[accNum];
        acc->balance += amount;
        acc->recordTransaction("Deposit", amount);

        cout << "Successfully deposited $" << fixed << setprecision(2) << amount
            << ". New balance: $" << fixed << setprecision(2) << acc->balance << "\n";
    }

    void withdraw(int accNum, double amount) {
        if (amount <= 0) {
            cout << "Error: Withdrawal amount must be greater than zero.\n";
            return;
        }

        Account* acc = accountIndex[accNum];
        if (acc->balance < amount) {
            cout << "Error: Insufficient funds. Current balance is $"
                << fixed << setprecision(2) << acc->balance << "\n";
            return;
        }

        acc->balance -= amount;
        acc->recordTransaction("Withdrawal", amount);

        cout << "Successfully withdrew $" << fixed << setprecision(2) << amount
            << ". New balance: $" << fixed << setprecision(2) << acc->balance << "\n";
    }

    void transfer(int fromAccNum, int toAccNum, double amount) {
        if (amount <= 0) {
            cout << "Error: Transfer amount must be greater than zero.\n";
            return;
        }
        if (accountIndex.find(toAccNum) == accountIndex.end()) {
            cout << "Error: Receiver account not found.\n";
            return;
        }
        if (fromAccNum == toAccNum) {
            cout << "Error: Cannot transfer money to yourself.\n";
            return;
        }

        Account* sender = accountIndex[fromAccNum];
        Account* receiver = accountIndex[toAccNum];

        if (sender->balance < amount) {
            cout << "Error: Insufficient funds for transfer.\n";
            return;
        }

        sender->balance -= amount;
        sender->recordTransaction("Transfer Out to #" + to_string(toAccNum), amount);

        receiver->balance += amount;
        receiver->recordTransaction("Transfer In from #" + to_string(fromAccNum), amount);

        cout << "Successfully transferred $" << fixed << setprecision(2) << amount
            << " to Account #" << toAccNum << ".\n";
    }
};

static bool isAlphaNumeric(const string& str) {
    for (char c : str) {
        if (!isalnum(c)) {
            return false;
        }
    }
    return true;
}

static void toLowerCase(string& str) {
    for (char& c : str) {
        c = tolower(c);
    }
}

int main() {
    BankSystem bank;
    int loggedInAccount = -1;
    string activeUsername = "";

    cout << "========================================\n";
    cout << "          Welcome to C++ Bank    \n";
    cout << "========================================\n";

    while (true) {
        int choice;

        if (loggedInAccount == -1) {
            cout << "\n--- Welcome Menu ---\n";
            cout << "1. Login\n";
            cout << "2. Create Account\n";
            cout << "3. Exit System\n";
            cout << "Enter your choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input. Please enter a number.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (choice == 1) {
                cout << "\n--- Secure Login ---\n";
                string username, password;
                cout << "Username: ";
                cin >> username;
                toLowerCase(username);

                cout << "Password: ";
                cin >> password;

                int authResult = bank.login(username, password);

                if (authResult != -1) {
                    loggedInAccount = authResult;
                    activeUsername = username;
                    cout << ">> Login Successful! Welcome back, " << username << ".\n";
                }
                else {
                    cout << ">> Error: Invalid username or password.\n";
                }
            }
            else if (choice == 2) {
                cout << "\n--- Account Creation ---\n";
                string username, pass1, pass2;
                double depositAmt;

                while (true) {
                    cout << "Enter a new username (letters/numbers only, case insensitive, 20 character limit): ";
                    cin >> username;
                    toLowerCase(username);

                    if (username.length() > 20) {
                        cout << ">> Error: Username is too long. Maximum 20 characters.\n";
                        continue;
                    }

                    if (!isAlphaNumeric(username)) {
                        cout << ">> Error: Username contains invalid characters. Please try again.\n";
                        continue;
                    }

                    if (bank.isUsernameTaken(username)) {
                        cout << ">> Error: Username '" << username << "' is already taken.\n";
                        continue;
                    }
                    break;
                }

                while (true) {
                    cout << "Enter a password (8-64 characters): ";
                    cin >> pass1;

                    if (pass1.length() < 8 || pass1.length() > 64) {
                        cout << ">> Error: Password must be between 8 and 64 characters.\n";
                        continue;
                    }

                    cout << "Confirm your password: ";
                    cin >> pass2;

                    if (pass1 != pass2) {
                        cout << ">> Error: Passwords do not match. Please try again.\n";
                        continue;
                    }
                    break;
                }

                cout << "Enter initial deposit: $";
                while (!(cin >> depositAmt) || depositAmt < 0) {
                    cout << ">> Invalid amount. Please enter a positive number: $";
                    cin.clear();
                    cin.ignore(10000, '\n');
                }

                bank.createAccount(username, pass1, depositAmt);
            }
            else if (choice == 3) {
                cout << "System shutting down. Goodbye!\n";
                break;
            }
            else {
                cout << "Invalid choice.\n";
            }
        }
        else {
            cout << "\n--- Account Dashboard (" << activeUsername << ") ---\n";
            cout << "1. Check Balance & History\n";
            cout << "2. Deposit Funds\n";
            cout << "3. Withdraw Funds\n";
            cout << "4. Transfer Money\n";
            cout << "5. Logout\n";
            cout << "Enter your choice: ";

            if (!(cin >> choice)) {
                cout << "Invalid input. Please enter a number.\n";
                cin.clear();
                cin.ignore(10000, '\n');
                continue;
            }

            if (choice == 1) {
                bank.checkBalance(loggedInAccount);
            }
            else if (choice == 2) {
                double amount;
                cout << "Enter amount to deposit: $";
                cin >> amount;
                bank.deposit(loggedInAccount, amount);
            }
            else if (choice == 3) {
                double amount;
                cout << "Enter amount to withdraw: $";
                cin >> amount;
                bank.withdraw(loggedInAccount, amount);
            }
            else if (choice == 4) {
                int toAcc;
                double amount;
                cout << "Enter destination account number: ";
                cin >> toAcc;
                cout << "Enter amount to transfer: $";
                cin >> amount;
                bank.transfer(loggedInAccount, toAcc, amount);
            }
            else if (choice == 5) {
                cout << ">> Successfully logged out.\n";
                loggedInAccount = -1;
                activeUsername = "";
            }
            else {
                cout << "Invalid choice.\n";
            }
        }
    }

    return 0;
}
