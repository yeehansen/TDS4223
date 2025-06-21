#include <iostream>
#include <fstream>
#include <string>
#include <stdexcept>
#include <conio.h>
#include <windows.h>
#include <cctype>
#include <cstdlib> 
#include <limits>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <direct.h>

using namespace std;

// ANSI Color Codes
#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define LIGHT_BLUE  "\033[94m"

// --- FORWARD DECLARATIONS ---
struct Product;

// --- UTILITY FUNCTIONS ---
void setColor(const char* color) {
    cout << color;
}

void resetColor() {
    cout << RESET;
}

void getConsoleSize(int& width, int& height) {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    } else {
        width = 80;
        height = 25;
    }
}

void showLoading() {
    int console_width, console_height;
    getConsoleSize(console_width, console_height);
    int barWidth = 50;
    string loadingText = "Loading...";
    string prefix = loadingText + " [";
    string suffix = "]";
    int content_width = prefix.length() + barWidth + suffix.length();
    int horizontal_padding = (console_width > content_width) ? (console_width - content_width) / 2 : 0;
    int vertical_padding = (console_height > 1) ? (console_height / 2) - 1 : 0;
    for (int i = 0; i < vertical_padding; ++i) {
        cout << "\n";
    }
    cout << string(horizontal_padding, ' ') + prefix << flush;
    int total_steps = barWidth;
    int duration_ms = 2000;
    int sleep_ms_per_step = (total_steps > 0) ? duration_ms / total_steps : 0;
    if (sleep_ms_per_step == 0 && total_steps > 0) sleep_ms_per_step = 1;
    for (int i = 0; i <= total_steps; ++i) {
        cout << string(i, '\xDB');
        cout << string(total_steps - i, '\xB0');
        cout << suffix << flush;
        cout << "\r" + string(horizontal_padding, ' ') + prefix << flush;
        if (i < total_steps) {
            Sleep(sleep_ms_per_step);
        }
    }
    cout << string(total_steps, '\xDB') + suffix << endl;
}

string getPassword() {
    string password;
    char ch;
    cout << flush;
    while ((ch = _getch()) != 13 && ch != 10) { // 13 is Enter, 10 is newline
        if (ch == 8) { // Backspace
            if (!password.empty()) {
                password.erase(password.length() - 1);
                cout << "\b \b" << flush;
            }
        } else if (ch >= 32 && ch <= 126) {
            password += ch;
            cout << '*' << flush;
        }
    }
    cout << endl;
    return password;
}

int manualMax(int a, int b) {
    return (a > b) ? a : b;
}

string intToString(int val) {
    stringstream ss;
    ss << val;
    return ss.str();
}

// **FIXED**: Replaced strol with manual parsing logic to ensure compatibility.
int custom_stoi(const string& str, size_t* pos = 0) {
    if (str.empty()) {
        throw invalid_argument("stoi: invalid argument (empty string)");
    }

    long long result = 0;
    int sign = 1;
    size_t i = 0;

    // Trim leading whitespace
    while (i < str.length() && isspace(str[i])) {
        i++;
    }

    // Handle sign
    if (i < str.length() && (str[i] == '+' || str[i] == '-')) {
        if (str[i] == '-') {
            sign = -1;
        }
        i++;
    }

    bool hasDigits = false;
    while (i < str.length() && isdigit(str[i])) {
        hasDigits = true;
        int digit = str[i] - '0';

        // Check for overflow before multiplication
        if (result > (numeric_limits<long long>::max() - digit) / 10) {
             throw out_of_range("stoi: out of range");
        }
        
        result = result * 10 + digit;
        i++;
    }

    if (!hasDigits) {
        throw invalid_argument("stoi: invalid argument (no digits)");
    }

    // Check for non-digit characters after the number part
    if (i < str.length()) {
        throw invalid_argument("stoi: invalid argument with extra chars");
    }

    result *= sign;

    if (result > numeric_limits<int>::max() || result < numeric_limits<int>::min()) {
        throw out_of_range("stoi: out of range");
    }

    if (pos) {
        *pos = i;
    }

    return static_cast<int>(result);
}


double custom_stod(const string& str, size_t* pos = 0) {
    stringstream ss(str);
    double result;
    ss >> result;
    if (ss.fail() || !ss.eof()) { // Simplified check
        throw invalid_argument("stod: invalid argument");
    }
    if (pos) {
        *pos = ss.tellg();
        if (*pos == (size_t)-1) *pos = str.length();
    }
    return result;
}

string manualToLower(const string& str) {
    string lower = str;
    for (size_t i = 0; i < lower.length(); ++i) {
        lower[i] = static_cast<char>(tolower(static_cast<unsigned char>(lower[i])));
    }
    return lower;
}

size_t manualFind(const string& str, char ch, size_t start_pos = 0) {
    for (size_t i = start_pos; i < str.length(); ++i) {
        if (str[i] == ch) {
            return i;
        }
    }
    return string::npos;
}

bool manualContains(const string& str, const string& substr) {
    if (substr.empty()) return true;
    string lowerStr = manualToLower(str);
    string lowerSubstr = manualToLower(substr);
    if (lowerSubstr.length() > lowerStr.length()) return false;
    for (size_t i = 0; i <= lowerStr.length() - lowerSubstr.length(); ++i) {
        bool match = true;
        for (size_t j = 0; j < lowerSubstr.length(); ++j) {
            if (lowerStr[i + j] != lowerSubstr[j]) {
                match = false;
                break;
            }
        }
        if (match) return true;
    }
    return false;
}

bool manualFileContains(const string& filename, const string& search_term) {
    ifstream file(filename.c_str());
    if (!file) {
        return false;
    }
    string line;
    string lower_search_term = manualToLower(search_term);
    while (getline(file, line)) {
        line.erase(0, line.find_first_not_of(" \t\n\r"));
        line.erase(line.find_last_not_of(" \t\n\r") + 1);
        if (!line.empty()) {
            size_t commaPos = manualFind(line, ',');
            string storedUsername = (commaPos == string::npos) ? line : line.substr(0, commaPos);
            storedUsername.erase(0, storedUsername.find_first_not_of(" \t\n\r"));
            storedUsername.erase(storedUsername.find_last_not_of(" \t\n\r") + 1);
            if (manualToLower(storedUsername) == lower_search_term) {
                file.close();
                return true;
            }
        }
    }
    file.close();
    return false;
}

// --- DATA STRUCTURES ---

struct Product {
    string name;
    string price;
    string category;
    int stock;
    Product(string n = "", string p = "", string c = "", int s = 0)
        : name(n), price(p), category(c), stock(s) {}
};

struct ProductNode {
    Product data;
    ProductNode* next;
    ProductNode(const Product& p) : data(p), next(NULL) {}
};

struct CategoryNode {
    string data;
    CategoryNode* next;
    CategoryNode(const string& s) : data(s), next(NULL) {}
};


// --- CLASS ProductList ---
class ProductList {
private:
    ProductNode* productHead;
    int productCount;
    CategoryNode* categoryHead;
    int categoryCount;

    void clearProductList() {
        ProductNode* current = productHead;
        while (current != NULL) {
            ProductNode* next = current->next;
            delete current;
            current = next;
        }
        productHead = NULL;
        productCount = 0;
    }

    void clearCategoryList() {
        CategoryNode* current = categoryHead;
        while (current != NULL) {
            CategoryNode* next = current->next;
            delete current;
            current = next;
        }
        categoryHead = NULL;
        categoryCount = 0;
    }

    Product* createProductArrayFromList() const {
        if (productCount == 0) return NULL;
        Product* arr = new Product[productCount];
        ProductNode* current = productHead;
        int i = 0;
        while (current != NULL) {
            arr[i++] = current->data;
            current = current->next;
        }
        return arr;
    }

    string* createCategoryArrayFromList() const {
        if (categoryCount == 0) return NULL;
        string* arr = new string[categoryCount];
        CategoryNode* current = categoryHead;
        int i = 0;
        while (current != NULL) {
            arr[i++] = current->data;
            current = current->next;
        }
        return arr;
    }

    void merge(Product* arr, int left, int mid, int right, bool ascending, const string& sortBy) const {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        Product* L = new Product[n1];
        Product* R = new Product[n2];

        for (int i = 0; i < n1; i++) L[i] = arr[left + i];
        for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            bool comparison;
            if (sortBy == "name") {
                comparison = ascending ? (manualToLower(L[i].name) <= manualToLower(R[j].name)) : (manualToLower(L[i].name) >= manualToLower(R[j].name));
            } else { // "category"
                comparison = ascending ? (manualToLower(L[i].category) <= manualToLower(R[j].category)) : (manualToLower(L[i].category) >= manualToLower(R[j].category));
            }

            if (comparison) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
            k++;
        }

        while (i < n1) { arr[k] = L[i]; i++; k++; }
        while (j < n2) { arr[k] = R[j]; j++; k++; }

