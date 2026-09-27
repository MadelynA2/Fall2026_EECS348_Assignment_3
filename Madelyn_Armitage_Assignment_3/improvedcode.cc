// email_priority_queue.cpp                                                     // Names the source file.
// CEO Email Priority Queue                                                     // Gives the program a descriptive title.
// This program prioritizes CEO emails using a hand-written MaxHeap.            // Explains the purpose of the program.
// Emails are ranked by sender category, date, and then arrival order.          // Explains how email priority is determined.
// NEXT displays, READ removes, and COUNT reports unread emails.                // Explains the available commands.

#include <iostream>                                                             // Provides input and output tools.
#include <fstream>                                                              // Provides file input tools.
#include <string>                                                               // Provides the string class.
#include <vector>                                                               // Provides the vector used to store the heap.
#include <stdexcept>                                                            // Provides exception classes.
#include <cctype>                                                               // Provides isspace for whitespace checking.

using namespace std;                                                            // Allows standard library names without std::.


// -----------------------------------------------------------------------------
// StringUtils Class
// Provides helper methods for working with strings.
// In this program, it removes extra whitespace from command input.
// -----------------------------------------------------------------------------
class StringUtils {                                                             // Begins the StringUtils class.

public:                                                                         // Begins the public section.

    // trim removes whitespace from the beginning and end of a string.
    static string trim(const string& inputText) {                               // Begins the trim method.

        size_t startIndex = 0;                                                  // Stores the first non-whitespace position.

        while (startIndex < inputText.size() &&                                 // Continues while characters remain.
               isspace(static_cast<unsigned char>(inputText[startIndex]))) {     // Checks whether the current character is whitespace.

            startIndex++;                                                       // Moves forward past the whitespace.
        }                                                                       // Ends the leading-whitespace loop.

        size_t endIndex = inputText.size();                                     // Starts at the end of the string.

        while (endIndex > startIndex &&                                         // Continues while characters remain.
               isspace(static_cast<unsigned char>(inputText[endIndex - 1]))) {   // Checks the last remaining character.

            endIndex--;                                                         // Moves backward past the whitespace.
        }                                                                       // Ends the trailing-whitespace loop.

        return inputText.substr(startIndex, endIndex - startIndex);              // Returns the trimmed string.
    }                                                                           // Ends the trim method.

};                                                                              // Ends the StringUtils class.


// -----------------------------------------------------------------------------
// Date Class
// Stores a date from an email.
// Parses the date, checks whether the month and day are valid, and compares
// two dates to determine which email is newer.
// -----------------------------------------------------------------------------
class Date {                                                                    // Begins the Date class.

public:                                                                         // Begins the public section.

    // Default constructor creates an empty Date object.
    Date() : month(0), day(0), year(0), originalText("") {}                     // Initializes all date fields.


