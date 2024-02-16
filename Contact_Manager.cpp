#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stack>

using namespace std;

// Node for Doubly Linked List
struct Contact {
    string name;
    string phone;
    string category;
    Contact* prev;
    Contact* next;
};

// Graph Node for Relationships
struct GraphNode {
    string name;
    string category;
    vector<GraphNode*> connections;
};

// Class for Contact Manager
class ContactManager {
private:
    Contact* head; // Doubly Linked List head
    stack<Contact*> undoStack; // Stack for undo functionality
    stack<Contact*> redoStack; // Stack for redo functionality
    vector<GraphNode*> graph; // Graph for relationships
    string contactsFile; // File to store contacts

public:
    // Constructor
    ContactManager(const string& fileName) : head(nullptr), contactsFile(fileName) {}

    // Function to add a contact
    void addContact(const string& name, const string& phone, const string& category) {
        if (name.empty() || phone.empty() || category.empty()) {
            cerr << "Error: Name, phone, and category cannot be empty. Contact not added." << endl;
            return;
        }

        if (phone.length() != 10 || !isDigits(phone)) {
            cerr << "Error: Phone number must be exactly 10 digits. Contact not added." << endl;
            return;
        }

        Contact* newContact = new Contact{ name, phone, category, nullptr, nullptr };

        // Add to Doubly Linked List
        if (head == nullptr) {
            head = newContact;
        }
        else {
            newContact->next = head;
            head->prev = newContact;
            head = newContact;
        }

        // Add to Undo Stack
        undoStack.push(newContact);

        // Update Graph
        updateGraph(name, category);

        // Save to file
        saveToFile();
    }

    // Function to undo the last action
 void undo() {
    if (!undoStack.empty()) {
        Contact* lastAction = undoStack.top();
        undoStack.pop();

        // Remove from Doubly Linked List
        if (lastAction->next != nullptr) {
            lastAction->next->prev = lastAction->prev;
        }

        if (lastAction->prev != nullptr) {
            lastAction->prev->next = lastAction->next;
        } else {
            // If there is no previous node, it means 'lastAction' was the head
            head = lastAction->next;
        }

        // Add to Redo Stack
        redoStack.push(lastAction);

        // Update Graph
        removeGraphEntry(lastAction->name);

        // Save to file
        saveToFile();
    } else {
        cerr << "Error: No action to undo." << endl;
    }
}

    // Function to redo the last undone action
    void redo() {
        if (!redoStack.empty()) {
            Contact* redoAction = redoStack.top();
            redoStack.pop();

            // Add back to Doubly Linked List
            if (head != nullptr) {
                redoAction->next = head;
                head->prev = redoAction;
            }
            head = redoAction;

            // Add to Undo Stack
            undoStack.push(redoAction);

            // Update Graph
            updateGraph(redoAction->name, redoAction->category);

            // Save to file
            saveToFile();
        }
        else {
            cerr << "Error: No action to redo." << endl;
        }
    }

    // Function to display all contacts
    void displayContacts() {
        if (head == nullptr) {
            cout << "No contacts to display." << endl;
            return;
        }

        Contact* current = head;
        while (current != nullptr) {
            cout << "Name: " << current->name << "\tPhone: " << current->phone
                 << "\tCategory: " << current->category << endl;
            current = current->next;
        }
    }

    // Function to search for a contact
    void searchContact(const string& searchTerm) {
        if (head == nullptr) {
            cout << "No contacts to search." << endl;
            return;
        }

        Contact* current = head;
        while (current != nullptr) {
            // Check if the search term matches the contact name, phone number, or category
            if (current->name.find(searchTerm) != string::npos ||
                current->phone.find(searchTerm) != string::npos ||
                current->category.find(searchTerm) != string::npos) {
                cout << "Contact found: Name: " << current->name << "\tPhone: " << current->phone
                     << "\tCategory: " << current->category << endl;
            }
            current = current->next;
        }
    }