        delete[] L;
        delete[] R;
    }

    void mergeSort(Product* arr, int left, int right, bool ascending, const string& sortBy) const {
        if (left >= right) return;
        int mid = left + (right - left) / 2;
        mergeSort(arr, left, mid, ascending, sortBy);
        mergeSort(arr, mid + 1, right, ascending, sortBy);
        merge(arr, left, mid, right, ascending, sortBy);
    }
    
    void merge(string* arr, int left, int mid, int right, bool ascending) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        string* L = new string[n1];
        string* R = new string[n2];

        for (int i = 0; i < n1; i++) L[i] = arr[left + i];
        for (int j = 0; j < n2; j++) R[j] = arr[mid + 1 + j];

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
             bool comparison = ascending ? (manualToLower(L[i]) <= manualToLower(R[j])) : (manualToLower(L[i]) >= manualToLower(R[j]));
            if (comparison) {
                arr[k] = L[i];
                i++;
            } else {
                arr[k] = R[j];
                j++;
            }
            k++;
        }

        while (i < n1) { arr[k] = L[i]; i++; k++; }
        while (j < n2) { arr[k] = R[j]; j++; k++; }
        delete[] L;
        delete[] R;
    }
    
    void mergeSortCategories(string* arr, int left, int right, bool ascending) {
        if (left >= right) return;
        int mid = left + (right - left) / 2;
        mergeSortCategories(arr, left, mid, ascending);
        mergeSortCategories(arr, mid + 1, right, ascending);
        merge(arr, left, mid, right, ascending);
    }

    int binarySearch(const Product* sortedArr, int size, const string& searchTerm) const {
        if (size == 0) return -1;
        string lowerSearchTerm = manualToLower(searchTerm);
        int low = 0;
        int high = size - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            string lowerMidName = manualToLower(sortedArr[mid].name);
            if (lowerMidName == lowerSearchTerm) {
                return mid;
            } else if (lowerMidName < lowerSearchTerm) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1;
    }
    
    int binarySearchCategory(const string* sortedArr, int size, const string& searchTerm) const {
        if (size == 0) return -1;
        string lowerSearchTerm = manualToLower(searchTerm);
        int low = 0;
        int high = size - 1;
        while (low <= high) {
            int mid = low + (high - low) / 2;
            string lowerMidName = manualToLower(sortedArr[mid]);
            if (lowerMidName == lowerSearchTerm) {
                return mid;
            } else if (lowerMidName < lowerSearchTerm) {
                low = mid + 1;
            } else {
                high = mid - 1;
            }
        }
        return -1;
    }

    string* getPurchasedProductNames(const string& ordersFilename, int& count) const {
        ifstream file(ordersFilename.c_str());
        if (!file) {
            count = 0;
            return NULL;
        }
        const int initialCapacity = 20;
        string* tempNames = new string[initialCapacity];
        int capacity = initialCapacity;
        count = 0;
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string user, time, name, qty, price;
            if (getline(ss, user, '\t') && getline(ss, time, '\t') && getline(ss, name, '\t')) {
                bool found = false;
                string lowerName = manualToLower(name);
                for (int i = 0; i < count; ++i) {
                    if (manualToLower(tempNames[i]) == lowerName) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    if (count == capacity) {
                        int newCapacity = capacity * 2;
                        string* newArray = new string[newCapacity];
                        for (int i = 0; i < count; ++i) newArray[i] = tempNames[i];
                        delete[] tempNames;
                        tempNames = newArray;
                        capacity = newCapacity;
                    }
                    tempNames[count++] = name;
                }
            }
        }
        file.close();
        return tempNames;
    }

    bool isProductPurchased(const string& productName, const string* purchasedNames, int count) const {
        string lowerProductName = manualToLower(productName);
        for (int i = 0; i < count; i++) {
            if (manualToLower(purchasedNames[i]) == lowerProductName) {
                return true;
            }
        }
        return false;
    }