    // fromString converts a date string into a Date object.
    static Date fromString(const string& inputText) {                           // Begins the fromString method.

        size_t firstDashIndex = inputText.find('-');                            // Finds the first dash.

        size_t secondDashIndex =                                                // Creates the second dash position.
            (firstDashIndex == string::npos)                                    // Checks whether the first dash exists.
            ? string::npos                                                      // Uses npos if there was no first dash.
            : inputText.find('-', firstDashIndex + 1);                          // Finds the second dash.

        if (firstDashIndex == string::npos ||                                   // Checks whether the first dash is missing.
            secondDashIndex == string::npos) {                                  // Checks whether the second dash is missing.

            throw invalid_argument(                                             // Throws an error for a malformed date.
                "date '" + inputText + "' is not in a recognizable date format");

        }                                                                       // Ends the dash validation.


        Date parsedDate;                                                        // Creates a Date object for the parsed date.

        parsedDate.originalText = inputText;                                    // Stores the original date text.


        try {                                                                   // Begins numeric date conversion.

            parsedDate.month = stoi(                                            // Converts the month to an integer.
                inputText.substr(0, firstDashIndex));                           // Gets the month portion.

            parsedDate.day = stoi(                                              // Converts the day to an integer.
                inputText.substr(                                               // Gets the day portion.
                    firstDashIndex + 1,                                         // Starts after the first dash.
                    secondDashIndex - firstDashIndex - 1));                     // Ends before the second dash.

            parsedDate.year = stoi(                                             // Converts the year to an integer.
                inputText.substr(secondDashIndex + 1));                         // Gets the year portion.

        } catch (const exception&) {                                            // Catches failed numeric conversions.

            throw invalid_argument(                                             // Throws an error for non-numeric date data.
                "date '" + inputText + "' contains non-numeric fields");

        }                                                                       // Ends numeric conversion error handling.


        if (parsedDate.month < 1 || parsedDate.month > 12) {                    // Checks for a valid month.

            throw invalid_argument(                                             // Throws an error for an invalid month.
                "date '" + inputText + "' contains an invalid month");

        }                                                                       // Ends month validation.


        int maximumDay = 31;                                                    // Assumes a month has 31 days initially.


        if (parsedDate.month == 4 ||                                            // Checks April.
            parsedDate.month == 6 ||                                            // Checks June.
            parsedDate.month == 9 ||                                            // Checks September.
            parsedDate.month == 11) {                                           // Checks November.

            maximumDay = 30;                                                    // Sets the maximum to 30 days.

        } else if (parsedDate.month == 2) {                                     // Checks February.

            bool leapYear =                                                     // Stores whether the year is a leap year.
                (parsedDate.year % 400 == 0) ||                                 // Years divisible by 400 are leap years.
                (parsedDate.year % 4 == 0 &&                                    // Checks divisibility by four.
                 parsedDate.year % 100 != 0);                                   // Excludes most century years.

            maximumDay = leapYear ? 29 : 28;                                   // Sets February's maximum number of days.

        }                                                                       // Ends month-specific day calculation.


        if (parsedDate.day < 1 || parsedDate.day > maximumDay) {                // Checks whether the day is valid.

            throw invalid_argument(                                             // Throws an error for an invalid day.
                "date '" + inputText + "' contains an invalid day");

        }                                                                       // Ends day validation.


        return parsedDate;                                                      // Returns the valid parsed date.

    }                                                                           // Ends the fromString method.


    // isNewerThan returns true when this date occurs after another date.
    bool isNewerThan(const Date& otherDate) const {                             // Begins date comparison.

        if (year != otherDate.year) {                                           // Checks whether the years differ.

            return year > otherDate.year;                                       // Gives priority to the newer year.

        }                                                                       // Ends year comparison.

        if (month != otherDate.month) {                                         // Checks whether the months differ.

            return month > otherDate.month;                                     // Gives priority to the newer month.

        }                                                                       // Ends month comparison.

        return day > otherDate.day;                                             // Gives priority to the newer day.

    }                                                                           // Ends isNewerThan.


    // isSameDate determines whether two emails have exactly the same date.
    bool isSameDate(const Date& otherDate) const {                              // Begins date equality comparison.

        return year == otherDate.year &&                                        // Checks that the years match.
               month == otherDate.month &&                                      // Checks that the months match.
               day == otherDate.day;                                            // Checks that the days match.

    }                                                                           // Ends isSameDate.


    // toString returns the original date text for displaying the email.
    string toString() const {                                                   // Begins toString.

        return originalText;                                                    // Returns the original date text.

    }                                                                           // Ends toString.


private:                                                                        // Begins the private section.

    int month;                                                                  // Stores the numeric month.
    int day;                                                                    // Stores the numeric day.
    int year;                                                                   // Stores the numeric year.
    string originalText;                                                        // Stores the original date text.

};                                                                              // Ends the Date class.


// -----------------------------------------------------------------------------
// SenderCategory Class
// Converts each allowed sender category into its numeric priority.
// A larger priority number means that the email should be read sooner.
// -----------------------------------------------------------------------------
class SenderCategory {                                                          // Begins the SenderCategory class.

public:                                                                         // Begins the public section.