    // Function to search contacts by category
    void searchByCategory(const string& category) {
        if (head == nullptr) {
            cout << "No contacts to search." << endl;
            return;
        }

        Contact* current = head;
        while (current != nullptr) {
            if (current->category == category) {
                cout << "Contact found: Name: " << current->name << "\tPhone: " << current->phone
                     << "\tCategory: " << current->category << endl;
            }
            current = current->next;
        }
    }

    // Function to delete a contact
    void deleteContact(const string& name) {
        if (head == nullptr) {
            cout << "No contacts to delete." << endl;
            return;
        }

        Contact* current = head;
        while (current != nullptr) {
            if (current->name == name) {
                // Remove from Doubly Linked List
                if (current->prev != nullptr) {
                    current->prev->next = current->next;
                }
                else {
                    head = current->next;
                }

                if (current->next != nullptr) {
                    current->next->prev = current->prev;
                }

                // Update Graph
                removeGraphEntry(name);

                // Save to file
                saveToFile();

                cout << "Contact deleted: Name: " << current->name << "\tPhone: " << current->phone
                     << "\tCategory: " << current->category << endl;

                delete current; // Free memory
                return;
            }
            current = current->next;
        }

        cout << "Contact not found. Deletion failed." << endl;
    }

// Function to update a contact
void updateContact(const string& name, const string& newPhone = "", const string& newCategory = "") {
    if (head == nullptr) {
        cout << "No contacts to update." << endl;
        return;
    }

    Contact* current = head;
    while (current != nullptr) {
        if (current->name == name) {
            int updateChoice;

            // Print contact details before asking for update choice
            cout << "Current Contact Details:\n";
            cout << "Name: " << current->name << "\tPhone: " << current->phone
                 << "\tCategory: " << current->category << endl;

            cout << "Select what to update:\n";
            cout << "1. Update Name\n";
            cout << "2. Update Phone Number\n";
            cout << "3. Update Category\n";
            cout << "Enter your choice: ";
            cin >> updateChoice;

            switch (updateChoice) {
            case 1:
                cout << "Enter the new name: ";
                cin.ignore();
                getline(cin, current->name);
                break;

            case 2:
                cout << "Enter the new phone number: ";
                cin.ignore();
                getline(cin, current->phone);
                break;

            case 3:
                cout << "Enter the new category: ";
                cin.ignore();
                getline(cin, current->category);
                break;

            default:
                cerr << "Error: Invalid choice. Contact not updated." << endl;
                return;
            }

            // Add to Undo Stack
            undoStack.push(current);

            // Update Graph
            removeGraphEntry(name);
            updateGraph(current->name, newCategory);

            // Save to file
            saveToFile();

            cout << "Contact updated: Name: " << current->name << "\tPhone: " << current->phone
                 << "\tCategory: " << current->category << endl;
            return;
        }
        current = current->next;
    }

    cout << "Contact not found. Update failed." << endl;
}

    // Function to sort contacts by name
    void sortContacts() {
        if (head == nullptr) {
            cout << "No contacts to sort." << endl;
            return;
        }

        vector<string> names;
        Contact* current = head;
        while (current != nullptr) {
            names.push_back(current->name);
            current = current->next;
        }

        sort(names.begin(), names.end());

        // Display sorted contacts
        cout << "Sorted Contacts:" << endl;
        for (const auto& name : names) {
            searchContact(name);
        }
    }

    // Function to check if a string contains only digits
    bool isDigits(const string& str) {
        return all_of(str.begin(), str.end(), ::isdigit);
    }

    // Function to update the graph with relationships
    void updateGraph(const string& name, const string& category) {
        GraphNode* newNode = new GraphNode{ name, category, {} };
        graph.push_back(newNode);

        // Connect the new node with existing nodes based on some logic
        // For simplicity, connect with the last added node in the graph
        if (graph.size() > 1) {
            graph[graph.size() - 2]->connections.push_back(newNode);
            newNode->connections.push_back(graph[graph.size() - 2]);
        }
    }

    // Function to remove a graph entry when undoing an action
    void removeGraphEntry(const string& name) {
        auto it = find_if(graph.begin(), graph.end(),
            [name](const GraphNode* node) { return node->name == name; });

        if (it != graph.end()) {
            graph.erase(it);
        }
    }