public:
    ProductList() : productHead(NULL), productCount(0), categoryHead(NULL), categoryCount(0) {}
    ~ProductList() {
        clearProductList();
        clearCategoryList();
    }

    ProductList(const ProductList& other) = delete;
    ProductList& operator=(const ProductList& other) = delete;

    Product* getSortedProductsArray(const string& sortBy, bool ascending) const {
        if (productCount == 0) return NULL;
        Product* arr = createProductArrayFromList();
        mergeSort(arr, 0, productCount - 1, ascending, sortBy);
        return arr;
    }
    
    bool isValidPrice(const string& price) const {
        if (price.length() < 4 || manualToLower(price.substr(0, 3)) != "rm ") return false;
        string numPart = price.substr(3);
        bool hasDecimal = false;
        bool hasDigit = false;
        size_t decimalPos = string::npos;
        for (size_t i = 0; i < numPart.length(); ++i) {
            char c = numPart[i];
            if (isdigit(static_cast<unsigned char>(c))) {
                hasDigit = true;
            } else if (c == '.') {
                if (hasDecimal) return false;
                hasDecimal = true;
                decimalPos = i;
            } else {
                return false;
            }
        }
        if (!hasDigit || !hasDecimal || (numPart.length() - 1 - decimalPos) != 2) return false;
        try {
            size_t pos;
            custom_stod(numPart, &pos);
            return pos == numPart.length();
        } catch (...) {
            return false;
        }
    }

    void searchCategory(const string& searchTerm) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Search Category ===\n"; resetColor();
        if (searchTerm.empty()) {
            setColor(RED); cout << "Search term cannot be empty.\n"; resetColor();
            cout << "\nPress any key to return to menu..."; _getch();
            return;
        }
        if (categoryCount == 0) {
            setColor(RED); cout << "No categories available to search.\n"; resetColor();
            cout << "\nPress any key to return to menu..."; _getch();
            return;
        }

        string* tempCategories = createCategoryArrayFromList();
        mergeSortCategories(tempCategories, 0, categoryCount - 1, true);

        int foundIndex = binarySearchCategory(tempCategories, categoryCount, searchTerm);

        if (foundIndex == -1) {
            setColor(RED); cout << "No exact category found matching '" << searchTerm << "'.\n"; resetColor();
        } else {
            string foundCategoryName = tempCategories[foundIndex];
            setColor(GREEN); cout << "=== Exact Match Category Found ===\n"; resetColor();
            int maxNumLength = (int)string("No.").length();
            int maxCategoryNameLength = (int)string("Category Name").length();
            maxNumLength = manualMax(maxNumLength, (int)intToString(1).length());
            maxCategoryNameLength = manualMax(maxCategoryNameLength, (int)foundCategoryName.length());
            const int COLUMN_PADDING_SPACES = 2;
            int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
            int categoryNameColWidth = maxCategoryNameLength + COLUMN_PADDING_SPACES;
            string headerLine = "+" + string(numColWidth, '-') + "+" + string(categoryNameColWidth, '-') + "+";
            cout << headerLine << endl;
            cout << "|" << left << setw(numColWidth) << " No."
                 << "|" << left << setw(categoryNameColWidth) << " Category Name"
                 << "|" << endl;
            cout << headerLine << endl;
            cout << "|" << left << setw(numColWidth) << (" " + intToString(1));
            cout << "|" << left << setw(categoryNameColWidth) << (" " + foundCategoryName);
            cout << "|" << endl;
            cout << headerLine << endl;
            
            cout << "Enter category ID (1) to view products, or 0 to return: ";
            string input;
            int id;
            getline(cin, input);
            try {
                size_t pos;
                id = custom_stoi(input, &pos);
            } catch (...) {
                id = -1; // Invalid input
            }

            if (id == 1) {
                system("cls");
                displayProductNamesByCategory(foundCategoryName);
            } else {
                setColor(RED); cout << "Returning to menu.\n"; resetColor();
            }
        }
        
        delete[] tempCategories;
        cout << "\nPress any key to return to menu...";
        _getch();
    }

    void loadCategories(const string& filename) {
        clearCategoryList();

        ifstream inFile(filename.c_str());
        if (!inFile) {
            cerr << "Categories file not found: " << filename << ". Creating default.\n";
            ofstream outFile(filename.c_str());
            if (outFile) {
                outFile << "Uncategorized\n";
                outFile.close();
                CategoryNode* newNode = new CategoryNode("Uncategorized");
                categoryHead = newNode;
                categoryCount = 1;
            } else {
                cerr << "Error: Cannot create default categories file: " << filename << endl;
                throw runtime_error("Initialization failed: Cannot create categories file.");
            }
        } else {
            string line;
            while (getline(inFile, line)) {
                size_t first = line.find_first_not_of(" \t\n\r");
                if (string::npos != first) {
                    size_t last = line.find_last_not_of(" \t\n\r");
                    line = line.substr(first, (last - first + 1));
                } else {
                    line = "";
                }
                if (!line.empty()) {
                    CategoryNode* newNode = new CategoryNode(line);
                    newNode->next = categoryHead;
                    categoryHead = newNode;
                    categoryCount++;
                }
            }
            inFile.close();
            if (categoryCount == 0) {
                 CategoryNode* newNode = new CategoryNode("Uncategorized");
                 categoryHead = newNode;
                 categoryCount = 1;
                 ofstream outFile(filename.c_str());
                 if (outFile) {
                    outFile << "Uncategorized\n";
                    outFile.close();
                 }
            }
        }

        bool uncategorizedExists = false;
        CategoryNode* current = categoryHead;
        while(current != NULL) {
            if (manualToLower(current->data) == "uncategorized") {
                uncategorizedExists = true;
                break;
            }
            current = current->next;
        }

        if (!uncategorizedExists) {
            cout << "Warning: 'Uncategorized' category not found. Adding it.\n";
            CategoryNode* newNode = new CategoryNode("Uncategorized");
            newNode->next = categoryHead;
            categoryHead = newNode;
            categoryCount++;
            saveCategories(filename);
        }
    }
    
    void saveCategories(const string& filename) {
        ofstream outFile(filename.c_str(), ios::out | ios::trunc);
        if (!outFile) {
             throw runtime_error("Cannot open categories file for writing: " + filename);
        }
        
        string* tempArray = createCategoryArrayFromList();
        if(tempArray) {
            mergeSortCategories(tempArray, 0, categoryCount - 1, true);
            for (int i = 0; i < categoryCount; i++) {
                outFile << tempArray[i] << endl;
            }
            delete[] tempArray;
        }
        outFile.close();
    }

    void addCategory(const string& filename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Add New Category ===\n"; resetColor();
        string newCategory;
        cout << "Enter new category name: ";
        getline(cin, newCategory);
        newCategory.erase(0, newCategory.find_first_not_of(" \t\n\r"));
        newCategory.erase(newCategory.find_last_not_of(" \t\n\r") + 1);
        
        if (newCategory.empty()) {
            setColor(RED); cout << "Category name cannot be empty.\n"; resetColor();
            cout << "\nPress any key to return..."; _getch();
            return;
        }
        
        bool exists = false;
        CategoryNode* current = categoryHead;
        while(current != NULL) {
            if (manualToLower(current->data) == manualToLower(newCategory)) {
                exists = true;
                break;
            }
            current = current->next;
        }

        if (exists) {
            setColor(RED); cout << "Category '" << newCategory << "' already exists.\n"; resetColor();
            cout << "\nPress any key to return..."; _getch();
            return;
        }
        
        CategoryNode* newNode = new CategoryNode(newCategory);
        newNode->next = categoryHead;
        categoryHead = newNode;
        categoryCount++;
        
        try {
            saveCategories(filename);
            setColor(GREEN); cout << "Category '" << newCategory << "' added successfully.\n"; resetColor();
            Sleep(1000);
            sortAndDisplayCategories();
        } catch (const runtime_error& e) {
            cerr << "Error saving categories: " << e.what() << endl;
            CategoryNode* toDelete = categoryHead;
            categoryHead = categoryHead->next;
            delete toDelete;
            categoryCount--;
            setColor(YELLOW); cout << "Warning: Category add failed to save. No changes made.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
        }
    }

    void deleteCategory(const string& categoriesFilename, const string& productsFilename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Delete Category ===\n"; resetColor();

        string* displayableCategories = new string[categoryCount];
        int displayableCategoryCount = 0;
        CategoryNode* currentCat = categoryHead;
        while (currentCat) {
            if (manualToLower(currentCat->data) != "uncategorized") {
                displayableCategories[displayableCategoryCount++] = currentCat->data;
            }
            currentCat = currentCat->next;
        }

        if (displayableCategoryCount == 0) {
            setColor(RED); cout << "No categories available to delete.\n"; resetColor();
            cout << "\nPress any key to return...";
            delete[] displayableCategories;
            _getch();
            return;
        }
        
        mergeSortCategories(displayableCategories, 0, displayableCategoryCount - 1, true);

        setColor(GREEN); cout << "=== Select Category to Delete ===\n"; resetColor();
        int maxNumLength = (int)string("No.").length();
        int maxCategoryNameLength = (int)string("Category Name").length();

        for (int i = 0; i < displayableCategoryCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxCategoryNameLength = manualMax(maxCategoryNameLength, (int)displayableCategories[i].length());
        }

        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int categoryNameColWidth = maxCategoryNameLength + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(categoryNameColWidth, '-') + "+";

        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(categoryNameColWidth) << " Category Name"
             << "|" << endl;
        cout << headerLine << endl;
        
        for (int i = 0; i < displayableCategoryCount; ++i) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(categoryNameColWidth) << (" " + displayableCategories[i]);
            cout << "|" << endl;
        }
        cout << headerLine << endl;
        
        cout << "Enter category ID (1-" << displayableCategoryCount << ") to delete, or 0 to return: ";
        string input;
        int id;
        getline(cin, input);
        try {
            size_t pos;
            id = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input. Please enter a number.\n"; resetColor();
            delete[] displayableCategories;
            return;
        }

        if (id == 0) {
            setColor(RED); cout << "Deletion cancelled.\n"; resetColor();
            delete[] displayableCategories;
            return;
        }

        if (id < 1 || id > displayableCategoryCount) {
            setColor(RED); cout << "Invalid category ID.\n"; resetColor();
            delete[] displayableCategories;
            return;
        }

        string categoryToDelete = displayableCategories[id - 1];
        delete[] displayableCategories;

        bool inUse = false;
        ProductNode* currentProd = productHead;
        while(currentProd) {
            if (manualToLower(currentProd->data.category) == manualToLower(categoryToDelete)) {
                inUse = true;
                break;
            }
            currentProd = currentProd->next;
        }
        
        if (inUse) {
            setColor(RED);
            cout << "This category cannot be deleted because it is still in use by products.\n";
            resetColor();
            return;
        }

        cout << "\nAre you sure you want to delete the empty category '" << categoryToDelete << "'? (y/n): ";
        string response;
        getline(cin, response);
        response = manualToLower(response);

        if (response == "y") {
            CategoryNode* current = categoryHead;
            CategoryNode* prev = NULL;
            while(current != NULL && manualToLower(current->data) != manualToLower(categoryToDelete)) {
                prev = current;
                current = current->next;
            }
            
            if (current != NULL) {
                if (prev == NULL) {
                    categoryHead = current->next;
                } else {
                    prev->next = current->next;
                }
                delete current;
                categoryCount--;
                
                try {
                    saveCategories(categoriesFilename);
                    setColor(GREEN); cout << "Category '" << categoryToDelete << "' deleted successfully.\n"; resetColor();
                } catch (const runtime_error& e) {
                    cerr << "Error saving categories after deletion: " << e.what() << endl;
                    setColor(YELLOW); cout << "Category deletion failed to save to file.\n"; resetColor();
                }
            }
        } else {
            setColor(RED); cout << "Category deletion cancelled.\n"; resetColor();
        }
    }

    void addProduct(const string& filename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Add New Product ===\n"; resetColor();
        string name, price, category, stockInput;
        int stock;
        
        cout << "Enter product name: ";
        getline(cin, name);
        name.erase(0, name.find_first_not_of(" \t\n\r"));
        name.erase(name.find_last_not_of(" \t\n\r") + 1);

        if (name.empty()) {
            setColor(RED); cout << "Product name cannot be empty.\n"; resetColor();
            _getch(); return;
        }
        
        Product* tempProducts = createProductArrayFromList();
        if(tempProducts) {
             mergeSort(tempProducts, 0, productCount - 1, true, "name");
             if (binarySearch(tempProducts, productCount, name) != -1) {
                setColor(RED); cout << "Product name '" << name << "' already exists!\n"; resetColor();
                delete[] tempProducts;
                _getch(); return;
             }
             delete[] tempProducts;
        }

        cout << "Enter price (e.g., RM 140.00): ";
        getline(cin, price);
        price.erase(0, price.find_first_not_of(" \t\n\r"));
        price.erase(price.find_last_not_of(" \t\n\r") + 1);
        if (!isValidPrice(price)) {
            setColor(RED); cout << "Invalid price format! Use format like RM 140.00.\n"; resetColor();
             _getch(); return;
        }

        cout << "\n";
        setColor(GREEN); cout << "=== Select a Category ===\n"; resetColor();
        if (categoryCount == 0) {
            setColor(RED); cout << "No categories exist. Please add a category first.\n"; resetColor();
             _getch(); return;
        }

        string* tempCategories = createCategoryArrayFromList();
        mergeSortCategories(tempCategories, 0, categoryCount - 1, true);
        
        int maxNumLength = (int)string("ID").length();
        int maxCategoryNameLength = (int)string("Category Name").length();
        for (int i = 0; i < categoryCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxCategoryNameLength = manualMax(maxCategoryNameLength, (int)tempCategories[i].length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int categoryNameColWidth = maxCategoryNameLength + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(categoryNameColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " ID" << "|" << left << setw(categoryNameColWidth) << " Category Name" << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < categoryCount; i++) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(categoryNameColWidth) << (" " + tempCategories[i]);
            cout << "|" << endl;
        }
        cout << headerLine << endl;

        cout << "Enter category ID (1-" << categoryCount << ", or 0 to cancel): ";
        string categoryIdInput;
        int categoryId;
        getline(cin, categoryIdInput);
        try {
            size_t pos;
            categoryId = custom_stoi(categoryIdInput, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor();
             delete[] tempCategories; _getch(); return;
        }

        if (categoryId == 0 || categoryId < 1 || categoryId > categoryCount) {
            setColor(RED); cout << "Invalid choice or cancellation.\n"; resetColor();
            delete[] tempCategories; _getch(); return;
        }
        
        category = tempCategories[categoryId - 1];
        delete[] tempCategories;

        cout << "\nEnter stock: ";
        getline(cin, stockInput);
        try {
            size_t pos;
            stock = custom_stoi(stockInput, &pos);
            if (stock < 0) {
                setColor(RED); cout << "Stock cannot be negative.\n"; resetColor();
                _getch(); return;
            }
        } catch (...) {
            setColor(RED); cout << "Invalid stock value!\n"; resetColor();
             _getch(); return;
        }

        try {
            Product newProd(name, price, category, stock);
            ProductNode* newNode = new ProductNode(newProd);
            newNode->next = productHead;
            productHead = newNode;
            productCount++;
            
            saveToFile(filename);
            setColor(GREEN); cout << "Product '" << name << "' added successfully.\n"; resetColor();
            Sleep(1000);
            sortAndDisplayProductNames();
        } catch (const runtime_error& e) {
            ProductNode* toDelete = productHead;
            productHead = productHead->next;
            delete toDelete;
            productCount--;
            cerr << "Error saving product: " << e.what() << endl;
            setColor(RED); cout << "Product addition failed to save.\n"; resetColor();
            _getch();
        }
    }

    void deleteProduct(const string& filename, const string& ordersFilename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Delete Product ===\n"; resetColor();
        if (productCount == 0) {
            setColor(RED); cout << "No products to delete.\n"; resetColor();
            _getch(); return;
        }
        
        Product* tempProducts = createProductArrayFromList();
        mergeSort(tempProducts, 0, productCount - 1, true, "name");

        setColor(GREEN); cout << "=== Select Product to Delete ===\n"; resetColor();
        int maxNumLength = (int)string("No.").length();
        int maxNameLength = (int)string("Product Name").length();
        for (int i = 0; i < productCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxNameLength = manualMax(maxNameLength, (int)tempProducts[i].name.length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int nameColWidth = maxNameLength + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(nameColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No." << "|" << left << setw(nameColWidth) << " Product Name" << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < productCount; i++) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(nameColWidth) << (" " + tempProducts[i].name);
            cout << "|" << endl;
        }
        cout << headerLine << endl;

        cout << "Enter product ID to delete (1-" << productCount << "), or 0 to return: ";
        string input;
        int id;
        try {
            getline(cin, input);
            size_t pos;
            id = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor();
            delete[] tempProducts; return;
        }

        if (id == 0 || id < 1 || id > productCount) {
            setColor(RED); cout << "Invalid choice or cancellation.\n"; resetColor();
            delete[] tempProducts; return;
        }

        string productNameToDelete = tempProducts[id - 1].name;
        delete[] tempProducts; 

        int purchasedCount = 0;
        string* purchasedNames = getPurchasedProductNames(ordersFilename, purchasedCount);
        bool purchased = isProductPurchased(productNameToDelete, purchasedNames, purchasedCount);
        delete[] purchasedNames;

        if (purchased) {
            setColor(RED);
            cout << "This product cannot be deleted because it has been purchased.\n";
            resetColor();
            return;
        }

        setColor(YELLOW); cout << "Are you sure you want to delete '" << productNameToDelete << "'? (y/n): "; resetColor();
        string confirmation;
        getline(cin, confirmation);
        
        if (manualToLower(confirmation) == "y") {
            ProductNode* current = productHead;
            ProductNode* prev = NULL;
            while(current != NULL && current->data.name != productNameToDelete) {
                prev = current;
                current = current->next;
            }

            if (current != NULL) {
                if (prev == NULL) productHead = current->next;
                else prev->next = current->next;
                delete current;
                productCount--;

                try {
                    saveToFile(filename);
                    setColor(GREEN); cout << "Product deleted successfully.\n"; resetColor();
                } catch (const runtime_error& e) {
                    cerr << "Error saving after deletion: " << e.what() << endl;
                    setColor(RED); cout << "Product deletion failed to save to file.\n"; resetColor();
                }
            }
        } else {
            setColor(RED); cout << "Deletion cancelled.\n"; resetColor();
        }
    }
    
    void manualReplace(string& str, char oldChar, char newChar) const {
        for (size_t i = 0; i < str.length(); ++i) {
            if (str[i] == oldChar) {
                str[i] = newChar;
            }
        }
    }
    
    void saveToFile(const string& filename) {
        ofstream outFile(filename.c_str(), ios::out | ios::trunc);
        if (!outFile) {
            throw runtime_error("Cannot open product file for writing: " + filename);
        }

        ProductNode* current = productHead;
        while (current != NULL) {
            string safeName = current->data.name;
            manualReplace(safeName, ',', ';');
            string safePrice = current->data.price;
            string safeCategory = current->data.category;
            manualReplace(safeCategory, ',', ';');
            
            outFile << safeName << "," << safePrice << ","
                    << safeCategory << "," << current->data.stock << "\n";
            
            current = current->next;
        }
        outFile.close();
    }
    
    void loadFromFile(const string& filename) {
        clearProductList();

        ifstream inFile(filename.c_str());
        if (!inFile) {
            cout << "Product file not found: " + filename + ". Creating empty file.\n";
            ofstream outFile(filename.c_str());
            if (outFile) outFile.close();
            return;
        }
        
        string line;
        int lineNumber = 0;
        while (getline(inFile, line)) {
            lineNumber++;
            stringstream ss(line);
            string tempName, tempPrice, tempCategory, tempStockStr;
            if (!(getline(ss, tempName, ',') && getline(ss, tempPrice, ',') && getline(ss, tempCategory, ',') && getline(ss, tempStockStr))) {
                cerr << "Warning: Malformed line #" << lineNumber << " in " << filename << ". Skipping.\n";
                continue;
            }
            tempName.erase(0, tempName.find_first_not_of(" \t")); tempName.erase(tempName.find_last_not_of(" \t") + 1);
            tempPrice.erase(0, tempPrice.find_first_not_of(" \t")); tempPrice.erase(tempPrice.find_last_not_of(" \t") + 1);
            tempCategory.erase(0, tempCategory.find_first_not_of(" \t")); tempCategory.erase(tempCategory.find_last_not_of(" \t") + 1);
            tempStockStr.erase(0, tempStockStr.find_first_not_of(" \t")); tempStockStr.erase(tempStockStr.find_last_not_of(" \t") + 1);
            
            manualReplace(tempName, ';', ',');
            manualReplace(tempCategory, ';', ',');

            int tempStock = 0;
            try {
                size_t pos;
                tempStock = custom_stoi(tempStockStr, &pos);
            } catch (...) { tempStock = 0; }

            if (!isValidPrice(tempPrice)) {
                cerr << "Warning: Invalid price for '" << tempName << "' on line #" << lineNumber << ". Skipping.\n";
                continue;
            }
            
            bool validCategory = false;
            CategoryNode* catCurrent = categoryHead;
            while(catCurrent) {
                if (manualToLower(catCurrent->data) == manualToLower(tempCategory)) {
                    tempCategory = catCurrent->data;
                    validCategory = true;
                    break;
                }
                catCurrent = catCurrent->next;
            }
            if (!validCategory) {
                 cerr << "Warning: Invalid category '" << tempCategory << "' on line #" << lineNumber << ". Assigning to 'Uncategorized'.\n";
                 tempCategory = "Uncategorized";
            }
            
            Product newProd(tempName, tempPrice, tempCategory, tempStock);
            ProductNode* newNode = new ProductNode(newProd);
            newNode->next = productHead;
            productHead = newNode;
            productCount++;
        }
        inFile.close();
    }
    
    void displayProductById(int id, const Product* arr, int arrSize) const {
        if (arr == NULL || id < 1 || id > arrSize) {
            cerr << "Internal Error: Invalid product ID or array for display.\n";
            return;
        }
        const Product& product = arr[id - 1];
        setColor(LIGHT_BLUE); cout << "\n--- Product Details ---\n"; resetColor();
        setColor(GREEN); cout << "Product Name: " << product.name << endl; resetColor();
        setColor(LIGHT_BLUE); cout << "Category: " << product.category << endl; resetColor();
        setColor(GREEN); cout << "Price: " << product.price << endl; resetColor();
        setColor(LIGHT_BLUE); cout << "Stock: " << product.stock << endl; resetColor();
        setColor(LIGHT_BLUE); cout << "-----------------------\n"; resetColor();
    }

    void displayProductNamesByCategory(const string& category) const {
        Product* tempFilteredArray = new Product[productCount];
        int tempFilteredCount = 0;
        ProductNode* current = productHead;
        string lowerCategory = manualToLower(category);

        while (current) {
            if (manualToLower(current->data.category) == lowerCategory) {
                tempFilteredArray[tempFilteredCount++] = current->data;
            }
            current = current->next;
        }

        if (tempFilteredCount == 0) {
            setColor(RED); cout << "No products found in category: '" + category + "'.\n"; resetColor();
            cout << "\nPress any key to return..."; _getch();
            delete[] tempFilteredArray;
            return;
        }

        mergeSort(tempFilteredArray, 0, tempFilteredCount - 1, true, "name");
        
        setColor(GREEN); cout << "=== Products in category '" + category + "' (sorted by name) ===\n"; resetColor();
        int maxNumLength = (int)string("No.").length();
        int maxProductNameLength = (int)string("Product Name").length();
        for (int i = 0; i < tempFilteredCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxProductNameLength = manualMax(maxProductNameLength, (int)tempFilteredArray[i].name.length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int productNameColWidth = maxProductNameLength + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(productNameColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No." << "|" << left << setw(productNameColWidth) << " Product Name" << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < tempFilteredCount; ++i) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(productNameColWidth) << (" " + tempFilteredArray[i].name);
            cout << "|" << endl;
        }
        cout << headerLine << endl;
        
        delete[] tempFilteredArray;
        cout << "\nPress any key to return...";
        _getch();
    }
    
    void searchProduct(const string& searchTerm) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Search Product ===\n"; resetColor();
        if (searchTerm.empty() || productCount == 0) {
             setColor(RED); cout << "Search term is empty or no products available.\n"; resetColor();
             _getch(); return;
        }

        Product* combinedResults = new Product[productCount];
        int combinedCount = 0;
        ProductNode* current = productHead;
        string lowerSearchTerm = manualToLower(searchTerm);

        while (current) {
            if (manualContains(manualToLower(current->data.name), lowerSearchTerm) ||
                manualContains(manualToLower(current->data.category), lowerSearchTerm)) {
                
                bool duplicate = false;
                for (int j = 0; j < combinedCount; ++j) {
                    if (manualToLower(current->data.name) == manualToLower(combinedResults[j].name)) {
                        duplicate = true;
                        break;
                    }
                }
                if (!duplicate) {
                    combinedResults[combinedCount++] = current->data;
                }
            }
            current = current->next;
        }

        if (combinedCount == 0) {
            setColor(RED); cout << "No products found matching '" << searchTerm << "'.\n"; resetColor();
        } else {
            mergeSort(combinedResults, 0, combinedCount - 1, true, "name");
            
            setColor(GREEN); cout << "=== Search Results for '" << searchTerm << "' ===\n"; resetColor();
            int maxNumLength = (int)string("No.").length();
            int maxProductNameLength = (int)string("Product Name").length();
            int maxCategoryLength = (int)string("Category").length();
            for (int i = 0; i < combinedCount; ++i) {
                maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
                maxProductNameLength = manualMax(maxProductNameLength, (int)combinedResults[i].name.length());
                maxCategoryLength = manualMax(maxCategoryLength, (int)combinedResults[i].category.length());
            }
            const int COLUMN_PADDING_SPACES = 2;
            int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
            int productNameColWidth = maxProductNameLength + COLUMN_PADDING_SPACES;
            int categoryColWidth = maxCategoryLength + COLUMN_PADDING_SPACES;
            string headerLine = "+" + string(numColWidth, '-') + "+" + string(productNameColWidth, '-') + "+" + string(categoryColWidth, '-') + "+";
            cout << headerLine << endl;
            cout << "|" << left << setw(numColWidth) << " No."
                 << "|" << left << setw(productNameColWidth) << " Product Name"
                 << "|" << left << setw(categoryColWidth) << " Category" << "|" << endl;
            cout << headerLine << endl;
            for (int i = 0; i < combinedCount; ++i) {
                 cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
                 cout << "|" << left << setw(productNameColWidth) << (" " + combinedResults[i].name);
                 cout << "|" << left << setw(categoryColWidth) << (" " + combinedResults[i].category);
                 cout << "|" << endl;
            }
            cout << headerLine << endl;

            cout << "Enter product ID (1-" << combinedCount << ") to view, or 0 to return: ";
            string input;
            int id;
            getline(cin, input);
            try {
                size_t pos;
                id = custom_stoi(input, &pos);
            } catch(...) { id = -1; }

            if (id >= 1 && id <= combinedCount) {
                system("cls");
                displayProductById(id, combinedResults, combinedCount);
                _getch();
            }
        }
        delete[] combinedResults;
        cout << "\nPress any key to return...";
        _getch();
    }
    
    void sortAndDisplayCategories() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Product Categories (sorted) ===\n"; resetColor();
        if (categoryCount == 0) {
            setColor(RED); cout << "No categories to display.\n"; resetColor();
            _getch(); return;
        }

        string* tempCategories = createCategoryArrayFromList();
        mergeSortCategories(tempCategories, 0, categoryCount - 1, true);

        int maxNumLength = (int)string("No.").length();
        int maxCategoryNameLength = (int)string("Category Name").length();
        for (int i = 0; i < categoryCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxCategoryNameLength = manualMax(maxCategoryNameLength, (int)tempCategories[i].length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int categoryNameColWidth = maxCategoryNameLength + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(categoryNameColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No." << "|" << left << setw(categoryNameColWidth) << " Category Name" << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < categoryCount; i++) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(categoryNameColWidth) << (" " + tempCategories[i]);
            cout << "|" << endl;
        }
        cout << headerLine << endl;

        cout << "Enter category ID (1-" << categoryCount << ") to view products, or 0 to return: ";
        string input;
        int id;
        getline(cin, input);
        try {
            size_t pos;
            id = custom_stoi(input, &pos);
        } catch(...) { id = -1; }

        if (id >= 1 && id <= categoryCount) {
            system("cls");
            displayProductNamesByCategory(tempCategories[id - 1]);
        }
        
        delete[] tempCategories;
    }
    
    void sortAndDisplayProductNames() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Products (sorted by name) ===\n"; resetColor();
        if (productCount == 0) {
            setColor(RED); cout << "No products to display.\n"; resetColor();
            _getch(); return;
        }
        
        Product* tempProducts = createProductArrayFromList();
        mergeSort(tempProducts, 0, productCount - 1, true, "name");

        int maxNumLength = (int)string("No.").length();
        int maxProductNameLength = (int)string("Product Name").length();
        int maxPriceLength = (int)string("Price").length();
        int maxStockLength = (int)string("Current Stock").length();
        for (int i = 0; i < productCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxProductNameLength = manualMax(maxProductNameLength, (int)tempProducts[i].name.length());
            maxPriceLength = manualMax(maxPriceLength, (int)tempProducts[i].price.length());
            maxStockLength = manualMax(maxStockLength, (int)intToString(tempProducts[i].stock).length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int productNameColWidth = maxProductNameLength + COLUMN_PADDING_SPACES;
        int priceColWidth = maxPriceLength + COLUMN_PADDING_SPACES;
        int stockColWidth = manualMax((int)string("Current Stock").length(), maxStockLength) + COLUMN_PADDING_SPACES + 2;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(productNameColWidth, '-') + "+" + string(priceColWidth, '-') + "+" + string(stockColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(productNameColWidth) << " Product Name"
             << "|" << left << setw(priceColWidth) << " Price"
             << "|" << left << setw(stockColWidth) << " Current Stock"
             << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < productCount; i++) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(productNameColWidth) << (" " + tempProducts[i].name);
            cout << "|" << left << setw(priceColWidth) << (" " + tempProducts[i].price);
            string stockStr = intToString(tempProducts[i].stock);
            cout << "|" << right << setw(stockColWidth - 2) << stockStr << "  |" << endl;
        }
        cout << headerLine << endl;

        cout << "Enter product ID (1-" << productCount << ") to view details, or 0 to return: ";
        string input;
        int id;
        getline(cin, input);
        try {
            size_t pos;
            id = custom_stoi(input, &pos);
        } catch(...) { id = -1; }

        if (id >= 1 && id <= productCount) {
            system("cls");
            displayProductById(id, tempProducts, productCount);
            _getch();
        }
        
        delete[] tempProducts;
    }

    Product* findProductByName(const string& productName) {
        ProductNode* current = productHead;
        string lowerProductName = manualToLower(productName);
        while (current) {
            if (manualToLower(current->data.name) == lowerProductName) {
                return &(current->data);
            }
            current = current->next;
        }
        return NULL;
    }

    int getProductStock(const string& productName) {
        Product* product = findProductByName(productName);
        return product ? product->stock : -1;
    }

    bool updateProductStock(const string& productName, int quantityChange, const string& filename) {
        Product* product = findProductByName(productName);
        if (product) {
            long long newStock = (long long)product->stock + quantityChange;
            if (newStock < 0) {
                product->stock = 0;
            } else if (newStock > numeric_limits<int>::max()) {
                product->stock = numeric_limits<int>::max();
            } else {
                product->stock = (int)newStock;
            }

            try {
                saveToFile(filename);
                return true;
            } catch (const runtime_error& e) {
                cerr << "Error saving stock update: " << e.what() << endl;
                return false;
            }
        }
        return false;
    }
    
    void editProductStock(const string& productsFilename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Edit Product Stock ===\n"; resetColor();
        if (productCount == 0) {
            setColor(RED); cout << "No products available.\n"; resetColor();
            _getch(); return;
        }
        
        Product* tempProducts = createProductArrayFromList();
        mergeSort(tempProducts, 0, productCount - 1, true, "name");

        setColor(GREEN); cout << "=== Current Products (sorted by name) ===\n"; resetColor();
        int maxNumLength = (int)string("No.").length();
        int maxProductNameLength = (int)string("Product Name").length();
        int maxStockLength = (int)string("Current Stock").length();
        for (int i = 0; i < productCount; ++i) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxProductNameLength = manualMax(maxProductNameLength, (int)tempProducts[i].name.length());
            maxStockLength = manualMax(maxStockLength, (int)intToString(tempProducts[i].stock).length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int productNameColWidth = maxProductNameLength + COLUMN_PADDING_SPACES;
        int stockColWidth = manualMax((int)string("Current Stock").length(), maxStockLength) + COLUMN_PADDING_SPACES + 2;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(productNameColWidth, '-') + "+" + string(stockColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(productNameColWidth) << " Product Name"
             << "|" << left << setw(stockColWidth) << " Current Stock"
             << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < productCount; i++) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(productNameColWidth) << (" " + tempProducts[i].name);
            string stockStr = intToString(tempProducts[i].stock);
            cout << "|" << right << setw(stockColWidth - 2) << stockStr << "  |" << endl;
        }
        cout << headerLine << endl;


        cout << "Enter product ID to edit (1-" << productCount << ", or 0 to cancel): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch(...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor();
            delete[] tempProducts;
            _getch(); return;
        }
        
        if (choice >= 1 && choice <= productCount) {
            string productToEditName = tempProducts[choice - 1].name;
            int currentStock = tempProducts[choice - 1].stock;
            
            cout << "Selected: " << productToEditName << " (Current Stock: " << currentStock << ")\n";
            cout << "Enter new stock quantity: ";
            string newStockInput;
            int newStock;
            getline(cin, newStockInput);
            try {
                size_t pos;
                newStock = custom_stoi(newStockInput, &pos);
                if(newStock < 0) {
                    setColor(RED); cout << "Stock cannot be negative.\n"; resetColor();
                    delete[] tempProducts;
                    _getch(); return;
                }
            } catch(...) {
                setColor(RED); cout << "Invalid quantity.\n"; resetColor();
                delete[] tempProducts;
                _getch(); return;
            }

            Product* realProduct = findProductByName(productToEditName);
            if (realProduct) {
                realProduct->stock = newStock;
                try {
                    saveToFile(productsFilename);
                    setColor(GREEN); cout << "Stock updated successfully.\n"; resetColor();
                } catch (const runtime_error& e) {
                    setColor(RED); cout << "Failed to save stock update to file.\n"; resetColor();
                }
            }
        }
        
        delete[] tempProducts;
        _getch();
    }
    
    Product* getProductsArrayForCustomer() {
        return createProductArrayFromList();
    }

    int getProductCount() const {
        return productCount;
    }
    
    Product* getProductByName(const string& name) {
        ProductNode* current = productHead;
        while(current) {
            if(manualToLower(current->data.name) == manualToLower(name)) {
                return &(current->data);
            }
            current = current->next;
        }
        return NULL;
    }
};

struct OrderItem {
    string name;
    string priceAtPurchase;
    string category;
    int quantity;
    string timestamp;
    OrderItem(string n = "", string p = "", string c = "", int q = 0, string ts = "")
        : name(n), priceAtPurchase(p), category(c), quantity(q), timestamp(ts) {}
};

struct CustomerNode {
    OrderItem data;
    CustomerNode* next;
    CustomerNode(const OrderItem& item) : data(item), next(NULL) {}
};

class Customer {
private:
    CustomerNode* cartList;
    CustomerNode* orderHistory;
    int cartCount;
    int orderHistoryCount;
    string username;
    string password;
    bool isLoggedIn;
    ProductList* globalProductList;

    void clearList(CustomerNode*& head) {
        while (head) {
            CustomerNode* temp = head;
            head = head->next;
            delete temp;
        }
        if (&head == &cartList) cartCount = 0;
        if (&head == &orderHistory) orderHistoryCount = 0;
    }

    void insertAtEnd(CustomerNode*& head, const OrderItem& item) {
        CustomerNode* newNode = new CustomerNode(item);
        if (!head) {
            head = newNode;
        } else {
            CustomerNode* current = head;
            while (current->next) current = current->next;
            current->next = newNode;
        }
    }

    void loadOrderHistory(const string& filename) {
        ifstream file(filename.c_str());
        if (!file) return;

        clearList(orderHistory);
        string line;
        while (getline(file, line)) {
            stringstream ss(line);
            string tempUser, tempTime, tempName, tempQtyStr, tempPriceStr;
            if (getline(ss, tempUser, '\t') && getline(ss, tempTime, '\t') && 
                getline(ss, tempName, '\t') && getline(ss, tempQtyStr, '\t') && 
                getline(ss, tempPriceStr)) {
                
                if (manualToLower(tempUser) == manualToLower(username)) {
                    int tempQty;
                    try { tempQty = custom_stoi(tempQtyStr); } catch(...) { continue; }
                    string tempCategory = "N/A";
                    Product* p = globalProductList->getProductByName(tempName);
                    if(p) tempCategory = p->category;
                    
                    OrderItem item(tempName, tempPriceStr, tempCategory, tempQty, tempTime);
                    insertAtEnd(orderHistory, item);
                    orderHistoryCount++;
                }
            }
        }
        file.close();
    }

public:
    Customer(ProductList* productListPtr)
        : cartList(NULL), orderHistory(NULL), cartCount(0), orderHistoryCount(0), 
          isLoggedIn(false), globalProductList(productListPtr) {
        username = "";
        password = "";
    }
    
    ~Customer() {
        clearList(cartList);
        clearList(orderHistory);
    }

    bool registerCustomer(const string& filename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Customer Registration ===\n"; resetColor();
        string newUsername, newPassword;
        cout << "Enter new username: ";
        getline(cin, newUsername);
        newUsername.erase(0, newUsername.find_first_not_of(" \t\n\r"));
        newUsername.erase(newUsername.find_last_not_of(" \t\n\r") + 1);
        if (newUsername.empty()) {
            setColor(RED); cout << "Username cannot be empty.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        if (manualFind(newUsername, ',') != string::npos || manualFind(newUsername, '\t') != string::npos) {
            setColor(RED); cout << "Username cannot contain commas or tabs.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        if (manualFileContains(filename, newUsername)) {
            setColor(RED); cout << "Username '" << newUsername << "' already exists (case-insensitive match).\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        cout << "Enter password: ";
        newPassword = getPassword();
        if (newPassword.empty()) {
            setColor(RED); cout << "Password cannot be empty.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        if (manualFind(newPassword, ',') != string::npos || manualFind(newPassword, '\t') != string::npos) {
            setColor(RED); cout << "Password cannot contain commas or tabs.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        ofstream file(filename.c_str(), ios::app);
        if (!file) {
            setColor(RED); cout << "Failed to open registration file. Registration failed.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return false;
        }
        file << newUsername << "," << newPassword << "\n";
        file.close();
        setColor(GREEN); cout << "Registration successful for user '" << newUsername << "'!\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return true;
    }

    bool login(const string& loginFilename, const string& ordersFilename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Customer Login ===\n"; resetColor();
        int attempts = 3;
        while (attempts > 0) {
            cout << "Enter username: ";
            getline(cin, username);
            username.erase(0, username.find_first_not_of(" \t\n\r"));
            username.erase(username.find_last_not_of(" \t\n\r") + 1);
            if (username.empty()) {
                setColor(RED); cout << "Username cannot be empty.\n"; resetColor();
                attempts--;
                if (attempts > 0) {
                    cout << "Attempts remaining: " << attempts << "\n";
                    Sleep(1500); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Customer Login ===\n"; resetColor();
                }
                continue;
            }
            cout << "Enter password: ";
            password = getPassword();
            if (password.empty()) {
                setColor(RED); cout << "Password cannot be empty.\n"; resetColor();
                attempts--;
                if (attempts > 0) {
                    cout << "Attempts remaining: " << attempts << "\n";
                    Sleep(1500); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Customer Login ===\n"; resetColor();
                }
                continue;
            }
            ifstream file(loginFilename.c_str());
            if (!file) {
                setColor(RED); cout << "Login file not found. Please register first.\n"; resetColor();
                cout << "\nPress any key to return...";
                _getch();
                return false;
            }
            string line;
            bool foundCredentials = false;
            while (getline(file, line)) {
                size_t commaPos = manualFind(line, ',');
                if (commaPos == string::npos) {
                    continue;
                }
                string storedUsername = line.substr(0, commaPos);
                string storedPassword = line.substr(commaPos + 1);
                storedUsername.erase(0, storedUsername.find_first_not_of(" \t\n\r"));
                storedUsername.erase(storedUsername.find_last_not_of(" \t\n\r") + 1);
                storedPassword.erase(0, storedPassword.find_first_not_of(" \t\n\r"));
                storedPassword.erase(storedPassword.find_last_not_of(" \t\n\r") + 1);
                if (username == storedUsername && password == storedPassword) {
                    file.close();
                    isLoggedIn = true;
                    loadOrderHistory(ordersFilename);
                    setColor(GREEN); cout << "Login successful! Welcome, " << username << ".\n"; resetColor();
                    Sleep(1000);
                    return true;
                }
            }
            file.close();
            attempts--;
            setColor(RED); cout << "Invalid username or password.\n"; resetColor();
            if (attempts > 0) {
                cout << "Attempts remaining: " << attempts << "\n";
                Sleep(1500); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Customer Login ===\n"; resetColor();
            } else {
                setColor(RED); cout << "Too many failed attempts. Returning to welcome menu.\n"; resetColor();
                Sleep(2000);
                return false;
            }
        }
        return false;
    }

    void logout() {
        if (isLoggedIn) {
            isLoggedIn = false;
            username = "";
            password = "";
            clearList(cartList);
            cartCount = 0;
            clearList(orderHistory);
            orderHistoryCount = 0;
            setColor(YELLOW); cout << "Customer logged out.\n"; resetColor();
            cout << "\nPress any key to return to welcome menu...";
            _getch();
        } else {
            setColor(RED); cout << "Not logged in.\n"; resetColor();
        }
    }

    void searchProduct() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Search Product ===\n"; resetColor();
        if (!globalProductList || globalProductList->getProductCount() == 0) {
            setColor(RED); cout << "No products available to search.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        cout << "Enter part of Product Name or Category to search: ";
        string searchTerm;
        getline(cin, searchTerm);
        searchTerm.erase(0, searchTerm.find_first_not_of(" \t\n\r"));
        searchTerm.erase(searchTerm.find_last_not_of(" \t\n\r") + 1);
        if (searchTerm.empty()) {
            setColor(RED); cout << "No input provided for search.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        globalProductList->searchProduct(searchTerm);
    }
    void sortProducts() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Sort Products ===\n"; resetColor();
        if (!globalProductList || globalProductList->getProductCount() == 0) {
            setColor(RED); cout << "No products available to sort.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        cout << "Sort Options:\n";
        cout << "1. Sort by Category\n";
        cout << "2. Sort by Product Name\n";
        cout << "Enter choice (1-2, or 0 to cancel): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input. Please enter a number.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        if (choice == 0) {
            setColor(RED); cout << "Cancelled.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        if (choice == 1) {
            globalProductList->sortAndDisplayCategories();
        } else if (choice == 2) {
            globalProductList->sortAndDisplayProductNames();
        } else {
            setColor(RED); cout << "Invalid choice.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
        }
    }

    void addProductToCart() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Add Product to Cart ===\n"; resetColor();
        if (!isLoggedIn) {
             setColor(RED); cout << "Please login first.\n"; _getch(); return;
        }

        Product* availableProducts = globalProductList->getSortedProductsArray("name", true);
        int arrSize = globalProductList->getProductCount();

        if (availableProducts == NULL || arrSize == 0) {
            setColor(RED); cout << "No products available.\n"; resetColor();
            _getch();
            if (availableProducts) delete[] availableProducts;
            return;
        }
        
        setColor(GREEN); cout << "=== Available Products ===\n"; resetColor();
        int maxNumLength = (int)string("No.").length();
        int maxNameLength = (int)string("Name").length();
        int maxPriceLength = (int)string("Price").length();
        int maxCategoryLength = (int)string("Category").length();
        int maxStockLength = (int)string("Stock").length();
        for (int i = 0; i < arrSize; i++) {
            maxNumLength = manualMax(maxNumLength, (int)intToString(i + 1).length());
            maxNameLength = manualMax(maxNameLength, (int)availableProducts[i].name.length());
            maxPriceLength = manualMax(maxPriceLength, (int)availableProducts[i].price.length());
            maxCategoryLength = manualMax(maxCategoryLength, (int)availableProducts[i].category.length());
            maxStockLength = manualMax(maxStockLength, (int)intToString(availableProducts[i].stock).length());
        }
        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int nameColWidth = maxNameLength + COLUMN_PADDING_SPACES;
        int priceColWidth = maxPriceLength + COLUMN_PADDING_SPACES;
        int categoryColWidth = maxCategoryLength + COLUMN_PADDING_SPACES;
        int stockColWidth = maxStockLength + COLUMN_PADDING_SPACES + 2;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(nameColWidth, '-') + "+" + string(priceColWidth, '-') + "+" + string(categoryColWidth, '-') + "+" + string(stockColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(nameColWidth) << " Name"
             << "|" << left << setw(priceColWidth) << " Price"
             << "|" << left << setw(categoryColWidth) << " Category"
             << "|" << left << setw(stockColWidth) << " Stock"
             << "|" << endl;
        cout << headerLine << endl;
        for (int i = 0; i < arrSize; ++i) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i + 1));
            cout << "|" << left << setw(nameColWidth) << (" " + availableProducts[i].name);
            cout << "|" << left << setw(priceColWidth) << (" " + availableProducts[i].price);
            cout << "|" << left << setw(categoryColWidth) << (" " + availableProducts[i].category);
            string stockStr = intToString(availableProducts[i].stock);
            cout << "|" << right << setw(stockColWidth - 2) << stockStr << "  |" << endl;
        }
        cout << headerLine << endl;

        cout << "\nEnter product number to add (1-" << arrSize << ", or 0 to cancel): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch(...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor();
            delete[] availableProducts;
            _getch(); return;
        }
        
        if (choice >= 1 && choice <= arrSize) {
            const Product& selectedProduct = availableProducts[choice - 1];
            int q;
            cout << "Enter Quantity (max 5, current stock: " << selectedProduct.stock << "): ";
            string qtyInput;
            getline(cin, qtyInput);
            try {
                size_t pos;
                q = custom_stoi(qtyInput, &pos);
                if (q <= 0 || q > 5) {
                    setColor(RED); cout << "Invalid quantity (1-5).\n"; resetColor();
                    delete[] availableProducts;
                    _getch(); return;
                }
            } catch(...) {
                setColor(RED); cout << "Invalid quantity.\n"; resetColor();
                delete[] availableProducts;
                _getch(); return;
            }
            
            if (selectedProduct.stock >= q) {
                 OrderItem itemToAdd(selectedProduct.name, selectedProduct.price, selectedProduct.category, q, "");
                 insertAtEnd(cartList, itemToAdd);
                 cartCount++;
                 setColor(GREEN); cout << "Added to cart!\n"; resetColor();
            } else {
                 setColor(RED); cout << "Insufficient stock.\n"; resetColor();
            }
        } else {
            if (choice != 0) setColor(RED); cout << "Invalid product choice.\n"; resetColor();
        }
        
        delete[] availableProducts; 
        _getch();
    }
    
    void displayCart() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Your Cart ===\n"; resetColor();
        if (!cartList) {
            setColor(RED); cout << "Cart is empty.\n"; resetColor();
            cout << "\nPress any key to return...";
            _getch();
            return;
        }
        int maxNumLength = (int)string("No.").length();
        int maxNameLength = (int)string("Name").length();
        int maxPriceLength = (int)string("Price").length();
        int maxCategoryLength = (int)string("Category").length();
        int maxQtyLength = (int)string("Qty.").length();
        CustomerNode* temp = cartList;
        int count = 0;
        while (temp) {
            count++;
            maxNameLength = manualMax(maxNameLength, (int)temp->data.name.length());
            maxPriceLength = manualMax(maxPriceLength, (int)temp->data.priceAtPurchase.length());
            maxCategoryLength = manualMax(maxCategoryLength, (int)temp->data.category.length());
            maxQtyLength = manualMax(maxQtyLength, (int)intToString(temp->data.quantity).length());
            temp = temp->next;
        }
        maxNumLength = manualMax(maxNumLength, (int)intToString(count).length());

        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int nameColWidth = maxNameLength + COLUMN_PADDING_SPACES;
        int priceColWidth = maxPriceLength + COLUMN_PADDING_SPACES;
        int categoryColWidth = maxCategoryLength + COLUMN_PADDING_SPACES;
        int qtyColWidth = manualMax((int)string("Qty.").length(), maxQtyLength) + COLUMN_PADDING_SPACES + 2;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(nameColWidth, '-') + "+" + string(priceColWidth, '-') + "+" + string(categoryColWidth, '-') + "+" + string(qtyColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(nameColWidth) << " Name"
             << "|" << left << setw(priceColWidth) << " Price"
             << "|" << left << setw(categoryColWidth) << " Category"
             << "|" << left << setw(qtyColWidth) << " Qty."
             << "|" << endl;
        cout << headerLine << endl;
        int i = 1;
        CustomerNode* displayCurrent = cartList;
        while (displayCurrent) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i));
            cout << "|" << left << setw(nameColWidth) << (" " + displayCurrent->data.name);
            cout << "|" << left << setw(priceColWidth) << (" " + displayCurrent->data.priceAtPurchase);
            cout << "|" << left << setw(categoryColWidth) << (" " + displayCurrent->data.category);
            string qtyStr = intToString(displayCurrent->data.quantity);
            cout << "|" << right << setw(qtyColWidth - 2) << qtyStr << "  |" << endl;
            displayCurrent = displayCurrent->next;
            i++;
        }
        cout << headerLine << endl;
        cout << "\nPress any key to return...";
        _getch();
    }

    void removeFromCart() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Remove from Cart ===\n"; resetColor();
        if (!isLoggedIn) {
            setColor(RED); cout << "Please login first.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        if (!cartList) {
            setColor(RED); cout << "Cart is empty. Nothing to remove.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        displayCart();
        cout << "\nEnter the number of the product to remove (1-" << cartCount << ", or 0 to cancel): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input. Please enter a number.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        if (choice == 0) {
            setColor(RED); cout << "Removal cancelled.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        if (choice < 1 || choice > cartCount) {
            setColor(RED); cout << "Product not found in cart.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        CustomerNode* current = cartList;
        CustomerNode* prev = NULL;
        for (int i = 1; i < choice && current; i++) {
            prev = current;
            current = current->next;
        }
        if (!current) {
            setColor(RED); cout << "Product not found in cart. (Internal error).\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        if (prev) {
            prev->next = current->next;
        } else {
            cartList = current->next;
        }
        delete current;
        cartCount--;
        setColor(GREEN); cout << "Product removed from cart successfully!\n"; resetColor();
        cout << "\nPress any key to return...";_getch();
    }

    void order(const string& productsFilename, const string& ordersFilename) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Place Order ===\n"; resetColor();
        if (!isLoggedIn || !cartList) {
            setColor(RED); cout << "Please login or add items to cart first.\n"; resetColor();
            _getch(); return;
        }

        setColor(YELLOW); cout << "Items in your cart:\n"; resetColor();
        displayCart();
        
        cout << "\nConfirm your order? (1 for Yes, 0 for No): ";
        string input;
        int choice;
        getline(cin, input);
        try{
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch(...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor(); _getch(); return;
        }
        
        if (choice == 1) {
            ofstream file(ordersFilename.c_str(), ios::app);
            if (!file) {
                setColor(RED); cout << "Failed to open orders file.\n"; resetColor();
                _getch(); return;
            }

            time_t now = time(0);
            char time_buffer[50];
            strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H:%M:%S", localtime(&now));
            string dt = time_buffer;

            float totalOrderAmount = 0.0;
            system("cls");
            setColor(LIGHT_BLUE); cout << "\n=== Order Invoice ===\n"; resetColor();
            cout << "Date: " << dt << endl;
            cout << "Customer: " << username << endl;
            
            CustomerNode* current = cartList;
            while (current) {
                file << username << "\t" << dt << "\t" << current->data.name << "\t"
                     << current->data.quantity << "\t" << current->data.priceAtPurchase << endl;
                
                globalProductList->updateProductStock(current->data.name, -current->data.quantity, productsFilename);
                
                OrderItem orderedItem = current->data;
                orderedItem.timestamp = dt;
                insertAtEnd(orderHistory, orderedItem);
                orderHistoryCount++;
                
                current = current->next;
            }
            file.close();
            
            clearList(cartList);
            
            setColor(GREEN); cout << "Order placed successfully! Thank you.\n"; resetColor();
            _getch();
        } else {
            setColor(RED); cout << "Order cancelled.\n"; resetColor();
            _getch();
        }
    }
    
    void viewOrderHistory() {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Purchase Histories ===\n"; resetColor();
        if (!isLoggedIn) {
            setColor(RED); cout << "Please login first to view purchase history.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        if (!orderHistory) {
            setColor(RED); cout << "No purchase history available.\n"; resetColor();
            cout << "\nPress any key to return...";_getch();return;
        }
        CustomerNode* current = orderHistory;
        int maxNumLength = (int)string("No.").length();
        int maxNameLength = (int)string("Product").length();
        int maxQtyLength = (int)string("Qty").length();
        int maxPriceLength = (int)string("Price").length();
        int maxTimeLength = (int)string("Timestamp").length();
        CustomerNode* temp = orderHistory;
        int count = 0;
        while (temp) {
            count++;
            maxNameLength = manualMax(maxNameLength, (int)temp->data.name.length());
            maxQtyLength = manualMax(maxQtyLength, (int)intToString(temp->data.quantity).length());
            maxPriceLength = manualMax(maxPriceLength, (int)temp->data.priceAtPurchase.length());
            maxTimeLength = manualMax(maxTimeLength, (int)temp->data.timestamp.length());
            temp = temp->next;
        }
        maxNumLength = manualMax(maxNumLength, (int)intToString(count).length());

        const int COLUMN_PADDING_SPACES = 2;
        int numColWidth = maxNumLength + COLUMN_PADDING_SPACES;
        int nameColWidth = maxNameLength + COLUMN_PADDING_SPACES;
        int qtyColWidth = manualMax((int)string("Qty").length(), maxQtyLength) + COLUMN_PADDING_SPACES + 2;
        int priceColWidth = maxPriceLength + COLUMN_PADDING_SPACES;
        int timeColWidth = manualMax((int)string("Timestamp").length(), maxTimeLength) + COLUMN_PADDING_SPACES;
        string headerLine = "+" + string(numColWidth, '-') + "+" + string(nameColWidth, '-') + "+" + string(priceColWidth, '-') + "+" + string(qtyColWidth, '-') + "+" + string(timeColWidth, '-') + "+";
        cout << headerLine << endl;
        cout << "|" << left << setw(numColWidth) << " No."
             << "|" << left << setw(nameColWidth) << " Product"
             << "|" << left << setw(priceColWidth) << " Price"
             << "|" << left << setw(qtyColWidth) << " Qty"
             << "|" << left << setw(timeColWidth) << " Timestamp"
             << "|" << endl;
        cout << headerLine << endl;
        int i = 1;
        current = orderHistory;
        while (current) {
            cout << "|" << left << setw(numColWidth) << (" " + intToString(i));
            cout << "|" << left << setw(nameColWidth) << (" " + current->data.name);
            cout << "|" << left << setw(priceColWidth) << (" " + current->data.priceAtPurchase);
            string qtyStr = intToString(current->data.quantity);
            cout << "|" << right << setw(qtyColWidth - 2) << qtyStr << "  |";
            cout << left << setw(timeColWidth) << (" " + current->data.timestamp);
            cout << "|" << endl;
            current = current->next;
            i++;
        }
        cout << headerLine << endl;
        cout << "\nPress any key to return...";
        _getch();
    }
    
    bool isUserLoggedIn() const { return isLoggedIn; }
};