    // priorityOf returns the priority number for a sender category.
    static int priorityOf(const string& senderCategory) {                       // Begins priorityOf.

        if (senderCategory == "Boss") {                                         // Checks for Boss.

            return 5;                                                           // Boss has the highest priority.

        }                                                                       // Ends Boss condition.

        if (senderCategory == "Subordinate") {                                  // Checks for Subordinate.

            return 4;                                                           // Subordinate has the second-highest priority.

        }                                                                       // Ends Subordinate condition.

        if (senderCategory == "Peer") {                                         // Checks for Peer.

            return 3;                                                           // Peer has the middle priority.

        }                                                                       // Ends Peer condition.

        if (senderCategory == "ImportantPerson") {                              // Checks for ImportantPerson.

            return 2;                                                           // ImportantPerson has priority two.

        }                                                                       // Ends ImportantPerson condition.

        if (senderCategory == "OtherPerson") {                                  // Checks for OtherPerson.

            return 1;                                                           // OtherPerson has the lowest valid priority.

        }                                                                       // Ends OtherPerson condition.


        throw invalid_argument(                                                 // Throws an error for an invalid sender category.
            "unknown sender category '" + senderCategory + "'");

    }                                                                           // Ends priorityOf.

};                                                                              // Ends the SenderCategory class.


// -----------------------------------------------------------------------------
// Email Class
// Represents one email in the CEO's inbox.
// Stores the sender category, subject, date, sender priority, and arrival order.
// It also determines whether one email has higher priority than another.
// -----------------------------------------------------------------------------
class Email {                                                                   // Begins the Email class.

public:                                                                         // Begins the public section.

    // Default constructor creates an empty Email.
    Email() : priority(0), arrivalOrder(0) {}                                   // Initializes numeric fields to zero.


    // Constructor creates an Email using all necessary information.
    Email(const string& senderCategoryInput,                                    // Receives the sender category.
          const string& subjectInput,                                           // Receives the subject.
          const Date& dateInput,                                                // Receives the parsed date.
          size_t arrivalOrderInput)                                             // Receives the arrival-order value.
        : senderCategory(senderCategoryInput),                                  // Stores the sender category.
          subject(subjectInput),                                                // Stores the subject.
          date(dateInput),                                                      // Stores the date.
          priority(SenderCategory::priorityOf(senderCategoryInput)),            // Stores the sender priority.
          arrivalOrder(arrivalOrderInput) {}                                    // Stores the arrival order.


    // getSenderCategory returns the sender category.
    const string& getSenderCategory() const {                                   // Begins getSenderCategory.

        return senderCategory;                                                  // Returns the sender category.

    }                                                                           // Ends getSenderCategory.


    // getSubject returns the email subject.
    const string& getSubject() const {                                          // Begins getSubject.

        return subject;                                                         // Returns the subject.

    }                                                                           // Ends getSubject.


    // getDate returns the email's Date object.
    const Date& getDate() const {                                               // Begins getDate.

        return date;                                                            // Returns the stored date.

    }                                                                           // Ends getDate.


    // getPriority returns the sender's numeric priority.
    int getPriority() const {                                                   // Begins getPriority.

        return priority;                                                        // Returns the stored priority.

    }                                                                           // Ends getPriority.


    // getArrivalOrder returns when the email was added relative to other emails.
    size_t getArrivalOrder() const {                                            // Begins getArrivalOrder.

        return arrivalOrder;                                                    // Returns the arrival order.

    }                                                                           // Ends getArrivalOrder.


    // hasHigherPriorityThan determines which of two emails belongs first.
    bool hasHigherPriorityThan(const Email& otherEmail) const {                 // Begins email priority comparison.

        if (priority != otherEmail.priority) {                                  // Checks whether sender priorities differ.

            return priority > otherEmail.priority;                              // Higher sender priority comes first.

        }                                                                       // Ends sender-priority comparison.


        if (!date.isSameDate(otherEmail.date)) {                                // Checks whether the dates differ.

            return date.isNewerThan(otherEmail.date);                           // Newer email comes first.

        }                                                                       // Ends date comparison.


        return arrivalOrder < otherEmail.arrivalOrder;                          // Earlier arrival wins a complete tie.

    }                                                                           // Ends hasHigherPriorityThan.


private:                                                                        // Begins the private section.

