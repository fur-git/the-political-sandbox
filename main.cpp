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
#include <algorithm>

using namespace std;

const string MENU = R"(

1. End turn.
2. Exit.
3. Propose a bill.
4. See pending bills.
5. See parties approval rates.
6. DEBUG: Skip to elections.

)";

const string INPUT = ">>> ";

const string PROPOSE_BILL = R"(

1. Amend election rate.
2. Amend votes required for constitutional bills.
3. Amend votes required for simple bills.
4. Amend electoral system.
5. Outlaw a political party.
6. Legalize a political party.
7. Amend form of government.

)";

const string VOTING = R"(

1. Aye.
2. Abstain.
3. Nay.

)";

const string PRESIDENTIAL_DECISION = R"(

1. Sign the bill.
2. Reject the bill.

)";

const string VOTES_REQUIRED_OPTIONS = R"(

1. Relative majority.
2. 50% + 1.
3. 2/3.
4. 3/4.

)";

const string ELECTORAL_SYSTEM_OPTIONS = R"(

1. Proportional.
2. Majoritarian.

)";

const string GOVERNMENT_FORM_OPTIONS = R"(

1. Republic.
2. Monarchy.

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
        enum class ElectoralSystemOption {
            Proportional,
            Majoritarian,
            PLACEHOLDER,
        };
        enum class GovernmentFormOption {
            Republic,
            Monarchy,
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
            bool isOutlawed;
        };
        struct Bill {
            string name;
            Ideology preferences;
            BillType type;
            uint16_t votingTurn;
            uint16_t optionalNumerical;
            VotesRequiredOption optionalVotesRequired;
            ElectoralSystemOption optionalElectoralSystem;
            GovernmentFormOption optionalGovernmentForm;
        };
        struct Constitution {
            uint16_t electionRate;
            VotesRequiredOption votesRequiredForConstitutionalBills;
            VotesRequiredOption votesRequiredForSimpleBills;
            ElectoralSystemOption electoralSystem;
            GovernmentFormOption governmentForm;
            void editElectionRate(uint16_t newRate) { electionRate = newRate; }
            void editVotesRequiredForConstitutionalBills(VotesRequiredOption newRequirement) { votesRequiredForConstitutionalBills = newRequirement; }
            void editVotesRequiredForSimpleBills(VotesRequiredOption newRequirement) { votesRequiredForSimpleBills = newRequirement; }
            void editElectoralSystem(ElectoralSystemOption newSystem) { electoralSystem = newSystem; }
            void editGovernmentForm(GovernmentFormOption newForm) { governmentForm = newForm; }
        };
        const Ideology _nationalism = {
            3,
            -3,
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
        const array<Bill, 7> _availableBills = {{
            {
                "amendElectionRate",
                { 0, -2, -1, -1, -1 },
                BillType::Constitutional,
            },
            {
                "amendVotesRequirementForConstitutional",
                { 0, -2, -1, -1, -1 },
                BillType::Constitutional,
            },
            {
                "amendVotesRequirementForSimple",
                { 0, -1, -1, -1, -1 },
                BillType::Constitutional,
            },
            {
                "amendElectoralSystem",
                { 0, -3, -2, -2, -2 },
                BillType::Constitutional,
            },
            {
                "outlawPoliticalParty",
                { 0, -3, -1, -2, -2 },
                BillType::Constitutional,
            },
            {
                "legalizePoliticalParty",
                { 0, 3, 1, 2, 2 },
                BillType::Constitutional,
            },
            {
                "amendGovernmentForm",
                { 0, -3, -2, -2, -2 },
                BillType::Constitutional,
            }
        }};
        vector<PoliticalParty> _politicalParties;
        vector<Bill> _pendingBills;
        PoliticalParty* _playerParty;
        PoliticalParty* _headOfGovernmentParty;
        Constitution _theConsitution;
        uint16_t _totalParlimentarySeats;
        uint16_t _currentTurn;
        uint16_t _electionTurn;
        bool _madeAnActionThisTurn;
        bool _submittedBillThisTurn;
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
        void makePoliticalPartyOutlawed(PoliticalParty& party) { party.isOutlawed = true; }
        void makePoliticalPartyLegal(PoliticalParty& party) { party.isOutlawed = false; }
        void nextTurn(void) { _currentTurn++; _madeAnActionThisTurn = false; }
        void electPresident(void) {
            uint8_t highestApprovalRatePartyIndex = 0;
            float highestApprovalRate = 0;
            for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                if (_politicalParties[i].isOutlawed) { continue; }
                if (_politicalParties[i].approvalPercentage > highestApprovalRate) {
                    highestApprovalRate = _politicalParties[i].approvalPercentage;
                    highestApprovalRatePartyIndex = i;
                }
            }
            _headOfGovernmentParty = &_politicalParties[highestApprovalRatePartyIndex];
        }
        void proposeBill(
                Bill bill,
                optional<uint16_t> valueNumerical,
                optional<VotesRequiredOption> valueVotesRequired,
                optional<ElectoralSystemOption> valueElectoralSystem,
                optional<GovernmentFormOption> valueGovernmentForm
            ) {
            bool isAValidBill = false;
            uint16_t actualValue = valueNumerical.value_or(0);
            VotesRequiredOption actualVotesRequired = valueVotesRequired.value_or(VotesRequiredOption::PLACEHOLDER);
            ElectoralSystemOption actualElectoralSystem = valueElectoralSystem.value_or(ElectoralSystemOption::PLACEHOLDER);
            GovernmentFormOption actualGovernmentForm = valueGovernmentForm.value_or(GovernmentFormOption::PLACEHOLDER);
            for (unsigned char i = 0; i < _availableBills.size(); i++) { if (bill.name == _availableBills[i].name) { isAValidBill = true; } }
            if (!isAValidBill) { cerr << "Not a valid bill" << endl; return; }
            uint16_t votingTurn = _currentTurn + 4;
            bill.votingTurn = votingTurn;
            bill.optionalNumerical = actualValue;
            bill.optionalVotesRequired = actualVotesRequired;
            bill.optionalElectoralSystem = actualElectoralSystem;
            bill.optionalGovernmentForm = actualGovernmentForm;
            if (bill.name == _availableBills[0].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[1].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[2].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[3].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[4].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[5].name) { _pendingBills.push_back(bill); }
            else if (bill.name == _availableBills[6].name) { _pendingBills.push_back(bill); }
            else { cerr << "Something unexpected happened (the bill name doesn't fit into any available bills)" << endl; }
            _madeAnActionThisTurn = true;
            _submittedBillThisTurn = true;
            _playerParty->approvalPercentage += 2;
        }
        void displayPartyApprovalRates(void) {
            for (PoliticalParty& party : _politicalParties) {
                cout << party.name << ": " << party.approvalPercentage << "%" << endl;
            }
        }
        bool checkIfApprovalRatesAreRight(void) {
            float sumFloat = 0;
            for (PoliticalParty& party : _politicalParties) {
                if (party.isOutlawed) { continue; }
                sumFloat += party.approvalPercentage;
            }
            sumFloat = round(sumFloat);
            if (sumFloat == 100) { return true; }
            else { return false; }
        }
        void correctifyApprovalRates(void) {
            bool wrong = true;
            float sum = 0;
            for (PoliticalParty& party : _politicalParties) {
                if (party.isOutlawed) { continue; }
                sum += party.approvalPercentage;
            }
            for (PoliticalParty& party : _politicalParties) {
                if (party.isOutlawed) {
                    party.approvalPercentage = 0;
                }
            }
            if (sum > 100) {
                while (wrong) {
                    if (checkIfApprovalRatesAreRight()) { wrong = false; }
                    else {
                        uint8_t highestApprovalRatePartyIndex = 0;
                        float highestApprovalRate = 0;
                        for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                            if (_politicalParties[i].isOutlawed) { continue; }
                            if (_politicalParties[i].name == _playerParty->name) { continue; }
                            if (_politicalParties[i].approvalPercentage > highestApprovalRate) {
                                highestApprovalRate = _politicalParties[i].approvalPercentage;
                                highestApprovalRatePartyIndex = i;
                            }
                        }
                        for (unsigned short i = 0; i < 10000; i++) {
                            _politicalParties[highestApprovalRatePartyIndex].approvalPercentage -= 0.01;
                            if (checkIfApprovalRatesAreRight()) { break; }
                        }
                    }
                }
            }
            else { 
                while (wrong) {
                    if (checkIfApprovalRatesAreRight()) { wrong = false; }
                    else {
                        uint8_t lowestApprovalRatePartyIndex = 0;
                        float lowestApprovalRate = 100;
                        for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                            if (_politicalParties[i].isOutlawed) { continue; }
                            if (_politicalParties[i].name == _playerParty->name) { continue; }
                            if (_politicalParties[i].approvalPercentage < lowestApprovalRate) {
                                lowestApprovalRate = _politicalParties[i].approvalPercentage;
                                lowestApprovalRatePartyIndex = i;
                            }
                        }
                        for (unsigned short i = 0; i < 10000; i++) {
                            _politicalParties[lowestApprovalRatePartyIndex].approvalPercentage += 0.01;
                            if (checkIfApprovalRatesAreRight()) { break; }
                        }
                    }
                }
            }
        }
        void startElections(ElectoralSystemOption electoralSystem) {
            switch (electoralSystem) {
                case ElectoralSystemOption::Proportional: {
                    for (PoliticalParty& party : _politicalParties) {
                        party.parliamentSeats = _totalParlimentarySeats * (party.approvalPercentage / 100);
                    }
                    break;
                }
                case ElectoralSystemOption::Majoritarian: {
                    float highestApprovalRate = 0;
                    uint8_t highestApprovalRatePartyIndex = 0;
                    for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                        _politicalParties[i].parliamentSeats = 0;
                        if (_politicalParties[i].approvalPercentage > highestApprovalRate) {
                            highestApprovalRate = _politicalParties[i].approvalPercentage;
                            highestApprovalRatePartyIndex = i;
                        }
                    }
                    _politicalParties[highestApprovalRatePartyIndex].parliamentSeats = _totalParlimentarySeats;
                    break;
                }
                default: {
                    cerr << "Something unexpected happened (the electoral system does not exist)" << endl; exit(EXIT_FAILURE);
                    break;
                }
            }
            if (_theConsitution.governmentForm == GovernmentFormOption::Republic) { electPresident(); }
            _electionTurn += _theConsitution.electionRate;
        }
        void voteOnBill(Bill& bill) {
            VotesRequiredOption requiredVotes;
            switch (bill.type) {
                case BillType::Constitutional: {
                    requiredVotes = checkHowManyVotesRequired(BillType::Constitutional);
                    break;
                }
                case BillType::Simple: {
                    requiredVotes = checkHowManyVotesRequired(BillType::Simple);
                    break;
                }
                default: {
                    cerr << "Something unexpected happened (the bill is neither of the BillType categories)" << endl;
                    exit(EXIT_FAILURE);
                    break;
                }
            }
            uint16_t ayeVotes = 0;
            uint16_t abstainVotes = 0;
            uint16_t nayVotes = 0;
            string userInput;
            if (_playerParty->parliamentSeats != 0) {
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
            }
            else { cout << "You cannot vote" << endl; inputAnythingToContinue(userInput); }
            for (PoliticalParty& party : _politicalParties) {
                if (&party == _playerParty) { continue; }
                if (party.parliamentSeats == 0) { continue; }
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
                case VotesRequiredOption::RelativeMajority: {
                    if (ayeVotes > nayVotes) { isPassed = true; }
                    break;
                }
                case VotesRequiredOption::FiftyPercentPlusOne: {
                    if (ayeVotes >= _totalParlimentarySeats / 2 + 1) { isPassed = true; }
                    break;
                }
                case VotesRequiredOption::TwoThirds: {
                    if (ayeVotes >= _totalParlimentarySeats / 3 * 2) { isPassed = true; }
                    break;
                }
                case VotesRequiredOption::ThreeFourths: {
                    if (ayeVotes >= _totalParlimentarySeats / 4 * 3) { isPassed = true; }
                    break;
                }
                default: {
                    cerr << "Something went wrong (the required votes don't match any options)" << endl;
                    exit(EXIT_FAILURE);
                    break;
                }
            }
            if (isPassed) {
                cout << "The bill has been PASSED with the votes of " << ayeVotes << "-" << abstainVotes << "-" << nayVotes << " (Aye-Abstain-Nay)" << endl;
                cout << "The bill has been sent to "
                     << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "President " : "Monarch ")
                     << _headOfGovernmentParty->name << endl;
                bool isSigned = false;
                if (_headOfGovernmentParty == _playerParty) {
                    bool validInput = false;
                    while (!validInput) {
                        cout << "As "
                             << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "President" : "Monarch")
                             << ", your party must sign or reject " << bill.name << endl;
                        cout << PRESIDENTIAL_DECISION << endl;
                        cout << INPUT;
                        cout.flush();
                        cin >> userInput;
                        if (userInput == "1") { isSigned = true; validInput = true; }
                        else if (userInput == "2") { validInput = true; }
                        else { cout << "Invalid option" << endl; }
                    }
                }
                else {
                    uint8_t supportCount = 0;
                    uint8_t oppositionCount = 0;
                    for (unsigned char i = 0; i < _headOfGovernmentParty->ideology.preferences.size(); i++) {
                        if (_headOfGovernmentParty->ideology.preferences[i] > bill.preferences.preferences[i]) { supportCount++; }
                        else if (_headOfGovernmentParty->ideology.preferences[i] < bill.preferences.preferences[i]) { oppositionCount++; }
                    }
                    if (supportCount > oppositionCount) { isSigned = true; }
                }
                if (isSigned) {
                    cout << "The "
                         << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "President" : "Monarch")
                         << " has SIGNED the bill, it is now law" << endl;
                    if (bill.name == _availableBills[0].name) { _theConsitution.editElectionRate(bill.optionalNumerical); }
                    else if (bill.name == _availableBills[1].name) { _theConsitution.editVotesRequiredForConstitutionalBills(bill.optionalVotesRequired); }
                    else if (bill.name == _availableBills[2].name) { _theConsitution.editVotesRequiredForSimpleBills(bill.optionalVotesRequired); }
                    else if (bill.name == _availableBills[3].name) { _theConsitution.editElectoralSystem(bill.optionalElectoralSystem); }
                    else if (bill.name == _availableBills[4].name) {
                        if (bill.optionalNumerical >= _politicalParties.size()) {
                            cerr << "Something went wrong (the political party does not exist)" << endl;
                            exit(EXIT_FAILURE);
                        }
                        PoliticalParty& party = _politicalParties[bill.optionalNumerical];
                        makePoliticalPartyOutlawed(party);
                    }
                    else if (bill.name == _availableBills[5].name) {
                        if (bill.optionalNumerical >= _politicalParties.size()) {
                            cerr << "Something went wrong (the political party does not exist)" << endl;
                            exit(EXIT_FAILURE);
                        }
                        PoliticalParty& party = _politicalParties[bill.optionalNumerical];
                        makePoliticalPartyLegal(party);
                    }
                    else if (bill.name == _availableBills[6].name) { _theConsitution.editGovernmentForm(bill.optionalGovernmentForm); }
                    else { cerr << "Something went wrong (bill is non-existent)" << endl; exit(EXIT_FAILURE); }
                }
                else {
                    cout << "The "
                         << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "President" : "Monarch")
                         << " has REJECTED the bill, it will not become law" << endl;
                }
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
            _submittedBillThisTurn = false;
            _totalParlimentarySeats = totalParlimentarySeats;
            _theConsitution = {
                48,
                VotesRequiredOption::TwoThirds,
                VotesRequiredOption::RelativeMajority,
                ElectoralSystemOption::Proportional,
                GovernmentFormOption::Republic
            };
            _electionTurn = _theConsitution.electionRate;
            for (unsigned char i = 0; i < amountOfParties; i++) {
                const uint8_t partyNameIndex = rand() % possiblePartyNames.size();
                const string partyName = possiblePartyNames[partyNameIndex];
                possiblePartyNames.erase(possiblePartyNames.begin() + partyNameIndex);
                const uint8_t partyIdeologyIndex = rand() % 4;
                Ideology partyIdeology;
                switch (partyIdeologyIndex) {
                    case 0: {
                        partyIdeology = _nationalism;
                        break;
                    }
                    case 1: {
                        partyIdeology = _republican;
                        break;
                    }
                    case 2: {
                        partyIdeology = _anarchist;
                        break;
                    }
                    case 3: {
                        partyIdeology = _nonAligned;
                        break;
                    }
                    default: {
                        cerr << "Something unexpected happened (something other than 0-3 got generated in ideology generation)" << endl;
                        exit(EXIT_FAILURE);
                        break;
                    }
                }
                float partyApprovalPercentage = 100.0f / amountOfParties;
                partyApprovalPercentage = round(partyApprovalPercentage * 100) / 100;
                const uint16_t partyParlimentarySeats = _totalParlimentarySeats * (partyApprovalPercentage / 100);
                PoliticalParty party = {
                    partyName,
                    partyApprovalPercentage,
                    partyParlimentarySeats,
                    partyIdeology,
                    false,
                };
                _politicalParties.push_back(party);
            }
            const uint8_t playerPartyIndex = rand() % _politicalParties.size();
            _playerParty = &_politicalParties[playerPartyIndex];
            electPresident();
        }
        void run(void) {
            bool stopFlag = false;
            string userInput;
            while (!stopFlag) {
                system("clear");
                _submittedBillThisTurn = false;
                if (!checkIfApprovalRatesAreRight()) { correctifyApprovalRates(); }
                if (_electionTurn == _currentTurn) {
                    startElections(_theConsitution.electoralSystem);
                    cout << "Elections concluded" << endl;
                    for (PoliticalParty& party : _politicalParties) { cout << party.name << " now has " << party.parliamentSeats << " seats in the parliament" << endl; }
                    if (_theConsitution.governmentForm == GovernmentFormOption::Republic) {
                        cout << _headOfGovernmentParty->name << " won the presidency with " << _headOfGovernmentParty->approvalPercentage << "% approval" << endl;
                    }
                    else { cout << "Monarch " << _headOfGovernmentParty->name << " remains head of government" << endl; }
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
                cout << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "President: " : "Monarch: ")
                     << _headOfGovernmentParty->name << endl;
                cout << MENU << endl;
                cout << "Form of government: "
                     << (_theConsitution.governmentForm == GovernmentFormOption::Republic ? "Republic" : "Monarchy")
                     << endl;
                cout << "Electoral system: "
                     << (_theConsitution.electoralSystem == ElectoralSystemOption::Proportional
                         ? "Proportional"
                         : "Majoritarian")
                     << endl;
                cout << "Elections in " << _electionTurn - _currentTurn << " turns" << endl;
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
                else if (userInput == "5") { displayPartyApprovalRates(); inputAnythingToContinue(userInput); }
                else if (userInput == "6") { _currentTurn = _electionTurn - 1; }
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
                            proposeBill(
                                _availableBills[0],
                                stoi(userInput),
                                nullopt,
                                nullopt,
                                nullopt
                            );
                        }
                        else if (userInput == "2") {
                            cout << VOTES_REQUIRED_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") {
                                proposeBill(
                                    _availableBills[1],
                                    nullopt,
                                    VotesRequiredOption::RelativeMajority,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "2") {
                                proposeBill(
                                    _availableBills[1],
                                    nullopt,
                                    VotesRequiredOption::FiftyPercentPlusOne,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "3") {
                                proposeBill(
                                    _availableBills[1],
                                    nullopt,
                                    VotesRequiredOption::TwoThirds,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "4") {
                                proposeBill(
                                    _availableBills[1],
                                    nullopt,
                                    VotesRequiredOption::ThreeFourths,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                        }
                        else if (userInput == "3") {
                            cout << VOTES_REQUIRED_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") {
                                proposeBill(
                                    _availableBills[2],
                                    nullopt,
                                    VotesRequiredOption::RelativeMajority,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "2") {
                                proposeBill(
                                    _availableBills[2],
                                    nullopt,
                                    VotesRequiredOption::FiftyPercentPlusOne,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "3") {
                                proposeBill(
                                    _availableBills[2],
                                    nullopt,
                                    VotesRequiredOption::TwoThirds,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else if (userInput == "4") {
                                proposeBill(
                                    _availableBills[2],
                                    nullopt,
                                    VotesRequiredOption::ThreeFourths,
                                    nullopt,
                                    nullopt
                                );
                            }
                            else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                        }
                        else if (userInput == "4") {
                            cout << ELECTORAL_SYSTEM_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") {
                                proposeBill(
                                    _availableBills[3],
                                    nullopt,
                                    nullopt,
                                    ElectoralSystemOption::Proportional,
                                    nullopt
                                );
                            }
                            else if (userInput == "2") {
                                proposeBill(
                                    _availableBills[3],
                                    nullopt,
                                    nullopt,
                                    ElectoralSystemOption::Majoritarian,
                                    nullopt
                                );
                            }
                            else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                        }
                        else if (userInput == "5") {
                            cout << "Select a political party to outlaw:" << endl;
                            for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                                if (&_politicalParties[i] == _playerParty || _politicalParties[i].isOutlawed) { continue; }
                                cout << i + 1 << ". " << _politicalParties[i].name << endl;
                            }
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            try {
                                const int selectedParty = stoi(userInput);
                                const int amountOfPoliticalParties = _politicalParties.size();
                                if (selectedParty < 1 || selectedParty > amountOfPoliticalParties) {
                                    cout << "Invalid option" << endl;
                                }
                                else {
                                    const uint16_t selectedPartyIndex = selectedParty - 1;
                                    PoliticalParty& selectedPoliticalParty = _politicalParties[selectedPartyIndex];
                                    if (&selectedPoliticalParty == _playerParty || selectedPoliticalParty.isOutlawed) {
                                        cout << "Invalid option" << endl;
                                    }
                                    else {
                                        proposeBill(
                                            _availableBills[4],
                                            selectedPartyIndex,
                                            nullopt,
                                            nullopt,
                                            nullopt
                                        );
                                    }
                                }
                            }
                            catch (const exception&) {
                                cout << "Invalid option" << endl;
                            }
                        }
                        else if (userInput == "6") {
                            cout << "Select a political party to legalize:" << endl;
                            for (unsigned char i = 0; i < _politicalParties.size(); i++) {
                                if (!_politicalParties[i].isOutlawed) { continue; }
                                cout << i + 1 << ". " << _politicalParties[i].name << endl;
                            }
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            try {
                                const int selectedParty = stoi(userInput);
                                const int amountOfPoliticalParties = _politicalParties.size();
                                if (selectedParty < 1 || selectedParty > amountOfPoliticalParties) {
                                    cout << "Invalid option" << endl;
                                }
                                else {
                                    const uint16_t selectedPartyIndex = selectedParty - 1;
                                    PoliticalParty& selectedPoliticalParty = _politicalParties[selectedPartyIndex];
                                    if (!selectedPoliticalParty.isOutlawed) {
                                        cout << "Invalid option" << endl;
                                    }
                                    else {
                                        proposeBill(
                                            _availableBills[5],
                                            selectedPartyIndex,
                                            nullopt,
                                            nullopt,
                                            nullopt
                                        );
                                    }
                                }
                            }
                            catch (const exception&) {
                                cout << "Invalid option" << endl;
                            }
                        }
                        else if (userInput == "7") {
                            cout << GOVERNMENT_FORM_OPTIONS << endl;
                            cout << INPUT;
                            cout.flush();
                            cin >> userInput;
                            if (userInput == "1") {
                                proposeBill(
                                    _availableBills[6],
                                    nullopt,
                                    nullopt,
                                    nullopt,
                                    GovernmentFormOption::Republic
                                );
                            }
                            else if (userInput == "2") {
                                proposeBill(
                                    _availableBills[6],
                                    nullopt,
                                    nullopt,
                                    nullopt,
                                    GovernmentFormOption::Monarchy
                                );
                            }
                            else { cout << "Invalid option" << endl; }
                        }
                        else { cout << "Invalid option" << endl; }
                        inputAnythingToContinue(userInput);
                    }
                }
                else { cout << "Invalid option" << endl; inputAnythingToContinue(userInput); }
                if (!_submittedBillThisTurn && _pendingBills.empty()) { _playerParty->approvalPercentage -= 0.5; }
            }
        }
};

int main(void) {
    srand(static_cast<unsigned int>(time(nullptr)));
    ParliamentEmulation parliament(4, 100);
    parliament.run();
    return 0;
}
