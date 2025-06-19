#include <iostream>
#include <string>
#include <fstream>
#include <cstdlib>      // For atoi, atof, system
#include <cstdio>       // For sprintf
#include <cctype>       // For tolower, isdigit
#include <conio.h>      // For _getch()
#include <cmath>        // For floor
#include <windows.h>    // For console and color
#include <stdexcept>    // For standard exceptions
#include <sstream>      // For stringstream
#include <iomanip>      // For formatted table UI
#include <ctime>        // For timestamp in logger
#include <vector>

using namespace std;

// Forward declarations for all classes and structs
class HashTable;
class UserManager;
class BaseUser;
class Admin;
class Customer;
class ReportGenerator;
class InventoryReport;
class SalesReport;
class ShoppingCart;
class Logger;

// =================================================================
// DESIGN & UI (HELPER FUNCTIONS)
// =================================================================

// --- COLOR CONSTANTS ---
const int COLOR_DEFAULT = 7;
const int COLOR_TITLE = 11;     // Light Aqua
const int COLOR_SUCCESS = 10;   // Light Green
const int COLOR_ERROR = 12;     // Light Red
const int COLOR_WARNING = 14;   // Yellow
const int COLOR_INPUT = 15;     // Bright White
const int COLOR_HEADER = 13;    // Light Purple

HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

void setColor(int color) {
    SetConsoleTextAttribute(hConsole, color);
}

void printHeader(const string& title) {
    system("cls");
    setColor(COLOR_TITLE);
    cout << "======================================================================\n";
    cout << "||        " << left << setw(52) << title << "||\n";
    cout << "======================================================================\n\n";
    setColor(COLOR_DEFAULT);
}

void printSubHeader(const string& title) {
    setColor(COLOR_HEADER);
    cout << "\n--- " << title << " ---\n";
    setColor(COLOR_DEFAULT);
}

void pressKeyToContinue() {
    setColor(COLOR_WARNING);
    cout << "\n\nPress any key to continue...";
    _getch();
    setColor(COLOR_DEFAULT);
}

// =================================================================
// UTILITY FUNCTIONS
// =================================================================

string intToString(int value) {
    char buffer[50];
    sprintf(buffer, "%d", value);
    return string(buffer);
}

string doubleToString(double value) {
    char buffer[50];
    sprintf(buffer, "%.2f", value);
    return string(buffer);
}

int getIntegerInput() {
    string line;
    int number;
    while (true) {
        setColor(COLOR_INPUT);
        getline(cin, line);
        setColor(COLOR_DEFAULT);

        if (line.empty()) {
            continue;
        }

        stringstream ss(line);
        if (ss >> number && ss.eof()) {
            return number;
        } else {
            setColor(COLOR_ERROR);
            cout << "Invalid input. Please enter a valid integer: ";
        }
    }
}


struct Product {
    int id;
    string name;
    string category;
    double price;
    int quantityInStock;

    Product(int prodId = 0, string prodName = "N/A", string prodCat = "Other", double prodPrice = 0.0, int prodQty = 0)
        : id(prodId), name(prodName), category(prodCat), price(prodPrice), quantityInStock(prodQty) {}

    void display() const {
        cout << "  ID: " << id << "\n";
        cout << "  Name: " << name << "\n";
        cout << "  Category: " << category << "\n";
        cout << "  Price: $" << fixed << setprecision(2) << price << "\n";
        cout << "  In Stock: " << quantityInStock << " units" << endl;
    }

    friend void showProductPriceWithTax(const Product& product);
};

void showProductPriceWithTax(const Product& product) {
    setColor(COLOR_WARNING);
    cout << "Info: Product '" << product.name << "' price with 8% tax is $" 
         << fixed << setprecision(2) << product.price * 1.08 << endl;
    setColor(COLOR_DEFAULT);
}

struct User {
    string username;
    string password;
    string role;
};


class Logger {
private:
    static Logger* instance;
    ofstream logFile;
    string filename;

    Logger(string fname = "system_log.txt") : filename(fname) {
        logFile.open(filename.c_str(), ios::app);
        if (!logFile.is_open()) {
            cerr << "FATAL ERROR: Could not open log file: " << filename << endl;
        }
    }

    string getCurrentTimestamp() {
        time_t now = time(0);
        char buf[80];
        strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
        return string(buf);
    }

public:
    Logger(const Logger&) = delete;
    void operator=(const Logger&) = delete;

    static Logger& getInstance() {
        if (instance == NULL) {
            instance = new Logger();
        }
        return *instance;
    }

    void log(const string& level, const string& message) {
        if (logFile.is_open()) {
            logFile << "[" << getCurrentTimestamp() << "] [" << level << "] " << message << endl;
        }
    }

    void logInfo(const string& message) {
        log("INFO", message);
    }

    void logWarning(const string& message) {
        log("WARN", message);
    }

    void logError(const string& message) {
        log("ERROR", message);
    }
    
    ~Logger() {
        if (logFile.is_open()) {
            logFile.close();
        }
    }
};

Logger* Logger::instance = NULL;



class HashTable {
private:
    Product** table;
    int capacity;
    int currentSize;
    const double HASH_CONSTANT = 0.6180339887; 

    int hash(int key) {
        if (key < 0) {
            key = -key;
        }
        double temp = key * HASH_CONSTANT;
        double fractionalPart = temp - static_cast<long>(temp);
        return static_cast<int>(floor(capacity * fractionalPart));
    }

public:
    // =================== MODIFICATION: INCREASED CAPACITY ===================
    HashTable(int cap = 200) { // Changed default capacity from 50 to 200
    // =================== END OF MODIFICATION ===================
        capacity = cap;
        currentSize = 0;
        table = new Product*[capacity];
        for (int i = 0; i < capacity; i++) {
            table[i] = NULL;
        }
    }

    ~HashTable() {
        for (int i = 0; i < capacity; i++) {
            if (table[i] != NULL) {
                delete table[i];
            }
        }
        delete[] table;
    }

    void add(const Product& product) {
        if (currentSize >= capacity * 0.7) {
            cout << "Warning: Hash table is getting full." << endl;
        }
        
        int index = hash(product.id);
        int probeCount = 0;

        while (table[index] != NULL && table[index]->id != product.id && probeCount < capacity) {
            index = (index + 1) % capacity;
            probeCount++;
        }

        if (probeCount == capacity) {
            setColor(COLOR_ERROR);
            cout << "Error: Hash table is completely full. Cannot add new product.\n";
            setColor(COLOR_DEFAULT);
            Logger::getInstance().logError("Hash table is full. Failed to add product ID: " + intToString(product.id));
            return;
        }

        if (table[index] == NULL) {
            table[index] = new Product(product);
            currentSize++;
            Logger::getInstance().logInfo("Added new product to catalog: " + product.name + " (ID: " + intToString(product.id) + ")");
        } else {
            *(table[index]) = product;
            Logger::getInstance().logInfo("Updated product in catalog: " + product.name + " (ID: " + intToString(product.id) + ")");
        }
    }