    string senderCategory;                                                      // Stores the sender category.
    string subject;                                                             // Stores the subject.
    Date date;                                                                  // Stores the email date.
    int priority;                                                               // Stores the sender priority.
    size_t arrivalOrder;                                                        // Stores the email arrival order.

};                                                                              // Ends the Email class.


// -----------------------------------------------------------------------------
// MaxHeap Class
// Implements the priority queue using a list-based binary MaxHeap.
// The heap is stored in a vector, but all heap operations are written manually.
// No pre-existing heap or priority-queue module is used.
// -----------------------------------------------------------------------------
class MaxHeap {                                                                 // Begins the MaxHeap class.

public:                                                                         // Begins the public section.

    // Default constructor creates an empty MaxHeap.
    MaxHeap() {}                                                                // Creates an empty vector automatically.


    // isEmpty returns true when there are no emails in the heap.
    bool isEmpty() const {                                                      // Begins isEmpty.

        return emails.empty();                                                  // Returns whether the vector is empty.

    }                                                                           // Ends isEmpty.


    // size returns the number of emails currently stored.
    size_t size() const {                                                       // Begins size.

        return emails.size();                                                   // Returns the current heap size.

    }                                                                           // Ends size.


    // insert adds a new email and restores MaxHeap order.
    void insert(const Email& email) {                                           // Begins insert.

        emails.push_back(email);                                                // Adds the email to the end of the vector.

        siftUp(emails.size() - 1);                                              // Moves the email upward if necessary.

    }                                                                           // Ends insert.


    // peekMax returns the highest-priority email without removing it.
    const Email& peekMax() const {                                              // Begins peekMax.

        if (isEmpty()) {                                                        // Checks whether the heap is empty.

            throw runtime_error(                                                // Throws an error if the root does not exist.
                "peekMax() called on an empty heap");

        }                                                                       // Ends empty-heap check.

        return emails[0];                                                       // Returns the highest-priority email.

    }                                                                           // Ends peekMax.


    // removeMax removes the highest-priority email.
    // It does not create or return an unused copy of the removed email.
    void removeMax() {                                                          // Begins removeMax.

        if (isEmpty()) {                                                        // Checks whether the heap is empty.

            throw runtime_error(                                                // Prevents invalid removal.
                "removeMax() called on an empty heap");

        }                                                                       // Ends empty-heap check.


        if (emails.size() == 1) {                                               // Checks whether only one email remains.

            emails.pop_back();                                                  // Removes the only email.

            return;                                                             // Stops because no heap repair is needed.

        }                                                                       // Ends single-email condition.


        emails[0] = emails.back();                                              // Moves the final email to the root.

        emails.pop_back();                                                      // Removes the old final vector position.

        siftDown(0);                                                            // Restores MaxHeap order.

    }                                                                           // Ends removeMax.


private:                                                                        // Begins the private section.

    vector<Email> emails;                                                       // Stores the emails that form the MaxHeap.


    // parentOf calculates the parent index of a heap element.
    static size_t parentOf(size_t index) {                                      // Begins parentOf.

        return (index - 1) / 2;                                                 // Calculates the parent index.

    }                                                                           // Ends parentOf.


    // leftChildOf calculates the left-child index.
    static size_t leftChildOf(size_t index) {                                   // Begins leftChildOf.

        return 2 * index + 1;                                                   // Calculates the left-child index.

    }                                                                           // Ends leftChildOf.


    // rightChildOf calculates the right-child index.
    static size_t rightChildOf(size_t index) {                                  // Begins rightChildOf.

        return 2 * index + 2;                                                   // Calculates the right-child index.

    }                                                                           // Ends rightChildOf.


    // siftUp restores heap order after a new email is inserted.
    void siftUp(size_t currentIndex) {                                          // Begins siftUp.

        while (currentIndex > 0) {                                              // Continues until the root is reached.

            size_t parentIndex = parentOf(currentIndex);                         // Finds the parent index.


            if (emails[currentIndex].hasHigherPriorityThan(                     // Compares the current email to its parent.
                    emails[parentIndex])) {                                     // Finishes the comparison.

                swap(emails[currentIndex], emails[parentIndex]);                // Swaps them when the child has higher priority.

                currentIndex = parentIndex;                                     // Continues from the new position.

            } else {                                                            // Runs when heap order is correct.

                break;                                                          // Stops the loop.

            }                                                                   // Ends the comparison condition.

        }                                                                       // Ends the sift-up loop.

    }                                                                           // Ends siftUp.


