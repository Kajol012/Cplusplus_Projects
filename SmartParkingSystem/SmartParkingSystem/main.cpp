#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include <fstream>
#include <sstream>
#include <ctime>
#include <iomanip>
#include <limits>
using namespace std;

const int TOTAL_SLOTS = 10;
const double CAR_RATE = 50.0;
const double BIKE_RATE = 30.0;

struct Vehicle {
    string number, type;
    time_t entryTime;
};

struct History {
    string number, type;
    time_t entryTime, exitTime;
    double fee;
};

Vehicle slots[TOTAL_SLOTS];
queue<Vehicle> waiting;
vector<History> history;
double totalIncome = 0;

string timeText(time_t value) {
    tm *info = localtime(&value);

    if (!info)
        return "Unknown";

    char buffer[30];

    strftime(buffer, sizeof(buffer),
             "%Y-%m-%d %H:%M:%S", info);

    return buffer;
}

string upperText(string text) {
    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] >= 'a' && text[i] <= 'z')
            text[i] = text[i] - ('a' - 'A');
    }

    return text;
}

int findFreeSlot() {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].number.empty())
            return i;
    }

    return -1;
}

int findVehicle(string number) {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (slots[i].number == number)
            return i;
    }

    return -1;
}

bool isWaiting(string number) {
    queue<Vehicle> temp = waiting;

    while (!temp.empty()) {
        if (temp.front().number == number)
            return true;

        temp.pop();
    }

    return false;
}

void saveData() {
    ofstream out("parking_data.txt");

    if (!out) {
        cout << "Error: Could not save data.\n";
        return;
    }

    out << fixed << setprecision(2);
    out << totalIncome << '\n';
    out << "#SLOTS\n";

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (!slots[i].number.empty()) {
            out << i << ' '
                << slots[i].number << ' '
                << slots[i].type << ' '
                << (long long)slots[i].entryTime << '\n';
        }
    }

    out << "#WAITING\n";

    queue<Vehicle> temp = waiting;

    while (!temp.empty()) {
        Vehicle v = temp.front();
        temp.pop();

        out << v.number << ' '
            << v.type << ' '
            << (long long)v.entryTime << '\n';
    }

    out << "#HISTORY\n";

    for (int i = 0; i < (int)history.size(); i++) {
        out << history[i].number << ' '
            << history[i].type << ' '
            << (long long)history[i].entryTime << ' '
            << (long long)history[i].exitTime << ' '
            << history[i].fee << '\n';
    }

    out.close();

    cout << "Data saved successfully.\n";
}

void loadData() {
    ifstream in("parking_data.txt");

    if (!in) {
        cout << "No previous data found. Starting new records.\n";
        return;
    }

    if (!(in >> totalIncome)) {
        cout << "Saved data is empty or damaged.\n";
        totalIncome = 0;
        return;
    }

    string line;


    while (getline(in, line)) {
        if (line == "#SLOTS")
            break;
    }

    if (line != "#SLOTS") {
        cout << "Invalid data file format.\n";
        return;
    }


    while (getline(in, line)) {
        if (line == "#WAITING")
            break;

        if (line.empty())
            continue;

        stringstream row(line);

        int slot;
        string number, type;
        long long entry;

        if (row >> slot >> number >> type >> entry) {
            if (slot >= 0 && slot < TOTAL_SLOTS) {
                slots[slot].number = number;
                slots[slot].type = type;
                slots[slot].entryTime = (time_t)entry;
            }
        }
    }


    while (getline(in, line)) {
        if (line == "#HISTORY")
            break;

        if (line.empty())
            continue;

        stringstream row(line);

        Vehicle v;
        long long entry;

        if (row >> v.number >> v.type >> entry) {
            v.entryTime = (time_t)entry;
            waiting.push(v);
        }
    }

    while (getline(in, line)) {
        if (line.empty())
            continue;

        stringstream row(line);

        History h;
        long long entry, exitTime;

        if (row >> h.number >> h.type
                >> entry >> exitTime >> h.fee) {
            h.entryTime = (time_t)entry;
            h.exitTime = (time_t)exitTime;
            history.push_back(h);
        }
    }

    cout << "Saved data loaded successfully.\n";
}
void showSlots() {
    cout << "\n--- PARKING SLOTS ---\n";

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        cout << "Slot " << i + 1 << ": ";

        if (slots[i].number.empty()) {
            cout << "AVAILABLE\n";
        } else {
            cout << "OCCUPIED | "
                 << slots[i].number << " | "
                 << slots[i].type
                 << " | Entry: "
                 << timeText(slots[i].entryTime) << '\n';
        }
    }
}