    Product* find(int key) {
        int index = hash(key);
        int probeCount = 0;
        
        do {
            if (table[index] != NULL && table[index]->id == key) {
                return table[index];
            }
            index = (index + 1) % capacity;
            probeCount++;
        } while (probeCount < capacity && table[index-1 % capacity] != NULL);

        return NULL;
    }

    bool remove(int key) {
        Product* allProducts = new Product[currentSize];
        int count = getAll(allProducts, currentSize);
        bool found = false;

        for (int i = 0; i < count; i++) {
            if (allProducts[i].id == key) {
                found = true;
                break;
            }
        }
        
        if (!found) {
            delete[] allProducts;
            return false;
        }

        for (int i = 0; i < capacity; i++) {
            if (table[i] != NULL) {
                delete table[i];
                table[i] = NULL;
            }
        }
        this->currentSize = 0;
        
        for (int i = 0; i < count; i++) {
            if (allProducts[i].id != key) {
                add(allProducts[i]);
            }
        }
        delete[] allProducts;
        Logger::getInstance().logInfo("Removed product from catalog with ID: " + intToString(key));
        return true;
    }

    int getAll(Product* productsArray, int maxSize) {
        int count = 0;
        for (int i = 0; i < capacity && count < maxSize; i++) {
            if (table[i] != NULL) {
                productsArray[count++] = *(table[i]);
            }
        }
        return count;
    }
    
    int getSize() const { 
        return currentSize; 
    }
};

// --- 3.2 SORTING ALGORITHM (Merge Sort) ---
void merge(Product arr[], int left, int mid, int right, const string& sortBy, bool ascending) {
    int n1 = mid - left + 1;
    int n2 = right - mid;

    Product* L = new Product[n1];
    Product* R = new Product[n2];

    for (int i = 0; i < n1; i++) {
        L[i] = arr[left + i];
    }
    for (int j = 0; j < n2; j++) {
        R[j] = arr[mid + 1 + j];
    }

    int i = 0;
    int j = 0;
    int k = left;

    while (i < n1 && j < n2) {
        bool condition;
        if (sortBy == "price") {
            if (ascending == true) {
                condition = (L[i].price <= R[j].price);
            } else {
                condition = (L[i].price >= R[j].price);
            }
        } else if (sortBy == "id") {
            if (ascending == true) {
                condition = (L[i].id <= R[j].id);
            } else {
                condition = (L[i].id >= R[j].id);
            }
        } else {
            if (ascending == true) {
                condition = (L[i].name <= R[j].name);
            } else {
                condition = (L[i].name >= R[j].name);
            }
        }

        if (condition) {
            arr[k] = L[i];
            i++;
        } else {
            arr[k] = R[j];
            j++;
        }
        k++;
    }

    while (i < n1) {
        arr[k] = L[i];
        i++;
        k++;
    }
    while (j < n2) {
        arr[k] = R[j];
        j++;
        k++;
    }

    delete[] L;
    delete[] R;
}

void mergeSort(Product arr[], int left, int right, const string& sortBy, bool ascending) {
    if (left >= right) {
        return;
    }
    int mid = left + (right - left) / 2;
    mergeSort(arr, left, mid, sortBy, ascending);
    mergeSort(arr, mid + 1, right, sortBy, ascending);
    merge(arr, left, mid, right, sortBy, ascending);
}

// --- 3.3 SEARCHING ALGORITHM (Binary Search) ---
Product* binarySearch(Product arr[], int low, int high, int key) {
    while (low <= high) {
        int mid = low + (high - low) / 2;
        if (arr[mid].id == key) {
            return &arr[mid];
        }
        if (arr[mid].id < key) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }
    return NULL;
}

Product* findProductById_BinarySearch(HashTable& catalog, int productId) {
    int count = catalog.getSize();
    if (count == 0) {
        return NULL;
    }

    Product* allProducts = new Product[count];
    catalog.getAll(allProducts, count);

    mergeSort(allProducts, 0, count - 1, "id", true);

    Product* foundInTemp = binarySearch(allProducts, 0, count - 1, productId);
    
    delete[] allProducts;

    if (foundInTemp != NULL) {
        return catalog.find(productId);
    }
    
    return NULL;
}


// --- 3.4 LINKED LIST (for Customer's Shopping Cart) ---
struct CartItemNode {
    Product data;
    int quantity;
    CartItemNode* next;

    CartItemNode(const Product& prod, int qty) : data(prod), quantity(qty), next(NULL) {}
};

class ShoppingCart {
private:
    CartItemNode* head;
    int listSize;

public:
    ShoppingCart() : head(NULL), listSize(0) {}

    ~ShoppingCart() {
        clear();
    }
    
    void addProduct(const Product& newProduct, int quantity) {
        CartItemNode* current = head;
        while(current != NULL) {
            if(current->data.id == newProduct.id) {
                current->quantity += quantity;
                return;
            }
            current = current->next;
        }

        CartItemNode* newNode = new CartItemNode(newProduct, quantity);
        if (head == NULL) {
            head = newNode;
        } else {
            current = head;
            while (current->next != NULL) {
                current = current->next;
            }
            current->next = newNode;
        }
        listSize++;
    }

    void displayCart() const {
        if (isEmpty()) {
            cout << "Your shopping cart is empty.\n";
            return;
        }
        CartItemNode* current = head;
        int index = 1;
        double total = 0.0;
        printSubHeader("Your Shopping Cart");
        while (current != NULL) {
            cout << "\n";
            cout << index++ << ". ";
            cout << current->data.name << " (ID: " << current->data.id << ")\n";
            cout << "   Quantity: " << current->quantity << "\n";
            cout << "   Price per unit: $" << fixed << setprecision(2) << current->data.price << "\n";
            cout << "   Subtotal: $" << fixed << setprecision(2) << current->data.price * current->quantity << "\n";
            total += current->data.price * current->quantity;
            current = current->next;
        }
        setColor(COLOR_HEADER);
        cout << "\n-----------------------------------\n";
        cout << "Total Cart Value: $" << fixed << setprecision(2) << total << endl;
        cout << "-----------------------------------\n";
        setColor(COLOR_DEFAULT);
    }
    
    void clear() {
        CartItemNode* current = head;
        while (current != NULL) {
            CartItemNode* toDelete = current;
            current = current->next;
            delete toDelete;
        }
        head = NULL;
        listSize = 0;
    }
    
    bool removeItem(int productId) {
        if (head == NULL) {
            return false;
        }
        
        if (head->data.id == productId) {
            CartItemNode* toDelete = head;
            head = head->next;
            delete toDelete;
            listSize--;
            return true;
        }

        CartItemNode* current = head;
        while(current->next != NULL) {
            if (current->next->data.id == productId) {
                CartItemNode* toDelete = current->next;
                current->next = current->next->next;
                delete toDelete;
                listSize--;
                return true;
            }
            current = current->next;
        }
        
        return false;
    }