    // siftDown restores heap order after the root email is removed.
    void siftDown(size_t currentIndex) {                                        // Begins siftDown.

        size_t heapSize = emails.size();                                        // Stores the heap size.


        while (true) {                                                          // Continues until no more swaps are needed.

            size_t leftChildIndex =                                             // Stores the left-child index.
                leftChildOf(currentIndex);                                      // Calculates the left child.

            size_t rightChildIndex =                                            // Stores the right-child index.
                rightChildOf(currentIndex);                                     // Calculates the right child.

            size_t highestPriorityIndex = currentIndex;                         // Assumes the current position is highest.


            if (leftChildIndex < heapSize &&                                    // Checks whether the left child exists.
                emails[leftChildIndex].hasHigherPriorityThan(                   // Compares the left child.
                    emails[highestPriorityIndex])) {                            // Completes the comparison.

                highestPriorityIndex = leftChildIndex;                          // Records the left child as highest priority.

            }                                                                   // Ends left-child comparison.


            if (rightChildIndex < heapSize &&                                   // Checks whether the right child exists.
                emails[rightChildIndex].hasHigherPriorityThan(                  // Compares the right child.
                    emails[highestPriorityIndex])) {                            // Completes the comparison.

                highestPriorityIndex = rightChildIndex;                         // Records the right child as highest priority.

            }                                                                   // Ends right-child comparison.


            if (highestPriorityIndex == currentIndex) {                         // Checks whether no swap is necessary.

                break;                                                          // Stops when heap order is correct.

            }                                                                   // Ends no-swap condition.


            swap(emails[currentIndex],                                          // Begins swapping the two emails.
                 emails[highestPriorityIndex]);                                 // Completes the swap.

            currentIndex = highestPriorityIndex;                                // Continues from the new location.

        }                                                                       // Ends sift-down loop.

    }                                                                           // Ends siftDown.

};                                                                              // Ends the MaxHeap class.


// -----------------------------------------------------------------------------
// EmailInbox Class
// Owns the MaxHeap containing unread CEO emails.
// Implements the behavior required by the NEXT, READ, and COUNT commands.
// -----------------------------------------------------------------------------
class EmailInbox {                                                              // Begins the EmailInbox class.

public:                                                                         // Begins the public section.

    // addEmail inserts a new email into the priority queue.
    void addEmail(const Email& email) {                                         // Begins addEmail.

        emailHeap.insert(email);                                                // Inserts the email into the MaxHeap.

    }                                                                           // Ends addEmail.


    // showNext displays the highest-priority email without removing it.
    void showNext() const {                                                     // Begins showNext.

        if (emailHeap.isEmpty()) {                                              // Checks whether no emails remain.

            cout << "No emails to read." << endl;                              // Prints the required empty-inbox message.

            cout << endl;                                                       // Prints a blank line after the message.

            return;                                                             // Stops because no email can be displayed.

        }                                                                       // Ends empty-inbox condition.


        const Email& nextEmail = emailHeap.peekMax();                           // Gets the highest-priority email.

        cout << "Next email:" << endl;                                          // Prints the heading.

        cout << "\tSender: "                                                    // Begins the sender output.
             << nextEmail.getSenderCategory()                                   // Prints the sender category.
             << endl;                                                           // Ends the sender line.

        cout << "\tSubject: "                                                   // Begins the subject output.
             << nextEmail.getSubject()                                          // Prints the subject.
             << endl;                                                           // Ends the subject line.

        cout << "\tDate: "                                                      // Begins the date output.
             << nextEmail.getDate().toString()                                  // Prints the original date text.
             << endl;                                                           // Ends the date line.

        cout << endl;                                                           // Prints a blank line after the email.

    }                                                                           // Ends showNext.


