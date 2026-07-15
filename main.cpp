#include <iostream>
#include <map>
#include <cmath>
#include <vector>
#include <cstdint>
#include <string>
#include <array>
#include <cstdlib>
#include <ctime>
#include <optional>

using namespace std;

const string MENU = R"(

1. End turn.
2. Exit.
3. Propose a bill.
4. See pending bills.
5. DEBUG: Skip to elections.


Elections in )";

const string INPUT = ">>> ";

const string PROPOSE_BILL = R"(

1. Amend election rate.
2. Amend votes required for constitutional bills.
3. Amend votes required for simple bills.

)";

const string VOTING = R"(

1. Aye.
2. Abstain.
3. Nay.

)";

const string VOTES_REQUIRED_OPTIONS = R"(

1. Relative majority.
2. 50% + 1.
3. 2/3.
4. 3/4.

)";


class ParliamentEmulation {
    private:
        enum class BillType {
            Simple,
            Constitutional,
        };
        enum class IdeologicalCategory {
            Military,
            Democracy,
            InternalPolicy,
            Unity,
            Stability,
        };
        enum class VotesRequiredOption {
            RelativeMajority,
            FiftyPercentPlusOne,
            TwoThirds,
            ThreeFourths,
            PLACEHOLDER,
        };
        struct Ideology {
            array<int8_t, 5> preferences;
        };
        struct PoliticalParty {
            string name;
            float approvalPercentage;
            uint16_t parliamentSeats;
            Ideology ideology;
        };
        struct Bill {
            string name;
            Ideology preferences;
            BillType type;
            uint16_t votingTurn;
            uint16_t optionalNumerical;
            VotesRequiredOption optionalVotesRequired;
        };
        struct Constitution {
            uint16_t electionRate;
            VotesRequiredOption votesRequiredForConstitutionalBills;
            VotesRequiredOption votesRequiredForSimpleBills;
            void editElectionRate(uint16_t newRate) { electionRate = newRate; }
            void editVotesRequiredForConstitutionalBills(VotesRequiredOption newRequirement) { votesRequiredForConstitutionalBills = newRequirement; }
            void editVotesRequiredForSimpleBills(VotesRequiredOption newRequirement) { votesRequiredForSimpleBills = newRequirement; }
        };
        const Ideology _nationalism = {
            3,
            -2,
            3,
            -1,
            -1,
        };
        const Ideology _republican = {
            1,
            2,
            1,
            2,
            2,
        };
        const Ideology _anarchist = {
            0,
            0,
            0,
            -2,
            -3,
        };
        const Ideology _nonAligned = {
            0,
            0,
            0,
            0,
            0,
        };
        const array<Bill, 2> _availableBills = {{
            {
                "amendElectionRate",
                { 0, -2, -1, -1, -1 },
                BillType::Constitutional,
            },
            {
                "amendVotesRequirementForConstitutional",
                { 0, 0, 0, 0, 0 }, // intentionally zeros for testing purposes
                BillType::Constitutional,
            }
        }};
        vector<PoliticalParty> _politicalParties;
        vector<Bill> _pendingBills;
        PoliticalParty* _playerParty;
        Constitution _theConsitution;
        uint16_t _totalParlimentarySeats;
        uint16_t _currentTurn;
        uint16_t _electionTurn;
        bool _madeAnActionThisTurn;
        static void inputAnythingToContinue(string& buffer) {
            cout << "Input anything to continue" << endl;
            cin >> buffer;
            return;
        }
        VotesRequiredOption checkHowManyVotesRequired(BillType billType) {
            if (billType == BillType::Constitutional) { return _theConsitution.votesRequiredForConstitutionalBills; }
            else if (billType == BillType::Simple) { return _theConsitution.votesRequiredForSimpleBills; }
            else { cerr << "Something unexpected happened (the BillType is not constitutional nor simple)" << endl; exit(EXIT_FAILURE); }
        }
        void nextTurn(void) { _currentTurn++; _madeAnActionThisTurn = false; }
        void proposeBill(Bill bill, optional<uint16_t> valueNumerical, optional<VotesRequiredOption> valueVotesRequired) {
            bool isAValidBill = false;
            uint16_t actualValue = valueNumerical.value_or(0);
            VotesRequiredOption actualVotesRequired = valueVotesRequired.value_or(VotesRequiredOption::PLACEHOLDER);
            for (unsigned char i = 0; i < _availableBills.size(); i++) { if (bill.name == _availableBills[i].name) { isAValidBill = true; } }
            if (!isAValidBill) { cerr << "Not a valid bill" << endl; return; }
            uint16_t votingTurn = _currentTurn + 4;
            bill.votingTurn = votingTurn;
            bill.optionalNumerical = actualValue;
            bill.optionalVotesRequired = actualVotesRequired;
            if (bill.name == _availableBills[0].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[1].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[2].name) { _pendingBills.push_back(bill); }
            else { cerr << "Something unexpected happened (the bill name doesn't fit into any available bills)" << endl; }
        }
        void startElections(void) {
            for (PoliticalParty& party : _politicalParties) { party.parliamentSeats = _totalParlimentarySeats * (party.approvalPercentage / 100); }
            _electionTurn += _theConsitution.electionRate;
        }
        void voteOnBill(Bill& bill) {
            VotesRequiredOption requiredVotes;
            if (bill.type == BillType::Constitutional) { requiredVotes = checkHowManyVotesRequired(BillType::Constitutional); }
            else if (bill.type == BillType::Simple) { requiredVotes = checkHowManyVotesRequired(BillType::Simple); }
            else { cerr << "Something went wrong (the bill does not fit into any BillType category)" << endl; exit(EXIT_FAILURE); }
            uint16_t ayeVotes = 0;
            uint16_t abstainVotes = 0;
            uint16_t nayVotes = 0;
            string userInput;
            bool validInput = false;
            while (!validInput) {
                cout << VOTING << endl;
                cout << INPUT;
                cout.flush();
                cin >> userInput;
                if (userInput == "1") { ayeVotes += _playerParty->parliamentSeats; validInput = true; }
                else if (userInput == "2") { abstainVotes += _playerParty->parliamentSeats; validInput = true; }
                else if (userInput == "3") { nayVotes += _playerParty->parliamentSeats; validInput = true; }
                else { cout << "Invalid option" << endl; }
            }
            for (PoliticalParty& party : _politicalParties) {
                if (&party == _playerParty) { continue; }
                uint8_t ayeCount = 0;
                uint8_t abstainCount = 0;
                uint8_t nayCount = 0;
                for (unsigned char i = 0; i < party.ideology.preferences.size(); i++) {
                    if (party.ideology.preferences[i] == bill.preferences.preferences[i]) { abstainCount++; }
                    else if (party.ideology.preferences[i] > bill.preferences.preferences[i]) { ayeCount++; }
                    else if (party.ideology.preferences[i] < bill.preferences.preferences[i]) { nayCount++; }
                }
                if (abstainCount > ayeCount && abstainCount > nayCount) { abstainVotes += party.parliamentSeats; }
                else if (ayeCount > abstainCount && ayeCount > nayCount) { ayeVotes += party.parliamentSeats; }
                else if (nayCount > abstainCount && nayCount > ayeCount) { nayVotes += party.parliamentSeats; }
                else { abstainVotes += party.parliamentSeats; }
            }
            bool isPassed = false;
            switch (requiredVotes) {
                case VotesRequiredOption::RelativeMajority:
                    if (ayeVotes > nayVotes) { isPassed = true; }
                    break;
                case VotesRequiredOption::FiftyPercentPlusOne:
                    if (ayeVotes >= _totalParlimentarySeats / 2 + 1) { isPassed = true; }
                    break;
                case VotesRequiredOption::TwoThirds:
                    if (ayeVotes >= _totalParlimentarySeats / 3 * 2) { isPassed = true; }
                    break;
                case VotesRequiredOption::ThreeFourths:
                    if (ayeVotes >= _totalParlimentarySeats / 4 * 3) { isPassed = true; }
                    break;
                default:
                    cerr << "Something went wrong (the required votes don't match any options)" << endl;
                    exit(EXIT_FAILURE);
                    break;
            }
            if (isPassed) {
                cout << "The bill has been PASSED with the votes of " << ayeVotes << "-" << abstainVotes << "-" << nayVotes << " (Aye-Abstain-Nay)" << endl;
                if (bill.name == _availableBills[0].name) { _theConsitution.editElectionRate(bill.optionalNumerical); }
                else if (bill.name == _availableBills[1].name) { _theConsitution.editVotesRequiredForConstitutionalBills(bill.optionalVotesRequired); }
                else if (bill.name == _availableBills[2].name) { _theConsitution.editVotesRequiredForSimpleBills(bill.optionalVotesRequired); }
                else { cerr << "Something went wrong (bill is non-existent)" << endl; exit(EXIT_FAILURE); }
            } else {
                cout << "The bill has been REJECTED with the votes of " << ayeVotes << "-" << abstainVotes << "-" << nayVotes << " (Aye-Abstain-Nay)" << endl;
            }
            _pendingBills.pop_back();
            inputAnythingToContinue(userInput);
        }
    public:
        ParliamentEmulation(const uint8_t amountOfParties, const uint16_t totalParlimentarySeats) {
            vector<string> possiblePartyNames = {"Justice Party", "Unity Party", "Republican Party", "Green Party"};
            _currentTurn = 1;
            _madeAnActionThisTurn = false;
            _totalParlimentarySeats = totalParlimentarySeats;
            _theConsitution = { 48, VotesRequiredOption::TwoThirds, VotesRequiredOption::RelativeMajority };
            _electionTurn = _theConsitution.electionRate;
            for (unsigned char i = 0; i < amountOfParties; i++) {
                const uint8_t partyNameIndex = rand() % possiblePartyNames.size();
                const string partyName = possiblePartyNames[partyNameIndex];
                possiblePartyNames.erase(possiblePartyNames.begin() + partyNameIndex);
                const uint8_t partyIdeologyIndex = rand() % 4;
                Ideology partyIdeology;
                switch (partyIdeologyIndex) {
                    case 0:
                        partyIdeology = _nationalism;
                        break;
                    case 1:
                        partyIdeology = _republican;
                        break;
                    case 2:
                        partyIdeology = _anarchist;
                        break;
                    case 3:
                        partyIdeology = _nonAligned;
                        break;
                    default:
                        cerr << "Something unexpected happened (something other than 0-3 got generated in ideology generation)" << endl;
                        exit(EXIT_FAILURE);
                        break;
                }
                float partyApprovalPercentage = 100.0f / amountOfParties;
                partyApprovalPercentage = round(partyApprovalPercentage * 100) / 100;
                const uint16_t partyParlimentarySeats = _totalParlimentarySeats * (partyApprovalPercentage / 100);
                PoliticalParty party = {
                    partyName,
                    partyApprovalPercentage,
                    partyParlimentarySeats,
                    partyIdeology,
                };
                _politicalParties.push_back(party);
            }
            const uint8_t playerPartyIndex = rand() % _politicalParties.size();
            _playerParty = &_politicalParties[playerPartyIndex];
        }
        void run(void) {
            bool stopFlag = false;
            string userInput;
            while (!stopFlag) {
                system("clear");
                if (_electionTurn == _currentTurn) {
                    startElections();
                    cout << "Elections concluded" << endl;
                    for (PoliticalParty& party : _politicalParties) { cout << party.name << " now has " << party.parliamentSeats << " seats in the parliament" << endl; }
                    inputAnythingToContinue(userInput);
                }
                if (!_pendingBills.empty()) {
                    if (_pendingBills[0].votingTurn == _currentTurn) {
                        voteOnBill(_pendingBills[0]);
                    }
                }
                system("clear");
                cout << "Current turn is " << _currentTurn << endl;
                cout << "Your party is " << _playerParty->name << endl;
                cout << MENU << _electionTurn - _currentTurn << " turns" << endl;
                cout << INPUT;
                cout.flush();
                cin >> userInput;
                if (userInput == "1") { nextTurn(); }
                else if (userInput == "2") { stopFlag = true; }
                else if (userInput == "4") {
                    if (_pendingBills.empty()) { cout << "No bills to see" << endl; }
                    else { cout << _pendingBills[0].name << endl; }
                    inputAnythingToContinue(userInput);
                }
                else if (userInput == "5") {
                    _currentTurn = _electionTurn - 1;
                }
                else if (_madeAnActionThisTurn) { cout << "You cannot make any other actions this turn" << endl; inputAnythingToContinue(userInput); }
                else if (userInput == "3") {
                    if (!_pendingBills.empty()) { cout << "A bill is already pending" << endl; inputAnythingToContinue(userInput); }
                    else {
                        cout << PROPOSE_BILL << endl;
                        cout << INPUT;
                        cout.flush();
                        cin >> userInput;
                        if (userInput == "1") {
                            cout << "Enter the new rate: ";
                            cout.flush();
                            cin >> userInput;
                            proposeBill(_availableBills[0], stoi(userInput), nullopt);
                            _madeAnActionThisTurn = true;
                        }
                        else if (userInput == "2") {
                            cout << VOTES_REQUIRED_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") { proposeBill(_availableBills[1], nullopt, VotesRequiredOption::RelativeMajority); }
                            else if (userInput == "2") { proposeBill(_availableBills[1], nullopt, VotesRequiredOption::FiftyPercentPlusOne); }
                            else if (userInput == "3") { proposeBill(_availableBills[1], nullopt, VotesRequiredOption::TwoThirds); }
                            else if (userInput == "4") { proposeBill(_availableBills[1], nullopt, VotesRequiredOption::ThreeFourths); }
                            else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                        }
                        else if (userInput == "3") {
                            cout << VOTES_REQUIRED_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") { proposeBill(_availableBills[2], nullopt, VotesRequiredOption::RelativeMajority); }
                            else if (userInput == "2") { proposeBill(_availableBills[2], nullopt, VotesRequiredOption::FiftyPercentPlusOne); }
                            else if (userInput == "3") { proposeBill(_availableBills[2], nullopt, VotesRequiredOption::TwoThirds); }
                            else if (userInput == "4") { proposeBill(_availableBills[2], nullopt, VotesRequiredOption::ThreeFourths); }
                            else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                        }
                        else { cout << "Invalid option" << endl; }
                        inputAnythingToContinue(userInput);
                    }
                }
                else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
            }
        }
};

int main(void) {
    srand(static_cast<unsigned int>(time(nullptr)));
    ParliamentEmulation parliament(4, 100);
    parliament.run();
    return 0;
}