bool adminLogin(const string& staffFilename);
void adminRegister(const string& staffFilename);
void displayAdminMenu(ProductList& list, const string& categoriesFilename, const string& productsFilename, const string& ordersFilename);
void displayCustomerMenu(Customer& cust, const string& productsFilename, const string& ordersFilename);
bool authenticateUser(const string& username, const string& password, const string& filename);

int main() {
    ProductList globalProductList;
    Customer customerApp(&globalProductList);
    
    // Updated filenames as per your request
    string categoriesFilename = "categories.txt";
    string productsFilename = "products.txt"; 
    string staffFilename = "admin.txt";
    string loginFilename = "customer.txt";
    string ordersFilename = "purchase_history.txt";

    while (true) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n==== Welcome to The Furnish Shop ====\n"; resetColor();
        cout << "1. Admin Login/Registration\n";
        cout << "2. Customer Login/Registration\n";
        cout << "3. Exit Program\n";
        cout << "Enter your choice (1-3): ";
        
        string input;
        int initialChoice;
        getline(cin, input);
        try {
            size_t pos;
            initialChoice = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input.\n"; resetColor();
            Sleep(1500);
            continue;
        }

        switch (initialChoice) {
            case 1: { 
                bool adminAccessGranted = false;
                while (!adminAccessGranted) {
                    system("cls");
                    setColor(LIGHT_BLUE); cout << "\n=== Admin Access ===\n"; resetColor();
                    cout << "1. Admin Register\n";
                    cout << "2. Admin Login\n";
                    cout << "3. Back to Main Menu\n";
                    cout << "Enter choice (1-3): ";
                    string adminInput;
                    int adminChoice;
                    getline(cin, adminInput);
                     try {
                        size_t pos;
                        adminChoice = custom_stoi(adminInput, &pos);
                    } catch (...) {
                        setColor(RED); cout << "Invalid input.\n"; Sleep(1500); continue;
                    }

                    switch(adminChoice) {
                        case 1: adminRegister(staffFilename); break;
                        case 2:
                             if (adminLogin(staffFilename)) {
                                adminAccessGranted = true;
                                try {
                                    cout << "Loading data...\n";
                                    globalProductList.loadCategories(categoriesFilename);
                                    globalProductList.loadFromFile(productsFilename);
                                    showLoading();
                                    displayAdminMenu(globalProductList, categoriesFilename, productsFilename, ordersFilename);
                                } catch (const runtime_error& e) {
                                     cerr << "Fatal Error during data loading: " << e.what() << endl;
                                    _getch(); return 1;
                                }
                            }
                            break;
                        case 3: adminAccessGranted = true; break;
                        default: setColor(RED); cout << "Invalid choice.\n"; Sleep(1500); break;
                    }
                }
                break;
            }
            case 2: {
                 bool customerAccessGranted = false;
                 while (!customerAccessGranted) {
                    system("cls");
                    setColor(LIGHT_BLUE); cout << "\n=== Customer Access ===\n"; resetColor();
                    cout << "1. Customer Register\n";
                    cout << "2. Customer Login\n";
                    cout << "3. Back to Main Menu\n";
                    cout << "Enter choice (1-3): ";
                    string custInput;
                    int custChoice;
                     getline(cin, custInput);
                     try {
                        size_t pos;
                        custChoice = custom_stoi(custInput, &pos);
                    } catch (...) {
                        setColor(RED); cout << "Invalid input.\n"; Sleep(1500); continue;
                    }
                    
                    switch(custChoice) {
                        case 1: customerApp.registerCustomer(loginFilename); break;
                        case 2:
                            if (customerApp.login(loginFilename, ordersFilename)) {
                                customerAccessGranted = true;
                                 try {
                                    cout << "Loading products...\n";
                                    globalProductList.loadCategories(categoriesFilename);
                                    globalProductList.loadFromFile(productsFilename);
                                    showLoading();
                                    displayCustomerMenu(customerApp, productsFilename, ordersFilename);
                                } catch (const runtime_error& e) {
                                     cerr << "Fatal Error during data loading: " << e.what() << endl;
                                    _getch(); return 1;
                                }
                            }
                            break;
                        case 3: customerAccessGranted = true; break;
                        default: setColor(RED); cout << "Invalid choice.\n"; Sleep(1500); break;
                    }
                 }
                 break;
            }
            case 3:
                setColor(RED); cout << "Exiting The Furnish Shop program. Goodbye!\n"; resetColor();
                Sleep(1000);
                return 0;
            default:
                setColor(RED); cout << "Invalid choice.\n"; resetColor();
                Sleep(1000);
                break;
        }
    }
    return 0;
}