void vehicleEntry() {
    Vehicle v;

    cout << "Enter vehicle number (no spaces): ";
    cin >> v.number;

    if (findVehicle(v.number) != -1 ||
        isWaiting(v.number)) {
        cout << "This vehicle is already parked or waiting.\n";
        return;
    }

    cout << "Enter type (Car/Bike): ";
    cin >> v.type;

    v.type = upperText(v.type);

    if (v.type != "CAR" && v.type != "BIKE") {
        cout << "Invalid type. Enter Car or Bike.\n";
        return;
    }

    v.entryTime = time(NULL);

    int slot = findFreeSlot();

    if (slot == -1) {
        waiting.push(v);

        cout << "Parking is full.\n";
        cout << "Vehicle added to waiting list.\n";
    } else {
        slots[slot] = v;

        cout << "Vehicle parked successfully in slot "
             << slot + 1 << ".\n";

        cout << "Entry time: "
             << timeText(v.entryTime) << '\n';
    }

    saveData();
}

void vehicleExit() {
    string number;

    cout << "Enter vehicle number: ";
    cin >> number;

    int slot = findVehicle(number);

    if (slot == -1) {
        cout << "Vehicle is not currently parked.\n";
        return;
    }

    time_t exitTime = time(NULL);

    double hours =
        difftime(exitTime, slots[slot].entryTime) / 3600.0;

    int chargedHours = (int)hours;

    if (hours > chargedHours)
        chargedHours++;

    if (chargedHours < 1)
        chargedHours = 1;

    double rate =
        (slots[slot].type == "BIKE")
        ? BIKE_RATE : CAR_RATE;

    double fee = chargedHours * rate;

    cout << "\n--- PAYMENT RECEIPT ---\n";
    cout << "Vehicle: " << slots[slot].number << '\n';
    cout << "Type: " << slots[slot].type << '\n';

    cout << "Entry: "
         << timeText(slots[slot].entryTime) << '\n';

    cout << "Exit: "
         << timeText(exitTime) << '\n';

    cout << fixed << setprecision(2);

    cout << "Duration: " << hours << " hour(s)\n";
    cout << "Charged hours: " << chargedHours << '\n';
    cout << "Parking fee: " << fee << " Taka\n";

    History h;

    h.number = slots[slot].number;
    h.type = slots[slot].type;
    h.entryTime = slots[slot].entryTime;
    h.exitTime = exitTime;
    h.fee = fee;

    history.push_back(h);
    totalIncome += fee;

    slots[slot].number = "";
    slots[slot].type = "";
    slots[slot].entryTime = 0;

    if (!waiting.empty()) {
        slots[slot] = waiting.front();
        waiting.pop();

        cout << "Waiting vehicle "
             << slots[slot].number
             << " assigned to slot "
             << slot + 1 << ".\n";
    }

    saveData();
}

void showParked() {
    bool found = false;

    cout << "\n--- PARKED VEHICLES ---\n";

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (!slots[i].number.empty()) {
            found = true;

            cout << "Vehicle: " << slots[i].number
                 << " | Type: " << slots[i].type
                 << " | Slot: " << i + 1
                 << " | Entry: "
                 << timeText(slots[i].entryTime) << '\n';
        }
    }

    if (!found)
        cout << "No vehicles are currently parked.\n";
}

void showWaiting() {
    if (waiting.empty()) {
        cout << "Waiting list is empty.\n";
        return;
    }

    queue<Vehicle> temp = waiting;
    int position = 1;

    cout << "\n--- WAITING LIST ---\n";

    while (!temp.empty()) {
        cout << position++ << ". "
             << temp.front().number << " | "
             << temp.front().type << '\n';

        temp.pop();
    }
}

