#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

using namespace std;


// -------------------------------------------------------
// Email object
// -------------------------------------------------------
class Email {
private:
    string sender;
    string subject;
    string date;
    int arrivalOrder;

public:
    Email() {
        sender = "";
        subject = "";
        date = "";
        arrivalOrder = 0;
    }

    Email(string s, string sub, string d, int order) {
        sender = s;
        subject = sub;
        date = d;
        arrivalOrder = order;
    }

    string getSender() const {
        return sender;
    }

    string getSubject() const {
        return subject;
    }

    string getDate() const {
        return date;
    }

    int getArrivalOrder() const {
        return arrivalOrder;
    }

    // Gives each sender category a priority number
    int getSenderPriority() const {
        if (sender == "Boss")
            return 5;
        else if (sender == "Subordinate")
            return 4;
        else if (sender == "Peer")
            return 3;
        else if (sender == "ImportantPerson")
            return 2;
        else if (sender == "OtherPerson")
            return 1;

        return 0;
    }

    // Turns MM-DD-YYYY into YYYYMMDD
    // This makes dates easy to compare as integers
    int getDateValue() const {
        int month, day, year;
        char dash1, dash2;

        stringstream ss(date);

        ss >> month >> dash1 >> day >> dash2 >> year;

        return year * 10000 + month * 100 + day;
    }

    void display() const {
        cout << "Next email:" << endl;
        cout << "\tSender: " << sender << endl;
        cout << "\tSubject: " << subject << endl;
        cout << "\tDate: " << date << endl;
    }
};


// -------------------------------------------------------
// MaxHeap object
// -------------------------------------------------------
class MaxHeap {
private:
    vector<Email> heap;

    // Returns true when email a should be placed
    // ahead of email b.
    bool higherPriority(const Email& a, const Email& b) const {

        // First compare sender category
        if (a.getSenderPriority() != b.getSenderPriority()) {
            return a.getSenderPriority() > b.getSenderPriority();
        }

        // Same sender category:
        // newest email should come first
        if (a.getDateValue() != b.getDateValue()) {
            return a.getDateValue() > b.getDateValue();
        }

        // If both sender priority and date are identical,
        // keep the one that arrived first ahead.
        return a.getArrivalOrder() < b.getArrivalOrder();
    }

    void swapEmails(int a, int b) {
        Email temp = heap[a];
        heap[a] = heap[b];
        heap[b] = temp;
    }

    void heapifyUp(int index) {

        while (index > 0) {

            int parent = (index - 1) / 2;

            if (higherPriority(heap[index], heap[parent])) {
                swapEmails(index, parent);
                index = parent;
            }
            else {
                break;
            }
        }
    }

    void heapifyDown(int index) {

        int size = heap.size();

        while (true) {

            int leftChild = 2 * index + 1;
            int rightChild = 2 * index + 2;

            int highest = index;

            if (leftChild < size &&
                higherPriority(heap[leftChild], heap[highest])) {

                highest = leftChild;
            }

            if (rightChild < size &&
                higherPriority(heap[rightChild], heap[highest])) {

                highest = rightChild;
            }

            if (highest == index) {
                break;
            }

            swapEmails(index, highest);
            index = highest;
        }
    }

public:
    MaxHeap() {
    }

    void insert(const Email& email) {

        heap.push_back(email);

        int newIndex = heap.size() - 1;

        heapifyUp(newIndex);
    }

    bool isEmpty() const {
        return heap.empty();
    }

    int getCount() const {
        return heap.size();
    }

    // NEXT
    void displayNext() const {

        if (isEmpty()) {
            cout << "There are no emails to read." << endl;
            return;
        }

        heap[0].display();
    }

    // READ
    void removeNext() {

        if (isEmpty()) {
            return;
        }

        // If only one email exists, just remove it
        if (heap.size() == 1) {
            heap.pop_back();
            return;
        }

        // Move the last email to the root
        heap[0] = heap.back();

        // Delete the old last position
        heap.pop_back();

        // Restore MaxHeap order
        heapifyDown(0);
    }

    // COUNT
    void displayCount() const {
        cout << "There are " << heap.size()
             << " emails to read." << endl;
    }
};


// -------------------------------------------------------
// CEO Inbox object
// -------------------------------------------------------
class CEOInbox {
private:
    MaxHeap emails;
    int arrivalCounter;

public:
    CEOInbox() {
        arrivalCounter = 0;
    }

    void addEmail(string line) {

        // Remove "EMAIL "
        string information = line.substr(6);

        int firstComma = information.find(',');
        int secondComma = information.find(',', firstComma + 1);

        string sender =
            information.substr(0, firstComma);

        string subject =
            information.substr(
                firstComma + 1,
                secondComma - firstComma - 1
            );

        string date =
            information.substr(secondComma + 1);

        Email newEmail(
            sender,
            subject,
            date,
            arrivalCounter
        );

        arrivalCounter++;

        emails.insert(newEmail);
    }

    void processCommand(string line) {

        if (line.substr(0, 6) == "EMAIL ") {

            addEmail(line);
        }
        else if (line == "NEXT") {

            emails.displayNext();
        }
        else if (line == "READ") {

            emails.removeNext();
        }
        else if (line == "COUNT") {

            emails.displayCount();
        }
    }

    void processFile(string fileName) {

        ifstream inputFile(fileName);

        if (!inputFile.is_open()) {
            cout << "Unable to open file." << endl;
            return;
        }

        string line;

        while (getline(inputFile, line)) {

            // Handles Windows-style line endings
            if (!line.empty() && line.back() == '\r') {
                line.pop_back();
            }

            if (!line.empty()) {
                processCommand(line);
            }
        }

        inputFile.close();
    }
};


// -------------------------------------------------------
// Main
// -------------------------------------------------------
int main(int argc, char* argv[]) {

    CEOInbox inbox;

    string fileName;

    // Allows:
    // ./program test.txt
    if (argc > 1) {
        fileName = argv[1];
    }
    else {
        cout << "Enter test file name: ";
        cin >> fileName;
    }

    inbox.processFile(fileName);

    return 0;
}