// email_priority_queue.cpp
//
// CEO Email Priority Queue
// -------------------------
// Prioritizes a CEO's inbox using a hand-written, list/array-based MaxHeap
// (no std::priority_queue or <algorithm> heap functions anywhere). Emails
// are ordered first by sender category (Boss > Subordinate > Peer >
// ImportantPerson > OtherPerson) and, within the same category, by date
// (newest first).
//
// Commands (one per line):
//   EMAIL <senderCategory>,<subject>,<date>   -> enqueue a new email
//   NEXT                                       -> peek at top email (no removal)
//   READ                                       -> remove top email (no display)
//   COUNT                                      -> print number of unread emails
//
// Reads from a file given as a command-line argument, or from stdin if none
// is given, so it works with both "prog file.txt" and "prog < file.txt".

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdexcept>
#include <cctype>

using namespace std;

// ---------------------------------------------------------------------
// StringUtils - small helper object for whitespace trimming
// ---------------------------------------------------------------------
class StringUtils {
public:
    static string trim(const string& text) {
        size_t start = 0;
        while (start < text.size() &&
               isspace(static_cast<unsigned char>(text[start]))) {
            start++;
        }
        size_t end = text.size();
        while (end > start &&
               isspace(static_cast<unsigned char>(text[end - 1]))) {
            end--;
        }
        return text.substr(start, end - start);
    }
};

// ---------------------------------------------------------------------
// Date - represents a MM-DD-YYYY date and knows how to compare itself
// ---------------------------------------------------------------------
class Date {
public:
    Date() : month(0), day(0), year(0), originalText("") {}

    static Date fromString(const string& text) {
        size_t firstDash = text.find('-');
        size_t secondDash = (firstDash == string::npos)
                                 ? string::npos
                                 : text.find('-', firstDash + 1);

        if (firstDash == string::npos || secondDash == string::npos) {
            throw invalid_argument("date '" + text + "' is not in MM-DD-YYYY format");
        }

        Date result;
        result.originalText = text;
        try {
            result.month = stoi(text.substr(0, firstDash));
            result.day = stoi(text.substr(firstDash + 1, secondDash - firstDash - 1));
            result.year = stoi(text.substr(secondDash + 1));
        } catch (const exception&) {
            throw invalid_argument("date '" + text + "' contains non-numeric fields");
        }
        return result;
    }

    // Returns true if this date comes after (is newer than) other.
    bool isNewerThan(const Date& other) const {
        if (year != other.year) return year > other.year;
        if (month != other.month) return month > other.month;
        return day > other.day;
    }

    string toString() const { return originalText; }

private:
    int month;
    int day;
    int year;
    string originalText;
};

// ---------------------------------------------------------------------
// SenderCategory - maps a sender category name to a numeric priority
// ---------------------------------------------------------------------
class SenderCategory {
public:
    // Higher number = higher priority = read first.
    static int priorityOf(const string& category) {
        if (category == "Boss") return 5;
        if (category == "Subordinate") return 4;
        if (category == "Peer") return 3;
        if (category == "ImportantPerson") return 2;
        if (category == "OtherPerson") return 1;
        throw invalid_argument("unknown sender category '" + category + "'");
    }
};

// ---------------------------------------------------------------------
// Email - one inbox item; knows how to compare its priority to another
// ---------------------------------------------------------------------
class Email {
public:
    Email() : priority(0) {}

    Email(const string& senderCategoryIn, const string& subjectIn, const Date& dateIn)
        : senderCategory(senderCategoryIn),
          subject(subjectIn),
          date(dateIn),
          priority(SenderCategory::priorityOf(senderCategoryIn)) {}

    const string& getSenderCategory() const { return senderCategory; }
    const string& getSubject() const { return subject; }
    const Date& getDate() const { return date; }
    int getPriority() const { return priority; }

    // Max-heap ordering: higher sender-category priority wins; ties are
    // broken by the newest date (per the CEO's Sprint-era trick).
    bool hasHigherPriorityThan(const Email& other) const {
        if (priority != other.priority) return priority > other.priority;
        return date.isNewerThan(other.date);
    }

private:
    string senderCategory;
    string subject;
    Date date;
    int priority;
};

// ---------------------------------------------------------------------
// MaxHeap - a hand-written, list/array-based (std::vector-backed) binary
// max-heap of Email objects. No pre-existing heap containers/algorithms
// (std::priority_queue, std::make_heap, etc.) are used anywhere here.
// ---------------------------------------------------------------------
class MaxHeap {
public:
    MaxHeap() {}

    bool isEmpty() const { return items.empty(); }
    size_t size() const { return items.size(); }

    void insert(const Email& email) {
        items.push_back(email);
        siftUp(items.size() - 1);
    }

    // Returns the highest priority email without removing it.
    const Email& peekMax() const {
        if (isEmpty()) {
            throw runtime_error("peekMax() called on an empty heap");
        }
        return items[0];
    }

    // Removes and returns the highest priority email.
    Email extractMax() {
        if (isEmpty()) {
            throw runtime_error("extractMax() called on an empty heap");
        }
        Email maxEmail = items[0];
        items[0] = items.back();
        items.pop_back();
        if (!items.empty()) {
            siftDown(0);
        }
        return maxEmail;
    }

private:
    vector<Email> items; // the "list" backing this list-based heap