    bool updateQuantity(int productId, int newQuantity) {
        if (newQuantity <= 0) {
            return removeItem(productId);
        }

        CartItemNode* current = head;
        while(current != NULL) {
            if(current->data.id == productId) {
                current->quantity = newQuantity;
                return true;
            }
            current = current->next;
        }
        return false;
    }

    bool isEmpty() const { 
        return head == NULL; 
    }
    int getSize() const { 
        return listSize; 
    }
    CartItemNode* getHead() const { 
        return head; 
    }
};

// =================================================================
// 5. USER MANAGEMENT & INHERITANCE STRUCTURE
// =================================================================

class UserManager {
private:
    User* users;
    int userCount;
    int capacity;
    string userFile;

    void grow() {
        capacity *= 2;
        User* temp = new User[capacity];
        for (int i = 0; i < userCount; ++i) {
            temp[i] = users[i];
        }
        delete[] users;
        users = temp;
    }

public:
    UserManager(string filename = "users.txt") : userFile(filename) {
        capacity = 10;
        userCount = 0;
        users = new User[capacity];
        loadUsers();
    }

    ~UserManager() {
        saveUsers();
        delete[] users;
    }

    void loadUsers() {
        ifstream inFile(userFile.c_str());
        if (!inFile) {
            cout << "User file not found. Creating default admin (admin/admin)...\n";
            User defaultAdmin = {"admin", "admin", "admin"};
            addUser(defaultAdmin);
            return;
        }

        string line;
        while (getline(inFile, line)) {
            stringstream ss(line);
            string uname, pwd, role_str;
            getline(ss, uname, ',');
            getline(ss, pwd, ',');
            getline(ss, role_str);

            if (!uname.empty()) {
                User newUser = {uname, pwd, role_str};
                addUser(newUser);
            }
        }
        inFile.close();
    }

    void saveUsers() {
        ofstream outFile(userFile.c_str());
        for (int i = 0; i < userCount; ++i) {
            outFile << users[i].username << "," << users[i].password << "," << users[i].role << endl;
        }
        outFile.close();
    }

    void addUser(const User& user) {
        if (userCount == capacity) {
            grow();
        }
        users[userCount++] = user;
    }

    BaseUser* login(const string& role);
    void signUp(const string& role);

    friend class Admin;
};


class BaseUser {
protected:
    string username;
    string role;
    UserManager& userManager;
    HashTable& productCatalog;

public:
    BaseUser(string uname, string userRole, UserManager& um, HashTable& pc) 
        : username(uname), role(userRole), userManager(um), productCatalog(pc) {
            Logger::getInstance().logInfo("User '" + username + "' logged in as " + role + ".");
        }

    virtual ~BaseUser() {
        Logger::getInstance().logInfo("User '" + username + "' logged out.");
        cout << "\nUser " << username << " logged out successfully.\n";
    }

    string getUsername() const { 
        return username; 
    }
    string getRole() const { 
        return role; 
    }

    virtual void showMenu() = 0;
    virtual void displayAllRecords() = 0;
    virtual void searchRecord() = 0;
    virtual void sortRecords() = 0;
};


class Admin : public BaseUser {
public:
    Admin(string uname, UserManager& um, HashTable& pc) : BaseUser(uname, "admin", um, pc) {}
    
    void showMenu() override;
    void displayAllRecords() override;
    void searchRecord() override;
    void sortRecords() override;
    
    void addProductRecord();
    void addNewCategory();
    void editProductRecord();
    void deleteProductRecord();
    void viewAllUsers();
    void generateSalesReport();
};


class Customer : public BaseUser {
private:
    ShoppingCart myCart;

public:
    Customer(string uname, UserManager& um, HashTable& pc) 
        : BaseUser(uname, "customer", um, pc) {}
    
    void showMenu() override;
    void displayAllRecords() override;
    void searchRecord() override;
    void sortRecords() override;

    void addProductToCart();
    void viewCart();
    void editCart();
    void checkout();
    void viewPurchaseHistory();
};

// =================================================================
// 6. REPORT GENERATOR (POLYMORPHISM EXAMPLE)
// =================================================================

class ReportGenerator {
protected:
    string reportContent;
    string reportFilename;
public:
    ReportGenerator(string fname = "reports.txt") : reportFilename(fname) {}
    virtual ~ReportGenerator() {}

    virtual void generate(const string& username = "") = 0;

    void display() const {
        printSubHeader("Generated Report");
        cout << reportContent << endl;
    }
    
    void save() {
        ofstream outFile(reportFilename.c_str(), ios::app);
        if(outFile) {
            outFile << "--- REPORT START [" << __TIME__ << " " << __DATE__ << "] ---\n";
            outFile << reportContent;
            outFile << "--- REPORT END ---\n\n";
            outFile.close();
            setColor(COLOR_SUCCESS);
            cout << "Report appended to " << reportFilename << endl;
            setColor(COLOR_DEFAULT);
        } else {
            setColor(COLOR_ERROR);
            cout << "Error saving report to file.\n";
            setColor(COLOR_DEFAULT);
        }
    }
};

class InventoryReport : public ReportGenerator {
private:
    HashTable& productCatalog;
public:
    InventoryReport(HashTable& pc) : productCatalog(pc) {}
    
    void generate(const string& username = "") override {
        stringstream ss;
        ss << "Inventory Status Report\n";
        ss << "------------------------------------\n";
        ss << "Generated for: Admin (" << username << ")\n\n";

        Product allProducts[100];
        int count = productCatalog.getAll(allProducts, 100);
        
        ss << "Low Stock Products (<= 5 units):\n";
        bool lowStockFound = false;
        for (int i = 0; i < count; i++) {
            if (allProducts[i].quantityInStock <= 5 && allProducts[i].quantityInStock > 0) {
                ss << "  - ID: " << allProducts[i].id << ", Name: " << allProducts[i].name 
                   << ", Stock: " << allProducts[i].quantityInStock << "\n";
                lowStockFound = true;
            }
        }
        if (!lowStockFound) {
            ss << "  None\n";
        }

        ss << "\nOut of Stock Products:\n";
        bool outOfStockFound = false;
        for (int i = 0; i < count; i++) {
            if (allProducts[i].quantityInStock == 0) {
                ss << "  - ID: " << allProducts[i].id << ", Name: " << allProducts[i].name << "\n";
                outOfStockFound = true;
            }
        }
        if (!outOfStockFound) {
            ss << "  None\n";
        }

        reportContent = ss.str();
    }
};