bool authenticateUser(const string& username, const string& password, const string& filename) {
    ifstream file(filename.c_str());
    if (!file) {
        return false;
    }
    string line;
    while (getline(file, line)) {
        size_t first = line.find_first_not_of(" \t\n\r");
        if (string::npos != first) {
            size_t last = line.find_last_not_of(" \t\n\r");
            line = line.substr(first, (last - first + 1));
        } else {
            line = "";
        }
        if (line.empty()) continue;
        size_t commaPos = manualFind(line, ',');
        if (commaPos == string::npos) {
            continue;
        }
        string storedUsername = line.substr(0, commaPos);
        string storedPassword = line.substr(commaPos + 1);
        storedUsername.erase(0, storedUsername.find_first_not_of(" \t\n\r"));
        storedUsername.erase(storedUsername.find_last_not_of(" \t\n\r") + 1);
        storedPassword.erase(0, storedPassword.find_first_not_of(" \t\n\r"));
        storedPassword.erase(storedPassword.find_last_not_of(" \t\n\r") + 1);
        if (username == storedUsername && password == storedPassword) {
            file.close();
            return true;
        }
    }
    file.close();
    return false;
}

bool adminLogin(const string& staffFilename) {
    system("cls");
    string username, password;
    int attempts = 3;
    setColor(LIGHT_BLUE); cout << "\n=== Admin Login ===\n"; resetColor();
    while (attempts > 0) {
        cout << "Enter username: ";
        getline(cin, username);
        username.erase(0, username.find_first_not_of(" \t\n\r"));
        username.erase(username.find_last_not_of(" \t\n\r") + 1);
        if (username.empty()) {
            setColor(RED); cout << "Username cannot be empty.\n"; resetColor();
            attempts--;
            if (attempts > 0) {
                cout << "Attempts remaining: " << attempts << "\n";
                Sleep(1000); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Admin Login ===\n"; resetColor();
            }
            continue;
        }
        cout << "Enter password: ";
        password = getPassword();
        if (password.empty()) {
            setColor(RED); cout << "Password cannot be empty.\n"; resetColor();
            attempts--;
            if (attempts > 0) {
                cout << "Attempts remaining: " << attempts << "\n";
                Sleep(500); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Admin Login ===\n"; resetColor();
            }
            continue;
        }
        if (authenticateUser(username, password, staffFilename)) {
            return true;
        } else {
            attempts--;
            setColor(RED); cout << "Invalid credentials.\n"; resetColor();
            if (attempts > 0) {
                cout << "Attempts remaining: " << attempts << "\n";
                Sleep(1500); system("cls"); setColor(LIGHT_BLUE); cout << "\n=== Admin Login ===\n"; resetColor();
            }
        }
    }
    setColor(RED); cout << "Too many failed attempts. Returning to main menu.\n"; resetColor();
    Sleep(2000);
    return false;
}