    static size_t parentOf(size_t i) { return (i - 1) / 2; }
    static size_t leftChildOf(size_t i) { return 2 * i + 1; }
    static size_t rightChildOf(size_t i) { return 2 * i + 2; }

    void siftUp(size_t index) {
        while (index > 0) {
            size_t parent = parentOf(index);
            if (items[index].hasHigherPriorityThan(items[parent])) {
                swap(items[index], items[parent]);
                index = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t index) {
        size_t n = items.size();
        while (true) {
            size_t left = leftChildOf(index);
            size_t right = rightChildOf(index);
            size_t largest = index;

            if (left < n && items[left].hasHigherPriorityThan(items[largest])) {
                largest = left;
            }
            if (right < n && items[right].hasHigherPriorityThan(items[largest])) {
                largest = right;
            }
            if (largest == index) break;

            swap(items[index], items[largest]);
            index = largest;
        }
    }
};

// ---------------------------------------------------------------------
// EmailInbox - owns the MaxHeap and implements CEO-facing behavior:
// adding mail, peeking at the next email, marking one as read, and
// reporting the unread count. This is where command semantics (and the
// edge cases the spec calls out) live.
// ---------------------------------------------------------------------
class EmailInbox {
public:
    void addEmail(const Email& email) { heap.insert(email); }

    // NEXT: non-destructive peek. Calling this twice in a row with no
    // intervening READ always shows the same email, because nothing is
    // removed here.
    void showNext() const {
        if (heap.isEmpty()) {
            cout << "There are no emails to read." << endl;
            cout << endl;
            return;
        }
        const Email& top = heap.peekMax();
        cout << "Next email:" << endl;
        cout << "\tSender: " << top.getSenderCategory() << endl;
        cout << "\tSubject: " << top.getSubject() << endl;
        cout << "\tDate: " << top.getDate().toString() << endl;
        cout << endl;
    }

    // READ: the CEO dealt with the top email. It is removed with no
    // display. Calling READ twice in a row (no intervening NEXT) removes
    // two different emails, neither one ever shown - exactly as specified.
    // If the inbox is empty, this is a safe no-op.
    void markTopAsRead() {
        if (heap.isEmpty()) {
            return;
        }
        heap.extractMax();
    }

    void showCount() const {
        cout << "There are " << heap.size() << " emails to read." << endl;
        cout << endl;
    }

private:
    MaxHeap heap;
};

// ---------------------------------------------------------------------
// EmailPriorityApp - reads command lines from an input stream and
// dispatches each one to the EmailInbox. Handles blank lines and
// malformed input defensively instead of crashing.
// ---------------------------------------------------------------------
class EmailPriorityApp {
public:
    void run(istream& input) {
        string rawLine;
        while (getline(input, rawLine)) {
            handleLine(rawLine);
        }
    }

private:
    EmailInbox inbox;

    void handleLine(const string& rawLine) {
        string line = StringUtils::trim(rawLine);
        if (line.empty()) return; // ignore blank lines

        size_t spacePos = line.find(' ');
        string command = (spacePos == string::npos) ? line : line.substr(0, spacePos);

        if (command == "EMAIL") {
            handleEmailCommand(line, rawLine);
        } else if (command == "NEXT") {
            inbox.showNext();
        } else if (command == "READ") {
            inbox.markTopAsRead();
        } else if (command == "COUNT") {
            inbox.showCount();
        } else {
            cerr << "Warning: ignoring unrecognized command: " << rawLine << endl;
        }
    }

    void handleEmailCommand(const string& line, const string& rawLine) {
        // line looks like: EMAIL <senderCategory>,<subject>,<date>
        string rest = StringUtils::trim(line.substr(5));

        size_t firstComma = rest.find(',');
        size_t lastComma = rest.rfind(',');

        if (firstComma == string::npos || lastComma == string::npos ||
            firstComma == lastComma) {
            cerr << "Warning: malformed EMAIL command, skipping: " << rawLine << endl;
            return;
        }

        string senderCategory = StringUtils::trim(rest.substr(0, firstComma));
        string subject = StringUtils::trim(
            rest.substr(firstComma + 1, lastComma - firstComma - 1));
        string dateText = StringUtils::trim(rest.substr(lastComma + 1));

        try {
            Date date = Date::fromString(dateText);
            Email email(senderCategory, subject, date);
            inbox.addEmail(email);
        } catch (const exception& e) {
            cerr << "Warning: could not add email (" << e.what() << "): "
                 << rawLine << endl;
        }
    }
};

// ---------------------------------------------------------------------
// main - reads from a file named on the command line if given, otherwise
// from standard input.
// ---------------------------------------------------------------------
int main(int argc, char* argv[]) {
    EmailPriorityApp app;

    if (argc > 1) {
        ifstream file(argv[1]);
        if (!file.is_open()) {
            cerr << "Error: could not open file '" << argv[1] << "'" << endl;
            return 1;
        }
        app.run(file);
    } else {
        app.run(cin);
    }

    return 0;
}