class SalesReport : public ReportGenerator {
public:
    void generate(const string& username = "") override {
        stringstream ss;
        ss << "Overall Sales Report\n";
        ss << "---------------------------\n";
        
        ifstream inFile("purchase_history.txt");
        if (!inFile) {
            ss << "No purchase history data found.\n";
            reportContent = ss.str();
            return;
        }

        double totalRevenue = 0;
        int totalItemsSold = 0;
        string line;
        
        while (getline(inFile, line)) {
            stringstream line_ss(line);
            string user;
            string prodId;
            string prodName;
            string priceStr;
            string qtyStr;
            getline(line_ss, user, ',');
            getline(line_ss, prodId, ',');
            getline(line_ss, prodName, ',');
            getline(line_ss, priceStr, ',');
            getline(line_ss, qtyStr, ',');
            
            double price = atof(priceStr.c_str());
            int qty = atoi(qtyStr.c_str());
            totalRevenue += price * qty;
            totalItemsSold += qty;
        }
        inFile.close();

        ss << "Summary of All Sales:\n";
        ss << "  Total Items Sold: " << totalItemsSold << "\n";
        ss << "  Total Revenue Generated: $" << fixed << setprecision(2) << totalRevenue << "\n";

        reportContent = ss.str();
    }
};

// =================================================================
// 7. CLASS METHOD IMPLEMENTATIONS
// =================================================================

BaseUser* UserManager::login(const string& expectedRole) {
    if (expectedRole == "admin") {
        printHeader("Admin Login");
    } else {
        printHeader("Customer Login");
    }

    string inputUsername;
    string inputPassword;

    for (int attempts = 0; attempts < 3; ++attempts) {
        cout << "Enter Username: ";
        cin >> inputUsername;
        cout << "Enter Password: ";
        
        setColor(COLOR_INPUT);
        inputPassword = "";
        char ch;
        while((ch = _getch()) != '\r') {
            if(ch == '\b') { 
                if(!inputPassword.empty()) {
                    inputPassword.erase(inputPassword.length() - 1);
                    cout << "\b \b";
                }
            } else {
                inputPassword += ch;
                cout << '*';
            }
        }
        cout << endl;
        setColor(COLOR_DEFAULT);

        for (int i = 0; i < userCount; ++i) {
            if (users[i].username == inputUsername && users[i].password == inputPassword) {
                if(users[i].role == expectedRole) {
                    setColor(COLOR_SUCCESS);
                    cout << "\n";
                    cout << "Login successful! Welcome " << users[i].username << ".\n";
                    setColor(COLOR_DEFAULT);
                    pressKeyToContinue();
                    
                    extern HashTable productCatalog;
                    if (expectedRole == "admin") {
                        return new Admin(inputUsername, *this, productCatalog);
                    } else {
                        return new Customer(inputUsername, *this, productCatalog);
                    }
                } else {
                    setColor(COLOR_ERROR);
                    cout << "\n";
                    cout << "Access Denied: You have " << users[i].role << " credentials, not " << expectedRole << ".\n";
                    setColor(COLOR_DEFAULT);
                    pressKeyToContinue();
                    return NULL;
                }
            }
        }

        setColor(COLOR_ERROR);
        cout << "\n";
        cout << "Invalid credentials. You have " << 2 - attempts << " attempts left.\n";
        setColor(COLOR_DEFAULT);
    }

    cout << "\n";
    cout << "Login failed after 3 attempts. Returning to main menu.\n";
    Logger::getInstance().logWarning("Failed login attempt for role: " + expectedRole);
    pressKeyToContinue();
    return NULL;
}

void UserManager::signUp(const string& roleToAssign) {
    if (roleToAssign == "admin") {
        printHeader("New Admin Registration");
    } else {
        printHeader("New Customer Registration");
    }

    string newUsername;
    string newPassword;
    string confirmPassword;
    
    if (roleToAssign == "admin") {
        cout << "Enter new Admin username: ";
    } else {
        cout << "Enter new Customer username: ";
    }
    cin >> newUsername;

    for (int i = 0; i < userCount; ++i) {
        if (users[i].username == newUsername) {
            setColor(COLOR_ERROR);
            cout << "Username '" << newUsername << "' already exists. Please choose another.\n";
            setColor(COLOR_DEFAULT);
            Logger::getInstance().logWarning("Sign up failed: Username " + newUsername + " already exists.");
            pressKeyToContinue();
            return;
        }
    }
    
    cout << "Enter new password: ";
    cin >> newPassword;
    cout << "Confirm password: ";
    cin >> confirmPassword;

    if (newPassword != confirmPassword) {
        setColor(COLOR_ERROR);
        cout << "Passwords do not match. Registration failed.\n";
        setColor(COLOR_DEFAULT);
        Logger::getInstance().logWarning("Sign up failed for " + newUsername + ": Passwords did not match.");
        pressKeyToContinue();
        return;
    }

    User newUser = {newUsername, newPassword, roleToAssign};
    addUser(newUser);
    saveUsers();
    
    setColor(COLOR_SUCCESS);
    cout << "\n";
    if (roleToAssign == "admin") {
        cout << "Registration for Admin '" << newUsername << "' successful!\n";
    } else {
        cout << "Registration for Customer '" << newUsername << "' successful!\n";
    }
    setColor(COLOR_DEFAULT);
    Logger::getInstance().logInfo("New user registered: " + newUsername + " as " + roleToAssign);
    pressKeyToContinue();
}

void Admin::showMenu() {
    int choice;
    do {
        printHeader("Admin Menu | Welcome, " + username);
        cout << "1. Display Records (Products/Users)\n";
        cout << "2. Search for a Product\n";
        cout << "3. Sort and Display Products\n";
        cout << "4. Add New Product\n";
        cout << "5. Add New Category\n";
        cout << "6. Edit Product Details\n";
        cout << "7. Delete a Product\n";
        cout << "8. Generate Inventory & Sales Reports\n";
        cout << "0. Logout\n";
        cout << "\n";
        cout << "Enter your choice: ";
        
        cin.clear();
        fflush(stdin);
        choice = getIntegerInput();

        try {
            switch (choice) {
                case 1: 
                    displayAllRecords(); 
                    break;
                case 2: 
                    searchRecord(); 
                    break;
                case 3: 
                    sortRecords(); 
                    break;
                case 4: 
                    addProductRecord(); 
                    break;
                case 5:
                    addNewCategory();
                    break;
                case 6:
                    editProductRecord(); 
                    break;
                case 7:
                    deleteProductRecord(); 
                    break;
                case 8:
                    generateSalesReport(); 
                    break;
                case 0: 
                    break;
                default:
                    throw invalid_argument("Invalid choice. Please enter a number between 0 and 8.");
            }
        }
        catch (const exception& e) {
            setColor(COLOR_ERROR);
            cout << "Error: " << e.what() << endl;
            setColor(COLOR_DEFAULT);
            Logger::getInstance().logError("Exception in Admin Menu: " + string(e.what()));
            pressKeyToContinue();
        }
    } while (choice != 0);
}