    // Function to save contacts to a file
    void saveToFile() {
        ofstream file(contactsFile);
        if (file.is_open()) {
            Contact* current = head;
            while (current != nullptr) {
                file << current->name << "," << current->phone << "," << current->category << endl;
                current = current->next;
            }
            file.close();
        }
        else {
            cerr << "Error: Unable to open file for saving contacts." << endl;
        }
    }

    // Function to load contacts from a file
    void loadFromFile() {
        ifstream file(contactsFile);
        if (file.is_open()) {
            string line;
            while (getline(file, line)) {
                stringstream ss(line);
                string name, phone, category;
                getline(ss, name, ',');
                getline(ss, phone, ',');
                getline(ss, category);
                addContact(name, phone, category);
            }
            file.close();
        }
        else {
            cerr << "Warning: Contacts file not found. Creating a new file." << endl;
        }
    }

    // Function to check if a contact with a given name already exists
    bool contactExists(const string& name) {
        Contact* current = head;
        while (current != nullptr) {
            if (current->name == name) {
                return true;
            }
            current = current->next;
        }
        return false;
    }
~ContactManager() {
    // Release memory for contacts
    while (head != nullptr) {
        Contact* temp = head;
        head = head->next;
        delete temp;
    }

    // Release memory for graph nodes
    for (auto node : graph) {
        delete node;
    }
}

};

    int getNumericInput() {
    int input;
    while (!(cin >> input)) {
        cout << "Invalid input. Please enter a number: ";
        cin.clear();
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
    }
    return input;
}
int main() {
    const string contactsFile = "contacts.txt";
    ContactManager contactManager(contactsFile);

    // Load contacts from file
    contactManager.loadFromFile();

    int choice;
    string name, phone, category, searchTerm, newPhone, newCategory;

    do {
        cout << "\n===== Contact Manager Menu =====\n";
        cout << "1. Add Contact\n";
        cout << "2. Display Contacts\n";
        cout << "3. Undo\n";
        cout << "4. Redo\n";
        cout << "5. Search Contact\n";
        cout << "6. Search by Category\n";
        cout << "7. Delete Contact\n";
        cout << "8. Update Contact\n";
        cout << "9. Sort Contacts\n";
        cout << "0. Exit\n";
        cout << "Enter your choice: ";
        choice=getNumericInput();

        switch (choice) {
            case 1:
                cout << "Enter contact name: ";
                cin.ignore();
                getline(cin, name);
                cout << "Enter contact phone: ";
                getline(cin, phone);
                cout << "Enter contact category: ";
                getline(cin, category);
                contactManager.addContact(name, phone, category);
                break;

            case 2:
                cout << "\n===== All Contacts =====\n";
                contactManager.displayContacts();
                break;

            case 3:
                contactManager.undo();
                break;

            case 4:
                contactManager.redo();
                break;

            case 5:
                cout << "Enter search term: ";
                cin.ignore();
                getline(cin, searchTerm);
                contactManager.searchContact(searchTerm);
                break;

            case 6:
                cout << "Enter category to search: ";
                cin.ignore();
                getline(cin, category);
                contactManager.searchByCategory(category);
                break;

            case 7:
                contactManager.displayContacts();
                cout << "Enter the name of the contact to delete: ";
                cin.ignore();
                getline(cin, searchTerm);
                contactManager.deleteContact(searchTerm);
                break;

            case 8:
                {
                cout << "Enter the name of the contact to update: ";
                cin.ignore();
                getline(cin, searchTerm);

                // Check if the contact exists before proceeding to update
                if (contactManager.contactExists(searchTerm)) {
                    contactManager.updateContact(searchTerm, newPhone, newCategory);
                }
                else {
                    cerr << "Error: Contact not found. Update failed." << endl;
                }
                break;
            }

            case 9:
                contactManager.sortContacts();
                break;

            case 0:
                cout << "Exiting program.\n";
                break;

            default:
                cerr << "Error: Invalid choice. Please enter a valid option.\n";
        }

    } while (choice != 0);

    return 0;
}