void adminRegister(const string& staffFilename) {
    system("cls");
    string username, password;
    setColor(LIGHT_BLUE); cout << "\n=== Admin Registration ===\n"; resetColor();
    bool fileExistsInitially = ifstream(staffFilename.c_str()).good();
    cout << "Enter desired username: ";
    getline(cin, username);
    username.erase(0, username.find_first_not_of(" \t\n\r"));
    username.erase(username.find_last_not_of(" \t\n\r") + 1);
    if (username.empty()) {
        setColor(RED); cout << "Username cannot be empty.\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return;
    }
    if (manualFind(username, ',') != string::npos) {
        setColor(RED); cout << "Username cannot contain commas.\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return;
    }
    if (fileExistsInitially && manualFileContains(staffFilename, username)) {
        setColor(RED); cout << "Username '" << username << "' already exists (case-insensitive match).\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return;
    }
    cout << "Enter desired password: ";
    password = getPassword();
    if (password.empty()) {
        setColor(RED); cout << "Password cannot be empty.\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return;
    }
    if (manualFind(password, ',') != string::npos) {
        setColor(RED); cout << "Password cannot contain commas.\n"; resetColor();
        cout << "\nPress any key to return...";
        _getch();
        return;
    }
    ofstream outFile(staffFilename.c_str(), ios::app);
    if (!outFile) {
        cerr << "Error: Could not open staff file for writing: " << staffFilename << endl;
        setColor(RED); cout << "Registration failed due to file error.\n"; resetColor();
    } else {
        outFile << username << "," << password << endl;
        outFile.close();
        setColor(GREEN); cout << "Admin user '" << username << "' registered successfully.\n"; resetColor();
    }
    cout << "\nPress any key to return...";
    _getch();
}