void displayProductTable(Product arr[], int count) {
    int totalWidth = 90; 
    setColor(COLOR_HEADER);
    cout << left;
    cout << string(totalWidth, '=') << endl;
    cout << setw(5) << "ID";
    cout << setw(30) << "Name";
    cout << setw(25) << "Category";
    cout << setw(15) << "Price";
    cout << setw(10) << "In Stock" << endl;
    cout << string(totalWidth, '-') << endl;
    setColor(COLOR_DEFAULT);

    for (int i = 0; i < count; i++) {
        if (arr[i].quantityInStock <= 5 && arr[i].quantityInStock > 0) {
            setColor(COLOR_WARNING);
        } else if (arr[i].quantityInStock == 0) {
            setColor(COLOR_ERROR);
        }

        stringstream priceStream;
        priceStream << "$" << fixed << setprecision(2) << arr[i].price;

        cout << setw(5) << arr[i].id;
        cout << setw(30) << arr[i].name;
        cout << setw(25) << arr[i].category;
        cout << setw(15) << priceStream.str();
        cout << setw(10) << arr[i].quantityInStock << endl;

        setColor(COLOR_DEFAULT);
    }
    setColor(COLOR_HEADER);
    cout << string(totalWidth, '=') << endl;
    setColor(COLOR_DEFAULT);
}


void Admin::displayAllRecords() {
    printHeader("Display Records");
    cout << "1. All Products in Catalog\n";
    cout << "2. All Registered Users\n";
    cout << "Enter choice: ";
    int choice = getIntegerInput();

    if (choice == 1) {
        printSubHeader("Full Product Catalog");
        Product allProducts[100];
        int count = productCatalog.getAll(allProducts, 100);
        if (count == 0) {
            cout << "No products in the catalog.\n";
        } else {
            displayProductTable(allProducts, count);
        }
    } else if (choice == 2) {
        viewAllUsers();
    } else {
        setColor(COLOR_ERROR);
        cout << "Invalid choice.\n";
        setColor(COLOR_DEFAULT);
    }
    pressKeyToContinue();
}

void Admin::viewAllUsers(){
    printSubHeader("All Registered Users");
    int totalWidth = 50;
    setColor(COLOR_HEADER);
    cout << string(totalWidth, '=') << endl;
    cout << left << setw(25) << "Username" << setw(25) << "Role" << endl;
    cout << string(totalWidth, '-') << endl;
    setColor(COLOR_DEFAULT);
    for(int i = 0; i < userManager.userCount; ++i) {
        cout << left << setw(25) << userManager.users[i].username;
        cout << setw(25) << userManager.users[i].role << endl;
    }
    setColor(COLOR_HEADER);
    cout << string(totalWidth, '=') << endl;
    setColor(COLOR_DEFAULT);
}

void Admin::searchRecord() {
    printHeader("Search for a Product");
    cout << "Enter the ID of the product to search for: ";
    int id = getIntegerInput();

    Product* foundProduct = findProductById_BinarySearch(productCatalog, id);

    if(foundProduct) {
        setColor(COLOR_SUCCESS);
        cout << "\nProduct Found:\n";
        setColor(COLOR_DEFAULT);
        foundProduct->display();
        showProductPriceWithTax(*foundProduct);
    } else {
        setColor(COLOR_ERROR);
        cout << "\nProduct with ID " << id << " not found in the catalog.\n";
        setColor(COLOR_DEFAULT);
    }
    pressKeyToContinue();
}

void Admin::sortRecords() {
    printHeader("Sort Products");
    Product allProducts[100];
    int count = productCatalog.getAll(allProducts, 100);
    if (count == 0) {
        cout << "No products to sort.\n";
        pressKeyToContinue();
        return;
    }

    cout << "Sort by:\n";
    cout << "1. Price\n";
    cout << "2. Name\n";
    cout << "3. ID\n";
    cout << "Enter choice: ";
    int sortChoice = getIntegerInput();
    
    cout << "Order:\n";
    cout << "1. Ascending\n";
    cout << "2. Descending\n";
    cout << "Enter choice: ";
    int orderChoice = getIntegerInput();

    string sortByCriteria;
    switch (sortChoice) {
        case 1:
            sortByCriteria = "price";
            break;
        case 2:
            sortByCriteria = "name";
            break;
        case 3:
            sortByCriteria = "id";
            break;
        default:
            cout << "Invalid sort choice. Defaulting to sort by ID.\n";
            sortByCriteria = "id";
            break;
    }
    
    bool isAscending;
    if (orderChoice == 1) {
        isAscending = true;
    } else {
        isAscending = false;
    }

    mergeSort(allProducts, 0, count - 1, sortByCriteria, isAscending);
    
    printSubHeader("Sorted Product List");
    displayProductTable(allProducts, count);
    pressKeyToContinue();
}

// =================== MODIFICATION: GLOBAL CATEGORY LIST ===================
vector<string> categories;
// =================== END OF MODIFICATION ===================

void Admin::addProductRecord() {
    printHeader("Add New Product");
    int id;
    int quantity;
    string name;
    string category;
    double price;

    if (categories.empty()) {
        setColor(COLOR_ERROR);
        cout << "No categories exist. Please add a category first.\n";
        setColor(COLOR_DEFAULT);
        pressKeyToContinue();
        return;
    }

    cout << "Enter Product ID: ";
    id = getIntegerInput();

    if(findProductById_BinarySearch(productCatalog, id) != NULL){
        setColor(COLOR_ERROR);
        cout << "Error: A product with ID " << id << " already exists.\n";
        setColor(COLOR_DEFAULT);
        pressKeyToContinue();
        return;
    }

    cout << "Enter Product Name: "; 
    getline(cin, name);

    printSubHeader("Select a Category");
    for (size_t i = 0; i < categories.size(); ++i) {
        cout << i + 1 << ". " << categories[i] << endl;
    }
    cout << "Enter category number: ";
    int categoryChoice = getIntegerInput();

    if (categoryChoice < 1 || categoryChoice > categories.size()) {
        setColor(COLOR_ERROR);
        cout << "Invalid category selection.\n";
        setColor(COLOR_DEFAULT);
        pressKeyToContinue();
        return;
    }
    category = categories[categoryChoice - 1];

    cout << "Enter Price: $";
    string priceStr;
    getline(cin, priceStr);
    price = atof(priceStr.c_str());

    cout << "Enter Initial Stock Quantity: "; 
    quantity = getIntegerInput();

    Product newProduct(id, name, category, price, quantity);
    productCatalog.add(newProduct);

    setColor(COLOR_SUCCESS);
    cout << "\n";
    cout << "Product '" << name << "' added successfully to the catalog.\n";
    setColor(COLOR_DEFAULT);
    pressKeyToContinue();
}

void saveCategoriesToFile() {
    ofstream outFile("categories.txt");
    if (!outFile) {
        Logger::getInstance().logError("Could not open categories.txt for writing.");
        return;
    }
    for (size_t i = 0; i < categories.size(); ++i) {
        outFile << categories[i] << endl;
    }
    outFile.close();
}