void searchVehicle() {
    string number;

    cout << "Enter vehicle number: ";
    cin >> number;

    int slot = findVehicle(number);

    if (slot != -1) {
        cout << "Status: PARKED\n";
        cout << "Slot: " << slot + 1 << '\n';
        cout << "Type: " << slots[slot].type << '\n';
        cout << "Entry: "
             << timeText(slots[slot].entryTime) << '\n';
        return;
    }

    queue<Vehicle> temp = waiting;
    int position = 1;

    while (!temp.empty()) {
        if (temp.front().number == number) {
            cout << "Status: WAITING\n";
            cout << "Position: " << position << '\n';
            return;
        }

        temp.pop();
        position++;
    }

    for (int i = 0; i < (int)history.size(); i++) {
        if (history[i].number == number) {
            cout << "Status: EXITED\n";

            cout << "Entry: "
                 << timeText(history[i].entryTime) << '\n';

            cout << "Exit: "
                 << timeText(history[i].exitTime) << '\n';

            cout << "Fee: "
                 << history[i].fee << " Taka\n";
            return;
        }
    }

    cout << "No record found for this vehicle.\n";
}

void showHistory() {
    cout << "\n--- PARKING HISTORY ---\n";

    if (history.empty()) {
        cout << "No completed parking records yet.\n";
        return;
    }

    for (int i = 0; i < (int)history.size(); i++) {
        cout << i + 1 << ". "
             << history[i].number
             << " | " << history[i].type
             << " | Entry: "
             << timeText(history[i].entryTime)
             << " | Exit: "
             << timeText(history[i].exitTime)
             << " | Fee: " << fixed << setprecision(2)
             << history[i].fee << " Taka\n";
    }
}

void showReport() {
    int occupied = 0;

    for (int i = 0; i < TOTAL_SLOTS; i++) {
        if (!slots[i].number.empty())
            occupied++;
    }

    cout << "\n--- PARKING REPORT ---\n";
    cout << "Total slots: " << TOTAL_SLOTS << '\n';
    cout << "Occupied slots: " << occupied << '\n';
    cout << "Available slots: "
         << TOTAL_SLOTS - occupied << '\n';
    cout << "Vehicles waiting: " << waiting.size() << '\n';
    cout << "Completed visits: " << history.size() << '\n';

    cout << "Total parking earnings: "
         << fixed << setprecision(2)
         << totalIncome << " Taka\n";
}

int main() {
    for (int i = 0; i < TOTAL_SLOTS; i++) {
        slots[i].number = "";
        slots[i].type = "";
        slots[i].entryTime = 0;
    }

    loadData();

    int choice = -1;

    do {
        cout << "\n====================================\n";
        cout << "   SMART PARKING MANAGEMENT SYSTEM\n";
        cout << "====================================\n";

        cout << "1. View parking slots\n";
        cout << "2. Vehicle entry\n";
        cout << "3. Vehicle exit / payment\n";
        cout << "4. View parked vehicles\n";
        cout << "5. View waiting list\n";
        cout << "6. View parking report\n";
        cout << "7. Search vehicle\n";
        cout << "8. View parking history\n";
        cout << "9. Save data\n";
        cout << "0. Save and exit\n";

        cout << "Choose an option: ";

        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(
                numeric_limits<streamsize>::max(), '\n'
            );

            cout << "Please enter a number.\n";
            choice = -1;
            continue;
        }

        switch (choice) {
            case 1:
                showSlots();
                break;

            case 2:
                vehicleEntry();
                break;

            case 3:
                vehicleExit();
                break;

            case 4:
                showParked();
                break;

            case 5:
                showWaiting();
                break;

            case 6:
                showReport();
                break;

            case 7:
                searchVehicle();
                break;

            case 8:
                showHistory();
                break;

            case 9:
                saveData();
                break;

            case 0:
                saveData();
                cout << "Program closed. Goodbye!\n";
                break;

            default:
                cout << "Invalid option. Choose 0 to 9.\n";
        }

    } while (choice != 0);

    return 0;
}