    // markTopAsRead removes the highest-priority email without displaying it.
    void markTopAsRead() {                                                      // Begins markTopAsRead.

        if (emailHeap.isEmpty()) {                                              // Checks whether the inbox is empty.

            return;                                                             // READ on an empty inbox safely does nothing.

        }                                                                       // Ends empty-inbox condition.


        emailHeap.removeMax();                                                  // Removes the highest-priority email.

    }                                                                           // Ends markTopAsRead.


    // showCount displays the number of unread emails.
    void showCount() const {                                                    // Begins showCount.

        cout << "There are "                                                    // Begins the count message.
             << emailHeap.size()                                                // Prints the number of remaining emails.
             << " emails to read."                                              // Prints the rest of the message.
             << endl;                                                           // Ends the line.

        cout << endl;                                                           // Prints a blank line after the count.

    }                                                                           // Ends showCount.


private:                                                                        // Begins the private section.

    MaxHeap emailHeap;                                                          // Stores unread emails in priority order.

};                                                                              // Ends the EmailInbox class.


// -----------------------------------------------------------------------------
// EmailPriorityApp Class
// Controls the overall program.
// Reads command lines, determines which command was entered, parses EMAIL
// information, tracks arrival order, and sends operations to EmailInbox.
// -----------------------------------------------------------------------------
class EmailPriorityApp {                                                        // Begins the EmailPriorityApp class.

public:                                                                         // Begins the public section.

    // Constructor begins the arrival counter at zero.
    EmailPriorityApp() : arrivalCounter(0) {}                                   // Initializes the arrival counter.


    // run reads and processes every line from the chosen input stream.
    void run(istream& inputStream) {                                            // Begins run.

        string rawLine;                                                         // Stores each line read from input.


        while (getline(inputStream, rawLine)) {                                 // Continues while input lines remain.

            handleLine(rawLine);                                                // Processes the current command.

        }                                                                       // Ends input loop.

    }                                                                           // Ends run.


private:                                                                        // Begins the private section.

    EmailInbox inbox;                                                           // Stores the CEO's unread inbox.

    size_t arrivalCounter;                                                      // Tracks the order valid emails arrive.


    // handleLine determines which command is present on a line.
    void handleLine(const string& rawLine) {                                    // Begins handleLine.

        string trimmedLine =                                                    // Creates the cleaned line.
            StringUtils::trim(rawLine);                                         // Removes extra whitespace.


        if (trimmedLine.empty()) {                                              // Checks whether the line is blank.

            return;                                                             // Ignores blank lines.

        }                                                                       // Ends blank-line condition.


        size_t commandEndIndex =                                                // Stores where the command name ends.
            trimmedLine.find(' ');                                              // Finds the first space.


        string command =                                                        // Creates the command string.
            (commandEndIndex == string::npos)                                   // Checks whether there is no argument.
            ? trimmedLine                                                       // Uses the full line for simple commands.
            : trimmedLine.substr(0, commandEndIndex);                           // Extracts the command word.


        if (command == "EMAIL") {                                               // Checks for an EMAIL command.

            handleEmailCommand(trimmedLine, rawLine);                           // Parses and adds the new email.

        } else if (command == "NEXT") {                                         // Checks for NEXT.

            inbox.showNext();                                                   // Displays the highest-priority email.

        } else if (command == "READ") {                                         // Checks for READ.

            inbox.markTopAsRead();                                              // Removes the highest-priority email.

        } else if (command == "COUNT") {                                        // Checks for COUNT.

            inbox.showCount();                                                  // Displays the number of unread emails.

        } else {                                                                // Handles an unknown command.

            cerr << "Warning: ignoring unrecognized command: "                 // Begins the warning message.
                 << rawLine                                                     // Displays the unknown command.
                 << endl;                                                       // Ends the warning line.

        }                                                                       // Ends command selection.

    }                                                                           // Ends handleLine.