void Admin::addNewCategory() {
    printHeader("Add New Category");

    string newCategory;
    cout << "Enter the name for the new category: ";
    getline(cin, newCategory);

    if (newCategory.empty()) {
        setColor(COLOR_ERROR);
        cout << "Category name cannot be empty.\n";
        setColor(COLOR_DEFAULT);
        pressKeyToContinue();
        return;
    }

    for (size_t i = 0; i < categories.size(); ++i) {
        if (categories[i] == newCategory) {
            setColor(COLOR_ERROR);
            cout << "Category '" << newCategory << "' already exists.\n";
            setColor(COLOR_DEFAULT);
            pressKeyToContinue();
            return;
        }
    }

    categories.push_back(newCategory);
    saveCategoriesToFile();

    setColor(COLOR_SUCCESS);
    cout << "\nCategory '" << newCategory << "' added successfully.\n";
    setColor(COLOR_DEFAULT);
    Logger::getInstance().logInfo("Admin " + username + " added new category: " + newCategory);
    pressKeyToContinue();
}

void Admin::editProductRecord() {
    printHeader("Edit Product Details");
    cout << "Enter the ID of the product to edit: ";
    int id = getIntegerInput();

    Product* productToEdit = findProductById_BinarySearch(productCatalog, id);

    if (!productToEdit) {
        setColor(COLOR_ERROR);
        cout << "Product with ID " << id << " not found.\n";
        setColor(COLOR_DEFAULT);
        pressKeyToContinue();
        return;
    }

    cout << "\n";
    cout << "Editing Product: " << productToEdit->name << "\n";
    cout << "Press Enter to keep the current value.\n";

    string input;
    cout << "Enter New Name (current: " << productToEdit->name << "): ";
    getline(cin, input);
    if (!input.empty()) {
        productToEdit->name = input;
    }

    cout << "Select New Category (current: " << productToEdit->category << "). Press Enter to skip.\n";
    for (size_t i = 0; i < categories.size(); ++i) {
        cout << i + 1 << ". " << categories[i] << endl;
    }
    cout << "Enter category number (or just press Enter to keep current): ";
    getline(cin, input);
    if (!input.empty()) {
        int categoryChoice = atoi(input.c_str());
        if (categoryChoice >= 1 && categoryChoice <= categories.size()) {
            productToEdit->category = categories[categoryChoice - 1];
        } else {
            cout << "Invalid selection. Category not changed.\n";
        }
    }
    
    cout << "Enter New Price (current: " << productToEdit->price << "): $";
    getline(cin, input);
    if (!input.empty()) {
        productToEdit->price = atof(input.c_str());
    }
    
    cout << "Enter New Stock (current: " << productToEdit->quantityInStock << "): ";
    getline(cin, input);
    if (!input.empty()) {
        productToEdit->quantityInStock = atoi(input.c_str());
    }

    productCatalog.add(*productToEdit); 

    setColor(COLOR_SUCCESS);
    cout << "\n";
    cout << "Product details updated successfully.\n";
    setColor(COLOR_DEFAULT);
    pressKeyToContinue();
}

void Admin::deleteProductRecord() {
    printHeader("Delete Product");
    cout << "Enter the ID of the product to delete: ";
    int id = getIntegerInput();

    if (productCatalog.remove(id)) {
        setColor(COLOR_SUCCESS);
        cout << "Product with ID " << id << " has been deleted.\n";
        setColor(COLOR_DEFAULT);
    } else {
        setColor(COLOR_ERROR);
        cout << "Product with ID " << id << " not found.\n";
        setColor(COLOR_DEFAULT);
    }
    pressKeyToContinue();
}

void Admin::generateSalesReport() {
    printHeader("Generate Reports");
    cout << "Which report would you like to generate?\n";
    cout << "1. Inventory Status Report\n";
    cout << "2. Overall Sales Report\n";
    cout << "Enter choice: ";
    int choice = getIntegerInput();

    if (choice == 1) {
        InventoryReport report(productCatalog);
        report.generate(username);
        report.display();
        cout << "\nDo you want to save this report? (y/n): ";
        char saveChoice;
        cin >> saveChoice;
        if(tolower(saveChoice) == 'y') {
            report.save();
        }
    } else if (choice == 2) {
        SalesReport report;
        report.generate();
        report.display();
        cout << "\nDo you want to save this report? (y/n): ";
        char saveChoice;
        cin >> saveChoice;
        if(tolower(saveChoice) == 'y') {
            report.save();
        }
    } else {
        cout << "Invalid choice.\n";
    }

    pressKeyToContinue();
}

void Customer::showMenu() {
    int choice;
    do {
        printHeader("Customer Menu | Welcome, " + username);
        cout << "1. Add Products to Cart\n";
        cout << "2. View Shopping Cart\n";
        cout << "3. Edit Shopping Cart\n";
        cout << "4. Confirm Order / Checkout\n";
        cout << "5. Search for Products by Name\n";
        cout << "6. Sort and Browse Products\n";
        cout << "7. View My Purchase History\n";
        cout << "0. Logout\n";
        cout << "\n";
        cout << "Enter your choice: ";

        cin.clear();
        fflush(stdin);
        choice = getIntegerInput();

        try {
            switch (choice) {
                case 1: 
                    addProductToCart(); 
                    break;
                case 2: 
                    viewCart(); 
                    break;
                case 3:
                    editCart();
                    break;
                case 4: 
                    checkout(); 
                    break;
                case 5: 
                    searchRecord(); 
                    break;
                case 6:
                    sortRecords(); 
                    break;
                case 7:
                    viewPurchaseHistory(); 
                    break;
                case 0: 
                    break;
                default:
                    throw invalid_argument("Invalid choice. Please enter a number between 0 and 7.");
            }
        }
        catch (const exception& e) {
            setColor(COLOR_ERROR);
            cout << "Error: " << e.what() << endl;
            setColor(COLOR_DEFAULT);
            Logger::getInstance().logError("Exception in Customer Menu: " + string(e.what()));
            pressKeyToContinue();
        }
    } while (choice != 0);
}

void Customer::addProductToCart() {
    printHeader("Add Products to Cart");
    printSubHeader("Available Products");
    
    Product allProducts[100];
    int count = productCatalog.getAll(allProducts, 100);
    if(count == 0){
        cout << "Sorry, no products are available in the catalog right now.\n";
        pressKeyToContinue();
        return;
    }
    
    mergeSort(allProducts, 0, count-1, "id", true);
    displayProductTable(allProducts, count);
    
    cout << "\n";
    cout << "Enter the ID of the product you want to add (0 to back to menu): ";
    int id = getIntegerInput();
    if(id == 0) {
        return;
    }

    Product* productToAdd = findProductById_BinarySearch(productCatalog, id);

    if(productToAdd) {
        cout << "Enter quantity for '" << productToAdd->name << "': ";
        int qty = getIntegerInput();

        if (qty > 0 && qty <= productToAdd->quantityInStock) {
            myCart.addProduct(*productToAdd, qty);
            setColor(COLOR_SUCCESS);
            cout << "\n";
            cout << qty << " unit(s) of '" << productToAdd->name << "' added to your cart.\n";
            setColor(COLOR_DEFAULT);
        } else if (qty > productToAdd->quantityInStock) {
            setColor(COLOR_ERROR);
            cout << "Sorry, the requested quantity (" << qty 
                 << ") exceeds the available stock (" << productToAdd->quantityInStock << ").\n";
            setColor(COLOR_DEFAULT);
        } else {
            setColor(COLOR_ERROR);
            cout << "Invalid quantity.\n";
            setColor(COLOR_DEFAULT);
        }
    } else {
        setColor(COLOR_ERROR);
        cout << "\nProduct with ID " << id << " not found.\n";
        setColor(COLOR_DEFAULT);
    }
    pressKeyToContinue();
}