void displayAdminMenu(ProductList& list, const string& categoriesFilename, const string& productsFilename, const string& ordersFilename) {
    bool adminLoggedIn = true;
    while (adminLoggedIn) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Admin Menu ===\n"; resetColor();
        cout << "1. View Categories (Sorted)\n";
        cout << "2. View Products (Sorted by Name)\n";
        cout << "3. Add New Category\n";
        cout << "4. Add New Product\n";
        cout << "5. Delete Category\n";
        cout << "6. Delete Product\n";
        cout << "7. Search Product (by Name or Category)\n";
        cout << "8. Search Category (by Name)\n";
        cout << "9. Edit Product Stock\n";
        cout << "10. Logout\n";
        cout << "Enter your choice (1-10): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input. Please enter a number.\n"; resetColor();
            Sleep(1500);
            continue;
        }

        switch (choice) {
            case 1:
                list.sortAndDisplayCategories();
                break;
            case 2:
                list.sortAndDisplayProductNames();
                break;
            case 3:
                list.addCategory(categoriesFilename);
                break;
            case 4:
                list.addProduct(productsFilename);
                break;
            case 5:
                list.deleteCategory(categoriesFilename, productsFilename);
                cout << "\nPress any key to return to menu...";
                _getch();
                break;
            case 6:
                list.deleteProduct(productsFilename, ordersFilename);
                cout << "\nPress any key to return to menu...";
                _getch();
                break;
            case 7: {
                system("cls");
                setColor(LIGHT_BLUE); cout << "\n=== Search Product ===\n"; resetColor();
                string searchTerm;
                cout << "Enter product name or category to search: ";
                getline(cin, searchTerm);
                list.searchProduct(searchTerm);
                break;
            }
            case 8: {
                system("cls");
                setColor(LIGHT_BLUE); cout << "\n=== Search Category ===\n"; resetColor();
                string searchTerm;
                cout << "Enter category name to search: ";
                getline(cin, searchTerm);
                list.searchCategory(searchTerm);
                break;
            }
            case 9:
                list.editProductStock(productsFilename);
                break;
            case 10:
                adminLoggedIn = false;
                setColor(YELLOW); cout << "Admin logged out. Returning to main menu.\n"; resetColor();
                Sleep(1500);
                break;
            default:
                setColor(RED); cout << "Invalid choice. Please enter a number from 1 to 10.\n"; resetColor();
                Sleep(1500);
                break;
        }
    }
}