    // handleEmailCommand separates the sender, subject, and date fields.
    // It creates the Email object and assigns its arrival-order value.
    void handleEmailCommand(const string& trimmedLine,                          // Receives the cleaned line.
                            const string& rawLine) {                             // Receives the original line.

        string emailData =                                                      // Stores the EMAIL command data.
            StringUtils::trim(trimmedLine.substr(5));                           // Removes the word EMAIL.


        size_t firstCommaIndex =                                                // Stores the first comma location.
            emailData.find(',');                                                // Finds the sender/subject separator.

        size_t lastCommaIndex =                                                 // Stores the final comma location.
            emailData.rfind(',');                                               // Finds the subject/date separator.


        if (firstCommaIndex == string::npos ||                                  // Checks whether the first comma is missing.
            lastCommaIndex == string::npos ||                                   // Checks whether the final comma is missing.
            firstCommaIndex == lastCommaIndex) {                                // Checks whether there is only one comma.

            cerr << "Warning: malformed EMAIL command, skipping: "             // Begins malformed-command warning.
                 << rawLine                                                     // Prints the malformed input.
                 << endl;                                                       // Ends the warning.

            return;                                                             // Skips the invalid command.

        }                                                                       // Ends comma validation.


        string senderCategory =                                                 // Creates the sender category string.
            StringUtils::trim(                                                  // Removes extra whitespace.
                emailData.substr(0, firstCommaIndex));                          // Extracts the sender category.


        string subject =                                                        // Creates the subject string.
            StringUtils::trim(                                                  // Removes extra whitespace.
                emailData.substr(                                               // Extracts the subject.
                    firstCommaIndex + 1,                                        // Starts after the first comma.
                    lastCommaIndex - firstCommaIndex - 1));                     // Ends before the final comma.


        string dateText =                                                       // Creates the date string.
            StringUtils::trim(                                                  // Removes extra whitespace.
                emailData.substr(lastCommaIndex + 1));                          // Extracts the date.


        try {                                                                   // Begins protected input processing.

            Date emailDate =                                                    // Creates the Date object.
                Date::fromString(dateText);                                     // Parses and validates the date.


            Email newEmail(                                                     // Creates the Email object.
                senderCategory,                                                 // Supplies the sender category.
                subject,                                                        // Supplies the subject.
                emailDate,                                                      // Supplies the parsed date.
                arrivalCounter);                                                // Supplies the arrival order.


            inbox.addEmail(newEmail);                                           // Adds the valid email to the inbox.

            arrivalCounter++;                                                   // Advances arrival order after successful insertion.

        } catch (const exception& error) {                                      // Handles invalid email information.

            cerr << "Warning: could not add email ("                           // Begins the error message.
                 << error.what()                                                // Displays the reason for the error.
                 << "): "                                                       // Separates the message from the input.
                 << rawLine                                                     // Displays the original EMAIL line.
                 << endl;                                                       // Ends the warning.

        }                                                                       // Ends exception handling.

    }                                                                           // Ends handleEmailCommand.

};                                                                              // Ends the EmailPriorityApp class.


// -----------------------------------------------------------------------------
// main Function
// Entry point of the program.
// Creates the application and processes either a named test file or standard
// input when no filename is supplied.
// -----------------------------------------------------------------------------
int main(int argc, char* argv[]) {                                              // Begins the main function.

    EmailPriorityApp emailApp;                                                  // Creates the application object.


    if (argc > 1) {                                                             // Checks whether a filename was provided.

        ifstream inputFile(argv[1]);                                            // Opens the provided file.


        if (!inputFile.is_open()) {                                             // Checks whether the file opened.

            cerr << "Error: could not open file '"                             // Begins the file error message.
                 << argv[1]                                                     // Prints the requested filename.
                 << "'"                                                         // Closes the filename quote.
                 << endl;                                                       // Ends the error message.

            return 1;                                                           // Ends the program with an error status.

        }                                                                       // Ends file-open validation.


        emailApp.run(inputFile);                                                // Processes commands from the file.

    } else {                                                                    // Runs when no filename was provided.

        emailApp.run(cin);                                                      // Processes commands from standard input.

    }                                                                           // Ends input-source selection.


    return 0;                                                                   // Ends the program successfully.

}                                                                               // Ends main.