void Customer::viewCart(){
    printHeader("My Shopping Cart");
    myCart.displayCart();
    pressKeyToContinue();
}

void Customer::editCart() {
    int choice;
    do {
        printHeader("Edit Shopping Cart");

        if (myCart.isEmpty()) {
            cout << "Your cart is empty. Nothing to edit.\n";
            pressKeyToContinue();
            return;
        }

        myCart.displayCart();

        cout << "\n";
        printSubHeader("Edit Options");
        cout << "1. Update Item Quantity\n";
        cout << "2. Remove Item from Cart\n";
        cout << "0. Back to Customer Menu\n";
        cout << "\nEnter your choice: ";

        choice = getIntegerInput();

        if (choice == 1) {
            cout << "\nEnter the ID of the product to update: ";
            int id = getIntegerInput();

            Product* productInCatalog = findProductById_BinarySearch(productCatalog, id);

            if (!productInCatalog) {
                setColor(COLOR_ERROR);
                cout << "This product ID does not exist in the main catalog.\n";
                setColor(COLOR_DEFAULT);
                pressKeyToContinue();
                continue;
            }

            cout << "Enter the new quantity (enter 0 to remove): ";
            int newQty = getIntegerInput();

            if (newQty > productInCatalog->quantityInStock) {
                setColor(COLOR_ERROR);
                cout << "Error: New quantity (" << newQty 
                     << ") exceeds available stock (" << productInCatalog->quantityInStock << ").\n";
                setColor(COLOR_DEFAULT);
            } else {
                if(myCart.updateQuantity(id, newQty)) {
                    setColor(COLOR_SUCCESS);
                    cout << "Cart updated successfully.\n";
                    setColor(COLOR_DEFAULT);
                } else {
                    setColor(COLOR_ERROR);
                    cout << "Product with ID " << id << " not found in your cart.\n";
                    setColor(COLOR_DEFAULT);
                }
            }
            pressKeyToContinue();
        }
        else if (choice == 2) {
            cout << "\nEnter the ID of the product to remove: ";
            int id = getIntegerInput();
            if (myCart.removeItem(id)) {
                setColor(COLOR_SUCCESS);
                cout << "Product removed from cart successfully.\n";
                setColor(COLOR_DEFAULT);
            } else {
                setColor(COLOR_ERROR);
                cout << "Product with ID " << id << " not found in your cart.\n";
                setColor(COLOR_DEFAULT);
            }
            pressKeyToContinue();
        }
        else if (choice != 0) {
            setColor(COLOR_ERROR);
            cout << "Invalid choice. Please try again.\n";
            setColor(COLOR_DEFAULT);
            pressKeyToContinue();
        }

    } while (choice != 0);
}

void Customer::checkout() {
    printHeader("Checkout");
    if (myCart.isEmpty()) {
        cout << "Your cart is empty. Nothing to check out.\n";
        pressKeyToContinue();
        return;
    }

    myCart.displayCart();
    cout << "\n";
    cout << "Do you want to confirm this purchase? (y/n): ";
    char confirm;
    cin >> confirm;
    cin.ignore(1000, '\n'); // Clear buffer after char input

    if (tolower(confirm) == 'y') {
        CartItemNode* current = myCart.getHead();
        ofstream historyFile("purchase_history.txt", ios::app);

        while(current != NULL) {
            Product* productInStock = findProductById_BinarySearch(productCatalog, current->data.id);
            if(productInStock && productInStock->quantityInStock >= current->quantity) {
                productInStock->quantityInStock -= current->quantity;
                historyFile << username << "," << current->data.id << "," << current->data.name << "," 
                            << current->data.price << "," << current->quantity << endl;
            }
            current = current->next;
        }
        historyFile.close();
        
        myCart.clear();
        setColor(COLOR_SUCCESS);
        cout << "\n";
        cout << "Purchase successful! Thank you for your order.\n";
        setColor(COLOR_DEFAULT);
        Logger::getInstance().logInfo("User " + username + " completed a purchase.");

    } else {
        cout << "Purchase cancelled.\n";
        Logger::getInstance().logInfo("User " + username + " cancelled checkout.");
    }

    pressKeyToContinue();
}

void Customer::viewPurchaseHistory() {
    printHeader("My Purchase History");
    ifstream inFile("purchase_history.txt");
    if (!inFile) {
        cout << "No purchase history found.\n";
        pressKeyToContinue();
        return;
    }
    
    string line;
    bool foundRecords = false;
    printSubHeader("Items you have purchased:");
    while(getline(inFile, line)) {
        stringstream ss(line);
        string user, prodId, prodName, priceStr, qtyStr;
        getline(ss, user, ',');
        if (user == username) {
            foundRecords = true;
            getline(ss, prodId, ',');
            getline(ss, prodName, ',');
            getline(ss, priceStr, ',');
            getline(ss, qtyStr, ',');
            cout << " - Product: " << prodName << " (ID: " << prodId << "), Quantity: " << qtyStr << ", Price: $" << priceStr << endl;
        }
    }
    if(!foundRecords) {
        cout << "You have not made any purchases yet.\n";
    }
    inFile.close();
    pressKeyToContinue();
}

void Customer::searchRecord() {
    printHeader("Search Product Catalog by Name");
    Product allProducts[100];
    int count = productCatalog.getAll(allProducts, 100);
    if (count == 0) {
        cout << "Product catalog is empty. Nothing to search.\n";
        pressKeyToContinue();
        return;
    }

    string searchTerm;
    cout << "Enter a product name or the first letter(s) to search for: ";
    cin.clear();
    fflush(stdin);
    getline(cin, searchTerm);

    if (searchTerm.empty()) {
        cout << "Search term cannot be empty.\n";
        pressKeyToContinue();
        return;
    }

    string lowerSearchTerm = "";
    for (size_t i = 0; i < searchTerm.length(); i++) {
        lowerSearchTerm += tolower(searchTerm[i]);
    }

    Product foundProducts[100];
    int foundCount = 0;

    for (int i = 0; i < count; i++) {
        string productName = allProducts[i].name;
        if (productName.length() >= lowerSearchTerm.length()) {
            string prefix = productName.substr(0, lowerSearchTerm.length());
            
            string lowerPrefix = "";
            for (size_t j = 0; j < prefix.length(); j++) {
                lowerPrefix += tolower(prefix[j]);
            }

            if (lowerPrefix == lowerSearchTerm) {
                if (foundCount < 100) {
                    foundProducts[foundCount] = allProducts[i];
                    foundCount++;
                }
            }
        }
    }

    if (foundCount > 0) {
        setColor(COLOR_SUCCESS);
        cout << "\nFound " << foundCount << " product(s) starting with '" << searchTerm << "':\n";
        setColor(COLOR_DEFAULT);
        mergeSort(foundProducts, 0, foundCount - 1, "name", true); 
        displayProductTable(foundProducts, foundCount);
    } else {
        setColor(COLOR_ERROR);
        cout << "\nNo products found starting with '" << searchTerm << "'.\n";
        setColor(COLOR_DEFAULT);
    }

    pressKeyToContinue();
}