void displayCustomerMenu(Customer& cust, const string& productsFilename, const string& ordersFilename) {
    bool customerLoggedIn = true;
    while (customerLoggedIn) {
        system("cls");
        setColor(LIGHT_BLUE); cout << "\n=== Customer Menu ===\n"; resetColor();
        cout << "1. Add Product to Cart\n";
        cout << "2. View Cart\n";
        cout << "3. Search Product\n";
        cout << "4. Sort Products (by Category or Name)\n";
        cout << "5. Remove from Cart\n";
        cout << "6. Place Order\n";
        cout << "7. View Order History\n";
        cout << "8. Logout\n";
        cout << "Enter your choice (1-8): ";
        string input;
        int choice;
        getline(cin, input);
        try {
            size_t pos;
            choice = custom_stoi(input, &pos);
        } catch (...) {
            setColor(RED); cout << "Invalid input. Please enter a number.\n"; resetColor();
            Sleep(1500);
            continue;
        }
        
        switch (choice) {
            case 1: cust.addProductToCart(); break;
            case 2: cust.displayCart(); break;
            case 3: cust.searchProduct(); break;
            case 4: cust.sortProducts(); break;
            case 5: cust.removeFromCart(); break;
            case 6: cust.order(productsFilename, ordersFilename); break;
            case 7: cust.viewOrderHistory(); break;
            case 8:
                customerLoggedIn = false;
                cust.logout();
                break;
            default:
                setColor(RED); cout << "Invalid choice. Please enter 1 to 8.\n"; resetColor();
                Sleep(1500);
                break;
        }
    }
}