void Customer::sortRecords() {
    printHeader("Browse Sorted Product Catalog");
    Product allProducts[100];
    int count = productCatalog.getAll(allProducts, 100);
    if (count == 0) {
        cout << "No products to sort.\n";
        pressKeyToContinue();
        return;
    }

    cout << "Sort by:\n";
    cout << "1. Price\n";
    cout << "2. Name\n";
    cout << "3. ID\n";
    cout << "Enter choice: ";
    int sortChoice = getIntegerInput();
    
    cout << "Order:\n";
    cout << "1. Ascending\n";
    cout << "2. Descending\n";
    cout << "Enter choice: ";
    int orderChoice = getIntegerInput();

    string sortByCriteria;
    switch (sortChoice) {
        case 1:
            sortByCriteria = "price";
            break;
        case 2:
            sortByCriteria = "name";
            break;
        case 3:
            sortByCriteria = "id";
            break;
        default:
            cout << "Invalid sort choice. Defaulting to sort by ID.\n";
            sortByCriteria = "id";
            break;
    }
    
    bool isAscending;
    if (orderChoice == 1) {
        isAscending = true;
    } else {
        isAscending = false;
    }

    mergeSort(allProducts, 0, count - 1, sortByCriteria, isAscending);
    
    printSubHeader("Sorted Product Catalog");
    displayProductTable(allProducts, count);
    pressKeyToContinue();
}

void Customer::displayAllRecords(){
    printHeader("Browse Full Product Catalog");
    Product allProducts[100];
    int count = productCatalog.getAll(allProducts, 100);
    if (count == 0) {
        cout << "No products in the catalog.\n";
    } else {
        displayProductTable(allProducts, count);
    }
    pressKeyToContinue();
}

void saveProductCatalogToFile(HashTable& catalog) {
    ofstream outFile("products.txt");
    if(!outFile){
        Logger::getInstance().logError("Could not open products.txt for writing.");
        return;
    }
    Product allProducts[100];
    int count = catalog.getAll(allProducts, 100);
    mergeSort(allProducts, 0, count-1, "id", true);
    for (int i = 0; i < count; i++) {
        outFile << allProducts[i].id << "," << allProducts[i].name << "," << allProducts[i].category << ","
                << fixed << setprecision(2) << allProducts[i].price << "," << allProducts[i].quantityInStock << endl;
    }
    outFile.close();
}

void loadProductCatalogFromFile(HashTable& catalog) {
    ifstream inFile("products.txt");
    if (!inFile) {
        return;
    }
    string line;
    while(getline(inFile, line)) {
        stringstream ss(line);
        string id_str, name, cat, price_str, qty_str;
        getline(ss, id_str, ',');
        getline(ss, name, ',');
        getline(ss, cat, ',');
        getline(ss, price_str, ',');
        getline(ss, qty_str, ',');
        if(!id_str.empty()){
            catalog.add(Product(atoi(id_str.c_str()), name, cat, atof(price_str.c_str()), atoi(qty_str.c_str())));
        }
    }
    inFile.close();
}

// =================== MODIFICATION: NEW CATEGORY FILE I/O ===================
void loadCategoriesFromFile() {
    ifstream inFile("categories.txt");
    if (!inFile) {
        // Create default categories if file doesn't exist
        categories.push_back("Kitchen");
        categories.push_back("Living Room");
        categories.push_back("Bedroom");
        categories.push_back("Uncategorized");
        saveCategoriesToFile();
        return;
    }
    string line;
    while(getline(inFile, line)) {
        if (!line.empty()) {
            categories.push_back(line);
        }
    }
    inFile.close();
}
// =================== END OF MODIFICATION ===================


// Global objects
UserManager userManager;
HashTable productCatalog;


// =================================================================
// 8. MAIN DRIVER
// =================================================================

int main() {
    Logger::getInstance().logInfo("System Startup.");
    loadProductCatalogFromFile(productCatalog);
    // =================== MODIFICATION: LOAD CATEGORIES ===================
    loadCategoriesFromFile();
    // =================== END OF MODIFICATION ===================

    int choice;
    do {
        printHeader("Home Appliance Shop - Main Menu");
        cout << "1. Admin Portal\n";
        cout << "2. Customer Portal\n";
        cout << "0. Exit System\n";
        cout << "\n";
        cout << "Enter your choice: ";
        
        cin.clear();
        fflush(stdin);
        choice = getIntegerInput();

        if (choice == 1) {
            printHeader("Admin Portal");
            cout << "1. Login\n";
            cout << "2. Register New Admin\n";
            cout << "0. Back\n";
            cout << "Enter choice: ";
            int adminChoice = getIntegerInput();
            BaseUser* currentUser = NULL;
            if(adminChoice == 1) {
                currentUser = userManager.login("admin");
            } else if (adminChoice == 2) {
                userManager.signUp("admin");
            }
            
            if(currentUser != NULL){
                currentUser->showMenu();
                delete currentUser;
                currentUser = NULL;
            }

        } else if (choice == 2) {
            printHeader("Customer Portal");
            cout << "1. Login\n";
            cout << "2. Register New Customer\n";
            cout << "0. Back\n";
            cout << "Enter choice: ";
            int custChoice = getIntegerInput();
            BaseUser* currentUser = NULL;
            if(custChoice == 1) {
                currentUser = userManager.login("customer");
            } else if(custChoice == 2) {
                userManager.signUp("customer");
            }

            if(currentUser != NULL){
                currentUser->showMenu();
                delete currentUser;
                currentUser = NULL;
            }

        } else if (choice == 0) {
            setColor(COLOR_SUCCESS);
            cout << "Saving all data and exiting system. Goodbye!\n";
            setColor(COLOR_DEFAULT);
        } else {
            setColor(COLOR_ERROR);
            cout << "Invalid choice. Please try again.\n";
            setColor(COLOR_DEFAULT);
            pressKeyToContinue();
        }

    } while (choice != 0);

    saveProductCatalogToFile(productCatalog);
    Logger::getInstance().logInfo("System Shutdown.");
    return 0;
}


